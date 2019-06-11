#include <gammu-statemachine.h>
#include <gammu.h>
#include "misc/coding/coding.h"
#include "debug.h"
#include "gsmstate.h"

#include "bitstream.h"
#include "cdma.h"

#include <assert.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

int Decode7bitASCII(char *dest, const unsigned char *src, int src_length)
{
  BitReader reader;
  int i;

  BitReader_SetStart(&reader, src);
  for(i = 0; i < src_length; i++)
    dest[i] = BitReader_ReadBits(&reader, 7);

  return BitReader_GetPosition(&reader);
}

int Encode7bitASCII(unsigned char *dest, const char *src, int src_length)
{
  BitWriter writer;
  int i;

  BitWriter_SetStart(&writer, dest);
  for(i = 0; i < src_length; i++)
    BitWriter_WriteBits(&writer, src[i], 7);

  BitWriter_Flush(&writer);
  return BitWriter_GetPosition(&writer);
}

int CDMA_DecodeWithBCDAlphabet(unsigned char value)
{
  return 10*(value >> 4u) + (value & 0x0fu);
}

GSM_Error CDMA_CheckTeleserviceID(GSM_Debug_Info *di, const unsigned char *pos, int length)
{
  int teleservice_id;

  if(length != 2) {
    smfprintf(di, "Invalid Teleservice ID parameter length, expected 2 but given %d\n", length);
    return ERR_CORRUPTED;
  }
  teleservice_id = ntohs(*(unsigned short*)pos);

  if(teleservice_id != 4098) {
    smfprintf(di, "Unsupported teleservice identifier: %d\n", teleservice_id);
    return ERR_NOTSUPPORTED;
  }

  smfprintf(di, "Teleservice: [%d] %s\n", teleservice_id, TeleserviceIdToString(teleservice_id));

  return ERR_NONE;
}

GSM_Error ATCDMA_DecodeSMSDateTime(GSM_Debug_Info *di, GSM_DateTime *DT, const unsigned char *req)
{
  DT->Year    = CDMA_DecodeWithBCDAlphabet(req[0]);
  if (DT->Year < 90) {
    DT->Year = DT->Year + 2000;
  } else {
    DT->Year = DT->Year + 1990;
  }
  DT->Month   = CDMA_DecodeWithBCDAlphabet(req[1]);
  DT->Day     = CDMA_DecodeWithBCDAlphabet(req[2]);
  DT->Hour    = CDMA_DecodeWithBCDAlphabet(req[3]);
  DT->Minute  = CDMA_DecodeWithBCDAlphabet(req[4]);
  DT->Second  = CDMA_DecodeWithBCDAlphabet(req[5]);
  DT->Timezone = GSM_GetLocalTimezoneOffset();

  if (!CheckDate(DT) || !CheckTime(DT)) {
    smfprintf(di, "Invalid date & time!\n");
    DT->Year = 0;
    return ERR_NONE;
  }

  smfprintf(di, "Decoding date & time: %s\n", OSDateTime(*DT, TRUE));

  return ERR_NONE;
}

GSM_Error ATCDMA_DecodePDUFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS, const unsigned char *buffer, size_t length, size_t *final_pos) {
  int udh = 0;
  size_t pos = 0;
  int datalength = 0;
  unsigned char output[161];
  SMS_ENCODING encoding;
  GSM_Error error;

  GSM_SetDefaultReceivedSMSData(SMS);

  if (SMS->State == SMS_Read || SMS->State == SMS_UnRead) {
    smfprintf(di, "SMS type: Deliver\n");
    SMS->PDU = SMS_Deliver;
  } else {
    smfprintf(di, "SMS type: Submit\n");
    SMS->PDU = SMS_Submit;
  }

  error = GSM_UnpackSemiOctetNumber(di, SMS->Number, buffer, &pos, length, FALSE);
  if (error != ERR_NONE)
    return error;

  if (SMS->PDU == SMS_Submit) {
    error = GSM_UnpackSemiOctetNumber(di, SMS->OtherNumbers[0], buffer, &pos, length, FALSE);
    if (error != ERR_NONE)
      return error;

    smfprintf(di, "Destination Address: \"%s\"\n", DecodeUnicodeString(SMS->Number));
    smfprintf(di, "Callback Address: \"%s\"\n", DecodeUnicodeString(SMS->OtherNumbers[0]));
  } else {
    smfprintf(di, "Originating Address: \"%s\"\n", DecodeUnicodeString(SMS->Number));
  }

  if (SMS->PDU == SMS_Deliver) {
    error = ATCDMA_DecodeSMSDateTime(di, &SMS->DateTime, buffer + pos);
    pos += 6;
    if (error != ERR_NONE)
      return error;
  }

  error = CDMA_CheckTeleserviceID(di, buffer + pos, 2);
  pos += 2;
  if (error != ERR_NONE)
    return error;

  // TODO: [KS] How to map priorities?
  SMS->Priority = buffer[pos++];
  smfprintf(di, "Priority: [%d] %s\n", SMS->Priority, CDMA_SMSPriorityToString(SMS->Priority));

  encoding = buffer[pos++];
  smfprintf(di, "Encoding: [%d] %s\n", encoding, CDMA_SMSEncodingToString(encoding));

  udh = buffer[pos++];
  smfprintf(di, "UDH Present: %s\n", udh == 0 ? "No" : "Yes");
  if (udh != 0) {
    smfprintf(di, "UDH not currently supported.\n");
    return ERR_ABORTED;
  }

  datalength = buffer[pos++];
  smfprintf(di, "Data length: %d\n", datalength);

  switch (encoding) {
    case SMS_ENC_ASCII:
      SMS->Coding = SMS_Coding_ASCII;
      pos += Decode7bitASCII(output, buffer + pos, datalength);
      EncodeUnicode(SMS->Text, output, datalength);
      break;
    case SMS_ENC_GSM:
      SMS->Coding = SMS_Coding_Default_No_Compression;
      pos += Decode7bitASCII(output, buffer + pos, datalength);
      DecodeDefault(SMS->Text, output, SMS->Length, TRUE, NULL);
      break;
    case SMS_ENC_OCTET:
      SMS->Coding = SMS_Coding_8bit;
      memcpy(output, buffer + pos, datalength);
      pos += datalength;
      EncodeUnicode(SMS->Text, output, datalength);
      break;
    default:
      smfprintf(di, "Unsupported encoding.\n");
      error = ERR_ABORTED;
  }

  if (error != ERR_NONE)
    return error;

  SMS->Length = datalength;

#ifdef DEBUG
  if(encoding != SMS_ENC_GSM)
    DumpMessageText(&GSM_global_debug, SMS->Text, SMS->Length * 2);
#endif

  if (final_pos)
    *final_pos = pos;

  return error;
}

GSM_Error ATCDMA_EncodePDUFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, int *length)
{
  GSM_Error error = ERR_NONE;
  int encoding_ofs = -1;
  char *sms_text = NULL;
  unsigned char *dest_ptr = NULL;
  size_t len = 0;

  len = GSM_PackSemiOctetNumber(SMS->Number, buffer + 1, FALSE);
  *buffer = len;
  *length += len + 1;
  smfprintf(di, "Destination Address: \"%s\"\n", DecodeUnicodeString(SMS->Number));

  if(SMS->CallbackIndex >= 0) {
    assert(SMS->OtherNumbersNum > 0);
    len = GSM_PackSemiOctetNumber(SMS->OtherNumbers[SMS->CallbackIndex], buffer + *length + 1, FALSE);
    *(buffer + *length) = len;
    *length += len + 1;
    smfprintf(di, "Callback Address: \"%s\"\n", DecodeUnicodeString(SMS->OtherNumbers[SMS->CallbackIndex]));
  } else {
    *(buffer + (*length)++) = 0;
  }

  *((unsigned short*)&buffer[*length]) = htons(TELESERVICE_ID_SMS);
  *length += 2;

  buffer[(*length)++] = SMS->Priority;

  encoding_ofs = (*length)++;

  // TODO: [KS] support UDH
  buffer[(*length)++] = 0;
  buffer[(*length)++] = SMS->Length;

  sms_text = DecodeUnicodeString(SMS->Text);
  len = strlen(sms_text);
  dest_ptr = buffer + *length;
  switch(SMS->Coding) {
    case SMS_Coding_Default_No_Compression:
      buffer[encoding_ofs] = SMS_ENC_GSM;
      EncodeDefault(dest_ptr, SMS->Text, &len, TRUE, NULL);
      *length += Encode7bitASCII(dest_ptr, dest_ptr, len);
      break;
    case SMS_Coding_8bit:
      buffer[encoding_ofs] = SMS_ENC_OCTET;
      memcpy(dest_ptr, sms_text, len);
      *length += len;
      break;
    default:
      buffer[encoding_ofs] = SMS_ENC_ASCII;
      *length += Encode7bitASCII(dest_ptr, sms_text, len);
  }

  return error;
}

const char *NetworkTypeToString(NETWORK_TYPE networkType)
{
  switch(networkType) {
    case NETWORK_CDMA : return "CDMA";
    case NETWORK_GSM  : return "GSM";
    case NETWORK_AUTO : return "Undetermined";
    default           : return "Unknown";
  }
}

const char *CDMA_MsgTypeToString(CDMA_MSG_TYPE msgType)
{
  switch(msgType) {
    case CDMA_MSG_TYPE_P2P   : return "Point to Point";
    case CDMA_MSG_TYPE_BCAST : return "Broadcast";
    case CDMA_MSG_TYPE_ACK   : return "Acknowledge";
    default           : return "Unknown";
  }
}

const char *CDMA_ParameterIDToString(CDMA_PARAMETER_ID parameterID)
{
  switch(parameterID) {
    case CDMA_PARAM_TELESERVICE_ID   : return "Teleservice Identifier";
    case CDMA_PARAM_SERVICE_CATEGORY : return "Service Category";
    case CDMA_PARAM_ORIG_ADDRESS     : return "Originating Address";
    case CDMA_PARAM_ORIG_SUBADDRESS  : return "Originating Subaddress";
    case CDMA_PARAM_DEST_ADDRESS     : return "Destination Address";
    case CDMA_PARAM_DEST_SUBADDRESS  : return "Destination Subaddress";
    case CDMA_PARAM_BEARER_REPLY_OPT : return "Bearer Reply Option";
    case CDMA_PARAM_CAUSE_CODES      : return "Cause Codes";
    case CDMA_PARAM_BEARER_DATA      : return "Bearer Data";
    default           : return "Unknown";
  }
}

const char *TeleserviceIdToString(TELESERVICE_ID teleserviceId)
{
  switch(teleserviceId) {
    case TELESERVICE_ID_SMS       : return "CDMA Messaging";
    case TELESERVICE_ID_VOICEMAIL : return "CDMA Voice Mail Notification";
    case TELESERVICE_ID_PAGE      : return "CDMA Paging";
    default           : return "Unknown";
  }
}

const char *CDMA_SMSPriorityToString(SMS_PRIORITY priority)
{
  switch(priority) {
    case SMS_PRIORITY_NORMAL      : return "NORMAL";
    case SMS_PRIORITY_INTERACTIVE : return "INTERACTIVE";
    case SMS_PRIORITY_URGENT      : return "URGENT";
    case SMS_PRIORITY_EMERGENCY   : return "EMERGENCY";
    default           : return "Unknown";
  }
}

const char *CDMA_SMSEncodingToString(SMS_ENCODING encoding)
{
  switch(encoding) {
    case SMS_ENC_OCTET: return "8-bit Octet";
    case SMS_ENC_ASCII: return "7-bit ASCII";
    case SMS_ENC_UNICODE: return "16-bit Unicode";
    case SMS_ENC_GSM: return "GSM 7-bit";
    default           : return "Unknown";
  }
}

#ifndef GAMMU_MMS_TABLES_H
#define GAMMU_MMS_TABLES_H

#include "mms-data.h"

typedef MMSVALUEENUM MMSCHARSET;

enum WSPFieldID {
	WSP_ACCEPT = 0x00,
	WSP_ACCEPT_CHARSET = 0x01,
	WSP_ACCEPT_ENCODING = 0x02,
	WSP_ACCEPT_LANGUAGE = 0x03,
	WSP_ACCEPT_RANGES = 0x04,
	WSP_AGE = 0x05,
	WSP_ALLOW = 0x06,
	WSP_AUTHORIZATION = 0x07,
	WSP_CACHE_CONTROL = 0x08,
	WSP_CONNECTION = 0x09,
	WSP_CONTENT_BASE = 0x0A,
	WSP_CONTENT_ENCODING = 0x0B,
	WSP_CONTENT_LANGUAGE = 0x0C,
	WSP_CONTENT_LENGTH = 0x0D,
	WSP_CONTENT_LOCATION = 0x0E,
	WSP_CONTENT_MD5 = 0x0F,
	WSP_CONTENT_RANGE = 0x10,
	WSP_CONTENT_TYPE = 0x11,
	WSP_DATE = 0x12,
	WSP_ETAG = 0x13,
	WSP_EXPIRES = 0x14,
	WSP_FROM = 0x15,
	WSP_HOST = 0x16,
	WSP_IF_MODIFIED_SINCE = 0x17,
	WSP_IF_MATCH = 0x18,
	WSP_IF_NONE_MATCH = 0x19,
	WSP_IF_RANGE = 0x1A,
	WSP_IF_UNMODIFIED_SINCE = 0x1B,
	WSP_LOCATION = 0x1C,
	WSP_LAST_MODIFIED = 0x1D,
	WSP_MAX_FORWARDS = 0x1E,
	WSP_PRAGMA = 0x1F,
	WSP_PROXY_AUTHENTICATE = 0x20,
	WSP_PROXY_AUTHORIZATION = 0x21,
	WSP_PUBLIC = 0x22,
	WSP_RANGE = 0x23,
	WSP_REFERER = 0x24,
	WSP_RETRY_AFTER = 0x25,
	WSP_SERVER = 0x26,
	WSP_TRANSFER_ENCODING = 0x27,
	WSP_UPGRADE = 0x28,
	WSP_USER_AGENT = 0x29,
	WSP_VARY = 0x2A,
	WSP_VIA = 0x2B,
	WSP_WARNING = 0x2C,
	WSP_WWW_AUTHENTICATE = 0x2D,
	WSP_CONTENT_DISPOSITION = 0x2E,
	WSP_X_WAP_APPLICATION_ID = 0x2F,
	WSP_X_WAP_CONTENT_URI = 0x30,
	WSP_X_WAP_INITIATOR_URI = 0x31,
	WSP_ACCEPT_APPLICATION = 0x32,
	WSP_BEARER_INDICATION = 0x33,
	WSP_PUSH_FLAG = 0x34,
	WSP_PROFILE = 0x35,
	WSP_PROFILE_DIFF = 0x36,
	WSP_PROFILE_WARNING = 0x37,
};

enum MMSFieldID {
	MMS_BCC = 0x01,
	MMS_CC = 0x02,
	MMS_CONTENT_LOCATION = 0x03,
	MMS_CONTENT_TYPE = 0x04,
	MMS_DATE = 0x05,
	MMS_DELIVERY_REPORT = 0x06,
	MMS_DELIVERY_TIME = 0x07,
	MMS_EXPIRY = 0x08,
	MMS_FROM = 0x09,
	MMS_MESSAGE_CLASS = 0x0A,
	MMS_MESSAGE_ID = 0x0B,
	MMS_MESSAGE_TYPE = 0x0C,
	MMS_MMS_VERSION = 0x0D,
	MMS_MESSAGE_SIZE = 0x0E,
	MMS_PRIORITY = 0x0F,
	MMS_READ_REPLY = 0x10,
	MMS_REPORT_ALLOWED = 0x11,
	MMS_RESPONSE_STATUS = 0x12,
	MMS_RESPONSE_TEXT = 0x13,
	MMS_SENDER_VISIBILITY = 0x14,
	MMS_STATUS = 0x15,
	MMS_SUBJECT = 0x16,
	MMS_TO = 0x17,
	MMS_TRANSACTION_ID = 0x18
};

MMSVALUEENUM MMS_YESNO_YES;
MMSVALUEENUM MMS_YESNO_NO;

MMSFIELDINFO WSPFields_FindByID(int id);
MMSFIELDINFO MMSFields_FindByID(int id);
MMSFIELDINFO MMS_WkParams_FindByID(int id);
MMSVALUEENUM MMS_Charset_FindByID(MMSLongInt id);
MMSVALUEENUM MMS_WkContentType_FindByID(int id);
MMSVALUEENUM MMS_MessageType_FindByID(int id);
MMSVALUEENUM MMS_MessageClass_FindByID(int id);
MMSVALUEENUM MMS_Priority_FindByID(int id);
MMSVALUEENUM MMS_YesNo_FindByID(int id);
MMSVALUEENUM MMS_StatusValue_FindByID(int id);
MMSVALUEENUM MMS_ResponseStatus_FindByID(int id);

MMSFIELDINFO WSPFields_FindByName(const char *name);
MMSFIELDINFO MMSFields_FindByName(const char *name);
MMSFIELDINFO MMS_WkParam_FindByName(const char *name);
MMSVALUEENUM MMS_Charset_FindByName(const char *name);
MMSVALUEENUM MMS_WkContentType_FindByName(const char *name);
MMSVALUEENUM MMS_MessageType_FindByName(const char *name);
MMSVALUEENUM MMS_MessageClass_FindByName(const char *name);
MMSVALUEENUM MMS_Priority_FindByName(const char *name);
MMSVALUEENUM MMS_YesNo_FindByName(const char *name);
MMSVALUEENUM MMS_StatusValue_FindByName(const char *name);
MMSVALUEENUM MMS_ResponseStatus_FindByName(const char *name);


#endif //GAMMU_MMS_TABLES_H

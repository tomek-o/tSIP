#ifndef BARESIP_DIALOG_INFO_STATUS
#define BARESIP_DIALOG_INFO_STATUS

#include "baresip_dialog_info_direction.h"

enum dialog_info_status {
	DIALOG_INFO_UNKNOWN = 0,		///< not subscribed / subscription failed
	DIALOG_INFO_TERMINATED,
	DIALOG_INFO_EARLY,
	DIALOG_INFO_CONFIRMED
};

struct dialog_data {
	enum dialog_info_direction direction;
	char identity_display[64];
	char identity[64];
	enum dialog_info_status status;
};

enum { MAX_DIALOG_DATA_CNT = 6 };

#endif

#ifndef BARESIP_DIALOG_INFO_STATUS
#define BARESIP_DIALOG_INFO_STATUS

enum dialog_info_status {
	DIALOG_INFO_UNKNOWN = 0,		///< not subscribed / subscription failed
	DIALOG_INFO_TERMINATED,
	DIALOG_INFO_EARLY,
	DIALOG_INFO_CONFIRMED
};

#endif

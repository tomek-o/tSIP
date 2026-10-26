#ifndef BARESIP_ZRTP_H
#define BARESIP_ZRTP_H

#ifdef __cplusplus
extern "C" {
#endif

/** \param call_uid uid of the call (see call_set_uid()) whose ZRTP session is to be verified */
int baresip_zrtp_verify_sas(unsigned int call_uid, bool verify);


struct zrtp_st {
	bool active;
	char sas[32];
	char cipher[64];
	bool verified;
};

/** \param session_id ZRTP session id, -1 if session is destroyed
	\param call_uid uid of the call the session belongs to (0 if not known) */
typedef void (baresip_zrtp_state_h)(int session_id, unsigned int call_uid, struct zrtp_st *zrtp);

void baresip_zrtp_init(baresip_zrtp_state_h *state_h);

#ifdef __cplusplus
}
#endif

#endif

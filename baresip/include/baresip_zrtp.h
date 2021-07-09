#ifndef BARESIP_ZRTP_H
#define BARESIP_ZRTP_H

#ifdef __cplusplus
extern "C" {
#endif

int baresip_zrtp_verify_sas(int session_id, bool verify);


struct zrtp_st {
	bool active;
	char sas[32];
	char cipher[64];
	bool verified;
};

typedef void (baresip_zrtp_state_h)(int session_id, struct zrtp_st *zrtp);

void baresip_zrtp_init(baresip_zrtp_state_h *state_h);

#ifdef __cplusplus
}
#endif

#endif

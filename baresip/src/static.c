/* static.c - manually updated */
#include <re_types.h>
#include <re_mod.h>

extern const struct mod_export exports_g711;
extern const struct mod_export exports_winwave;
extern const struct mod_export exports_winwave2;
extern const struct mod_export exports_portaudio;
extern const struct mod_export exports_stun;
extern const struct mod_export exports_speex;
extern const struct mod_export exports_speex_aec;
extern const struct mod_export exports_speex_pp;
extern const struct mod_export exports_webrtc_aec;
extern const struct mod_export exports_gsm;
extern const struct mod_export exports_g722;
extern const struct mod_export exports_g726_32;
extern const struct mod_export exports_l16;
extern const struct mod_export exports_opus;
extern const struct mod_export exports_presence;
extern const struct mod_export exports_dialog_info;
extern const struct mod_export exports_recorder;
extern const struct mod_export exports_aufile;
extern const struct mod_export exports_aufile_mm;
extern const struct mod_export exports_softvol;
extern const struct mod_export exports_nullaudio;
extern const struct mod_export exports_gzrtp;


const struct mod_export *mod_table[] = {
	&exports_g711,
	&exports_winwave,
	&exports_winwave2,
	&exports_portaudio,
	&exports_stun,
	&exports_speex,
	&exports_speex_aec,
	&exports_speex_pp,
	&exports_webrtc_aec,
	&exports_gsm,
	&exports_g722,
	&exports_g726_32,
	&exports_l16,
	&exports_opus,
	&exports_presence,
	&exports_dialog_info,
	&exports_recorder,
	&exports_aufile,
	&exports_aufile_mm,
	&exports_softvol,
	&exports_nullaudio,
	&exports_gzrtp,
	NULL
};

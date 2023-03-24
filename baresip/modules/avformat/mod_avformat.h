/**
 * @file mod_avformat.h  libavformat media-source -- internal interface
 *
 * Copyright (C) 2010 - 2020 Alfred E. Heggestad
 */


struct shared {
	struct le le;
	struct ausrc_st *ausrc_st;    /* pointer */
	struct vidsrc_st *vidsrc_st;  /* pointer */
	mtx_t lock;
	AVFormatContext *ic;
	thrd_t thread;
	char *dev;
	bool is_realtime;
	bool run;
	/* bool is_pass_through; */

	struct stream {
		AVRational time_base;
		AVCodecContext *ctx;
		int idx;
	} au, vid;

	unsigned int no_audio_pts_cnt;
	uint64_t audio_multichannel_samples_decoded;	/* counting all channels set (e.g. L+R) as single sample */
};


int avformat_shared_alloc(struct shared **shp, const char *dev,
			  double fps, const struct vidsz *size,
			  bool video);
struct shared *avformat_shared_lookup(const char *dev);
void avformat_shared_set_audio(struct shared *sh, struct ausrc_st *st);
void avformat_shared_set_video(struct shared *sh, struct vidsrc_st *st);


int avformat_audio_alloc(struct ausrc_st **stp, struct ausrc *as,
			 struct media_ctx **ctx,
			 struct ausrc_prm *prm, const char *device,
			 ausrc_read_h *readh, ausrc_error_h *errh, void *arg);
void avformat_audio_decode(struct shared *st, AVPacket *pkt);


int  avformat_video_alloc(struct vidsrc_st **stp, struct vidsrc *vs,
		 struct media_ctx **ctx, struct vidsrc_prm *prm,
		 const struct vidsz *size,
		 const char *fmt, const char *dev,
		 vidsrc_frame_h *frameh,
		 vidsrc_error_h *errorh, void *arg);
void avformat_video_decode(struct shared *st, AVPacket *pkt);

/*add avformat_video_copy function which passes packets to packet handler*/
void avformat_video_copy(struct shared *st, AVPacket *pkt);

uint32_t avformat_audio_get_srate(struct shared *st);

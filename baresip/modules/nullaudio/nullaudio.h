/**
 * @file
 * "NULL" audio device
 */


struct dspbuf {
	struct mbuf *mb;
};


int nullaudio_src_alloc(struct ausrc_st **stp, struct ausrc *as,
			  struct media_ctx **ctx,
			  struct ausrc_prm *prm, const char *device,
			  ausrc_read_h *rh, ausrc_error_h *errh, void *arg);
int nullaudio_play_alloc(struct auplay_st **stp, struct auplay *ap,
		       struct auplay_prm *prm, const char *device,
		       auplay_write_h *wh, void *arg);

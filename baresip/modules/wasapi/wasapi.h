/**
 * @file wasapi.h Windows Audio Session API (WASAPI) sound driver -- internal api
 *
 * Shared mode only, polling (not event-driven: Windows 7 does not signal the
 * event for loopback capture). Audio engine is asked to convert to the
 * format baresip wants (AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM); if that is
 * rejected (older Windows versions) the device mix format is used and the
 * module converts sample format, channel count and sample rate itself.
 */
#ifndef WASAPI_H
#define WASAPI_H

#include "wasapi_com.h"


#define WASAPI_MAX_CH    8
#define WASAPI_REF_PER_MS 10000


/** Sample format of one side of a stream */
struct wasapi_fmt {
	bool is_float;
	unsigned bits;          /**< container bits per sample: 16, 24 or 32 */
	unsigned ch;
	uint32_t srate;
	unsigned block_align;   /**< bytes per frame */
};

/** \return false if the format is not supported by the converter */
bool wasapi_fmt_parse(const WAVEFORMATEX *wf, struct wasapi_fmt *f);
void wasapi_fmt_s16(struct wasapi_fmt *f, uint32_t srate, unsigned ch);
void wasapi_fmt_to_wfx(const struct wasapi_fmt *f, WAVEFORMATEX *wf);


/** Converter: sample format + channel count + sample rate */
struct wasapi_conv;

int wasapi_conv_alloc(struct wasapi_conv **cp, const struct wasapi_fmt *in,
		      const struct wasapi_fmt *out);
/** \return upper bound of output frames produced from in_frames */
size_t wasapi_conv_max_out(const struct wasapi_conv *c, size_t in_frames);
/** \brief Convert in_frames; out must have room for wasapi_conv_max_out() frames
    \param in NULL = in_frames of silence
    \return number of output frames written
*/
size_t wasapi_conv_process(struct wasapi_conv *c, const BYTE *in,
			   size_t in_frames, BYTE *out);


/** \brief Resolve device by name as listed by wasapi_enum_devices()
    \param capture true: audio source (capture devices + loopback entries),
		false: audio player (render devices)
    \param loopback set to true if the name selects loopback of a render device
    Unknown/empty name falls back to the default device.
*/
HRESULT wasapi_open_device(IMMDeviceEnumerator *enumerator, const char *name,
			   bool capture, IMMDevice **dev, bool *loopback);

/** \brief Activate and initialize an IAudioClient for the stream
    Tries the requested format with engine-side conversion first, then the
    device mix format.
    \param want format baresip wants on this side of the stream
    \param dev_fmt set to the format actually used on the device side
    \param converted set to true if dev_fmt != want and the caller has to convert
*/
HRESULT wasapi_client_init(IMMDevice *dev, DWORD flags, REFERENCE_TIME buffer,
			   const struct wasapi_fmt *want, IAudioClient **clientp,
			   struct wasapi_fmt *dev_fmt, bool *converted);

/** \return monotonic time in microseconds */
uint64_t wasapi_now_us(void);


int wasapi_src_alloc(struct ausrc_st **stp, struct ausrc *as,
		     struct media_ctx **ctx,
		     struct ausrc_prm *prm, const char *device,
		     ausrc_read_h *rh, ausrc_error_h *errh, void *arg);
int wasapi_play_alloc(struct auplay_st **stp, struct auplay *ap,
		      struct auplay_prm *prm, const char *device,
		      auplay_write_h *wh, void *arg);

#endif /* WASAPI_H */

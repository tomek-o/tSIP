/**
 * \file wasapi.cpp Windows Audio Session API (WASAPI) sound driver
 *
 * Input: capture devices (microphones) and loopback of render devices
 * ("what you hear"). Output: render devices.
 * Devices are selected by friendly name, as listed by wasapi_enum_devices().
 *
 * Partially based on baresip upstream wasapi module (Sebastian Reimers,
 * AGFEO GmbH & Co. KG), adapted to the old baresip API and to BDS2006
 * (no WASAPI headers in SDK).
 */
#include <string.h>
#include <math.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "wasapi.h"


#define DEBUG_MODULE "wasapi"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


const GUID WASAPI_CLSID_MMDeviceEnumerator = { 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };
const GUID WASAPI_IID_IMMDeviceEnumerator  = { 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };
const GUID WASAPI_IID_IAudioClient         = { 0x1CB9AD4C, 0xDBFA, 0x4C32, { 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2 } };
const GUID WASAPI_IID_IAudioCaptureClient  = { 0xC8ADBD64, 0xE71E, 0x48A0, { 0xA4, 0xDE, 0x18, 0x5C, 0x39, 0x5C, 0xD3, 0x17 } };
const GUID WASAPI_IID_IAudioRenderClient   = { 0xF294ACFC, 0x3146, 0x4483, { 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2 } };
const WasapiPropertyKey WASAPI_PKEY_Device_FriendlyName =
	{ { 0xA45C254E, 0xDF1C, 0x4EFD, { 0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0 } }, 14 };


static struct ausrc *ausrc;
static struct auplay *auplay;


/*
 * Device enumeration
 */

static void wide_to_ansi(const wchar_t *wide, char *buf, size_t size)
{
	buf[0] = '\0';
	if (wide == NULL || size == 0)
		return;
	if (WideCharToMultiByte(CP_ACP, 0, wide, -1, buf, (int)size, NULL, NULL) <= 0)
		buf[0] = '\0';
	buf[size - 1] = '\0';
}


static void get_friendly_name(IMMDevice *dev, char *buf, size_t size)
{
	IPropertyStore *store = NULL;

	buf[0] = '\0';

	if (SUCCEEDED(dev->OpenPropertyStore(STGM_READ, (void**)&store)) && store) {
		PROPVARIANT var;
		memset(&var, 0, sizeof(var));
		if (SUCCEEDED(store->GetValue(WASAPI_PKEY_Device_FriendlyName, &var))) {
			if (var.vt == VT_LPWSTR && var.pwszVal != NULL) {
				wide_to_ansi(var.pwszVal, buf, size);
				CoTaskMemFree(var.pwszVal);
			}
		}
		store->Release();
	}

	if (buf[0] == '\0') {
		LPWSTR id = NULL;
		if (SUCCEEDED(dev->GetId(&id)) && id) {
			wide_to_ansi(id, buf, size);
			CoTaskMemFree(id);
		}
	}
}


/** \return true to stop enumeration */
typedef bool (endpoint_h)(IMMDevice *dev, const char *name, void *arg);

static HRESULT for_each_endpoint(IMMDeviceEnumerator *enumerator, EDataFlow flow,
				 endpoint_h *h, void *arg)
{
	IMMDeviceCollection *coll = NULL;
	UINT count = 0;
	HRESULT hr;

	hr = enumerator->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &coll);
	if (FAILED(hr))
		return hr;

	if (FAILED(coll->GetCount(&count)))
		count = 0;

	for (UINT i = 0; i < count; i++) {
		IMMDevice *dev = NULL;
		char name[256];
		bool stop;

		if (FAILED(coll->Item(i, &dev)) || dev == NULL)
			continue;

		get_friendly_name(dev, name, sizeof(name));
		stop = h(dev, name, arg);
		dev->Release();
		if (stop)
			break;
	}

	coll->Release();

	return S_OK;
}


struct find_arg {
	const char *name;
	IMMDevice *found;
};

static bool find_by_name_handler(IMMDevice *dev, const char *name, void *arg)
{
	struct find_arg *fa = (struct find_arg *)arg;
	if (fa->name == NULL || strcmp(name, fa->name) == 0) {
		dev->AddRef();
		fa->found = dev;
		return true;
	}
	return false;
}


static HRESULT get_default(IMMDeviceEnumerator *enumerator, EDataFlow flow,
			   ERole role, IMMDevice **dev)
{
	struct find_arg fa;
	HRESULT hr;

	hr = enumerator->GetDefaultAudioEndpoint(flow, role, dev);
	if (SUCCEEDED(hr) && *dev)
		return hr;

	/* no role-default assigned (seen on Windows 7 and with some
	   devices on Windows 10) even though a device is present */
	DEBUG_NOTICE("GetDefaultAudioEndpoint failed (0x%08x), using first active device\n", hr);

	fa.name = NULL;
	fa.found = NULL;
	for_each_endpoint(enumerator, flow, find_by_name_handler, &fa);
	*dev = fa.found;

	return fa.found ? S_OK : E_FAIL;
}


HRESULT wasapi_open_device(IMMDeviceEnumerator *enumerator, const char *name,
			   bool capture, IMMDevice **dev, bool *loopback)
{
	static const size_t prefix_len = sizeof(WASAPI_DEV_LOOPBACK_PREFIX) - 1;
	EDataFlow flow = capture ? eCapture : eRender;
	ERole role = eCommunications;
	const char *n = name ? name : "";

	*dev = NULL;
	*loopback = false;

	if (capture && strncmp(n, WASAPI_DEV_LOOPBACK_PREFIX, prefix_len) == 0) {
		*loopback = true;
		flow = eRender;
		role = eConsole;	/* the device other applications play to */
		if (strcmp(n, WASAPI_DEV_LOOPBACK_DEFAULT) == 0)
			n = "";
		else
			n += prefix_len;
	}
	else if (strcmp(n, WASAPI_DEV_DEFAULT) == 0) {
		n = "";
	}

	if (n[0]) {
		struct find_arg fa;
		fa.name = n;
		fa.found = NULL;
		for_each_endpoint(enumerator, flow, find_by_name_handler, &fa);
		if (fa.found) {
			*dev = fa.found;
			return S_OK;
		}
		DEBUG_WARNING("device [%s] not found, using default\n", n);
	}

	return get_default(enumerator, flow, role, dev);
}


struct list_arg {
	wasapi_dev_h *h;
	void *arg;
	bool loopback;
};

static bool list_handler(IMMDevice *dev, const char *name, void *arg)
{
	struct list_arg *la = (struct list_arg *)arg;
	(void)dev;

	if (la->loopback) {
		char buf[300];
		re_snprintf(buf, sizeof(buf), "%s%s", WASAPI_DEV_LOOPBACK_PREFIX, name);
		la->h(buf, la->arg);
	}
	else {
		la->h(name, la->arg);
	}
	return false;
}


int wasapi_enum_devices(bool out, wasapi_dev_h *h, void *arg)
{
	IMMDeviceEnumerator *enumerator = NULL;
	struct list_arg la;
	HRESULT com_hr, hr;
	int err = 0;

	if (!h)
		return EINVAL;

	/* if COM is already initialized with a different apartment model
	   (e.g. STA on GUI thread) it is usable as it is */
	com_hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(com_hr) && com_hr != RPC_E_CHANGED_MODE)
		return ENODEV;

	hr = CoCreateInstance(WASAPI_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			      WASAPI_IID_IMMDeviceEnumerator, (void**)&enumerator);
	if (FAILED(hr) || enumerator == NULL) {
		/* e.g. Windows XP - no WASAPI */
		err = ENODEV;
		goto out;
	}

	la.h = h;
	la.arg = arg;

	h(WASAPI_DEV_DEFAULT, arg);
	la.loopback = false;
	for_each_endpoint(enumerator, out ? eRender : eCapture, list_handler, &la);

	if (!out) {
		h(WASAPI_DEV_LOOPBACK_DEFAULT, arg);
		la.loopback = true;
		for_each_endpoint(enumerator, eRender, list_handler, &la);
	}

 out:
	if (enumerator)
		enumerator->Release();
	if (SUCCEEDED(com_hr))
		CoUninitialize();

	return err;
}


/*
 * Stream setup helpers
 */

bool wasapi_fmt_parse(const WAVEFORMATEX *wf, struct wasapi_fmt *f)
{
	unsigned tag;

	if (!wf || !f)
		return false;

	tag = wf->wFormatTag;
	if (tag == WAVE_FORMAT_EXTENSIBLE && wf->cbSize >= 22) {
		const WasapiWaveFormatExtensible *ext = (const WasapiWaveFormatExtensible *)wf;
		tag = (unsigned)ext->SubFormat.Data1;
	}

	f->is_float = (tag == WAVE_FORMAT_IEEE_FLOAT);
	f->bits = wf->wBitsPerSample;
	f->ch = wf->nChannels;
	f->srate = wf->nSamplesPerSec;
	f->block_align = wf->nBlockAlign;

	if (!f->is_float && tag != WAVE_FORMAT_PCM)
		return false;
	if (f->is_float && f->bits != 32)
		return false;
	if (!f->is_float && f->bits != 16 && f->bits != 24 && f->bits != 32)
		return false;
	if (f->ch == 0 || f->ch > WASAPI_MAX_CH || f->srate == 0)
		return false;
	if (f->block_align != f->ch * f->bits / 8)
		return false;

	return true;
}


void wasapi_fmt_s16(struct wasapi_fmt *f, uint32_t srate, unsigned ch)
{
	f->is_float = false;
	f->bits = 16;
	f->ch = ch;
	f->srate = srate;
	f->block_align = 2 * ch;
}


void wasapi_fmt_to_wfx(const struct wasapi_fmt *f, WAVEFORMATEX *wf)
{
	memset(wf, 0, sizeof(*wf));
	wf->wFormatTag      = (WORD)(f->is_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
	wf->nChannels       = (WORD)f->ch;
	wf->nSamplesPerSec  = f->srate;
	wf->wBitsPerSample  = (WORD)f->bits;
	wf->nBlockAlign     = (WORD)f->block_align;
	wf->nAvgBytesPerSec = f->srate * f->block_align;
	wf->cbSize          = 0;
}


HRESULT wasapi_client_init(IMMDevice *dev, DWORD flags, REFERENCE_TIME buffer,
			   const struct wasapi_fmt *want, IAudioClient **clientp,
			   struct wasapi_fmt *dev_fmt, bool *converted)
{
	IAudioClient *client = NULL;
	WAVEFORMATEX *mix = NULL;
	WAVEFORMATEX wfx;
	HRESULT hr;

	*clientp = NULL;

	/* 1st attempt: let the audio engine convert to/from what baresip wants */
	hr = dev->Activate(WASAPI_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
	if (FAILED(hr))
		return hr;

	wasapi_fmt_to_wfx(want, &wfx);
	hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED,
				flags | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
				AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
				buffer, 0, &wfx, NULL);
	if (SUCCEEDED(hr)) {
		*dev_fmt = *want;
		*converted = false;
		*clientp = client;
		return hr;
	}

	DEBUG_NOTICE("engine-side format conversion not available (0x%08x),"
		     " using device mix format\n", hr);

	/* 2nd attempt: device mix format, converted by this module.
	   Fresh client - not every Windows version accepts Initialize()
	   again after a failed one. */
	client->Release();
	client = NULL;

	hr = dev->Activate(WASAPI_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
	if (FAILED(hr))
		return hr;

	hr = client->GetMixFormat(&mix);
	if (FAILED(hr))
		goto out;

	if (!wasapi_fmt_parse(mix, dev_fmt)) {
		DEBUG_WARNING("unsupported mix format: tag 0x%04x, %u bits\n",
			      mix->wFormatTag, mix->wBitsPerSample);
		hr = E_FAIL;
		goto out;
	}

	hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, buffer, 0, mix, NULL);
	if (FAILED(hr))
		goto out;

	*converted = true;

 out:
	if (mix)
		CoTaskMemFree(mix);

	if (SUCCEEDED(hr))
		*clientp = client;
	else
		client->Release();

	return hr;
}


uint64_t wasapi_now_us(void)
{
	static double freq = 0;
	LARGE_INTEGER now;

	if (freq == 0) {
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);
		freq = (double)f.QuadPart;
	}
	QueryPerformanceCounter(&now);

	return (uint64_t)((double)now.QuadPart * 1000000.0 / freq);
}


/*
 * Converter: sample format, channel count, sample rate (linear interpolation,
 * with 4th order low-pass before downsampling). Only used as a fallback if the
 * audio engine refuses to convert (AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM).
 */

struct biquad {
	float b0, b1, b2, a1, a2;
	float x1[WASAPI_MAX_CH], x2[WASAPI_MAX_CH];
	float y1[WASAPI_MAX_CH], y2[WASAPI_MAX_CH];
};

struct wasapi_conv {
	struct wasapi_fmt in, out;
	bool resample;
	bool lowpass;
	double step;            /**< input frames per output frame */
	double pos;             /**< position of next output frame, relative to current input block */
	float prev[WASAPI_MAX_CH]; /**< last input frame of previous block */
	struct biquad lp[2];
	float *buf1;
	float *buf2;
	size_t buf1_len, buf2_len;   /**< in floats */
};


static void conv_destructor(void *arg)
{
	struct wasapi_conv *c = (struct wasapi_conv *)arg;
	mem_deref(c->buf1);
	mem_deref(c->buf2);
}


static void biquad_lowpass_init(struct biquad *bq, double fc, double fs, double q)
{
	const double w0 = 2 * 3.14159265358979 * fc / fs;
	const double alpha = sin(w0) / (2 * q);
	const double cw = cos(w0);
	const double a0 = 1 + alpha;

	memset(bq, 0, sizeof(*bq));
	bq->b0 = (float)(((1 - cw) / 2) / a0);
	bq->b1 = (float)((1 - cw) / a0);
	bq->b2 = bq->b0;
	bq->a1 = (float)((-2 * cw) / a0);
	bq->a2 = (float)((1 - alpha) / a0);
}


static void biquad_process(struct biquad *bq, float *v, size_t frames, unsigned ch)
{
	for (size_t i = 0; i < frames; i++) {
		for (unsigned c = 0; c < ch; c++) {
			float x = v[i * ch + c];
			float y = bq->b0 * x + bq->b1 * bq->x1[c] + bq->b2 * bq->x2[c]
				- bq->a1 * bq->y1[c] - bq->a2 * bq->y2[c];
			bq->x2[c] = bq->x1[c];
			bq->x1[c] = x;
			bq->y2[c] = bq->y1[c];
			bq->y1[c] = y;
			v[i * ch + c] = y;
		}
	}
}


int wasapi_conv_alloc(struct wasapi_conv **cp, const struct wasapi_fmt *in,
		      const struct wasapi_fmt *out)
{
	struct wasapi_conv *c;

	if (!cp || !in || !out)
		return EINVAL;

	c = (struct wasapi_conv *)mem_zalloc(sizeof(*c), conv_destructor);
	if (!c)
		return ENOMEM;

	c->in = *in;
	c->out = *out;
	c->resample = (in->srate != out->srate);
	c->step = (double)in->srate / (double)out->srate;
	c->pos = 0;
	c->lowpass = (out->srate < in->srate);

	if (c->lowpass) {
		/* 4th order Butterworth as two biquads, cutoff a bit below
		   the output Nyquist frequency */
		const double fc = 0.45 * out->srate;
		biquad_lowpass_init(&c->lp[0], fc, in->srate, 0.5412);
		biquad_lowpass_init(&c->lp[1], fc, in->srate, 1.3066);
	}

	*cp = c;

	return 0;
}


size_t wasapi_conv_max_out(const struct wasapi_conv *c, size_t in_frames)
{
	if (!c->resample)
		return in_frames;
	return (size_t)((double)(in_frames + 1) / c->step) + 2;
}


static int ensure_buf(float **buf, size_t *len, size_t need)
{
	float *p;

	if (*len >= need)
		return 0;

	/* note: mem_realloc() does not accept NULL */
	if (*buf)
		p = (float *)mem_realloc(*buf, need * sizeof(float));
	else
		p = (float *)mem_alloc(need * sizeof(float), NULL);
	if (!p)
		return ENOMEM;

	*buf = p;
	*len = need;

	return 0;
}


static void to_float(const BYTE *in, const struct wasapi_fmt *f, size_t frames, float *out)
{
	const size_t n = frames * f->ch;
	size_t i;

	if (f->is_float) {
		memcpy(out, in, n * sizeof(float));
	}
	else if (f->bits == 16) {
		const int16_t *s = (const int16_t *)in;
		for (i = 0; i < n; i++)
			out[i] = (float)s[i] / 32768.0f;
	}
	else if (f->bits == 24) {
		for (i = 0; i < n; i++) {
			const BYTE *b = in + 3 * i;
			int32_t v = (int32_t)(((uint32_t)b[0] << 8) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 24));
			out[i] = (float)((double)v / 2147483648.0);
		}
	}
	else {
		const int32_t *s = (const int32_t *)in;
		for (i = 0; i < n; i++)
			out[i] = (float)((double)s[i] / 2147483648.0);
	}
}


static double clamp1(double x)
{
	if (x > 1.0)
		return 1.0;
	if (x < -1.0)
		return -1.0;
	return x;
}


static void from_float(const float *in, const struct wasapi_fmt *f, size_t frames, BYTE *out)
{
	const size_t n = frames * f->ch;
	size_t i;

	if (f->is_float) {
		memcpy(out, in, n * sizeof(float));
	}
	else if (f->bits == 16) {
		int16_t *s = (int16_t *)out;
		for (i = 0; i < n; i++)
			s[i] = (int16_t)floor(clamp1(in[i]) * 32767.0 + 0.5);
	}
	else if (f->bits == 24) {
		for (i = 0; i < n; i++) {
			int32_t v = (int32_t)floor(clamp1(in[i]) * 2147483647.0 + 0.5);
			BYTE *b = out + 3 * i;
			b[0] = (BYTE)(v >> 8);
			b[1] = (BYTE)(v >> 16);
			b[2] = (BYTE)(v >> 24);
		}
	}
	else {
		int32_t *s = (int32_t *)out;
		for (i = 0; i < n; i++)
			s[i] = (int32_t)floor(clamp1(in[i]) * 2147483647.0 + 0.5);
	}
}


static void map_channels(const float *in, unsigned ich, float *out, unsigned och, size_t frames)
{
	for (size_t i = 0; i < frames; i++) {
		const float *src = in + i * ich;
		float *dst = out + i * och;

		if (och == ich) {
			memcpy(dst, src, och * sizeof(float));
		}
		else if (och == 1) {
			/* downmix: front left + front right */
			dst[0] = (src[0] + src[1]) * 0.5f;
		}
		else {
			for (unsigned c = 0; c < och; c++) {
				if (c < ich)
					dst[c] = src[c];
				else if (ich == 1 && c == 1)
					dst[c] = src[0];	/* mono to both front channels */
				else
					dst[c] = 0.0f;
			}
		}
	}
}


size_t wasapi_conv_process(struct wasapi_conv *c, const BYTE *in,
			   size_t in_frames, BYTE *out)
{
	const unsigned ich = c->in.ch;
	const unsigned och = c->out.ch;
	const size_t max_out = wasapi_conv_max_out(c, in_frames);
	const size_t buf1_need = (in_frames * ich > max_out * och) ? in_frames * ich : max_out * och;
	size_t out_frames;
	const float *res;

	if (in_frames == 0)
		return 0;

	if (ensure_buf(&c->buf1, &c->buf1_len, buf1_need) ||
	    ensure_buf(&c->buf2, &c->buf2_len, in_frames * och))
		return 0;

	if (in)
		to_float(in, &c->in, in_frames, c->buf1);
	else
		memset(c->buf1, 0, in_frames * ich * sizeof(float));

	map_channels(c->buf1, ich, c->buf2, och, in_frames);

	if (c->lowpass) {
		biquad_process(&c->lp[0], c->buf2, in_frames, och);
		biquad_process(&c->lp[1], c->buf2, in_frames, och);
	}

	if (c->resample) {
		const float *x = c->buf2;
		const long n = (long)in_frames;
		float *y = c->buf1;

		out_frames = 0;
		while (out_frames < max_out) {
			const long i0 = (long)floor(c->pos);	/* -1 .. n-2 */
			const float frac = (float)(c->pos - i0);
			if (i0 + 1 >= n)
				break;
			for (unsigned ch = 0; ch < och; ch++) {
				const float a = (i0 < 0) ? c->prev[ch] : x[i0 * och + ch];
				const float b = x[(i0 + 1) * och + ch];
				y[out_frames * och + ch] = a + (b - a) * frac;
			}
			out_frames++;
			c->pos += c->step;
		}

		c->pos -= n;
		memcpy(c->prev, &x[(n - 1) * och], och * sizeof(float));
		res = y;
	}
	else {
		out_frames = in_frames;
		res = c->buf2;
	}

	from_float(res, &c->out, out_frames, out);

	return out_frames;
}


/*
 * Module
 */

static void count_handler(const char *name, void *arg)
{
	(void)name;
	(*(unsigned *)arg)++;
}


static int wasapi_init(void)
{
	unsigned play_cnt = 0, src_cnt = 0;
	int err;

	if (wasapi_enum_devices(true, count_handler, &play_cnt) == 0 &&
	    wasapi_enum_devices(false, count_handler, &src_cnt) == 0) {
		/* counts include "default" entries and loopback entries */
		re_printf("wasapi: output entries: %u, input entries: %u\n", play_cnt, src_cnt);
	}
	else {
		re_printf("wasapi: WASAPI not available\n");
	}

	err  = ausrc_register(&ausrc, "wasapi", wasapi_src_alloc);
	err |= auplay_register(&auplay, "wasapi", wasapi_play_alloc);

	return err;
}


static int wasapi_close(void)
{
	ausrc  = (struct ausrc *)mem_deref(ausrc);
	auplay = (struct auplay *)mem_deref(auplay);

	return 0;
}


extern "C" const struct mod_export DECL_EXPORTS(wasapi) = {
	"wasapi",
	"sound",
	wasapi_init,
	wasapi_close
};

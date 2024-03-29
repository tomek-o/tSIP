/**
 * @file dshow.cpp Windows DirectShow video-source
 *
 * Copyright (C) 2010 Creytiv.com
 * Copyright (C) 2010 Dusan Stevanovic
 */

#include <stdio.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#ifdef USE_VIDEO

#include <comutil.h>
#include <commctrl.h>
#include <dshow.h>
#include <qedit.h>

#define __TCHAR_H	/**< remove tchar.h content from <vector> header to avoid redefinition warning */
#include <vector>


#define DEBUG_MODULE "dshow"
#define DEBUG_LEVEL 6
#include <re_dbg.h>


const CLSID CLSID_SampleGrabber = { 0xc1f400a0, 0x3f08, 0x11d3,
  { 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37 }
};


class Grabber;

struct vidsrc_st {
	struct vidsrc *vs;  /* inheritance */

	ICaptureGraphBuilder2 *capture;
	IBaseFilter *grabber_filter;
	IBaseFilter *dev_filter;
	ISampleGrabber *grabber;
	IMoniker *dev_moniker;
	IGraphBuilder *graph;
	IMediaControl *mc;

	Grabber *grab;

	struct vidsz size;
	vidsrc_frame_h *frameh;
	void *arg;
};


class Grabber : public ISampleGrabberCB {
public:
	Grabber(struct vidsrc_st *st) : src(st) { }

	STDMETHOD(QueryInterface)(REFIID InterfaceIdentifier,
				  VOID** ppvObject) throw()
	{
		if (InterfaceIdentifier == __uuidof(ISampleGrabberCB)) {
			*ppvObject = (ISampleGrabberCB**) this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)() throw()
	{
		return 2;
	}

	STDMETHOD_(ULONG, Release)() throw()
	{
		return 1;
	}

	STDMETHOD(BufferCB) (double sample_time, BYTE *buf, long buf_len)
	{
		int i, j = 0, k;
		int buf_len_RGB32 = src->size.h*src->size.w;
		uint32_t tmp_pix_RGB32;
		uint32_t *buf_RGB32;
		struct vidframe vidframe;
		//uint64_t timestamp = (uint64_t)(sample_time * VIDEO_TIMEBASE);
		if (buf_len != buf_len_RGB32 * 4) {
			DEBUG_WARNING("dshow: BufferCB got %uB, required %uB (%ux%u)\n", buf_len, buf_len_RGB32 * 4, src->size.w, src->size.h);
			return S_OK;
		}

		vidframe_init_buf(&vidframe, VID_FMT_RGB32, &src->size, buf);

		//By default in Dshow, RGB32 image orientation is bottom-up
		buf_RGB32 = (uint32_t *)buf;
		for (i = buf_len_RGB32-1 ; i > buf_len_RGB32/2; i-=1) {
			k = src->size.w*(j/src->size.w) + i%(src->size.w);
			tmp_pix_RGB32 = buf_RGB32[k];
			buf_RGB32[k] = buf_RGB32[i];
			buf_RGB32[i] = tmp_pix_RGB32;
			++j;
		}

		if (src->frameh)
			src->frameh(&vidframe, src->arg);

		return S_OK;
	}

	STDMETHOD(SampleCB) (double sample_time, IMediaSample *samp)
	{
		return S_OK;
	}

private:
	struct vidsrc_st *src;
};


static struct vidsrc *vsrc;


static int get_device(struct vidsrc_st *st, const char *name)
{
	ICreateDevEnum *dev_enum;
	IEnumMoniker *enum_mon;
	IMoniker *mon;
	ULONG fetched;
	HRESULT res;
	int id = 0;
	bool found = false;

	if (!st)
		return EINVAL;

	res = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
			       CLSCTX_INPROC_SERVER,
			       IID_ICreateDevEnum, (void**)&dev_enum);
	if (res != NOERROR)
		return ENOENT;

	res = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
					      &enum_mon, 0);
	if (res != NOERROR)
		return ENOENT;

	enum_mon->Reset();
	while (enum_mon->Next(1, &mon, &fetched) == S_OK && !found) {

		IPropertyBag *bag;
		VARIANT var;
		char dev_name[256];
		int len = 0;

		res = mon->BindToStorage(0, 0, IID_IPropertyBag,
					 (void **)&bag);
		if (!SUCCEEDED(res))
			continue;

		::VariantInit(&var);			
		var.vt = VT_BSTR;

		LPVOID fns = CoTaskMemAlloc(26);
		memcpy(fns, L"FriendlyName", 26);
		res = bag->Read((wchar_t*)fns, &var, NULL);
		CoTaskMemFree(fns);
		if (NOERROR != res)
			continue;

		len = WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
					  dev_name, sizeof(dev_name),
					  NULL, NULL);

		SysFreeString(var.bstrVal);
		bag->Release();

		if (len > 0) {
			found = !str_isset(name) ||
				!str_casecmp(dev_name, name);

			if (found) {
				if (!str_isset(name)) {
					re_printf("dshow: empty device name, got device '%s' id=%d\n", dev_name, id);
				} else {
					re_printf("dshow: got device '%s' id=%d\n", name, id);
				}
				st->dev_moniker = mon;
			}
		}

		if (!found) {
			mon->Release();
			++id;
		}
	}

	return found ? 0 : ENOENT;
}


static int add_sample_grabber(struct vidsrc_st *st)
{
	AM_MEDIA_TYPE mt;
	HRESULT hr;

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
			      IID_IBaseFilter, (void**) &st->grabber_filter);
	if (FAILED(hr))
		return ENOMEM;

	hr = st->graph->AddFilter(st->grabber_filter, L"Sample Grabber");
	if (FAILED(hr))
		return ENOMEM;

	hr = st->grabber_filter->QueryInterface(IID_ISampleGrabber,
						(void**)&st->grabber);
	if (FAILED(hr))
		return ENODEV;

	hr = st->grabber->SetCallback(st->grab, 1);
	if (FAILED(hr))
		return ENOSYS;

	memset(&mt, 0, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB32;
	hr = st->grabber->SetMediaType(&mt);
	if (FAILED(hr))
		return ENODEV;

	st->grabber->SetOneShot(FALSE);
	st->grabber->SetBufferSamples(FALSE);

	return 0;
}


static AM_MEDIA_TYPE *free_mt(AM_MEDIA_TYPE *mt)
{
	if (!mt)
		return NULL;

	if (mt->cbFormat) {
		CoTaskMemFree((PVOID)mt->pbFormat);
	}
	if (mt->pUnk != NULL) {
		mt->pUnk->Release();
		mt->pUnk = NULL;
	}

	CoTaskMemFree((PVOID)mt);

	return NULL;
}


static int config_pin(struct vidsrc_st *st, IPin *pin)
{
	AM_MEDIA_TYPE *mt = NULL;	/* exact match, if found */
	AM_MEDIA_TYPE *best_mt = NULL;	/* next best match, if found */
	IEnumMediaTypes *media_enum = NULL;
	IAMStreamConfig *stream_conf = NULL;
	VIDEOINFOHEADER *vih;
	HRESULT hr;
	int h = st->size.h;
	int w = st->size.w;
	int rh, rw;
	int wh, rwrh;
	int best_match = 0;
	int err = 0;
	struct config * cfg = conf_config();

	std::vector<BITMAPINFOHEADER> bmiHeaders;

	if (!pin || !st)
		return EINVAL;

	hr = pin->EnumMediaTypes(&media_enum);
	if (FAILED(hr))
		return ENODATA;

	while ((hr = media_enum->Next(1, &mt, NULL)) == S_OK) {
		if (mt->formattype != FORMAT_VideoInfo) {
			mt = free_mt(mt);
			continue;
		}

		vih = (VIDEOINFOHEADER *) mt->pbFormat;
		rw = vih->bmiHeader.biWidth;
		rh = vih->bmiHeader.biHeight;
		bmiHeaders.push_back(vih->bmiHeader);

		wh = w * h;
		rwrh = rw * rh;
		if (wh == rwrh) {
			best_mt = free_mt(best_mt);
			break;
		}
		else {
			int diff = abs(rwrh - wh);

			if (best_match != 0 && diff >= best_match)
				mt = free_mt(mt);
			else {
				best_match = diff;
				free_mt(best_mt);
				best_mt = mt;
				mt = NULL;
			}
		}
	}
	if (hr != S_OK)
		mt = free_mt(mt);

	if (mt == NULL && best_mt == NULL) {
		err = ENODATA;
		goto out;
	}
	if (mt == NULL)
		mt = best_mt;

	hr = pin->QueryInterface(IID_IAMStreamConfig,
				 (void **) &stream_conf);
	if (FAILED(hr)) {
		err = EINVAL;
		goto out;
	}

	vih = (VIDEOINFOHEADER *) mt->pbFormat;
	hr = stream_conf->SetFormat(mt);
	if (FAILED(hr)) {
		err = ERANGE;
		goto out;
	}

	if (cfg->video.dshow.skip_reading_back_media_format == false) {
		/*
			WTF? OBS camera returns back 1920x1080 below (GetFormat) after 640x360 was set
			but later passes only 640x360x4 bytes in data callback.
			Should GetFormat result be ignored?
		*/
		mt = free_mt(mt);
		hr = stream_conf->GetFormat(&mt);
		if (FAILED(hr)) {
			err = EINVAL;
			goto out;
		}
		if (mt->formattype != FORMAT_VideoInfo) {
			err = EINVAL;
			goto out;
		}
	}

	vih = (VIDEOINFOHEADER *)mt->pbFormat;
	rw = vih->bmiHeader.biWidth;
	rh = vih->bmiHeader.biHeight;

	if (w != rw || h != rh) {
		DEBUG_WARNING("dshow alloc: video source size mismatch, wanted %d x %d, got (best match) %d x %d\n", w, h, rw, rh);
		if (!bmiHeaders.empty()) {
			DEBUG_WARNING("dshow available video sizes: ");
			for (unsigned int i=0; i<bmiHeaders.size(); i++) {
				const BITMAPINFOHEADER &bh = bmiHeaders[i];
				DEBUG_WARNING("%s%d x %d", i!=0?", ":"", bh.biWidth, bh.biHeight);
			}
			DEBUG_WARNING("\n");
		}
	}
	st->size.w = rw;
	st->size.h = rh;

 out:
	if (media_enum)
		media_enum->Release();
	if (stream_conf)
		stream_conf->Release();
	free_mt(mt);

	return err;
}


static void destructor(void *arg)
{
	struct vidsrc_st *st = (struct vidsrc_st *)arg;

	if (st->mc) {
		st->mc->Stop();
		st->mc->Release();
	}

	if (st->grabber) {
		st->grabber->SetCallback(NULL, 1);
		st->grabber->Release();
	}
	if (st->grabber_filter)
		st->grabber_filter->Release();
	if (st->dev_moniker)
		st->dev_moniker->Release();
	if (st->dev_filter)
		st->dev_filter->Release();
	if (st->capture) {
		st->capture->RenderStream(&PIN_CATEGORY_CAPTURE,
					  &MEDIATYPE_Video,
					  NULL, NULL, NULL);
		st->capture->Release();
	}
	if (st->graph)
		st->graph->Release();

	delete st->grab;

	mem_deref(st->vs);
}


static int alloc(struct vidsrc_st **stp, struct vidsrc *vs,
		 struct media_ctx **ctx, struct vidsrc_prm *prm,
		 const struct vidsz *size,
		 const char *fmt, const char *dev,
		 vidsrc_frame_h *frameh,
		 vidsrc_error_h *errorh, void *arg)
{
	struct vidsrc_st *st;
	IEnumPins *pin_enum = NULL;
	IPin *pin = NULL;
	HRESULT hr;
	int err;
	(void)ctx;
	(void)errorh;

	if (!stp || !vs || !prm || !size)
		return EINVAL;

	st = (struct vidsrc_st *) mem_zalloc(sizeof(*st), destructor);
	if (!st)
		return ENOMEM;

	err = get_device(st, dev);
	if (err)
		goto out;

	st->vs = (struct vidsrc *)mem_ref(vs);

	st->size   = *size;
	st->frameh = frameh;
	st->arg    = arg;

	st->grab = new Grabber(st);
	if (!st->grab) {
		err = ENOMEM;
		goto out;
	}

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                              IID_IGraphBuilder, (void **) &st->graph);
	if (FAILED(hr)) {
		DEBUG_WARNING("alloc: IID_IGraphBuilder failed: %ld\n", hr);
		err = ENODEV;
		goto out;
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2 , NULL,
			      CLSCTX_INPROC, IID_ICaptureGraphBuilder2,
			      (void **) &st->capture);
	if (FAILED(hr)) {
		DEBUG_WARNING("alloc: IID_ICaptureGraphBuilder2: %ld\n", hr);
		err = ENODEV;
		goto out;
	}

	hr = st->capture->SetFiltergraph(st->graph);
	if (FAILED(hr)) {
		DEBUG_WARNING("alloc: SetFiltergraph failed: %ld\n", hr);
		err = ENODEV;
		goto out;
	}

	hr = st->dev_moniker->BindToObject(NULL, NULL, IID_IBaseFilter,
				    (void **) &st->dev_filter);
	if (FAILED(hr)) {
		DEBUG_WARNING("alloc: bind to base filter failed: %ld\n", hr);
		err = ENODEV;
		goto out;
	}

	hr = st->graph->AddFilter(st->dev_filter, L"Video Capture");
	if (FAILED(hr)) {
		DEBUG_WARNING("alloc: VideoCapture failed: %ld\n", hr);
		err = ENODEV;
		goto out;
	}

	hr = st->dev_filter->EnumPins(&pin_enum);
	if (pin_enum) {
		pin_enum->Reset();
		hr = pin_enum->Next(1, &pin, NULL);
	}

	add_sample_grabber(st);
	err = config_pin(st, pin);
	pin->Release();
	if (err)
		goto out;

	hr = st->capture->RenderStream(&PIN_CATEGORY_CAPTURE,
				       &MEDIATYPE_Video,
				       st->dev_filter,
				       NULL, st->grabber_filter);
	if (FAILED(hr)) {
		DEBUG_WARNING("dshow: alloc: RenderStream failed\n");
		err = ENODEV;
		goto out;
	}

	hr = st->graph->QueryInterface(IID_IMediaControl,
				       (void **) &st->mc);
	if (FAILED(hr)) {
		DEBUG_WARNING("dshow: alloc: IMediaControl failed\n");
		err = ENODEV;
		goto out;
	}

	hr = st->mc->Run();
	if (FAILED(hr)) {
		DEBUG_WARNING("dshow: alloc: IMediaControl Run failed, result = %d (0x%08X)\n", hr, hr);
		if (hr == 0x800705AA /* = directshow ERROR_NO_SYSTEM_RESOURCES */) {
			DEBUG_WARNING("dshow: alloc: ERROR_NO_SYSTEM_RESOURCES failure (two applications trying to use camera same time?)\n");
		}
		err = ENODEV;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static int module_init(void)
{
#if 0
	HRESULT res = CoInitialize(NULL);
	if (res != S_OK)
		return ENODATA;
#endif
	return vidsrc_register(&vsrc, "dshow", alloc, NULL);
}


static int module_close(void)
{
	vsrc = (struct vidsrc *) mem_deref(vsrc);
#if 0
	CoUninitialize();
#endif

	return 0;
}


extern "C" const struct mod_export DECL_EXPORTS(dshow) = {
	"dshow",
	"vidsrc",
	module_init,
	module_close
};

#endif	/* USE_VIDEO */

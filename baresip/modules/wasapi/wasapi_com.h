/**
 * @file wasapi_com.h Minimal WASAPI COM declarations
 *
 * BDS2006's SDK predates Vista, so mmdeviceapi.h/audioclient.h don't exist
 * there. Interfaces are hand-declared in their real vtable order - these are
 * calls into the OS's mmdevapi.dll/audioses.dll, not something implemented
 * locally. Only ole32 (CoCreateInstance) is needed at link time, so on
 * Windows XP the module simply finds no devices instead of failing to load.
 *
 * C++ only (DECLARE_INTERFACE_ C++ flavor, references).
 */
#ifndef WASAPI_COM_H
#define WASAPI_COM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>
#include <mmsystem.h>	// WAVEFORMATEX


/*
 * mmdeviceapi.h subset
 */

/* plain ints instead of enums: enum size depends on compiler settings and
   must match the 4-byte COM ABI */
typedef int EDataFlow;
#define eRender   ((EDataFlow)0)
#define eCapture  ((EDataFlow)1)

typedef int ERole;
#define eConsole        ((ERole)0)
#define eMultimedia     ((ERole)1)
#define eCommunications ((ERole)2)

#define DEVICE_STATE_ACTIVE 0x1

struct IMMDevice;
struct IMMDeviceCollection;

DECLARE_INTERFACE_(IMMDeviceEnumerator, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(EnumAudioEndpoints)(THIS_ EDataFlow dataFlow, DWORD dwStateMask, IMMDeviceCollection **ppDevices) PURE;
	STDMETHOD(GetDefaultAudioEndpoint)(THIS_ EDataFlow dataFlow, ERole role, IMMDevice **ppEndpoint) PURE;
	STDMETHOD(GetDevice)(THIS_ LPCWSTR pwstrId, IMMDevice **ppDevice) PURE;
	STDMETHOD(RegisterEndpointNotificationCallback)(THIS_ void *pClient) PURE;
	STDMETHOD(UnregisterEndpointNotificationCallback)(THIS_ void *pClient) PURE;
};

DECLARE_INTERFACE_(IMMDevice, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	/* pActivationParams is really a PROPVARIANT*; always NULL here */
	STDMETHOD(Activate)(THIS_ REFIID iid, DWORD dwClsCtx, void *pActivationParams, void **ppInterface) PURE;
	STDMETHOD(OpenPropertyStore)(THIS_ DWORD stgmAccess, void **ppProperties) PURE;
	STDMETHOD(GetId)(THIS_ LPWSTR *ppstrId) PURE;
	STDMETHOD(GetState)(THIS_ DWORD *pdwState) PURE;
};

DECLARE_INTERFACE_(IMMDeviceCollection, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(GetCount)(THIS_ UINT *pcDevices) PURE;
	STDMETHOD(Item)(THIS_ UINT nDevice, IMMDevice **ppDevice) PURE;
};

/* Stand-in for PROPERTYKEY (fmtid + pid), same 20-byte layout */
struct WasapiPropertyKey
{
	GUID fmtid;
	unsigned long pid;
};

DECLARE_INTERFACE_(IPropertyStore, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(GetCount)(THIS_ DWORD *cProps) PURE;
	STDMETHOD(GetAt)(THIS_ DWORD iProp, WasapiPropertyKey *pkey) PURE;
	STDMETHOD(GetValue)(THIS_ const WasapiPropertyKey &key, PROPVARIANT *pv) PURE;
	STDMETHOD(SetValue)(THIS_ const WasapiPropertyKey &key, const void *propvar) PURE;
	STDMETHOD(Commit)(THIS) PURE;
};


/*
 * audioclient.h subset
 */

typedef __int64 REFERENCE_TIME;

typedef int AUDCLNT_SHAREMODE;
#define AUDCLNT_SHAREMODE_SHARED    ((AUDCLNT_SHAREMODE)0)

#define AUDCLNT_STREAMFLAGS_LOOPBACK            0x00020000
#define AUDCLNT_STREAMFLAGS_EVENTCALLBACK       0x00040000
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM      0x80000000

#define AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY  0x1
#define AUDCLNT_BUFFERFLAGS_SILENT              0x2

DECLARE_INTERFACE_(IAudioClient, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(Initialize)(THIS_ AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags,
		REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity,
		const WAVEFORMATEX *pFormat, LPCGUID AudioSessionGuid) PURE;
	STDMETHOD(GetBufferSize)(THIS_ UINT32 *pNumBufferFrames) PURE;
	STDMETHOD(GetStreamLatency)(THIS_ REFERENCE_TIME *phnsLatency) PURE;
	STDMETHOD(GetCurrentPadding)(THIS_ UINT32 *pNumPaddingFrames) PURE;
	STDMETHOD(IsFormatSupported)(THIS_ AUDCLNT_SHAREMODE ShareMode,
		const WAVEFORMATEX *pFormat, WAVEFORMATEX **ppClosestMatch) PURE;
	STDMETHOD(GetMixFormat)(THIS_ WAVEFORMATEX **ppDeviceFormat) PURE;
	STDMETHOD(GetDevicePeriod)(THIS_ REFERENCE_TIME *phnsDefaultDevicePeriod,
		REFERENCE_TIME *phnsMinimumDevicePeriod) PURE;
	STDMETHOD(Start)(THIS) PURE;
	STDMETHOD(Stop)(THIS) PURE;
	STDMETHOD(Reset)(THIS) PURE;
	STDMETHOD(SetEventHandle)(THIS_ HANDLE eventHandle) PURE;
	STDMETHOD(GetService)(THIS_ REFIID riid, void **ppv) PURE;
};

DECLARE_INTERFACE_(IAudioCaptureClient, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(GetBuffer)(THIS_ BYTE **ppData, UINT32 *pNumFramesToRead,
		DWORD *pdwFlags, UINT64 *pu64Position, UINT64 *pu64QPCPosition) PURE;
	STDMETHOD(ReleaseBuffer)(THIS_ UINT32 NumFramesRead) PURE;
	STDMETHOD(GetNextPacketSize)(THIS_ UINT32 *pNumFramesInNextPacket) PURE;
};

DECLARE_INTERFACE_(IAudioRenderClient, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;
	STDMETHOD(GetBuffer)(THIS_ UINT32 NumFramesRequested, BYTE **ppData) PURE;
	STDMETHOD(ReleaseBuffer)(THIS_ UINT32 NumFramesWritten, DWORD dwFlags) PURE;
};


/*
 * GUIDs (defined in wasapi.cpp) - WASAPI_ prefix so nothing clashes with
 * whatever the SDK may or may not declare
 */

extern const GUID WASAPI_CLSID_MMDeviceEnumerator;
extern const GUID WASAPI_IID_IMMDeviceEnumerator;
extern const GUID WASAPI_IID_IAudioClient;
extern const GUID WASAPI_IID_IAudioCaptureClient;
extern const GUID WASAPI_IID_IAudioRenderClient;
extern const WasapiPropertyKey WASAPI_PKEY_Device_FriendlyName;


#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif
#ifndef WAVE_FORMAT_EXTENSIBLE
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#endif

struct WasapiWaveFormatExtensible
{
	WAVEFORMATEX Format;
	WORD wValidBitsPerSample;
	DWORD dwChannelMask;
	GUID SubFormat;		///< Data1 = WAVE_FORMAT_PCM or WAVE_FORMAT_IEEE_FLOAT
};

#endif /* WASAPI_COM_H */

#pragma hdrstop

#include "PhoneInterface.h"
#include "Phone.h"
#include "PhoneConf.h"
#include "PluginV2Methods.h"
#include "scripting/ScriptSource.h"
#include "scripting/ScriptExec.h"
#include "AppStatus.h"
#include "common/Utilities.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include "Log.h"
#include <json/json.h>
#include <SysUtils.hpp>
#include <assert.h>
#include <map>
#include <deque>
#include <set>
#include <ctype.h>
#include <string.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

std::vector<DllInfo> PhoneInterface::dlls;
std::map<AnsiString, class PhoneInterface*> PhoneInterface::instances;

AnsiString PhoneInterface::asDllDir;
std::list<PhoneConf> PhoneInterface::cfg;

AnsiString PhoneInterface::profileDir;

PhoneInterface::CallbackKey PhoneInterface::callbackKey = NULL;
PhoneInterface::CallbackPagingTx PhoneInterface::callbackPagingTx = NULL;
PhoneInterface::CallbackClearDial PhoneInterface::callbackClearDial = NULL;
PhoneInterface::CallbackRedial PhoneInterface::callbackRedial = NULL;
PhoneInterface::CallbackGetNumberDescription PhoneInterface::callbackGetNumberDescription = NULL;
PhoneInterface::CallbackSetKeepForeground PhoneInterface::callbackSetKeepForeground = NULL;
PhoneInterface::CallbackGetCalls PhoneInterface::callbackGetCalls = NULL;
PhoneInterface::CallbackMakeCall PhoneInterface::callbackMakeCall = NULL;
PhoneInterface::CallbackAnswer PhoneInterface::callbackAnswer = NULL;
PhoneInterface::CallbackHangup PhoneInterface::callbackHangup = NULL;
PhoneInterface::CallbackRunScriptContext PhoneInterface::callbackRunScriptContext = NULL;
TPopupMenu* PhoneInterface::trayPopupMenu = NULL;

namespace
{

Mutex mutexInstances;

struct TrayMenuItemEntry
{
	CALLBACK_MENU_ITEM_CLICK callback;
	void *cookie;
	TrayMenuItemEntry(void):
		callback(NULL),
		cookie(NULL)
	{}
};

std::map<TMenuItem*, struct TrayMenuItemEntry> trayMenuItemsMap;

class TrayEventsHandler
{
public:
	// A VCL event handler is expected to be a non-static member of a class.
	// https://stackoverflow.com/questions/49040803/assigning-events-to-a-vcl-control-created-dynamically-at-runtime
	void __fastcall TrayMenuItemClick(TObject *Sender)
	{
		std::map<TMenuItem*, struct TrayMenuItemEntry>::iterator iter;
		TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
		if (item == NULL)
		{
        	return;
		}
		iter = trayMenuItemsMap.find(item);
		if (iter != trayMenuItemsMap.end())
		{
			const struct TrayMenuItemEntry& entry = iter->second;
			if (entry.callback)
			{
            	entry.callback(entry.cookie);
			}
		}
	}
} trayEventsHandler;

PhoneInterface::CallbackRunScript cbRunScript = NULL;

/** static initialization runs in main thread */
const DWORD mainThreadId = GetCurrentThreadId();

/** last registration state per account, for plugin v2 GetRegistration */
std::map<int, Callback::reg_state_e> regStates;

enum { INVOKE_TIMEOUT_MS = 10000 };

/** \brief Plugin v2 Invoke() request passed from other thread to main thread
	\note Reference counted: shared by waiting thread and event queue, so it stays
	valid if waiting thread gives up after timeout.
*/
struct InvokeRequest
{
	LONG refs;
	AnsiString method;
	AnsiString params;
	AnsiString result;
	int status;
	HANDLE done;
	InvokeRequest(void):
		refs(2),
		status(TSIP_ERR_TIMEOUT),
		done(CreateEvent(NULL, TRUE, FALSE, NULL))
	{}
	~InvokeRequest()
	{
		if (done)
			CloseHandle(done);
	}
	void Release(void)
	{
		if (InterlockedDecrement(&refs) == 0)
			delete this;
	}
	void Complete(int status)
	{
		this->status = status;
		SetEvent(done);
	}
};

enum PhoneEventType
{
	PHONE_EVENT__INVALID = 0,
	PHONE_EVENT_SCRIPT,
	PHONE_EVENT_APP_STATUS,
	PHONE_EVENT_KEEP_FOREGROUND,
	PHONE_EVENT_KEY,
	PHONE_PAGING_TX,
	PHONE_CLEAR_DIAL,
	PHONE_REDIAL,
	PHONE_EVENT_INVOKE,		///< plugin v2 Invoke() from other than main thread
	PHONE_EVENT_RESTART,	///< plugin v2 RestartPlugin: Stop() + Start()
};

struct PhoneEvent
{
	void *cookie;
	AnsiString deviceFilename;

	enum PhoneEventType type;

	AnsiString script;

	struct AppStatus
	{
		AnsiString id;
		int priority;
		AnsiString text;
		AppStatus(void):
			priority(-1)
		{}
	} appStatus;

	int keepForeground;		///< 1 = allow, 0 = suspend

	int keyCode;
	int keyState;

	struct PagingTx
	{
		AnsiString target;
		AnsiString filename;
		AnsiString codecname;
	} pagingTx;

	InvokeRequest *invoke;

	PhoneEvent(void):
		cookie(NULL),
		type(PHONE_EVENT__INVALID),
		keepForeground(1),
		keyCode(-1),
		keyState(-1),
		invoke(NULL)
	{}
};

/** \brief Lowercase file names of plugins that suspended "keep main window in foreground"; main thread only */
std::set<AnsiString> keepForegroundSuspenders;

Mutex mutexEventQueue;
std::deque<PhoneEvent> eventQueue;
enum { MAX_EVENT_QUEUE_SIZE = 500 };

/** \brief Message-only window waking main thread to process queued plugin events
	immediately (instead of waiting for next poll timer tick)
	\note created in main thread; PostMessage is safe from any thread
*/
class PollWakeWindow
{
public:
	enum { WM_APP_POLL = WM_APP + 0x51 };
	HWND handle;
	PollWakeWindow(void):
		handle(NULL)
	{}
	// declared before use as closure in Create() - BCC 5.82 internal compiler error otherwise
	void __fastcall WndProc(Messages::TMessage &msg)
	{
		if (msg.Msg == WM_APP_POLL)
		{
			PhoneInterface::Poll();
			msg.Result = 0;
		}
		else
		{
			msg.Result = DefWindowProc(handle, msg.Msg, msg.WParam, msg.LParam);
		}
	}
	void Create(void)
	{
		if (handle == NULL)
			handle = Classes::AllocateHWnd(WndProc);
	}
	void Destroy(void)
	{
		if (handle)
		{
			Classes::DeallocateHWnd(handle);
			handle = NULL;
		}
	}
	void Wake(void)
	{
		if (handle)
			PostMessage(handle, WM_APP_POLL, 0, 0);
	}
};

PollWakeWindow pollWakeWindow;

int EnqueueEvent(const PhoneEvent &event)
{
	{
		ScopedLock<Mutex> lock(mutexEventQueue);
		if (eventQueue.size() >= MAX_EVENT_QUEUE_SIZE)
		{
			return -1;
		}
		eventQueue.push_back(event);
	}
	pollWakeWindow.Wake();
	return 0;
}

/** Registry of live instances for validating cookies in callbacks from plugin
	threads. Separate from mutexInstances: main thread holds mutexInstances
	while calling plugin Stop() (SetCfg, Close) and plugin may join its threads
	there - these threads must not block on mutexInstances in callbacks
	(deadlock). mutexValidInstances is never held while calling into plugin.
*/
Mutex mutexValidInstances;
std::set<void*> validInstances;

}	// namespace


/** Check whether cookie is a live PhoneInterface* without dereferencing it.
    Must be called with mutexValidInstances held. */
bool PhoneInterface::IsValidInstance(void* cookie)
{
	return validInstances.find(cookie) != validInstances.end();
}


void PhoneInterface::EnumerateDlls(AnsiString dir)
{
	if (dir[dir.Length()] != '\\')
		dir += "\\";
	LOG("Enumerating plugin/phone dlls from %s...\n", dir.c_str());
	pollWakeWindow.Create();	// main thread, before any plugin is loaded
	asDllDir = dir;
	dlls.clear();
	WIN32_FIND_DATA file;
	AnsiString asSrchPath = dir + "*.dll";
	HANDLE hFind = FindFirstFile(asSrchPath.c_str(), &file);
	int hasfiles = (hFind != INVALID_HANDLE_VALUE);
	struct DllInfo dllinfo;

	while (hasfiles)
	{
		AnsiString filename = /*dir + */ file.cFileName;
		if(VerifyDll(filename, &dllinfo) == E_OK)
		{
			AddDll(dllinfo);
		}
		else
		{
			LOG("Library %s not loaded\n", filename.c_str());
        }
		hasfiles = FindNextFile(hFind, &file);
	}
	FindClose(hFind);
}

void PhoneInterface::ReEnumerateDlls(void)
{
	EnumerateDlls(asDllDir);
}

void PhoneInterface::SetCfg(std::list<PhoneConf> &newcfg)
{
	ScopedLock<Mutex> lock(mutexInstances);
	std::list<PhoneConf>::iterator it, it2;

	// unloading unused DLLs
	for (it = cfg.begin(); it != cfg.end(); ++it)
	{
		AnsiString dllName = it->dllName;
		bool found = false;
		for (it2 = newcfg.begin(); it2 != newcfg.end(); ++it2)
		{
			if (it2->dllName == dllName)
			{
				found = true;
				break;
			}
		}
		if (found == false)
		{
			std::map<AnsiString, class PhoneInterface*>::iterator itinst = instances.find(LowerCase(dllName));
			if (itinst != instances.end())
			{
				PhoneInterface *interf = itinst->second;
				LOG("Unloading %s\n", interf->filename.c_str());
				interf->Disconnect();
				delete (interf);
			}
		}
	}

	// loading new DLLs
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;

	for (it = newcfg.begin(); it != newcfg.end(); ++it)
	{
		AnsiString dllName = it->dllName;
		bool found = false;		
		for (it2 = cfg.begin(); it2 != cfg.end(); ++it2)
		{
			if (it2->dllName == dllName)
			{
				found = true;
				break;
			}
		}
		if (found == false)
		{
			itinst = instances.find(LowerCase(dllName));
			if (itinst == instances.end())
			{
				PhoneInterface* newPhone = new PhoneInterface(dllName);
				if (newPhone->Load())
				{
					LOG("Error loading %s\n", dllName.c_str());
                    delete newPhone;
				}
				else
				{
					LOG("Loaded %s\n", dllName.c_str());
					instances[LowerCase(dllName)] = newPhone;
					newPhone->SetProfileDir(profileDir);
					newPhone->Connect();
				}
			}
		}
	}
	cfg = newcfg;
}

void PhoneInterface::Close(void)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	while (!instances.empty())
	{
		itinst = instances.begin();
		itinst->second->Disconnect();
		itinst->second->SaveSettings(&itinst->second->settings);
		LOG("Unloading %s\n", itinst->second->filename.c_str());
		delete (itinst->second);
	}
	instances.clear();
	pollWakeWindow.Destroy();
}

void PhoneInterface::UpdateRegistrationState(int accountId, Callback::reg_state_e state)
{
	regStates[accountId] = state;

	{
		ScopedLock<Mutex> lock(mutexInstances);

		std::map<AnsiString, class PhoneInterface*>::iterator itinst;
		for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
		{
			itinst->second->SetRegistrationState(state == Callback::REG_STATE_REGISTER_OK ? 1 : 0);
		}
	}

	Json::Value jv(Json::objectValue);
	jv["accountId"] = accountId;
	jv["state"] = static_cast<int>(state);
	jv["stateName"] = Callback::GetRegStateName(state);
	NotifyEventV2("registration", jv);
}

void PhoneInterface::UpdateCallState(int state, const char* display)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetCallState(state, display);
	}
}

void PhoneInterface::UpdateRing(int state)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->Ring(state);
	}
}

void PhoneInterface::UpdateMuteState(unsigned int callUid, int state)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetMuteState(callUid, state);
	}
}

void PhoneInterface::UpdatePagingTxState(int state)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetPagingTxState(state);
	}
}

void PhoneInterface::UpdateMwi(int accountId, unsigned int newMsg, unsigned int oldMsg)
{
	{
		ScopedLock<Mutex> lock(mutexInstances);

		std::map<AnsiString, class PhoneInterface*>::iterator itinst;
		for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
		{
			itinst->second->SetMwi(accountId, newMsg, oldMsg);
		}
	}

	Json::Value jv(Json::objectValue);
	jv["accountId"] = accountId;
	jv["newMsg"] = newMsg;
	jv["oldMsg"] = oldMsg;
	NotifyEventV2("mwi", jv);
}


int PhoneInterface::ShowPluginSettings(AnsiString dllName, HWND parent)
{
	PhoneInterface *dev = NULL;
	{
		ScopedLock<Mutex> lock(mutexInstances);
		std::map<AnsiString, class PhoneInterface*>::iterator itinst = instances.find(LowerCase(dllName));
		if (itinst == instances.end())
			return SHOW_SETTINGS_NOT_LOADED;
		dev = itinst->second;
		if (!dev->HasSettings())
			return SHOW_SETTINGS_NOT_SUPPORTED;
	}
	// called without lock: settings window is usually modal and plugin threads
	// may need mutexInstances (Invoke, Log) meanwhile;
	// instances are deleted only by main thread
	dev->ShowSettings(parent);
	return SHOW_SETTINGS_OK;
}

int PhoneInterface::SendMessageText(AnsiString asDllName, AnsiString text)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	itinst = instances.find(LowerCase(asDllName));
	if (itinst == instances.end())
	{
		LOG("SendMessageBuffer: DLL %s not found\n", asDllName.c_str());
		return -1;
	}
	return itinst->second->SendMessageText(text);
}

void PhoneInterface::UpdateAudioError(void)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetAudioError();
	}
}

void PhoneInterface::UpdateProfileDir(AnsiString dir)
{
	ScopedLock<Mutex> lock(mutexInstances);

	profileDir = dir;
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetProfileDir(dir);
	}
}


enum PhoneInterface::E_LIB_STATUS PhoneInterface::VerifyDll(AnsiString filename, struct DllInfo* const dllinfo)
{
#if 0
	// Suppressing "Cannot Find ThisLibrary.dll" or "Cannot Find SomeOtherRequired.dll" Error Box
	UINT uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE hInstance = LoadLibrary((asDllDir + filename).c_str());
	SetErrorMode(uOldErrorMode);

	if ((int)hInstance == 2)
	{
		//MessageBox(NULL, "MYDLL.DLL does not exist", "ERROR", MB_OK);
		return false;
	}
	else if ((int)hInstance <= HINSTANCE_ERROR)
	{
		//MessageBox(NULL, "Other problems loading MYDLL.DLL, "ERROR", MB_OK);
		return false;
	}
#else
	// Windows error message when loading may actually be usefull when resolving additional dll dependencies
	HINSTANCE hInstance = LoadLibrary((asDllDir + filename).c_str());
	if (!hInstance)
	{
		int error = GetLastError();
		LOG("Failed to load %s library (error %d)\n", filename.c_str(), error);
		return E_LOADFAILED;
	}
#endif

	// plugin interface v2: checked before v1, plugin exporting both uses only v2;
	// plugin code (tSipPluginInit) is not called here
	if (GetProcAddress(hInstance, TSIP_PLUGIN_INIT_NAME))
	{
		FreeLibrary(hInstance);
		dllinfo->name = filename;
		dllinfo->file_version = GetFileVer(dllinfo->name);
		dllinfo->ver.majorVersion = 2;
		dllinfo->ver.minorVersion = 0;
		return E_OK;
	}

	pfGetPhoneInterfaceDescription gmid =
		(pfGetPhoneInterfaceDescription)GetProcAddress(hInstance, "GetPhoneInterfaceDescription");
	if (!gmid)
	{
		LOG("Library %s DOES NOT look like a compatible plugin (GetPhoneInterfaceDescription() not found)\n",
			filename.c_str());
		FreeLibrary(hInstance);
		return E_NOTVALID;
	}
	struct S_PHONE_DLL_INTERFACE interf_version;
	gmid(&interf_version);
	if (interf_version.majorVersion != DLL_INTERFACE_MAJOR_VERSION ||
		interf_version.minorVersion != DLL_INTERFACE_MINOR_VERSION)
	{
		LOG("Library %s has interface v.%d.%d instead of required %d.%d\n",
			filename.c_str(),
			interf_version.majorVersion, interf_version.minorVersion,
			DLL_INTERFACE_MAJOR_VERSION);
		FreeLibrary(hInstance);
		return E_INCOMPATIBLE_VER;
	}

	FreeLibrary(hInstance);
	dllinfo->name = filename;
	dllinfo->file_version = GetFileVer(dllinfo->name);
	dllinfo->ver.majorVersion = interf_version.majorVersion;
	dllinfo->ver.minorVersion = interf_version.minorVersion;
	return E_OK;
}

void PhoneInterface::AddDll(const struct DllInfo &dllinfo)
{
	LOG("Adding %s\n", dllinfo.name.c_str());
	dlls.push_back(dllinfo);
}

void __stdcall PhoneInterface::OnLog(void *cookie, const char *szText)
{
#if 0
	ScopedLock<Mutex> lock(mutexInstances);
	class DeviceInterface *dev;
	dev = reinterpret_cast<class DeviceInterface*>(cookie);
	if (instances.find(LowerCase(dev)) == instances.end())
	{
		LOG(E_LOG_TRACE, "OnLog called with unknown cookie %p. No matching object.\n", dev);
		return;
	}
#endif
	LOG("%s", szText);
}

void __stdcall PhoneInterface::OnConnect(void *cookie, int state, const char *szMsgText)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		//LOG(E_LOG_TRACE, "OnConnect called with unknown cookie %p. No matching object.\n", dev);
		return;
	}
	dev->connInfo.state = static_cast<enum E_CONNECTION_STATE>(state);
	dev->connInfo.msg = szMsgText;
	if (dev->callbackConnect)
		dev->callbackConnect(state, szMsgText);
}

void __stdcall PhoneInterface::OnKey(void *cookie, int keyCode, int state)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_EVENT_KEY;
	event.keyCode = keyCode;
	event.keyState = state;
	EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnPagingTx(void *cookie, const char* target, const char* filename, const char* codecname)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return -1;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_PAGING_TX;
	if (filename == NULL)
	{
		LOG("Phone: invalid argument for PagingTx, filename == NULL\n");
		return -1;
	}
	if (target == NULL || codecname == NULL)
	{
		LOG("Phone: invalid argument for PagingTx, target or codecname == NULL\n");
		return -1;
	}
	event.pagingTx.target = target;
	event.pagingTx.filename = filename;
	event.pagingTx.codecname = codecname;
	LOG("Phone: pagingTx, target = %s, filename = %s, codecname = %s\n", target, filename, codecname);

	return EnqueueEvent(event);
}

void __stdcall PhoneInterface::OnClearDial(void *cookie)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_CLEAR_DIAL;
	LOG("Phone: ClearDial\n");
	EnqueueEvent(event);
}

void __stdcall PhoneInterface::OnRedial(void *cookie)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_REDIAL;
	LOG("Phone: Redial\n");
	EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnGetNumberDescription(void *cookie, const char* number, char* description, int descriptionSize)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	class PhoneInterface *dev = reinterpret_cast<class PhoneInterface*>(cookie);
	//LOG("Phone: GetNumberDescription\n");
	if (dev->callbackGetNumberDescription)
		return dev->callbackGetNumberDescription(number, description, descriptionSize);
	return -2;
}

int __stdcall PhoneInterface::OnSetVariable(void *cookie, const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	return ScriptExec::SetVariable(name, value);
}

int __stdcall PhoneInterface::OnClearVariable(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	return ScriptExec::ClearVariable(name);
}

int __stdcall PhoneInterface::OnQueuePush(void *cookie, const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	ScriptExec::QueuePush(name, value);
	return 0;
}

int __stdcall PhoneInterface::OnQueuePop(void *cookie, const char* name, char* value, unsigned int valueSize)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;

	if (valueSize == 0)
		return -1;
	AnsiString asValue;
	int ret = ScriptExec::QueuePop(name, asValue);
	strncpy(value, asValue.c_str(), valueSize-1);
	value[valueSize-1] = '\0';
	return ret;
}

int __stdcall PhoneInterface::OnQueueClear(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	return ScriptExec::QueueClear(name);
}

int __stdcall PhoneInterface::OnQueueGetSize(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexValidInstances);
	if (!IsValidInstance(cookie))
		return -1;
	return ScriptExec::QueueGetSize(name);
}

int __stdcall PhoneInterface::OnRunScriptAsync(void *cookie, const char* script)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return -1;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_EVENT_SCRIPT;
	event.script = script;
	return EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnSetAppStatus(void *cookie, const char* id, int priority, const char* text)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return -1;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_EVENT_APP_STATUS;
	event.appStatus.id = id;
	event.appStatus.priority = priority;
	event.appStatus.text = text;
	return EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnSetKeepForeground(void *cookie, int state)
{
	PhoneEvent event;
	event.cookie = cookie;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return -1;
		event.deviceFilename = LowerCase(reinterpret_cast<PhoneInterface*>(cookie)->filename);
	}
	event.type = PHONE_EVENT_KEEP_FOREGROUND;
	event.keepForeground = state ? 1 : 0;
	return EnqueueEvent(event);
}

void PhoneInterface::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	cbRunScript = cb;
}

//---------------------------------------------------------------------------
// Plugin interface v2 (see tSIP_plugin_v2.h, _doc/plugin_interface_v2.md)
//---------------------------------------------------------------------------

int PhoneInterface::LoadV2(pfTSipPluginInit init)
{
	if (stopEventV2 == NULL)
		stopEventV2 = CreateEvent(NULL, TRUE, FALSE, NULL);
	hostApiV2.size = sizeof(hostApiV2);
	hostApiV2.cookie = this;
	hostApiV2.Log = OnLogV2;
	hostApiV2.Invoke = OnInvokeV2;
	hostApiV2.Free = OnFreeV2;
	hostApiV2.SetVariable = OnSetVariableV2;
	hostApiV2.GetVariable = OnGetVariableV2;
	hostApiV2.ClearVariable = OnClearVariableV2;
	hostApiV2.QueuePush = OnQueuePushV2;
	hostApiV2.QueuePop = OnQueuePopV2;
	hostApiV2.QueueGetSize = OnQueueGetSizeV2;
	hostApiV2.QueueClear = OnQueueClearV2;

	memset(&pluginApiV2, 0, sizeof(pluginApiV2));
	pluginApiV2.size = sizeof(pluginApiV2);

	int status = init(&hostApiV2, &pluginApiV2);
	if (status != 0)
	{
		LOG("Plugin %s (interface v2) rejected initialization, status = %d\n", filename.c_str(), status);
		return 3;
	}
	// plugin must not change size; protect against misbehaving plugin
	pluginApiV2.size = sizeof(pluginApiV2);
	isV2 = true;

	if (pluginApiV2.GetMetadata)
	{
		const char* meta = pluginApiV2.GetMetadata();
		LOG("Plugin %s uses interface v2, metadata: %s\n", filename.c_str(), meta ? meta : "");
	}
	else
	{
		LOG("Plugin %s uses interface v2\n", filename.c_str());
	}
	return 0;
}

namespace
{

/** \brief Compact JSON text (plugin interface v2) */
AnsiString JsonToString(const Json::Value &jv)
{
	Json::FastWriter writer;
	std::string text = writer.write(jv);
	if (!text.empty() && text[text.size() - 1] == '\n')
		text.erase(text.size() - 1);
	return text.c_str();
}

Json::Value CallInfoToJson(const PhoneInterface::CallInfoV2 &info)
{
	Json::Value jv(Json::objectValue);
	jv["uid"] = info.uid;
	jv["state"] = static_cast<int>(info.state);
	jv["stateName"] = Callback::GetCallStateName(info.state);
	if (info.hasDetails)
	{
		jv["incoming"] = info.incoming;
		jv["peer"] = info.peerUri.c_str();
		jv["peerName"] = info.peerName.c_str();
		jv["accountId"] = info.accountId;
	}
	return jv;
}

}	// namespace

void PhoneInterface::NotifyEventV2(const char* type, const Json::Value &jv)
{
	AnsiString json = JsonToString(jv);

	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		PhoneInterface *dev = itinst->second;
		if (dev->isV2 && TSIP_PLUGIN_HAS(&dev->pluginApiV2, OnEvent) && dev->pluginApiV2.OnEvent)
		{
			dev->pluginApiV2.OnEvent(type, json.c_str());
		}
	}
}

void PhoneInterface::UpdateCallStateV2(const CallInfoV2 &info)
{
	NotifyEventV2("call_state", CallInfoToJson(info));
}

int PhoneInterface::ExecuteInvoke(AnsiString method, AnsiString jsonParams, AnsiString &jsonResult)
{
	assert(GetCurrentThreadId() == mainThreadId);

	Json::Value params;
	{
		Json::Reader reader;
		if (!reader.parse(jsonParams.c_str(), params) || !(params.isObject() || params.isNull()))
		{
			return TSIP_ERR_INVALID_ARG;
		}
	}

	Json::Value result(Json::objectValue);
	int status = TSIP_OK;
	try
	{
		if (method == "RunScript")
		{
			if (!params["script"].isString())
				return TSIP_ERR_INVALID_ARG;
			if (!callbackRunScriptContext)
				return TSIP_ERR_NOT_AVAILABLE;
			AnsiString script = ::Utf8ToAnsi(params["script"].asString().c_str());
			bool breakRequest = false;
			bool handled = true;
			AnsiString errorText, outputText, returnText;
			ScriptContext context(SCRIPT_SRC_PLUGIN_QUEUE, -1);
			context.errorText = &errorText;
			context.outputText = &outputText;
			context.returnText = &returnText;
			callbackRunScriptContext(context, script, breakRequest, handled);
			if (outputText != "")
				result["output"] = ::AnsiToUtf8(outputText).c_str();
			if (errorText != "")
			{
				result["error"] = ::AnsiToUtf8(errorText).c_str();
				status = TSIP_ERR_GENERIC;
			}
			else if (returnText != "")
			{
				result["return"] = ::AnsiToUtf8(returnText).c_str();
			}
		}
		else if (method == "GetAppInfo")
		{
			char exeName[MAX_PATH] = {0};
			GetModuleFileNameA(NULL, exeName, sizeof(exeName) - 1);
			result["exeName"] = ::AnsiToUtf8(exeName).c_str();
			result["profileDir"] = ::AnsiToUtf8(profileDir).c_str();
			result["pluginInterface"] = 2;
		}
		else if (method == "GetCalls")
		{
			if (!callbackGetCalls)
				return TSIP_ERR_NOT_AVAILABLE;
			CallInfoV2List calls;
			callbackGetCalls(calls);
			Json::Value jcalls(Json::arrayValue);
			for (unsigned int i=0; i<calls.size(); i++)
			{
				jcalls.append(CallInfoToJson(calls[i]));
			}
			result["calls"] = jcalls;
		}
		else if (method == "Call")
		{
			if (!params["target"].isString())
				return TSIP_ERR_INVALID_ARG;
			if (!callbackMakeCall)
				return TSIP_ERR_NOT_AVAILABLE;
			AnsiString headerLines;
			if (!params["headers"].isNull())
			{
				std::string error;
				if (!PluginV2Methods::HeadersToLines(params["headers"], headerLines, error))
				{
					result["error"] = error;
					return TSIP_ERR_INVALID_ARG;
				}
			}
			int accountId = -1;
			if (!params["accountId"].isNull())
			{
				accountId = params["accountId"].asInt();
				if (!PluginV2Methods::IsAccountUsable(accountId))
				{
					result["error"] = "account does not exist or is disabled";
					return TSIP_ERR_INVALID_ARG;
				}
			}
			unsigned int callUid = 0;
			status = callbackMakeCall(::Utf8ToAnsi(params["target"].asString().c_str()), headerLines, callUid, accountId);
			if (status == TSIP_OK)
				result["uid"] = callUid;
		}
		else if (method == "Answer")
		{
			if (!callbackAnswer)
				return TSIP_ERR_NOT_AVAILABLE;
			status = callbackAnswer(params.get("uid", 0).asUInt());
		}
		else if (method == "Hangup")
		{
			if (!callbackHangup)
				return TSIP_ERR_NOT_AVAILABLE;
			int sipCode = params.get("sipCode", 486).asInt();
			if (sipCode < 400 || sipCode > 699)
				sipCode = 486;
			AnsiString reason = ::Utf8ToAnsi(params.get("reason", "Busy Here").asString().c_str());
			status = callbackHangup(params.get("uid", 0).asUInt(), sipCode, reason);
		}
		else if (method == "GetRegistration")
		{
			// all configured accounts, with registration state if known
			Json::Value jaccounts(Json::arrayValue);
			std::map<int, Json::Value> accounts;
			PluginV2Methods::GetAccounts(accounts);
			std::map<int, Json::Value>::iterator ita;
			for (ita = accounts.begin(); ita != accounts.end(); ++ita)
			{
				Json::Value &ja = ita->second;
				std::map<int, Callback::reg_state_e>::const_iterator it = regStates.find(ita->first);
				if (it != regStates.end())
				{
					ja["state"] = static_cast<int>(it->second);
					ja["stateName"] = Callback::GetRegStateName(it->second);
					ja["registered"] = (it->second == Callback::REG_STATE_REGISTER_OK);
				}
				else
				{
					ja["registered"] = false;
				}
				jaccounts.append(ja);
			}
			result["accounts"] = jaccounts;
			result["currentAccountId"] = PluginV2Methods::GetCurrentAccountId();
		}
		else
		{
			status = PluginV2Methods::Execute(method, params, result);
		}
	}
	catch (...)
	{
		// e.g. JSON value of unexpected type
		return TSIP_ERR_INVALID_ARG;
	}

	// result passed also on error, e.g. {"error": "..."}
	jsonResult = JsonToString(result);
	return status;
}

void __stdcall PhoneInterface::OnLogV2(void *cookie, const char *text)
{
	if (text == NULL)
		return;
	OnLog(cookie, ::Utf8ToAnsi(text).c_str());
}

int __stdcall PhoneInterface::OnInvokeV2(void *cookie, const char *method, const char *jsonParams, char **jsonResult)
{
	if (jsonResult)
		*jsonResult = NULL;
	if (method == NULL)
		return TSIP_ERR_INVALID_ARG;

	AnsiString deviceFilename;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		if (!IsValidInstance(cookie))
			return TSIP_ERR_INVALID_ARG;
		PhoneInterface *dev = reinterpret_cast<PhoneInterface*>(cookie);
		deviceFilename = LowerCase(dev->filename);
		if (dev->stopEventV2 && GetCurrentThreadId() != mainThreadId &&
			WaitForSingleObject(dev->stopEventV2, 0) == WAIT_OBJECT_0)
		{
			return TSIP_ERR_NOT_AVAILABLE;	// plugin is being stopped
		}
	}

	AnsiString asMethod = method;
	AnsiString asParams = jsonParams ? jsonParams : "{}";

	// restart is always deferred: plugin may request it from its own Start/Stop/ShowSettings
	if (asMethod == "RestartPlugin")
	{
		PhoneEvent event;
		event.cookie = cookie;
		event.deviceFilename = deviceFilename;
		event.type = PHONE_EVENT_RESTART;
		if (EnqueueEvent(event) != 0)
			return TSIP_ERR_GENERIC;
		if (jsonResult)
			*jsonResult = strdup("{}");
		return TSIP_OK;
	}

	// asynchronous script execution: queue it, same as v1 RunScriptAsync
	if (asMethod == "RunScript")
	{
		Json::Value params;
		Json::Reader reader;
		if (!reader.parse(asParams.c_str(), params) || !params.isObject() || !params["script"].isString())
		{
			return TSIP_ERR_INVALID_ARG;
		}
		if (params.get("async", false).asBool())
		{
			AnsiString script = ::Utf8ToAnsi(params["script"].asString().c_str());
			int status = OnRunScriptAsync(cookie, script.c_str());
			if (status == 0 && jsonResult)
			{
				*jsonResult = strdup("{}");
			}
			return (status == 0) ? TSIP_OK : TSIP_ERR_GENERIC;
		}
	}

	AnsiString result;
	int status;

	if (GetCurrentThreadId() == mainThreadId)
	{
		status = ExecuteInvoke(asMethod, asParams, result);
	}
	else
	{
		// marshal to main thread through event queue, processed by Poll()
		HANDLE stopEvent = NULL;	// own copy: instance may be deleted while waiting
		{
			ScopedLock<Mutex> lock(mutexValidInstances);
			if (!IsValidInstance(cookie))
				return TSIP_ERR_INVALID_ARG;
			PhoneInterface *dev = reinterpret_cast<PhoneInterface*>(cookie);
			if (dev->stopEventV2)
			{
				DuplicateHandle(GetCurrentProcess(), dev->stopEventV2, GetCurrentProcess(), &stopEvent,
					0, FALSE, DUPLICATE_SAME_ACCESS);
			}
		}
		InvokeRequest *req = new InvokeRequest();
		req->method = asMethod;
		req->params = asParams;

		PhoneEvent event;
		event.cookie = cookie;
		event.deviceFilename = deviceFilename;
		event.type = PHONE_EVENT_INVOKE;
		event.invoke = req;
		if (EnqueueEvent(event) != 0)
		{
			req->Release();	// not queued: release both references
			req->Release();
			if (stopEvent)
				CloseHandle(stopEvent);
			return TSIP_ERR_GENERIC;
		}
		HANDLE handles[2] = { req->done, stopEvent };
		DWORD waitStatus = WaitForMultipleObjects(stopEvent ? 2 : 1, handles, FALSE, INVOKE_TIMEOUT_MS);
		if (waitStatus == WAIT_OBJECT_0)
		{
			status = req->status;
			result = req->result;
		}
		else if (waitStatus == WAIT_OBJECT_0 + 1)
		{
			LOG("Plugin %s: Invoke(%s) cancelled - plugin is being stopped\n", deviceFilename.c_str(), asMethod.c_str());
			status = TSIP_ERR_NOT_AVAILABLE;
		}
		else
		{
			LOG("Plugin %s: Invoke(%s) timeout\n", deviceFilename.c_str(), asMethod.c_str());
			status = TSIP_ERR_TIMEOUT;
		}
		req->Release();
		if (stopEvent)
			CloseHandle(stopEvent);
	}

	if (jsonResult && (status == TSIP_OK || result != ""))
	{
		*jsonResult = strdup(result != "" ? result.c_str() : "{}");
	}
	return status;
}

void __stdcall PhoneInterface::OnFreeV2(void *cookie, void *ptr)
{
	(void)cookie;
	free(ptr);	// allocated with strdup() by host
}

/*
 * Variables and queues: ScriptExec functions are thread-safe (own mutexes),
 * executed directly in caller thread, values passed unchanged (no encoding
 * conversion). mutexInstances is deliberately not taken - plugin threads must
 * not wait for main thread here.
 */

int __stdcall PhoneInterface::OnSetVariableV2(void *cookie, const char *name, const char *value)
{
	(void)cookie;
	if (name == NULL || value == NULL)
		return TSIP_ERR_INVALID_ARG;
	ScriptExec::SetVariable(name, value);
	return TSIP_OK;
}

int __stdcall PhoneInterface::OnGetVariableV2(void *cookie, const char *name, char **value)
{
	(void)cookie;
	if (name == NULL || value == NULL)
		return TSIP_ERR_INVALID_ARG;
	*value = NULL;
	AnsiString asValue;
	if (ScriptExec::GetVariable(name, asValue) != 0)
		return TSIP_ERR_NOT_AVAILABLE;
	*value = strdup(asValue.c_str());
	return TSIP_OK;
}

int __stdcall PhoneInterface::OnClearVariableV2(void *cookie, const char *name)
{
	(void)cookie;
	if (name == NULL)
		return TSIP_ERR_INVALID_ARG;
	ScriptExec::ClearVariable(name);
	return TSIP_OK;
}

int __stdcall PhoneInterface::OnQueuePushV2(void *cookie, const char *name, const char *value)
{
	(void)cookie;
	if (name == NULL || value == NULL)
		return TSIP_ERR_INVALID_ARG;
	ScriptExec::QueuePush(name, value);
	return TSIP_OK;
}

int __stdcall PhoneInterface::OnQueuePopV2(void *cookie, const char *name, char **value)
{
	(void)cookie;
	if (name == NULL || value == NULL)
		return TSIP_ERR_INVALID_ARG;
	*value = NULL;
	AnsiString asValue;
	if (ScriptExec::QueuePop(name, asValue) != 0)
		return TSIP_ERR_NOT_AVAILABLE;
	*value = strdup(asValue.c_str());
	return TSIP_OK;
}

int __stdcall PhoneInterface::OnQueueGetSizeV2(void *cookie, const char *name)
{
	(void)cookie;
	if (name == NULL)
		return 0;
	return ScriptExec::QueueGetSize(name);
}

int __stdcall PhoneInterface::OnQueueClearV2(void *cookie, const char *name)
{
	(void)cookie;
	if (name == NULL)
		return TSIP_ERR_INVALID_ARG;
	return (ScriptExec::QueueClear(name) == 0) ? TSIP_OK : TSIP_ERR_NOT_AVAILABLE;
}

void* __stdcall PhoneInterface::OnAddTrayMenuItem(void *cookie, void* parent, const char* caption, CALLBACK_MENU_ITEM_CLICK lpMenuItemClickFn, void *menuItemClickCookie)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return NULL;
	}
	if (GetCurrentThreadId() != MainThreadID)
	{
		LOG("Plugin [%s] error: AddTrayMenuItem must be called from GUI/main thread, e.g. inside of Connect()!\n", dev->filename.c_str());
		return NULL;
	}

	if (parent == NULL)
	{
		int count = trayPopupMenu->Items->Count;
		assert(count > 0);	// expecting at least default "Exit" item, inserting new items before
		TMenuItem *item = new TMenuItem(trayPopupMenu);
		item->AutoHotkeys = maManual;
		item->Caption = caption;
		item->OnClick = &trayEventsHandler.TrayMenuItemClick;
		trayPopupMenu->Items->Insert(count - 1, item);

		struct TrayMenuItemEntry entry;
		entry.callback = lpMenuItemClickFn;
		entry.cookie = menuItemClickCookie;

		trayMenuItemsMap[item] = entry;
		dev->trayMenuItems.push_back(item);

		return item;
	}
	else
	{
		/** \todo Cascaded tray menu for plugins */
		LOG("Cascaded menu is not implemented yet!\n");
	}
	
	return NULL;
}

PhoneInterface::PhoneInterface(AnsiString asDllName):
	hInstance(NULL),
	filename(asDllName),
	capabilities(NULL),
	callbackConnect(NULL),
	dllGetPhoneInterfaceDescription(NULL),
	dllGetPhoneSettings(NULL),
	dllSavePhoneSettings(NULL),
	dllSetCallbacks(NULL),
	dllShowSettings(NULL),
	dllGetPhoneCapabilities(NULL),
	dllConnect(NULL), dllDisconnect(NULL),
	dllSetRegistrationState(NULL),
	dllSetCallState(NULL),
	dllRing(NULL),
	dllSetMuteState(NULL),
	dllSendMessageText(NULL),
	dllSetPagingTxCallback(NULL),
	dllSetPagingTxState(NULL),
	dllSetMwi(NULL),
	dllSetClearDialCallback(NULL),
	dllSetRedialCallback(NULL),
	dllSetGetNumberDescriptionCallback(NULL),
	dllSetSetVariableCallback(NULL),
	dllSetClearVariableCallback(NULL),
	dllSetQueuePushCallback(NULL),
	dllSetQueuePopCallback(NULL),
	dllSetQueueClearCallback(NULL),
	dllSetQueueGetSizeCallback(NULL),
	dllSetAudioError(NULL),
	dllSetProfileDir(NULL),
	dllSetRunScriptAsyncCallback(NULL),
	dllSetAddTrayMenuItemCallback(NULL),
	isV2(false),
	stopEventV2(NULL)
{
	memset(&hostApiV2, 0, sizeof(hostApiV2));
	memset(&pluginApiV2, 0, sizeof(pluginApiV2));
	memset(&settings, 0, sizeof(settings));
	LOG("Creating plugin object using %s\n", asDllName.c_str());
	connInfo.state = DEVICE_DISCONNECTED;
	connInfo.msg = "Not connected";
	instances[LowerCase(asDllName)] = this;
	{
		ScopedLock<Mutex> lock(mutexValidInstances);
		validInstances.insert(this);
	}
}

int PhoneInterface::Connect(void) {
	if (isV2)
	{
		if (TSIP_PLUGIN_HAS(&pluginApiV2, Start) && pluginApiV2.Start)
			return pluginApiV2.Start();
		return 0;
	}
	if (dllConnect)
		return dllConnect();
	return 0;
}

int PhoneInterface::Disconnect(void) {
	std::list<TMenuItem*>::iterator iter;
	for (iter = trayMenuItems.begin(); iter != trayMenuItems.end(); ++iter)
	{
		TMenuItem* item = *iter;
		trayPopupMenu->Items->Remove(item);
	}
	trayMenuItems.clear();

	if (isV2)
	{
		int status = 0;
		if (TSIP_PLUGIN_HAS(&pluginApiV2, Stop) && pluginApiV2.Stop)
		{
			if (stopEventV2)
				SetEvent(stopEventV2);
			status = pluginApiV2.Stop();
			if (stopEventV2)
				ResetEvent(stopEventV2);	// plugin may be started again
		}
		return status;
	}
	if (dllDisconnect)
		return dllDisconnect();
	return 0;
}

PhoneInterface::~PhoneInterface()
{
	{
		// first: callbacks from plugin threads see instance as invalid from now on
		ScopedLock<Mutex> lock(mutexValidInstances);
		validInstances.erase(this);
	}
	ScopedLock<Mutex> lock(mutexInstances);
	std::map<AnsiString, class PhoneInterface*>::iterator it = instances.find(LowerCase(filename));
	if (it != instances.end())
	{
		instances.erase(it);
	}
	if (keepForegroundSuspenders.erase(LowerCase(filename)) > 0 && keepForegroundSuspenders.empty() && callbackSetKeepForeground)
	{
		callbackSetKeepForeground(true);	// plugin unloaded while it was suspending keep-foreground
	}
	if (isV2 && TSIP_PLUGIN_HAS(&pluginApiV2, Shutdown) && pluginApiV2.Shutdown)
	{
		pluginApiV2.Shutdown();
	}
	if (hInstance)
		FreeLibrary(hInstance);
	if (stopEventV2)
		CloseHandle(stopEventV2);
}

int PhoneInterface::Load(void)
{
	ScopedLock<Mutex> lock(mutexInstances);
	AnsiString fullDllName = asDllDir + filename;
	hInstance = LoadLibrary(fullDllName.c_str());
	if (!hInstance)
	{
		if (!FileExists(fullDllName))
		{
			LOG("Plugin DLL file %s was not found\n", filename.c_str());
		}
		return 1;
	}

	pfTSipPluginInit initV2 = (pfTSipPluginInit)GetProcAddress(hInstance, TSIP_PLUGIN_INIT_NAME);
	if (initV2)
	{
		return LoadV2(initV2);
	}

	dllSetCallbacks = (pfSetCallbacks)GetProcAddress(hInstance, "SetCallbacks");
	dllShowSettings = (pfShowSettings)GetProcAddress(hInstance, "ShowSettings");
	dllGetPhoneCapabilities = (pfGetPhoneCapabilities)GetProcAddress(hInstance, "GetPhoneCapabilities");

	dllGetPhoneSettings = (pfGetPhoneSettings)GetProcAddress(hInstance, "GetPhoneSettings");
	dllSavePhoneSettings = (pfSavePhoneSettings)GetProcAddress(hInstance, "SavePhoneSettings");
	dllConnect = (pfConnect)GetProcAddress(hInstance, "Connect");
	dllDisconnect = (pfDisconnect)GetProcAddress(hInstance, "Disconnect");
	dllSetRegistrationState = (pfSetRegistrationState)GetProcAddress(hInstance, "SetRegistrationState");
	dllSetCallState = (pfSetCallState)GetProcAddress(hInstance, "SetCallState");
	dllRing = (pfRing)GetProcAddress(hInstance, "Ring");
	dllSetMuteState = (pfSetMuteState)GetProcAddress(hInstance, "SetMuteState");
	dllSendMessageText = (pfSendMessageText)GetProcAddress(hInstance, "SendMessageText");

	dllSetPagingTxCallback = (pfSetPagingTxCallback)GetProcAddress(hInstance, "SetPagingTxCallback");
	dllSetPagingTxState = (pfSetPagingTxState)GetProcAddress(hInstance, "SetPagingTxState");
	dllSetMwi = (pfSetMwi)GetProcAddress(hInstance, "SetMwi");	
	dllSetClearDialCallback = (pfSetClearDialCallback)GetProcAddress(hInstance, "SetClearDialCallback");
	dllSetRedialCallback = (pfSetRedialCallback)GetProcAddress(hInstance, "SetRedialCallback");

	dllSetGetNumberDescriptionCallback = (pfSetGetNumberDescriptionCallback)GetProcAddress(hInstance, "SetGetNumberDescriptionCallback");

	dllSetSetVariableCallback = (pfSetSetVariableCallback)GetProcAddress(hInstance, "SetSetVariableCallback");
	dllSetClearVariableCallback = (pfSetClearVariableCallback)GetProcAddress(hInstance, "SetClearVariableCallback");

	dllSetQueuePushCallback = (pfSetQueuePushCallback)GetProcAddress(hInstance, "SetQueuePushCallback");
	dllSetQueuePopCallback = (pfSetQueuePopCallback)GetProcAddress(hInstance, "SetQueuePopCallback");
	dllSetQueueClearCallback = (pfSetQueueClearCallback)GetProcAddress(hInstance, "SetQueueClearCallback");
	dllSetQueueGetSizeCallback = (pfSetQueueGetSizeCallback)GetProcAddress(hInstance, "SetQueueGetSizeCallback");

	dllSetAudioError = (pfSetAudioError)GetProcAddress(hInstance, "SetAudioError");

    dllSetProfileDir = (pfSetProfileDir)GetProcAddress(hInstance, "SetProfileDir");

	dllSetRunScriptAsyncCallback = (pfSetRunScriptAsyncCallback)GetProcAddress(hInstance, "SetRunScriptAsyncCallback");

    dllSetAddTrayMenuItemCallback = (pfSetAddTrayMenuItemCallback)GetProcAddress(hInstance, "SetAddTrayMenuItemCallback");

	if ((dllSetCallbacks && dllShowSettings &&
		dllGetPhoneCapabilities && dllConnect &&
		dllDisconnect) == 0)
	{
		LOG("Dll load failed. Some of the required functions are missing\n");
		return 2;
	}

	dllSetCallbacks(this, &OnLog, &OnConnect, &OnKey);
	dllGetPhoneCapabilities(&this->capabilities);

	if (dllSetPagingTxCallback)
	{
		dllSetPagingTxCallback(&OnPagingTx);
	}

	if (dllSetClearDialCallback)
	{
		dllSetClearDialCallback(&OnClearDial);
	}

	if (dllSetRedialCallback)
	{
		dllSetRedialCallback(&OnRedial);
	}

	if (dllSetGetNumberDescriptionCallback)
	{
        dllSetGetNumberDescriptionCallback(&OnGetNumberDescription);
	}

	if (dllSetSetVariableCallback)
	{
		dllSetSetVariableCallback(&OnSetVariable);
	}

	if (dllSetClearVariableCallback)
	{
		dllSetClearVariableCallback(&OnClearVariable);
	}

	if (dllSetQueuePushCallback)
	{
		dllSetQueuePushCallback(&OnQueuePush);
	}

	if (dllSetQueuePopCallback)
	{
		dllSetQueuePopCallback(&OnQueuePop);
	}

	if (dllSetQueueClearCallback)
	{
		dllSetQueueClearCallback(&OnQueueClear);
	}

	if (dllSetQueueGetSizeCallback)
	{
        dllSetQueueGetSizeCallback(&OnQueueGetSize);
	}

	if (dllSetRunScriptAsyncCallback)
	{
    	dllSetRunScriptAsyncCallback(&OnRunScriptAsync);
	}

	if (dllSetAddTrayMenuItemCallback)
	{
    	dllSetAddTrayMenuItemCallback(&OnAddTrayMenuItem);
	}

	pfSetCallbackSetAppStatus dllSetCallbackSetAppStatus = (pfSetCallbackSetAppStatus)GetProcAddress(hInstance, "SetCallbackSetAppStatus");
	if (dllSetCallbackSetAppStatus)
	{
	    dllSetCallbackSetAppStatus(&OnSetAppStatus);
	}

	pfSetCallbackSetKeepForeground dllSetCallbackSetKeepForeground = (pfSetCallbackSetKeepForeground)GetProcAddress(hInstance, "SetCallbackSetKeepForeground");
	if (dllSetCallbackSetKeepForeground)
	{
		dllSetCallbackSetKeepForeground(&OnSetKeepForeground);
	}

	GetSettings(&settings);

	return 0;
}

void PhoneInterface::Poll(void)
{
	// process all queued events (limited, to keep GUI responsive)
	enum { MAX_EVENTS_PER_POLL = 50 };
	for (int i=0; i<MAX_EVENTS_PER_POLL; i++)
	{
		if (!PollOne())
			break;
	}
}

bool PhoneInterface::PollOne(void)
{
	PhoneEvent event;
	{
		// queue lock released before processing: other threads can enqueue
		// (e.g. Invoke requests) while long running script is executed
		ScopedLock<Mutex> lock(mutexEventQueue);
		if (eventQueue.empty())
		{
			return false;
		}
		event = eventQueue.front();
		eventQueue.pop_front();
	}

	ScopedLock<Mutex> lockInstances(mutexInstances);
	std::map<AnsiString, class PhoneInterface*>::iterator itinst = instances.find(event.deviceFilename);
	class PhoneInterface *dev = NULL;
	if (itinst != instances.end())
	{
		dev = itinst->second;
	}
	else if (event.type == PHONE_EVENT_SCRIPT || event.type == PHONE_EVENT_APP_STATUS)
	{
		// Plugin was unloaded after queueing request (e.g. from Disconnect / Stop).
		// Script / application status do not depend on plugin instance - request
		// was made while plugin was loaded, so it is still executed.
	}
	else
	{
		// other requests (e.g. Invoke: Call, Hangup) must not be executed for
		// plugin that is not loaded anymore
		LOG("PhoneInterface: plugin %s is not loaded anymore, dropping queued request (type %d)\n",
			event.deviceFilename.c_str(), static_cast<int>(event.type));
		if (event.invoke)
		{
			event.invoke->Complete(TSIP_ERR_NOT_AVAILABLE);
			event.invoke->Release();
		}
		return true;
	}

	switch (event.type)
	{

	case PHONE_EVENT_INVOKE: {
		InvokeRequest *req = event.invoke;
		assert(req);
		int status = ExecuteInvoke(req->method, req->params, req->result);
		req->Complete(status);
		req->Release();
		break;
	}

	case PHONE_EVENT_RESTART: {
		LOG("Restarting plugin %s (requested by plugin)\n", dev->filename.c_str());
		dev->Disconnect();
		dev->Connect();
		break;
	}

	case PHONE_EVENT_SCRIPT: {
		/** \todo Global break request */
		bool breakRequest = false;
		bool handled = true;
		assert(cbRunScript);
		cbRunScript(SCRIPT_SRC_PLUGIN_QUEUE, -1, event.script, breakRequest, handled);
		break;
	}

	case PHONE_EVENT_APP_STATUS: {
		SetAppStatus(event.appStatus.id, event.appStatus.priority, event.appStatus.text);
		break;
	}

	case PHONE_EVENT_KEEP_FOREGROUND: {
		if (event.keepForeground)
			keepForegroundSuspenders.erase(event.deviceFilename);
		else
			keepForegroundSuspenders.insert(event.deviceFilename);
		if (callbackSetKeepForeground)
			callbackSetKeepForeground(keepForegroundSuspenders.empty());
		break;
	}

	case PHONE_EVENT_KEY: {
		if (dev->callbackKey)
			dev->callbackKey(event.keyCode, event.keyState);
		break;
	}

	case PHONE_PAGING_TX: {
		if (dev->callbackPagingTx)
		{
			dev->callbackPagingTx(event.pagingTx.target.c_str(), event.pagingTx.filename.c_str(), event.pagingTx.codecname.c_str());
		}
		break;
	}

	case PHONE_CLEAR_DIAL: {
		if (dev->callbackClearDial)
			dev->callbackClearDial();
		break;
	}

	case PHONE_REDIAL: {
		if (dev->callbackRedial)
			dev->callbackRedial();
		break;
	}

	default:
		assert(!"Unhandled event type!");
		LOG("PhoneInterface: unhandled event type = %d!\n", static_cast<int>(event.type));
		break;
	}
	return true;
}



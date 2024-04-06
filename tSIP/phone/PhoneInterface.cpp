#pragma hdrstop

#include "PhoneInterface.h"
#include "Phone.h"
#include "PhoneConf.h"
#include "scripting/ScriptSource.h"
#include "scripting/ScriptExec.h"
#include "AppStatus.h"
#include "common/Utilities.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include "Log.h"
#include <SysUtils.hpp>
#include <assert.h>
#include <map>
#include <deque>

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
PhoneInterface::CallbackGetNumberDescription PhoneInterface::callbackGetNumberDescription = NULL;
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

enum PhoneEventType
{
	PHONE_EVENT__INVALID = 0,
	PHONE_EVENT_SCRIPT,
	PHONE_EVENT_APP_STATUS,
	PHONE_EVENT_KEY,
	PHONE_PAGING_TX,
	PHONE_CLEAR_DIAL,
};

struct PhoneEvent
{
	void *cookie;

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

	int keyCode;
	int keyState;

	struct PagingTx
	{
		AnsiString target;
		AnsiString filename;
		AnsiString codecname;
	} pagingTx;

	PhoneEvent(void):
		cookie(NULL),
		type(PHONE_EVENT__INVALID),
		keyCode(-1),
		keyState(-1)
	{}
};

Mutex mutexEventQueue;
std::deque<PhoneEvent> eventQueue;
enum { MAX_EVENT_QUEUE_SIZE = 500 };

int EnqueueEvent(const PhoneEvent &event)
{
	ScopedLock<Mutex> lock(mutexEventQueue);

	if (eventQueue.size() < MAX_EVENT_QUEUE_SIZE)
	{
		eventQueue.push_back(event);
		return 0;
	}
	return -1;
}

}	// namespace


void PhoneInterface::EnumerateDlls(AnsiString dir)
{
	if (dir[dir.Length()] != '\\')
		dir += "\\";
	LOG("Enumerating plugin/phone dlls from %s...\n", dir.c_str());
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
}

void PhoneInterface::UpdateRegistrationState(int state)
{
	ScopedLock<Mutex> lock(mutexInstances);

	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetRegistrationState(state);
	}
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
	event.type = PHONE_EVENT_KEY;
	event.keyCode = keyCode;
	event.keyState = state;
	EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnPagingTx(void *cookie, const char* target, const char* filename, const char* codecname)
{
	PhoneEvent event;
	event.cookie = cookie;
	event.type = PHONE_PAGING_TX;
	event.pagingTx.target = target;
	event.pagingTx.filename = filename;
	event.pagingTx.codecname = codecname;
	if (filename == NULL)
	{
		LOG("Phone: invalid argument for PagingTx, filename == NULL\n");
		return -1;
	}
	LOG("Phone: pagingTx, target = %s, filename = %s, codecname = %s\n", target, filename, codecname);

	return EnqueueEvent(event);
}

void __stdcall PhoneInterface::OnClearDial(void *cookie)
{
	PhoneEvent event;
	event.cookie = cookie;
	event.type = PHONE_CLEAR_DIAL;
	LOG("Phone: ClearDial\n");
	EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnGetNumberDescription(void *cookie, const char* number, char* description, int descriptionSize)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	//LOG("Phone: GetNumberDescription\n");
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackGetNumberDescription)
		return dev->callbackGetNumberDescription(number, description, descriptionSize);
	return -2;
}

int __stdcall PhoneInterface::OnSetVariable(void *cookie, const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return ScriptExec::SetVariable(name, value);
}

int __stdcall PhoneInterface::OnClearVariable(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return ScriptExec::ClearVariable(name);
}

int __stdcall PhoneInterface::OnQueuePush(void *cookie, const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	ScriptExec::QueuePush(name, value);
	return 0;
}

int __stdcall PhoneInterface::OnQueuePop(void *cookie, const char* name, char* value, unsigned int valueSize)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}

	AnsiString asValue;
	int ret = ScriptExec::QueuePop(name, asValue);
	strncpy(value, asValue.c_str(), valueSize-1);
	value[valueSize-1] = '\0';
	return ret;
}

int __stdcall PhoneInterface::OnQueueClear(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return ScriptExec::QueueClear(name);
}

int __stdcall PhoneInterface::OnQueueGetSize(void *cookie, const char* name)
{
	ScopedLock<Mutex> lock(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return ScriptExec::QueueGetSize(name);
}

int __stdcall PhoneInterface::OnRunScriptAsync(void *cookie, const char* script)
{
	PhoneEvent event;
	event.cookie = cookie;
	event.type = PHONE_EVENT_SCRIPT;
	event.script = script;
	return EnqueueEvent(event);
}

int __stdcall PhoneInterface::OnSetAppStatus(void *cookie, const char* id, int priority, const char* text)
{
	PhoneEvent event;
	event.cookie = cookie;
	event.type = PHONE_EVENT_APP_STATUS;
	event.appStatus.id = id;
	event.appStatus.priority = priority;
	event.appStatus.text = text;
	return EnqueueEvent(event);
}

void PhoneInterface::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	cbRunScript = cb;	
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
	dllSetClearDialCallback(NULL),
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
	dllSetAddTrayMenuItemCallback(NULL)
{
	LOG("Creating plugin object using %s\n", asDllName.c_str());
	connInfo.state = DEVICE_DISCONNECTED;
	connInfo.msg = "Not connected";
	instances[LowerCase(asDllName)] = this;
}

int PhoneInterface::Connect(void) {
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

	if (dllDisconnect)
		return dllDisconnect();
	return 0;
}

PhoneInterface::~PhoneInterface()
{
	ScopedLock<Mutex> lock(mutexInstances);
	std::map<AnsiString, class PhoneInterface*>::iterator it = instances.find(LowerCase(filename));
	if (it != instances.end())
	{
		instances.erase(it);
	}
	if (hInstance)
		FreeLibrary(hInstance);
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
	dllSetClearDialCallback = (pfSetClearDialCallback)GetProcAddress(hInstance, "SetClearDialCallback");

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

	GetSettings(&settings);

	return 0;
}

void PhoneInterface::Poll(void)
{
	ScopedLock<Mutex> lock(mutexEventQueue);
	if (eventQueue.empty())
	{
		return;
	}

	PhoneEvent event = eventQueue.front();
	eventQueue.pop_front();

	ScopedLock<Mutex> lockInstances(mutexInstances);
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(event.cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
        LOG("PhoneInterface: device %p not found!\n", dev);
		return;
	}

	switch (event.type)
	{

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

	default:
		assert(!"Unhandled event type!");
		LOG("PhoneInterface: unhandled event type = %d!\n", static_cast<int>(event.type));
		break;
	}
}



#pragma hdrstop

#include "PhoneInterface.h"
#include "Phone.h"
#include "PhoneConf.h"
#include "ScriptSource.h"
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
PhoneInterface::CallbackSetVariable PhoneInterface::callbackSetVariable = NULL;
PhoneInterface::CallbackClearVariable PhoneInterface::callbackClearVariable = NULL;
PhoneInterface::CallbackQueuePush PhoneInterface::callbackQueuePush = NULL;
PhoneInterface::CallbackQueuePop PhoneInterface::callbackQueuePop = NULL;
PhoneInterface::CallbackQueueClear PhoneInterface::callbackQueueClear = NULL;
PhoneInterface::CallbackQueueGetSize PhoneInterface::callbackQueueGetSize = NULL;
TPopupMenu* PhoneInterface::trayPopupMenu = NULL;

namespace
{

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

class EventsHandler
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
} eventsHandler;

Mutex mutexScriptQueue;
std::deque<AnsiString> enqueuedScripts;
enum { MAX_SCRIPT_QUEUE_SIZE = 1000 };
PhoneInterface::CallbackRunScript cbRunScript = NULL;

int EnqueueScript(AnsiString script)
{
	ScopedLock<Mutex> lock(mutexScriptQueue);

	if (enqueuedScripts.size() < MAX_SCRIPT_QUEUE_SIZE)
	{
		enqueuedScripts.push_back(script);
		return 0;
	}
	return -1;
}

void PollScriptQueue(void)
{
	ScopedLock<Mutex> lock(mutexScriptQueue);
	if (enqueuedScripts.empty())
	{
		return;
	}
	AnsiString script = enqueuedScripts.front();
	enqueuedScripts.pop_front();

	/** \todo Global break request */
	bool breakRequest = false;
	bool handled = true;
	assert(cbRunScript);
	cbRunScript(SCRIPT_SRC_PLUGIN_QUEUE, -1, script, breakRequest, handled);
}


Mutex mutexAppStatus;
struct AppStatusEntry
{
	AnsiString id;
	int priority;
	AnsiString text;
};
std::deque<struct AppStatusEntry> enqueuedAppStatus;
enum { MAX_APP_STATUS_QUEUE_SIZE = 1000 };
int EnqueueAppStatus(const char* id, int priority, const char* text)
{
	ScopedLock<Mutex> lock(mutexAppStatus);
	if (enqueuedAppStatus.size() < MAX_APP_STATUS_QUEUE_SIZE)
	{
		struct AppStatusEntry entry;
		entry.id = id;
		entry.priority = priority;
		entry.text = text;	
		enqueuedAppStatus.push_back(entry);
		return 0;
	}
	return -1;
}

void PollAppStatusQueue(void)
{
	ScopedLock<Mutex> lock(mutexAppStatus);
	if (enqueuedAppStatus.empty())
	{
		return;
	}
	const struct AppStatusEntry entry = enqueuedAppStatus.front();
	enqueuedAppStatus.pop_front();
	SetAppStatus(entry.id, entry.priority, entry.text);
}

}	// namespace


void PhoneInterface::EnumerateDlls(AnsiString dir)
{
	LOG("Enumerating phone device dlls...\n");
	if (dir[dir.Length()] != '\\')
		dir += "\\";	
	LOG("Directory: %s\n", dir.c_str());
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
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetRegistrationState(state);
	}
}

void PhoneInterface::UpdateCallState(int state, const char* display)
{
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetCallState(state, display);
	}
}

void PhoneInterface::UpdateRing(int state)
{
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->Ring(state);
	}
}

void PhoneInterface::UpdatePagingTxState(int state)
{
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetPagingTxState(state);
	}
}

int PhoneInterface::SendMessageText(AnsiString asDllName, AnsiString text)
{
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
	std::map<AnsiString, class PhoneInterface*>::iterator itinst;
	for (itinst = instances.begin(); itinst != instances.end(); ++itinst)
	{
		itinst->second->SetAudioError();
	}
}

void PhoneInterface::UpdateProfileDir(AnsiString dir)
{
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
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	LOG("Phone: key %d, state %d\n", keyCode, state);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		//LOG(E_LOG_TRACE, "OnKey called with unknown cookie %p. No matching object.\n", dev);
		return;
	}
	if (dev->callbackKey)
		dev->callbackKey(keyCode, state);
}

int __stdcall PhoneInterface::OnPagingTx(void *cookie, const char* target, const char* filename, const char* codecname)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (filename == NULL)
	{
		LOG("Phone: invalid argument for PagingTx, filename == NULL\n");
		return -1;
	}
	LOG("Phone: pagingTx, target = %s, filename = %s, codecname = %s\n", target, filename, codecname);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		//LOG(E_LOG_TRACE, "OnPagingTx called with unknown cookie %p. No matching object.\n", dev);
		return -2;
	}
	if (dev->callbackPagingTx)
	{
		return dev->callbackPagingTx(target, filename, codecname);
	}
	else
	{
		return -3;
	}
}

void __stdcall PhoneInterface::OnClearDial(void *cookie)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	LOG("Phone: ClearDial\n");
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		//LOG(E_LOG_TRACE, "OnKey called with unknown cookie %p. No matching object.\n", dev);
		return;
	}
	if (dev->callbackClearDial)
		dev->callbackClearDial();
}

int __stdcall PhoneInterface::OnGetNumberDescription(void *cookie, const char* number, char* description, int descriptionSize)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	//LOG("Phone: ClearDial\n");
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
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackSetVariable)
		return dev->callbackSetVariable(name, value);
	return -2;
}

int __stdcall PhoneInterface::OnClearVariable(void *cookie, const char* name)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackClearVariable)
		return dev->callbackClearVariable(name);
	return -2;
}

int __stdcall PhoneInterface::OnQueuePush(void *cookie, const char* name, const char* value)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackQueuePush)
	{
		dev->callbackQueuePush(name, value);
		return 0;
	}
	return -2;
}

int __stdcall PhoneInterface::OnQueuePop(void *cookie, const char* name, char* value, unsigned int valueSize)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackQueuePop == NULL)
		return -2;
	AnsiString asValue;
	int ret = dev->callbackQueuePop(name, asValue);
	strncpy(value, asValue.c_str(), valueSize-1);
	value[valueSize-1] = '\0';
	return ret;
}

int __stdcall PhoneInterface::OnQueueClear(void *cookie, const char* name)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackQueueClear)
		return dev->callbackQueueClear(name);
	return -2;
}

int __stdcall PhoneInterface::OnQueueGetSize(void *cookie, const char* name)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	if (dev->callbackQueueGetSize)
		return dev->callbackQueueGetSize(name);
	return -2;
}

int __stdcall PhoneInterface::OnRunScriptAsync(void *cookie, const char* script)
{
	class PhoneInterface *dev;
	dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return EnqueueScript(script);
}

int __stdcall PhoneInterface::OnSetAppStatus(void *cookie, const char* id, int priority, const char* text)
{
	class PhoneInterface *dev = reinterpret_cast<class PhoneInterface*>(cookie);
	if (instances.find(LowerCase(dev->filename)) == instances.end())
	{
		return -1;
	}
	return EnqueueAppStatus(id, priority, text);
}

void PhoneInterface::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	cbRunScript = cb;	
}

void* __stdcall PhoneInterface::OnAddTrayMenuItem(void *cookie, void* parent, const char* caption, CALLBACK_MENU_ITEM_CLICK lpMenuItemClickFn, void *menuItemClickCookie)
{
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
		item->OnClick = &eventsHandler.TrayMenuItemClick;
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
	PollScriptQueue();
	PollAppStatusQueue();
}



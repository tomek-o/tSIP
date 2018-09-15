#pragma hdrstop

#include "PhoneInterface.h"
#include "Phone.h"
#include "PhoneConf.h"
#include "Utilities.h"
#include "Log.h"
#include <SysUtils.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

std::vector<DllInfo> PhoneInterface::dlls;
std::map<AnsiString, class PhoneInterface*> PhoneInterface::instances;
AnsiString PhoneInterface::asDllDir;
std::list<PhoneConf> PhoneInterface::cfg;

PhoneInterface::CallbackKey PhoneInterface::callbackKey = NULL;
PhoneInterface::CallbackPagingTx PhoneInterface::callbackPagingTx = NULL;
PhoneInterface::CallbackClearDial PhoneInterface::callbackClearDial = NULL;
PhoneInterface::CallbackGetNumberDescription PhoneInterface::callbackGetNumberDescription = NULL;
PhoneInterface::CallbackSetVariable PhoneInterface::callbackSetVariable = NULL;
PhoneInterface::CallbackClearVariable PhoneInterface::callbackClearVariable = NULL;

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
		LOG("Library %s DOES NOT look like a compatible plugin\n",
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
		//LOG(E_LOG_TRACE, "OnConnect called with unknown cookie %p. No matching object.", dev);
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
		//LOG(E_LOG_TRACE, "OnKey called with unknown cookie %p. No matching object.", dev);
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
		//LOG(E_LOG_TRACE, "OnPagingTx called with unknown cookie %p. No matching object.", dev);
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
		//LOG(E_LOG_TRACE, "OnKey called with unknown cookie %p. No matching object.", dev);
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
	dllSetClearVariableCallback(NULL)
{
	LOG("Creating object using %s\n", asDllName.c_str());
	connInfo.state = DEVICE_DISCONNECTED;
	connInfo.msg = "Not connected";
	instances[LowerCase(asDllName)] = this;
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
	hInstance = LoadLibrary((asDllDir + filename).c_str());
	if (!hInstance)
		return 1;
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

	GetSettings(&settings);

	return 0;
}



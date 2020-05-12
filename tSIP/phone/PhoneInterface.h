#ifndef PhoneInterfaceH
#define PhoneInterfaceH
//---------------------------------------------------------------------------

#include "Phone.h"
#include "PhoneSettings.h"
#include <System.hpp>
#include <Menus.hpp>
#include <vector>
#include <map>
#include <list>

struct PhoneConf;

/** \brief Info about dll device library */
struct DllInfo
{
	AnsiString name;
	AnsiString description;
	AnsiString file_version;			///< file version (as reported by OS)
	struct S_PHONE_DLL_INTERFACE ver;
};

/** \brief Info about current connection state
 */
struct ConnectionInfo
{
	enum E_CONNECTION_STATE state;
	AnsiString msg;
};

/** \brief Encapsulates dll plugin
*/
class PhoneInterface
{
private:
	enum E_LIB_STATUS
	{
		E_OK = 0,
		E_LOADFAILED,			///< failed to load library - library not found or other dll dependency not met
		E_NOTVALID,				///< does not look like a plugin library
		E_INCOMPATIBLE_VER		///< incompatible plugin interface version
	};

	/** \brief Verify if dll looks correctly and fill dllinfo for valid dll
		\return loading status
	*/
	static enum E_LIB_STATUS VerifyDll(AnsiString filename, struct DllInfo* const dllinfo);
	/** \brief Add dll to list
	*/
	static void AddDll(const struct DllInfo &dllinfo);

	static std::map<AnsiString, class PhoneInterface*> instances;

	static AnsiString asDllDir;
	AnsiString filename;	///< dll name (with full path)
	HINSTANCE hInstance;	///< dll instance ptr
	// dll function pointers
	pfGetPhoneInterfaceDescription dllGetPhoneInterfaceDescription;
	pfGetPhoneSettings dllGetPhoneSettings;
	pfSavePhoneSettings dllSavePhoneSettings;
	pfSetCallbacks dllSetCallbacks;
	pfShowSettings dllShowSettings;
	pfGetPhoneCapabilities dllGetPhoneCapabilities;
	pfConnect dllConnect;
	pfDisconnect dllDisconnect;
	pfSetRegistrationState dllSetRegistrationState;
	pfSetCallState dllSetCallState;
	pfRing dllRing;
	pfSendMessageText dllSendMessageText;

    pfSetPagingTxCallback dllSetPagingTxCallback;
	pfSetPagingTxState dllSetPagingTxState;

	pfSetClearDialCallback dllSetClearDialCallback;

	pfSetGetNumberDescriptionCallback dllSetGetNumberDescriptionCallback;

	pfSetSetVariableCallback dllSetSetVariableCallback;
	pfSetClearVariableCallback dllSetClearVariableCallback;

	pfSetQueuePushCallback dllSetQueuePushCallback;
	pfSetQueuePopCallback dllSetQueuePopCallback;
	pfSetQueueClearCallback dllSetQueueClearCallback;
	pfSetQueueGetSizeCallback dllSetQueueGetSizeCallback;

	pfSetAudioError dllSetAudioError;

	pfSetProfileDir dllSetProfileDir;

	pfSetRunScriptAsyncCallback dllSetRunScriptAsyncCallback;

	pfSetAddTrayMenuItemCallback dllSetAddTrayMenuItemCallback;

	struct ConnectionInfo connInfo;

	S_PHONE_SETTINGS settings;
	std::list<TMenuItem*> trayMenuItems;	

	// CALLBACKS DEFINITIONS
	typedef void (__closure *CallbackConnect)(int state, const char *szMsgText);
	typedef void (__closure *CallbackKey)(int keyCode, int state);
	typedef int (__closure *CallbackPagingTx)(const char* target, const char* filename, const char* codecname);
	typedef void (__closure *CallbackClearDial)(void);
	typedef int (__closure *CallbackGetNumberDescription)(const char* number, char* description, int descriptionSize);
	typedef int (__closure *CallbackSetVariable)(const char* name, const char* value);
	typedef int (__closure *CallbackClearVariable)(const char* name);
	typedef void (__closure *CallbackQueuePush)(const char* name, const char* value);
	typedef int (__closure *CallbackQueuePop)(const char* name, AnsiString &value);
	typedef int (__closure *CallbackQueueClear)(const char* name);
	typedef int (__closure *CallbackQueueGetSize)(const char* name);

	static std::list<PhoneConf> cfg;
	static PhoneConf& FindCfg(AnsiString dllName);

	static AnsiString profileDir;

public:
	/** \brief Make list of valid dlls in supplied location
	*/
	static void EnumerateDlls(AnsiString dir);
	/** \brief Refresh list of valid dlls in previously supplied location
	*/
	static void ReEnumerateDlls(void);

	static void SetCfg(std::list<PhoneConf> &cfg);

	static void Close(void);

	static void UpdateRegistrationState(int state);

	static void UpdateCallState(int state, const char* display);

	static void UpdateRing(int state);

	static void UpdatePagingTxState(int state);

	static int SendMessageText(AnsiString asDllName, AnsiString text);

	static void UpdateAudioError(void);

	static void UpdateProfileDir(AnsiString dir); 

	/** \brief Constructor
		\param asDllName	Name of dll used by object to communicate with device
	*/
	PhoneInterface(AnsiString asDllName);
	~PhoneInterface();
	int Load(void);			///< load dll (dll name was supplied to constructor)
    /** Info about found device interface libraries */
	static std::vector<DllInfo> dlls;
	/** Ptr to structure describing device functionality. Actual structure is allocated
	 *  by device library
	 */
	struct S_PHONE_CAPABILITIES *capabilities;
	CallbackConnect callbackConnect;   ///< if set called when connection state changes
	static CallbackKey callbackKey;
	static CallbackPagingTx callbackPagingTx;
	static CallbackClearDial callbackClearDial;
	static CallbackGetNumberDescription callbackGetNumberDescription;
	static CallbackSetVariable callbackSetVariable;
	static CallbackClearVariable callbackClearVariable;
	static CallbackQueuePush callbackQueuePush;
	static CallbackQueuePop callbackQueuePop;
	static CallbackQueueClear callbackQueueClear;
	static CallbackQueueGetSize callbackQueueGetSize;
	static TPopupMenu *trayPopupMenu;

	// dll callbacks
	static void __stdcall OnLog(void *cookie, const char *szText);                   ///< called to generate log in parent application
	static void __stdcall OnConnect(void *cookie, int state, const char *szMsgText); ///< called on connection/disconnection of device
	static void __stdcall OnKey(void *cookie, int keyCode, int state);
	static int __stdcall OnPagingTx(void *cookie, const char* target, const char* filename, const char* codecname);
	static void __stdcall OnClearDial(void *cookie);
	static int __stdcall OnGetNumberDescription(void *cookie, const char* number, char* description, int descriptionSize);
	static int __stdcall OnSetVariable(void *cookie, const char* name, const char* value);
	static int __stdcall OnClearVariable(void *cookie, const char* name);
	static int __stdcall OnQueuePush(void *cookie, const char* name, const char* value);
	static int __stdcall OnQueuePop(void *cookie, const char* name, char* value, unsigned int valueSize);
	static int __stdcall OnQueueClear(void *cookie, const char* name);
	static int __stdcall OnQueueGetSize(void *cookie, const char* name);
	static void* __stdcall OnAddTrayMenuItem(void *cookie, void* parent, const char* caption, CALLBACK_MENU_ITEM_CLICK lpMenuItemClickFn, void *menuItemClickCookie);
	static int __stdcall OnRunScriptAsync(void *cookie, const char* script);
	static int __stdcall OnSetAppStatus(void *cookie, const char* id, int priority, const char* text);
	
	static void Poll(void);
	typedef int (__closure *CallbackRunScript)(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled);
	static void SetCallbackRunScript(CallbackRunScript cb);

    /** \brief Show device settings window.
     *
     *  Show device settings window. Application doesn't manage any settigs related to input devices. Device dll
     *  is responsible for storing/restoring data and can supply it's own window for this settings.
     */
	void ShowSettings(HANDLE parent) {
		if (dllShowSettings)
			dllShowSettings(parent);
	}
	/** \brief Connect device
	*/
	int Connect(void);
	/** \brief Disconnect device
	*/
	int Disconnect(void);
	/** \brief Get default or saved in configuration settings for device
	*/
	int GetSettings(struct S_PHONE_SETTINGS* settings)
	{
		if (dllGetPhoneSettings)
			return dllGetPhoneSettings(settings);
		return -1;
	}

	/** \brief Save configuration settings for device
	*/
	int SaveSettings(struct S_PHONE_SETTINGS* settings)
	{
		if (dllSavePhoneSettings)
			return dllSavePhoneSettings(settings);
		return -1;
	}

	int SetRegistrationState(int state)
	{
		if (dllSetRegistrationState)
			return dllSetRegistrationState(state);
		return -1;
	}

	int SetCallState(int state, const char* display)
	{
		if (dllSetCallState)
			return dllSetCallState(state, display);
		return -1;
	}

	int Ring(int state) {
		if (settings.ring == 0)
			state = 0;
		if (dllRing)
			return dllRing(state);
		return -1;
	}

	int SendMessageText(AnsiString text) {
		if (dllSendMessageText)
			return dllSendMessageText(text.c_str());		
		return -1;
	}

	int SetPagingTxState(int state)
	{
		if (dllSetPagingTxState)
			return dllSetPagingTxState(state);
		return -1;
	}

	int SetAudioError(void)
	{
		if (dllSetAudioError)
		{
			dllSetAudioError();
			return 0;
		}
		return -1;
	}

	int SetProfileDir(AnsiString dir)
	{
		if (dllSetProfileDir)
		{
			dllSetProfileDir(dir.c_str());
			return 0;
		}
		return -1;
	}

	const struct ConnectionInfo& GetConnectionInfo(void) {
		return connInfo;
	}

};

#endif

#ifndef PhoneInterfaceH
#define PhoneInterfaceH
//---------------------------------------------------------------------------

#include "Phone.h"
#include "tSIP_plugin_v2.h"
#include "PhoneSettings.h"
#include "Callback.h"
#include "ScriptContext.h"
#include <System.hpp>
#include <Menus.hpp>
#include <vector>
#include <map>
#include <list>

namespace Json
{
	class Value;
}

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

	static bool IsValidInstance(void* cookie);	

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
	pfSetMuteState dllSetMuteState;
	pfSendMessageText dllSendMessageText;

    pfSetPagingTxCallback dllSetPagingTxCallback;
	pfSetPagingTxState dllSetPagingTxState;
	pfSetMwi dllSetMwi;

	pfSetClearDialCallback dllSetClearDialCallback;

	pfSetRedialCallback dllSetRedialCallback;

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

	// plugin interface v2 (see tSIP_plugin_v2.h); v1 function pointers stay NULL for v2 plugin
	bool isV2;
	/** \brief Manual-reset event, signaled while plugin Stop() is executed:
		Invoke() calls from plugin threads waiting for main thread are cancelled
		(main thread is busy in Stop(), plugin may be joining these threads)
	*/
	HANDLE stopEventV2;
	struct TSIP_HOST_API hostApiV2;
	struct TSIP_PLUGIN_API pluginApiV2;
	int LoadV2(pfTSipPluginInit init);

	S_PHONE_SETTINGS settings;
	std::list<TMenuItem*> trayMenuItems;	

	// CALLBACKS DEFINITIONS
	typedef void (__closure *CallbackConnect)(int state, const char *szMsgText);
	typedef void (__closure *CallbackKey)(int keyCode, int state);
	typedef int (__closure *CallbackPagingTx)(const char* target, const char* filename, const char* codecname);
	typedef void (__closure *CallbackClearDial)(void);
	typedef void (__closure *CallbackRedial)(void);
	typedef int (__closure *CallbackGetNumberDescription)(const char* number, char* description, int descriptionSize);

	static std::list<PhoneConf> cfg;
	static PhoneConf& FindCfg(AnsiString dllName);

	static AnsiString profileDir;

	/** \brief Pass event to plugins using interface v2
		\param type event type, e.g. "call_state"
		\param jv event data (JSON object, UTF-8 strings)
	*/
	static void NotifyEventV2(const char* type, const Json::Value &jv);

public:
	/** \brief Make list of valid dlls in supplied location
	*/
	static void EnumerateDlls(AnsiString dir);
	/** \brief Refresh list of valid dlls in previously supplied location
	*/
	static void ReEnumerateDlls(void);

	static void SetCfg(std::list<PhoneConf> &cfg);

	static void Close(void);

	/** \brief Notify plugins about registration state change
		\details v1 plugins get simplified state (1 = registered, 0 = not registered),
		v2 plugins get "registration" event
	*/
	static void UpdateRegistrationState(int accountId, Callback::reg_state_e state);

	static void UpdateCallState(int state, const char* display);

	static void UpdateRing(int state);

	static void UpdateMuteState(unsigned int callUid, int state);

	static void UpdatePagingTxState(int state);

	/** \brief Notify plugins about MWI state: v1 SetMwi + v2 "mwi" event */
	static void UpdateMwi(int accountId, unsigned int newMsg, unsigned int oldMsg);

	static int SendMessageText(AnsiString asDllName, AnsiString text);

	static void UpdateAudioError(void);

	enum ShowPluginSettingsStatus {
		SHOW_SETTINGS_OK = 0,
		SHOW_SETTINGS_NOT_LOADED = -1,	///< plugin is not active (not loaded)
		SHOW_SETTINGS_NOT_SUPPORTED = -2	///< plugin does not provide settings window
	};
	/** \brief Show settings window of loaded plugin (v1 ShowSettings export or v2 ShowSettings) - main thread
		\return ShowPluginSettingsStatus value
	*/
	static int ShowPluginSettings(AnsiString dllName, HWND parent);

	static void UpdateProfileDir(AnsiString dir);

	/** \brief Call information for plugin interface v2 */
	struct CallInfoV2
	{
		unsigned int uid;
		Callback::ua_state_e state;
		bool hasDetails;		///< false if call object is not available (e.g. already closed): fields below are not valid
		bool incoming;
		AnsiString peerUri;		///< UTF-8
		AnsiString peerName;	///< UTF-8
		CallInfoV2(void):
			uid(0),
			state(Callback::CALL_STATE_CLOSED),
			hasDetails(false),
			incoming(false)
		{}
	};

	/** \brief Notify v2 plugins about call state change ("call_state" event) */
	static void UpdateCallStateV2(const CallInfoV2 &info);

	// Handlers for plugin v2 Invoke() methods implemented by application; called in main thread.
	// Return TSIP_OK or TSIP_ERR_* code.
	typedef std::vector<CallInfoV2> CallInfoV2List;
	typedef void (__closure *CallbackGetCalls)(CallInfoV2List &calls);
	/** \param target number or URI, ANSI
		\param extraHeaderLines validated "Name: value" lines separated with CRLF, may be empty */
	typedef int (__closure *CallbackMakeCall)(AnsiString target, AnsiString extraHeaderLines, unsigned int &callUid);
	/** \param callUid 0 = current call */
	typedef int (__closure *CallbackAnswer)(unsigned int callUid);
	/** \param callUid 0 = current call
		\param reason ANSI
	*/
	typedef int (__closure *CallbackHangup)(unsigned int callUid, int sipCode, AnsiString reason);
	static CallbackGetCalls callbackGetCalls;
	static CallbackMakeCall callbackMakeCall;
	static CallbackAnswer callbackAnswer;
	static CallbackHangup callbackHangup;

	/** \brief Execute plugin v2 Invoke() request - main thread only */
	static int ExecuteInvoke(AnsiString method, AnsiString jsonParams, AnsiString &jsonResult);

	// plugin interface v2 host functions
	static void __stdcall OnLogV2(void *cookie, const char *text);
	static int __stdcall OnInvokeV2(void *cookie, const char *method, const char *jsonParams, char **jsonResult);
	static void __stdcall OnFreeV2(void *cookie, void *ptr);
	static int __stdcall OnSetVariableV2(void *cookie, const char *name, const char *value);
	static int __stdcall OnGetVariableV2(void *cookie, const char *name, char **value);
	static int __stdcall OnClearVariableV2(void *cookie, const char *name);
	static int __stdcall OnQueuePushV2(void *cookie, const char *name, const char *value);
	static int __stdcall OnQueuePopV2(void *cookie, const char *name, char **value);
	static int __stdcall OnQueueGetSizeV2(void *cookie, const char *name);
	static int __stdcall OnQueueClearV2(void *cookie, const char *name);

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
	static CallbackRedial callbackRedial;
	static CallbackGetNumberDescription callbackGetNumberDescription;
	static TPopupMenu *trayPopupMenu;

	// dll callbacks
	static void __stdcall OnLog(void *cookie, const char *szText);                   ///< called to generate log in parent application
	static void __stdcall OnConnect(void *cookie, int state, const char *szMsgText); ///< called on connection/disconnection of device
	static void __stdcall OnKey(void *cookie, int keyCode, int state);
	static int __stdcall OnPagingTx(void *cookie, const char* target, const char* filename, const char* codecname);
	static void __stdcall OnClearDial(void *cookie);
	static void __stdcall OnRedial(void *cookie);
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
	/** \brief Process single queued plugin event
		\return false if queue was empty
	*/
	static bool PollOne(void);
	typedef int (__closure *CallbackRunScript)(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled);
	static void SetCallbackRunScript(CallbackRunScript cb);
	/** \brief Script execution with full context (v2 RunScript: error returned to plugin) */
	typedef int (__closure *CallbackRunScriptContext)(const ScriptContext &context, AnsiString script, bool &breakRequest, bool &handled);
	static CallbackRunScriptContext callbackRunScriptContext;

    /** \brief Show device settings window.
     *
     *  Show device settings window. Application doesn't manage any settigs related to input devices. Device dll
     *  is responsible for storing/restoring data and can supply it's own window for this settings.
     */
	bool HasSettings(void) const {
		if (isV2)
			return TSIP_PLUGIN_HAS(&pluginApiV2, ShowSettings) && pluginApiV2.ShowSettings;
		return dllShowSettings != NULL;
	}
	void ShowSettings(HANDLE parent) {
		if (isV2)
		{
			if (TSIP_PLUGIN_HAS(&pluginApiV2, ShowSettings) && pluginApiV2.ShowSettings)
				pluginApiV2.ShowSettings(parent);
			return;
		}
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

	int SetMuteState(unsigned int callUid, int state)
	{
		if (dllSetMuteState)
			return dllSetMuteState(callUid, state);
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

	int SetMwi(int accountId, unsigned int newMsg, unsigned int oldMsg)
	{
		if (dllSetMwi)
			return dllSetMwi(accountId, newMsg, oldMsg);
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

	const struct ConnectionInfo& GetConnectionInfo(void) const {
		return connInfo;
	}

};

#endif

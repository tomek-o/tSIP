//---------------------------------------------------------------------------

#ifndef ScriptExecH
#define ScriptExecH
//---------------------------------------------------------------------------

#include "ScriptSource.h"
#include "LuaWinapi.h"

#include <string>
#include <System.hpp>

class LuaState;
struct lua_State;
struct luaL_reg;

class ButtonConf;

struct Call;

class ScriptExec
{
private:
	friend class LuaState;

	typedef void (__closure *CallbackAddOutputText)(const char* text);
	typedef void (__closure *CallbackCall)(const char* number);
	typedef void (__closure *CallbackHangup)(void);
	typedef void (__closure *CallbackAnswer)(void);
	typedef std::string (__closure *CallbackGetDial)(void);
	typedef void (__closure *CallbackSetDial)(std::string number);
	typedef void (__closure *CallbackSwitchAudioSource)(std::string mod, std::string dev);	
	typedef void (__closure *CallbackSendDtmf)(const std::string& digits);
	typedef void (__closure *CallbackBlindTransfer)(const std::string& target);
	typedef Call* (__closure *CallbackGetCall)(void);
	typedef AnsiString (__closure *CallbackGetContactName)(AnsiString number);	///< get name for number/uri
	typedef int (__closure *CallbackGetStreamingState)(void);	///< get current streaming (paging) state, values as in Callback::paging_tx_state_e
	typedef void (__closure *CallbackSetTrayIcon)(const char* file);
	typedef int (__closure *CallbackGetRegistrationState)(void);///< as in Callback::reg_state_e
	typedef void (__closure *CallbackSetButtonCaption)(int id, std::string text);
	typedef void (__closure *CallbackSetButtonDown)(int id, bool state);
	typedef void (__closure *CallbackSetButtonImage)(int id, const char* file);
	typedef int (__closure *CallbackPluginSendMessageText)(const char* dllName, const char* text);
	typedef int (__closure *CallbackPluginEnable)(const char* dllName, bool state);
	typedef int (__closure *CallbackGetBlfState)(int contactId, std::string &number);
	typedef int (__closure *CallbackRecordStart)(const char* file, int channels, int side);
	// pop single DTMF character from RX queue
	typedef std::string (__closure *CallbackGetRxDtmf)(void);
	typedef void (__closure *CallbackShowTrayNotifier)(AnsiString description, AnsiString uri, bool incoming);
	typedef std::string (__closure *CallbackGetUserName)(void);
	typedef void (__closure *CallbackProgrammableButtonClick)(int id);
	typedef int (__closure *CallbackUpdateSettings)(AnsiString json);
	typedef const ButtonConf* (__closure *CallbackGetButtonConf)(int id);
	typedef void (__closure *CallbackMainMenuShow)(bool state);

	CallbackAddOutputText onAddOutputText;
	CallbackCall onCall;
	CallbackHangup onHangup;
	CallbackAnswer onAnswer;
	CallbackGetDial onGetDial;
	CallbackSetDial onSetDial;
	CallbackSwitchAudioSource onSwitchAudioSource;
	CallbackSendDtmf onSendDtmf;
	CallbackBlindTransfer onBlindTransfer;
	CallbackGetCall onGetCall;
	CallbackGetContactName onGetContactName;
	CallbackGetStreamingState onGetStreamingState;
	CallbackSetTrayIcon onSetTrayIcon;
	CallbackGetRegistrationState onGetRegistrationState;
	CallbackSetButtonCaption onSetButtonCaption;
	CallbackSetButtonDown onSetButtonDown;
	CallbackSetButtonImage onSetButtonImage;
	CallbackPluginSendMessageText onPluginSendMessageText;
	CallbackPluginEnable onPluginEnable;
	CallbackGetBlfState onGetBlfState;
	CallbackRecordStart onRecordStart;
	CallbackGetRxDtmf onGetRxDtmf;
	CallbackShowTrayNotifier onShowTrayNotifier;
	CallbackGetUserName onGetUserName;
	CallbackProgrammableButtonClick onProgrammableButtonClick;
	CallbackUpdateSettings onUpdateSettings;
	CallbackGetButtonConf onGetButtonConf;
	CallbackMainMenuShow onMainMenuShow;

	static int LuaPrint(lua_State *L);
	static int LuaError( lua_State *L );
	static int l_ShowMessage( lua_State* L );
	static int l_MessageBox(lua_State* L);
	static int l_InputQuery(lua_State* L);
	static int l_Sleep(lua_State *L);
	static int l_Beep(lua_State *L);
	static int l_CheckBreak(lua_State *L);
	static int l_GetClipboardText(lua_State* L);	
	static int l_SetClipboardText(lua_State* L);
	static int l_ForceDirectories(lua_State* L);
	static int l_FindWindowByCaptionAndExeName(lua_State* L);

	// sharing "Beep" and "MessageBox" - both in global namespace (backward compatibility) and in library
	friend int luaopen_tsip_winapi (lua_State *L);

	static int l_Call(lua_State* L);
	static int l_Hangup(lua_State* L);
	static int l_Answer(lua_State* L);
	static int l_GetDial(lua_State* L);
	static int l_SetDial(lua_State* L);
	static int l_SwitchAudioSource(lua_State* L);
	static int l_SendDtmf(lua_State* L);
	static int l_BlindTransfer(lua_State* L);
	static int l_GetCallState(lua_State* L);
	static int l_IsCallIncoming(lua_State* L);
	static int l_GetCallPeer(lua_State* L);
	static int l_GetCallInitialRxInvite(lua_State* L);
	static int l_GetContactName(lua_State* L);
	static int l_GetStreamingState(lua_State* L);
	static int l_SetVariable(lua_State* L);
	static int l_GetVariable(lua_State* L);
	static int l_QueuePush(lua_State* L);
	static int l_QueuePop(lua_State* L);
	static int l_QueueClear(lua_State* L);
	static int l_QueueGetSize(lua_State* L);
	static int l_ClearVariable(lua_State* L);
	static int l_ClearAllVariables(lua_State* L);
	static int l_GetInitialCallTarget(lua_State* L);
	static int l_GetCallCodecName(lua_State* L);
	static int l_SetInitialCallTarget(lua_State* L);
	static int l_ShellExecute(lua_State* L);
	static int l_SetTrayIcon(lua_State* L);
	static int l_GetRegistrationState(lua_State* L);
	static int l_SetButtonCaption(lua_State* L);
	static int l_SetButtonDown(lua_State* L);
	static int l_SetButtonImage(lua_State* L);
	static int l_PluginSendMessageText(lua_State* L);
	static int l_PluginEnable(lua_State* L);
	static int l_GetExecSourceType(lua_State* L);
	static int l_GetExecSourceId(lua_State* L);
	static int l_GetRecordFile(lua_State* L);
	static int l_GetBlfState(lua_State* L);
	static int l_RecordStart(lua_State* L);
	static int l_GetExeName(lua_State* L);
	static int l_GetProfileDir(lua_State* L);
	static int l_GetRecordingState(lua_State* L);
	static int l_GetRxDtmf(lua_State* L);
	static int l_ShowTrayNotifier(lua_State* L);
	static int l_GetUserName(lua_State* L);
	static int l_ProgrammableButtonClick(lua_State* L);
    static int l_RefreshAudioDevicesList(lua_State* L);
	static int l_GetAudioDevice(lua_State* L);
	static int l_UpdateSettings(lua_State* L);
	static int l_SetHandled(lua_State* L);
	static int l_GetButtonType(lua_State* L);
	static int l_GetButtonNumber(lua_State* L);
	static int l_MainMenuShow(lua_State* L);

	bool &breakReq;
	bool &handled;	///< notifying back that event handling is completed - used to skip default action
					///< when programmable button was clicked, with calling "SetHandled" froom "on programmable button" script 
	bool running;
public:
	ScriptExec(
		enum ScriptSource srcType,
		int srcId,
		bool &breakReq,
		bool &handled,
		CallbackAddOutputText onAddOutputText,
		CallbackCall onCall,
		CallbackHangup onHangup,
		CallbackAnswer onAnswer,
		CallbackGetDial onGetDial,
		CallbackSetDial onSetDial,
		CallbackSwitchAudioSource onSwitchAudioSource,
		CallbackSendDtmf onSendDtmf,
		CallbackBlindTransfer onBlindTransfer,
		CallbackGetCall onGetCall,
		CallbackGetContactName onGetContactName,
		CallbackGetStreamingState onGetStreamingState,
		CallbackSetTrayIcon onSetTrayIcon,
		CallbackGetRegistrationState onGetRegistrationState,
		CallbackSetButtonCaption onSetButtonCaption,
		CallbackSetButtonDown onSetButtonDown,
		CallbackSetButtonImage onSetButtonImage,
		CallbackPluginSendMessageText onPluginSendMessageText,
		CallbackPluginEnable onPluginEnable,
		CallbackGetBlfState onGetBlfState,
		CallbackRecordStart onRecordStart,
		CallbackGetRxDtmf onGetRxDtmf,
		CallbackShowTrayNotifier onShowTrayNotifier,
		CallbackGetUserName onGetUserName,
		CallbackProgrammableButtonClick onProgrammableButtonClick,
		CallbackUpdateSettings onUpdateSettings,
		CallbackGetButtonConf onGetButtonConf,
		CallbackMainMenuShow onMainMenuShow
		);
	~ScriptExec();
	void Run(const char* script);
	void Break(void);
	bool isRunning(void) {
		return running;
	}

	static int SetVariable(const char* name, const char* value);
	static int ClearVariable(const char* name);

	/** \brief Add value to the end of the queue; queue is created if does not exist
		\param name queue name
	*/
	static void QueuePush(const char* name, const char* value);
	/**	\brief Try to take value from the front of the queue
		\param name queue name
		\param value returned value, valid if queue exists and was not empty
		\return 0 if value was successfully taken from queue
	*/
	static int QueuePop(const char* name, AnsiString &value);
	/** \brief Delete queue
		\param name queue name
		\return 0 on success (queue existed)
	*/
	static int QueueClear(const char* name);
	/** \brief Get number of elements in queue
		\param name queue name
		\return number of elements in queue; 0 if queue does not exist
	*/
	static int QueueGetSize(const char* name);
	/** \brief Set break flag for all currently active scripts
	*/
	static void BreakAllScripts(void);
	/** \brief Try to wait while any of the script is running
	*/
	static void WaitWhileAnyRunning(unsigned int ms);
private:
	enum ScriptSource srcType;
	int srcId;
};

#endif

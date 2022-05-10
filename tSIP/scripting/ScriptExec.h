//---------------------------------------------------------------------------

#ifndef ScriptExecH
#define ScriptExecH
//---------------------------------------------------------------------------

#include "ScriptSource.h"
#include "LuaWinapi.h"
#include "baresip_dialog_info_direction.h"

#include <string>
#include <vector>
#include <System.hpp>

struct lua_State;

class ButtonConf;

struct Call;

struct Recorder;

class ScriptExec
{
private:
	friend class LuaState;

	typedef void (__closure *CallbackAddOutputText)(const char* text);
	typedef void (__closure *CallbackCall)(const char* number);
	typedef void (__closure *CallbackHangup)(int sipCode, AnsiString reason);
	typedef void (__closure *CallbackAnswer)(void);
	typedef std::string (__closure *CallbackGetDial)(void);
	typedef void (__closure *CallbackSetDial)(std::string number);
	typedef void (__closure *CallbackSwitchAudioSource)(std::string mod, std::string dev);	
	typedef void (__closure *CallbackSendDtmf)(const std::string& digits, bool runScript);
	typedef void (__closure *CallbackBlindTransfer)(const std::string& target);
	typedef Call* (__closure *CallbackGetCall)(void);
	typedef void (__closure *CallbackResetCall)(void);
	typedef Call* (__closure *CallbackGetPreviousCall)(void);
	typedef Recorder* (__closure *CallbackGetRecorder)(int id);
	typedef AnsiString (__closure *CallbackGetContactName)(AnsiString number);	///< get name for number/uri
	typedef int (__closure *CallbackGetStreamingState)(void);	///< get current streaming (paging) state, values as in Callback::paging_tx_state_e
	typedef void (__closure *CallbackSetTrayIcon)(const char* file);
	typedef int (__closure *CallbackGetRegistrationState)(void);///< as in Callback::reg_state_e
	typedef int (__closure *CallbackPluginSendMessageText)(const char* dllName, const char* text);
	typedef int (__closure *CallbackPluginEnable)(const char* dllName, bool state);
	typedef int (__closure *CallbackGetContactId)(const char* user);
	typedef int (__closure *CallbackGetBlfState)(int contactId, std::string &number, std::string &remoteIdentity, std::string &remoteIdentityDisplay, enum dialog_info_direction &direction);
	typedef int (__closure *CallbackRecordStart)(const char* file, int channels, int side, int fileFormat, unsigned int bitrate);
	// pop single DTMF character from RX queue
	typedef std::string (__closure *CallbackGetRxDtmf)(void);
	typedef void (__closure *CallbackShowTrayNotifier)(AnsiString description, AnsiString uri, bool incoming);
	typedef void (__closure *CallbackHideTrayNotifier)(void);
	typedef std::string (__closure *CallbackGetUserName)(void);
	typedef void (__closure *CallbackProgrammableButtonClick)(int id);
	typedef int (__closure *CallbackUpdateSettings)(AnsiString json);
	typedef int (__closure *CallbackUpdateButtons)(AnsiString json);
	typedef const ButtonConf* (__closure *CallbackGetButtonConf)(int id);
	typedef void (__closure *CallbackMainMenuShow)(bool state);
	typedef void (__closure *CallbackApplicationClose)(void);
	typedef unsigned int (__closure *CallbackGetAudioErrorCount)(void);

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
	CallbackResetCall onResetCall;
	CallbackGetPreviousCall onGetPreviousCall;
	CallbackGetRecorder onGetRecorder;
	CallbackGetContactName onGetContactName;
	CallbackGetStreamingState onGetStreamingState;
	CallbackGetAudioErrorCount onGetAudioErrorCount;
	CallbackSetTrayIcon onSetTrayIcon;
	CallbackGetRegistrationState onGetRegistrationState;
	CallbackPluginSendMessageText onPluginSendMessageText;
	CallbackPluginEnable onPluginEnable;
	CallbackGetContactId onGetContactId;
	CallbackGetBlfState onGetBlfState;
	CallbackRecordStart onRecordStart;
	CallbackGetRxDtmf onGetRxDtmf;
	CallbackShowTrayNotifier onShowTrayNotifier;
	CallbackHideTrayNotifier onHideTrayNotifier;
	CallbackGetUserName onGetUserName;
	CallbackProgrammableButtonClick onProgrammableButtonClick;
	CallbackUpdateSettings onUpdateSettings;
	CallbackUpdateButtons onUpdateButtons;
	CallbackGetButtonConf onGetButtonConf;
	CallbackMainMenuShow onMainMenuShow;
	CallbackApplicationClose onApplicationClose;

	friend class ScriptImp;

	static int l_Beep(lua_State *L);
	static int l_MessageBox(lua_State* L);

	// sharing "Beep" and "MessageBox" - both in global namespace (backward compatibility) and in library
	friend int luaopen_tsip_winapi (lua_State *L);

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
		CallbackResetCall onResetCall,
		CallbackGetPreviousCall onGetPreviousCall,
		CallbackGetRecorder onGetRecorder,
		CallbackGetContactName onGetContactName,
		CallbackGetStreamingState onGetStreamingState,
		CallbackGetAudioErrorCount onGetAudioErrorCount,
		CallbackSetTrayIcon onSetTrayIcon,
		CallbackGetRegistrationState onGetRegistrationState,
		CallbackPluginSendMessageText onPluginSendMessageText,
		CallbackPluginEnable onPluginEnable,
		CallbackGetContactId onGetContactId,
		CallbackGetBlfState onGetBlfState,
		CallbackRecordStart onRecordStart,
		CallbackGetRxDtmf onGetRxDtmf,
		CallbackShowTrayNotifier onShowTrayNotifier,
		CallbackHideTrayNotifier onHideTrayNotifier,
		CallbackGetUserName onGetUserName,
		CallbackProgrammableButtonClick onProgrammableButtonClick,
		CallbackUpdateSettings onUpdateSettings,
		CallbackUpdateButtons onUpdateButtons,
		CallbackGetButtonConf onGetButtonConf,
		CallbackMainMenuShow onMainMenuShow,
		CallbackApplicationClose onApplicationClose
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

	struct Symbol
	{
		AnsiString name;
		AnsiString brief;
		AnsiString description;
		bool operator<(const struct Symbol &other) const
		{
		   return name < other.name;
		}
	};

	static const std::vector<Symbol>& GetSymbols(void);
private:
	enum ScriptSource srcType;
	int srcId;
};

#endif

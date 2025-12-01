//---------------------------------------------------------------------------

#ifndef ScriptExecH
#define ScriptExecH
//---------------------------------------------------------------------------

#include "ScriptSource.h"
#include "LuaWinapi.h"
#include "baresip_dialog_info_direction.h"

#include <string>
#include <vector>
#include <map>
#include <System.hpp>

struct lua_State;

class ButtonConf;

struct Call;

struct Recorder;

class ScriptExec
{
private:
	friend class LuaState;

	typedef int (__closure *CallbackCall)(AnsiString number, unsigned int &callUid);
	typedef void (__closure *CallbackHangup)(unsigned int callUid, int sipCode, AnsiString reason);
	typedef void (__closure *CallbackAnswer)(unsigned int callUid);
	typedef void (__closure *CallbackRedial)(void);
	typedef std::string (__closure *CallbackGetDial)(void);
	typedef void (__closure *CallbackSetDial)(std::string number);
	typedef void (__closure *CallbackSendDtmf)(const std::string& digits, bool runScript);
	typedef AnsiString (__closure *CallbackGetContactName)(AnsiString number);	///< get name for number/uri
	typedef int (__closure *CallbackGetStreamingState)(void);	///< get current streaming (paging) state, values as in Callback::paging_tx_state_e
	typedef void (__closure *CallbackSetTrayIcon)(const char* file);
	typedef int (__closure *CallbackGetRegistrationState)(void);///< as in Callback::reg_state_e
	typedef int (__closure *CallbackPluginSendMessageText)(const char* dllName, const char* text);
	typedef int (__closure *CallbackPluginEnable)(const char* dllName, bool state);
	typedef int (__closure *CallbackRecordStart)(unsigned int callUid, const char* file, int channels, int side, int fileFormat, unsigned int bitrate);
	typedef void (__closure *CallbackShowTrayNotifier)(unsigned int callUid, AnsiString description, AnsiString uri, bool incoming);
	typedef void (__closure *CallbackHideTrayNotifier)(void);
	typedef int (__closure *CallbackUpdateSettings)(AnsiString json);
	typedef int (__closure *CallbackUpdateButtons)(AnsiString json);
	typedef int (__closure *CallbackUpdateButton)(unsigned int btnId, AnsiString json);
	typedef void (__closure *CallbackMainMenuShow)(bool state);
	typedef void (__closure *CallbackApplicationShow)(bool focused);
	typedef void (__closure *CallbackApplicationHide)(void);
	typedef void (__closure *CallbackApplicationClose)(void);

	CallbackCall onCall;
	CallbackHangup onHangup;
	CallbackAnswer onAnswer;
	CallbackRedial onRedial;
	CallbackGetDial onGetDial;
	CallbackSetDial onSetDial;
	CallbackSendDtmf onSendDtmf;
	CallbackGetContactName onGetContactName;
	CallbackGetStreamingState onGetStreamingState;
	CallbackSetTrayIcon onSetTrayIcon;
	CallbackGetRegistrationState onGetRegistrationState;
	CallbackPluginSendMessageText onPluginSendMessageText;
	CallbackPluginEnable onPluginEnable;
	CallbackRecordStart onRecordStart;
	CallbackShowTrayNotifier onShowTrayNotifier;
	CallbackHideTrayNotifier onHideTrayNotifier;
	CallbackUpdateSettings onUpdateSettings;
	CallbackUpdateButtons onUpdateButtons;
	CallbackUpdateButton onUpdateButton;
	CallbackMainMenuShow onMainMenuShow;
	CallbackApplicationShow onApplicationShow;
	CallbackApplicationHide onApplicationHide;
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
		CallbackCall onCall,
		CallbackHangup onHangup,
		CallbackAnswer onAnswer,
		CallbackRedial onRedial,
		CallbackGetDial onGetDial,
		CallbackSetDial onSetDial,
		CallbackSendDtmf onSendDtmf,
		CallbackGetContactName onGetContactName,
		CallbackGetStreamingState onGetStreamingState,
		CallbackSetTrayIcon onSetTrayIcon,
		CallbackGetRegistrationState onGetRegistrationState,
		CallbackPluginSendMessageText onPluginSendMessageText,
		CallbackPluginEnable onPluginEnable,
		CallbackRecordStart onRecordStart,
		CallbackShowTrayNotifier onShowTrayNotifier,
		CallbackHideTrayNotifier onHideTrayNotifier,
		CallbackUpdateSettings onUpdateSettings,
		CallbackUpdateButtons onUpdateButtons,
		CallbackUpdateButton onUpdateButton,
		CallbackMainMenuShow onMainMenuShow,
		CallbackApplicationShow onApplicationShow,
		CallbackApplicationHide onApplicationHide,
		CallbackApplicationClose onApplicationClose
		);
	~ScriptExec();
	void Run(const char* script);
	void Break(void);
	bool isRunning(void) const {
		return running;
	}

	static int SetVariable(const char* name, const char* value);
	static int ClearVariable(const char* name);
	static std::map<AnsiString, AnsiString> GetVariables(void);	

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

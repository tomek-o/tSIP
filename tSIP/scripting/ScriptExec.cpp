//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ScriptExec.h"
#include "Log.h"
#include "LogUnit.h"
#include "LuaState.h"
#include "lua.hpp"
#include "AudioDevicesList.h"
#include "Paths.h"
#include "common/TelecomUtils.h"
#include "ButtonConf.h"
#include "Call.h"
#include "Recorder.h"
#include "UaCustomRequests.h"
#include "UaMain.h"
#include "Calls.h"
#include "AppStatus.h"
#include "Settings.h"
#include "Globals.h"
#include "Contacts.h"
#include "FormContactPopup.h"
#include "SIMPLE_Messages.h"
#include "ControlQueue.h"
#include "buttons/ProgrammableButtons.h"
#include "buttons/ProgrammableButton.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include "common/Os.h"
#include <Clipbrd.hpp>
#include "CustomDateUtils.hpp"
#include <psapi.h>
#include <string>
#include <assert.h>
#include <time.h>
#include <map>
#include <deque>

#pragma link "psapi.lib"

namespace {

std::map<lua_State*, ScriptExec*> contexts;

ScriptExec* GetContext(lua_State* L)
{
	std::map<lua_State*, ScriptExec*>::iterator it;
	it = contexts.find(L);
	if (it == contexts.end())
	{
		assert(!"Unregistered lua_State!");
		return NULL;
	}
	return it->second;
}

/** \brief Mutex protecting access to variables */
Mutex mutexVariables;
/** \brief Named variables - shared by scripts and plugins */
std::map<AnsiString, AnsiString> variables;

/** \brief Mutex protecting access to queues */
Mutex mutexQueues;
/** \brief Named queues - shared by scripts and plugins */
std::map<AnsiString, std::deque<AnsiString> > queues;

long timediff(clock_t t1, clock_t t2) {
	long elapsed;
	elapsed = static_cast<long>(((double)t2 - t1) / CLOCKS_PER_SEC * 1000);
	return elapsed;
}

struct {
	HWND hWndFound;
	const char* windowName;
	const char* exeName;
} findWindowData =
	{ NULL, NULL, NULL };

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (!hWnd)
		return TRUE;		// Not a window
	if (findWindowData.windowName)
	{
		char String[512];
		if (!SendMessage(hWnd, WM_GETTEXT, sizeof(String), (LPARAM)String))
		{
			return TRUE;		// No window title (length = 0)
		}
		if (strcmp(String, findWindowData.windowName))
		{
			return TRUE;
		}
	}
	if (findWindowData.exeName)
	{
		HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		DWORD dwProcessId;
		DWORD dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);

		#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
		HANDLE hProcess;
		if (IsWinVistaOrLater())
		{
			hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, dwProcessId);
		}
		else
		{
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwProcessId);
		}
		BOOL result = TRUE;
		if (hProcess)
		{
			char exeName[512] = {0};
			if (GetProcessImageFileName(hProcess, exeName, sizeof(exeName)))
			{
				if (stricmp(exeName, findWindowData.exeName) == 0)
				{
					findWindowData.hWndFound = hWnd;
					result = FALSE;
				}
			}
			CloseHandle(hProcess);
		}
		return result;
	}
	return TRUE;
}

std::vector<ScriptExec::Symbol> symbols;
bool globalsSetComplete = false;

inline void AddSymbol(const char* name, const char* brief, const char* description)
{
	struct ScriptExec::Symbol s;
	s.name = name;
	s.brief = brief;
	s.description = description;
	symbols.push_back(s);
}

inline void lua_register2(Lua_State &L, lua_CFunction fn, const char* name, const char* brief, const char* description)
{
	if (!globalsSetComplete)
	{
		AddSymbol(name, brief, description);
	}
	lua_register(L, name, fn);
}

/** \brief Get call specified by the first arg or current call */
Call* GetCall(lua_State *L)
{
	int argCount = lua_gettop(L);
	Call *call = NULL;
	if (argCount >= 1)
	{
		unsigned int callUid = lua_tointegerx(L, 1, NULL);
		call = Calls::FindByUid(callUid);
	}
	else
	{
		call = Calls::GetCurrentCall();
	}
	return call;
}


}	// namespace

int ScriptExec::SetVariable(const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexVariables);
	variables[name] = value;
	return 0;
}

int ScriptExec::ClearVariable(const char* name)
{
	ScopedLock<Mutex> lock(mutexVariables);
	std::map<AnsiString, AnsiString>::iterator it;
	it = variables.find(name);
	if (it != variables.end())
	{
		variables.erase(it);
	}
	return 0;
}

std::map<AnsiString, AnsiString> ScriptExec::GetVariables(void)
{
	ScopedLock<Mutex> lock(mutexVariables);
	return variables;
}

void ScriptExec::QueuePush(const char* name, const char* value)
{
	ScopedLock<Mutex> lock(mutexQueues);
	std::deque<AnsiString> &que = queues[name];
	que.push_back(value);
}

int ScriptExec::QueuePop(const char* name, AnsiString &value)
{
	std::map<AnsiString, std::deque<AnsiString> >::iterator it;
	it = queues.find(name);
	if (it != queues.end())
	{
		std::deque<AnsiString> &que = it->second;
		if (que.empty())
		{
            return -2;
        }
		value = que[0];
		que.pop_front();
		return 0;
	}
	return -1;
}

int ScriptExec::QueueClear(const char* name)
{
	std::map<AnsiString, std::deque<AnsiString> >::iterator it;
	it = queues.find(name);
	if (it != queues.end())
	{
		queues.erase(it);
		return 0;
	}
	return -1;
}

int ScriptExec::QueueGetSize(const char* name)
{
	std::map<AnsiString, std::deque<AnsiString> >::iterator it;
	it = queues.find(name);
	if (it != queues.end())
	{
		return it->second.size();
	}
	return 0;
}

void ScriptExec::BreakAllScripts(void)
{
	std::map<lua_State*, ScriptExec*>::iterator it = contexts.begin();
	while (it != contexts.end()) {
		ScriptExec* se = it->second;
		se->Break();
		++it;
	}
}

void ScriptExec::WaitWhileAnyRunning(unsigned int ms)
{
	const int sleepPerPoll = 10;
	for (unsigned int i=0; i<ms/sleepPerPoll; i++) {
		std::map<lua_State*, ScriptExec*>::iterator it = contexts.begin();
		bool anyRunning = false;
		while (it != contexts.end()) {
			ScriptExec* se = it->second;
			if (se->isRunning()) {
				anyRunning = true;
				break;
			}
			++it;
		}
		if (anyRunning == false) {
			break;
		}
		Sleep(sleepPerPoll);
	}
}

const std::vector<ScriptExec::Symbol>& ScriptExec::GetSymbols(void)
{
	return symbols;
}

/** Beep(frequency, time_ms)
	\note blocking and not interruptable
*/
int ScriptExec::l_Beep(lua_State *L)
{
	/*
	To refer to elements in the stack, the API uses indices. The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on. We can also access elements using the top of the stack as our reference, using negative indices. In that case, -1 refers to the element at the top (that is, the last element pushed), -2 to the previous element, and so on. For instance, the call lua_tostring(L, -1) returns the value at the top of the stack as a string.
	*/
	int freq = lua_tointegerx(L, 1, NULL);
	int time = lua_tointegerx(L, 2, NULL);
	if (freq > 0 && time > 0)
	{
		Beep(freq, time);
    }
	return 0;
}

int ScriptExec::l_MessageBox(lua_State* L)
{
	const char* msg = lua_tostring(L, 1);
	const char* title = lua_tostring(L, 2);
	unsigned int flags = lua_tointegerx(L, 3, NULL);
	int ret = MessageBox(Application->Handle, msg, title, flags);
	lua_pushnumber(L, ret);
	return 1;
}

class ScriptImp {
public:
static int LuaPrint(lua_State *L)
{
	int nArgs = lua_gettop(L);
	int i;
	lua_getglobal(L, "tostring");
	std::string ret;//this is where we will dump the output
	//make sure you start at 1 *NOT* 0
	for(i = 1; i <= nArgs; i++)
	{
		const char *s;
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);
		if(s == NULL)
			return luaL_error(L, LUA_QL("tostring") " must return a string to ", LUA_QL("print"));

		if(i > 1) ret.append("\t");

		ret.append(s);
		lua_pop(L, 1);
	};
	//ret.append("\n");
	//GetContext(L)->onAddOutputText(ret.c_str());
	LOG("%s", ret.c_str());
	return 0;
}

static int LuaError( lua_State *L )
{
	const char* str = lua_tostring( L, -1 );
	lua_pop(L, 1);
	//send string to console
	//GetContext(L)->onAddOutputText(str);
	LOG("%s", str);	
	return 0;
}

static int l_ShowMessage( lua_State* L )
{
	AnsiString msg = lua_tostring(L, -1);
	MessageBox(NULL, msg.c_str(), "Lua message", MB_ICONINFORMATION);
	lua_pushnumber( L, 0 );
	return 1;
}

static int l_InputQuery(lua_State* L)
{
	const char* caption = lua_tostring(L, 1);
	const char* prompt = lua_tostring(L, 2);
	AnsiString text = lua_tostring(L, 3);
	bool ret = InputQuery(caption, prompt, text);
	lua_pushstring(L, text.c_str());
	lua_pushboolean(L, ret);
	return 2;
}

/** \note Sleep(ms) returns non-zero if break is called */
static int l_Sleep(lua_State* L)
{
	int ret = 0;
	int time = lua_tointegerx(L, -1, NULL);
	clock_t t1 = clock();
	long elapsed = 0;
	ScriptExec* context = GetContext(L);
	if (context->breakReq)
	{
		ret = -1;
	}
	else
	{
		while (elapsed < time)
		{
			if (context->breakReq)
			{
				ret = -1;
				break;
			}
			Application->ProcessMessages();
			Sleep(1);
			elapsed = timediff(t1, clock());
			if (context->breakReq)
			{
				ret = -1;
				break;
			}
		}
	}
	lua_pushnumber(L, ret);
	return 1;					// number of return values
}

/** \brief Check if user interrupted script for clean, controlled exit
*/
static int l_CheckBreak(lua_State *L)
{
	int ret = GetContext(L)->breakReq?1:0;
	lua_pushnumber(L, ret);
	return 1;					// number of return values
}

static int l_GetClipboardText( lua_State* L )
{
	AnsiString text = Clipboard()->AsText;
	lua_pushstring( L, text.c_str() );
	return 1;
}

static int l_SetClipboardText( lua_State* L )
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	Clipboard()->AsText = str;
	return 0;
}

/** \return 0 on success */
static int l_ForceDirectories(lua_State* L)
{
	const char* str = lua_tostring(L, -1);
	if (DirectoryExists(str))
	{
		lua_pushnumber(L, 0);
		return 1;
	}
	int ret = -1;
	if (ForceDirectories(str))
	{
		ret = 0;
	}
	lua_pushnumber(L, ret);
	return 1;
}

static int l_FindWindowByCaptionAndExeName(lua_State* L)
{
	static Mutex mutex;

	ScopedLock<Mutex> lock(mutex);
	findWindowData.hWndFound = NULL;

	findWindowData.windowName = lua_tostring(L, 1);
	const char* exeName = lua_tostring(L, 2);
	AnsiString dosExeName = "";
	if (exeName)
	{
		if (strlen(exeName) >= 2)
		{
			char targetPath[512];
			char drive[3];
			drive[0] = exeName[0];
			drive[1] = exeName[1];
			drive[2] = '\0';
			if (QueryDosDevice(drive, targetPath, sizeof(targetPath)))
			{
				dosExeName.cat_printf("%s%s", targetPath, &exeName[2]);
				findWindowData.exeName = dosExeName.c_str();
			}
		}
	}
	else
	{
		findWindowData.exeName = NULL;
	}
	if (findWindowData.windowName == NULL && findWindowData.exeName == NULL)
	{
		LOG("Lua: either window name or exe name is required\n");
		lua_pushnumber(L, 0);
		return 1;
	}

	::EnumWindows((WNDENUMPROC)EnumWindowsProc, NULL);

	lua_pushnumber(L, reinterpret_cast<unsigned int>(findWindowData.hWndFound));
	return 1;
}

static int l_Call(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	unsigned int callUid = 0;
	int ret = GetContext(L)->onCall(str, callUid);
	lua_pushnumber(L, ret);
	lua_pushnumber(L, callUid);
	return 2;
}

static int l_Hangup(lua_State* L)
{
	Call* call = Calls::GetCurrentCall();
	if (call == NULL)
	{
		return 0;
	}

	int argCount = lua_gettop(L);
	enum { DEFAULT_CODE = 486 };
	int sipCode = DEFAULT_CODE;
	if (argCount >= 1)
	{
		sipCode = lua_tointeger(L, 1);
		if (sipCode < 400 || sipCode > 699)
			sipCode = DEFAULT_CODE;
	}
	AnsiString reason = "Busy Here";
	if (argCount >= 2)
	{
		reason = lua_tostring(L, 2);
	}
	GetContext(L)->onHangup(call->uid, sipCode, reason);
	return 0;
}

static int l_Hangup2(lua_State* L)
{
	Call* call = GetCall(L);
	if (call == NULL)
	{
		return 0;
	}

	int argCount = lua_gettop(L);
	enum { DEFAULT_CODE = 486 };
	int sipCode = DEFAULT_CODE;
	if (argCount >= 2)
	{
		sipCode = lua_tointeger(L, 2);
		if (sipCode < 400 || sipCode > 699)
			sipCode = DEFAULT_CODE;
	}
	AnsiString reason = "Busy Here";
	if (argCount >= 3)
	{
		reason = lua_tostring(L, 3);
	}
	GetContext(L)->onHangup(call->uid, sipCode, reason);
	return 0;
}

static int l_HangupAll(lua_State* L)
{
	int argCount = lua_gettop(L);
	enum { DEFAULT_CODE = 486 };
	int sipCode = DEFAULT_CODE;
	if (argCount >= 1)
	{
		sipCode = lua_tointeger(L, 1);
		if (sipCode < 400 || sipCode > 699)
			sipCode = DEFAULT_CODE;
	}
	AnsiString reason = "Busy Here";
	if (argCount >= 2)
	{
		reason = lua_tostring(L, 2);
	}

	std::vector<unsigned int> uids = Calls::GetUids();
	for (unsigned int i=0; i<uids.size(); i++)
	{
		GetContext(L)->onHangup(uids[i], sipCode, reason);
	}
	return 0;
}


static int l_Answer(lua_State* L)
{
	Call* call = GetCall(L);
	if (call == NULL)
	{
		return 0;
	}
	GetContext(L)->onAnswer(call->uid);
	return 0;
}

static int l_Redial(lua_State* L)
{
	GetContext(L)->onRedial();
	return 0;
}

static int l_SetMute(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		int argCount = lua_gettop(L);
		if (argCount >= 2)
		{
			int state = lua_tointeger(L, 2);
			call->setMute(state);
		}
		else
		{
			LOG("Lua error: missing state argument for SetMute()\n");
		}
	}
	else
	{
		LOG("Lua error: call for SetMute() not found\n");
	}
	return 0;
}

static int l_ToggleMute(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		call->setMute(!call->mute);
	}
	else
	{
		LOG("Lua error: call for ToggleMute() not found\n");
	}
	return 0;
}

static int l_GetMute(lua_State* L)
{
	bool mute = false;
	Call *call = GetCall(L);
	if (call)
	{
		mute = call->mute;
	}
	else
	{
		LOG("Lua error: call for GetMute() not found\n");
	}
	lua_pushinteger(L, mute);	
	return 1;
}

static int l_SetHold(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		int argCount = lua_gettop(L);
		if (argCount >= 2)
		{
			int state = lua_tointeger(L, 2);
			call->setHold(state);
		}
		else
		{
			LOG("Lua error: missing state argument for SetHold()\n");  
		}
	}
	else
	{
		LOG("Lua error: call for SetHold() not found\n");
	}
	return 0;
}

static int l_ToggleHold(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		call->setHold(!call->hold);
	}
	else
	{
		LOG("Lua error: call for ToggleHold() not found\n");
	}
	return 0;
}

static int l_GetHold(lua_State* L)
{
	bool hold = false;
	Call *call = GetCall(L);
	if (call)
	{
		hold = call->hold;
	}
	else
	{
		LOG("Lua error: call for GetHold() not found\n");
	}
	lua_pushinteger(L, hold);	
	return 1;
}

static int l_SetDial(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	GetContext(L)->onSetDial(str);
	return 0;
}

static int l_GetDial(lua_State* L)
{
	std::string num = GetContext(L)->onGetDial();
	lua_pushstring( L, num.c_str() );
	return 1;
}

static int l_SetInitialCallTarget(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	Call* call = Calls::GetCurrentCall();
	if (call)
	{
		call->initialTarget = str;
	}
	else
	{
		LOG("Lua error: no call for SetInitialCallTarget\n");
	}
	return 0;
}

static int l_SetCallTarget(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	Call* call = Calls::GetCurrentCall();
	if (call)
	{
        call->uri = str;
		call->initialTarget = str;
	}
	else
	{
		LOG("Lua error: no call for SetCallTarget\n");
	}
	return 0;
}

static int l_GetInitialCallTarget(lua_State* L)
{
	Call* call = Calls::GetCurrentCall();
	if (call)
	{
		std::string num = call->initialTarget.c_str();
		lua_pushstring( L, num.c_str() );
		return 1;
	}
	return 0;
}

static int l_ResetCall(lua_State* L)
{
	Call* call = Calls::GetCurrentCall();
	if (call)
		call->reset();
	return 0;
}

static int l_GetPreviousCallStatusCode(lua_State* L)
{
	Call &call = Calls::GetPreviousCall();
	lua_pushinteger( L, call.lastScode );
	return 1;
}

static int l_GetPreviousCallReplyLine(lua_State* L)
{
	Call &call = Calls::GetPreviousCall();
	lua_pushstring( L, call.lastReplyLine.c_str() );
	return 1;
}

static int l_SwitchAudioSource(lua_State* L)
{
	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	const char* mod = lua_tostring( L, 1 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchAudioSource mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 2 );
	if (dev == NULL)
	{
		LOG("Lua error: SwitchAudioSource dev == NULL\n");
		return 0;
	}
	unsigned int uid = Calls::GetCurrentCallUid();
	UA->SwitchAudioSource(uid, mod, dev);
	return 0;
}

static int l_SwitchAudioSource2(lua_State* L)
{
	unsigned int callUid;

	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	int argCount = lua_gettop(L);
	if (argCount >= 1)
	{
		callUid = lua_tointegerx(L, 1, NULL);
	}
	else
	{
		LOG("Lua error: SwitchAudioSource2 missing call UID\n");
		return 0;
	}
	const char* mod = lua_tostring( L, 2 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchAudioSource2 mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 3 );
	if (dev == NULL)
	{
		LOG("Lua error: SwitchAudioSource2 dev == NULL\n");
		return 0;
	}
	UA->SwitchAudioSource(callUid, mod, dev);
	return 0;
}

static int l_SwitchAudioSourceToConfigured(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		UA->SwitchAudioSource(call->uid, appSettings.uaConf.audioCfgSrc.mod.c_str(), appSettings.uaConf.audioCfgSrc.dev.c_str());
	}
	else
	{
		LOG("Lua: SwitchAudioSourceToConfigured: call not found\n");
	}
	return 0;
}

static int l_SwitchAudioPlayer(lua_State* L)
{
	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	const char* mod = lua_tostring( L, 1 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchAudioPlayer mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 2 );
	if (dev == NULL)
	{
		LOG("Lua error: SwitchAudioPlayer dev == NULL\n");
		return 0;
	}
	unsigned int uid = Calls::GetCurrentCallUid();
	UA->SwitchAudioPlayer(uid, mod, dev);
	return 0;
}

static int l_SwitchAudioPlayer2(lua_State* L)
{
	unsigned int callUid;

	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	int argCount = lua_gettop(L);
	if (argCount >= 1)
	{
		callUid = lua_tointegerx(L, 1, NULL);
	}
	else
	{
		LOG("Lua error: SwitchAudioPlayer2 missing call UID\n");
		return 0;
	}
	const char* mod = lua_tostring( L, 2 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchAudioPlayer2 mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 3 );
	if (dev == NULL)
	{
		LOG("Lua error: SwitchAudioPlayer2 dev == NULL\n");
		return 0;
	}
	UA->SwitchAudioPlayer(callUid, mod, dev);
	return 0;
}

static int l_SwitchVideoSource(lua_State* L)
{
	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	const char* mod = lua_tostring( L, 1 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchVideoSource mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 2 );
	if (dev == NULL)
	{
		LOG("Lua error: SwitchVideoSource dev == NULL\n");
		return 0;
	}
	unsigned int uid = Calls::GetCurrentCallUid();
	UA->SwitchVideoSource(uid, mod, dev);
	return 0;
}

static int l_SwitchVideoSource2(lua_State* L)
{
	unsigned int callUid;

	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	int argCount = lua_gettop(L);
	if (argCount >= 1)
	{
		callUid = lua_tointegerx(L, 1, NULL);
	}
	else
	{
		LOG("Lua error: SwitchVideoSource2 missing call UID\n");
		return 0;
	}
	const char* mod = lua_tostring( L, 2 );
	if (mod == NULL)
	{
		LOG("Lua error: SwitchVideoSource2 mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 3 );
	if (dev == NULL)
	{
		LOG("Lua error: dev == NULL\n");
		return 0;
	}
	UA->SwitchVideoSource(callUid, mod, dev);
	return 0;
}

static int l_SendDtmf(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	GetContext(L)->onSendDtmf(str, false);
	return 0;
}

static int l_GenerateTones(lua_State* L)
{
	unsigned int uid = Calls::GetCurrentCallUid();
	UA->GenerateTone(uid,
		lua_tonumber(L, 1), lua_tonumber(L, 2),	// amplitude, frequency
		lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5), lua_tonumber(L, 6),
		lua_tonumber(L, 7), lua_tonumber(L, 8)
	);
	return 0;
}

static int l_GenerateTones2(lua_State* L)
{
	unsigned int callUid;

	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	int argCount = lua_gettop(L);
	if (argCount >= 1)
	{
		callUid = lua_tointegerx(L, 1, NULL);
	}
	else
	{
		LOG("Lua error: missing call UID\n");
		return 0;
	}
	UA->GenerateTone(callUid,
		lua_tonumber(L, 1), lua_tonumber(L, 2),	// amplitude, frequency
		lua_tonumber(L, 3), lua_tonumber(L, 4),
		lua_tonumber(L, 5), lua_tonumber(L, 6),
		lua_tonumber(L, 7), lua_tonumber(L, 8)
	);
	return 0;
}

static int l_BlindTransfer(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
        LOG("Lua BlindTransfer error: str == NULL\n");
		return 0;
	}
	unsigned int uid = Calls::GetCurrentCallUid();
	UA->Transfer(uid, str);
	return 0;
}

static int l_BlindTransfer2(lua_State* L)
{
	int argCount = lua_gettop(L);
	if (argCount >= 2)
	{
		unsigned int callUid = lua_tointegerx(L, 1, NULL);
		const char* str = lua_tostring( L, 2 );
		if (str == NULL)
		{
			LOG("Lua BlindTransfer error: str == NULL\n");
			return 0;
		}
		UA->Transfer(callUid, str);
	}
	else
	{
		LOG("Lua BlindTransfer2 requires callUid and transfer target!\n");
	}
	return 0;
}

static int l_AttendedTransfer(lua_State* L)
{
	int argCount = lua_gettop(L);
	if (argCount >= 2)
	{
		unsigned int callUid1 = lua_tointegerx(L, 1, NULL);
		unsigned int callUid2 = lua_tointegerx(L, 2, NULL);
		UA->TransferReplace(callUid1, callUid2);
	}
	else
	{
		LOG("Lua AttendedTransfer requires two call UIDs!\n");
	}
	return 0;
}

static int l_GetCalls(lua_State* L)
{
	std::vector<unsigned int> uids = Calls::GetUids();
	// returns table
	lua_newtable(L);
	int top = lua_gettop(L);

	for (unsigned int i = 0; i < uids.size(); i++)
	{
		lua_pushnumber(L, i+1);		// push the index, starting from 1 in Lua
		lua_pushnumber(L, uids[i]); // push the value at 'i'
		lua_settable(L, top);
	}
	return 1;
}

static int l_GetCallUidFromLineButton(lua_State* L)
{
	int argCount = lua_gettop(L);
	if (argCount == 1)
	{
		unsigned int buttonId = lua_tointegerx(L, 1, NULL);
		const std::map<unsigned int, Call> calls = Calls::GetCalls();
		for (std::map<unsigned int, Call>::const_iterator iter = calls.begin(); iter != calls.end(); ++iter)
		{
			const Call &call = iter->second;
			if (call.btnId == buttonId)
			{
				lua_pushnumber(L, call.uid);
				return 1;
			}
		}
		lua_pushnumber(L, Call::INVALID_UID);
		return 1;
	}
	else
	{
		LOG("Lua error: missing button id parameter\n");
		return 0;		
	}
}

static int l_GetCurrentCallUid(lua_State* L)
{
	unsigned int uid = Calls::GetCurrentCallUid();
	lua_pushnumber(L, uid);
	return 1;
}

static int l_SetCurrentCallUid(lua_State* L)
{
	int argCount = lua_gettop(L);
	if (argCount >= 1)
	{
		unsigned int callUid = lua_tointegerx(L, 1, NULL);
		int status = Calls::SetCurrentCallUid(callUid);
		lua_pushnumber(L, status);
		return 1;
	}
	lua_pushnumber(L, -1);
	return 1;
}

static int l_GetCallState(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		lua_pushinteger( L, call->GetState() );
	}
	else
	{
		lua_pushinteger( L, Callback::CALL_STATE_CLOSED );
	}
	return 1;
}

static int l_GetCallButtonId(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		lua_pushinteger( L, call->btnId );
		return 1;
	}
	return 0;
}

static int l_GetRecorderState(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		lua_pushinteger( L, call->recorder.state );
	}
	else
	{
		lua_pushinteger( L, Callback::RECORDER_STATE_IDLE );
	}
	return 1;
}

static int l_GetZrtpState(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		const Call::Zrtp &zrtp = call->zrtp;
		lua_pushinteger(L, zrtp.sessionId);
		lua_pushinteger(L, zrtp.active);
		lua_pushstring(L, zrtp.sas.c_str());
		lua_pushstring(L, zrtp.cipher.c_str());
		lua_pushinteger(L, zrtp.verified);
		return 5;
	}
	return 0;
}

static int l_IsCallIncoming(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushinteger( L, call->incoming );
		return 1;
	}
	return 0;
}

static int l_GetCallPeer(lua_State* L)
{
	Call* call = GetCall(L);
	if (call == NULL)
		return 0;

	AnsiString uri;
	if (call->incoming)
	{
		uri = ExtractNumberFromUri(call->uri.c_str());
	}
	else
	{
		uri = call->uri;
	}
	lua_pushstring( L, uri.c_str() );
	return 1;
}

static int l_GetCallPeerName(lua_State* L)
{
	Call* call = GetCall(L);
	if (call == NULL)
		return 0;

	AnsiString peerName = call->getPeerName();
	lua_pushstring( L, peerName.c_str() );
	return 1;
}

static int l_GetCallInitialRxInvite(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushstring( L, call->initialRxInvite.c_str() );
		return 1;
	}
	return 0;
}

static int l_GetCallCodecName(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushstring( L, call->codecName.c_str() );
		return 1;
	}
	return 0;
}

static int l_GetCallReason(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushstring( L, call->sipReason.c_str() );
		return 1;
	}
	return 0;
}

static int l_GetCallDurationFromStart(lua_State* L)
{
	Call *call = GetCall(L);
	if (call)
	{
		int milliseconds = static_cast<int>(MilliSecondsBetween(Now(), call->timestamp));
		lua_pushinteger( L, milliseconds );
		return 1;
	}
	return 0;
}

static int l_GetCallDurationFromConfirmed(lua_State* L)
{
	Call *call = GetCall(L);
	if (call && call->connected)
	{
		int milliseconds = static_cast<int>(MilliSecondsBetween(Now(), call->timeTalkStart));
		lua_pushinteger( L, milliseconds );
		return 1;
	}
	return 0;
}

static int l_GetContactName(lua_State* L)
{
	const char* number = lua_tostring( L, -1 );
	if (number == NULL)
	{
		LOG("Lua GetContactName error: number == NULL\n");
		return 0;
	}
	AnsiString desc = GetContext(L)->onGetContactName(number);
	lua_pushstring( L, desc.c_str() );
	return 1;
}

static int l_GetStreamingState(lua_State* L)
{
	int state = GetContext(L)->onGetStreamingState();
	lua_pushinteger( L, state );
	return 1;
}

static int l_GetAudioErrorCount(lua_State* L)
{
	Call* call = GetCall(L);
	unsigned int count = 0;
	if (call)
	{
		count = call->audioErrorCount;
	}
	lua_pushinteger( L, count );
	return 1;
}

static int l_GetRegistrationState(lua_State* L)
{
	int state = GetContext(L)->onGetRegistrationState();
	lua_pushinteger( L, state );
	return 1;
}

static int l_Unregister(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	UA->UnRegister(id);
	return 0;
}

static int l_Reregister(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	UA->ReRegister(id);
	return 0;
}

static int l_SetButtonCaption(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	if (str == NULL)
		str = "";
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetCaption(str);
	}
	return 0;
}

static int l_SetButtonCaption2(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	if (str == NULL)
		str = "";
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetCaption2(str);
	}
	return 0;
}

static int l_SetButtonDown(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	int state = lua_tointeger( L, 2 );
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetDown(state);
	}
	return 0;
}

static int l_GetButtonDown(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	bool down = false;
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		down = btn->GetDown();
	}
	lua_pushinteger( L, down?1:0 );
	return 1;
}

static int l_GetButtonMouseDown(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	bool down = false;
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		down = btn->GetMouseDown();
	}
	lua_pushinteger( L, down?1:0 );
	return 1;
}

static int l_GetButtonBlfState(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		enum dialog_info_status state = btn->GetState();
		lua_pushinteger( L, state );
		return 1;
	}
	return 0;
}

static int l_GetButtonHandle(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	if (str == NULL)
		str = "";
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		lua_pushinteger(L, reinterpret_cast<unsigned int>(btn->Handle));
		return 1;
	}
	return 0;
}

static int l_SetButtonInactive(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	int state = lua_tointeger( L, 2 );
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetInactive(state);
	}
	return 0;
}

static int l_SetButtonVisible(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	int state = lua_tointeger( L, 2 );
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetVisible(state);
	}
	return 0;
}

static int l_SetButtonImage(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	TProgrammableButton *btn = buttons.GetBtn(id);
	if (btn)
	{
		btn->SetImage(str);
	}
	return 0;
}

static int l_ProgrammableButtonClick(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	TProgrammableButton* btn = buttons.GetBtn(id);
	if (btn)
		btn->OnClick(btn);
	return 0;
}

static int l_PluginSendMessageText(lua_State* L)
{
	const char* dllName = lua_tostring(L, 1);
	if (dllName == NULL)
	{
		lua_pushinteger( L, -1 );
		return 1;
	}
	const char* buffer = lua_tostring(L, 2);
	if (buffer == NULL)
	{
		lua_pushinteger( L, -1 );
		return 1;
	}
	GetContext(L)->onPluginSendMessageText(dllName, buffer);

	lua_pushinteger( L, 0 );
	return 1;
}

static int l_PluginEnable(lua_State* L)
{
	const char* dllName = lua_tostring(L, 1);
	if (dllName == NULL)
	{
		lua_pushinteger( L, -1 );
		return 1;
	}
	int state = lua_tointeger(L, 2);
	int ret = GetContext(L)->onPluginEnable(dllName, state);

	lua_pushinteger( L, ret );
	return 1;
}

static int l_SetVariable(lua_State* L)
{
	const char* name = lua_tostring( L, 1 );
	if (name == NULL)
	{
		LOG("Lua error: name == NULL\n");
		return 0;
	}
	const char* value = lua_tostring( L, 2 );
	if (value == NULL)
	{
		LOG("Lua error: value == NULL\n");
		return 0;
	}
	ScriptExec::SetVariable(name, value);
	return 0;
}

static int l_GetVariable(lua_State* L)
{
	const char* name = lua_tostring( L, 1 );
	if (name == NULL)
	{
		LOG("Lua error: name == NULL\n");
		return 0;
	}
	ScopedLock<Mutex> lock(mutexVariables);
	std::map<AnsiString, AnsiString>::iterator it;
	it = variables.find(name);
	if (it != variables.end())
	{
		// returning two variables, second one informs if variable is found
		lua_pushstring( L, it->second.c_str() );
		lua_pushinteger( L, 1 );
	}
	else
	{
        // void lua_pushnil (lua_State *L);
		lua_pushstring( L, "" );
		lua_pushinteger( L, 0 );
	}
	return 2;
}

static int l_ClearVariable(lua_State* L)
{
	const char* name = lua_tostring( L, 1 );
	if (name == NULL)
	{
		LOG("Lua error: name == NULL\n");
		return 0;
	}	
	ScriptExec::ClearVariable(name);
	return 0;
}

static int l_ClearAllVariables(lua_State* L)
{
	variables.clear();
	return 0;
}

static int l_QueuePush(lua_State* L)
{
	int argCnt = lua_gettop(L);
	if (argCnt < 2)
	{
		LOG("Lua error: missing argument(s) for QueuePush() - 2 arguments required, %d received\n", argCnt);
		return 0;
	}
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		return 0;
	}
	const char* value = lua_tostring( L, 2 );
	if (value == NULL)
	{
		LOG("Lua error: value == NULL\n");
		return 0;
	}
	ScriptExec::QueuePush(queue_name, value);
	return 0;
}

static int l_QueuePop(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		return 0;
	}
	AnsiString value;
	int ret = ScriptExec::QueuePop(queue_name, value);
	lua_pushstring(L, value.c_str());
	if (ret != 0)
	{
		lua_pushinteger(L, 0);
	}
	else
	{
		lua_pushinteger(L, 1);	// "valid"
	}
	return 2;
}

static int l_QueueClear(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		lua_pushinteger(L, -1);
		return 1;
	}
	int ret = ScriptExec::QueueClear(queue_name);
	lua_pushinteger(L, ret);
	return 1;
}

static int l_QueueGetSize(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		lua_pushinteger(L, 0);
		return 1;
	}
	int ret = ScriptExec::QueueGetSize(queue_name);
	lua_pushinteger(L, ret);
	return 1;
}

static int l_ShellExecute(lua_State* L)
{
	const char* verb = lua_tostring( L, 1 );
	const char* file = lua_tostring( L, 2 );
	const char* parameters = lua_tostring( L, 3 );
	const char* directory = lua_tostring( L, 4 );
	int showCmd = lua_tointeger( L, 5 );
	int ret = reinterpret_cast<int>(ShellExecute(NULL, verb, file, parameters, directory, showCmd));
	lua_pushinteger(L, ret);
	return 1;
}

static int l_SetTrayIcon(lua_State* L)
{
	const char* file = lua_tostring( L, 1 );
	GetContext(L)->onSetTrayIcon(file);
	return 0;
}

static int l_GetExecSourceType(lua_State* L)
{
	lua_pushinteger(L, GetContext(L)->srcType);
	return 1;
}

static int l_GetExecSourceId(lua_State* L)
{
	lua_pushinteger(L, GetContext(L)->srcId);
	return 1;
}

static int l_GetRecordFile(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushstring( L, call->recordFile.c_str() );
		return 1;
	}
	return 0;
}

static int l_GetContactId(lua_State* L)
{
	const char* str = lua_tostring( L, 1 );
	if (str == NULL)
		return 0;

	int id = -1;
	for (unsigned int i=0; i<appSettings.uaConf.contacts.size(); i++)
	{
		if (appSettings.uaConf.contacts[i].user == str)
		{
			id = i;
			break;
		}
	}
	lua_pushinteger( L, id );
	return 1;
}

static int l_GetBlfState(lua_State* L)
{
	int contactId = lua_tointeger( L, 1 );
	if (contactId < 0 || contactId >= appSettings.uaConf.contacts.size())
	{
		LOG("GetBlfState: invalid contactId = %d\n", contactId);
		return 0;
	}

	const UaConf::Contact &contact = appSettings.uaConf.contacts[contactId];
	lua_pushstring( L, contact.user.c_str() );
	lua_pushinteger( L, contact.dialog_info_state );
	lua_pushstring( L, contact.remoteIdentity.c_str() );
	lua_pushstring( L, contact.remoteIdentityDisplay.c_str() );
	lua_pushinteger( L, contact.direction );
	return 5;
}

static int l_RecordStart(lua_State* L)
{
	const char* file = lua_tostring( L, 1 );
	int channels = lua_tointeger( L, 2 );
	int side = lua_tointeger( L, 3 );		// optional, introduced in tSIP 0.1.66
	int fileFormat = lua_tointeger( L, 4 );	// optional, introduced in tSIP 0.2.9
	unsigned int bitrate = lua_tointeger( L, 5 );
	if (bitrate <= 0)
		bitrate = 64000;

	int argCount = lua_gettop(L);
	Call *call = NULL;
	if (argCount >= 6)
	{
		unsigned int callUid = lua_tointegerx(L, 6, NULL);
		call = Calls::FindByUid(callUid);
	}
	else
	{
		call = Calls::GetCurrentCall();
	}
	if (call == NULL)
	{
		lua_pushinteger(L, -1);
		return 1;
	}

	int ret = GetContext(L)->onRecordStart(call->uid, file, channels, side, fileFormat, bitrate);
	lua_pushinteger(L, ret);
	return 1;
}

static int l_GetRecordingState(lua_State* L)
{
	Call* call = GetCall(L);
	if (call)
	{
		lua_pushinteger( L, call->recording );
		return 1;
	}
	return 0;
}

static int l_GetRxDtmf(lua_State* L)
{
	Call* call = GetCall(L);
	std::string num = "";
	if (call != NULL)
	{
		std::deque<char> &dtmfRxQueue = call->dtmfRxQueue;
		if (!dtmfRxQueue.empty())
		{
			num = std::string(1, dtmfRxQueue[0]);
			dtmfRxQueue.pop_front();
		}
	}
	lua_pushstring( L, num.c_str() );
	return 1;
}

static int l_ShowTrayNotifier(lua_State* L)
{
	const char* description = lua_tostring(L, 1);
	if (description == NULL)
	{
		lua_pushinteger( L, -1 );
		return 1;
	}
	const char* uri = lua_tostring(L, 2);
	if (uri == NULL)
	{
		lua_pushinteger( L, -1 );
		return 1;
	}
	int incoming = lua_tointeger(L, 3);

	unsigned int callUid = lua_tointegerx(L, 4, NULL);

	GetContext(L)->onShowTrayNotifier(callUid, description, uri, incoming);

	lua_pushinteger(L, 0);
	return 1;
}

static int l_HideTrayNotifier(lua_State* L)
{
	GetContext(L)->onHideTrayNotifier();
	return 0;
}

static int l_GetUserName(lua_State* L)
{
	std::string user;
	if (!appSettings.uaConf.accounts.empty())
	{
		user = appSettings.uaConf.accounts[0].user.c_str();
	}
	lua_pushstring(L, user.c_str());
	return 1;
}

static int l_GetExeName(lua_State* L)
{
	lua_pushstring(L, Application->ExeName.c_str());
	return 1;
}

static int l_GetProfileDir(lua_State* L)
{
	lua_pushstring(L, Paths::GetProfileDir().c_str());
	return 1;
}

static int l_RefreshAudioDevicesList(lua_State* L)
{
	AudioDevicesList::Instance().Refresh();
	return 0;
}

static int l_GetAudioDevice(lua_State* L)
{
	const char* module = lua_tostring(L, 1);
	if (module == NULL)
	{
		LOG("Lua error: audio module name == NULL\n");
		lua_pushstring(L, "");
		lua_pushinteger(L, 0);	// "valid"
		return 2;
	}
	const char* dir = lua_tostring(L, 2);
	bool out = false;
	if (dir == NULL)
	{
		LOG("Lua error: direction for audio module == NULL (expecting \"in\" or \"out\"\n");
		lua_pushstring(L, "");
		lua_pushinteger(L, 0);	// "valid"
		return 2;
	}
	else
	{
		if (strcmp(dir, "in") == 0)
		{
			out = false;
		}
		else if (strcmp(dir, "out") == 0)
		{
			out = true;
		}
		else
		{
			LOG("Lua error: unexpected direction for audio module (expecting \"in\" or \"out\"\n");
			lua_pushstring(L, "");
			lua_pushinteger(L, 0);	// "valid"
			return 2;
		}
	}

	if (!AudioDevicesList::Instance().IsFilled())
		AudioDevicesList::Instance().Refresh();	

	std::vector<AnsiString> *v = NULL;
	if (strcmp(module, AudioModules::portaudio) == 0)
	{
		if (out)
			v = &AudioDevicesList::Instance().portaudioDevsOut;
		else
			v = &AudioDevicesList::Instance().portaudioDevsIn;
	}
	else if (strcmp(module, AudioModules::winwave) == 0 || strcmp(module, AudioModules::winwave2) == 0)
	{
		if (out)
			v = &AudioDevicesList::Instance().winwaveDevsOut;
		else
			v = &AudioDevicesList::Instance().winwaveDevsIn;
	}
	else
	{
		LOG("Lua error: unhandled audio module name\n");
		lua_pushstring(L, "");
		lua_pushinteger(L, 0);	// "valid"
		return 2;
	}
	int devId = lua_tointeger(L, 3);
	if (devId < 0 || devId >= v->size())
	{
		lua_pushstring(L, "");
		lua_pushinteger(L, 0);	// "valid"
		return 2;
	}
	lua_pushstring(L, (*v)[devId].c_str());
	lua_pushinteger(L, 1);	// "valid"
	return 2;
}

static int l_GetAudioDevicesList(lua_State* L)
{
	const char* module = lua_tostring(L, 1);
	if (module == NULL)
	{
		LOG("Lua error: audio module name == NULL\n");
		return 0;
	}
	const char* dir = lua_tostring(L, 2);
	bool out = false;
	if (dir == NULL)
	{
		LOG("Lua error: direction for audio module == NULL (expecting \"in\" or \"out\"\n");
		return 0;
	}
	else
	{
		if (strcmp(dir, "in") == 0)
		{
			out = false;
		}
		else if (strcmp(dir, "out") == 0)
		{
			out = true;
		}
		else
		{
			LOG("Lua error: unexpected direction for audio module (expecting \"in\" or \"out\"\n");
			return 0;
		}
	}

	if (!AudioDevicesList::Instance().IsFilled())
		AudioDevicesList::Instance().Refresh();

	std::vector<AnsiString> *v = NULL;
	if (strcmp(module, AudioModules::portaudio) == 0)
	{
		if (out)
			v = &AudioDevicesList::Instance().portaudioDevsOut;
		else
			v = &AudioDevicesList::Instance().portaudioDevsIn;
	}
	else if (strcmp(module, AudioModules::winwave) == 0 || strcmp(module, AudioModules::winwave2) == 0)
	{
		if (out)
			v = &AudioDevicesList::Instance().winwaveDevsOut;
		else
			v = &AudioDevicesList::Instance().winwaveDevsIn;
	}
	else
	{
		LOG("Lua error: unhandled audio module name\n");
		return 0;
	}

	// returns table
	lua_newtable(L);
	int top = lua_gettop(L);

	for (unsigned int i=0; i<v->size(); i++)
	{
		lua_pushnumber(L, i+1);		// push the index, starting from 1 in Lua
		lua_pushstring(L, (*v)[i].c_str()); // push the value at 'i'
		lua_settable(L, top);
	}
	return 1;
}

static int l_UpdateSettings(lua_State* L)
{
	const char* json = lua_tostring(L, 1);
	if (json == NULL)
	{
		LOG("Lua error: missing parameter (json)\n");
		return 0;
	}
	int status = GetContext(L)->onUpdateSettings(json);
	lua_pushinteger(L, status);
	return 1;
}

static int l_UpdateButtons(lua_State* L)
{
	const char* json = lua_tostring(L, 1);
	if (json == NULL)
	{
		LOG("Lua error: missing parameter (json)\n");
		return 0;
	}
	int status = GetContext(L)->onUpdateButtons(json);
	lua_pushinteger(L, status);
	return 1;
}

static int l_UpdateButton(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	if (id < 0 || id >= buttons.btnConf.size())
	{
		LOG("Lua error: invalid button id = %d\n", id);
		return 0;
	}

	const char* json = lua_tostring(L, 2);
	if (json == NULL)
	{
		LOG("Lua error: missing parameter (json)\n");
		return 0;
	}
	int status = GetContext(L)->onUpdateButton(id, json);
	lua_pushinteger(L, status);
	return 1;
}

static int l_SetHandled(lua_State* L)
{
	bool handled = lua_tointeger( L, 1 );
	GetContext(L)->handled = handled;	
	return 0;
}

static int l_GetButtonType(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	if (id < 0 || id >= buttons.btnConf.size())
	{
		LOG("Lua error: no button configuration for id = %d\n", id);
		return 0;
	}
	const ButtonConf& btnConf = buttons.btnConf[id];
	lua_pushinteger(L, btnConf.type);
	return 1;
}

static int l_GetButtonNumber(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	if (id < 0 || id >= buttons.btnConf.size())
	{
		LOG("Lua error: no button configuration for id = %d\n", id);
		return 0;
	}
	const ButtonConf& btnConf = buttons.btnConf[id];
	lua_pushstring(L, btnConf.number.c_str());
	return 1;
}

static int l_MainMenuShow(lua_State* L)
{
	bool state = lua_tointeger( L, 1 );
	GetContext(L)->onMainMenuShow(state);
	return 0;
}

static int l_ApplicationShow(lua_State* L)
{
	bool focused = lua_tointeger( L, 1 );
	GetContext(L)->onApplicationShow(focused);
	return 0;
}

static int l_ApplicationHide(lua_State* L)
{
	GetContext(L)->onApplicationHide();
	return 0;
}

static int l_ApplicationClose(lua_State* L)
{
	GetContext(L)->onApplicationClose();
	return 0;
}

static int l_SendCustomRequest(lua_State* L)
{
	const char* uri = lua_tostring(L, 1);
	if (uri == NULL)
	{
		LOG("Lua error: uri == NULL for SendCustomRequest()\n");
		return 0;
	}
	const char* method = lua_tostring(L, 2);
	if (method == NULL)
	{
		LOG("Lua error: method == NULL for SendCustomRequest()\n");
		return 0;
	}
	AnsiString extraHeaderLines;
	const char* extraHeaderLinesStr = lua_tostring(L, 3);
	if (extraHeaderLinesStr != NULL)
	{
		extraHeaderLines = extraHeaderLinesStr;
	}

	int uid = -1;
	int status = UaCustomRequests::Send(uid, uri, method, extraHeaderLines);
	if (status != 0)
		uid = -1;
	lua_pushinteger(L, uid);
	return 1;
}

static int l_ClearCustomRequests(lua_State* L)
{
	UaCustomRequests::Clear();
	return 0;
}

static int l_DeleteCustomRequest(lua_State* L)
{
	int uid = lua_tointeger( L, 1 );
	UaCustomRequests::DeleteRequest(uid);
	return 0;
}

static int l_GetCustomRequest(lua_State* L)
{
	int uid = lua_tointeger(L, 1);
	struct UaCustomRequests::Request* request = UaCustomRequests::GetRequest(uid);
	if (request == NULL)
		return 0;
	lua_pushstring(L, request->uri.c_str());
	lua_pushstring(L, request->method.c_str());
	lua_pushstring(L, request->extraHeaderLines.c_str());
	return 3;
}

static int l_GetCustomRequestReply(lua_State* L)
{
	int uid = lua_tointeger(L, 1);
	struct UaCustomRequests::Request* request = UaCustomRequests::GetRequest(uid);
	if (request == NULL)
		return 0;
	lua_pushinteger(L, request->haveReply?1:0);
	lua_pushinteger(L, request->error);
	lua_pushinteger(L, request->sipStatusCode);
	return 3;
}

static int l_GetCustomRequestReplyText(lua_State* L)
{
	int uid = lua_tointeger(L, 1);
	struct UaCustomRequests::Request* request = UaCustomRequests::GetRequest(uid);
	if (request == NULL)
		return 0;
	lua_pushstring(L, request->replyText.c_str());
	return 1;
}

static int l_GetAudioRxSignalLevel(lua_State* L)
{
	unsigned int level = Ua::Instance().GetAudioRxSignalLevel();
	lua_pushinteger(L, level);
	return 1;
}

static int l_ReadContacts(lua_State* L)
{
	contacts.Read();
	return 0;
}

static int l_ReadXmlContacts(lua_State* L)
{
	const char* filename = lua_tostring(L, 1);
	int status = contacts.ReadXml(filename);
	if (status == 0)
	{
		contacts.Write();
	}
	lua_pushinteger(L, status);
	return 1;
}

static int l_AppendContactNoteText(lua_State* L)
{
	const char* text = lua_tostring(L, 1);
	if (frmContactPopup)
	{
		frmContactPopup->AppendNoteText(text);
	}
	return 0;
}

static int l_SendTextMessage(lua_State* L)
{
	const char* target = lua_tostring( L, 1 );
	const char* text = lua_tostring( L, 2 );
	int sendImmediately = lua_tointeger(L, 3);
	SIMPLE_Messages::Send(target, text, sendImmediately);
	return 0;
}

static int l_SetAppStatus(lua_State* L)
{
	const char* id = lua_tostring( L, 1 );
	int priority = lua_tointeger(L, 2);
	const char* text = lua_tostring( L, 3 );
	SetAppStatus(id, priority, text);
	return 0;
}

static int l_ShowLogWindow(lua_State* L)
{
	if (!frmLog->Visible)
		frmLog->Show();
	frmLog->WindowState = wsNormal;
	frmLog->BringToFront();
	return 0;
}

};	// ScriptImp


ScriptExec::ScriptExec(
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
	):
	srcType(srcType),
	srcId(srcId),
	breakReq(breakReq),
	handled(handled),
	onCall(onCall),
	onHangup(onHangup),
	onAnswer(onAnswer),
	onRedial(onRedial),
	onGetDial(onGetDial),
	onSetDial(onSetDial),
	onSendDtmf(onSendDtmf),
	onGetContactName(onGetContactName),
	onGetStreamingState(onGetStreamingState),
	onSetTrayIcon(onSetTrayIcon),
	onGetRegistrationState(onGetRegistrationState),
	onPluginSendMessageText(onPluginSendMessageText),
	onPluginEnable(onPluginEnable),
	onRecordStart(onRecordStart),
	onShowTrayNotifier(onShowTrayNotifier),
	onHideTrayNotifier(onHideTrayNotifier),
	onUpdateSettings(onUpdateSettings),
	onUpdateButtons(onUpdateButtons),
	onUpdateButton(onUpdateButton),
	onMainMenuShow(onMainMenuShow),
	onApplicationShow(onApplicationShow),
	onApplicationHide(onApplicationHide),
	onApplicationClose(onApplicationClose),

	running(false)
{
	assert(onCall && onHangup && onAnswer && onRedial && onGetDial && onSetDial &&
		onSendDtmf &&
		onGetContactName &&
		onGetStreamingState &&
		onSetTrayIcon &&
		onGetRegistrationState &&
		onPluginSendMessageText && onPluginEnable &&
		onRecordStart &&
		onShowTrayNotifier &&
		onHideTrayNotifier &&
		onUpdateSettings &&
		onUpdateButtons && onUpdateButton &&
		onMainMenuShow &&
		onApplicationShow &&
		onApplicationHide &&
		onApplicationClose
		);
}

ScriptExec::~ScriptExec()
{
}

void ScriptExec::Run(const char* script)
{
	breakReq = false;
	running = true;
	Lua_State L;
	luaL_openlibs(L);
	contexts[L] = this;

	// do not include internal/standard functions in help
	lua_register(L, "_ALERT", ScriptImp::LuaError);
	lua_register(L, "print", ScriptImp::LuaPrint);

	lua_register2(L, ScriptImp::l_ShowMessage, "ShowMessage", "Show simple message dialog", "Example: ShowMessage(\"text\")");
	lua_register2(L, l_MessageBox, "MessageBox", "Show standard WinAPI MessageBox",
		"Examples:\n"
		"  MessageBox(\"message with just [OK] button\", \"message title\", 0)\n"
		"  MessageBox(\"message with ICON_INFORMATION\", \"message title\", 64)\n"
		"  local res = MessageBox(\"message with MB_YESNO and question icon\", \"message title\", 4+32)\n"
		"  if res == 6 then\n"
		"  \tShowMessage(\"\\\"Yes\\\" was pressed\")\n"
		"  else\n"
		"  \tShowMessage(\"Result is other than \\\"Yes\\\"\")\n"
		"  end  ");
	lua_register2(L, ScriptImp::l_InputQuery, "InputQuery", "Display modal dialog allowing to take text input from the user", "Example: local text, isAccepted = InputQuery(caption, prompt, defaultText).");
	lua_register2(L, ScriptImp::l_Sleep, "Sleep", "Pause script for specified time (miliseconds)", "Function returns non-zero if it exited due to user break, zero if full delay passed. Example: Sleep(100).");
	lua_register2(L, l_Beep, "Beep", "Equivalent of WinAPI Beep(frequency, time)", "Example: Beep(400, 250).");
	lua_register2(L, ScriptImp::l_CheckBreak, "CheckBreak", "Check if \"Break\" button was pressed by the user", "Allowing to interrupt scripts");
	lua_register2(L, ScriptImp::l_GetClipboardText, "GetClipboardText", "Get clipboard content as text", "");
	lua_register2(L, ScriptImp::l_SetClipboardText, "SetClipboardText", "Copy text to clipboard", "");
	lua_register2(L, ScriptImp::l_ForceDirectories, "ForceDirectories", "Make sure directory path exists, possibly creating folders recursively", "Equivalent of VCL function with same name.");
	lua_register2(L, ScriptImp::l_FindWindowByCaptionAndExeName, "FindWindowByCaptionAndExeName", "Search for window by caption and executable name", "");
	lua_register2(L, ScriptImp::l_Call, "Call", "Call to specified number or URI", "Returns status (0 on success) and allocated call ID. May fail if current call number reaches limit.");
	lua_register2(L, ScriptImp::l_Hangup, "Hangup", "Disconnect or reject current incoming call", "Examples:\n    Hangup()\n    Hangup(sipCode, reasonText)");
	lua_register2(L, ScriptImp::l_Hangup2, "Hangup2", "Disconnect or reject specific incoming call", "Examples:\n    Hangup2(callUid)\n    Hangup2(callUid, sipCode, reasonText)");
	lua_register2(L, ScriptImp::l_HangupAll, "HangupAll", "Disconnect all calls", "Example: HangupAll()");
	lua_register2(L, ScriptImp::l_Answer, "Answer", "Answer current or specified incoming call", "Takes call UID as optional argument to answer specific call.");
	lua_register2(L, ScriptImp::l_Redial, "Redial", "Redial - action identical to redial button", "Make call using number/URI from last outgoing call.");
	lua_register2(L, ScriptImp::l_SetMute, "SetMute", "Set mute state for transmitted audio (microphone) of the specified call", "Examples:\nSetMute(callUid, 1) -- mute\nSetMute(callUid, 0) -- unmute");
	lua_register2(L, ScriptImp::l_ToggleMute, "ToggleMute", "Toggle (reverse) mute state for current or specified call", "Takes callUid as optional argument.");
	lua_register2(L, ScriptImp::l_GetMute, "GetMute", "Get mute state for current call or specified call", "Takes callUid as optional argument, returns 0/1.");
	lua_register2(L, ScriptImp::l_SetHold, "SetHold", "Set hold state for specified call", "Example: SetHold(callUid, 1).");
	lua_register2(L, ScriptImp::l_ToggleHold, "ToggleHold", "Toggle hold state for current or specified call", "Examples:\nToggleHold(callUid)\nToggleHold()");
	lua_register2(L, ScriptImp::l_GetHold, "GetHold", "Get hold state for current or specified call", "Examples:\nlocal state = GetHold()\nlocal state2 = GetHold(callUid)");
	lua_register2(L, ScriptImp::l_GetDial, "GetDial", "Get number (string) from softphone dial edit", "");
	lua_register2(L, ScriptImp::l_SetDial, "SetDial", "Set text on softphone dialing edit control", "");
	lua_register2(L, ScriptImp::l_SwitchAudioSource, "SwitchAudioSource", "Change audio source during the call", "Example: SwitchAudioSource(\"aufile\", \"file.wav\").");
	lua_register2(L, ScriptImp::l_SwitchAudioSource2, "SwitchAudioSource2", "Change audio source for the specified call", "Example: SwitchAudioSource2(callUid, \"aufile\", \"file.wav\").");
	lua_register2(L, ScriptImp::l_SwitchAudioSourceToConfigured, "SwitchAudioSourceToConfigured", "Change audio source for current or specified call to default/configured", "Example: SwitchAudioSourceToConfigured(optional_callUid).");
	lua_register2(L, ScriptImp::l_SwitchAudioPlayer, "SwitchAudioPlayer", "Change audio output during the call", "Example: SwitchAudioPlayer(\"winwave2\", \"Headphones\").");
	lua_register2(L, ScriptImp::l_SwitchAudioPlayer2, "SwitchAudioPlayer2", "Change audio output for the specified call", "Example: SwitchAudioPlayer2(callUid, \"winwave2\", \"Headphones\").");
	lua_register2(L, ScriptImp::l_SwitchVideoSource, "SwitchVideoSource", "Change video source during the call", "Example: SwitchVideoSource(\"avformat\", \"file.mp4\").");
	lua_register2(L, ScriptImp::l_SwitchVideoSource2, "SwitchVideoSource2", "Change video source for the specified call", "Example: SwitchVideoSource(callUid, \"avformat\", \"file.mp4\").");
	lua_register2(L, ScriptImp::l_SendDtmf, "SendDtmf", "Send DTMF symbols during the call", "Accepts single DTMF or whole string");
	lua_register2(L, ScriptImp::l_GenerateTones, "GenerateTones", "Generate up to 4 tones with specified amplitude and frequency", "Tone generator is able to generate up to 4 sine waves at the same time, each one with separate amplitude and frequency setting. Sum of sine waves is saturated. Tone generator is placed before softvol module (software volume control sliders) in transmit chain and replaces \"regular\" audio source when is activated.\nGenerateTones function takes up to 8 parameters (up to 4 pairs of amplitude + frequency). Amplitude is interpreted as a fraction of full-scale.\nCalling this function without arguments stops generator.\nExample generating 1000 Hz at 0.2 FS + 3000 Hz at 0.1 FS:\n\tGenerateTones(0.2, 1000, 0.1, 3000)");
	lua_register2(L, ScriptImp::l_GenerateTones2, "GenerateTones2", "Second version of GenerateTones function, taking call ID as first argument", "Tone generator is able to generate up to 4 sine waves at the same time, each one with separate amplitude and frequency setting. Sum of sine waves is saturated. Tone generator is placed before softvol module (software volume control sliders) in transmit chain and replaces \"regular\" audio source when is activated.\nGenerateTones2 function takes up to 9 parameters (call ID + up to 4 pairs of amplitude + frequency). Amplitude is interpreted as a fraction of full-scale.\nCalling this function without arguments stops generator.\nExample generating 1000 Hz at 0.2 FS + 3000 Hz at 0.1 FS:\n\tGenerateTones2(callUid, 0.2, 1000, 0.1, 3000)");
	lua_register2(L, ScriptImp::l_BlindTransfer, "BlindTransfer", "Send REFER during the call", "");
	lua_register2(L, ScriptImp::l_BlindTransfer2, "BlindTransfer2", "Send REFER for specific call", "Example: BlindTransfer(callUid, target)");
	lua_register2(L, ScriptImp::l_AttendedTransfer, "AttendedTransfer", "Attended transfer using two already established calls", "Example: AttendedTransfer(callUid1, callUid2)");
	lua_register2(L, ScriptImp::l_GetCalls, "GetCalls", "Get a table with UIDs of currently active calls", "");
	lua_register2(L, ScriptImp::l_GetCallUidFromLineButton, "GetCallUidFromLineButton", "Get call UID assigned to specified line button", "Allows to e.g. create answer/hangup button associated with specified line button.\nExample: local callUid = GetCallUidFromLineButton(buttonId)");
	lua_register2(L, ScriptImp::l_GetCurrentCallUid, "GetCurrentCallUid", "Get UID of current call, 0 = invalid/none", "");
	lua_register2(L, ScriptImp::l_SetCurrentCallUid, "SetCurrentCallUid", "Set current call to call with specified UID", "Returns 0 on success.");
	lua_register2(L, ScriptImp::l_GetCallState, "GetCallState", "Get state of current or specified call", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallButtonId, "GetCallButtonId", "Get button ID assigned to current or specified call", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetRecorderState, "GetRecorderState", "Check if recording is running for current or specified call", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetZrtpState, "GetZrtpState", "Get current state of ZRTP encryption for current or specified call", "Returns session ID, active/inactive state, SAS code, cipher, verfication state. Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_IsCallIncoming, "IsCallIncoming", "Check if current or specified call is incoming", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallPeer, "GetCallPeer", "Get number/URI of caller/callee from current or specified call", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallPeerName, "GetCallPeerName", "Get name of caller/callee from current or specified call", "Takes one, optional argument: call UID. Returns 0 results, display name or (depending on settings) PAI display name.");
	lua_register2(L, ScriptImp::l_GetCallInitialRxInvite, "GetCallInitialRxInvite", "Get full text of initial received INVITE", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallCodecName, "GetCallCodecName", "Get name of codec used during current or specified call", "Takes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallReason, "GetCallReason", "Get SIP Reason associated with current or specified call", "Takes one, optional argument: call UID. Returns reason from request line or from Reason line like \"Call completed elsewhere\"");
	lua_register2(L, ScriptImp::l_GetCallDurationFromStart, "GetCallDurationFromStart", "Get time in ms from the call start/creation", "Returns milliseconds passed since call was created (call incoming event or making call).\nTakes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetCallDurationFromConfirmed, "GetCallDurationFromConfirmed", "Get time in ms from the call start/creation", "Returns milliseconds passed since call was confirmed.\nIf call was not confirmed or does not exits, nothing is returned.\nTakes one, optional argument: call UID.");
	lua_register2(L, ScriptImp::l_GetContactName, "GetContactName", "Get number description from phonebook", "");
	lua_register2(L, ScriptImp::l_GetStreamingState, "GetStreamingState", "Get current state of RTP streaming", "");
	lua_register2(L, ScriptImp::l_GetAudioErrorCount, "GetAudioErrorCount", "Get number of audio device errors during the call", "Used to detect end-of-file event for wave input files. Takes one, optional argument: call UID.");

	lua_register2(L, ScriptImp::l_SetVariable, "SetVariable", "Set value for variable with specified name", "Example: SetVariable(\"runcount\", count).");
	lua_register2(L, ScriptImp::l_GetVariable, "GetVariable", "Get variable value and isSet flag for variable with specified name", "Example: local count, var_isset = GetVariable(\"runcount\")");
	lua_register2(L, ScriptImp::l_ClearVariable, "ClearVariable", "Delete/unset variable with specified name", "Example: ClearVariable(\"runcount\")");
	lua_register2(L, ScriptImp::l_ClearAllVariables, "ClearAllVariables", "Delete/unset all variables", "");

	// QueuePush(queueName, stringValue)
	lua_register2(L, ScriptImp::l_QueuePush, "QueuePush", "Push string value to queue with specified name", "");
	lua_register2(L, ScriptImp::l_QueuePop, "QueuePop", "Get value from specified queue", "Example: local value, isValid = QueuePop(queueName)");
	lua_register2(L, ScriptImp::l_QueueClear, "QueueClear", "Clear specified queue", "");
	// local queue_size = QueueGetSize(queueName)
	lua_register2(L, ScriptImp::l_QueueGetSize, "QueueGetSize", "Get number of elements in specified queue", "");

	lua_register2(L, ScriptImp::l_GetInitialCallTarget, "GetInitialCallTarget", "Get number/URI that was initially dialed by the user", "");
	lua_register2(L, ScriptImp::l_SetInitialCallTarget, "SetInitialCallTarget", "Override number dialed by the user", "");
	lua_register2(L, ScriptImp::l_SetCallTarget, "SetCallTarget", "Overwrite both initial call target and current URI of the call", "");
	lua_register2(L, ScriptImp::l_ResetCall, "ResetCall", "Clear whole call state", "Use with care!");
	lua_register2(L, ScriptImp::l_GetPreviousCallStatusCode, "GetPreviousCallStatusCode", "Get status code of call that ended", "");
	lua_register2(L, ScriptImp::l_GetPreviousCallReplyLine, "GetPreviousCallReplyLine", "Get SIP reply line from the call that ended", "");
	lua_register2(L, ScriptImp::l_ShellExecute, "ShellExecute", "Run another application (WinAPI equivalent)", "");
	lua_register2(L, ScriptImp::l_SetTrayIcon, "SetTrayIcon", "Change tray icon bitmap", "");
	lua_register2(L, ScriptImp::l_GetRegistrationState, "GetRegistrationState", "Check if softphone is registered", "");
	lua_register2(L, ScriptImp::l_Unregister, "Unregister", "Unregister SIP account", "");
	lua_register2(L, ScriptImp::l_Reregister, "Reregister", "Re-register SIP account", "");
	lua_register2(L, ScriptImp::l_SetButtonCaption, "SetButtonCaption", "Set text for the first line of the button", "Example: SetButtonCaption(buttonId, text).");
	lua_register2(L, ScriptImp::l_SetButtonCaption2, "SetButtonCaption2", "Set text for the second line of the button", "Example: SetButtonCaption2(buttonId, text).");
	lua_register2(L, ScriptImp::l_SetButtonDown, "SetButtonDown", "Change button state to down/pressed", "Example: SetButtonDown(buttonId, buttonState).");
	lua_register2(L, ScriptImp::l_GetButtonDown, "GetButtonDown", "Check if button is down", "Example state = GetButtonDown(buttonId) -- returning 0 or 1.");
	lua_register2(L, ScriptImp::l_GetButtonMouseDown, "GetButtonMouseDown", "Check is mouse button is pressed on programmable button", "");
	lua_register2(L, ScriptImp::l_GetButtonBlfState, "GetButtonBlfState", "Get BLF state from the button", "");
	lua_register2(L, ScriptImp::l_GetButtonHandle, "GetButtonHandle", "Get Windows HANDLE for specified button", "");	
	lua_register2(L, ScriptImp::l_SetButtonInactive, "SetButtonInactive", "Prevent button from being pressed, set its state to inactive", "");
	lua_register2(L, ScriptImp::l_SetButtonVisible, "SetButtonVisible", "Show/hide button", "");
	lua_register2(L, ScriptImp::l_SetButtonImage, "SetButtonImage", "Set button bitmap", "");
	lua_register2(L, ScriptImp::l_PluginSendMessageText, "PluginSendMessageText", "Send text to specified plugin", "");
	lua_register2(L, ScriptImp::l_PluginEnable, "PluginEnable", "Enable/disable specified plugin", "Example: PluginEnable(\"TTS.dll\", 0/1)");
	lua_register2(L, ScriptImp::l_GetExecSourceType, "GetExecSourceType", "Get type of event that triggered script execution", "See also: GetExecSourceId().");
	lua_register2(L, ScriptImp::l_GetExecSourceId, "GetExecSourceId", "Get ID of object that triggered script (depending on trigger type)", "See also: GetExecSourceType().");
	lua_register2(L, ScriptImp::l_GetRecordFile, "GetRecordFile", "Get name of recording file from current call or call that ended", "");
	lua_register2(L, ScriptImp::l_GetContactId, "GetContactId", "Get contact ID for specified number/URI", "");
	lua_register2(L, ScriptImp::l_GetBlfState, "GetBlfState", "Get BLF state of specified contact (by contact ID)", "To be used in \"on BLF change\" (GetExecSourceId() as contact id / argument) or together with GetContactId(number).\nReturning number, state, remote identity number/URI, remote identity display name and call direction.");
	lua_register2(L, ScriptImp::l_RecordStart, "RecordStart", "Start recording", "Example: RecordStart(filename, channels, side, fileFormat, bitrate, optionalCallUid).");
	lua_register2(L, ScriptImp::l_GetExeName, "GetExeName", "Get name and full path of this executable",  "");
	lua_register2(L, ScriptImp::l_GetProfileDir, "GetProfileDir", "Get folder name where settings and other files are stored", "");
	lua_register2(L, ScriptImp::l_GetRecordingState, "GetRecordingState", "Check if softphone is recording at the moment", "");
	lua_register2(L, ScriptImp::l_GetRxDtmf, "GetRxDtmf", "Get DTMF from receiving queue, empty string if queue is empty", "");
	lua_register2(L, ScriptImp::l_ShowTrayNotifier, "ShowTrayNotifier", "Show tray notifier window with specified description, URI, incoming state, call UID", "Example: ShowTrayNotifier(description, uri, dirIncoming, callUid)");
	lua_register2(L, ScriptImp::l_HideTrayNotifier, "HideTrayNotifier", "Hide tray notifier window", "");
	lua_register2(L, ScriptImp::l_GetUserName, "GetUserName", "Get user name from account settings", "");
	lua_register2(L, ScriptImp::l_ProgrammableButtonClick, "ProgrammableButtonClick", "Programmatically press button", "");
	lua_register2(L, ScriptImp::l_RefreshAudioDevicesList, "RefreshAudioDevicesList", "Rescan available audio devices", "");
	lua_register2(L, ScriptImp::l_GetAudioDevice, "GetAudioDevice", "Get the name of selected audio input or output device", "name, valid = GetAudioDevice(moduleName, direction, id)\nEnumerating audio devices one by one; moduleName = \"winwave\", \"winwave2\" or \"portaudio\", direction = \"in\" (recording device) or \"out\" (playback device), id = index starting from 0; returning name (string) of the device and valid (int): 1 if name is valid / device exists, 0 otherwise.");
	lua_register2(L, ScriptImp::l_GetAudioDevicesList, "GetAudioDevicesList", "Get table/array of audio device names for specified module and direction", "local devices = GetAudioDevicesList(moduleName, direction)\nwhere: moduleName = \"winwave\", \"winwave2\" or \"portaudio\", direction = \"in\" (recording device) or \"out\" (playback device).");
	lua_register2(L, ScriptImp::l_UpdateSettings, "UpdateSettings", "Update main settings with JSON", "Application provisioning or changing settings while running. JSON is merged. Example:\nlocal settings = [[\n{\n   \"uaConf\" : {\n      \"audioCfgAlert\" : {\n         \"volume\" : 0.1\n      },\n      \"audioCfgRing\" : {\n         \"volume\" : 0.2\n      }\n   }\n}\n]]\n\nUpdateSettings(settings)");
	lua_register2(L, ScriptImp::l_UpdateButtons, "UpdateButtons", "Update buttons settings with JSON", "Provisioning for buttons or changing settings while running. JSON is merged. Example:\nUpdateButtons('{\"btnConf\":[{\"caption\":\"    REDIAL\"}]}')");
	lua_register2(L, ScriptImp::l_UpdateButton, "UpdateButton", "Update single button settings with JSON", "Provisioning or changing settings while running for a single button. JSON is merged. Example:\nUpdateButton(BUTTON_ID, '{\"caption\":\"    REDIAL\"}')");

	lua_register2(L, ScriptImp::l_SetHandled, "SetHandled", "Set \"handled\" flag associated with script trigger event", "Possibility of skipping default event handling after script was called (replacing default behavior with script), Example: SetHandled(1).");

	lua_register2(L, ScriptImp::l_GetButtonType, "GetButtonType", "Get type of the button with specified id", "");
	lua_register2(L, ScriptImp::l_GetButtonNumber, "GetButtonNumber", "Get number/URI from button configuration", "");

	lua_register2(L, ScriptImp::l_MainMenuShow, "MainMenuShow", "Show/hide main menu (e.g. in kiosk applications)", "");
	lua_register2(L, ScriptImp::l_ApplicationShow, "ApplicationShow", "Show application (restoring minimized or from tray)", "Example: ApplicationShow(focused)");
	lua_register2(L, ScriptImp::l_ApplicationHide, "ApplicationHide", "Hide application to tray", "");
	lua_register2(L, ScriptImp::l_ApplicationClose, "ApplicationClose", "Close this program", "");

	lua_register2(L, ScriptImp::l_SendCustomRequest, "SendCustomRequest", "Send custom SIP request", "Example: requestUid = SendCustomRequest(uri, method, extraHeaderLines)\nrequestUid is > 0 on success\nextraHeaderLines parameter is optional");
	lua_register2(L, ScriptImp::l_ClearCustomRequests, "ClearCustomRequests", "Delete status info of custom SIP requests", "");
	lua_register2(L, ScriptImp::l_DeleteCustomRequest, "DeleteCustomRequest", "Delete single custom request info", "");
	lua_register2(L, ScriptImp::l_GetCustomRequest, "GetCustomRequest", "Get information about custom request", "");
	lua_register2(L, ScriptImp::l_GetCustomRequestReply, "GetCustomRequestReply", "Get reply SIP code for custom request", "");
	lua_register2(L, ScriptImp::l_GetCustomRequestReplyText, "GetCustomRequestReplyText", "Get full text of received reply for custom request", "");

	lua_register2(L, ScriptImp::l_GetAudioRxSignalLevel, "GetAudioRxSignalLevel", "Get amplitude of received audio from the call", "Lenny/IVR-like applications");

	lua_register2(L, ScriptImp::l_ReadContacts, "ReadContacts", "Read again contacts from default JSON file", "This function can be used to reload phonebook from file after provisioning, e.g. after fetching JSON file using curl using provisioning script.");
	lua_register2(L, ScriptImp::l_ReadXmlContacts, "ReadXmlContacts", "Read contacts from XML Yealink-like file", "");
	lua_register2(L, ScriptImp::l_AppendContactNoteText, "AppendContactNoteText", "Add text to note from currently opened contact popup", "");

	lua_register2(L, ScriptImp::l_SendTextMessage, "SendTextMessage", "Send SIP SIMPLE message", "Example: SendTextMesssage(target, text, sendImmediately)\nOpens text messaging window and optionally sends immediately text.");

	lua_register2(L, ScriptImp::l_SetAppStatus, "SetAppStatus", "Set status visible as hint in system tray", "");

	lua_register2(L, ScriptImp::l_ShowLogWindow, "ShowLogWindow", "Show and bring to front log window", "");

	// add library
	luaL_requiref(L, "tsip_winapi", luaopen_tsip_winapi, 0);

	if (!globalsSetComplete)
	{
        // symbols from tsip_winapi
		AddSymbol("winapi.FindWindow", "WinAPI FindWindow equivalent", "");
		AddSymbol("winapi.SendMessage", "WinAPI SendMessage equivalent", "Example use: sending WM_CLOSE to other application");
		AddSymbol("winapi.Beep", "WinAPI Beep equivalent", "Same as Beep, example: Beep(frequencyHz, timeMs)");
		AddSymbol("winapi.MessageBox", "WinAPI MessageBox equivalent", "");
		AddSymbol("winapi.GetAsyncKeyState", "WinAPI GetAsyncKeyState equivalent", "Example use: modify button behavior depending on Ctrl/Alt/Shift state.");
		AddSymbol("winapi.PlaySound", "WinAPI PlaySound equivalent", "");
		AddSymbol("winapi.keybd_event", "WinAPI keybd_event equivalent but without 4th parameter", "");
		AddSymbol("winapi.GetAdaptersInfoIp", "Function returning array of computer IPs", "");		
	}
	globalsSetComplete = true;

	int res = luaL_dostring(L, script);
	if(res != 0)
	{
		AnsiString txt;
		txt.sprintf("Execution error:\n%s", lua_tostring(L, -1));
		MessageBox(NULL, txt.c_str(), "Lua", MB_ICONINFORMATION);
	}
	running = false;

	std::map<lua_State*, ScriptExec*>::iterator it;
	it = contexts.find(L);
	assert(it != contexts.end());
	contexts.erase(it);
}

void ScriptExec::Break(void)
{
	breakReq = true;
}


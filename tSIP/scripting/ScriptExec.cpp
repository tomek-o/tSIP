//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ScriptExec.h"
#include "Log.h"
#include "LuaState.h"
#include "lua.hpp"
#include "AudioDevicesList.h"
#include "Paths.h"
#include "common/Utils.h"
#include "ButtonConf.h"
#include "Call.h"
#include "UaCustomRequests.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include <Clipbrd.hpp>
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
	DWORD dwThreadId, dwProcessId;
	HINSTANCE hInstance;
	char String[255];
	HANDLE hProcess;
	if (!hWnd)
		return TRUE;		// Not a window
	if (findWindowData.windowName)
	{
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
		hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
		// GetModuleFileNameEx uses psapi, which works for NT only!
		BOOL result = TRUE;
		if (GetModuleFileNameEx(hProcess, hInstance, String, sizeof(String)))
		{
			String[sizeof(String)-1] = '\0';
			if (stricmp(String, findWindowData.exeName) == 0)
			{
				findWindowData.hWndFound = hWnd;
				result = FALSE;
			}
		}
		CloseHandle(hProcess);
		return result;
	}
	return TRUE;
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

int ScriptExec::LuaPrint(lua_State *L)
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
	GetContext(L)->onAddOutputText(ret.c_str());
	return 0;
};

int ScriptExec::LuaError( lua_State *L )
{
	const char* str = lua_tostring( L, -1 );
	lua_pop(L, 1);
	//send string to console
	GetContext(L)->onAddOutputText(str);
	return 0;
}

int ScriptExec::l_ShowMessage( lua_State* L )
{
	AnsiString msg = lua_tostring(L, -1);
	MessageBox(NULL, msg.c_str(), "Lua message", MB_ICONINFORMATION);
	lua_pushnumber( L, 0 );
	return 1;
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

int ScriptExec::l_InputQuery(lua_State* L)
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
int ScriptExec::l_Sleep(lua_State* L)
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

/** \brief Check if user interrupted script for clean, controlled exit
*/
int ScriptExec::l_CheckBreak(lua_State *L)
{
	int ret = GetContext(L)->breakReq?1:0;
	lua_pushnumber(L, ret);
	return 1;					// number of return values
}

int ScriptExec::l_GetClipboardText( lua_State* L )
{
	AnsiString text = Clipboard()->AsText;
	lua_pushstring( L, text.c_str() );
	return 1;
}

int ScriptExec::l_SetClipboardText( lua_State* L )
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
int ScriptExec::l_ForceDirectories(lua_State* L)
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

int ScriptExec::l_FindWindowByCaptionAndExeName(lua_State* L)
{
	static Mutex mutex;

	ScopedLock<Mutex> lock(mutex);
	findWindowData.hWndFound = NULL;

	findWindowData.windowName = lua_tostring(L, 1);
	findWindowData.exeName = lua_tostring(L, 2);
	if (findWindowData.windowName == NULL && findWindowData.exeName == NULL)
	{
		LOG("Lua: either window name or exe name is required");
		lua_pushnumber(L, 0);
		return 1;
	}

	::EnumWindows((WNDENUMPROC)EnumWindowsProc, NULL);

	lua_pushnumber(L, reinterpret_cast<unsigned int>(findWindowData.hWndFound));
	return 1;
}

int ScriptExec::l_Call(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	GetContext(L)->onCall(str);
	return 0;
}

int ScriptExec::l_Hangup(lua_State* L)
{
	GetContext(L)->onHangup();
	return 0;
}

int ScriptExec::l_Answer(lua_State* L)
{
	GetContext(L)->onAnswer();
	return 0;
}

int ScriptExec::l_SetDial(lua_State* L)
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

int ScriptExec::l_GetDial(lua_State* L)
{
	std::string num = GetContext(L)->onGetDial();
	lua_pushstring( L, num.c_str() );
	return 1;
}

int ScriptExec::l_SetInitialCallTarget(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	Call *call = GetContext(L)->onGetCall();
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

int ScriptExec::l_GetInitialCallTarget(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		std::string num = call->initialTarget.c_str();
		lua_pushstring( L, num.c_str() );
		return 1;
	}
	return 0;
}

int ScriptExec::l_SwitchAudioSource(lua_State* L)
{
	//  The first element in the stack (that is, the element that was pushed first) has index 1, the next one has index 2, and so on.
	const char* mod = lua_tostring( L, 1 );
	if (mod == NULL)
	{
		LOG("Lua error: mod == NULL\n");
		return 0;
	}
	const char* dev = lua_tostring( L, 2 );
	if (dev == NULL)
	{
		LOG("Lua error: dev == NULL\n");
		return 0;
	}
	GetContext(L)->onSwitchAudioSource(mod, dev);
	return 0;
}

int ScriptExec::l_SendDtmf(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
		LOG("Lua error: str == NULL\n");
		return 0;
	}
	GetContext(L)->onSendDtmf(str);
	return 0;
}

int ScriptExec::l_BlindTransfer(lua_State* L)
{
	const char* str = lua_tostring( L, -1 );
	if (str == NULL)
	{
        LOG("Lua BlindTransfer error: str == NULL\n");
		return 0;
	}
	GetContext(L)->onBlindTransfer(str);
	return 0;
}

int ScriptExec::l_GetCallState(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushinteger( L, call->state );
		return 1;
	}
	return 0;
}

int ScriptExec::l_IsCallIncoming(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushinteger( L, call->incoming );
		return 1;
	}
	return 0;
}

int ScriptExec::l_GetCallPeer(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
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

int ScriptExec::l_GetCallInitialRxInvite(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushstring( L, call->initialRxInvite.c_str() );
		return 1;
	}
	return 0;
}

int ScriptExec::l_GetCallCodecName(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushstring( L, call->codecName.c_str() );
		return 1;
	}
	return 0;
}

int ScriptExec::l_GetContactName(lua_State* L)
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

int ScriptExec::l_GetStreamingState(lua_State* L)
{
	int state = GetContext(L)->onGetStreamingState();
	lua_pushinteger( L, state );
	return 1;
}

int ScriptExec::l_GetRegistrationState(lua_State* L)
{
	int state = GetContext(L)->onGetRegistrationState();
	lua_pushinteger( L, state );
	return 1;
}

int ScriptExec::l_SetButtonCaption(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	if (str == NULL)
		str = "";
	GetContext(L)->onSetButtonCaption(id, str);
	return 0;
}

int ScriptExec::l_SetButtonCaption2(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	if (str == NULL)
		str = "";
	GetContext(L)->onSetButtonCaption2(id, str);
	return 0;
}

int ScriptExec::l_SetButtonDown(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	int state = lua_tointeger( L, 2 );
	GetContext(L)->onSetButtonDown(id, state);
	return 0;
}

int ScriptExec::l_GetButtonDown(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	bool down = GetContext(L)->onGetButtonDown(id);
	lua_pushinteger( L, down?1:0 );
	return 1;
}

int ScriptExec::l_SetButtonImage(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const char* str = lua_tostring( L, 2 );
	GetContext(L)->onSetButtonImage(id, str);
	return 0;
}

int ScriptExec::l_ProgrammableButtonClick(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	GetContext(L)->onProgrammableButtonClick(id);
	return 0;
}

int ScriptExec::l_PluginSendMessageText(lua_State* L)
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

int ScriptExec::l_PluginEnable(lua_State* L)
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

int ScriptExec::l_SetVariable(lua_State* L)
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
	SetVariable(name, value);
	return 0;
}

int ScriptExec::l_GetVariable(lua_State* L)
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

int ScriptExec::l_ClearVariable(lua_State* L)
{
	const char* name = lua_tostring( L, 1 );
	if (name == NULL)
	{
		LOG("Lua error: name == NULL\n");
		return 0;
	}	
	ClearVariable(name);
	return 0;
}

int ScriptExec::l_ClearAllVariables(lua_State* L)
{
	variables.clear();
	return 0;
}

int ScriptExec::l_QueuePush(lua_State* L)
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
	QueuePush(queue_name, value);
	return 0;
}

int ScriptExec::l_QueuePop(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		return 0;
	}
	AnsiString value;
	int ret = QueuePop(queue_name, value);
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

int ScriptExec::l_QueueClear(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		lua_pushinteger(L, -1);
		return 1;
	}
	int ret = QueueClear(queue_name);
	lua_pushinteger(L, ret);
	return 1;
}

int ScriptExec::l_QueueGetSize(lua_State* L)
{
	const char* queue_name = lua_tostring( L, 1 );
	if (queue_name == NULL)
	{
		LOG("Lua error: queue_name == NULL\n");
		lua_pushinteger(L, 0);
		return 1;
	}
	int ret = QueueGetSize(queue_name);
	lua_pushinteger(L, ret);
	return 1;
}

int ScriptExec::l_ShellExecute(lua_State* L)
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

int ScriptExec::l_SetTrayIcon(lua_State* L)
{
	const char* file = lua_tostring( L, 1 );
	GetContext(L)->onSetTrayIcon(file);
	return 0;
}

int ScriptExec::l_GetExecSourceType(lua_State* L)
{
	lua_pushinteger(L, GetContext(L)->srcType);
	return 1;
}

int ScriptExec::l_GetExecSourceId(lua_State* L)
{
	lua_pushinteger(L, GetContext(L)->srcId);
	return 1;
}

int ScriptExec::l_GetRecordFile(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushstring( L, call->recordFile.c_str() );
		return 1;
	}
	return 0;
}

int ScriptExec::l_GetBlfState(lua_State* L)
{
	int contactId = lua_tointeger( L, 1 );

	std::string number;
	int state = GetContext(L)->onGetBlfState(contactId, number);
	lua_pushstring( L, number.c_str() );
	lua_pushinteger( L, state );
	return 2;
}

int ScriptExec::l_RecordStart(lua_State* L)
{
	const char* file = lua_tostring( L, 1 );
	int channels = lua_tointeger( L, 2 );
	int side = lua_tointeger( L, 3 );	// optional, introduced in tSIP 0.1.66
	int ret = GetContext(L)->onRecordStart(file, channels, side);
	lua_pushinteger(L, ret);
	return 1;
}

int ScriptExec::l_GetRecordingState(lua_State* L)
{
	Call *call = GetContext(L)->onGetCall();
	if (call)
	{
		lua_pushinteger( L, call->recording );
		return 1;
	}
	return 0;
}

int ScriptExec::l_GetRxDtmf(lua_State* L)
{
	std::string num = GetContext(L)->onGetRxDtmf();
	lua_pushstring( L, num.c_str() );
	return 1;
}

int ScriptExec::l_ShowTrayNotifier(lua_State* L)
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
	int incoming = lua_tointegerx(L, 3, NULL);

	GetContext(L)->onShowTrayNotifier(description, uri, incoming);

	lua_pushinteger(L, 0);
	return 1;
}

int ScriptExec::l_GetUserName(lua_State* L)
{
	std::string user = GetContext(L)->onGetUserName();
	lua_pushstring(L, user.c_str());
	return 1;
}

int ScriptExec::l_GetExeName(lua_State* L)
{
	lua_pushstring(L, Application->ExeName.c_str());
	return 1;
}

int ScriptExec::l_GetProfileDir(lua_State* L)
{
	lua_pushstring(L, Paths::GetProfileDir().c_str());
	return 1;
}

int ScriptExec::l_RefreshAudioDevicesList(lua_State* L)
{
	AudioDevicesList::Instance().Refresh();
	return 0;
}

int ScriptExec::l_GetAudioDevice(lua_State* L)
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

	std::vector<AnsiString> *v = NULL;
	if (strcmp(module, "portaudio") == 0)
	{
		if (out)
			v = &AudioDevicesList::Instance().portaudioDevsOut;
		else
			v = &AudioDevicesList::Instance().portaudioDevsIn;
	}
	else if (strcmp(module, "winwave") == 0)
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

int ScriptExec::l_UpdateSettings(lua_State* L)
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

int ScriptExec::l_SetHandled(lua_State* L)
{
	bool handled = lua_tointeger( L, 1 );
	GetContext(L)->handled = handled;	
	return 0;
}

int ScriptExec::l_GetButtonType(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const ButtonConf* btnConf = GetContext(L)->onGetButtonConf(id);
	if (btnConf == NULL)
	{
		LOG("Lua error: no button configuration for id = %d\n", id);
		return 0;
	}
	lua_pushinteger(L, btnConf->type);
	return 1;
}

int ScriptExec::l_GetButtonNumber(lua_State* L)
{
	int id = lua_tointeger( L, 1 );
	const ButtonConf* btnConf = GetContext(L)->onGetButtonConf(id);
	if (btnConf == NULL)
	{
		LOG("Lua error: no button configuration for id = %d\n", id);
		return 0;
	}
	lua_pushstring(L, btnConf->number.c_str());
	return 1;
}

int ScriptExec::l_MainMenuShow(lua_State* L)
{
	bool state = lua_tointeger( L, 1 );
	GetContext(L)->onMainMenuShow(state);
	return 0;
}

int ScriptExec::l_SendCustomRequest(lua_State* L)
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


ScriptExec::ScriptExec(
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
	CallbackSetButtonCaption2 onSetButtonCaption2,
	CallbackSetButtonDown onSetButtonDown,
	CallbackGetButtonDown onGetButtonDown,
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
	):
	srcType(srcType),
	srcId(srcId),
	breakReq(breakReq),
	handled(handled),
	onAddOutputText(onAddOutputText),
	onCall(onCall),
	onHangup(onHangup),
	onAnswer(onAnswer),
	onGetDial(onGetDial),
	onSetDial(onSetDial),
	onSwitchAudioSource(onSwitchAudioSource),
	onSendDtmf(onSendDtmf),
	onBlindTransfer(onBlindTransfer),
	onGetCall(onGetCall),
	onGetContactName(onGetContactName),
	onGetStreamingState(onGetStreamingState),
	onSetTrayIcon(onSetTrayIcon),
	onGetRegistrationState(onGetRegistrationState),
	onSetButtonCaption(onSetButtonCaption),
	onSetButtonCaption2(onSetButtonCaption2),	
	onSetButtonDown(onSetButtonDown),
	onGetButtonDown(onGetButtonDown),
	onSetButtonImage(onSetButtonImage),
	onPluginSendMessageText(onPluginSendMessageText),
	onPluginEnable(onPluginEnable),
	onGetBlfState(onGetBlfState),
	onRecordStart(onRecordStart),
	onGetRxDtmf(onGetRxDtmf),
	onShowTrayNotifier(onShowTrayNotifier),
	onGetUserName(onGetUserName),
	onProgrammableButtonClick(onProgrammableButtonClick),
	onUpdateSettings(onUpdateSettings),
	onGetButtonConf(onGetButtonConf),
	onMainMenuShow(onMainMenuShow),

	running(false)
{
	assert(onAddOutputText && onCall && onHangup && onAnswer && onGetDial && onSetDial &&
		onSwitchAudioSource && onSendDtmf && onBlindTransfer &&
		onGetCall &&
		onGetContactName &&
		onGetStreamingState &&
		onSetTrayIcon &&
		onGetRegistrationState &&
		onSetButtonCaption && onSetButtonCaption2 && onSetButtonDown && onGetButtonDown && onSetButtonImage &&
		onPluginSendMessageText && onPluginEnable &&
		onGetBlfState &&
		onRecordStart &&
		onGetRxDtmf &&
		onShowTrayNotifier &&
		onGetUserName &&
		onProgrammableButtonClick &&
		onUpdateSettings &&
		onGetButtonConf &&
		onMainMenuShow
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

	lua_register(L, "_ALERT", LuaError );
	lua_register(L, "print", LuaPrint );
	lua_register(L, "ShowMessage", l_ShowMessage);
	lua_register(L, "MessageBox", l_MessageBox);
	lua_register(L, "InputQuery", l_InputQuery);
	lua_register(L, "Sleep", l_Sleep);
	lua_register(L, "Beep", l_Beep);
	lua_register(L, "CheckBreak",l_CheckBreak);
	lua_register(L, "GetClipboardText", l_GetClipboardText);
	lua_register(L, "SetClipboardText", l_SetClipboardText);
	lua_register(L, "ForceDirectories", l_ForceDirectories);
	lua_register(L, "FindWindowByCaptionAndExeName", l_FindWindowByCaptionAndExeName);
	lua_register(L, "Call", l_Call);
	lua_register(L, "Hangup", l_Hangup);
	lua_register(L, "Answer", l_Answer);
	lua_register(L, "GetDial", l_GetDial);
	lua_register(L, "SetDial", l_SetDial);
	lua_register(L, "SwitchAudioSource", l_SwitchAudioSource);
	lua_register(L, "SendDtmf", l_SendDtmf);
	lua_register(L, "BlindTransfer", l_BlindTransfer);
	lua_register(L, "GetCallState", l_GetCallState);
	lua_register(L, "IsCallIncoming", l_IsCallIncoming);
	lua_register(L, "GetCallPeer", l_GetCallPeer);
	lua_register(L, "GetCallInitialRxInvite", l_GetCallInitialRxInvite);
	lua_register(L, "GetCallCodecName", l_GetCallCodecName);
	lua_register(L, "GetContactName", l_GetContactName);
	lua_register(L, "GetStreamingState", l_GetStreamingState);

	lua_register(L, "SetVariable", l_SetVariable);
	lua_register(L, "GetVariable", l_GetVariable);
	lua_register(L, "ClearVariable", l_ClearVariable);
	lua_register(L, "ClearAllVariables", l_ClearAllVariables);

	// QueuePush(queueName, stringValue)
	lua_register(L, "QueuePush", l_QueuePush);
	// local value, isValid = QueuePop(queueName)
	lua_register(L, "QueuePop", l_QueuePop);
	lua_register(L, "QueueClear", l_QueueClear);
	// local queue_size = QueueGetSize(queueName)
	lua_register(L, "QueueGetSize", l_QueueGetSize);

	lua_register(L, "GetInitialCallTarget", l_GetInitialCallTarget);
	lua_register(L, "SetInitialCallTarget", l_SetInitialCallTarget);
    lua_register(L, "ShellExecute", l_ShellExecute);
    lua_register(L, "SetTrayIcon", l_SetTrayIcon);
    lua_register(L, "GetRegistrationState", l_GetRegistrationState);
	lua_register(L, "SetButtonCaption", l_SetButtonCaption);
	lua_register(L, "SetButtonCaption2", l_SetButtonCaption2);
	lua_register(L, "SetButtonDown", l_SetButtonDown);
	lua_register(L, "GetButtonDown", l_GetButtonDown);
	lua_register(L, "SetButtonImage", l_SetButtonImage);
	lua_register(L, "PluginSendMessageText", l_PluginSendMessageText);
	lua_register(L, "PluginEnable", l_PluginEnable);	// PluginEnable("TTS.dll", 0/1)
	lua_register(L, "GetExecSourceType", l_GetExecSourceType);
	lua_register(L, "GetExecSourceId", l_GetExecSourceId);
	lua_register(L, "GetRecordFile", l_GetRecordFile);
	lua_register(L, "GetBlfState", l_GetBlfState);
	lua_register(L, "RecordStart", l_RecordStart);
	lua_register(L, "GetExeName", l_GetExeName);
	lua_register(L, "GetProfileDir", l_GetProfileDir);
	lua_register(L, "GetRecordingState", l_GetRecordingState);
	lua_register(L, "GetRxDtmf", l_GetRxDtmf);
	lua_register(L, "ShowTrayNotifier", l_ShowTrayNotifier);
	lua_register(L, "GetUserName", l_GetUserName);	
	lua_register(L, "ProgrammableButtonClick", l_ProgrammableButtonClick);
	lua_register(L, "RefreshAudioDevicesList", l_RefreshAudioDevicesList);
	lua_register(L, "GetAudioDevice", l_GetAudioDevice);
	lua_register(L, "UpdateSettings", l_UpdateSettings);

	lua_register(L, "SetHandled", l_SetHandled);

	lua_register(L, "GetButtonType", l_GetButtonType);
	lua_register(L, "GetButtonNumber", l_GetButtonNumber);

    lua_register(L, "MainMenuShow", l_MainMenuShow);

	// requestUid = SendCustomRequest(uri, method, extraHeaderLines)
	// requestUid is > 0 on success
	// extraHeaderLines parameter is optional
    lua_register(L, "SendCustomRequest", l_SendCustomRequest);

	// add library
	luaL_requiref(L, "tsip_winapi", luaopen_tsip_winapi, 0);

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


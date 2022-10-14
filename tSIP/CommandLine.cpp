#include <vcl.h>
#pragma hdrstop

#include "CommandLine.h"
#include "Settings.h"
#include "common/TelecomUtils.h"
#include "common/base64.h"
#include "Branding.h"
#include "Log.h"
#include <string>
#include <stdio.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace {

	std::string UrlDecode(const std::string &SRC) {
		std::string ret;
		char ch;
		int i, ii;
		for (i=0; i<SRC.length(); i++) {
			if (int(SRC[i])=='%') {
				sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
				ch=static_cast<char>(ii);
				ret+=ch;
				i=i+2;
			} else {
				ret+=SRC[i];
			}
		}
		return (ret);
	}

	const char* PROGRAMMABLE_BTN_STR = "PROGRAMMABLE_BTN_";

	const char* SCRIPT_STR = "SCRIPT=";
	const char* SCRIPT_B64_STR = "SCRIPT_B64=";
	const char* SCRIPT_FILE_STR = "SCRIPT_FILE=";
}	// namespace


CommandLine CommandLine::instance;


CommandLine::CommandLine(void):
	action(ACTION_NONE),
	programmableBtnId(-1)
{
}

int CommandLine::Process(void)
{
	char *data = NULL;
	char *buf = NULL;

	// check for previous app instance
	AnsiString expectedCaption;
	if (appSettings.frmMain.bUseCustomCaption)
	{
		expectedCaption = appSettings.frmMain.customCaption;
	}
	else
	{
		expectedCaption = Branding::appName;
	}
	HWND hWnd = FindWindow("TfrmMain", expectedCaption.c_str());

	int paramcnt = ParamCount() + 1;
	if (paramcnt <= 1)
	{
		if (appSettings.frmMain.bSingleInstance && hWnd)
		{
			// bring previous instance to front
			/** \note For more sophisticated behavior:
				compare executable location of previous process
				and then decide if previous instance should be used
			*/
			data = new char[MAX_CMD_PARAM_LEN*2];
			memset(data, 0, MAX_CMD_PARAM_LEN*2);
			buf = data;
			sprintf(data + MAX_CMD_PARAM_LEN, "/%s=SHOWWINDOW", Branding::appProto.c_str());
			paramcnt = 2;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		data = new char[paramcnt * MAX_CMD_PARAM_LEN];
		buf = data;
		for (int i=0; i<paramcnt; i++)
		{
			AnsiString param = ParamStr(i);
			strncpy(buf + (MAX_CMD_PARAM_LEN*i), param.c_str(), MAX_CMD_PARAM_LEN);
			buf[MAX_CMD_PARAM_LEN*i + MAX_CMD_PARAM_LEN-1] = '\0';
		}
	}

	if (hWnd && (appSettings.Integration.bDoNotPassParametersToPreviousInstance == false))
	{
		// Forward the command line in a WM_COPYDATA message to other instance.
		COPYDATASTRUCT cds;
		cds.cbData = paramcnt * MAX_CMD_PARAM_LEN;
		cds.lpData = buf;
		DWORD dwRetVal;
		if (SendMessageTimeout(hWnd, WM_COPYDATA, 0, (LPARAM)&cds, SMTO_NORMAL, 5000, &dwRetVal))
		{
			//ShowMessage("WM_COPYDATA sent");
		}
		else
		{
			//ShowMessage("WM_COPYDATA: failed to send");
		}
		delete[] data;
		return 1;
	}
	else
	{
		Execute(buf, paramcnt);
		delete[] data;
	}
	
	return 0;
}

void CommandLine::Execute(char* buf, int paramcnt)
{
	AnsiString asCallParam;
	asCallParam.sprintf("/%s=", Branding::appProto.c_str());
	const char* const szCallParam = asCallParam.c_str();
	for (int i=1; i<=paramcnt; i++)
	{
		AnsiString asParam = buf + (MAX_CMD_PARAM_LEN*i);
        //LOG("asParam = %s\n", asParam.c_str());
		if (LowerCase(asParam).Pos(szCallParam) == 1)
		{
			AnsiString asCallTo =
				asParam.SubString(strlen(szCallParam)+1, asParam.Length() - strlen(szCallParam)).Trim();

			AnsiString asCommandPart;
			int i = asCallTo.Pos(":");
			if (i && i < 9)
				asCommandPart = asCallTo.SubString(i+1, asCallTo.Length()-i);
			else
				asCommandPart = asCallTo;

			asCallTo = UrlDecode(asCallTo.c_str()).c_str();

			//LOG("asCallTo = %s\n", asCallTo.c_str());

			if (asCommandPart == "HANGUP")
			{
				action = ACTION_HANGUP;
			}
			else if (asCommandPart == "ANSWER")
			{
				action = ACTION_ANSWER;
			}
			else if (asCommandPart == "SHOWWINDOW")
			{
				action = ACTION_SHOWWINDOW;
			}
			else if (asCommandPart == "APP_QUIT")
			{
                action = ACTION_APP_QUIT;
			}
			else if (asCommandPart.Pos(PROGRAMMABLE_BTN_STR) == 1)
			{
				AnsiString btnIdStr = asCommandPart.SubString(strlen(PROGRAMMABLE_BTN_STR)+1, asCommandPart.Length() - strlen(PROGRAMMABLE_BTN_STR));
				programmableBtnId = StrToIntDef(btnIdStr, programmableBtnId);
				if (programmableBtnId >= 0)
				{
                    action = ACTION_PROGRAMMABLE_BTN;
                }
			}
			else if (asCommandPart == "RECORD_START")
			{
				action = ACTION_RECORD_START;
			}
			else if (asCommandPart == "RECORD_PAUSE")
			{
            	action = ACTION_RECORD_PAUSE;
			}
			else if (asCommandPart.Pos(SCRIPT_STR) == 1)
			{
				script = asCommandPart.SubString(strlen(SCRIPT_STR)+1, asCommandPart.Length() - strlen(SCRIPT_STR));
			#if 0	// unnecessary? Windows removes single double quotes in parameters
				if (script[1] == '"')
				{
					if (script.Length() < 2)
						continue;
					script = script.SubString(2, script.Length()-1);
				}
				if (script[script.Length()] == '"')
				{
					if (script.Length() < 2)
						continue;
					script = script.SubString(1, script.Length()-1);
				}
			#endif
				action = ACTION_SCRIPT;
			}
			else if (asCommandPart.Pos(SCRIPT_B64_STR) == 1)
			{
				script = asCommandPart.SubString(strlen(SCRIPT_B64_STR)+1, asCommandPart.Length() - strlen(SCRIPT_B64_STR));
				script = base64_decode(script.c_str(), BASE64_ALPHABET_BASIC).c_str();
				action = ACTION_SCRIPT;
			}
			else if (asCommandPart.Pos(SCRIPT_FILE_STR) == 1)
			{
				scriptFile = asCommandPart.SubString(strlen(SCRIPT_FILE_STR)+1, asCommandPart.Length() - strlen(SCRIPT_FILE_STR));
				action = ACTION_SCRIPT_FILE;
			}
			else
			{
				if (asCallTo.Pos("sip:") == 1 && (appSettings.Integration.bDoNotUseSipPrefixForDirectIpCalls == false))
				{
					asTarget = asCallTo;
				}
				else
				{
					asTarget = CleanNumber(asCallTo);
				}
				if (asTarget.Length())
				{
					action = ACTION_CALL;
				}
			}
		}
	}
}

AnsiString CommandLine::GetProfileDir(void)
{
	AnsiString needle = "/profiledir=";
	int paramcnt = ParamCount() + 1;
	for (int i=1; i<paramcnt; i++)
	{
		AnsiString asParam = ParamStr(i);
		if (LowerCase(asParam).Pos(needle) == 1)
		{
			AnsiString dir = asParam.SubString(needle.Length()+1, asParam.Length() - needle.Length());
			return dir;
		}
	}
	return "";
}

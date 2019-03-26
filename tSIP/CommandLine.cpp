#include <vcl.h>
#pragma hdrstop

#include "CommandLine.h"
#include "Settings.h"
#include "Utils.h"
#include "Branding.h"
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
	HWND hWnd = FindWindow("TfrmMain", Branding::appName.c_str());

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
			strncpy(buf + (MAX_CMD_PARAM_LEN*i), ParamStr(i).c_str(), MAX_CMD_PARAM_LEN);
			buf[MAX_CMD_PARAM_LEN*i + MAX_CMD_PARAM_LEN-1] = '\0';
		}
	}

	if (hWnd)
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
		if (LowerCase(asParam).Pos(szCallParam) == 1)
		{
			AnsiString asCallTo =
				asParam.SubString(strlen(szCallParam)+1, asParam.Length() - strlen(szCallParam)).Trim();
		#if 0
			int i = asCallTo.Pos(":");
			if (i)
				asCallTo = asCallTo.SubString(i+1, asCallTo.Length()-i);
		#endif
			asCallTo = UrlDecode(asCallTo.c_str()).c_str();

			if (asCallTo == "HANGUP")
			{
				action = ACTION_HANGUP;
			}
			else if (asCallTo == "ANSWER")
			{
				action = ACTION_ANSWER;
			}
			else if (asCallTo == "SHOWWINDOW")
			{
				action = ACTION_SHOWWINDOW;
			}
			else if (asCallTo == "APP_QUIT")
			{
                action = ACTION_APP_QUIT;
			}
			else if (asCallTo.Pos(PROGRAMMABLE_BTN_STR) == 1)
			{
				AnsiString btnIdStr = asCallTo.SubString(strlen(PROGRAMMABLE_BTN_STR)+1, asCallTo.Length() - strlen(PROGRAMMABLE_BTN_STR));
				programmableBtnId = StrToIntDef(btnIdStr, programmableBtnId);
				if (programmableBtnId >= 0)
				{
                    action = ACTION_PROGRAMMABLE_BTN;
                }
            }
			else
			{
				if (asCallTo.Pos("sip:") == 1)
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

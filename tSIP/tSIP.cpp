//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("FormMain.cpp", frmMain);
USEFORM("FormSettings.cpp", frmSettings);
USEFORM("FormAbout.cpp", frmAbout);
USEFORM("LogUnit.cpp", frmLog);
USEFORM("FormAccount.cpp", frmAccount);
USEFORM("FormHistory.cpp", frmHistory);
USEFORM("FormButtonContainer.cpp", frmButtonContainer);
USEFORM("FormButtonEdit.cpp", frmButtonEdit);
USEFORM("FormContacts.cpp", frmContacts);
USEFORM("FormContactEditor.cpp", frmContactEditor);
USEFORM("FormContactPopup.cpp", frmContactPopup);
USEFORM("FormTrayNotifier.cpp", frmTrayNotifier);
USEFORM("FormHotkeys.cpp", frmHotkeys);
USEFORM("FormPhones.cpp", frmPhones);
USEFORM("FormContactsCsvImport.cpp", frmContactsCsvImport);
USEFORM("FormLuaScript.cpp", frmLuaScript);
USEFORM("FormTextEditor.cpp", frmTextEditor);
//---------------------------------------------------------------------------
#pragma link "re.lib"
#pragma link "rem.lib"
#pragma link "baresip.lib"
#pragma link "portaudio.lib"
#pragma link "libspeex.lib"
#pragma link "gsm.lib"
#pragma link "g722.lib"
#pragma link "webrtc_tc.lib"
#pragma link "jsoncpp.lib"
#pragma link "lua_static.lib"
//#pragma link "libopus.lib"
#pragma link "scintilla.lib"

#include "Settings.h"
#include "FormMain.h"
#include "FormContactPopup.h"
#include "CommandLine.h"
#include "WindowsMessageFilter.h"
#include "LogUnit.h"
#include "Log.h"
#include "Branding.h"

namespace
{
	AnsiString GetFullImgName(AnsiString name)
	{
		AnsiString fname;
		fname.sprintf("%s\\img\\%s", ExtractFileDir(Application->ExeName).c_str(), name.c_str());
		return fname;
	}
}

WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Branding::init();
		Application -> ShowMainForm = false;

		AnsiString dir = ExtractFileDir(Application->ExeName);
		if (chdir(dir.c_str()) != 0)
		{
			ShowMessage("Failed to set path for current directory");
		}

		AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
		appSettings.Read(asConfigFile);

		if (appSettings.frmMain.bUseCustomApplicationTitle)
		{
			Application->Title = appSettings.frmMain.customApplicationTitle;
		}
		else
		{
			Application->Title = Branding::appName;
        }

		if (appSettings.frmMain.mainIcon != "")
		{
			try
			{
				Application->Icon->LoadFromFile(GetFullImgName(appSettings.frmMain.mainIcon));
			}
			catch (...)
			{
				ShowMessage("Failed to load main icon - check configuration");
            }
		}

		if (CommandLine::Instance().Process())
		{
			// command line content forwarded to another instance
			return 0;
		}

		if (appSettings.Integration.bAddFilterWMCopyData)
		{
			/* 	Allows passing WM_COPYDATA to elevated app in Vista+.
				This would be most likely needed only for debugging (TC++ 2006 requires running with admin privileges).
			*/
			ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
		}

		Application->CreateForm(__classid(TfrmMain), &frmMain);
		Application->CreateForm(__classid(TfrmLog), &frmLog);
		Application->CreateForm(__classid(TfrmSettings), &frmSettings);
		Application->CreateForm(__classid(TfrmAbout), &frmAbout);
		Application->CreateForm(__classid(TfrmButtonEdit), &frmButtonEdit);
		Application->CreateForm(__classid(TfrmContactEditor), &frmContactEditor);
		Application->CreateForm(__classid(TfrmTrayNotifier), &frmTrayNotifier);
		Application->CreateForm(__classid(TfrmHotkeys), &frmHotkeys);
		Application->CreateForm(__classid(TfrmContactPopup), &frmContactPopup);
		Application->CreateForm(__classid(TfrmPhones), &frmPhones);
		Application->CreateForm(__classid(TfrmContactsCsvImport), &frmContactsCsvImport);
		frmContactPopup->Left = appSettings.frmContactPopup.iPosX;
		frmContactPopup->Top = appSettings.frmContactPopup.iPosY;
		frmContactPopup->Width = appSettings.frmContactPopup.iWidth;
		frmContactPopup->Height = appSettings.frmContactPopup.iHeight;

		if (appSettings.frmMain.bStartMinimizedToTray == false)
		{
			frmMain->Show();
		}
		else
		{
			ShowWindow(Application->Handle, SW_HIDE); // hide taskbar button
		}

		CLog::Instance()->SetLevel(E_LOG_TRACE);
		CLog::Instance()->callbackLog = frmLog->OnLog;
		LOG("\n===================\nApplication started\n");
		LOG("Main config file: %s\n", asConfigFile.c_str());
#if 0
		frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
		frmLog->BorderStyle = bsNone;
		frmLog->Align = alClient;
		frmLog->Parent = tsLog;
		frmLog->Visible = true;
#endif
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	CLog::Instance()->Close();	
	return 0;
}
//---------------------------------------------------------------------------

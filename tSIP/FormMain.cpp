//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormMain.h"
#include "CommandLine.h"
#include "Paths.h"
#include "Bitmaps.h"
#include "Branding.h"
#include "AppStatus.h"
#include "FormAbout.h"
#include "FormSettings.h"
#include "FormSettingsPatch.h"
#include "FormAccount.h"
#include "FormHistory.h"
#include "FormButtonContainer.h"
#include "ButtonContainers.h"
#include "FormButtonEdit.h"
#include "History.h"
#include "FormContacts.h"
#include "Contacts.h"
#include "FormContactPopup.h"
#include "FormContactEditor.h"
#include "FormTrayNotifier.h"
#include "LogUnit.h"
#include "Log.h"
#include "UaMain.h"
#include "UaGlobals.h"
#include "Calls.h"
#include "UaCustomRequests.h"
#include "ControlQueue.h"
#include "Callback.h"
#include "CallbackQueue.h"
#include "CustomDateUtils.hpp"
#include "ProgrammableButton.h"
#include "ProgrammableButtons.h"
#include "ScriptExec.h"
#include "HotKeys.h"
#include "FormMessage.h"
#include "PhoneInterface.h"
#include "FormContactsCsvImport.h"
#include "FormLuaScript.h"
#include "FormScriptPluginVariables.h"
#include "Troubleshooting.h"
#include "FormTroubleshooting.h"
#include "SIMPLE_Messages.h"
#include "Globals.h"
#include "PortaudioLock.h"
#include "Translate.h"
#include "common\Utilities.h"
#include "common\ScopedLock.h"
#include "common\TelecomUtils.h"
#include "common\Unicode.h"
#include "common\OS.h"
#include "common\TrayIcon.h"
#include "common\base64.h"

#include <Clipbrd.hpp>
#include <assert.h>
#include <stdio.h>
#include <math.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

TfrmMain *frmMain;

namespace {
	AnsiString asTransferHint = "Transfer to... [Enter]";
	bool useOwnTrayIcon = false;

	struct PagingTx {
		bool active;
		int state;				///< as in Callback
		PagingTx(void):
			active(false),
			state(0)
		{}
	} pagingTx;
	struct Registration {
		int state;
		Registration(void):
			state(0)
		{}
	} registration;

	Contacts::Entry *lastContactEntry = NULL;
	void ShowContactPopup(Contacts::Entry *entry)
	{
        assert(entry);
		if (frmContactPopup->isNoteModified() && !appSettings.Contacts.storeNoteInSeparateFile)
		{
            frmContactPopup->UpdateEntry();
			/** \todo Limit write frequency
			*/
			contacts.Write();
		}
		frmContactPopup->SetData(entry, appSettings.Contacts.storeNoteInSeparateFile);
		frmContactPopup->Visible = true;	// steals focus
		//ShowWindow(frmContactPopup->Handle,SW_SHOWNOACTIVATE);
		//SetWindowPos(frmContactPopup->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		if (appSettings.Scripts.onContactNoteOpen != "")
		{
			AnsiString asScriptFile;
			bool handled = true;
			asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onContactNoteOpen.c_str());
			frmMain->RunScriptFile(SCRIPT_SRC_ON_CONTACT_NOTE_OPEN, -1, asScriptFile.c_str(), handled);
		}
	}

	void OpenContactFile(Contacts::Entry *entry)
	{
		if (entry->file != "")
		{
			ShellExecute(NULL, "open", entry->file.c_str(), NULL, NULL, SW_SHOWNORMAL); 
		}
	}

	AnsiString GetPeerName(AnsiString displayName)
	{
		if (appSettings.Display.bDecodeUtfDisplayToAnsi)
			return ::Utf8ToAnsi(displayName);
		else
			return displayName;
	}

	AnsiString GetContactsFileName(void)
	{
		AnsiString asContactsFile;
		if (appSettings.Contacts.file != "")
		{
			asContactsFile = appSettings.Contacts.file;
		}
		else
		{
			asContactsFile.sprintf("%s\\%s_contacts.json", Paths::GetProfileDir().c_str(),
				ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
		}
		return asContactsFile;
	}

	void UpdateContactsFile(void)
	{
		AnsiString asContactsFile = GetContactsFileName();

		contacts.SetFilename(asContactsFile);
		contacts.Read();
		if (frmContactPopup)
		{
			frmContactPopup->Close();
		}
		if (frmContactEditor)
		{
        	frmContactEditor->Close();
		}
	}

	void SetStatus(AnsiString text)
	{
		SetAppStatus("FormMain", 0, text);
	}

	void* GetDisplayParentHandle(void)
	{
		if (appSettings.video.enabled)
		{
			switch (appSettings.video.displayParentType)
			{
			case VideoConf::DISPLAY_PARENT_NONE:
				break;
			case VideoConf::DISPLAY_PARENT_BUTTON:
			{
				TProgrammableButton *btn = buttons.GetBtn(appSettings.video.displayParentId);
				if (btn)
					return btn->Handle;
				break;
			}
			default:
				break;
			}
		}
		return NULL;
	}
}

void TfrmMain::TranslateForm(void* obj)
{
	TfrmMain *frm = reinterpret_cast<TfrmMain*>(obj);
	assert(frm);
	TRANSLATE_TMP("TfrmMain.btnMakeCall", frm->btnMakeCall->Caption);
	TRANSLATE_TMP("TfrmMain.btnHangup", frm->btnHangup->Caption);
	TRANSLATE_TMP("TfrmMain.tsDialpad", frm->tsDialpad->Caption);
	TRANSLATE_TMP("TfrmMain.tsContacts", frm->tsContacts->Caption);
	TRANSLATE_TMP("TfrmMain.tsHistory", frm->tsHistory->Caption);
	TRANSLATE_TMP("TfrmMain.miFile", frm->miFile->Caption);
	TRANSLATE_TMP("TfrmMain.miView", frm->miView->Caption);
	TRANSLATE_TMP("TfrmMain.miSettings", frm->miSettings->Caption);
	TRANSLATE_TMP("TfrmMain.miTools", frm->miTools->Caption);
	TRANSLATE_TMP("TfrmMain.miHelp", frm->miHelp->Caption);
	TRANSLATE_TMP("TfrmMain.miMinimizeTray", frm->miMinimizeTray->Caption);
	TRANSLATE_TMP("TfrmMain.miExit", frm->miExit->Caption);
	TRANSLATE_TMP("TfrmMain.miViewLog", frm->miViewLog->Caption);
	TRANSLATE_TMP("TfrmMain.miCommonSettings", frm->miCommonSettings->Caption);
	TRANSLATE_TMP("TfrmMain.miSettingsPatch", frm->miSettingsPatch->Caption);
	TRANSLATE_TMP("TfrmMain.miPatchButtonSettings", frm->miPatchButtonSettings->Caption);
	TRANSLATE_TMP("TfrmMain.miImportContactsFromCsv", frm->miImportContactsFromCsv->Caption);
	TRANSLATE_TMP("TfrmMain.miImportContactsFromXml", frm->miImportContactsFromXml->Caption);
	TRANSLATE_TMP("TfrmMain.miClearCallsHistory", frm->miClearCallsHistory->Caption);
	TRANSLATE_TMP("TfrmMain.miRefreshTranslationFromFile", frm->miRefreshTranslationFromFile->Caption);
	TRANSLATE_TMP("TfrmMain.miScripting", frm->miScripting->Caption);
	TRANSLATE_TMP("TfrmMain.miScriptPluginVariables", frm->miScriptPluginVariables->Caption);	
	TRANSLATE_TMP("TfrmMain.miMessages", frm->miMessages->Caption);
	TRANSLATE_TMP("TfrmMain.miTroubleshooting", frm->miTroubleshooting->Caption);
	TRANSLATE_TMP("TfrmMain.miAbout", frm->miAbout->Caption);
	Translate("TfrmMain.edTransferHint", asTransferHint);
}

//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner),
	initialScaling(1.0),
	trIcon(NULL),
	muteRing(false),
	notificationIconState(false),
	errorIconState(false),
	appState(Callback::APP_STATE_UNKNOWN),
	appStartFailCount(0)
{
	srand(time(0));
	RegisterTranslationCb(this, TranslateForm);
	lbl2ndParty->Caption = "";
	lbl2ndPartyDesc->Caption = "";
	lblCallState->Caption = "";
	frmHistory = new TfrmHistory(this->tsHistory, &history, appSettings.history, &MakeCall, &OnPhonebookEdit, &OnHttpQuery);
	frmHistory->Scale(appSettings.gui.scalingPct);
	frmHistory->Parent = tsHistory;
	frmHistory->Visible = true;

	frmContacts = new TfrmContacts(this->tsContacts, &contacts, &MakeCall);
	frmContacts->Scale(appSettings.gui.scalingPct);
	frmContacts->Parent = tsContacts;
	frmContacts->Visible = true;
	frmContacts->FilterUsingNote(appSettings.Contacts.filterUsingNote);
	frmContacts->StoreNoteInSeparateFile(appSettings.Contacts.storeNoteInSeparateFile);

	if (appSettings.frmMain.bNoTrayIcon == false)
	{
		trIcon = new TrayIcon(this);
		trIcon->OnLeftBtnDown = OnTrayIconLeftBtnDown;
		trIcon->SetPopupMenu(popupTray);
		trIcon->SetIcon(Application->Icon);
		trIcon->ShowInTray(true);
	}

	TfrmLuaScript::SetCallbackRunScript(&RunScript);
	PhoneInterface::SetCallbackRunScript(&RunScript);

	if (appSettings.frmMain.trayNotificationImage != "")
	{
		Graphics::TBitmap *bmp = new Graphics::TBitmap();
		try
		{
			bmp->LoadFromFile(Paths::GetFullImgName(appSettings.frmMain.trayNotificationImage));
			imgListIcons->Clear();
			imgListIcons->Add(bmp, NULL);
		}
		catch (...)
		{
			ShowMessage("Failed to load image for tray notification - check configuration");
		}
		delete bmp;
	}

	pnlMain->DoubleBuffered = true;	// removes call timer label flickering on update
	pnlCallControls->DoubleBuffered = true;
	pnlDialpad->DoubleBuffered = true;

	SetMainWindowLayout(appSettings.frmMain.layout);

	Application->OnRestore = OnRestore;
}

__fastcall TfrmMain::~TfrmMain()
{
	UnregisterTranslationCb(this);
	if (trIcon)
	{
		delete trIcon;
		trIcon = NULL;
	}
	PortaudioLockShutdown();
	FreeBitmapResources();
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
	LoadTranslation();

	if (appSettings.frmMain.bUseCustomCaption)
	{
		this->Caption = appSettings.frmMain.customCaption;
	}
	else
	{
		this->Caption = Branding::appName;
	}
	tsDialpad->Visible = !appSettings.frmMain.bHideDialpad;
	tsDialpad->TabVisible = !appSettings.frmMain.bHideDialpad;
	pnlCallControls->Visible = !appSettings.frmMain.bHideCallPanel;
	pnlMain->Visible = !appSettings.frmMain.bHideMainPanel;

#if 0 // this MIGHT work for scaling scrollbar width - not working
	TNonClientMetrics NCMet;
	memset(&NCMet, 0, sizeof(NCMet));
	// get the current metrics
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NCMet), &NCMet, 0);
	// set the new metrics
	NCMet.iScrollWidth = 30;
	SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(NCMet), &NCMet, SPIF_SENDCHANGE);
#endif
	this->ScaleBy(appSettings.gui.scalingPct, 100);

	if (appSettings.frmMain.bHideMouseCursor)
	{
		Screen->Cursor = crNone;
	}

	UpdateContactsFile();

	contacts.addObserver(*this);

	/** \note must be initialized AFTER contacts */
	AnsiString asHistoryFile;
	asHistoryFile.sprintf("%s\\%s_history.json", Paths::GetProfileDir().c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	history.SetFilename(asHistoryFile);
	history.Read(&OnGetContactName);

	initialScaling = static_cast<double>(appSettings.gui.scalingPct) / 100;
	UpdateSize();
	btnSpeedDialPanel->Visible = !appSettings.frmMain.bHideSpeedDialToggleButton;
	UpdateDialpad();

	this->Top = appSettings.frmMain.iPosY;
	this->Left = appSettings.frmMain.iPosX;
	if (appSettings.frmMain.bAlwaysOnTop)
		this->FormStyle = fsStayOnTop;
	else
		this->FormStyle = fsNormal;
	if (appSettings.frmMain.bWindowMaximized)
		this->WindowState = wsMaximized;
	UpdateLogConfig();

	SetSpeedDial(appSettings.frmMain.bSpeedDialVisible);
	UpdateBitmaps();
	miSettings->Visible = !appSettings.frmMain.bHideSettings;
	miView->Visible = !appSettings.frmMain.bHideView;
	miTools->Visible = !appSettings.frmMain.bHideTools;
	miHelp->Visible = !appSettings.frmMain.bHideHelp;

	trbarSoftvolMic->Position = trbarSoftvolMic->Max + trbarSoftvolMic->Min - appSettings.uaConf.audioSoftVol.tx;
	trbarSoftvolSpeaker->Position = trbarSoftvolSpeaker->Max + trbarSoftvolSpeaker->Min - appSettings.uaConf.audioSoftVol.rx;

	if (appSettings.frmMain.bKioskMode)
	{
		SetKioskMode(appSettings.frmMain.bKioskMode);
	}
	else
	{
		if (appSettings.frmMain.bHideStatusBar)
		{
			StatusBar->Visible = false;
		}
		if (appSettings.frmMain.bHideMainMenu)
		{
			this->Menu = NULL;
		}
		if (appSettings.frmMain.bBorderless)
		{
        	this->BorderStyle = bsNone;
		}
	}

	cbCallURI->Clear();
	UpdateCallHistory();
}
//---------------------------------------------------------------------------

void TfrmMain::InitButtons(void)
{
	AnsiString asButtonsFile;
	asButtonsFile.sprintf("%s\\%s_buttons.json", Paths::GetProfileDir().c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	buttons.SetFilename(asButtonsFile);
	buttons.Read();
	buttons.UpdateContacts(appSettings.uaConf.contacts);

	buttons.SetScalingPercentage(appSettings.gui.scalingPct);

	{
		const int id = BUTTON_CONTAINER_DIALPAD;
		TfrmButtonContainer *frmButtonContainerBasic = new TfrmButtonContainer(
			this->pnlDialpad,
			buttons,
			id, true,
			this->pnlDialpad->Width, this->pnlDialpad->Height, appSettings.gui.scalingPct,
			&OnSetKeepForeground,
			false, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
		frmButtonContainerBasic->Parent = this->pnlDialpad;
		frmButtonContainerBasic->ApplyConfig();
		frmButtonContainerBasic->Visible = true;
		frmButtonContainers[id] = frmButtonContainerBasic;
	}
	
	{
		const int id = BUTTON_CONTAINER_MAIN;
		TfrmButtonContainer *& container = frmButtonContainers[id];
		container = new TfrmButtonContainer(this,
			buttons,
			id, false,
			300, 0, appSettings.gui.scalingPct,
			&OnSetKeepForeground,
			appSettings.frmSpeedDial.showStatus, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
		container->Parent = this;
		container->ApplyConfig();
		container->Align = alClient;
		container->Visible = true;
		container->SendToBack();
	}

	{
		const int id = BUTTON_CONTAINER_CALL_PANEL;
		TfrmButtonContainer *& container = frmButtonContainers[id];
		container = new TfrmButtonContainer(this->pnlCallControls,
			buttons,
			id, true,
			this->pnlCallControls->Width, this->pnlCallControls->Height, appSettings.gui.scalingPct,
			&OnSetKeepForeground,
			false, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
		container->Parent = this->pnlCallControls;
		container->ApplyConfig();
		//container->Align = alClient;
		container->Visible = true;
	}

	{
		const int id = BUTTON_CONTAINER_TRAY_NOTIFIER;
		TfrmButtonContainer *& container = frmButtonContainers[id];
		container = new TfrmButtonContainer(frmTrayNotifier,
			buttons,
			id, true,
			300, 0, appSettings.gui.scalingPct,
			&OnSetKeepForeground,
			false, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
		container->Parent = frmTrayNotifier;
		container->ApplyConfig();
		container->SendToBack();
		container->Align = alClient;
		container->Visible = true;
	}

	buttons.Create(this, appSettings.gui.scalingPct,
		&OnProgrammableBtnClick,
		&OnProgrammableBtnMouseUpDown,
		&OnSetKeepForeground,
		&OnRestartUa
		);
	buttons.UseContextMenu(appSettings.frmMain.bSpeedDialPopupMenu);

	UpdateAutoAnswer();
	Calls::OnButtonConfigChange();
}

void TfrmMain::Finalize(void)
{
	static bool finalized = false;
	if (finalized)
		return;
	finalized = true;

	LOG("Finalizing...\n");

	tmrScript->Enabled = false;
	tmrScript2->Enabled = false;

	hotKeys.Unregister(Handle);

	PhoneInterface::Close();

	ScriptExec::BreakAllScripts();

	SIMPLE_Messages::CloseAllWindows();

	// inverting trackbars
	appSettings.uaConf.audioSoftVol.tx = trbarSoftvolMic->Max - trbarSoftvolMic->Position + trbarSoftvolMic->Min;
	appSettings.uaConf.audioSoftVol.rx = trbarSoftvolSpeaker->Max - trbarSoftvolSpeaker->Position + trbarSoftvolSpeaker->Min;
	appSettings.frmMain.bWindowMaximized = (this->WindowState == wsMaximized);
	if (!appSettings.frmMain.bWindowMaximized)
	{
		// width and height values are meaningless is window is maximized, storing only position
		appSettings.frmMain.iPosY = this->Top;
		appSettings.frmMain.iPosX = this->Left;
	}

	appSettings.frmContactPopup.iPosX = frmContactPopup->Left;
	appSettings.frmContactPopup.iPosY = frmContactPopup->Top;
	appSettings.frmContactPopup.iWidth = frmContactPopup->Width;
	appSettings.frmContactPopup.iHeight = frmContactPopup->Height;

	appSettings.history.listColumnWidths = frmHistory->GetColumnWidths();

	appSettings.Logging.windowWidth = frmLog->Width;
	appSettings.Logging.windowHeight = frmLog->Height;

	appSettings.Write(Paths::GetConfig());
	if (appSettings.history.noStoreToFile == false)
	{
		history.Write();
    }

	frmContactPopup->Close();
	if (frmContactPopup->isNoteModified() && !appSettings.Contacts.storeNoteInSeparateFile)
	{
		contacts.Write();
	}

	// no need to write buttons configuration here

	ScriptExec::WaitWhileAnyRunning(2000);
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::actShowAboutExecute(TObject *Sender)
{
	if (frmAbout == NULL)
		Application->CreateForm(__classid(TfrmAbout), &frmAbout);
	frmAbout->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowSettingsExecute(TObject *Sender)
{
	Settings prev = appSettings;	// keep track what is changed
	if (frmSettings == NULL)
	{
		Application->CreateForm(__classid(TfrmSettings), &frmSettings);
	}
	frmSettings->ShowModal();
	appSettings.UpdateFromText(Branding::fixedSettings);	// not checking status: this may fail, resource may be empty
	UpdateSettings(prev);
}
//---------------------------------------------------------------------------

int TfrmMain::UpdateSettingsFromJson(AnsiString json)
{
	Settings prev = appSettings;	// keep track what is changed
	int status = appSettings.UpdateFromText(json);
	if (status != 0)
		return status;
	appSettings.UpdateFromText(Branding::fixedSettings);	// not checking status: this may fail, resource may be empty
	UpdateSettings(prev);
	return status;
}

void TfrmMain::UpdateSettings(const Settings &prev)
{
	UpdateSize();
	if (appSettings.frmMain.bKioskMode)
	{
		SetKioskMode(appSettings.frmMain.bKioskMode);
	}	
	if (prev.dialpad != appSettings.dialpad)
	{
		UpdateDialpad();
	}
	btnSpeedDialPanel->Visible = !appSettings.frmMain.bHideSpeedDialToggleButton;
	if (prev.frmMain.bSpeedDialPopupMenu != appSettings.frmMain.bSpeedDialPopupMenu)
	{
		buttons.UseContextMenu(appSettings.frmMain.bSpeedDialPopupMenu);
	}
	if (prev.Translation.language != appSettings.Translation.language)
	{
		LoadTranslation();
	}

	// modify application title and main window caption only if config changes,
	// allowing to keep text set by Lua API or other methods
	if ((prev.frmMain.bUseCustomApplicationTitle != appSettings.frmMain.bUseCustomApplicationTitle) ||
		(appSettings.frmMain.bUseCustomApplicationTitle && (prev.frmMain.customApplicationTitle != appSettings.frmMain.customApplicationTitle)))
	{
		if (appSettings.frmMain.bUseCustomApplicationTitle)
		{
			Application->Title = appSettings.frmMain.customApplicationTitle;
		}
		else
		{
			Application->Title = Branding::appName;
		}
	}
	if ((prev.frmMain.bUseCustomCaption != appSettings.frmMain.bUseCustomCaption) ||
		(appSettings.frmMain.bUseCustomCaption && (prev.frmMain.customCaption != appSettings.frmMain.customCaption)))
	{
		if (appSettings.frmMain.bUseCustomCaption)
		{
			this->Caption = appSettings.frmMain.customCaption;
		}
		else
		{
			this->Caption = Branding::appName;
		}
	}

	if (appSettings.uaConf != prev.uaConf)
	{
		SetStatus("Restarting UA...");
		Ua::Instance().Restart();
	}
	UA->UpdateVolume();
	if (appSettings.uaConf.logMessages != prev.uaConf.logMessages ||
        appSettings.uaConf.logMessagesOnlyFirstLine != prev.uaConf.logMessagesOnlyFirstLine)
	{
		UA->SetMsgLogging(appSettings.uaConf.logMessages, appSettings.uaConf.logMessagesOnlyFirstLine);
	}
	if (appSettings.uaConf.logAubuf != prev.uaConf.logAubuf)
	{
    	UA->SetAubufLogging(appSettings.uaConf.logAubuf);
	}
	if (appSettings.frmMain.bKioskMode != prev.frmMain.bKioskMode)
	{
		SetKioskMode(appSettings.frmMain.bKioskMode);
	}
	if (!appSettings.frmMain.bKioskMode)
	{
		StatusBar->Visible = !appSettings.frmMain.bHideStatusBar;
		this->Menu = appSettings.frmMain.bHideMainMenu ? NULL : MainMenu;
		this->BorderStyle = appSettings.frmMain.bBorderless ? bsNone : bsSingle;
	}
	if (appSettings.frmMain.bHideMouseCursor)
	{
		Screen->Cursor = crNone;
	}
	else
	{
		Screen->Cursor = crDefault;
	}
	tsDialpad->Visible = !appSettings.frmMain.bHideDialpad;
	tsDialpad->TabVisible = !appSettings.frmMain.bHideDialpad;
	pnlCallControls->Visible = !appSettings.frmMain.bHideCallPanel;
	pnlMain->Visible = !appSettings.frmMain.bHideMainPanel;
	// enable/disable popup menu
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		if (container)
		{
			container->UpdatePopupSettings();
		}
	}
	if (appSettings.frmMain.bAlwaysOnTop)
		this->FormStyle = fsStayOnTop;
	else
		this->FormStyle = fsNormal;
	if (appSettings.frmMain.layout != prev.frmMain.layout)
	{
		SetMainWindowLayout(appSettings.frmMain.layout);
	}
	if (appSettings.frmMain.dialComboboxOrder != prev.frmMain.dialComboboxOrder)
	{
		UpdateCallHistory();
	}
	UpdateLogConfig();
	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
	if (frmLog->Visible)
	{
    	frmLog->UpdateUi();
	}

	if (prev.hotKeyConf != appSettings.hotKeyConf)
	{
		RegisterGlobalHotKeys();
	}
	if (prev.Contacts.file != appSettings.Contacts.file)
	{
		UpdateContactsFile();
	}
	frmContacts->FilterUsingNote(appSettings.Contacts.filterUsingNote);
	frmContacts->StoreNoteInSeparateFile(appSettings.Contacts.storeNoteInSeparateFile);

	if (prev.history != appSettings.history)
	{
		frmHistory->UpdateConf(prev.history);
	}

	UpdateBitmaps();
	/** \todo bug? use prev settings */
	frmTrayNotifier->ScaleBy(100, appSettings.trayNotifier.scalingPct);
	frmTrayNotifier->ScaleBy(appSettings.trayNotifier.scalingPct, 100);
	frmTrayNotifier->Width = appSettings.trayNotifier.iWidth * appSettings.trayNotifier.scalingPct / 100;
	frmTrayNotifier->Height = appSettings.trayNotifier.iHeight * appSettings.trayNotifier.scalingPct / 100;

	buttons.SetSaveAllSettings(appSettings.frmSpeedDial.saveAllSettings);

	miSettings->Visible = !appSettings.frmMain.bHideSettings;
	miView->Visible = !appSettings.frmMain.bHideView;
	miTools->Visible = !appSettings.frmMain.bHideTools;
	miHelp->Visible = !appSettings.frmMain.bHideHelp;

	PhoneInterface::SetCfg(appSettings.phoneConf);

	tmrScript->Interval = appSettings.Scripts.timer;
	tmrScript->Enabled = true;

	tmrScript2->Interval = appSettings.Scripts.timer2;
	tmrScript2->Enabled = true;

	appSettings.Write(Paths::GetConfig());
}

int TfrmMain::UpdateButtonsFromJson(AnsiString json)
{
	std::vector<ButtonConf> prevConf = buttons.btnConf;
	int status = buttons.ReadFromString(json);
	if (status != 0)
	{
		return status;
	}
	bool changed = false;
	bool restartUa = false;
	for (unsigned int i=0; i<buttons.btnConf.size(); i++)
	{
		const ButtonConf &conf = buttons.btnConf[i];
		const ButtonConf &prev = prevConf[i];
		if (conf != prev)
		{
			changed = true;
			if (conf.UaRestartNeeded(prev))
				restartUa = true;
			buttons.SetConfig(i, conf);
		}
	}
	if (changed)
	{
		int status = buttons.Write();
		if (status != 0)
		{
        	LOG("Failed to write button configuration!\n");
		}
		Calls::OnButtonConfigChange();
	}
	if (restartUa)
	{
		buttons.UpdateContacts(appSettings.uaConf.contacts);
		Ua::Instance().Restart();
	}
	return 0;
}

void __fastcall TfrmMain::FormDestroy(TObject *Sender)
{
	UA->Destroy();
	UA_CB->Destroy();
	buttons.Destroy();
	CLog::Instance()->Destroy();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowLogExecute(TObject *Sender)
{
	if (!frmLog->Visible)
		frmLog->Show();
	frmLog->WindowState = wsNormal;
	frmLog->BringToFront();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::tmrStartupTimer(TObject *Sender)
{
	tmrStartup->Enabled = false;

	// make sure window position is not outside of available monitors
	OnRestore(NULL);

	{
		// trying to fight with inconsistent state of tray notifier when trying to show it without activation
		frmTrayNotifier->Show();
		frmTrayNotifier->Hide();
	}

	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
	
	edTransfer->Enabled = true;
	edTransfer->Text = asTransferHint;
	edTransfer->Hint = asTransferHint;

    frmTrayNotifier->Caption = Branding::appName;
	frmTrayNotifier->OnHangup = Hangup;
	frmTrayNotifier->OnAnswer = Answer;
	frmTrayNotifier->ScaleBy(appSettings.trayNotifier.scalingPct, 100);
	frmTrayNotifier->Width = appSettings.trayNotifier.iWidth * appSettings.trayNotifier.scalingPct / 100;
	frmTrayNotifier->Height = appSettings.trayNotifier.iHeight * appSettings.trayNotifier.scalingPct / 100;

	buttons.SetSaveAllSettings(appSettings.frmSpeedDial.saveAllSettings);

	if (appSettings.frmMain.bShowSettingsIfAccountSettingIsHidden)
	{
		for (unsigned int i=0; i<appSettings.uaConf.accounts.size(); i++)
		{
			if (appSettings.uaConf.accounts[i].isAnySettingHidden())
			{
				actShowSettingsExecute(NULL);
				break;
			}
		}
	}

    PortaudioLockInit();

	Ua::Instance().Start();
	SetStatus("Initializing...");

    //LOG("Registering hotkeys...\n");
	RegisterGlobalHotKeys();

	FocusCbCallUri();

	AnsiString dir = Paths::GetProfileDir();
	PhoneInterface::callbackKey = OnPhoneKey;
	PhoneInterface::callbackPagingTx = OnPhonePagingTx;
	PhoneInterface::callbackClearDial = OnPhoneClearDial;
	PhoneInterface::callbackGetNumberDescription = OnGetNumberDescription;
	PhoneInterface::trayPopupMenu = popupTray;

	PhoneInterface::EnumerateDlls(dir + "\\phone");
	PhoneInterface::UpdateProfileDir(dir);
	PhoneInterface::SetCfg(appSettings.phoneConf);
	PhoneInterface::UpdateRegistrationState(0);
	PhoneInterface::UpdateCallState(0, "");
	PhoneInterface::UpdatePagingTxState(0);	

	if (appSettings.Scripts.onStartup != "")
	{
		AnsiString asScriptFile;
		bool handled = true;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onStartup.c_str());
		RunScriptFile(SCRIPT_SRC_ON_STARTUP, -1, asScriptFile.c_str(), handled);
	}

	tmrScript->Interval = appSettings.Scripts.timer;
	tmrScript->Enabled = true;

	tmrScript2->Interval = appSettings.Scripts.timer2;
	tmrScript2->Enabled = true;
}
//---------------------------------------------------------------------------

void TfrmMain::RegisterGlobalHotKeys(void)
{
	int rc = hotKeys.RegisterGlobal(appSettings.hotKeyConf, Handle);
	if (rc > 0)
	{
		AnsiString msg;
		msg.sprintf("Failed to register %d global hotkey(s), see log for details", rc);
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONINFORMATION);
	}
}

void TfrmMain::UpdateCallHistory(void)
{
	const std::deque<History::Entry>& entries = history.GetEntries();
	std::set<std::string> numbers;
	std::vector<std::string> numbersByDate;
	for (unsigned int i=0; i<std::min(100u, entries.size()); i++)
	{
		const History::Entry &entry = entries[i];
		// add only outgoing calls
		if (entry.incoming == false)
		{
			if (numbers.find(entry.uri.c_str()) == numbers.end())
			{
				numbers.insert(entry.uri.c_str());
				numbersByDate.push_back(entry.uri.c_str());
			}
		}
	}
	if (appSettings.frmMain.dialComboboxOrder == Settings::_frmMain::DialComboboxOrderByNumber)
	{
		std::set<std::string>::iterator iter;
		for (iter = numbers.begin(); iter != numbers.end(); ++iter)
		{
			cbCallURI->Items->Add(iter->c_str());
		}
	}
	else if (appSettings.frmMain.dialComboboxOrder == Settings::_frmMain::DialComboboxOrderByTime)
	{
		for (unsigned int i=0; i<numbersByDate.size(); i++)
		{
        	cbCallURI->Items->Add(numbersByDate[i].c_str());
		}
	}
	else
	{
		assert(!"Unhandled dialCombobox sorting order!");
	}
}

void TfrmMain::ShowTrayNotifier(unsigned int callUid, AnsiString description, AnsiString uri, bool incoming)
{
	frmTrayNotifier->SetData(callUid, description, uri, incoming);
	frmTrayNotifier->ShowWithoutFocus();
}

void TfrmMain::HideTrayNotifier(void)
{
	frmTrayNotifier->HideWindow();
}

void __fastcall TfrmMain::btnMakeCallClick(TObject *Sender)
{
	if (pagingTx.active)
		return;
	Call* call = Calls::GetCurrentCall();
	if (call == NULL)
	{
		AnsiString target = cbCallURI->Text.Trim();
		if (target != "")
		{
			unsigned int callUid;
			MakeCall(target, callUid);
		}
	}
	else if (call->incoming)
	{
		Answer(call->uid);
	}
}
//---------------------------------------------------------------------------

int TfrmMain::MakeCall(AnsiString target, unsigned int &callUid)
{
	if (appState == Callback::APP_INIT_FAILED || appState == Callback::APP_START_FAILED)
	{
        LOG("Cannot make call: app not started properly\n");
		return -1;
	}

	Call* newCall = Calls::Alloc();
	Call& call = *newCall;

	callUid = call.uid;
	call.timestamp = Now();
	call.incoming = false;
	call.uri = target;
	call.initialTarget = call.uri;
	call.dialString = cbCallURI->Text;
	call.SetInitialState(Callback::CALL_STATE_OUTGOING);

	int btnId;
	if (Calls::AssignLineButton(newCall, true, btnId) != 0)
	{
		Calls::RemoveByUid(newCall->uid);
		return -1;
	}

	if (appSettings.frmMain.bShowWhenMakingCall)
	{
		ApplicationShow(true);	///< \todo Focused / not focused setting
	}

	if (appSettings.Scripts.onMakeCall != "")
	{
		AnsiString asScriptFile;
		bool handled = false;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onMakeCall.c_str());
		// this script may change initial target implementing SIP originate function
		RunScriptFile(SCRIPT_SRC_ON_MAKING_CALL, call.uid, asScriptFile.c_str(), handled);
		if (handled)
		{
			return 0;
		}
	}

	call.displayParentHandle = GetDisplayParentHandle();
	UA->Call(0, call.uid, call.initialTarget, appSettings.Calls.extraHeaderLines, appSettings.video.enabled, call.displayParentHandle);
	return 0;	
}

void __fastcall TfrmMain::btnHangupClick(TObject *Sender)
{
	Call* call = Calls::GetCurrentCall();
	if (call)
	{
		Hangup(call->uid);
	}
	else if (pagingTx.active)
	{
		UA->Hangup(Call::INVALID_UID, 200, "");
	}
}
//---------------------------------------------------------------------------

void TfrmMain::Hangup(unsigned int callUid, int sipCode, AnsiString reason)
{
	Call *c = Calls::FindByUid(callUid);
	if (c == NULL)
		return;
	c->disconnecting = true;
	if (c->connected)
	{
		c->sipReason = "Local hangup";
	}
	else
	{
		c->sipReason.sprintf("Local hangup, %d / %s", sipCode, reason.c_str());
	}
	UA->Hangup(callUid, sipCode, reason);
}

void TfrmMain::Answer(unsigned int callUid)
{
	Call *call = Calls::FindByUid(callUid);
	if (call == NULL)
		return;
	if (call->incoming)
	{
		call->displayParentHandle = GetDisplayParentHandle();
		UA->Answer(callUid, "", "", appSettings.video.enabled, call->displayParentHandle);
		if (appSettings.frmMain.bShowWhenAnsweringCall)
		{
			if (!Visible)
			{
                ToggleVisibility();
			}
		}
		if (appSettings.trayNotifier.hideWhenAnsweringCall)
		{
			frmTrayNotifier->HideWindow();
        }
	}
	else
	{
    	LOG("Answer: call with UID = %u is not in incoming state, current state = %d\n", callUid, static_cast<int>(call->GetState()));
	}
}

std::string TfrmMain::OnGetDial(void)
{
	return cbCallURI->Text.c_str();
}

void TfrmMain::OnSetDial(std::string number)
{
	cbCallURI->Text = number.c_str();
}

int TfrmMain::OnGetStreamingState(void)
{
	return pagingTx.state;
}

int TfrmMain::OnGetRegistrationState(void)
{
	return registration.state;
}

void TfrmMain::OnSetTrayIcon(const char* file)
{
	if (file != NULL && file[0] != '\0' && trIcon)
	{
		useOwnTrayIcon = true;
		Graphics::TBitmap *bmp = new Graphics::TBitmap();
		try
		{
			bmp->LoadFromFile(Paths::GetFullImgName(file));
			imgListIcons->Clear();
			imgListIcons->Add(bmp, NULL);
			trIcon->SetIcon(imgListIcons, 0);
		}
		catch (...)
		{
			AnsiString msg;
			msg.sprintf("Failed to load icon bitmap (%s)", file);
			ShowMessage(msg);
		}
		delete bmp;
	}
}

void TfrmMain::OnSetKeepForeground(bool state)
{
#if 0
	if (state == false)
	{
		tmrHideOtherWindow->Enabled = false;
		tmrBringToFront->Enabled = false;
	}
	else
	{
		tmrHideOtherWindow->Enabled = appSettings.frmMain.bHideOtherWindow;
		tmrBringToFront->Enabled = true;
	}
#endif
}

int TfrmMain::OnPluginSendMessageText(const char* dllName, const char* text)
{
	return PhoneInterface::SendMessageText(dllName, text);
}

int TfrmMain::OnPluginEnable(const char* dllName, bool state)
{
	bool changed = false;
	std::list<PhoneConf>::iterator iter;

	if (state)
	{
		for (iter = appSettings.phoneConf.begin(); iter != appSettings.phoneConf.end(); ++iter)
		{
			if (iter->dllName == dllName)
			{
				return -1;
			}
		}
		PhoneConf pconf;
		pconf.dllName = dllName;
		appSettings.phoneConf.push_back(pconf);
		changed = true;
	}
	else
	{
		for (iter = appSettings.phoneConf.begin(); iter != appSettings.phoneConf.end(); ++iter)
		{
			if (iter->dllName == dllName)
			{
				appSettings.phoneConf.erase(iter);
				changed = true;
				break;
			}
		}
	}

	if (changed)
	{
		PhoneInterface::SetCfg(appSettings.phoneConf);
		appSettings.Write(Paths::GetConfig());
		return 0;
	}
	return -1;
}

int TfrmMain::OnRecordStart(unsigned int callUid, const char* file, int channels, int side, int fileFormat, unsigned int bitrate)
{
	if (appSettings.uaConf.recording.enabled == false)
	{
		LOG("OnRecordStart: recording is not enabled in configuration!\n");
		return -1;
	}
	Call *call = Calls::FindByUid(callUid);
	if (call == NULL)
	{
		LOG("OnRecordStart: call with UID %u not found\n", callUid);
		return -2;
	}
	if (call->connected == false && call->progress == false)
	{
		LOG("OnRecordStart: call %u with no active media\n", callUid);
		return -2;
	}
	UA->Record(callUid, file, channels, side, fileFormat, bitrate);
	call->recordFile = file;
	call->recording = true;
	return 0;
}

void TfrmMain::MainMenuShow(bool state)
{
	this->Menu = (state)?(MainMenu):(NULL);
}

void TfrmMain::ApplicationShow(bool focused)
{
	if (focused)
	{
		if (!Visible)
		{
			ToggleVisibility();
		}
		else
		{
			Application->Restore();
			SetActiveWindow (Handle);
			SetForegroundWindow (Handle);
			SetWindowPos (Handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		BringToFront();
	}
	else
	{
        ShowWindow(Handle, SW_RESTORE);
		ShowWindow(Handle, SW_SHOWNOACTIVATE);
		SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		//SetForegroundWindow(Handle);
		//SetWindowPos(Handle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		Visible = true;
	}
}

void TfrmMain::ApplicationHide(void)
{
	if (Visible)
	{
		ToggleVisibility();
	}
}

void TfrmMain::ApplicationClose(void)
{
	actExit->Execute();
}

void TfrmMain::SetMainWindowLayout(int id)
{
	if (id == 0)
	{
		pnlCallControls->Parent = this;
		pcMain->Height = ClientHeight - StatusBar->Height - pnlCallControls->Height;
		pcMain->Top = pnlCallControls->Height;
	}
	else if (id == 1)
	{
		pnlCallControls->Parent = tsDialpad;
		pnlCallControls->Left = -3;
		pcMain->Height = ClientHeight - StatusBar->Height;
		pcMain->Top = 0;
	}
}

void TfrmMain::UpdateSize(void)
{
	int iWidth, iHeight;
	int callPanelLeft, callPanelTop;
	int mainPanelLeft, mainPanelTop;
	if (appSettings.frmMain.bSpeedDialVisible)
	{
		iWidth = appSettings.frmMain.expandedWidth;
		iHeight = appSettings.frmMain.expandedHeight;
		callPanelLeft = appSettings.frmMain.expandedCallPanelLeft;
		callPanelTop = appSettings.frmMain.expandedCallPanelTop;
		mainPanelLeft = appSettings.frmMain.expandedMainPanelLeft;
		mainPanelTop = appSettings.frmMain.expandedMainPanelTop;
	}
	else
	{
		iWidth = appSettings.frmMain.collapsedWidth;
		iHeight = appSettings.frmMain.collapsedHeight;
		callPanelLeft = appSettings.frmMain.collapsedCallPanelLeft;
		callPanelTop = appSettings.frmMain.collapsedCallPanelTop;
		mainPanelLeft = appSettings.frmMain.collapsedMainPanelLeft;
		mainPanelTop = appSettings.frmMain.collapsedMainPanelTop;
	}
	iHeight = floor(iHeight * initialScaling + 0.5);
	iWidth = floor(iWidth * initialScaling + 0.5);
	if (appSettings.frmMain.bUseClientAreaSizes)
	{
		this->ClientWidth = iWidth;
		this->ClientHeight = iHeight;
	}
	else
	{
		this->Width = iWidth;
		this->Height = iHeight;
	}

	pnlCallControls->Left = callPanelLeft;
	pnlCallControls->Top = callPanelTop;
	pnlMain->Left = mainPanelLeft;
	pnlMain->Top = mainPanelTop;
}

void TfrmMain::UpdateDialpad(void)
{
	static float scale;
	static bool once = false;
	if (!once)
	{
		once = true;
		scale = static_cast<float>(appSettings.gui.scalingPct) / 100.0f;
	}

	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_ED_TRANSFER];
		edTransfer->Visible = el.visible;
		edTransfer->Left = el.left * scale;
		edTransfer->Top = el.top * scale;
		edTransfer->Width = el.width * scale;
		edTransfer->Height = el.height * scale;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_TRBAR_MIC_VOLUME];
		trbarSoftvolMic->Visible = el.visible;
		trbarSoftvolMic->Left = el.left * scale;
		trbarSoftvolMic->Top = el.top * scale;
		trbarSoftvolMic->Width = el.width * scale;
		trbarSoftvolMic->Height = el.height * scale;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_BTN_RESET_MIC_VOLUME];
		btnResetMicVolume->Visible = el.visible;
		btnResetMicVolume->Left = el.left * scale;
		btnResetMicVolume->Top = el.top * scale;
		btnResetMicVolume->Width = el.width * scale;
		btnResetMicVolume->Height = el.height * scale;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_TRBAR_SPEAKER_VOLUME];
		trbarSoftvolSpeaker->Visible = el.visible;
		trbarSoftvolSpeaker->Left = el.left * scale;
		trbarSoftvolSpeaker->Top = el.top * scale;
		trbarSoftvolSpeaker->Width = el.width * scale;
		trbarSoftvolSpeaker->Height = el.height * scale;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_BTN_RESET_SPEAKER_VOLUME];
		btnResetSpeakerVolume->Visible = el.visible;
		btnResetSpeakerVolume->Left = el.left * scale;
		btnResetSpeakerVolume->Top = el.top * scale;
		btnResetSpeakerVolume->Width = el.width * scale;
		btnResetSpeakerVolume->Height = el.height * scale;
	}
}

void TfrmMain::FocusCbCallUri(void)
{
	if (appSettings.frmMain.bHideCallPanel || (appSettings.frmMain.layout != 0 && appSettings.frmMain.bHideMainPanel) || !Visible)
		return;
	if (appSettings.frmMain.layout != 0 && pcMain->ActivePage != tsDialpad)
		return;
	cbCallURI->SetFocus();
}

void __fastcall TfrmMain::tmrCallbackPollTimer(TObject *Sender)
{
	{
		// update call duration time
		static int pollCnt = 0;
		pollCnt++;
		if ((pollCnt % 20) == 0)
		{
			Call *call = Calls::GetCurrentCall();
			if (call && call->connected)
			{
				unsigned int delta = SecondsBetween(Now(), call->timeTalkStart);
				AnsiString caption;
				caption.sprintf("%s, %02d:%02d", call->getStateTranslatedDescription().c_str(), delta/60, delta%60);
				lblCallState->Caption = caption;
			}
		}
	}

	if (appSettings.Contacts.checkIfFileUpdated)
	{
		static int pollCnt = 0;
		pollCnt++;
		unsigned int period = appSettings.Contacts.checkIfFileUpdatedPeriod * 1000 / tmrCallbackPoll->Interval;
		if (period < 100)
			period = 100;
		if (pollCnt > period)
		{
			pollCnt = 0;
			FILETIME ft;
			GetFileWriteTime(GetContactsFileName(), &ft);
			if (memcmp(&ft, &contacts.getFiletime(), sizeof(ft)))
			{
				LOG("Contacts file time stamp changed - reloading...\n");
				contacts.Read();
			}
		}
	}

	{
		static int pollCnt = 0;
		if (++pollCnt > 5)
		{
			pollCnt = 0;
			PhoneInterface::Poll();
		}
	}

	PollCallbackQueue();

	{
		static int pollCnt = 0;

		if ((appState == Callback::APP_INIT_FAILED || appState == Callback::APP_START_FAILED) && (!frmSettings || !frmSettings->Visible))
		{
			pollCnt++;
			unsigned int timeToRestart = std::min(appStartFailCount * 15, 600u);
			if (pollCnt > (timeToRestart * 1000 / tmrCallbackPoll->Interval))
			{
				pollCnt = 0;
				SetStatus("Restarting UA (after init error)...");
				LOG("Restarting UA (after init error), app start fail count = %u\n", appStartFailCount);
				Ua::Instance().Restart();
			}
		}
		else
		{
			pollCnt = 0;
		}
	}
}
//---------------------------------------------------------------------------

void TfrmMain::PollCallbackQueue(void)
{
	Callback cb;
	bool answered = false;
	if (UA_CB->GetCallback(cb) != 0)
	{
		return;
	}
	switch (cb.type)
	{
		case Callback::CALL_STATE:
		{
			LOG("Call [uid = %d] state changed to %s (%d)\n", cb.callUid, Callback::GetCallStateName(cb.state), static_cast<int>(cb.state));
			switch(cb.state)
			{
			case Callback::CALL_STATE_INCOMING: {
				// call structure was allocated earlier by UaMain thread
				Call *call = Calls::FindByUid(cb.callUid);
				assert(call);

                if (Calls::Count() > 1)
				{
					const Settings::_Calls::AutoAnswerSecondCall &aa = appSettings.Calls.autoAnswerSecondCall;
					if (aa.enabled && aa.code >= 400)
					{
                        LOG("Auto-denying incoming call with code %d, reason %s (another call is active)\n", aa.code, aa.reason.c_str());
						call->disconnecting = true;
						call->lastScode = aa.code;
						call->lastReplyLine = aa.reason;
						UA->Hangup(cb.callUid, aa.code, aa.reason);
					}
				}

				int btnId;
				if (Calls::AssignLineButton(call, false, btnId) != 0)
				{
					LOG("Failed to assign LINE for incoming call %u, denying\n", cb.callUid);
					call->disconnecting = true;
					UA->Hangup(cb.callUid);
					return;
				}
				call->incoming = true;
				call->timestamp = Now();
				call->uri = cb.caller;
				call->peerName = GetPeerName(cb.callerName);
				call->recordFile = "";
				call->dtmfRxQueue.clear();
				call->paiPeerUri = cb.paiPeerUri;
				call->paiPeerName = GetPeerName(cb.paiPeerName);
				call->autoAnswerIntercom = false;
				call->initialRxInvite = cb.initialRxInvite;

				if (appSettings.uaConf.startAudioSourceAtCallStart)
				{
					UA->CallStartAudioExtraSource(call->uid);
				}

				if (Calls::Count() == 1 || appSettings.Calls.enableAutoAnswerEvenIfAnotherCallIsActive)
				{
					if (appSettings.uaConf.autoAnswerCallInfo && cb.callAnswerAfter >= 0)
					{
						LOG("Intercom/paging auto answer, answer-after = %d\n", cb.callAnswerAfter);
						call->autoAnswerCode = 200;
						call->autoAnswerIntercom = true;
						int time = cb.callAnswerAfter * 1000;
						if (time < appSettings.uaConf.autoAnswerCallInfoDelayMin)
						{
							time = appSettings.uaConf.autoAnswerCallInfoDelayMin;
						}
						if (time == 0)
						{
							AutoAnswer(*call);
							answered = true;
						}
						else
						{
							LOG("Delayed auto answer, time = %u ms\n", time);
							if (call->tmrAutoAnswer == NULL)
							{
								call->tmrAutoAnswer = new TTimer(NULL);
							}
							else
							{
								call->tmrAutoAnswer->Enabled = false;
							}
							call->tmrAutoAnswer->OnTimer = tmrAutoAnswerTimer;
							call->tmrAutoAnswer->Interval = time;
							call->tmrAutoAnswer->Enabled = true;
						}
					}
					else if (appSettings.uaConf.autoAnswer)
					{
						int time = appSettings.uaConf.autoAnswerDelayMin;
						int delta = appSettings.uaConf.autoAnswerDelayMax - appSettings.uaConf.autoAnswerDelayMin + 1;
						unsigned int rand32 = (((unsigned int)rand()&0xFFFF)<<16) + (rand()&0xFFFF);
						if (delta)
						{
							time += rand32 % delta;
						}
						call->autoAnswerCode = appSettings.uaConf.autoAnswerCode;
						if (time == 0)
						{
							AutoAnswer(*call);
							answered = true;
						}
						else
						{
							LOG("Delayed auto answer, time = %u ms\n", time);
							if (call->tmrAutoAnswer == NULL)
							{
								call->tmrAutoAnswer = new TTimer(NULL);
							}
							else
							{
								call->tmrAutoAnswer->Enabled = false;
							}
							call->tmrAutoAnswer->OnTimer = tmrAutoAnswerTimer;
							call->tmrAutoAnswer->Interval = time;
							call->tmrAutoAnswer->Enabled = true;
						}
					}
				}
				if (answered == false && muteRing == false)
				{
					StartRing(*call, RingFile(cb.alertInfo));
				}
				lastContactEntry = contacts.GetEntry(CleanUri(call->getPeerUri()));
				if (lastContactEntry)
				{
					if (appSettings.frmContactPopup.showOnIncoming)
					{
						if (lastContactEntry->note != "")
						{
							ShowContactPopup(lastContactEntry);
						}
					}
					if (appSettings.Contacts.openFileOnIncoming)
					{
						OpenContactFile(lastContactEntry);
					}
				}
				if (Calls::Count() == 1)
				{
					PhoneInterface::UpdateCallState(1, ExtractNumberFromUri(call->getPeerUri()).c_str()); //CleanUri(cb.caller).c_str());
				}
				if (appSettings.HttpQuery.openMode == Settings::_HttpQuery::openAutomaticOnIncoming)
				{
					HttpQuery(call);
				}

				call->accessUrl = cb.accessUrl;
				call->accessUrlMode = cb.accessUrlMode;
				if (appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeAlwaysActive ||
					(appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeFromMsg && call->accessUrlMode == 2)) {
					AccessCallUrl(call);
				}

				break;
			}
			case Callback::CALL_STATE_OUTGOING: {
				AnsiString secondParty, secondPartyDesc;
				secondParty = GetClip(cb.caller, appSettings.Display.bUserOnlyClip);
				lastContactEntry = contacts.GetEntry(CleanUri(cb.caller));
				Call *call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					AnsiString shortClip = GetClip(cb.caller, true);
					if (shortClip != "" && shortClip != call->uri)
					{
						// do not overwrite initial value set when making call
						call->uri = cb.caller;
					}
					call->recordFile = "";
				}
				if (lastContactEntry)
				{
					secondPartyDesc = lastContactEntry->description;
					if (appSettings.frmContactPopup.showOnOutgoing)
					{
						if (lastContactEntry->note != "")
						{
							ShowContactPopup(lastContactEntry);
						}
					}
					if (appSettings.Contacts.openFileOnOutgoing)
					{
						OpenContactFile(lastContactEntry);
					}
				}
				else
				{
					secondPartyDesc = GetPeerName(cb.callerName);
				}

				if (Visible == false && appSettings.trayNotifier.showOnOutgoing)
				{
					if (call)
						ShowTrayNotifier(call->uid, secondPartyDesc, secondParty, call->incoming);
				}
				PhoneInterface::UpdateCallState(1, ExtractNumberFromUri(cb.caller).c_str()); // CleanUri(cb.caller).c_str());

				break;
			}
			case Callback::CALL_STATE_TRYING:
				break;
			case Callback::CALL_STATE_RINGING:
				break;
			case Callback::CALL_STATE_PROGRESS: {
				Call *call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					call->progress = true;
					if (appSettings.uaConf.recording.enabled && call->recording == false && (appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallEarly))
					{
						StartRecording(*call);
					}
				}
				break;
			}
			case Callback::CALL_STATE_ESTABLISHED: {
				Call *call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					call->connected = true;
					call->timeTalkStart = Now();
					call->paiPeerUri = cb.paiPeerUri;
					call->paiPeerName = GetPeerName(cb.paiPeerName);
					call->codecName = cb.codecName;
					call->lastScode = 200;
					call->lastReplyLine = "200 OK";

					PhoneInterface::UpdateRing(0);
					if (call->tmrAutoAnswer)
					{
						call->tmrAutoAnswer->Enabled = false;
					}
					if (appSettings.uaConf.recording.enabled && call->recording == false &&
						((appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallEarly) || (appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallConfirmed))
						)
					{
						StartRecording(*call);
					}
					if (call->incoming == true && appSettings.HttpQuery.openMode == Settings::_HttpQuery::openAutomaticOnIncomingAnswer)
					{
						HttpQuery(call);
					}
					if (appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeAlwaysActiveOnConfirmed ||
						(appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeFromMsgOnConfirmed && call->accessUrlMode == 2)) {
						AccessCallUrl(call);
					}				
				}
				break;
			}
			case Callback::CALL_STATE_TRANSFER: {
				Call *call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					call->uri = cb.caller;
					call->peerName = GetPeerName(cb.callerName);
				}
				break;
			}
			case Callback::CALL_STATE_TRANSFER_OOD:
				if (appSettings.uaConf.handleOodRefer)
				{
					Call *call = Calls::GetCurrentCall();
					if (call)
					{
						if (call->uri == "" && call->incoming == false && !pagingTx.active)
						{
							int TODO__EXTRACT_NUMBER_FROM_URI_IF_SERVER_IS_MATCHING;	// otherwise full URI goes into history
							AnsiString target = cb.caller;
							if (target != "")
							{
								unsigned int callUid;
								MakeCall(target, callUid);
							}
						}
					}
				}
				else
				{
					LOG("Incoming REFER: ignoring out-of-dialog transfer, see configuration\n");
				}
				break;
			case Callback::CALL_STATE_CLOSED: {
				Call * call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					if (cb.scode != 0)
					{
						call->lastScode = cb.scode;
						call->lastReplyLine = cb.caller;
					}
					Calls::SetPreviousCall(*call);

					if (cb.caller != "")
					{
						call->sipReason = cb.caller;
					}
					bool completedElsewhere = (call->sipReason.UpperCase() == CALL_COMPLETED_ELSEWHERE);

					History::Entry entry;
					DecodeDateTime(call->timestamp,
						entry.timestamp.year, entry.timestamp.month, entry.timestamp.day,
						entry.timestamp.hour, entry.timestamp.min, entry.timestamp.sec,
						entry.timestamp.msec);
					entry.uri = call->uri.c_str();
					entry.peerName = call->peerName.c_str();
					entry.paiUri = call->paiPeerUri.c_str();
					entry.paiPeerName = call->paiPeerName.c_str();
					entry.incoming = call->incoming;
					entry.codecName = call->codecName;
					if (call->connected)
					{
						entry.time = SecondsBetween(Now(), call->timeTalkStart) + 1;
					}
					else
					{
						entry.time = 0;
						if (entry.incoming && !call->disconnecting && !completedElsewhere)
						{
							SetNotificationIcon(true);
						}
					}
					if (entry.incoming)
					{
						entry.uri = ExtractNumberFromUri(entry.uri.c_str()).c_str();
					}
					// trying to extract number from PAI URI for any call
					entry.paiUri = ExtractNumberFromUri(entry.paiUri.c_str()).c_str();
					entry.lastScode = call->lastScode;
					entry.lastReplyLine = call->lastReplyLine;
					entry.recordFile = call->recordFile;
					entry.reason = call->sipReason;

					if (!(appSettings.history.ignoreCallsCompletedElsewhere && completedElsewhere))
					{
						history.AddEntry(entry);
						UpdateCallHistory();
					}

					buttons.UpdateBtnState(Button::HOLD, false);
					buttons.UpdateBtnState(Button::MUTE, false);
					if (call->tmrAutoAnswer)
					{
						call->tmrAutoAnswer->Enabled = false;
					}
					frmTrayNotifier->HideWindow();
				#if 0
					lbl2ndParty->Caption = "";
					lbl2ndPartyDesc->Caption = "";
				#endif
					if (call == Calls::GetCurrentCall())
					{
						PhoneInterface::UpdateCallState(0, "");
					}
					PhoneInterface::UpdateRing(0);
				}
				break;
			}
			default:
				LOG("Unhandled call state = %d\n", static_cast<int>(cb.state));
				break;
			}

			Call *call = Calls::FindByUid(cb.callUid);
			if (call)
			{
				call->SetState(cb.state);
				if (call == Calls::GetCurrentCall())
				{
					UpdateMainCallDisplay();
					if (cb.state == Callback::CALL_STATE_CLOSED)
					{
						tmrClearCallState->Enabled = true;
					}
					else
					{
						tmrClearCallState->Enabled = false;
					}
				}
			}

			if (cb.state == Callback::CALL_STATE_INCOMING)
			{
				if (appSettings.trayNotifier.showOnIncoming)
				{
					if (
						(Visible == false || appSettings.trayNotifier.skipIfMainWindowVisible == false) &&
						((answered && appSettings.trayNotifier.hideWhenAnsweringCallAutomatically) == false)
						)
					{
						if (call)
						{
							AnsiString party = GetClip(call->getPeerUri(), appSettings.Display.bUserOnlyClip);
							AnsiString desc;
							Contacts::Entry *entry = contacts.GetEntry(CleanUri(call->getPeerUri()));
							if (entry)
							{
								desc = lastContactEntry->description;
							}
							else
							{
								desc = call->getPeerName();
							}
							ShowTrayNotifier(call->uid, desc, party, call->incoming);
						}
					}
				}
				if (appSettings.frmMain.bRestoreOnIncomingCall)
				{
					if (IsIconic(Application->Handle))
					{
						//Application->Restore();	// this takes focus
						ShowWindow(Application->Handle, SW_SHOWNOACTIVATE);
					}
					//SetForegroundWindow( Application->Handle );
					//this->BringToFront();
					// https://stackoverflow.com/questions/19136365/win32-setforegroundwindow-not-working-all-the-time
					SetWindowPos(this->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					if (appSettings.frmMain.bAlwaysOnTop == false)
					{
						SetWindowPos(this->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					}
				}
			}

			if (frmTrayNotifier)
			{
            	frmTrayNotifier->SetCallState(cb.state);
			}

			if (appSettings.Scripts.onCallState != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onCallState.c_str());
				RunScriptFile(SCRIPT_SRC_ON_CALL_STATE, cb.callUid, asScriptFile.c_str(), handled);
			}

			if (cb.state == Callback::CALL_STATE_CLOSED)
			{
				if (call)
					ClearLineButton(call->btnId);
				Calls::RemoveByUid(cb.callUid);
			}

			break;
		}
		case Callback::CALL_REINVITE_RECEIVED:
		{
			Call *call = Calls::FindByUid(cb.callUid);
			if (call)
			{
				call->uri = cb.caller;
				call->peerName = GetPeerName(cb.callerName);
				call->paiPeerUri = cb.paiPeerUri;
				call->paiPeerName = GetPeerName(cb.paiPeerName);
				call->ShowOnLineButton();
				if (call == Calls::GetCurrentCall())
				{
					UpdateMainCallDisplay();

					int TODO__TRAY_NOTIFIER_MULTIPLE_CALLS;
					frmTrayNotifier->SetData(call->uid, lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, false);
				}
			}
			break;
		}
		case Callback::CALL_DTMF_STATE:
		{
			if (cb.dtmfActive == true)
			{
				Call *call = Calls::FindByUid(cb.callUid);
				if (call)
				{
					call->dtmfRxQueue.push_back(cb.dtmf[1]);
				}
			}
			break;
		}
		case Callback::REG_STATE:
		{
			AnsiString asRegText;
			switch(cb.reg_state)
			{
			case Callback::REG_STATE_REGISTERING:
				asRegText = "Registering";
				break;
			case Callback::REG_STATE_REGISTER_OK:
				asRegText = "Registered";
				SetErrorIcon(false);
				break;
			case Callback::REG_STATE_REGISTER_FAIL:
				asRegText = "Error registering";
				SetErrorIcon(true);
				break;
			case Callback::REG_STATE_UNREGISTERING:
				asRegText = "Unregistering...";
				break;
			case Callback::REG_STATE_UNREGISTER_OK:
				asRegText = "Unregistered";
				break;
			case Callback::REG_STATE_UNREGISTER_FAIL:
				asRegText = "Unregistering error";
				break;
			default:
				assert(!"Unhandled reg state");
				break;
			}
			registration.state = cb.reg_state;

			if (cb.reg_state != Callback::REG_STATE_UNREGISTERING &&
				cb.reg_state != Callback::REG_STATE_UNREGISTER_OK &&
				cb.reg_state != Callback::REG_STATE_UNREGISTER_FAIL)
				// skipping acc info for these states (current acc info may be different)
			{
				if (appSettings.uaConf.accounts.size())
				{
					asRegText.cat_sprintf(" %s@%s",
						appSettings.uaConf.accounts[0].user.c_str(),
						appSettings.uaConf.accounts[0].reg_server.c_str());
				}
				if (cb.reg_state != Callback::REG_STATE_REGISTER_OK && cb.reg_state != Callback::REG_STATE_UNREGISTER_OK)
				{
					if (cb.prm.Length() > 0)
					{
						// failure reason for registration error
						asRegText.cat_sprintf(" (%s)", cb.prm.c_str());
					}
				}
			}

			if (cb.reg_state == Callback::REG_STATE_REGISTER_OK) {
				HandleCommandLine();
				PhoneInterface::UpdateRegistrationState(1);
			}
			else
			{
				PhoneInterface::UpdateRegistrationState(0);
			}

			SetStatus(asRegText);

			if (appSettings.Scripts.onRegistrationState != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onRegistrationState.c_str());
				RunScriptFile(SCRIPT_SRC_ON_REGISTRATION_STATE, -1, asScriptFile.c_str(), handled);
			}

			break;
		}
		case Callback::RECORDER_STATE:
		{
			Call *call = Calls::FindByUid(cb.callUid);
			if (call)
			{
				Recorder &recorder = call->recorder;
				recorder.state = cb.rec_state;

				if (appSettings.Scripts.onRecorderState != "")
				{
					AnsiString asScriptFile;
					bool handled = true;
					asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onRecorderState.c_str());
					RunScriptFile(SCRIPT_SRC_ON_RECORDER_STATE, call->uid, asScriptFile.c_str(), handled);
				}
			}
			break;
		}
		case Callback::ENCRYPTION_STATE:
		{
			Call *call = Calls::FindByUid(cb.callUid);
			if (call)
			{
				call->zrtp.sessionId = cb.zrtp.sessionId;
				call->zrtp.active = cb.zrtp.active;
				call->zrtp.sas = cb.zrtp.sas;
				call->zrtp.cipher = cb.zrtp.cipher;
				call->zrtp.verified = cb.zrtp.verified;
				if (appSettings.Scripts.onEncryptionState != "")
				{
					AnsiString asScriptFile;
					bool handled = true;
					asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onEncryptionState.c_str());
					int TODO__ZRTP_SESSION_ID_VS_CALL_ID;
					RunScriptFile(SCRIPT_SRC_ON_ENCRYPTION_STATE, cb.zrtp.sessionId, asScriptFile.c_str(), handled);
				}
			}
			break;
		}
		case Callback::APP_STATE:
		{
			AnsiString text;
			this->appState = cb.app_state;
			/** \note Settings menu is disabled while initializing app */
			switch(cb.app_state)
			{
			case Callback::APP_INIT_FAILED:
				SetStatus("Failed to init application");
				SetErrorIcon(true);
				HandleCommandLine();
				appStartFailCount++;
				break;
			case Callback::APP_START_FAILED:
				SetStatus("Failed to start application");
				SetErrorIcon(true);
				HandleCommandLine();
				appStartFailCount++;
				break;
			case Callback::APP_STARTED:
				UaCustomRequests::Clear();
				SetErrorIcon(false);
				text = "Initialized";
				appStartFailCount = 0;
				if (appSettings.uaConf.accounts.size())
				{
					if (appSettings.uaConf.accounts[0].user.length())
					{
						text.cat_sprintf(": %s@%s",
							appSettings.uaConf.accounts[0].user.c_str(),
							appSettings.uaConf.accounts[0].reg_server.c_str());
					}
					else if (appSettings.uaConf.accounts[0].reg_server.length())
					{
						text.cat_sprintf(": %s",
							appSettings.uaConf.accounts[0].user.c_str(),
							appSettings.uaConf.accounts[0].reg_server.c_str());
					}
					if (appSettings.uaConf.accounts[0].reg_expires == 0)
					{
						SetStatus(text);
						HandleCommandLine();
					}
				}
				else
				{
					SetStatus(text);
					HandleCommandLine();					
				}
				break;
			default:
				assert(!"Unhandled app state");
				break;
			}
			break;
		}
		case Callback::DLG_INFO_STATE:
		{
			enum dialog_info_direction direction = DIALOG_INFO_DIR_UNKNOWN;
			AnsiString remoteIdentity;
			AnsiString remoteIdentityDisplay;
			bool updateRemoteIdentity = true;
			enum dialog_info_status dlgInfoState = DIALOG_INFO_UNKNOWN;

			UaConf::Contact &contact = appSettings.uaConf.contacts[cb.contactId];

			struct dialog_data *ddata = NULL;
			for (unsigned int i=0; i<cb.ddata_cnt; i++)
			{
				struct dialog_data *d = &cb.ddata[i];
				if (appSettings.frmMain.dialogInfoPreferredState == Settings::_frmMain::DIALOG_INFO_PREFERRED_FIRST)
				{
					ddata = d;
					break;
				}
				else if (appSettings.frmMain.dialogInfoPreferredState == Settings::_frmMain::DIALOG_INFO_PREFERRED_EARLY)
				{
					if (d->status == DIALOG_INFO_EARLY)
					{
						ddata = d;
						break;
					}
				}
				else if (appSettings.frmMain.dialogInfoPreferredState == Settings::_frmMain::DIALOG_INFO_PREFERRED_CONFIRMED)
				{
					if (d->status == DIALOG_INFO_CONFIRMED)
					{
						ddata = d;
						break;
					}
				}
			}
			if (ddata == NULL)
			{
				for (unsigned int i=0; i<cb.ddata_cnt; i++)
				{
					struct dialog_data *d = &cb.ddata[i];
					if (d->status == DIALOG_INFO_EARLY || d->status == DIALOG_INFO_CONFIRMED)
					{
						ddata = d;
						break;
					}
				}
			}
			if (ddata == NULL)
			{
				if (cb.ddata_cnt > 0)
					ddata = &cb.ddata[0];
			}
			if (ddata)
			{
				dlgInfoState = ddata->status;
				if (!appSettings.frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity)
				{

					if (dlgInfoState == DIALOG_INFO_TERMINATED && appSettings.frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated)
					{
						// clearing/ignoring remote identity if call is terminated
					}
					else
					{
						direction = ddata->direction;
						remoteIdentity = ddata->identity;
						remoteIdentityDisplay = GetPeerName(ddata->identity_display);
						if (!remoteIdentity.Length() && !remoteIdentity.Length() && appSettings.frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing)
						{
							updateRemoteIdentity = false;
						}
					}
				}
				if (updateRemoteIdentity)
				{
					contact.direction = direction;
					contact.remoteIdentity = remoteIdentity.c_str();
					contact.remoteIdentityDisplay = remoteIdentityDisplay.c_str();
				}
			}
			contact.dialog_info_state = dlgInfoState;
			std::list<int> &ids = appSettings.uaConf.contacts[cb.contactId].btnIds;
			std::list<int>::iterator iter;
			for (iter = ids.begin(); iter != ids.end(); ++iter)
			{
				buttons.UpdateDlgInfoState(*iter, dlgInfoState, updateRemoteIdentity, direction, remoteIdentity, remoteIdentityDisplay);
			}

			if (appSettings.Scripts.onDialogInfo != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onDialogInfo.c_str());
				RunScriptFile(SCRIPT_SRC_ON_DIALOG_INFO, cb.contactId, asScriptFile.c_str(), handled);
			}
			break;
		}
		case Callback::PRESENCE_STATE:
		{
			AnsiString note = cb.presenceNote;
			if (appSettings.frmMain.bSpeedDialIgnorePresenceNote)
			{
				note = "";
			}
			std::list<int> &ids = appSettings.uaConf.contacts[cb.contactId].btnIds;
			std::list<int>::iterator iter;
			for (iter = ids.begin(); iter != ids.end(); ++iter)
			{
				buttons.UpdatePresenceState(*iter, cb.presenceState, note);
			}
			break;
		}
		case Callback::MWI_STATE:
		{
			SetNotificationIcon(cb.mwiNewMsg > 0);
			buttons.UpdateMwiState(cb.mwiNewMsg, cb.mwiOldMsg);
			break;
		}
		case Callback::PAGING_TX_STATE:
		{
			AnsiString asStateText;
			pagingTx.state = cb.paging_tx_state;
			switch (cb.paging_tx_state)
			{
			case Callback::PAGING_TX_STARTED:
				asStateText = "Streaming...";
				pagingTx.active = true;
				PhoneInterface::UpdatePagingTxState(1);
				break;
			case Callback::PAGING_TX_ENDED:
				asStateText = "";
				pagingTx.active = false;
				PhoneInterface::UpdatePagingTxState(0);
				break;
			default:
				assert(!"Unhandled paging TX state");
			}
			lblCallState->Caption = asStateText;

			if (appSettings.Scripts.onStreamingState != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onStreamingState.c_str());
				RunScriptFile(SCRIPT_SRC_ON_STREAMING_STATE, -1, asScriptFile.c_str(), handled);
			}

			break;
		}
		case Callback::EVENT_TALK:
		{
			if (Calls::Count() == 1 || appSettings.Calls.enableAutoAnswerEvenIfAnotherCallIsActive)
			{
				Call *call = Calls::GetCurrentCall();
				if (call && call->incoming)
				{
					if (appSettings.uaConf.answerOnEventTalk)
					{
						Answer(call->uid);
					}
					else
					{
						LOG("Ignoring \"Event: talk\" (enable in configuration)\n");
					}
				}
			}
			break;
		}
		case Callback::AUDIO_ERROR:
		{
			Call *call = Calls::FindByUid(cb.callUid);
			if (call)
			{
				bool handled = false;
				call->audioErrorCount++;
				if (appSettings.Scripts.onAudioDeviceError != "")
				{
					AnsiString asScriptFile;
					asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onAudioDeviceError.c_str());
					RunScriptFile(SCRIPT_SRC_ON_AUDIO_ERROR, cb.callUid, asScriptFile.c_str(), handled);
				}
				PhoneInterface::UpdateAudioError();
				if (appSettings.Calls.bDisconnectCallOnAudioError && !handled)
				{
					LOG("Disconnecting call %u on audio error or end of file\n", call->uid);
					Hangup(call->uid);
				}
			}
			break;
		}
		case Callback::CUSTOM_REQUEST_STATUS:
		{
			if (UaCustomRequests::NotifyReply(cb.requestUid, cb.requestError, cb.scode, cb.requestReplyText) == 0)
			{
				if (appSettings.Scripts.onCustomRequestReply != "")
				{
					AnsiString asScriptFile;
					bool handled = true;
					asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onCustomRequestReply.c_str());
					RunScriptFile(SCRIPT_SRC_ON_CUSTOM_REQUEST_REPLY, cb.requestUid, asScriptFile.c_str(), handled);
				}
			}
			break;
		}
		case Callback::SIMPLE_MESSAGE:
		{
			SIMPLE_Messages::OnIncomingMessage(cb.caller, cb.contentType, cb.body);
			AnsiString file = appSettings.Messages.ring;
			if (file != "")
			{
				AnsiString fileFull;
				fileFull.sprintf("%s\\%s", Paths::GetProfileDir().c_str(), file.c_str());
				if (FileExists(fileFull))
				{
					UA->StartRing2(file);
				}
				else
				{
					LOG("Ring file (%s) for MESSAGE not found\n", file.c_str());
				}
			}
			break;
		}
		case Callback::SIMPLE_MESSAGE_STATUS:
		{
			SIMPLE_Messages::OnMessageStatus(cb.requestUid, cb.requestError, cb.scode, cb.reason);
			break;
		}
		default:
		{
			assert(!"Unhandled callback type");
		}
	}
}

void TfrmMain::Dial(char digit)
{
	Call *call = Calls::GetCurrentCall();

	if (call && (call->connected || call->progress))
	{
		UA->SendDigit(call->uid, digit);
	}
	else
	{
		if (digit == 'R')
		{
			return;
		}
		cbCallURI->Text = cbCallURI->Text + digit;
		// move cursor to the end
		cbCallURI->SelStart = cbCallURI->Text.Length();
		PhoneInterface::UpdateCallState(1, cbCallURI->Text.c_str());
	}
}

void TfrmMain::DialString(const std::string& digits, bool runScript)
{
	for (int i=0; i<digits.length(); i++)
	{
		const char* allowed = "0123456789ABCD*#R";
		char digit = toupper(digits[i]);
		if (strchr(allowed, digit))
		{
			Dial(digit);
			if (runScript && appSettings.Scripts.onDial != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onDial.c_str());
                int TODO__ON_DIAL_PASS_CALL_UID_TO_SCRIPT__OR_GET_CURRENT_CALL;
				RunScriptFile(SCRIPT_SRC_ON_DIAL, digit, asScriptFile.c_str(), handled);
			}
		}
	}
}

void TfrmMain::StartRecording(Call &call)
{
	if (appSettings.uaConf.recording.enabled == false)
	{
        LOG("Call recording is not enabled in configuration!\n");
		return;
	}

	try
	{
		AnsiString file;
		AnsiString dir;
		if (appSettings.uaConf.recording.recDir == UaConf::RecordingCfg::RecDirRelative)
		{
			dir = Paths::GetProfileDir() + "\\recordings\\";
		}
		else
		{
			dir = appSettings.uaConf.recording.customRecDir.c_str();
			if (dir[dir.Length()] != '\\')
				dir += "\\";
		}
		if (ForceDirectories(dir))
		{
			file += dir;
			file += FormatDateTime("yyyymmdd_hhnnss", Now());
			file += call.incoming?"_1_":"_0_";
			// let's limit URI/number length to some reasonable value
			AnsiString uri;
			if (call.incoming)
			{
				uri = ExtractNumberFromUri(call.getPeerUri().c_str()).c_str();
			}
			else
			{
				uri = call.uri;
			}
			uri = uri.SubString(1, 32);
			if (appSettings.uaConf.recording.noNumberB64Encoding == false)
			{
				std::string b64uri = base64_encode((unsigned char*)uri.c_str(), uri.Length(), BASE64_ALPHABET_FSAFE);
				file += b64uri.c_str();
			}
			else
			{
				/// \note There are more invalid characters (<>:"/\|?*) - file creation would fail for them
				AnsiString escaped = StringReplace(uri, "*", "A", TReplaceFlags() << rfReplaceAll);
				file += escaped;
			}
			if (appSettings.uaConf.recording.fileFormat == 0)
			{
				file += ".wav";
			}
			else
			{
				file += ".ogg";
			}
			if (call.recording == false)
			{
				LOG("Record file: %s\n", file.c_str());
			}
			UA->Record(call.uid, file, appSettings.uaConf.recording.channels, appSettings.uaConf.recording.side,
				appSettings.uaConf.recording.fileFormat, appSettings.uaConf.recording.bitrate);
			call.recordFile = file;
			call.recording = true;
		}
		else
		{
			LOG("Failed to create directory for call recording!\n");
		}
	}
	catch(...)
	{}
}

void __fastcall TfrmMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (appSettings.frmMain.bXBtnMinimize == false)
	{
        Finalize();
		Ua::Instance().Quit();
		Application->Terminate();			
		return;
	}
	if(!Application->Terminated)
	{
		if (trIcon)
		{
			ShowWindow(Application->Handle, SW_HIDE);
			Visible = false;
		}
		Action = caNone;
    }
	else
	{
		PostQuitMessage(0);
	}
}
//---------------------------------------------------------------------------


void TfrmMain::Redial(void)
{
	const std::deque<History::Entry>& entries = history.GetEntries();
	for (unsigned int i=0; i<entries.size(); i++)
	{
		const History::Entry &entry = entries[i];
		if (entry.incoming == false)
		{
			unsigned int callId;
			MakeCall(entry.uri.c_str(), callId);
			break;
		}
	}
}

void TfrmMain::HttpQuery(const Call *call)
{
	AnsiString target;
	if (call && call->uri != "" && call->incoming)
	{
		target = ExtractNumberFromUri(call->uri);
		if (target == "")
		{
			target = CleanUri(call->uri);
		}
	}
	else
	{
		const std::deque<History::Entry>& entries = history.GetEntries();
		for (unsigned int i=0; i<entries.size(); i++)
		{
			const History::Entry &entry = entries[i];
			if (entry.incoming == true)
			{
				target = entry.uri.c_str();
				break;
			}
		}
	}
	if (target != "")
	{
    	OnHttpQuery(target);
	}
	else
	{
		LOG("HTTP query: no incoming call with number found\n");
	}
}

void TfrmMain::AccessCallUrl(const Call *call)
{
	if (call == NULL || call->accessUrl == "")
		return;
	ShellExecute(NULL, "open", call->accessUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void TfrmMain::OnHttpQuery(AnsiString target)
{
	char buf[1024];
	target = urlencode(buf, sizeof(buf), target.c_str());
	AnsiString url = StringReplace(appSettings.HttpQuery.url, "[number]", target, TReplaceFlags() << rfReplaceAll);
	ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}


void TfrmMain::ExecuteApp(AnsiString cmd, AnsiString params)
{
	Call *call = Calls::GetCurrentCall();

	AnsiString target;
	if (call && call->uri != "")
	{
		target = call->uri;
	}
	else
	{
		const std::deque<History::Entry>& entries = history.GetEntries();
		for (unsigned int i=0; i<entries.size(); i++)
		{
			const History::Entry &entry = entries[i];
			target = entry.uri.c_str();
			break;
		}
	}
	if (params.Pos("[number]"))
	{
		if (target != "")
		{
			params = StringReplace(params, "[number]", target, TReplaceFlags() << rfReplaceAll);
		}
		else
		{
			return;
		}
	}
	ShellExecute(NULL, "open", cmd.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
}

void TfrmMain::OnPhonebookEdit(AnsiString uri)
{
	bool adding = false;
	Contacts::Entry newEntry;
	Contacts::Entry *entry = contacts.GetEntry(uri);
	if (entry == NULL)
	{
		adding = true;
		entry = &newEntry;
		entry->uri1 = uri;
	}
	frmContactEditor->Start(entry, appSettings.Contacts.storeNoteInSeparateFile);
	if (frmContactEditor->isConfirmed())
	{
		if (adding)
		{
			contacts.GetEntries().push_back(*entry);
		}
		contacts.Sort();
		contacts.Update();
		contacts.Write();
	}
}

AnsiString TfrmMain::OnGetContactName(AnsiString uri)
{
	Contacts::Entry *entry = contacts.GetEntry(CleanUri(uri));
	if (entry)
	{
        return entry->description;
	}
	return "";
}

void TfrmMain::OnProgrammableBtnClick(int id, TProgrammableButton* btn)
{
	assert(id >= 0 && id < buttons.btnConf.size());
	ButtonConf &cfg = buttons.btnConf[id];
	if (cfg.inactive)
	{
		return;
	}

	if (appSettings.Scripts.onProgrammableButton != "")
	{
		AnsiString asScriptFile;
		bool handled = false;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onProgrammableButton.c_str());
		RunScriptFile(SCRIPT_SRC_BUTTON, id, asScriptFile.c_str(), handled);
		if (handled)
		{
			return;
		}
	}

	bool down = btn->GetDown();
	switch (cfg.type)
	{
	case Button::DISABLED:
		LOG("Button with id = %d is disabled\n", id);	
		break;
	case Button::MWI: {
		unsigned int callUid;
		MakeCall(cfg.number.c_str(), callUid);
		break;
	}
	case Button::BLF:
	case Button::PRESENCE:
	case Button::SPEED_DIAL: {
		std::string dial = cfg.number;
		if (cfg.type == Button::BLF) {
			// possible override of default number depending on state
			// (allows definining pickup number like with Yealink phones and more)
			enum dialog_info_status status = btn->GetState();
			switch (status) {
			case DIALOG_INFO_UNKNOWN:
				if (cfg.blfOverrideIdle.active)
					dial = cfg.blfOverrideIdle.number;
				break;
			case DIALOG_INFO_TERMINATED:
				if (cfg.blfOverrideTerminated.active)
					dial = cfg.blfOverrideTerminated.number;
				break;
			case DIALOG_INFO_EARLY:
				if (cfg.blfOverrideEarly.active)
					dial = cfg.blfOverrideEarly.number;
				break;
			case DIALOG_INFO_CONFIRMED:
				if (cfg.blfOverrideConfirmed.active)
					dial = cfg.blfOverrideConfirmed.number;
				break;
			}
		}
		Call *call = Calls::GetCurrentCall();
		if (call && call->connected)
		{
            int TODO__ATTENDED_TRANSFER_WITH_BLF;	// nothing to do?
			switch (cfg.blfActionDuringCall)
			{
			case ButtonConf::BLF_IN_CALL_NONE:
				break;
			case ButtonConf::BLF_IN_CALL_DTMF:
				// add prefix (optional in configuration)
				dial = cfg.blfDtmfPrefixDuringCall + dial;
				DialString(dial, false);
				break;
			case ButtonConf::BLF_IN_CALL_TRANSFER:
				UA->Transfer(call->uid, dial.c_str()); 			
				break;
			default:
				assert(0);
				break;
			}
		}
		else
		{
			unsigned int callUid;
			MakeCall(dial.c_str(), callUid);
		}
		break;
	}
	case Button::REDIAL:
		Redial();
		break;
	case Button::DTMF: {
		DialString(cfg.number, true);
		break;
	}
	case Button::TRANSFER: {
		AnsiString target = cfg.number.c_str();
		if (target == "")
		{
			if (edTransfer->Text == asTransferHint || edTransfer->Text == "")
				return;
			target = edTransfer->Text;
		}
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			UA->Transfer(call->uid, target);
		}
		break;
	}
	case Button::ATTENDED_TRANSFER: {
		Call *call = Calls::GetCurrentCall();
		int TODO__IS_CHECKING_STATE_REALLY_NEEDED;
		if (call)
		{
			if (call->GetState() == Callback::CALL_STATE_ESTABLISHED)
			{
				int TODO__ATT_TRANSFER_FOR_MORE_THAN_2_CALLS;
				if (Calls::Count() == 2)
				{
					std::vector<unsigned int> ids = Calls::GetUids();
					for (unsigned int i=0; i<ids.size(); i++)
					{
						if (ids[i] == call->uid)
							continue;
						Call *secondCall = Calls::FindByUid(ids[i]);
						if (secondCall && secondCall->GetState() == Callback::CALL_STATE_ESTABLISHED)
						{
							UA->TransferReplace(call->uid, secondCall->uid);
							break;
						}
						else
						{
							LOG("Attended transfer: no second call or second call not established!\n");
						}
					}
				}
				else
				{
                	LOG("Attended transfer requires 2 simultaneous calls\n");
				}
			}
			else
			{
				LOG("Attended transfer: current call is not established!\n");
			}
		}
		else
		{
			LOG("Attended transfer: no current call!\n");
		}
		break;
	}
	case Button::HOLD: {
		Call *call = Calls::GetCurrentCall();
		if (call) {
			call->setHold(down);
		}
		break;
	}
	case Button::CONFERENCE_START: {
		// unhold all calls first
		Calls::SetHold(false);
		UA->ConferenceStart();
		break;
	}
	case Button::REREGISTER:
		UA->ReRegister(0);
		break;
	case Button::UNREGISTER:
		UA->UnRegister(0);
		break;
	case Button::MUTE: {
		Call *call = Calls::GetCurrentCall();
		if (call) {
			call->setMute(down);
		}
		break;
	}
	case Button::MUTE_RING:
		buttons.UpdateBtnState(cfg.type, down);
		muteRing = down;
		if (muteRing)
		{
			bool mutePlugins = false;
			const std::map<unsigned int, Call>& calls = Calls::GetCalls();
			for(std::map<unsigned int, Call>::const_iterator iter = calls.begin(); iter != calls.end(); ++iter)
			{
				if (iter->second.incoming)
				{
					UA->PlayStop(iter->second.uid);
					mutePlugins = true;
				}
			}
			if (mutePlugins)
			{
				int TODO__PHONE_INTERFACE_CALL_ID;
				PhoneInterface::UpdateRing(0);
			}
		}
		break;
	case Button::CONTACT_NOTE:
		if (lastContactEntry && !(appSettings.Contacts.storeNoteInSeparateFile && lastContactEntry->file == ""))
		{
			ShowContactPopup(lastContactEntry);
		}
		else
		{
			Call *call = Calls::GetCurrentCall();
			if (call)
			{
				AnsiString num = call->getPeerUri();
				if (num != "")
				{
					OnPhonebookEdit(num);
				}
			}
		}
		break;
	case Button::CONTACT_FILE:
		if (lastContactEntry)
		{
			OpenContactFile(lastContactEntry);
		}
		break;
	case Button::HTTP_QUERY: {
		Call *call = Calls::GetCurrentCall();
		/** \note HttpQuery accepts also NULL, taking number from last history entry in this case */
		HttpQuery(call);
		break;
	}
	case Button::EXECUTE:
		ExecuteApp(cfg.number.c_str(), cfg.arg1.c_str());
		break;
	case Button::PAGING_TX:
		if (pagingTx.active)
		{
			LOG("Could not start streaming - streaming already running\n");
			break;
		}
		UA->PagingTx(cfg.number.c_str(), cfg.pagingTxWaveFile.c_str(), cfg.pagingTxCodec.c_str(), cfg.pagingTxPtime);
		break;
	case Button::SCRIPT: {
		if (cfg.script == "")
		{
			AnsiString msg;
			msg.sprintf("Script file is not configured for button #%d.", id);
			MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
			break;
		}
		AnsiString asScriptFile;
		bool handled = true;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), cfg.script.c_str());
		RunScriptFile(SCRIPT_SRC_BUTTON, id, asScriptFile.c_str(), handled);
		break;
	}
	case Button::SIP_ACCESS_URL: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			AccessCallUrl(call);
		}
		break;
	}
	case Button::SWITCH_AUDIO_SOURCE: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			UA->SwitchAudioSource(call->uid, cfg.audioRxMod.c_str(), cfg.audioRxDev.c_str());
		}
		else if (pagingTx.active)
		{
			UA->SwitchAudioSource(Call::INVALID_UID, cfg.audioRxMod.c_str(), cfg.audioRxDev.c_str());
		}
		break;
	}
	case Button::SWITCH_AUDIO_PLAYER: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			UA->SwitchAudioPlayer(call->uid, cfg.audioTxMod.c_str(), cfg.audioTxDev.c_str());
		}
		break;
	}
	case Button::SWITCH_VIDEO_SOURCE: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			UA->SwitchVideoSource(call->uid, cfg.videoRxMod.c_str(), cfg.videoRxDev.c_str());
		}
		break;
	}
	case Button::HANGUP: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			Hangup(call->uid, cfg.sipCode, cfg.sipReason.c_str());
		}
		break;
	}
	case Button::HANGUP_ALL: {
		std::vector<unsigned int> uids = Calls::GetUids();
		for (unsigned int i=0; i<uids.size(); i++)
		{
			Hangup(uids[i], cfg.sipCode, cfg.sipReason.c_str());
		}
		break;
	}
	case Button::CALL_MAKE_OR_ANSWER: {
		btnMakeCallClick(NULL);
		break;
	}
	case Button::CALL_ANSWER: {
		if (pagingTx.active)
			break;
		Call* call = Calls::GetCurrentCall();
		if (call && call->incoming)
		{
			Answer(call->uid);
		}
		break;
	}
	case Button::SEND_TEXT_MESSAGE: {
		AnsiString target = cfg.number.c_str();
		if (target == "")
		{
			target = cbCallURI->Text;
		}
		SIMPLE_Messages::Send(target, "", false);
		break;
	}
	case Button::RECORD: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			StartRecording(*call);
		}
		break;
	}
	case Button::RECORD_PAUSE: {
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			if (call->recording)
			{
				LOG("Pause recording\n");
				UA->RecordPause(call->uid);
			}
			else
			{
				LOG("RECORD_PAUSE: not recording\n");
			}
		}
		break;
	}
	case Button::SHOW_SETTINGS:
		if (appSettings.frmMain.bHideSettings == false)
		{
			actShowSettingsExecute(NULL);
        }
		break;
	case Button::MINIMIZE:
		Application->Minimize();
		break;
	case Button::SHOW_LOG:
		actShowLogExecute(NULL);
		break;
	case Button::EXIT:
		actExitExecute(NULL);	
		break;
	case Button::UA_RESTART:
		SetStatus("Restarting UA (manual restart)...");
		Ua::Instance().Restart();
		break;
	case Button::AUTO_ANSWER_DND:
		if (down)
		{
			appSettings.uaConf.autoAnswer = false;
			UpdateAutoAnswer();
		}
		else
		{
			appSettings.uaConf.autoAnswer = true;
			appSettings.uaConf.autoAnswerCode = cfg.sipCode;
			appSettings.uaConf.autoAnswerReason = cfg.sipReason;
			UpdateAutoAnswer();
		}
		appSettings.Write(Paths::GetConfig());		
		break;
	case Button::ZRTP_VERIFY_SAS:
		UA->ZrtpVerifySas(true);
		break;
	case Button::ZRTP_UNVERIFY_SAS:
		UA->ZrtpVerifySas(false);
		break;
	case Button::LINE: {
		tmrClearCallState->Enabled = false;
		unsigned int prevUid = Calls::GetCurrentCallUid();
		Calls::OnLineButtonClick(id, btn);
		if (prevUid != Calls::GetCurrentCallUid())
		{
			UpdateMainCallDisplay();
		}
		break;
	}

	default:
		LOG("Unhandled BTN type = %d\n", static_cast<int>(cfg.type));
		break;
	}
}

void TfrmMain::OnProgrammableBtnMouseUpDown(int id, TProgrammableButton* btn)
{
	assert(id >= 0 && id < buttons.btnConf.size());

	if (appSettings.Scripts.onProgrammableButtonMouseUpDown != "")
	{
		AnsiString asScriptFile;
		bool handled = false;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onProgrammableButtonMouseUpDown.c_str());
		RunScriptFile(SCRIPT_SRC_BUTTON_MOUSE_UP_DOWN, id, asScriptFile.c_str(), handled);
	}
}

void TfrmMain::OnRestartUa(void)
{
	if (!appSettings.uaConf.disableUa)
	{
		buttons.UpdateContacts(appSettings.uaConf.contacts);
		Ua::Instance().Restart();
	}
}

void TfrmMain::RunScriptFile(int srcType, int srcId, AnsiString filename, bool &handled, bool showLog)
{
	if (showLog)
	{
    	LOG("Running Lua script: %s\n", ExtractFileName(filename).c_str());
	}
	std::auto_ptr<TStrings> strings(new TStringList());
	if (FileExists(filename))
	{
		AnsiString scriptText;
		try
		{
			strings->LoadFromFile(filename);
			scriptText = strings->Text;
		}
		catch(...)
		{
			AnsiString msg;
			msg.sprintf("Failed to load script file (%s).", filename.c_str());
			MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		bool breakReq = false;
		RunScript(srcType, srcId, scriptText, breakReq, handled);
	}
	else
	{
		AnsiString msg;
		msg.sprintf("Script file not found (%s).", filename.c_str());
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
	}
}

int TfrmMain::RunScript(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled)
{
	ScriptExec scriptExec(
		static_cast<enum ScriptSource>(srcType), srcId, breakRequest, handled,
		&MakeCall, &Hangup, &Answer, &OnGetDial, &OnSetDial,
		&DialString,
		&OnGetContactName,
		&OnGetStreamingState,
		&OnSetTrayIcon,
		&OnGetRegistrationState,
		&OnPluginSendMessageText, &OnPluginEnable,
		&OnRecordStart,
		&ShowTrayNotifier,
		&HideTrayNotifier,
		&UpdateSettingsFromJson,
		&UpdateButtonsFromJson,
		&MainMenuShow,
		&ApplicationShow,
		&ApplicationHide,
		&ApplicationClose
		);
	scriptExec.Run(script.c_str());
	return 0;
}

void __fastcall TfrmMain::edTransferEnter(TObject *Sender)
{
	if (edTransfer->Text == asTransferHint)
	{
		edTransfer->Text = "";
	}
}
//---------------------------------------------------------------------------


void __fastcall TfrmMain::btnSpeedDialPanelClick(TObject *Sender)
{
	ToggleSpeedDial();
}
//---------------------------------------------------------------------------

void TfrmMain::UpdateLogConfig(void)
{
	CLog *log = CLog::Instance();
	if (appSettings.Logging.bLogToFile)
		log->SetFile((Paths::GetProfileDir() + "\\" + ChangeFileExt(ExtractFileName(Application->ExeName), ".log")).c_str());
	else
		log->SetFile("");
	log->SetFlush(appSettings.Logging.bFlush);
	log->SetMaxFileSize(appSettings.Logging.iMaxFileSize);
	log->SetLogRotateCnt(appSettings.Logging.iLogRotate);
}

void TfrmMain::SetSpeedDial(bool visible)
{
	UpdateBtnConsole();
	UpdateSize();
}

void TfrmMain::ToggleSpeedDial(void)
{
	appSettings.frmMain.bSpeedDialVisible = !appSettings.frmMain.bSpeedDialVisible;
	if (appSettings.frmMain.expandingPosLeftOffset != 0)
	{
		if (appSettings.frmMain.bSpeedDialVisible)
		{
			this->Left = this->Left + appSettings.frmMain.expandingPosLeftOffset;
		}
		else
		{
			this->Left = this->Left - appSettings.frmMain.expandingPosLeftOffset;
		}
	}
	if (appSettings.frmMain.expandingPosTopOffset != 0)
	{
		if (appSettings.frmMain.bSpeedDialVisible)
		{
			this->Top = this->Top + appSettings.frmMain.expandingPosLeftOffset;
		}
		else
		{
			this->Top = this->Top - appSettings.frmMain.expandingPosLeftOffset;
		}
	}
	SetSpeedDial(appSettings.frmMain.bSpeedDialVisible);
}

void __fastcall TfrmMain::cbCallURIKeyPress(TObject *Sender, char &Key)
{
	Call* call = Calls::GetCurrentCall();

	if (Key == VK_RETURN)
	{
		if (call == NULL && !pagingTx.active)
		{
			AnsiString target = cbCallURI->Text.Trim();
			if (target != "")
			{
				unsigned int callId;
				MakeCall(target, callId);
			}
		}
		if (appSettings.frmMain.bNoBeepOnEnterKey)
		{
			// this was unintentional, but it looks like Windows beep is meaning that [Enter] is not accepted by the control
			Key = NULL;
		}
		return;
	}

	if (call)
	{
		const char* dtmfKeys = "01234567890*#ABCDabcd";
		if (strchr(dtmfKeys, Key))
		{
			if (call->connected || call->progress)
			{
				UA->SendDigit(call->uid, toupper(Key));
			}
		}
	}

	if (appSettings.Scripts.onDial != "")
	{
		AnsiString asScriptFile;
		bool handled = true;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onDial.c_str());
		RunScriptFile(SCRIPT_SRC_ON_DIAL, Key, asScriptFile.c_str(), handled);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnBackspaceClick(TObject *Sender)
{
	CallNumberBackspace();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnBackspaceMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	tmrBackspace->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnBackspaceMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	tmrBackspace->Enabled = false;
}
//---------------------------------------------------------------------------

void TfrmMain::CallNumberBackspace(void)
{
	cbCallURI->Text = cbCallURI->Text.SubString(1, cbCallURI->Text.Length() - 1);
	cbCallURI->SelStart = cbCallURI->Text.Length();
	PhoneInterface::UpdateCallState(Calls::Count()?1:0, cbCallURI->Text.c_str());
}

void __fastcall TfrmMain::tmrBackspaceTimer(TObject *Sender)
{
	CallNumberBackspace();
#if 0
	// debug
	LOG("RX signal level = %u\n", Ua::Instance().GetAudioRxSignalLevel());
#endif
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::OnTrayIconLeftBtnDown(TObject *Sender)
{
	ToggleVisibility();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miMinimizeTrayClick(TObject *Sender)
{
	if (trIcon)
	{
		Visible = false;
		ShowWindow(Application->Handle, SW_HIDE);	// hide taskbar button
	}
	else
	{
		AnsiString msg = "Tray icon is disabled in configuration";
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONINFORMATION);
	}
}
//---------------------------------------------------------------------------

void TfrmMain::ToggleVisibility(void)
{
	if (trIcon == NULL)
	{
    	return;
	}

	Visible = !Visible;
	if (Visible)
	{
		Application->Restore();
		if (appSettings.frmMain.bNoTaskbarButtonRestore == false)
		{
			ShowWindow(Application->Handle, SW_SHOW);	// show taskbar button
		}
		SetActiveWindow (Handle);
		SetForegroundWindow (Handle);
		SetWindowPos (Handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		FocusCbCallUri();
		frmHistory->SetUpdating(pcMain->ActivePage == tsHistory);
		OnRestore(NULL);
	}
	else
	{
		ShowWindow(Application->Handle, SW_HIDE);	// hide taskbar button
		frmHistory->SetUpdating(false);
	}
}

void TfrmMain::SetTrayIconHint(AnsiString text)
{
	if (trIcon)
	{
		trIcon->SetHint(text);
	}
}

void __fastcall TfrmMain::actExitExecute(TObject *Sender)
{
    Finalize();
	Ua::Instance().Quit();
	Application->Terminate();
	Close();
}
//---------------------------------------------------------------------------

void TfrmMain::AutoAnswer(Call &call)
{
	if (call.autoAnswerCode == 200) {
		if (call.autoAnswerIntercom) {
			LOG("Answering with module %s, device %s\n", appSettings.uaConf.audioCfgPlayIntercom.mod.c_str(), appSettings.uaConf.audioCfgPlayIntercom.dev.c_str());
			UA->Answer(call.uid, appSettings.uaConf.audioCfgPlayIntercom.mod.c_str(), appSettings.uaConf.audioCfgPlayIntercom.dev.c_str(), appSettings.video.enabled, GetDisplayParentHandle());
		} else {
			LOG("Auto-answer\n", call.autoAnswerCode);
			Answer(call.uid);
		}
		if (appSettings.trayNotifier.hideWhenAnsweringCallAutomatically)
		{
			frmTrayNotifier->HideWindow();
		}
	} else if (call.autoAnswerCode >= 400) {
		LOG("Auto-answer (DND) with SIP code = %d\n", call.autoAnswerCode);
		UA->Hangup(call.uid, call.autoAnswerCode, appSettings.uaConf.autoAnswerReason.c_str());
		if (&call == Calls::GetCurrentCall())
		{
			lbl2ndParty->Caption = "";
			lbl2ndPartyDesc->Caption = "";
			lblCallState->Caption = "";
		}
		call.incoming = false;
		call.progress = false;
		call.connected = false;
		if (appSettings.trayNotifier.hideWhenAnsweringCallAutomatically)
		{
			frmTrayNotifier->HideWindow();
		}
	}
}

void __fastcall TfrmMain::tmrAutoAnswerTimer(TObject *Sender)
{
	TTimer *tmr = dynamic_cast<TTimer*>(Sender);
	assert(tmr);
	tmr->Enabled = false;

	Call *call = Calls::FindByAutoAnswerTimer(tmr);
	if (call)
		AutoAnswer(*call);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edTransferKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_RETURN)
	{
		if (appSettings.frmMain.bNoBeepOnEnterKey)
		{
			Key = NULL;
		}	
		if (edTransfer->Text == asTransferHint || edTransfer->Text == "")
			return;
		Call *call = Calls::GetCurrentCall();
		if (call)
		{
			UA->Transfer(call->uid, edTransfer->Text);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::WMCopyData(TWMCopyData& Message)
{
    char *data = new char [Message.CopyDataStruct->cbData+1];
	memcpy(data, Message.CopyDataStruct->lpData, Message.CopyDataStruct->cbData);
    data[Message.CopyDataStruct->cbData] = '\0';
    CommandLine::Instance().Execute(data, Message.CopyDataStruct->cbData/CommandLine::MAX_CMD_PARAM_LEN);
    delete[] data;

    HandleCommandLine();

    Message.Result = 1; // send sth back
}

void __fastcall TfrmMain::WMHotKey(TWMHotKey &Message)
{
	TForm::Dispatch(&Message);
#if 1
	LOG("WMHotKey: Message.Hotkey = %d\n", static_cast<int>(Message.HotKey));
#endif
	const HotKeyConf* cfg = hotKeys.FindGlobal(Message.HotKey);
	if (IsWin7OrLater() == false)
	{
		// MOD_NOREPEAT not supported, use own antirepeat
		if (cfg == lastHotkey)
		{
			// anti-repeat
			return;
		}
		lastHotkey = cfg;
	}
	if (cfg)
	{
		LOG("Received hotkey: %s -> action: %s\n", cfg->GetDescription().c_str(), Action::getTypeDescription(cfg->action.type));
		ExecAction(cfg->action);
		if (IsWin7OrLater() == false)
		{
			tmrAntirepeat->Enabled = false;
			tmrAntirepeat->Enabled = true;
		}
	}
}

void TfrmMain::HandleCommandLine(void)
{
	CommandLine &cmd = CommandLine::Instance();
	if (cmd.action == CommandLine::ACTION_NONE)
	{
		return;
	}
	LOG("Handling commnand line\n");
	if (cmd.action == CommandLine::ACTION_HANGUP)
	{
		LOG("action = HANGUP\n");
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			Hangup(call->uid);
		}
	}
	else if (cmd.action == CommandLine::ACTION_ANSWER)
	{
		LOG("action = ANSWER\n");
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			Answer(call->uid);
		}
	}
	else if (cmd.action == CommandLine::ACTION_CALL)
	{
		AnsiString target = cmd.asTarget.Trim();
		LOG("action = CALL, target = %s\n", target.c_str());
		if (target != "" && !pagingTx.active)
		{
			unsigned int callUid;
			MakeCall(target, callUid);
		}
		else
		{
			if (target == "")
			{
				LOG("Cannot make call - empty target\n");
			}
			else
			{
				LOG("Cannot make call - call is already active\n");
			}
		}
	}
	else if (cmd.action == CommandLine::ACTION_SHOWWINDOW)
	{
		LOG("action = SHOWWINDOW\n");
		if (!Visible) {
			Visible = true;
			Application->Restore();
			ShowWindow(Application->Handle, SW_SHOW);	// show taskbar button
		}
		SetActiveWindow (Handle);
		SetForegroundWindow (Handle);
		SetWindowPos (Handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		FocusCbCallUri();
	}
	else if (cmd.action == CommandLine::ACTION_APP_QUIT)
	{
		LOG("action = APP_QUIT\n");	
    	actExit->Execute();
	}
	else if (cmd.action == CommandLine::ACTION_PROGRAMMABLE_BTN)
	{
		LOG("action = PROGRAMMABLE_BTN\n");	
		ProgrammableButtonClick(cmd.programmableBtnId);
	}
	else if (cmd.action == CommandLine::ACTION_RECORD_START)
	{
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			StartRecording(*call);
		}
	}
	else if (cmd.action == CommandLine::ACTION_RECORD_PAUSE)
	{
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			if (call->recording)
			{
				LOG("Pause recording\n");
				UA->RecordPause(call->uid);
			}
			else
			{
				LOG("RECORD_PAUSE: not recording\n");
			}
		}
		else
		{
			LOG("RECORD_PAUSE: no current call\n");
		}
	}
	else if (cmd.action == CommandLine::ACTION_SCRIPT)
	{
		LOG("Command line script:\n%s\n", cmd.script.c_str());
		if (appSettings.Integration.bAcceptCommandLineScript == false)
		{
			LOG("Script processing from command line is not enabled in configuration!\n");
			return;
		}
		bool breakReq = false;
		bool handled = false;
		RunScript(SCRIPT_SRC_COMMAND_LINE, 0, cmd.script, breakReq, handled);
	}
	else if (cmd.action == CommandLine::ACTION_SCRIPT_FILE)
	{
		LOG("Command line script file:\n%s\n", cmd.script.c_str());
		if (appSettings.Integration.bAcceptCommandLineScript == false)
		{
			LOG("Script processing from command line is not enabled in configuration!\n");
			return;
		}
		bool breakReq = false;
		bool handled = false;
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), cmd.scriptFile.c_str());
		RunScriptFile(SCRIPT_SRC_COMMAND_LINE, 0, asScriptFile, breakReq, handled);
	}
	else
	{
    	LOG("Unhandled action type!\n");
	}
	cmd.action = CommandLine::ACTION_NONE;
}

void TfrmMain::ProgrammableButtonClick(int buttonId)
{
	TProgrammableButton* btn = buttons.GetBtn(buttonId);
	if (btn)
		btn->OnClick(btn);
}

void TfrmMain::StartRing(Call &call, AnsiString wavFile)
{
	UA->StartRing(call.uid, wavFile);
	PhoneInterface::UpdateRing(1);
}

AnsiString TfrmMain::RingFile(AnsiString alertInfo)
{
	alertInfo = LowerCase(alertInfo);
	AnsiString needle = "bellcore-dr";
	int pos = alertInfo.Pos(needle);
	if (pos && pos < alertInfo.Length())
	{
		char c = alertInfo[pos + needle.Length()];
		c -= '0';
		if (c >=1 && c <= 8)
		{
			AnsiString file = appSettings.Ring.bellcore[c-1];
			if (file != "")
			{
				AnsiString fileFull;
				fileFull.sprintf("%s\\%s", Paths::GetProfileDir().c_str(), file.c_str());
				if (FileExists(fileFull))
					return file;
			}
		}
	}
	return appSettings.Ring.defaultRing;
}

void __fastcall TfrmMain::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
#if 0
	LOG("FormKeyDown: Key = %d, CTRL = %d, SHIFT = %d, ALT = %d\n",
		Key,
		Shift.Contains(ssCtrl), Shift.Contains(ssShift), Shift.Contains(ssAlt));
#endif
	const HotKeyConf* cfg = hotKeys.Find(appSettings.hotKeyConf, Key, Shift, false);
	if (cfg == lastHotkey)
	{
		// anti-repeat
		return;
	}
	lastHotkey = cfg;
	if (cfg)
	{
		LOG("Received hotkey: %s -> action: %s\n", cfg->GetDescription().c_str(), Action::getTypeDescription(cfg->action.type));
		ExecAction(cfg->action);
		tmrAntirepeat->Enabled = false;
		tmrAntirepeat->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void TfrmMain::ExecAction(const struct Action& action)
{
	switch (action.type)
	{
	case Action::TYPE_NONE:
		break;
	case Action::TYPE_SHOWHIDE:
		ToggleVisibility();
		break;
	case Action::TYPE_BUTTON:
		if (action.id >= 0)
		{
			TProgrammableButton* btn = buttons.GetBtn(action.id);
			if (btn)
				btn->OnClick(btn);
		}
		break;
	case Action::TYPE_ANSWER: {
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			Answer(call->uid);
		}
		break;
	}
	case Action::TYPE_HANGUP: {
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			Hangup(call->uid);
		}
		break;
	}
	case Action::TYPE_REDIAL:
		Redial();
		break;
	case Action::TYPE_CALL_CLIPBOARD:
		if (!pagingTx.active)
		{
			AnsiString target = CleanNumber(Clipboard()->AsText);
			if (target != "")
			{
				unsigned int callUid;
				MakeCall(target, callUid);
			}
		}
		break;
	case Action::TYPE_SHOWHIDE_SIDECAR:
		ToggleSpeedDial();
		break;
	case Action::TYPE_ANSWER_HANGUP: {
		// answer incoming call, hangup if call state is other
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			if (call->GetState() == Callback::CALL_STATE_INCOMING)
				Answer(call->uid);
			else
				Hangup(call->uid);
		}
		break;
	}
	case Action::TYPE_SCRIPT_FILE:
		if (action.scriptFile != "")
		{
			AnsiString asScriptFile;
			bool handled = true;
			asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), action.scriptFile.c_str());
			frmMain->RunScriptFile(SCRIPT_SRC_HOTKEY, -1, asScriptFile.c_str(), handled);
		}
		break;
	default:
		break;
	}
}

void __fastcall TfrmMain::tmrAntirepeatTimer(TObject *Sender)
{
	tmrAntirepeat->Enabled = false;
	lastHotkey = NULL;
}
//---------------------------------------------------------------------------

/** Notify on contacts update to invalidate current note window
*/
void TfrmMain::obsUpdate(Observable* o, Argument * arg)
{
	frmContactPopup->InvalidateData();
	lastContactEntry = NULL;
	frmContactPopup->Close();
}

void TfrmMain::OnPhoneKey(int keyCode, int state)
{
	if (state == 0 && keyCode != KEY_HOOK)
	{
		return;
	}

	switch (keyCode)
	{
	case KEY_HOOK:
		if (state == 1)
		{
			Call* call = Calls::GetCurrentCall();
			if (call)
				Hangup(call->uid);
		}
		else
		{
			Call* call = Calls::GetCurrentCall();
			if (call)
				Answer(call->uid);
		}
		break;
	case KEY_0:
	case KEY_1:
	case KEY_2:
	case KEY_3:
	case KEY_4:
	case KEY_5:
	case KEY_6:
	case KEY_7:
	case KEY_8:
	case KEY_9:
		Dial('0' + keyCode - KEY_0);
		break;
	case KEY_STAR:
		Dial('*');
		break;
	case KEY_HASH:
		Dial('#');
		break;
	case KEY_OK: {
		Call* call = Calls::GetCurrentCall();	
		if (call == NULL && !pagingTx.active)
		{
			AnsiString target = cbCallURI->Text.Trim();
			if (target != "")
			{
				unsigned int callUid;
				MakeCall(target, callUid);
			}
		}
		break;
	}
	case KEY_C: {
		Call* call = Calls::GetCurrentCall();
		if (call && call->incoming && !call->connected)
		{
			Hangup(call->uid);
		}
		else
		{
			CallNumberBackspace();
		}
		break;
	}
	case KEY_APP: {
		ToggleVisibility();
		break;
	}
	case KEY_CALL_MAKE_ANSWER: {
		btnMakeCallClick(NULL);
		break;
	}
	case KEY_CALL_HANGUP: {
		btnHangupClick(NULL);
		break;
	}
	case KEY_MUTE_TOGGLE: {
		Call* call = Calls::GetCurrentCall();
		if (call)
		{
			call->setMute(!call->mute);
		}
		break;
	}
	default:
		break;
	}
}

int TfrmMain::OnPhonePagingTx(const char* target, const char* filename, const char* codecname)
{
	Call* call = Calls::GetCurrentCall();
	if (call != NULL || pagingTx.active)
	{
		LOG("Could not start streaming - call or streaming already running\n");
		return -1;
	}
	/** \todo plugins: ptime selection for paging */
	UA->PagingTx(target, filename, codecname, 20);
	return 0;
}

void TfrmMain::OnPhoneClearDial(void)
{
	cbCallURI->Text = "";
}

int TfrmMain::OnGetNumberDescription(const char* number, char* description, int descriptionSize)
{
	AnsiString desc = OnGetContactName(number);
	if (descriptionSize > 0)
	{
		snprintf(description, descriptionSize, "%s", desc.c_str());
		description[descriptionSize-1] = '\0';
	}
	return 0;
}

void __fastcall TfrmMain::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	// Remy: TApplication already catches the WM_ENDSESSION message to set the Application->Terminated property to true.
	// not quite?
	if (Application->Terminated)
	{
		Finalize();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::WMEndSession(TWMEndSession &Msg)
{
	if (Msg.EndSession)
	{
    	Finalize();
	}
}

void __fastcall TfrmMain::actContactsCsvImportExecute(TObject *Sender)
{
	//openDialogCsv->InitialDir = appSettings.Editor.asDefaultDir;
	openDialog->Filter = "CSV files (*.csv)|*.csv|All files (*.*)|*.*";
	openDialog->Title = "Open CSV file";
	if (openDialog->Execute())
	{
		if (frmContactsCsvImport == NULL)
		{
			Application->CreateForm(__classid(TfrmContactsCsvImport), &frmContactsCsvImport);
		}
		frmContactsCsvImport->Load(openDialog->FileName, &contacts);
		if (frmContactsCsvImport->ModalResult == mrOk)
		{
			contacts.Update();
			contacts.Write();
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::pcMainChange(TObject *Sender)
{
	frmHistory->SetUpdating(pcMain->ActivePage == tsHistory);	
}
//---------------------------------------------------------------------------

void TfrmMain::SetNotificationIcon(bool state)
{
	if (trIcon == NULL)
		return;
	if (state == notificationIconState || useOwnTrayIcon || errorIconState /* higher priority */)
		return;
	if (state)
	{
		trIcon->SetIcon(imgListIcons, 0);
	}
	else
	{
		trIcon->SetIcon(Application->Icon);
	}
	notificationIconState = state;
}

void TfrmMain::SetErrorIcon(bool state)
{
	if (trIcon == NULL)
		return;
	if (state == errorIconState || useOwnTrayIcon)
		return;
	if (state)
	{
		trIcon->SetIcon(imgListIcons, 1);
	}
	else
	{
		trIcon->SetIcon(Application->Icon);
	}
	errorIconState = state;	
}

void __fastcall TfrmMain::FormShow(TObject *Sender)
{
	SetNotificationIcon(false);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::FormMouseActivate(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y, int HitTest,
      TMouseActivate &MouseActivate)
{
	SetNotificationIcon(false);
}
//---------------------------------------------------------------------------


void __fastcall TfrmMain::tmrScriptTimer(TObject *Sender)
{
	tmrScript->Enabled = false;
	if (appSettings.Scripts.onTimer != "")
	{
		AnsiString asScriptFile;
		bool handled = true;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onTimer.c_str());
		RunScriptFile(SCRIPT_SRC_ON_TIMER, -1, asScriptFile.c_str(), handled, false);
		tmrScript->Enabled = true;
	}
	// timer disables itself if onTimer script is not configured
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trbarSoftvolMicChange(TObject *Sender)
{
	TTrackBar* tr = trbarSoftvolMic;
	AnsiString asHint;
	appSettings.uaConf.audioSoftVol.tx = tr->Max - tr->Position + tr->Min;
	asHint.sprintf("%d%%", appSettings.uaConf.audioSoftVol.tx*100/128);
	tr->Hint = asHint;
	UA->UpdateVolume();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trbarSoftvolSpeakerChange(TObject *Sender)
{
	TTrackBar* tr = trbarSoftvolSpeaker;
	AnsiString asHint;
	appSettings.uaConf.audioSoftVol.rx = tr->Max - tr->Position + tr->Min;
	asHint.sprintf("%d%%", appSettings.uaConf.audioSoftVol.rx*100/128);
	tr->Hint = asHint;
	UA->UpdateVolume();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnResetMicVolumeClick(TObject *Sender)
{
	trbarSoftvolMic->Position = trbarSoftvolMic->Max + trbarSoftvolMic->Min - 128;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnResetSpeakerVolumeClick(TObject *Sender)
{
	trbarSoftvolSpeaker->Position = trbarSoftvolSpeaker->Max + trbarSoftvolSpeaker->Min - 128;
}
//---------------------------------------------------------------------------

void TfrmMain::SetKioskMode(bool state)
{
	if (state)
	{
		this->Menu = NULL;
		this->StatusBar->Visible = false;
		this->BorderStyle = bsNone;
		//this->Align = alClient;
		//Refresh();
		this->WindowState = wsMaximized;
		this->BringToFront();
		this->Left = 0;
		this->Top = 0;
		this->ClientWidth = Screen->Width;
		this->ClientHeight = Screen->Height;
	}
	else
	{
		if (appSettings.frmMain.bHideMainMenu == false)
		{
			this->Menu = MainMenu;
		}
		if (appSettings.frmMain.bHideStatusBar == false)
		{
			this->StatusBar->Visible = true;
		}
		if (appSettings.frmMain.bBorderless == false)
		{
			this->BorderStyle = bsSingle;
		}
		SetSpeedDial(true);
    }
}

void __fastcall TfrmMain::miClearCallsHistoryClick(TObject *Sender)
{
	if (MessageBox(this->Handle, "Delete call history?",
		this->Caption.c_str(), MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) == IDYES)
	{
		cbCallURI->Clear();
		history.Clear();
		history.Write();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::tmrClearCallStateTimer(TObject *Sender)
{
	tmrClearCallState->Enabled = false;
	lbl2ndParty->Caption = "";
	lbl2ndPartyDesc->Caption = "";
	lblCallState->Caption = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miScriptingClick(TObject *Sender)
{
	TfrmLuaScript *frmLuaScript = new TfrmLuaScript(this);
	frmLuaScript->Show();
	// frmLuaScript is self-deleting with caFree
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miTroubleshootingClick(TObject *Sender)
{
    Troubleshooting::Update();
	if (Troubleshooting::getItems().empty())
	{
		MessageBox(this->Handle, "No issues detected", this->Caption.c_str(), MB_ICONINFORMATION);
	}
	else
	{
		if (frmTroubleshooting == NULL)
			Application->CreateForm(__classid(TfrmTroubleshooting), &frmTroubleshooting);			
		frmTroubleshooting->ShowModal();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::tmrScript2Timer(TObject *Sender)
{
	tmrScript2->Enabled = false;
	if (appSettings.Scripts.onTimer2 != "")
	{
		AnsiString asScriptFile;
		bool handled = true;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onTimer2.c_str());
		RunScriptFile(SCRIPT_SRC_ON_TIMER, -1, asScriptFile.c_str(), handled, false);
		tmrScript2->Enabled = true;
	}
	// timer2 disables itself if onTimer script is not configured
}
//---------------------------------------------------------------------------


void __fastcall TfrmMain::miMessagesClick(TObject *Sender)
{
	TfrmMessage *frmMessage = new TfrmMessage(NULL);
	frmMessage->Show();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miSettingsPatchClick(TObject *Sender)
{
	if (frmSettingsPatch == NULL)
		Application->CreateForm(__classid(TfrmSettingsPatch), &frmSettingsPatch);
	frmSettingsPatch->Caption = "Patch/update main settings";
	frmSettingsPatch->onUpdateSettings = UpdateSettingsFromJson;
	frmSettingsPatch->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miPatchButtonSettingsClick(TObject *Sender)
{
	if (frmSettingsPatch == NULL)
		Application->CreateForm(__classid(TfrmSettingsPatch), &frmSettingsPatch);
	frmSettingsPatch->Caption = "Patch/update button settings";
	frmSettingsPatch->onUpdateSettings = UpdateButtonsFromJson;
	frmSettingsPatch->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miImportContactsFromXmlClick(TObject *Sender)
{
	if (MessageBox(this->Handle,
		"This function reads contacts from XML file with popular Yealink-like format.\r\n"
        "Current contacts would be completely overwritten.\r\n"
		"Continue?",
		"XML import", MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDYES)
	{
		return;
	}
	openDialog->Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";
	openDialog->Title = "Open XML file";
	if (openDialog->Execute())
	{
		if (contacts.ReadXml(openDialog->FileName) == 0)
		{
			contacts.Write();
		}
		else
		{
			Application->MessageBox("Failed to load contacts", "XML import", MB_ICONSTOP);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miRefreshTranslationFromFileClick(TObject *Sender)
{
	LoadTranslation();
}
//---------------------------------------------------------------------------

void TfrmMain::LoadTranslation(void)
{
	int status = LoadTranslations(appSettings.Translation.language, appSettings.Translation.logMissingKeys);
	if (appSettings.Translation.language != "" && status != 0)
	{
		AnsiString msg;
		msg.sprintf("Failed to load %s translation", appSettings.Translation.language.c_str());
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
	}
}

void TfrmMain::UpdateAutoAnswer(void)
{
	buttons.UpdateAutoAnswer(appSettings.uaConf.autoAnswer, appSettings.uaConf.autoAnswerCode, appSettings.uaConf.autoAnswerReason.c_str());
}

void __fastcall TfrmMain::btnResetMicVolumeMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	ActiveControl = NULL;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::btnResetSpeakerVolumeMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	ActiveControl = NULL;	
}


void TfrmMain::ClearLineButton(int btnId)
{
	if (btnId < 0)
		return;

	TProgrammableButton *btn = buttons.GetBtn(btnId);
	if (btn == NULL)
		return;
#if 0
	btn->SetCaption("LINE");
	btn->SetCaption2(Callback::GetCallStateDescription(Callback::CALL_STATE_CLOSED));
#else
	// restore initial button captions
	const ButtonConf &cfg = buttons.btnConf[btnId];
	btn->SetCaption(cfg.caption.c_str());
	btn->SetCaption2(cfg.caption2.c_str());
#endif
}

void TfrmMain::UpdateMainCallDisplay(void)
{
	Call *call = Calls::GetCurrentCall();
	if (call == NULL)
	{
		lblCallState->Caption = "";
		lbl2ndParty->Caption = "";
		lbl2ndPartyDesc->Caption = "";
	}
	else
	{
		if(call->GetState() == Callback::CALL_STATE_CLOSED)
		{
			cbCallURI->Text = "";
		}
		else if (cbCallURI->Text != call->dialString)
		{
        	cbCallURI->Text = call->dialString;
		}
		lblCallState->Caption = call->getStateTranslatedDescription();
		lbl2ndParty->Caption = GetClip(call->getPeerUri(), appSettings.Display.bUserOnlyClip);
		lastContactEntry = contacts.GetEntry(CleanUri(call->getPeerUri()));
		if (lastContactEntry)
		{
			lbl2ndPartyDesc->Caption = lastContactEntry->description;
			/** \todo popup contact note if clip changes */
		#if 0
			if (appSettings.frmContactPopup.showOnIncoming)
			{
				if (lastContactEntry->note != "")
				{
					ShowContactPopup(lastContactEntry);
				}
			}
		#endif
		}
		else
		{
			lbl2ndPartyDesc->Caption = call->getPeerName();
		}
	}
}

void __fastcall TfrmMain::OnRestore(TObject *Sender)
{
	bool monitorFound = false;
	for (int i=0; i<Screen->MonitorCount; i++)
	{
		TMonitor *monitor = Screen->Monitors[i];
		enum { MARGIN = 30 };
		if (
			(Left + Width + MARGIN >= monitor->Left) &&
			(Left - MARGIN <= monitor->Left + monitor->Width) &&
			(Top + 5 >= monitor->Top) &&
			(Top - MARGIN <= monitor->Top + monitor->Height)
			)
		{
			monitorFound = true;
			break;
		}
	}
	if (!monitorFound)
	{
		if (Screen->MonitorCount > 0)
		{
			TMonitor *monitor = Screen->Monitors[0];
			if (monitor)
			{
				LOG("Moving main window to first monitor\n");
				Left = monitor->Left + 50;
				Top = monitor->Top + 50;
			}
		}
	}
}

void __fastcall TfrmMain::cbCallURIChange(TObject *Sender)
{
	Call* call = Calls::GetCurrentCall();
	if (call)
	{
		call->dialString = cbCallURI->Text;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miScriptPluginVariablesClick(TObject *Sender)
{
	if (frmScriptPluginVariables == NULL)
	{
		Application->CreateForm(__classid(TfrmScriptPluginVariables), &frmScriptPluginVariables);
	}
	frmScriptPluginVariables->Show();
}
//---------------------------------------------------------------------------


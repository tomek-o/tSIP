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
#include "Call.h"
#include "Recorder.h"
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
	AnsiString asTransferHint = "Transfer to ... [Enter]";
	bool useOwnTrayIcon = false;
	unsigned int audioErrorCount;

	Call call;
	Call previousCall;
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
	Recorder recorder;

	Contacts::Entry *lastContactEntry = NULL;
	void ShowContactPopup(Contacts::Entry *entry)
	{
		if (frmContactPopup->isNoteModified() && !appSettings.Contacts.storeNoteInSeparateFile)
		{
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

	AnsiString GetCallPeerUri(void)
	{
		if (appSettings.Display.bUsePAssertedIdentity)
		{
			if (call.paiPeerUri != "")
				return call.paiPeerUri;
		}
		return call.uri;
	}

	AnsiString GetCallPeerName(void)
	{
		if (appSettings.Display.bUsePAssertedIdentity)
		{
			if (call.paiPeerUri != "")	// using uri to check if header line is present as it shouldn't be empty
				return call.paiPeerName;
		}
		return call.peerName;
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
	TRANSLATE_TMP("TfrmMain.miMessages", frm->miMessages->Caption);
	TRANSLATE_TMP("TfrmMain.miTroubleshooting", frm->miTroubleshooting->Caption);
	TRANSLATE_TMP("TfrmMain.miAbout", frm->miAbout->Caption);
}

//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner),
	initialScaling(1.0),
	trIcon(NULL),
	muteRing(false),
	notificationIconState(false),
	errorIconState(false),
	appState(Callback::APP_STATE_UNKNOWN)
{
	srand(time(0));
	RegisterTranslationCb(this, TranslateForm);
	lbl2ndParty->Caption = "";
	lbl2ndPartyDesc->Caption = "";
	lblCallState->Caption = "";
	frmHistory = new TfrmHistory(this->tsHistory, &history, appSettings.history, &OnCall, &OnPhonebookEdit, &OnHttpQuery);
	frmHistory->Scale(appSettings.gui.scalingPct);
	frmHistory->Parent = tsHistory;
	frmHistory->Visible = true;

	AnsiString asButtonsFile;
	asButtonsFile.sprintf("%s\\%s_buttons.json", Paths::GetProfileDir().c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	buttons.SetFilename(asButtonsFile);
	buttons.Read();
	buttons.UpdateContacts(appSettings.uaConf.contacts);

    buttons.SetScalingPercentage(appSettings.gui.scalingPct);

	TfrmButtonContainer *frmButtonContainerBasic = new TfrmButtonContainer(
		this->pnlDialpad,
		buttons,
		0,
		this->pnlDialpad->Width, this->pnlDialpad->Height, appSettings.gui.scalingPct,
		&OnSetKeepForeground,
		appSettings.frmSpeedDial.showStatus, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
	frmButtonContainerBasic->Parent = this->pnlDialpad;
	frmButtonContainerBasic->UpdateBackgroundImage();
	frmButtonContainerBasic->Visible = true;
	frmButtonContainers[0] = frmButtonContainerBasic;

	trbarSoftvolMic->Parent = frmButtonContainerBasic;
	trbarSoftvolSpeaker->Parent = frmButtonContainerBasic;
	edTransfer->Enabled = false;	// this eliminates ugly effect with edTransfer content being selected for a moment at the startup - probably because of parent change
	edTransfer->Parent = frmButtonContainerBasic;
	btnResetMicVolume->Parent = frmButtonContainerBasic;
	btnResetSpeakerVolume->Parent = frmButtonContainerBasic;

	for (int i=1; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container = new TfrmButtonContainer(this,
			buttons,
			i,
			300, 0, appSettings.gui.scalingPct,
			&OnSetKeepForeground,
			appSettings.frmSpeedDial.showStatus, appSettings.frmSpeedDial.statusPanelHeight, appSettings.frmSpeedDial.hideEmptyStatus);
		container->Parent = this; //this->pnlSpeedDial;
		container->UpdateBackgroundImage();
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

	frmContacts = new TfrmContacts(this->tsContacts, &contacts, &OnCall);
	frmContacts->Scale(appSettings.gui.scalingPct);
	frmContacts->Parent = tsContacts;
	frmContacts->Visible = true;
	frmContacts->FilterUsingNote(appSettings.Contacts.filterUsingNote);
	frmContacts->StoreNoteInSeparateFile(appSettings.Contacts.storeNoteInSeparateFile);
	edTransfer->Text = asTransferHint;

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

	//edTransfer->Text = asTransferHint;			

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

	//btnAutoAnswer->Down = appSettings.uaConf.autoAnswer;
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

	UpdateCallHistory();
	UpdateAutoAnswer();
}
//---------------------------------------------------------------------------

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
	frmAbout->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowSettingsExecute(TObject *Sender)
{
	Settings prev = appSettings;	// keep track what is changed
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
			container->UpdateSettings();
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
	frmTrayNotifier->UpdateBackgroundImage();
	/** \todo bug? use prev settings */
	frmTrayNotifier->ScaleBy(100, appSettings.frmTrayNotifier.scalingPct);
	frmTrayNotifier->ScaleBy(appSettings.frmTrayNotifier.scalingPct, 100);

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

	{
		// trying to fight with inconsistent state of tray notifier when trying to show it without activation
		frmTrayNotifier->Show();
		frmTrayNotifier->Hide();
	}

	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
	
	edTransfer->Enabled = true;

    frmTrayNotifier->Caption = Branding::appName;
	frmTrayNotifier->OnHangup = Hangup;
	frmTrayNotifier->OnAnswer = Answer;
	frmTrayNotifier->UpdateBackgroundImage();
	frmTrayNotifier->ScaleBy(appSettings.frmTrayNotifier.scalingPct, 100);

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
	PhoneInterface::callbackSetVariable = OnSetVariable;
	PhoneInterface::callbackClearVariable = OnClearVariable;
	PhoneInterface::callbackQueuePush = OnQueuePush;
	PhoneInterface::callbackQueuePop = OnQueuePop;
	PhoneInterface::callbackQueueClear = OnQueueClear;
	PhoneInterface::callbackQueueGetSize = OnQueueGetSize;
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
	cbCallURI->Clear();
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

void TfrmMain::ShowTrayNotifier(AnsiString description, AnsiString uri, bool incoming)
{
	frmTrayNotifier->SetData(description, uri, incoming);
	frmTrayNotifier->ShowWithoutFocus();
}

void TfrmMain::HideTrayNotifier(void)
{
	frmTrayNotifier->HideWindow();
}

void __fastcall TfrmMain::btnMakeCallClick(TObject *Sender)
{
	if (call.uri == "" && call.incoming == false && !pagingTx.active)
	{
		AnsiString target = cbCallURI->Text.Trim();
		if (target != "")
		{
			MakeCall(target);
		}
	}
	else if (call.incoming)
	{
		Answer();
		//UA->Answer(0, appSettings.uaConf.audioCfgPlayIntercom.mod, appSettings.uaConf.audioCfgPlayIntercom.dev);
	}
}
//---------------------------------------------------------------------------

void TfrmMain::MakeCall(AnsiString target)
{
	call.reset();
	call.timestamp = Now();
	call.incoming = false;
	call.uri = target;
	call.initialTarget = call.uri;

	if (appSettings.frmMain.bShowWhenMakingCall)
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

	if (appSettings.Scripts.onMakeCall != "")
	{
		AnsiString asScriptFile;
		bool handled = false;
		asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onMakeCall.c_str());
		// this script may change initial target implementing SIP originate function
		RunScriptFile(SCRIPT_SRC_ON_MAKING_CALL, -1, asScriptFile.c_str(), handled);
		if (handled)
		{
			return;
		}
	}

	call.displayParentHandle = GetDisplayParentHandle();
	UA->Call(0, call.initialTarget, appSettings.Calls.extraHeaderLines, appSettings.video.enabled, call.displayParentHandle);
}

void __fastcall TfrmMain::btnHangupClick(TObject *Sender)
{
	Hangup();
	UA->PlayStop();
}
//---------------------------------------------------------------------------

void TfrmMain::Hangup(int sipCode, AnsiString reason)
{
	call.disconnecting = true;
	UA->Hangup(0, sipCode, reason);
}

void TfrmMain::Answer(void)
{
	if (call.incoming)
	{
		call.displayParentHandle = GetDisplayParentHandle();
		UA->Answer(0, "", "", appSettings.video.enabled, call.displayParentHandle);
		if (appSettings.frmMain.bShowWhenAnsweringCall)
		{
			if (!Visible)
			{
                ToggleVisibility();
            }
		}
		if (appSettings.frmTrayNotifier.hideWhenAnsweringCall)
		{
			frmTrayNotifier->HideWindow();
        }
	}
	else
	{
    	LOG("Answer: no incoming call, current call.state = %d\n", call.state);
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

void TfrmMain::OnSwitchAudioSource(std::string mod, std::string dev)
{
	UA->SwitchAudioSource(0, mod.c_str(), dev.c_str());
}

void TfrmMain::OnBlindTransfer(const std::string& target)
{
	UA->Transfer(0, target.c_str());
}

Call* TfrmMain::OnGetCall(void)
{
	return &call;
}

void TfrmMain::OnResetCall(void)
{
	call.reset();
}

Call* TfrmMain::OnGetPreviousCall(void)
{
	return &previousCall;
}

Recorder* TfrmMain::OnGetRecorder(int id)
{
	if (id == recorder.id)
	{
		return &recorder;
	}
	return NULL;
}

int TfrmMain::OnGetContactId(const char* user)
{
	for (unsigned int i=0; i<appSettings.uaConf.contacts.size(); i++)
	{
		if (appSettings.uaConf.contacts[i].user == user)
			return i;
	}
	return -1;
}

int TfrmMain::OnGetBlfState(int contactId, std::string &number, std::string &remoteIdentity, std::string &remoteIdentityDisplay, enum dialog_info_direction &direction)
{
	if (contactId < 0 || contactId >= appSettings.uaConf.contacts.size())
	{
        LOG("OnGetBlfState: invalid contactId = %d\n", contactId);
		return -1;
    }
	const UaConf::Contact &contact = appSettings.uaConf.contacts[contactId];
	number = contact.user;
	remoteIdentity = contact.remoteIdentity.c_str();
	remoteIdentityDisplay = contact.remoteIdentityDisplay.c_str();
	direction = contact.direction;
	return contact.dialog_info_state;
}

int TfrmMain::OnGetStreamingState(void)
{
	return pagingTx.state;
}

unsigned int TfrmMain::OnGetAudioErrorCount(void)
{
	return audioErrorCount;
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

int TfrmMain::OnRecordStart(const char* file, int channels, int side, int fileFormat, unsigned int bitrate)
{
	if (appSettings.uaConf.recording.enabled == false)
	{
		LOG("OnRecordStart: recording is not enabled in configuration!\n");
		return -1;
	}
	if (call.connected == false && call.progress == false)
	{
		LOG("OnRecordStart: no current call with active media\n");
		return -2;
	}
	UA->Record(file, channels, side, fileFormat, bitrate);
	call.recordFile = file;
	call.recording = true;
	return 0;
}

std::string TfrmMain::OnGetRxDtmf(void)
{
	if (call.dtmfRxQueue.empty())
		return "";
	std::string ret(1, call.dtmfRxQueue[0]);
	call.dtmfRxQueue.pop_front();
	return ret;
}

std::string TfrmMain::OnGetUserName(void)
{
	if (appSettings.uaConf.accounts.empty())
		return "";
	return appSettings.uaConf.accounts[0].user.c_str();
}

const ButtonConf* TfrmMain::OnGetButtonConf(int id)
{
	if (id < 0 || id >= buttons.btnConf.size())
	{
		return NULL;
	}
	return &buttons.btnConf[id];
}

void TfrmMain::MainMenuShow(bool state)
{
	this->Menu = (state)?(MainMenu):(NULL);
}

void TfrmMain::ApplicationClose(void)
{
	actExit->Execute();
}

AnsiString TfrmMain::CleanUri(AnsiString uri)
{
	AnsiString res = uri;
	int start = uri.Pos("<");
	if (start == 0)
		start = 1;
	int end = uri.Pos(">");
	if (end == 0 || (uri.Pos(";") < end))
		end = uri.Pos(";");
	if (start && end && end > start)
	{
		res = uri.SubString(start, end-start);
	}
	return res;
}

AnsiString TfrmMain::GetClip(AnsiString uri)
{
	AnsiString res;
	if (appSettings.Display.bUserOnlyClip)
	{
		res = ExtractNumberFromUri(uri);
		if (res != "")
		{
			return res;
		}
		else
		{
			return CleanUri(uri);
		}
	}
	else
	{
		return CleanUri(uri);
	}
}

void TfrmMain::UpdateClip(void)
{
	lbl2ndParty->Caption = GetClip(GetCallPeerUri());
	lastContactEntry = contacts.GetEntry(CleanUri(GetCallPeerUri()));
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
		lbl2ndPartyDesc->Caption = GetCallPeerName();
	}
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
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_ED_TRANSFER];
		edTransfer->Visible = el.visible;
		edTransfer->Left = el.left;
		edTransfer->Top = el.top;
		edTransfer->Width = el.width;
		edTransfer->Height = el.height;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_TRBAR_MIC_VOLUME];
		trbarSoftvolMic->Visible = el.visible;
		trbarSoftvolMic->Left = el.left;
		trbarSoftvolMic->Top = el.top;
		trbarSoftvolMic->Width = el.width;
		trbarSoftvolMic->Height = el.height;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_BTN_RESET_MIC_VOLUME];
		btnResetMicVolume->Visible = el.visible;
		btnResetMicVolume->Left = el.left;
		btnResetMicVolume->Top = el.top;
		btnResetMicVolume->Width = el.width;
		btnResetMicVolume->Height = el.height;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_TRBAR_SPEAKER_VOLUME];
		trbarSoftvolSpeaker->Visible = el.visible;
		trbarSoftvolSpeaker->Left = el.left;
		trbarSoftvolSpeaker->Top = el.top;
		trbarSoftvolSpeaker->Width = el.width;
		trbarSoftvolSpeaker->Height = el.height;
	}
	{
		const DialpadConf::ElementConf &el = appSettings.dialpad.elements[DialpadConf::EL_BTN_RESET_SPEAKER_VOLUME];
		btnResetSpeakerVolume->Visible = el.visible;
		btnResetSpeakerVolume->Left = el.left;
		btnResetSpeakerVolume->Top = el.top;
		btnResetSpeakerVolume->Width = el.width;
		btnResetSpeakerVolume->Height = el.height;
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
			if (call.connected)
			{
				unsigned int delta = SecondsBetween(Now(), call.timeTalkStart);
				AnsiString caption;
				caption.sprintf("Connected, %02d:%02d", delta/60, delta%60);
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
		if ((appState == Callback::APP_INIT_FAILED || appState == Callback::APP_START_FAILED) && !frmSettings->Visible)
		{
			pollCnt++;
			if (pollCnt > 18000)
			{
				pollCnt = 0;
				SetStatus("Restarting UA (after init error)...");
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
			AnsiString asStateText;
			tmrClearCallState->Enabled = false;
			switch(cb.state)
			{
			case Callback::CALL_STATE_INCOMING:
				asStateText = "Incoming call";
				call.incoming = true;
				call.timestamp = Now();
				call.uri = cb.caller;
				call.peerName = GetPeerName(cb.callerName);
				call.recordFile = "";
				call.dtmfRxQueue.clear();
				call.paiPeerUri = cb.paiPeerUri;
				call.paiPeerName = GetPeerName(cb.paiPeerName);
				autoAnswerIntercom = false;
				if (appSettings.uaConf.autoAnswerCallInfo && cb.callAnswerAfter >= 0)
				{
					LOG("Intercom/paging auto answer, answer-after = %d\n", cb.callAnswerAfter);
					autoAnswerCode = 200;
					autoAnswerIntercom = true;
					int time = cb.callAnswerAfter * 1000;
					if (time < appSettings.uaConf.autoAnswerCallInfoDelayMin)
					{
						time = appSettings.uaConf.autoAnswerCallInfoDelayMin;
					}
					if (time == 0)
					{
						AutoAnswer();
						answered = true;
					}
					else
					{
						LOG("Delayed auto answer, time = %u ms\n", time);
						tmrAutoAnswer->Enabled = false;
						tmrAutoAnswer->Interval = time;
						tmrAutoAnswer->Enabled = true;
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
					autoAnswerCode = appSettings.uaConf.autoAnswerCode;
					if (time == 0)
					{
						AutoAnswer();
						answered = true;
						if (appSettings.uaConf.autoAnswerCode >= 400)
						{
							asStateText = "";
						}
					}
					else
					{
						LOG("Delayed auto answer, time = %u ms\n", time);
						tmrAutoAnswer->Enabled = false;
						tmrAutoAnswer->Interval = time;
						tmrAutoAnswer->Enabled = true;
					}
				}
				if (answered == false && muteRing == false)
				{
					StartRing(RingFile(cb.alertInfo));
				}
				lbl2ndParty->Caption = GetClip(GetCallPeerUri());
				lastContactEntry = contacts.GetEntry(CleanUri(GetCallPeerUri()));
				if (lastContactEntry)
				{
					lbl2ndPartyDesc->Caption = lastContactEntry->description;
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
				else
				{
					lbl2ndPartyDesc->Caption = GetCallPeerName();
				}
				PhoneInterface::UpdateCallState(1, ExtractNumberFromUri(GetCallPeerUri()).c_str()); //CleanUri(cb.caller).c_str());
				if (appSettings.HttpQuery.openMode == Settings::_HttpQuery::openAutomaticOnIncoming)
				{
					HttpQuery();
				}

				call.accessUrl = cb.accessUrl;
				call.accessUrlMode = cb.accessUrlMode;
				if (appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeAlwaysActive ||
					(appSettings.SipAccessUrl.accessMode == Settings::_SipAccessUrl::accessModeFromMsg && call.accessUrlMode == 2)) {
					AccessCallUrl();
				}

				break;
			case Callback::CALL_STATE_OUTGOING:
				asStateText = "Calling...";
				lbl2ndParty->Caption = GetClip(cb.caller);
				lastContactEntry = contacts.GetEntry(CleanUri(cb.caller));
				call.recordFile = "";
				if (lastContactEntry)
				{
					lbl2ndPartyDesc->Caption = lastContactEntry->description;
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
					lbl2ndPartyDesc->Caption = GetPeerName(cb.callerName);
				}

				if (Visible == false && appSettings.frmTrayNotifier.showOnOutgoing)
				{
					ShowTrayNotifier(lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, call.incoming);
				}
				PhoneInterface::UpdateCallState(1, ExtractNumberFromUri(cb.caller).c_str()); // CleanUri(cb.caller).c_str());

				break;
			case Callback::CALL_STATE_TRYING:
				asStateText = "Trying...";
				break;
			case Callback::CALL_STATE_RINGING:
				asStateText = "Ringback";
				break;
			case Callback::CALL_STATE_PROGRESS:
				call.progress = true;
				asStateText = "Call state progress";
				if (appSettings.uaConf.recording.enabled && call.recording == false && (appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallEarly))
				{
					StartRecording();
				}
				break;
			case Callback::CALL_STATE_ESTABLISHED:
				asStateText = "Connected";
				call.connected = true;
				call.timeTalkStart = Now();
				call.ringStarted = false;
				call.paiPeerUri = cb.paiPeerUri;
				call.paiPeerName = GetPeerName(cb.paiPeerName);
				call.codecName = cb.codecName;
				call.lastScode = 200;
				call.lastReplyLine = "200 OK";

				UpdateClip();

				PhoneInterface::UpdateRing(0);
				tmrAutoAnswer->Enabled = false;
				if (appSettings.uaConf.recording.enabled && call.recording == false &&
					((appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallEarly) || (appSettings.uaConf.recording.recStart == UaConf::RecordingCfg::RecStartCallConfirmed))
					)
				{
					StartRecording();
				}
				if (call.incoming == true && appSettings.HttpQuery.openMode == Settings::_HttpQuery::openAutomaticOnIncomingAnswer)
				{
					HttpQuery();
				}
				break;
			case Callback::CALL_STATE_TRANSFER:
				call.uri = cb.caller;
				call.peerName = GetPeerName(cb.callerName);
				break;
			case Callback::CALL_STATE_TRANSFER_OOD:
				if (appSettings.uaConf.handleOodRefer)
				{
					if (call.uri == "" && call.incoming == false && !pagingTx.active)
					{
						int TODO__EXTRACT_NUMBER_FROM_URI_IF_SERVER_IS_MATCHING;	// otherwise full URI goes into history
						AnsiString target = cb.caller;
						if (target != "")
						{
							MakeCall(target);
						}
					}
				}
				else
				{
					LOG("Incoming REFER: ignoring out-of-dialog transfer, see configuration\n");
				}
				break;
			case Callback::CALL_STATE_CLOSED: {
				//LOG("Callback::CALL_STATE_CLOSED\n");
				if (cb.scode != 0)
				{
					call.lastScode = cb.scode;
					call.lastReplyLine = cb.caller;
				}
				previousCall = call;
				if (call.ringStarted) {
					UA->PlayStop();
					call.ringStarted = false;
				}
				if (cb.scode != 0 || cb.caller != "")
				{
					asStateText = cb.caller;	// here it's filled with e.g. "488 Not Acceptable Here" or local error string
					tmrClearCallState->Enabled = true;
				}
				else
				{
					asStateText = "";
				}
				History::Entry entry;
				DecodeDateTime(call.timestamp,
					entry.timestamp.year, entry.timestamp.month, entry.timestamp.day,
					entry.timestamp.hour, entry.timestamp.min, entry.timestamp.sec,
					entry.timestamp.msec);
				entry.uri = call.uri.c_str();
				entry.peerName = call.peerName.c_str();
				entry.paiUri = call.paiPeerUri.c_str();
				entry.paiPeerName = call.paiPeerName.c_str();
				entry.incoming = call.incoming;
				entry.codecName = call.codecName;
				if (call.connected)
				{
					entry.time = SecondsBetween(Now(), call.timeTalkStart) + 1;
				}
				else
				{
					entry.time = 0;
					if (entry.incoming && !call.disconnecting)
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
				entry.lastScode = call.lastScode;
				entry.lastReplyLine = call.lastReplyLine;
				entry.recordFile = call.recordFile;

				history.AddEntry(entry);
				UpdateCallHistory();

				AnsiString recordFile = call.recordFile;
				call.reset();
				call.recordFile = recordFile;	// should be preserved after the call to be used in script

				buttons.UpdateBtnState(Button::HOLD, false);
				buttons.UpdateBtnState(Button::MUTE, false);
				tmrAutoAnswer->Enabled = false;
				frmTrayNotifier->HideWindow();
				lbl2ndParty->Caption = "";
				lbl2ndPartyDesc->Caption = "";
				PhoneInterface::UpdateCallState(0, "");
				PhoneInterface::UpdateRing(0);
				break;
			}
			default:
				LOG("Unhandled call state = %d\n", static_cast<int>(cb.state));
				break;
			}

			call.state = cb.state;

			lblCallState->Caption = asStateText;

			if (cb.state == Callback::CALL_STATE_INCOMING)
			{
				if (appSettings.frmTrayNotifier.showOnIncoming)
				{
					if (
						(Visible == false || appSettings.frmTrayNotifier.skipIfMainWindowVisible == false) &&
						((answered && appSettings.frmTrayNotifier.hideWhenAnsweringCallAutomatically) == false)
						)
					{
						ShowTrayNotifier(lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, call.incoming);
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
				RunScriptFile(SCRIPT_SRC_ON_CALL_STATE, -1, asScriptFile.c_str(), handled);
			}

			break;
		}
		case Callback::CALL_REINVITE_RECEIVED:
		{
			call.uri = cb.caller;
			call.peerName = GetPeerName(cb.callerName);
			call.paiPeerUri = cb.paiPeerUri;
			call.paiPeerName = GetPeerName(cb.paiPeerName);

			UpdateClip();

			frmTrayNotifier->SetData(lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, false);

			break;
		}
		case Callback::CALL_DTMF_STATE:
		{
			if (cb.dtmfActive == true)
			{
				call.dtmfRxQueue.push_back(cb.dtmf[1]);
			}
			break;
		}
		case Callback::SET_CALL_DATA:
		{
			call.initialRxInvite = cb.initialRxInvite;
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
			recorder.id = cb.recorderId;
			recorder.state = cb.rec_state;

			if (appSettings.Scripts.onRecorderState != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onRecorderState.c_str());
				RunScriptFile(SCRIPT_SRC_ON_RECORDER_STATE, recorder.id, asScriptFile.c_str(), handled);
			}
			break;
		}
		case Callback::ENCRYPTION_STATE:
		{
			call.zrtp.sessionId = cb.zrtp.sessionId;
			call.zrtp.active = cb.zrtp.active;
			call.zrtp.sas = cb.zrtp.sas;
			call.zrtp.cipher = cb.zrtp.cipher;
			call.zrtp.verified = cb.zrtp.verified;
			if (appSettings.Scripts.onEncryptionState != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onEncryptionState.c_str());
				RunScriptFile(SCRIPT_SRC_ON_ENCRYPTION_STATE, cb.zrtp.sessionId, asScriptFile.c_str(), handled);
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
				break;
			case Callback::APP_START_FAILED:
				SetStatus("Failed to start application");
				SetErrorIcon(true);
				HandleCommandLine();
				break;
			case Callback::APP_STARTED:
				UaCustomRequests::Clear();
				SetErrorIcon(false);
				text = "Initialized";
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
				// update software volume
				{
					TTrackBar* tr = trbarSoftvolMic;
					UA->UpdateSoftvolTx(tr->Max - tr->Position + tr->Min);
				}
				{
					TTrackBar* tr = trbarSoftvolSpeaker;
					UA->UpdateSoftvolRx(tr->Max - tr->Position + tr->Min);
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
			if (call.incoming)
			{
				if (appSettings.uaConf.answerOnEventTalk)
				{
					Answer();
				}
				else
				{
					LOG("Ignoring \"Event: talk\" (enable in configuration)\n");
				}
			}
			break;
		}
		case Callback::AUDIO_ERROR:
		{
			audioErrorCount++;
			if (appSettings.Scripts.onAudioDeviceError != "")
			{
				AnsiString asScriptFile;
				bool handled = true;
				asScriptFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), appSettings.Scripts.onAudioDeviceError.c_str());
				RunScriptFile(SCRIPT_SRC_ON_AUDIO_ERROR, -1, asScriptFile.c_str(), handled);
			}
			PhoneInterface::UpdateAudioError();
			if (appSettings.Calls.bDisconnectCallOnAudioError)
			{
				LOG("Disconnecting call on audio error\n");
				Hangup();
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
	if (call.connected || call.progress)
	{
		UA->SendDigit(0, digit);
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
				RunScriptFile(SCRIPT_SRC_ON_DIAL, digit, asScriptFile.c_str(), handled);
			}
		}
	}
}

void TfrmMain::StartRecording(void)
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
				uri = ExtractNumberFromUri(GetCallPeerUri().c_str()).c_str();
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
			UA->Record(file, appSettings.uaConf.recording.channels, appSettings.uaConf.recording.side,
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

void __fastcall TfrmMain::btnAutoAnswerClick(TObject *Sender)
{
	//appSettings.uaConf.autoAnswer = btnAutoAnswer->Down;
}
//---------------------------------------------------------------------------


void TfrmMain::Redial(void)
{
	if (call.uri == "")
	{
		const std::deque<History::Entry>& entries = history.GetEntries();
		for (unsigned int i=0; i<entries.size(); i++)
		{
			const History::Entry &entry = entries[i];
			if (entry.incoming == false)
			{
				MakeCall(entry.uri.c_str());
				break;
			}
		}
	}
}

void TfrmMain::HttpQuery(void)
{
	AnsiString target;
	if (call.uri != "" && call.incoming)
	{
		target = ExtractNumberFromUri(call.uri);
		if (target == "")
		{
			target = CleanUri(call.uri);
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
}

void TfrmMain::AccessCallUrl(void)
{
	if (call.accessUrl == "")
		return;
	ShellExecute(NULL, "open", call.accessUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
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
	AnsiString target;
	if (call.uri != "")
	{
		target = call.uri;
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

void TfrmMain::OnCall(AnsiString uri)
{
	if (call.uri != "")
	{
    	return;
	}
	MakeCall(uri);
}

void TfrmMain::OnCall2(const char* uri)
{
    OnCall(uri);
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
	case Button::MWI:
		OnCall(cfg.number.c_str());
		break;
	case Button::BLF: {
	case Button::PRESENCE:
	case Button::SPEED_DIAL:
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
		if (call.connected)
		{
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
				UA->Transfer(0, dial.c_str()); 			
				break;
			default:
				assert(0);
				break;
			}
		}
		else
		{
			OnCall(dial.c_str());
		}
		break;
	}
	case Button::REDIAL:
		Redial();
		break;
	case Button::DTMF:
		DialString(cfg.number, true);
		break;
	case Button::TRANSFER:
		if (edTransfer->Text == asTransferHint || edTransfer->Text == "")
			return;
		UA->Transfer(0, edTransfer->Text);	
		break;
	case Button::HOLD:
		if (call.connected == false && call.progress == false)
			down = false;
		buttons.UpdateBtnState(cfg.type, down);
		UA->Hold(0, down);		
		break;
	case Button::REREGISTER:
		UA->ReRegister(0);	
		break;
	case Button::UNREGISTER:
		UA->UnRegister(0);
		break;
	case Button::MUTE:
		if (call.connected == false && call.progress == false)
			down = false;
		buttons.UpdateBtnState(cfg.type, down);
		UA->Mute(0, down);
		break;
	case Button::MUTE_RING:
		buttons.UpdateBtnState(cfg.type, down);
		muteRing = down;
		if (muteRing)
		{
			if (call.incoming)
			{
				UA->PlayStop();
				PhoneInterface::UpdateRing(0);
				call.ringStarted = false;
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
			AnsiString num = GetCallPeerUri();
			if (num != "")
			{
            	OnPhonebookEdit(num);
			}
		}
		break;
	case Button::CONTACT_FILE:
		if (lastContactEntry)
		{
			OpenContactFile(lastContactEntry);
		}
		break;
	case Button::HTTP_QUERY:
		HttpQuery();
		break;
	case Button::EXECUTE:
		ExecuteApp(cfg.number.c_str(), cfg.arg1.c_str());
		break;
	case Button::PAGING_TX:
		if (call.uri != "" || pagingTx.active)
		{
			LOG("Could not start streaming - call or streaming already running\n");
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
	case Button::SIP_ACCESS_URL:
		AccessCallUrl();
		break;
	case Button::SWITCH_AUDIO_SOURCE:
		UA->SwitchAudioSource(0, cfg.audioRxMod.c_str(), cfg.audioRxDev.c_str());
		break;
	case Button::SWITCH_AUDIO_PLAYER:
		UA->SwitchAudioPlayer(0, cfg.audioTxMod.c_str(), cfg.audioTxDev.c_str());
		break;
	case Button::HANGUP:
		Hangup(cfg.sipCode, cfg.sipReason.c_str());
		break;
	case Button::SEND_TEXT_MESSAGE: {
		AnsiString target = cfg.number.c_str();
		if (target == "")
		{
			target = cbCallURI->Text;
		}
		SIMPLE_Messages::Send(target, "", false);
		break;
	}
	case Button::RECORD:
		StartRecording();
		break;
	case Button::RECORD_PAUSE:
		if (call.recording)
		{
            LOG("Pause recording\n");
			UA->RecordPause();
		}
		else
		{
			LOG("RECORD_PAUSE: not recording\n");
		}
		break;
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
		&OnAddOutputText, &OnCall2, &Hangup, &Answer, &OnGetDial, &OnSetDial,
		&OnSwitchAudioSource, &DialString, &OnBlindTransfer,
		&OnGetCall,
		&OnResetCall,
		&OnGetPreviousCall,
		&OnGetRecorder,
		&OnGetContactName,
		&OnGetStreamingState,
		&OnGetAudioErrorCount,
		&OnSetTrayIcon,
		&OnGetRegistrationState,
		&OnPluginSendMessageText, &OnPluginEnable,
		&OnGetContactId,
		&OnGetBlfState,
		&OnRecordStart,
		&OnGetRxDtmf,
		&ShowTrayNotifier,
		&HideTrayNotifier,
		&OnGetUserName,
		&ProgrammableButtonClick,
		&UpdateSettingsFromJson,
		&UpdateButtonsFromJson,
		&OnGetButtonConf,
		&MainMenuShow,
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
	if (Key == VK_RETURN)
	{
		if (call.uri == "" && call.incoming == false && !pagingTx.active)
		{
			AnsiString target = cbCallURI->Text.Trim();
			if (target != "")
			{
				MakeCall(target);
			}
		}
		if (appSettings.frmMain.bNoBeepOnEnterKey)
		{
			// this was unintentional, but it looks like Windows beep is meaning that [Enter] is not accepted by the control
			Key = NULL;
		}
	}
	const char* dtmfKeys = "01234567890*#ABCDabcd";
	if (strchr(dtmfKeys, Key))
	{
		if (call.connected || call.progress)
		{
			UA->SendDigit(0, toupper(Key));
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
	if (cbCallURI->Text == "")
	{
		PhoneInterface::UpdateCallState(0, cbCallURI->Text.c_str());
	}
	else
	{
		PhoneInterface::UpdateCallState(1, cbCallURI->Text.c_str());
	}
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

void TfrmMain::AutoAnswer(void)
{
	if (autoAnswerCode == 200) {
		if (autoAnswerIntercom) {
			LOG("Answering with module %s, device %s\n", appSettings.uaConf.audioCfgPlayIntercom.mod.c_str(), appSettings.uaConf.audioCfgPlayIntercom.dev.c_str());
			UA->Answer(0, appSettings.uaConf.audioCfgPlayIntercom.mod.c_str(), appSettings.uaConf.audioCfgPlayIntercom.dev.c_str(), appSettings.video.enabled, GetDisplayParentHandle());
		} else {
			Answer();
		}
		if (appSettings.frmTrayNotifier.hideWhenAnsweringCallAutomatically)
		{
			frmTrayNotifier->HideWindow();
		}
	} else if (autoAnswerCode >= 400) {
		UA->Hangup(0, autoAnswerCode, appSettings.uaConf.autoAnswerReason.c_str());
		lbl2ndParty->Caption = "";
		lbl2ndPartyDesc->Caption = "";
		lblCallState->Caption = "";
		call.incoming = false;
		call.progress = false;
		call.connected = false;
		if (appSettings.frmTrayNotifier.hideWhenAnsweringCallAutomatically)
		{
			frmTrayNotifier->HideWindow();
		}
	}
}

void __fastcall TfrmMain::tmrAutoAnswerTimer(TObject *Sender)
{
	tmrAutoAnswer->Enabled = false;
	AutoAnswer();
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
		UA->Transfer(0, edTransfer->Text);
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
		Hangup();
	}
	else if (cmd.action == CommandLine::ACTION_ANSWER)
	{
		LOG("action = ANSWER\n");
		Answer();
	}
	else if (cmd.action == CommandLine::ACTION_CALL)
	{
		AnsiString target = cmd.asTarget.Trim();
		LOG("action = CALL, target = %s\n", target.c_str());
		if (target != "" && !pagingTx.active && call.uri == "")
		{
			MakeCall(target);
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
		StartRecording();
	}
	else if (cmd.action == CommandLine::ACTION_RECORD_PAUSE)
	{
		if (call.recording)
		{
			LOG("Pause recording\n");
			UA->RecordPause();
		}
		else
		{
			LOG("RECORD_PAUSE: not recording\n");
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

void TfrmMain::StartRing(AnsiString wavFile)
{
	UA->StartRing(wavFile);
	PhoneInterface::UpdateRing(1);
	call.ringStarted = true;	
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
	case Action::TYPE_ANSWER:
		Answer();
		break;
	case Action::TYPE_HANGUP:
		Hangup();
		break;
	case Action::TYPE_REDIAL:
		Redial();
		break;
	case Action::TYPE_CALL_CLIPBOARD:
		if (call.uri == "" && !pagingTx.active)	///< \todo better way to check if there is no current call
		{
			AnsiString target = CleanNumber(Clipboard()->AsText);
			if (target != "")
			{
				MakeCall(target);
			}
		}
		break;
	case Action::TYPE_SHOWHIDE_SIDECAR:
		ToggleSpeedDial();
		break;
	case Action::TYPE_ANSWER_HANGUP:
		if (call.state != Callback::CALL_STATE_CLOSED)
		{
			if (call.state == Callback::CALL_STATE_INCOMING)
				Answer();
			else
				Hangup();
		}
		break;
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
			Hangup();
		}
		else
		{
			Answer();
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
		Dial('0' + keyCode);
		break;
	case KEY_STAR:
		Dial('*');
		break;
	case KEY_HASH:
		Dial('#');
		break;
	case KEY_OK:
		if (call.uri == "" && call.incoming == false && !pagingTx.active)
		{
			AnsiString target = cbCallURI->Text.Trim();
			if (target != "")
			{
				MakeCall(target);
			}
		}
		break;
	case KEY_C:
		if (call.incoming && !call.connected)
		{
			Hangup();
		}
		else
		{
			CallNumberBackspace();
		}
		break;
	default:
		break;
	}
}

int TfrmMain::OnPhonePagingTx(const char* target, const char* filename, const char* codecname)
{
	if (call.uri != "" || pagingTx.active)
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

int TfrmMain::OnSetVariable(const char* name, const char* value)
{
	return ScriptExec::SetVariable(name, value);
}

int TfrmMain::OnClearVariable(const char* name)
{
	return ScriptExec::ClearVariable(name);
}

void TfrmMain::OnQueuePush(const char* name, const char* value)
{
	return ScriptExec::QueuePush(name, value);
}

int TfrmMain::OnQueuePop(const char* name, AnsiString &value)
{
	return ScriptExec::QueuePop(name, value);
}

int TfrmMain::OnQueueClear(const char* name)
{
	return ScriptExec::QueueClear(name);
}

int TfrmMain::OnQueueGetSize(const char* name)
{
    return ScriptExec::QueueGetSize(name);
}

void TfrmMain::OnAddOutputText(const char* text)
{
	LOG("%s", text);
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
	int value = tr->Max - tr->Position + tr->Min;
	asHint.sprintf("%d%%", value*100/128);
	tr->Hint = asHint;
	UA->UpdateSoftvolTx(value);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trbarSoftvolSpeakerChange(TObject *Sender)
{
	TTrackBar* tr = trbarSoftvolSpeaker;
	AnsiString asHint;
	int value = tr->Max - tr->Position + tr->Min;
	asHint.sprintf("%d%%", value*100/128);
	tr->Hint = asHint;
	UA->UpdateSoftvolRx(value);
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
	frmSettingsPatch->Caption = "Patch/update main settings";
	frmSettingsPatch->onUpdateSettings = UpdateSettingsFromJson;
	frmSettingsPatch->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::miPatchButtonSettingsClick(TObject *Sender)
{
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
//---------------------------------------------------------------------------


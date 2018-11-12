//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormMain.h"
#include "CommandLine.h"
#include "TrayIcon.h"
#include "FormAbout.h"
#include "FormSettings.h"
#include "FormAccount.h"
#include "FormHistory.h"
#include "FormButtonContainer.h"
#include "History.h"
#include "FormContacts.h"
#include "Contacts.h"
#include "FormContactPopup.h"
#include "FormContactEditor.h"
#include "FormTrayNotifier.h"
#include "LogUnit.h"
#include "Log.h"
#include "UaMain.h"
#include "ControlQueue.h"
#include "Callback.h"
#include "CallbackQueue.h"
#include <assert.h>
#include <stdio.h>
#include "CustomDateUtils.hpp"
#include "ProgrammableButton.h"
#include "ProgrammableButtons.h"
#include "ScriptExec.h"
#include "HotKeys.h"
#include "Utils.h"
#include "Unicode.h"
#include "OS.h"
#include "Branding.h"
#include "base64.h"
#include "PhoneInterface.h"
#include "FormContactsCsvImport.h"
#include "Utilities.h"
#include <Clipbrd.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

TfrmMain *frmMain;

namespace {
	History history;
	Contacts contacts;
	HotKeys hotKeys;
	ProgrammableButtons buttons;
	AnsiString asTransferHint = "Transfer to ... [Enter]";
	bool useOwnTrayIcon = false;

	Contacts::Entry *lastContactEntry = NULL;
	void ShowContactPopup(Contacts::Entry *entry)
	{
		if (frmContactPopup->isNoteModified())
		{
			/** \todo Limit write frequency
			*/
			contacts.Write();
		}
		frmContactPopup->SetData(entry);
		frmContactPopup->Visible = true;	// steals focus
		//ShowWindow(frmContactPopup->Handle,SW_SHOWNOACTIVATE);
		//SetWindowPos(frmContactPopup->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	AnsiString GetFullImgName(AnsiString name)
	{
		AnsiString fname;
		fname.sprintf("%s\\img\\%s", ExtractFileDir(Application->ExeName).c_str(), name.c_str());
		return fname;
	}
	AnsiString GetPeerName(AnsiString displayName)
	{
		if (appSettings.Display.bDecodeUtfDisplayToAnsi)
			return ::Utf8ToAnsi(displayName);
		else
			return displayName;
	}
}

//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
	: TForm(Owner),
	muteRing(false),
	notificationIconState(false)
{
	srand(time(0));
	lbl2ndParty->Caption = "";
	lbl2ndPartyDesc->Caption = "";
	lblCallState->Caption = "";
	frmHistory = new TfrmHistory(this->tsHistory, &history, &OnCall, &OnPhonebookEdit, &OnHttpQuery);
	frmHistory->Scale(appSettings.gui.scalingPct);
	frmHistory->Parent = tsHistory;
	frmHistory->Visible = true;

	AnsiString asButtonsFile;
	asButtonsFile.sprintf("%s\\%s_buttons.json", ExtractFileDir(Application->ExeName).c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	buttons.SetFilename(asButtonsFile);
	buttons.Read();
	buttons.UpdateContacts(appSettings.uaConf.contacts);

	TfrmButtonContainer *frmButtonContainerBasic = new TfrmButtonContainer(
		this->pnlButtonsBasic,
		buttons,
		this->pnlButtonsBasic->Width, this->pnlButtonsBasic->Height, appSettings.gui.scalingPct,
		0, 8,
		&OnProgrammableBtnClick);
	frmButtonContainerBasic->Parent = this->pnlButtonsBasic;
	frmButtonContainerBasic->Visible = true;
	frmButtonContainers[0] = frmButtonContainerBasic;

	for (int i=1; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container = new TfrmButtonContainer(this->pnlSpeedDial,
			buttons,
			appSettings.frmMain.iSpeedDialWidth, 0, appSettings.gui.scalingPct,
			i * ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN, ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN,
			&OnProgrammableBtnClick);
		container->Parent = this->pnlSpeedDial;
		container->Align = alLeft;
		container->Visible = true;
		//container->SetScaling(appSettings.gui.scalingPct);
	}

	frmContacts = new TfrmContacts(this->tsContacts, &contacts, &OnCall);
	frmContacts->Scale(appSettings.gui.scalingPct);
	frmContacts->Parent = tsContacts;
	frmContacts->Visible = true;
	frmContacts->FilterUsingNote(appSettings.Contacts.filterUsingNote);
	edTransfer->Text = asTransferHint;

	trIcon = new TrayIcon(this);
	trIcon->OnLeftBtnDown = OnTrayIconLeftBtnDown;
	trIcon->OnRightBtnDown = OnTrayIconRightBtnDown;
	trIcon->SetPopupMenu(popupTray);
	trIcon->SetIcon(Application->Icon);
	trIcon->ShowInTray(true);

	if (appSettings.frmMain.trayNotificationImage != "")
	{
		Graphics::TBitmap *bmp = new Graphics::TBitmap();
		try
		{
			bmp->LoadFromFile(GetFullImgName(appSettings.frmMain.trayNotificationImage));
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
}

__fastcall TfrmMain::~TfrmMain()
{
	delete trIcon;
	trIcon = NULL;
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::FormCreate(TObject *Sender)
{
	this->Caption = Branding::appName;
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

	AnsiString asContactsFile;
	asContactsFile.sprintf("%s\\%s_contacts.json", ExtractFileDir(Application->ExeName).c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	contacts.SetFilename(asContactsFile);
	contacts.Read();

	contacts.addObserver(*this);

	/** \note must be initialized AFTER contacts */
	AnsiString asHistoryFile;
	asHistoryFile.sprintf("%s\\%s_history.json", ExtractFileDir(Application->ExeName).c_str(),
		ChangeFileExt(ExtractFileName(Application->ExeName), "").c_str());
	history.SetFilename(asHistoryFile);
	history.Read(&OnGetContactName);

#if 0
	if (this->BorderStyle != bsSingle)
	{
		this->Width = appSettings.frmMain.iWidth;
		this->Height = appSettings.frmMain.iHeight;
	}
#endif
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
	SetSpeedDial(false);
	if (appSettings.frmMain.bSpeedDialOnly)
	{
		SetSpeedDial(true);
	}
	else if (appSettings.frmMain.bSpeedDialVisible)
	{
		SetSpeedDial(appSettings.frmMain.bSpeedDialVisible);
	}
	UpdateDialpadBackgroundImage();
	miSettings->Visible = !appSettings.frmMain.bHideSettings;
	miView->Visible = !appSettings.frmMain.bHideView;
	miHelp->Visible = !appSettings.frmMain.bHideHelp;

	trbarSoftvolMic->Position = trbarSoftvolMic->Max + trbarSoftvolMic->Min - appSettings.uaConf.audioSoftVol.tx;
	trbarSoftvolSpeaker->Position = trbarSoftvolSpeaker->Max + trbarSoftvolSpeaker->Min - appSettings.uaConf.audioSoftVol.rx;

	UpdateCallHistory();
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

	hotKeys.Unregister(Handle);

	PhoneInterface::Close();

	AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
	// inverting trackbars
	appSettings.uaConf.audioSoftVol.tx = trbarSoftvolMic->Max - trbarSoftvolMic->Position + trbarSoftvolMic->Min;
	appSettings.uaConf.audioSoftVol.rx = trbarSoftvolSpeaker->Max - trbarSoftvolSpeaker->Position + trbarSoftvolSpeaker->Min;
	appSettings.frmMain.bWindowMaximized = (this->WindowState == wsMaximized);
	if (!appSettings.frmMain.bWindowMaximized)
	{
		// these values are meaningless is wnd is maximized
		appSettings.frmMain.iWidth = this->Width;
		appSettings.frmMain.iHeight = this->Height;
		appSettings.frmMain.iPosY = this->Top;
		appSettings.frmMain.iPosX = this->Left;
	}
	appSettings.frmTrayNotifier.iPosX = frmTrayNotifier->Left;
	appSettings.frmTrayNotifier.iPosY = frmTrayNotifier->Top;

	appSettings.frmContactPopup.iPosX = frmContactPopup->Left;
	appSettings.frmContactPopup.iPosY = frmContactPopup->Top;
	appSettings.frmContactPopup.iWidth = frmContactPopup->Width;
	appSettings.frmContactPopup.iHeight = frmContactPopup->Height;

	// update application version in settings
	GetFileVer(Application->ExeName, appSettings.info.appVersion.FileVersionMS, appSettings.info.appVersion.FileVersionLS);
		
	appSettings.Write(asConfigFile);
	if (appSettings.History.bNoStoreToFile == false)
	{
		history.Write();
    }

	frmContactPopup->Close();
	if (frmContactPopup->isNoteModified())
	{
		contacts.Write();
	}

	// no need to write buttons configuration here
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::actShowAboutExecute(TObject *Sender)
{
	frmAbout->ShowModal();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actShowSettingsExecute(TObject *Sender)
{
	UaConf tmp = appSettings.uaConf;
	int iSpeedDialSize = appSettings.frmMain.iSpeedDialSize;
	bool bSpeedDialOnly = appSettings.frmMain.bSpeedDialOnly;
	bool bKioskMode = appSettings.frmMain.bKioskMode;
	int prevTrayNotifierScaling = appSettings.frmTrayNotifier.scalingPct;
	std::list<HotKeyConf> hkConf = appSettings.hotKeyConf;	
	frmSettings->ShowModal();
	if (appSettings.uaConf != tmp)
	{
		SetStatus("Restarting UA...");
		miSettings->Enabled = false;		
		Ua::Instance().Restart();
	}
	if (appSettings.uaConf.logMessages != tmp.logMessages)
	{
		UA->SetMsgLogging(appSettings.uaConf.logMessages);
	}
	if (appSettings.frmMain.bKioskMode != bKioskMode)
	{
		SetKioskMode(appSettings.frmMain.bKioskMode);
	}
	if (appSettings.frmMain.bHideMouseCursor)
	{
		Screen->Cursor = crNone;
	}
	else
	{
    	Screen->Cursor = crDefault;
    }
	if ((appSettings.frmMain.iSpeedDialSize != iSpeedDialSize && (appSettings.frmMain.bSpeedDialVisible || appSettings.frmMain.bSpeedDialOnly)) ||
		bSpeedDialOnly != appSettings.frmMain.bSpeedDialOnly
		)
	{
		// apply speed dial changes
		SetSpeedDial(appSettings.frmMain.bSpeedDialVisible || appSettings.frmMain.bSpeedDialOnly);		
	}
	// enable/disable popup menu
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container->UpdateSettings();
	}
	if (appSettings.frmMain.bAlwaysOnTop)
		this->FormStyle = fsStayOnTop;
	else
		this->FormStyle = fsNormal;
	UpdateLogConfig();
	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);
	if (frmLog->Visible)
	{
    	frmLog->UpdateUi();
	}
	if (hkConf != appSettings.hotKeyConf)
	{
		RegisterGlobalHotKeys();
	}
	frmContacts->FilterUsingNote(appSettings.Contacts.filterUsingNote);
	UpdateDialpadBackgroundImage();
	frmTrayNotifier->UpdateBackgroundImage();
	frmTrayNotifier->ScaleBy(100, prevTrayNotifierScaling);
	frmTrayNotifier->ScaleBy(appSettings.frmTrayNotifier.scalingPct, 100);
	miSettings->Visible = !appSettings.frmMain.bHideSettings;
	miView->Visible = !appSettings.frmMain.bHideView;
	miHelp->Visible = !appSettings.frmMain.bHideHelp;

	PhoneInterface::SetCfg(appSettings.phoneConf);

	tmrScript->Interval = appSettings.Scripts.timer;
	tmrScript->Enabled = true;

	AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
	// update application version in settings
	GetFileVer(Application->ExeName, appSettings.info.appVersion.FileVersionMS, appSettings.info.appVersion.FileVersionLS);
	appSettings.Write(asConfigFile);
}
//---------------------------------------------------------------------------


void __fastcall TfrmMain::FormDestroy(TObject *Sender)
{
	UA->Destroy();
	UA_CB->Destroy();
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

	frmLog->SetLogLinesLimit(appSettings.Logging.iMaxUiLogLines);

	if (appSettings.frmMain.bKioskMode)
	{
		SetKioskMode(appSettings.frmMain.bKioskMode);
    }

    frmTrayNotifier->Caption = Branding::appName;
	frmTrayNotifier->OnHangup = Hangup;
	frmTrayNotifier->OnAnswer = Answer;
	frmTrayNotifier->UpdateBackgroundImage();
	frmTrayNotifier->ScaleBy(appSettings.frmTrayNotifier.scalingPct, 100);

	Ua::Instance().Start();
	SetStatus("Initializing...");

	RegisterGlobalHotKeys();

	if (appSettings.frmMain.bSpeedDialOnly == false && Visible)
	{
		cbCallURI->SetFocus();
	}

	AnsiString dir = ExtractFileDir(Application->ExeName);
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
	PhoneInterface::EnumerateDlls(dir + "\\phone");
	PhoneInterface::SetCfg(appSettings.phoneConf);
	PhoneInterface::UpdateRegistrationState(0);
	PhoneInterface::UpdateCallState(0, "");
	PhoneInterface::UpdatePagingTxState(0);

	if (appSettings.Scripts.onStartup != "")
	{
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onStartup.c_str());
		RunScript(ScriptExec::SRC_TYPE_ON_STARTUP, -1, asScriptFile.c_str());
	}

	tmrScript->Interval = appSettings.Scripts.timer;
	tmrScript->Enabled = true;
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
	for (unsigned int i=0; i<std::min(100u, entries.size()); i++)
	{
		const History::Entry &entry = entries[i];
		// add only outgoing calls
		if (entry.incoming == false)
		{
			numbers.insert(entry.uri.c_str());
		}
	}
	std::set<std::string>::iterator iter;
	for (iter = numbers.begin(); iter != numbers.end(); ++iter)
	{
		cbCallURI->Items->Add(iter->c_str());
	}
}

void TfrmMain::ShowTrayNotifier(AnsiString description, AnsiString uri, bool incoming)
{
	frmTrayNotifier->SetData(lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, call.incoming);
#if 1
	/** \todo frmTrayNotifier steals focus at first call */
	frmTrayNotifier->Visible = true;
#else
	/** \todo frmTrayNotifier: no focus stealed, but inconsistent window state */
	ShowWindow(frmTrayNotifier->Handle, SW_SHOWNOACTIVATE);
	SetWindowPos(frmTrayNotifier->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//frmTrayNotifier->BringToFront();
#endif
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
	call.timestamp = Now();
	call.incoming = false;
	call.progress = false;
	call.uri = target;
	call.initialTarget = call.uri;
	call.peerName = "";
	call.recordFile = "";
	call.dtmfRxQueue.clear();

	if (appSettings.Scripts.onMakeCall != "")
	{
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onMakeCall.c_str());
        // this script may change initial target implementing SIP originate function
		RunScript(ScriptExec::SRC_TYPE_ON_MAKING_CALL, -1, asScriptFile.c_str());
	}

	UA->Call(0, call.initialTarget, appSettings.Calls.extraHeaderLines);
}

void __fastcall TfrmMain::btnHangupClick(TObject *Sender)
{
	Hangup();
}
//---------------------------------------------------------------------------

void TfrmMain::Hangup(void)
{
	call.disconnecting = true;
	UA->Hangup(0);
}

void TfrmMain::Answer(void)
{
	if (call.incoming)
	{
		UA->Answer(0);
		if (appSettings.frmMain.bShowWhenAnsweringCall)
		{
			if (!Visible)
			{
                ToggleVisibility();
            }
		}
		if (appSettings.frmTrayNotifier.hideWhenAnsweringCall)
		{
			frmTrayNotifier->Close();
        }
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

int TfrmMain::OnGetCallState(void)
{
	//STATIC_CHECK(Callback::CALL_STATE_CLOSED == 0, UpdateCallStateInitializer);
	return call.state;
}

int TfrmMain::OnIsCallIncoming(void)
{
    return call.incoming;
}

std::string TfrmMain::OnGetCallPeer(void)
{
	AnsiString uri;
	if (call.incoming)
	{
		uri = ExtractNumberFromUri(call.uri.c_str()).c_str();
	}
	else
	{
		uri = call.uri;
	}
	return uri.c_str();
}

std::string TfrmMain::OnGetCallInitialRxInvite(void)
{
	return call.initialRxInvite.c_str();
}

std::string TfrmMain::OnGetRecordFile(void)
{
	return call.recordFile.c_str();
}

int TfrmMain::OnGetBlfState(int contactId, std::string &number)
{
	if (contactId < 0 || contactId >= appSettings.uaConf.contacts.size())
	{
        LOG("OnGetBlfState: invalid contactId = %d\n", contactId);
		return -1;
    }
	const UaConf::Contact &contact = appSettings.uaConf.contacts[contactId];
	number = contact.user;
	return contact.dialog_info_state;
}

int TfrmMain::OnGetStreamingState(void)
{
	return pagingTx.state;
}

int TfrmMain::OnGetRegistrationState(void)
{
	return registration.state;
}

std::string TfrmMain::OnGetInitialCallTarget(void)
{
	return call.initialTarget.c_str();
}

void TfrmMain::OnSetTrayIcon(const char* file)
{
	if (file != NULL && file[0] != '\0')
	{
		useOwnTrayIcon = true;
		Graphics::TBitmap *bmp = new Graphics::TBitmap();
		try
		{
			bmp->LoadFromFile(GetFullImgName(file));
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

void TfrmMain::OnSetInitialCallTarget(std::string number)
{
    call.initialTarget = number.c_str();
}

void TfrmMain::OnSetButtonCaption(int id, std::string text)
{
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container->UpdateBtnCaption(id, text.c_str());
	}
}

void TfrmMain::OnSetButtonDown(int id, bool state)
{
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container->UpdateBtnDown(id, state);
	}
}

void TfrmMain::OnSetButtonImage(int id, const char* file)
{
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container->UpdateBtnImage(id, file);
	}
}

int TfrmMain::OnPluginSendMessageText(const char* dllName, const char* text)
{
	return PhoneInterface::SendMessageText(dllName, text);
}

int TfrmMain::OnRecordStart(const char* file, int channels)
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
	UA->Record(file, channels);
	call.recordFile = file;
	call.recording = true;
	return 0;
}

int TfrmMain::OnGetRecordingState(void)
{
	return call.recording;
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
				lbl2ndParty->Caption = GetClip(cb.caller);
				lastContactEntry = contacts.GetEntry(CleanUri(cb.caller));
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
				}
				else
				{
					lbl2ndPartyDesc->Caption = call.peerName;
				}
				PhoneInterface::UpdateCallState(1, ExtractNumberFromUri(cb.caller).c_str()); //CleanUri(cb.caller).c_str());
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
				if (call.ringStarted) {
					UA->StopRing();
					call.ringStarted = false;
				}
				if (cb.scode != 0)
				{
					asStateText = cb.caller;	// here it's filled with e.g. "488 Not Acceptable Here"
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
				entry.incoming = call.incoming;
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
				history.AddEntry(entry);
				UpdateCallHistory();
				call.incoming = false;
				call.progress = false;
				call.connected = false;
				call.disconnecting = false;
				call.recording = false;
				call.uri = "";
				call.last_scode = cb.scode;
				UpdateBtnState(Button::HOLD, false);
				UpdateBtnState(Button::MUTE, false);
				tmrAutoAnswer->Enabled = false;				
				frmTrayNotifier->Close();
				lbl2ndParty->Caption = "";
				lbl2ndPartyDesc->Caption = "";
				PhoneInterface::UpdateCallState(0, "");
				PhoneInterface::UpdateRing(0);				
				break;
			}
			default:
				assert(!"Unhandled call state");
				break;
			}

			call.state = cb.state;
			
			lblCallState->Caption = asStateText;

			if (cb.state == Callback::CALL_STATE_INCOMING)
			{
				if (appSettings.frmTrayNotifier.showOnIncoming)
				{
					if (Visible == false || appSettings.frmTrayNotifier.skipIfMainWindowVisible == false)
					{
						ShowTrayNotifier(lbl2ndPartyDesc->Caption, lbl2ndParty->Caption, call.incoming);
					}
				}
				if (appSettings.frmMain.bRestoreOnIncomingCall)
				{
					if (IsIconic(Application->Handle))
						Application->Restore();
                    SetForegroundWindow( Application->Handle );
					//this->BringToFront();
					//SetWindowPos(this->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}

			if (appSettings.Scripts.onCallState != "")
			{
				AnsiString asScriptFile;
				asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onCallState.c_str());
				RunScript(ScriptExec::SRC_TYPE_ON_CALL_STATE, -1, asScriptFile.c_str());
			}

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
				break;
			case Callback::REG_STATE_REGISTER_FAIL:
				asRegText = "Error registering";
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
				static bool once = false;
				if (!once)
				{
					once = true;
					HandleCommandLine();
				}
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
				asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onRegistrationState.c_str());
				RunScript(ScriptExec::SRC_TYPE_ON_REGISTRATION_STATE, -1, asScriptFile.c_str());
			}

			break;
		}
		case Callback::APP_STATE:
		{
			AnsiString text;
			/** \note Settings menu is disabled while initializing app */
			switch(cb.app_state)
			{
			case Callback::APP_INIT_FAILED:
				miSettings->Enabled = true;
				SetStatus("Failed to init application");
				HandleCommandLine();				
				break;
			case Callback::APP_INITIALIZED:
				miSettings->Enabled = true;
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
				}
				if (appSettings.uaConf.accounts[0].reg_expires == 0)
				{
					static bool once = false;
					if (!once)
					{
						once = true;
						HandleCommandLine();
					}
				}
				SetStatus(text);
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
			case Callback::APP_START_FAILED:
				SetStatus("Failed to start application");
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
			if (!appSettings.frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity)
			{
				direction = static_cast<enum dialog_info_direction>(cb.dlgInfoDirection);
				remoteIdentity = cb.dlgInfoRemoteIdentity;
				remoteIdentityDisplay = cb.dlgInfoRemoteIdentityDisplay;
            }
			appSettings.uaConf.contacts[cb.contactId].dialog_info_state = cb.dlgInfoState;
			std::list<int> &ids = appSettings.uaConf.contacts[cb.contactId].btnIds;
			std::list<int>::iterator iter;
			for (iter = ids.begin(); iter != ids.end(); ++iter)
			{
				for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
					TfrmButtonContainer *& container = frmButtonContainers[i];
					container->UpdateDlgInfoState(*iter, cb.dlgInfoState, direction, remoteIdentity, remoteIdentityDisplay);
				}
			}
			if (appSettings.Scripts.onDialogInfo != "")
			{
				AnsiString asScriptFile;
				asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onDialogInfo.c_str());
				RunScript(ScriptExec::SRC_TYPE_ON_DIALOG_INFO, cb.contactId, asScriptFile.c_str());
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
				for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
					TfrmButtonContainer *& container = frmButtonContainers[i];
					container->UpdatePresenceState(*iter, cb.presenceState, note);
				}
			}
			break;
		}
		case Callback::MWI_STATE:
		{
            SetNotificationIcon(cb.mwiNewMsg > 0);
			for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
				TfrmButtonContainer *& container = frmButtonContainers[i];
				container->UpdateMwiState(cb.mwiNewMsg, cb.mwiOldMsg);
			}
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
				asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onStreamingState.c_str());
				RunScript(ScriptExec::SRC_TYPE_ON_STREAMING_STATE, -1, asScriptFile.c_str());
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
		default:
		{
			assert(!"Unhandled callback type");
		}
	}
}
//---------------------------------------------------------------------------

void TfrmMain::UpdateBtnState(Button::Type type, bool state)
{
	for (int i=0; i<ARRAY_SIZE(frmButtonContainers); i++) {
		TfrmButtonContainer *& container = frmButtonContainers[i];
		container->UpdateBtnState(type, state);
	}
}

void __fastcall TfrmMain::btnDialClick(TObject *Sender)
{
	char digit = '\0';
	TSpeedButton* btn = dynamic_cast<TSpeedButton*>(Sender);
	assert(btn);
	switch(btn->Tag)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		digit = '0' + btn->Tag;
		break;
	case 10:
		digit = '*';
		break;
	case 11:
		digit = '#';
		break;
	case 12:
		digit = 'A';
		break;
	case 13:
		digit = 'B';
		break;
	case 14:
		digit = 'C';
		break;
	case 15:
		digit = 'D';
		break;
	case 16:	// FLASH
		digit = 'R';
		break;
	default:
		assert(!"Unhandled button tag value");
	}

	Dial(digit);

	if (appSettings.Scripts.onDial != "")
	{
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onDial.c_str());
		RunScript(ScriptExec::SRC_TYPE_ON_DIAL, digit, asScriptFile.c_str());
	}	
}
//---------------------------------------------------------------------------

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

void TfrmMain::DialString(const std::string& digits)
{
	for (int i=0; i<digits.length(); i++)
	{
		const char* allowed = "0123456789ABCD*#R";
		char digit = toupper(digits[i]);
		if (strchr(allowed, digit))
		{
			Dial(digit);
		}
	}
}

void TfrmMain::StartRecording(void)
{
	try
	{
		AnsiString file;
		AnsiString dir;
		if (appSettings.uaConf.recording.recDir == UaConf::RecordingCfg::RecDirRelative)
		{
			dir = ExtractFileDir(Application->ExeName) + "\\recordings\\";
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
				uri = ExtractNumberFromUri(call.uri.c_str()).c_str();
			}
			else
			{
				uri = call.uri;
			}
			uri = uri.SubString(1, 32);
			std::string b64uri = base64_encode((unsigned char*)uri.c_str(), uri.Length(), BASE64_ALPHABET_FSAFE);
			file += b64uri.c_str();
			file += ".wav";
			LOG("Record file: %s\n", file.c_str());
			UA->Record(file, appSettings.uaConf.recording.channels);
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
        ShowWindow(Application->Handle, SW_HIDE);
        Visible = false;
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
	frmContactEditor->Start(entry);
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
				DialString(dial);
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
		DialString(cfg.number);
		break;
	case Button::TRANSFER:
		if (edTransfer->Text == asTransferHint || edTransfer->Text == "")
			return;
		UA->Transfer(0, edTransfer->Text);	
		break;
	case Button::HOLD:
		if (call.connected == false && call.progress == false)
			down = false;
		UpdateBtnState(cfg.type, down);
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
		UpdateBtnState(cfg.type, down);
		UA->Mute(0, down);
		break;
	case Button::MUTE_RING:
		UpdateBtnState(cfg.type, down);
		muteRing = down;
		if (muteRing)
		{
			if (call.incoming)
			{
				UA->StopRing();
				PhoneInterface::UpdateRing(0);
				call.ringStarted = false;
			}
		}
		break;
	case Button::CONTACT_NOTE:
		if (lastContactEntry)
		{
        	ShowContactPopup(lastContactEntry);
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
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), cfg.script.c_str());
		RunScript(ScriptExec::SRC_TYPE_BUTTON, id, asScriptFile.c_str());
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
		Hangup();
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
	default:
		LOG("Unhandled BTN type = %d\n", static_cast<int>(cfg.type));
		break;
	}
}

void TfrmMain::RunScript(int srcType, int srcId, AnsiString filename, bool showLog)
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
		ScriptExec scriptExec(
			static_cast<enum ScriptExec::SrcType>(srcType), srcId,
			&OnAddOutputText, &OnCall2, &Hangup, &Answer, &OnGetDial, &OnSetDial,
			&OnSwitchAudioSource, &DialString, &OnBlindTransfer, &OnGetCallState,
			&OnIsCallIncoming, &OnGetCallPeer, &OnGetCallInitialRxInvite,
			&OnGetContactName,
			&OnGetStreamingState,
			&OnGetInitialCallTarget, &OnSetInitialCallTarget,
			&OnSetTrayIcon,
			&OnGetRegistrationState,
			&OnSetButtonCaption, &OnSetButtonDown, &OnSetButtonImage,
			&OnPluginSendMessageText,
			&OnGetRecordFile,
			&OnGetBlfState,
			&OnRecordStart,
			&OnGetRecordingState,
			&OnGetRxDtmf,
			&ShowTrayNotifier,
			&OnGetUserName,
			&ProgrammableButtonClick
			);
		scriptExec.Run(scriptText.c_str());
	}
	else
	{
		AnsiString msg;
		msg.sprintf("Script file not found (%s).", filename.c_str());
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
    }	
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
		log->SetFile(ChangeFileExt(Application->ExeName, ".log").c_str());
	else
		log->SetFile("");
	log->SetFlush(appSettings.Logging.bFlush);
	log->SetMaxFileSize(appSettings.Logging.iMaxFileSize);
	log->SetLogRotateCnt(appSettings.Logging.iLogRotate);
}

void TfrmMain::SetSpeedDial(bool visible)
{
	if (visible)
	{
		pnlSpeedDial->Width = (frmButtonContainers[1]->Width) * (appSettings.frmMain.iSpeedDialSize + 1) + 4;
		/*
		This weird column handling order and switching to alNone and back to alLeft
		is intended to keep proper order of components (align with respect to each order).
		With 1..N loop and alLeft always on columns get reordered when switching number of columns at runtime.
		*/
		for (int i=ARRAY_SIZE(frmButtonContainers)-1; i>=1; i--) {
			TfrmButtonContainer *& container = frmButtonContainers[i];
			if (i <= appSettings.frmMain.iSpeedDialSize + 1)
			{
				container->Visible = true;
				container->Align = alNone;
				container->Align = alLeft;
			}
			else
			{
				container->Visible = false;
			}
		}
		if (appSettings.frmMain.bSpeedDialOnly)
		{
			pnlMain->Visible = false;
			if (appSettings.frmMain.bKioskMode == false)
			{
				ClientWidth = pnlSpeedDial->Width + 6;
			}
			pnlSpeedDial->Left = 6;
		}
		else
		{
			pnlMain->Visible = true;
			pnlSpeedDial->Left = 0;
			if (appSettings.frmMain.bKioskMode == false)
			{
				this->ClientWidth = pnlMain->Width + pnlSpeedDial->Width;
            }
		}
		pnlSpeedDial->Visible = true;
		btnSpeedDialPanel->Caption = "<<";
	}
	else
	{
		pnlSpeedDial->Visible = false;
		if (appSettings.frmMain.bKioskMode == false)
		{
			this->Width -= pnlSpeedDial->Width;
        }
		btnSpeedDialPanel->Caption = ">>";				
	}
}

void TfrmMain::ToggleSpeedDial(void)
{
	appSettings.frmMain.bSpeedDialVisible = !appSettings.frmMain.bSpeedDialVisible;
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
				if (appSettings.frmMain.bNoBeepOnEnterKey)
				{
                    // this was unintentional, but it looks like Windows beep is meaning that [Enter] is not accepted by the control 
					Key = NULL;
				}
			}
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
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onDial.c_str());
		RunScript(ScriptExec::SRC_TYPE_ON_DIAL, Key, asScriptFile.c_str());
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
}

//---------------------------------------------------------------------------
void __fastcall TfrmMain::OnTrayIconLeftBtnDown(TObject *Sender)
{
	ToggleVisibility();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::OnTrayIconRightBtnDown(TObject *Sender)
{
	//PopupMenu1 -> Tag = 1;
}

void __fastcall TfrmMain::miMinimizeTrayClick(TObject *Sender)
{
	Visible = false;
	ShowWindow(Application->Handle, SW_HIDE);	// hide taskbar button
}
//---------------------------------------------------------------------------

void TfrmMain::ToggleVisibility(void)
{
	Visible = !Visible;
	if (Visible)
	{
		Application->Restore();
		ShowWindow(Application->Handle, SW_SHOW);	// show taskbar button
		SetActiveWindow (Handle);
		SetForegroundWindow (Handle);
		SetWindowPos (Handle, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		if (appSettings.frmMain.bSpeedDialOnly == false)
		{
			cbCallURI->SetFocus();
		}
		frmHistory->SetUpdating(pcMain->ActivePage == tsHistory);		
	}
	else
	{
		ShowWindow(Application->Handle, SW_HIDE);	// hide taskbar button
		frmHistory->SetUpdating(false);
	}
}

void TfrmMain::SetStatus(AnsiString text)
{
	if (StatusBar)
	{
		StatusBar->SimpleText = text;
	}
	trIcon->SetHint(text);
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
			UA->Answer(0, appSettings.uaConf.audioCfgPlayIntercom.mod, appSettings.uaConf.audioCfgPlayIntercom.dev);
		} else {
			Answer();
		}
	} else if (autoAnswerCode >= 400) {
		UA->Hangup(0, autoAnswerCode);
		lbl2ndParty->Caption = "";
		lbl2ndPartyDesc->Caption = "";
		lblCallState->Caption = "";
		call.incoming = false;
		call.progress = false;
		call.connected = false;
		return;
	}
}

void __fastcall TfrmMain::tmrAutoAnswerTimer(TObject *Sender)
{
	AutoAnswer();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::edTransferKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_RETURN)
	{
		if (edTransfer->Text == asTransferHint || edTransfer->Text == "")
			return;
		UA->Transfer(0, edTransfer->Text);
		if (appSettings.frmMain.bNoBeepOnEnterKey)
		{
			Key = NULL;
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
	LOG("Handling commnand line\n");
	CommandLine &cmd = CommandLine::Instance();
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
		cbCallURI->SetFocus();
		/** \todo Bring window to front */
		//BringToFront();
	}
	else if (cmd.action == CommandLine::ACTION_APP_QUIT)
	{
    	actExit->Execute();
	}
	else if (cmd.action == CommandLine::ACTION_PROGRAMMABLE_BTN)
	{
		ProgrammableButtonClick(cmd.programmableBtnId);
	}
	cmd.action = CommandLine::ACTION_NONE;
}

void TfrmMain::ProgrammableButtonClick(int buttonId)
{
	if (buttonId >= 0)
	{
		int containerId = buttonId/ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN;
		if (containerId < sizeof(frmButtonContainers)/sizeof(frmButtonContainers[0]))
		{
			int id = buttonId % ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN;
			TProgrammableButton* btn = frmButtonContainers[containerId]->GetBtn(id);
			btn->OnClick(btn);
		}
	}
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
				fileFull.sprintf("%s\\%s", ExtractFileDir(Application->ExeName).c_str(), file.c_str());
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
	LOG("Key = %d, CTRL = %d, SHIFT = %d, ALT = %d\n",
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
			int containerId = action.id/ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN;
			if (containerId < sizeof(frmButtonContainers)/sizeof(frmButtonContainers[0]))
			{
				int btnId = action.id % ProgrammableButtons::CONSOLE_BTNS_PER_COLUMN;
				TProgrammableButton* btn = frmButtonContainers[containerId]->GetBtn(btnId);
				btn->OnClick(btn);
				//OnProgrammableBtnClick(cfg->action.id, btn);
			}
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
		if (appSettings.frmMain.bSpeedDialOnly == false)
		{
        	ToggleSpeedDial();
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
	if (openDialogCsv->Execute())
	{
		frmContactsCsvImport->Load(openDialogCsv->FileName, &contacts);
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
	if (state == notificationIconState || useOwnTrayIcon)
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

void TfrmMain::UpdateDialpadBackgroundImage(void)
{
	AnsiString asDialpadBackgroundFile;
	try
	{
		static AnsiString lastImage;
		if (appSettings.frmMain.dialpadBackgroundImage != "" && appSettings.frmMain.dialpadBackgroundImage != lastImage)
		{
			asDialpadBackgroundFile.sprintf("%s\\img\\%s", ExtractFileDir(Application->ExeName).c_str(),
				appSettings.frmMain.dialpadBackgroundImage.c_str());
			imgDialpadBackground->Picture->Bitmap->PixelFormat = pf24bit;
			imgDialpadBackground->Picture->LoadFromFile(asDialpadBackgroundFile);
			lastImage = appSettings.frmMain.dialpadBackgroundImage;
		}
	}
	catch (...)
	{
		LOG("Failed to load dialpad background (%s)\n", asDialpadBackgroundFile.c_str());
	}
}

void __fastcall TfrmMain::tmrScriptTimer(TObject *Sender)
{
	tmrScript->Enabled = false;
	if (appSettings.Scripts.onTimer != "")
	{
		AnsiString asScriptFile;
		asScriptFile.sprintf("%s\\scripts\\%s", ExtractFileDir(Application->ExeName).c_str(), appSettings.Scripts.onTimer.c_str());
		RunScript(ScriptExec::SRC_TYPE_ON_TIMER, -1, asScriptFile.c_str(), false);
		tmrScript->Enabled = true;
	}
	// timer disables itself if onTimer script is not configured
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trbarSoftvolMicChange(TObject *Sender)
{
	TTrackBar* tr = trbarSoftvolMic;
	UA->UpdateSoftvolTx(tr->Max - tr->Position + tr->Min);
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::trbarSoftvolSpeakerChange(TObject *Sender)
{
	TTrackBar* tr = trbarSoftvolSpeaker;
	UA->UpdateSoftvolRx(tr->Max - tr->Position + tr->Min);
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
		this->Menu = MainMenu;
		this->StatusBar->Visible = true;
		this->BorderStyle = bsSingle;
		SetSpeedDial(false);
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


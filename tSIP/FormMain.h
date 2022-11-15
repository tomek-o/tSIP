//---------------------------------------------------------------------------

#ifndef FormMainH
#define FormMainH                                   
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#include "Settings.h"
#include "ProgrammableButtons.h"
#include <Menus.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <StdActns.hpp>
#include <Buttons.hpp>
#include "ButtonType.h"
#include "common/Observer.h"
#include "common/Mutex.h"
#include <Dialogs.hpp>
#include <list>
#include <string>

class TrayIcon;
class TProgrammableButton;
class ButtonConf;
class PhoneInterface;
struct HotKeyConf;
struct Action;
struct Call;
struct Recorder;

//---------------------------------------------------------------------------
class TfrmMain : public TForm, Observer
{
__published:	// IDE-managed Components
	TStatusBar *StatusBar;
	TPanel *pnlMain;
	TPageControl *pcMain;
	TTabSheet *tsDialpad;
	TTabSheet *tsHistory;
	TMainMenu *MainMenu;
	TMenuItem *miFile;
	TMenuItem *miExit;
	TMenuItem *miView;
	TMenuItem *miViewLog;
	TMenuItem *miSettings;
	TMenuItem *miCommonSettings;
	TMenuItem *miHelp;
	TMenuItem *miAbout;
	TActionList *ActionList;
	TAction *actShowAbout;
	TAction *actShowSettings;
	TAction *actShowLog;
	TFileExit *actFileExit;
	TImageList *imglistActions;
	TTimer *tmrStartup;
	TTimer *tmrCallbackPoll;
	TTimer *tmrBackspace;
	TTabSheet *tsContacts;
	TMenuItem *miMinimizeTray;
	TPopupMenu *popupTray;
	TMenuItem *miTrayExit;
	TAction *actExit;
	TTimer *tmrAutoAnswer;
	TImageList *imgListButtons;
	TTimer *tmrAntirepeat;
	TAction *actContactsCsvImport;
	TMenuItem *miImportContactsFromCsv;
	TOpenDialog *openDialog;
	TImageList *imgListIcons;
	TTimer *tmrScript;
	TMenuItem *miClearCallsHistory;
	TTimer *tmrClearCallState;
	TMenuItem *miTools;
	TMenuItem *miScripting;
	TMenuItem *miTroubleshooting;
	TTimer *tmrScript2;
	TMenuItem *miMessages;
	TMenuItem *miSettingsPatch;
	TMenuItem *miPatchButtonSettings;
	TMenuItem *miImportContactsFromXml;
	TMenuItem *miRefreshTranslationFromFile;
	TPanel *pnlCallControls;
	TComboBox *cbCallURI;
	TLabel *lbl2ndPartyDesc;
	TLabel *lbl2ndParty;
	TLabel *lblCallState;
	TSpeedButton *btnBackspace;
	TSpeedButton *btnSpeedDialPanel;
	TSpeedButton *btnHangup;
	TSpeedButton *btnMakeCall;
	TPanel *pnlDialpad;
	TEdit *edTransfer;
	TTrackBar *trbarSoftvolMic;
	TTrackBar *trbarSoftvolSpeaker;
	TBitBtn *btnResetMicVolume;
	TBitBtn *btnResetSpeakerVolume;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall actShowAboutExecute(TObject *Sender);
	void __fastcall actShowSettingsExecute(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall actShowLogExecute(TObject *Sender);
	void __fastcall tmrStartupTimer(TObject *Sender);
	void __fastcall btnMakeCallClick(TObject *Sender);
	void __fastcall btnHangupClick(TObject *Sender);
	void __fastcall tmrCallbackPollTimer(TObject *Sender);
	void __fastcall btnBackspaceClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall btnAutoAnswerClick(TObject *Sender);
	void __fastcall edTransferEnter(TObject *Sender);
	void __fastcall btnSpeedDialPanelClick(TObject *Sender);
	void __fastcall cbCallURIKeyPress(TObject *Sender, char &Key);
	void __fastcall btnBackspaceMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall btnBackspaceMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall tmrBackspaceTimer(TObject *Sender);
	void __fastcall miMinimizeTrayClick(TObject *Sender);
	void __fastcall actExitExecute(TObject *Sender);
	void __fastcall tmrAutoAnswerTimer(TObject *Sender);
	void __fastcall edTransferKeyPress(TObject *Sender, char &Key);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall tmrAntirepeatTimer(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall actContactsCsvImportExecute(TObject *Sender);
	void __fastcall pcMainChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormMouseActivate(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y, int HitTest,
          TMouseActivate &MouseActivate);
	void __fastcall tmrScriptTimer(TObject *Sender);
	void __fastcall trbarSoftvolMicChange(TObject *Sender);
	void __fastcall trbarSoftvolSpeakerChange(TObject *Sender);
	void __fastcall btnResetMicVolumeClick(TObject *Sender);
	void __fastcall btnResetSpeakerVolumeClick(TObject *Sender);
	void __fastcall miClearCallsHistoryClick(TObject *Sender);
	void __fastcall tmrClearCallStateTimer(TObject *Sender);
	void __fastcall miScriptingClick(TObject *Sender);
	void __fastcall miTroubleshootingClick(TObject *Sender);
	void __fastcall tmrScript2Timer(TObject *Sender);
	void __fastcall miMessagesClick(TObject *Sender);
	void __fastcall miSettingsPatchClick(TObject *Sender);
	void __fastcall miPatchButtonSettingsClick(TObject *Sender);
	void __fastcall miImportContactsFromXmlClick(TObject *Sender);
	void __fastcall miRefreshTranslationFromFileClick(TObject *Sender);
	void __fastcall btnResetMicVolumeMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall btnResetSpeakerVolumeMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
private:	// User declarations
	static void TranslateForm(void* obj);

	double initialScaling;

	TrayIcon *trIcon;

	void UpdateLogConfig(void);
	void SetSpeedDial(bool visible);
	void UpdateCallHistory(void);
	void ShowTrayNotifier(AnsiString description, AnsiString uri, bool incoming);
	void HideTrayNotifier(void);
	AnsiString CleanUri(AnsiString uri);
	AnsiString GetClip(AnsiString uri);
	void MakeCall(AnsiString target);
	void CallNumberBackspace(void);
	void Hangup(int sipCode = 486, AnsiString reason = "Busy Here");
	void Answer(void);
	std::string OnGetDial(void);
	void OnSetDial(std::string number);
	void OnSwitchAudioSource(std::string mod, std::string dev);
	void OnBlindTransfer(const std::string& target);
	Call* OnGetCall(void);
	void OnResetCall(void);
	Call* OnGetPreviousCall(void);	
	Recorder* OnGetRecorder(int id);
	int OnGetContactId(const char* user);
	int OnGetBlfState(int contactId, std::string &number, std::string &remoteIdentity, std::string &remoteIdentityDisplay, enum dialog_info_direction &direction);
	int OnGetStreamingState(void);
	unsigned int OnGetAudioErrorCount(void);
	int OnGetRegistrationState(void);
	void OnSetTrayIcon(const char* file);
	int OnPluginSendMessageText(const char* dllName, const char* text);
	int OnPluginEnable(const char* dllName, bool state);
	int OnRecordStart(const char* file, int channels, int side, int fileFormat, unsigned int bitrate);
	std::string OnGetRxDtmf(void);
	void OnShowBtnContainerStatusPanel(int id, bool state);
	void OnSetBtnContainerBackground(int id, const char* file);
	void OnDisableBringToFront(bool state);
	void OnSetKeepForeground(bool state);
	std::string OnGetUserName(void);
	const ButtonConf* OnGetButtonConf(int id);
	void MainMenuShow(bool state);
	void ApplicationClose(void);

	int autoAnswerCode;
	bool autoAnswerIntercom;
	bool muteRing;
	void AutoAnswer(void);
	void ProgrammableButtonClick(int buttonId);	
	void StartRing(AnsiString wavFile = "ring.wav");
	AnsiString RingFile(AnsiString alertInfo);
	void Redial(void);
	void HttpQuery(void);
	void AccessCallUrl(void);
	void ExecuteApp(AnsiString cmd, AnsiString params);
	void Dial(char digit);
	void DialString(const std::string& digits, bool runScript);
	void StartRecording(void);
	void PollCallbackQueue(void);
	void HandleCommandLine(void);
	void Finalize(void);

	const HotKeyConf *lastHotkey;

	std::list<PhoneInterface *> phones;
	void OnPhoneKey(int keyCode, int state);
	int OnPhonePagingTx(const char* target, const char* filename, const char* codecname);
	void OnPhoneClearDial(void);
	int OnGetNumberDescription(const char* number, char* description, int descriptionSize);
	int OnSetVariable(const char* name, const char* value);
	int OnClearVariable(const char* name);
	void OnQueuePush(const char* name, const char* value);
	int OnQueuePop(const char* name, AnsiString &value);
	int OnQueueClear(const char* name);
	int OnQueueGetSize(const char* name);
	void OnAddOutputText(const char* text);

	void ToggleVisibility(void);
	void ToggleSpeedDial(void);
	void RegisterGlobalHotKeys(void);
	void ExecAction(const struct Action& action);
	int RunScript(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled);

	bool notificationIconState;
	void SetNotificationIcon(bool state);
	bool errorIconState;
	void SetErrorIcon(bool state);
	int appState;
	void SetKioskMode(bool state);
	int UpdateSettingsFromJson(AnsiString json);
	void UpdateSettings(const Settings &prev);
	int UpdateButtonsFromJson(AnsiString json);
	void UpdateAutoAnswer(void);
	void UpdateClip(void);
	void SetMainWindowLayout(int id);
	void LoadTranslation(void);
	void UpdateSize(void);
	void UpdateDialpad(void);
	void FocusCbCallUri(void);
	void __fastcall OnTrayIconLeftBtnDown(TObject *Sender);
	void __fastcall WMCopyData(TWMCopyData& msg);
	void __fastcall WMEndSession(TWMEndSession &Msg);	
	void __fastcall WMHotKey(TWMHotKey &Message);
public:		// User declarations
	__fastcall TfrmMain(TComponent* Owner);
	__fastcall ~TfrmMain();
	void OnCall(AnsiString uri);
	void OnCall2(const char* uri);
	void OnPhonebookEdit(AnsiString uri);
	void OnHttpQuery(AnsiString uri);
	AnsiString OnGetContactName(AnsiString uri);	
	void OnProgrammableBtnClick(int id, TProgrammableButton* btn);
	void OnProgrammableBtnMouseUpDown(int id, TProgrammableButton* btn);
	void OnRestartUa(void);
	void obsUpdate(Observable* o, Argument * arg);
	void RunScriptFile(int srcType, int srcId, AnsiString filename, bool &handled, bool showLog = true);
	void SetTrayIconHint(AnsiString text);	

	BEGIN_MESSAGE_MAP
		MESSAGE_HANDLER(WM_COPYDATA, TWMCopyData, WMCopyData)
		MESSAGE_HANDLER(WM_ENDSESSION, TWMEndSession, WMEndSession)
		VCL_MESSAGE_HANDLER(WM_HOTKEY, TWMHotKey, WMHotKey)
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif

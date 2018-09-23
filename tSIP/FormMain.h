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
#include "Call.h"
#include "common/Observer.h"
#include <Dialogs.hpp>
#include <list>
#include <string>

class TrayIcon;
class TfrmButtonContainer;
class TProgrammableButton;
class PhoneInterface;
struct HotKeyConf;
struct Action;

//---------------------------------------------------------------------------
class TfrmMain : public TForm, Observer
{
__published:	// IDE-managed Components
	TStatusBar *StatusBar;
	TPanel *pnlMain;
	TLabel *lblCallState;
	TLabel *lbl2ndParty;
	TSpeedButton *btnBackspace;
	TComboBox *cbCallURI;
	TPageControl *pcMain;
	TTabSheet *tsDialpad;
	TSpeedButton *btnDial1;
	TSpeedButton *btnDial2;
	TSpeedButton *btnDial3;
	TSpeedButton *btnDial4;
	TSpeedButton *btnDial5;
	TSpeedButton *btnDial6;
	TSpeedButton *btnDial7;
	TSpeedButton *btnDial8;
	TSpeedButton *btnDial9;
	TSpeedButton *btnDialStar;
	TSpeedButton *btnDial0;
	TSpeedButton *btnDialHash;
	TEdit *edTransfer;
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
	TLabel *lbl2ndPartyDesc;
	TMenuItem *miMinimizeTray;
	TPopupMenu *popupTray;
	TMenuItem *miTrayExit;
	TAction *actExit;
	TTimer *tmrAutoAnswer;
	TImageList *imgListButtons;
	TPanel *pnlButtonsBasic;
	TTimer *tmrAntirepeat;
	TPanel *pnlSpeedDial;
	TAction *actContactsCsvImport;
	TMenuItem *ImportcontactsfromCSVfile1;
	TOpenDialog *openDialogCsv;
	TImageList *imgListIcons;
	TImage *imgDialpadBackground;
	TSpeedButton *btnMakeCall;
	TSpeedButton *btnHangup;
	TTimer *tmrScript;
	TSpeedButton *btnSpeedDialPanel;
	TTrackBar *trbarSoftvolMic;
	TTrackBar *trbarSoftvolSpeaker;
	TSpeedButton *btnResetMicVolume;
	TSpeedButton *btnResetSpeakerVolume;
	TMenuItem *miClearCallsHistory;
	TTimer *tmrClearCallState;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall actShowAboutExecute(TObject *Sender);
	void __fastcall actShowSettingsExecute(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall actShowLogExecute(TObject *Sender);
	void __fastcall tmrStartupTimer(TObject *Sender);
	void __fastcall btnMakeCallClick(TObject *Sender);
	void __fastcall btnHangupClick(TObject *Sender);
	void __fastcall tmrCallbackPollTimer(TObject *Sender);
	void __fastcall btnDialClick(TObject *Sender);
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
private:	// User declarations
	TrayIcon *trIcon;
	TfrmButtonContainer* frmButtonContainers[1 + ProgrammableButtons::EXT_CONSOLE_COLUMNS];
	Call call;
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
	void UpdateLogConfig(void);
	void SetSpeedDial(bool visible);
	void UpdateDialpadBackgroundImage(void);
	void UpdateCallHistory(void);
	void ShowTrayNotifier(AnsiString description, AnsiString uri, bool incoming);
	AnsiString CleanUri(AnsiString uri);
	AnsiString GetClip(AnsiString uri);
	void MakeCall(AnsiString target);
	void CallNumberBackspace(void);
	void Hangup(void);
	void Answer(void);
	std::string OnGetDial(void);
	void OnSetDial(std::string number);
	void OnSwitchAudioSource(std::string mod, std::string dev);
	void OnBlindTransfer(const std::string& target);
	int OnGetCallState(void);
	int OnIsCallIncoming(void);
	std::string OnGetCallPeer(void);
	std::string OnGetCallInitialRxInvite(void);
	std::string OnGetRecordFile(void);
	int OnGetBlfState(int contactId, std::string &number);
	int OnGetStreamingState(void);
	int OnGetRegistrationState(void);
	std::string OnGetInitialCallTarget(void);
	void OnSetInitialCallTarget(std::string number);
	void OnSetTrayIcon(const char* file);
	void OnSetButtonCaption(int id, std::string text);
	void OnSetButtonDown(int id, bool state);	
	void OnSetButtonImage(int id, const char* file);
	int OnPluginSendMessageText(const char* dllName, const char* text);
	int OnRecordStart(const char* file, int channels);
	int OnGetRecordingState(void);
	std::string OnGetRxDtmf(void);
	std::string OnGetUserName(void);

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
	void DialString(const std::string& digits);
	void StartRecording(void);
	void UpdateBtnState(enum Button::Type type, bool state);
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

	void SetStatus(AnsiString text);
	void ToggleVisibility(void);
	void ToggleSpeedDial(void);
	void RegisterGlobalHotKeys(void);
	void ExecAction(const struct Action& action);
	void RunScript(int srcType, int srcId, AnsiString filename, bool showLog = true);
	bool notificationIconState;
	void SetNotificationIcon(bool state);
	void SetKioskMode(bool state);
	void __fastcall OnTrayIconLeftBtnDown(TObject *Sender);
	void __fastcall OnTrayIconRightBtnDown(TObject *Sender);
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
	void obsUpdate(Observable* o, Argument * arg);

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

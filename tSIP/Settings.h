/** \file
*/
//---------------------------------------------------------------------------

#ifndef SettingsH
#define SettingsH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <Graphics.hpp>
#include "UaConf.h"
#include "HotKeyConf.h"
#include "PhoneConf.h"
#include "SettingsAppVersion.h"
#include "buttons/ButtonContainerConf.h"
#include "DialpadConf.h"
#include "history/HistoryConf.h"
#include <deque>
#include <list>


namespace Json
{
	struct Value;
}

struct Font
{
	AnsiString name;
	int size;
	TFontStyles style;
	Font(void);
	bool operator==(const Font& right) const {
		return (
			name == right.name &&
			size == right.size &&
			style == right.style
		);
	}
	bool operator!=(const Font& right) const {
		return !(*this == right);
	}
};

class Settings
{
private:
	struct BrandingInitializer
	{
		BrandingInitializer(void);
	} brandingInitializer;
public:
	int Read(AnsiString asFileName);
	int Write(AnsiString asFileName);
	int UpdateFromText(AnsiString text);
	struct _gui
	{
		enum { SCALING_MIN = 50 };
        enum { SCALING_DEF = 100 };
		enum { SCALING_MAX = 500 };
		int scalingPct;					///< scaling (percentage)
        _gui(void):
            scalingPct(SCALING_DEF)
        {
        }
	} gui;
	struct _info
	{
		SettingsAppVersion appVersion;	///< main executable version (from resources)
	} info;
	struct _frmMain
	{
		enum { MIN_HEIGHT = 200 };
		enum { MIN_WIDTH = 100 };
		int iPosX, iPosY;				///< main window coordinates
		int collapsedWidth, collapsedHeight;	///< width, height when console is not visible
		int expandedWidth, expandedHeight;		///< width, height with console visible
		int expandingPosLeftOffset, expandingPosTopOffset;	///< moving window when expanding/collapsing

		// call panel position
		int collapsedCallPanelLeft, collapsedCallPanelTop;
		int expandedCallPanelLeft, expandedCallPanelTop;

		// main panel position
		int collapsedMainPanelLeft, collapsedMainPanelTop;
		int expandedMainPanelLeft, expandedMainPanelTop;
		
		bool bWindowMaximized;			///< is main window maximized?
		bool bAlwaysOnTop;
		bool bStartMinimizedToTray;
		bool bSpeedDialVisible;
		bool bUseClientAreaSizes;
		bool bHideCallPanel;			///< do not show panel with dialing combobox, call/hangup buttons
		bool bHideMainPanel;			///< do not show panel with Dialpad/Contacts/History
		bool bSpeedDialPopupMenu;       ///< enable/disable popup menu (editing) for speed dial / BLF panels
		bool bSpeedDialIgnorePresenceNote;	///< do not show "note" for presence
		bool bSpeedDialIgnoreDialogInfoRemoteIdentity;	///< do not show remote id as 2nd line for BLF
		bool bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing;		///< if dialog-info remote identity is missing (e.g. in state = confirmed) keep identity from previous notification 
		bool bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated;	///< ignore/clear remote id for BLF if state = terminated (idle)
		enum DialogInfoPreferredState {
			DIALOG_INFO_PREFERRED_FIRST = 0,
			DIALOG_INFO_PREFERRED_EARLY,
			DIALOG_INFO_PREFERRED_CONFIRMED,
			DIALOG_INFO_PREFERRED__LIMITER
		} dialogInfoPreferredState;		///< which dialog information should be preferred if NOTIFY contains multiple dialog info?
		bool bXBtnMinimize;				///< minimize to tray when 'X' is pressed
		bool bRestoreOnIncomingCall;	///< restore minimized window on incoming call
		bool bSingleInstance;			///< check if another instance is running if started without parameters and bring it to front
		bool bNoBeepOnEnterKey;			///< do not beep when [Enter] is pressed in dial or transfer edit
		AnsiString mainIcon;				///< .ico file replacing regular icon
		AnsiString trayNotificationImage;	///< .bmp file to be shown in tray on missing call (replacing regular image)
		bool bHideSettings;				///< hide settings menu from user
		bool bHideView;					///< hide "View" menu from user
		bool bHideTools;				///< hide "Tools" menu from user
		bool bHideHelp;					///< hide "Help" menu from user
		bool bKioskMode;
		bool bHideStatusBar;
		bool bHideMainMenu;
		bool bHideDialpad;
		bool bBorderless;

		bool bHideSpeedDialToggleButton;///< hide "<<"/">>" button
		bool bHideMouseCursor;
		bool bShowWhenAnsweringCall;
		bool bShowWhenMakingCall;
		bool bUseCustomCaption;				///< use customCaption
		AnsiString customCaption;			///< custom caption for main window
		bool bUseCustomApplicationTitle;       ///< use customApplicationTitle
		AnsiString customApplicationTitle;	///< custom title for application (taskbar text)
		bool bShowSettingsIfAccountSettingIsHidden;
		bool bNoTaskbarButtonRestore;		///< do not restore/show taskbar button when toggling visibility (apparently restoring button may show wrong taskbar when using Windows RDP)
        bool bNoTrayIcon;					///< do not create tray icon

		std::vector<int> pre0p2speedDialWidth;	///< width of each console column from version < 0.2

		struct _bitmaps
		{
			AnsiString bmpBtnResetMicVolume;
			AnsiString bmpBtnResetSpeakerVolume;
			AnsiString bmpBtnBackspace;
			AnsiString bmpConsoleHide;
			AnsiString bmpConsoleShow;
		} bitmaps;

		int layout;

		enum DialComboboxOrder {
			DialComboboxOrderByNumber = 0,
			DialComboboxOrderByTime,

			DialComboboxOrder_Limiter
		} dialComboboxOrder;

		_frmMain(void);
	} frmMain;
	struct _frmSpeedDial
	{
		bool useGrid;
        enum {
			MIN_GRID_SIZE = 2,
			DEFAULT_GRID_SIZE = 4,
			MAX_GRID_SIZE = 64
        };
		int gridSize;
		bool showStatus;
		enum {
			MIN_STATUS_PANEL_HEIGHT = 30,
			DEF_STATUS_PANEL_HEIGHT = 80,
			MAX_STATUS_PANEL_HEIGHT = 700
		};
		int statusPanelHeight;
		bool hideEmptyStatus;
		bool dragApplicationWithButtonContainer;
		bool saveAllSettings;	///< save all (even identical to default) button settings; larger JSON file but possibly easier to edit
		_frmSpeedDial(void);
	} frmSpeedDial;	
	struct _frmTrayNotifier
	{
		int iPosX, iPosY;				///< coordinates
		int iHeight, iWidth;
		bool showOnIncoming;
		bool skipIfMainWindowVisible;
		bool showOnOutgoing;
		bool hideWhenAnsweringCall;		///< for manual call answer
		bool hideWhenAnsweringCallAutomatically;
		AnsiString backgroundImage;
		enum { SCALING_MIN = 50 };
		enum { SCALING_DEF = 100 };
		enum { SCALING_MAX = 500 };
		int scalingPct;					///< scaling (percentage)
        bool doNotChangePosition;

		_frmTrayNotifier(void);		
	} frmTrayNotifier;
	struct _frmContactPopup
	{
		bool showOnIncoming;
		bool showOnOutgoing;
		int iPosX, iPosY;				///< coordinates
		int iWidth, iHeight;

		_frmContactPopup(void);		
	} frmContactPopup;
	struct _Logging
	{
		bool bLogToFile;
		bool bFlush;
		enum {
			MIN_MAX_FILE_SIZE = 0,
			MAX_MAX_FILE_SIZE = 1000*1024*1024
		};
		enum {
            DEF_MAX_FILE_SIZE = 10*1024*1024
        };
		int iMaxFileSize;
		enum {
			MAX_LOGROTATE = 5
		};
		enum {
			DEF_LOGROTATE = 1
		};
		unsigned int iLogRotate;
		unsigned int iMaxUiLogLines;		

		Font consoleFont;

        _Logging(void):
            bLogToFile(false),
            bFlush(false),
            iMaxFileSize(Settings::_Logging::DEF_MAX_FILE_SIZE),
            iLogRotate(Settings::_Logging::DEF_LOGROTATE),
            iMaxUiLogLines(5000)
		{
			consoleFont.name = "Courier New";
        }
	} Logging;
	struct _Calls
	{
		AnsiString extraHeaderLines;
		bool bDisconnectCallOnAudioError;
		_Calls(void):
			bDisconnectCallOnAudioError(true)
		{
		}
	} Calls;
	struct _Display
	{
		bool bUserOnlyClip;				///< show only user part of URI (if present)
		bool bDecodeUtfDisplayToAnsi;
		bool bUsePAssertedIdentity;		///< use PAI if present		 
	} Display;
	struct _Integration
	{
		bool bAddFilterWMCopyData;
		AnsiString asProtocol;
		bool bDoNotUseSipPrefixForDirectIpCalls;
		bool bDoNotPassParametersToPreviousInstance;
		bool bAcceptCommandLineScript;
		_Integration(void);
	} Integration;
	struct _Ring
	{
		AnsiString defaultRing;
		AnsiString bellcore[8];
	} Ring;
	struct _HttpQuery
	{
		AnsiString url;
		enum OpenMode
		{
			openManualOnly = 0,
			openAutomaticOnIncoming,
			openAutomaticOnIncomingAnswer,

			openModeLimiter
		} openMode;
	} HttpQuery;
	struct _SipAccessUrl
	{
		enum AccessMode
		{
			accessModeAlwaysPassive = 0,
			accessModeFromMsg,
			accessModeAlwaysActive,

			accessModeLimiter
		} accessMode;
	} SipAccessUrl;
	struct _Contacts
	{
		AnsiString file;
		bool checkIfFileUpdated;
		unsigned int checkIfFileUpdatedPeriod;
		bool filterUsingNote;
		bool openFileOnIncoming;
		bool openFileOnOutgoing;
		bool storeNoteInSeparateFile;	// use file associated with contact to store note
		_Contacts(void):
			checkIfFileUpdated(false),
			checkIfFileUpdatedPeriod(60),
			filterUsingNote(false),
			openFileOnIncoming(false),
			openFileOnOutgoing(false),
			storeNoteInSeparateFile(false)
		{}
	} Contacts;

    struct HistoryConf history;

	struct _Messages
	{
    	AnsiString ring;
	} Messages;
	struct _Scripts
	{
		AnsiString onMakeCall;		///< running when outgoing call is initiated
		AnsiString onCallState;
		AnsiString onStreamingState;
		AnsiString onRegistrationState;
		AnsiString onRecorderState;
		AnsiString onEncryptionState;
		AnsiString onStartup;
		AnsiString onTimer;
		int timer;					///< ms
		AnsiString onTimer2;		///< second timer; having two timers allows having two different timer periods, putting long script in one timer even, shorted, frequently running in other
		int timer2;					///< ms
		AnsiString onDialogInfo;	///< on BLF (dialog-info) state change
		AnsiString onDial;			///< running when user presses one of the dialpad buttons or any key when main number edit is focused
									///< key value (char) is passed as srcId
		AnsiString onProgrammableButton;
		AnsiString onProgrammableButtonMouseUpDown;
		AnsiString onAudioDeviceError;
		AnsiString onCustomRequestReply;
		AnsiString onContactNoteOpen;	///< after contact note window is opened

		AnsiString lastScriptWindowDir;
	} Scripts;

	std::list<PhoneConf> phoneConf;

	UaConf uaConf;

	struct _Translation
	{
		AnsiString language;
		bool logMissingKeys;
		_Translation(void):
			logMissingKeys(false)
		{}
	} Translation;

	struct _ScriptWindow
	{
        AnsiString lastDir;
		/** Most Recently Used */
		enum { MRU_LIMIT = 20 };
		std::deque<AnsiString> MRU;
		void AddMru(AnsiString item);
		void ClearMruItems(void);
	} ScriptWindow;


	std::list<HotKeyConf> hotKeyConf;

	std::vector<ButtonContainerConf> buttonContainers;

	DialpadConf dialpad;

	struct _Branding
	{
		AnsiString appUrl;
		_Branding(void);
	} branding;

	struct _Locking
	{
		std::vector<AnsiString> hiddenSettingsPages;
	} locking;

	Settings(void);

private:
	int UpdateFromJsonValue(const Json::Value &root);
};

extern Settings appSettings;

#endif

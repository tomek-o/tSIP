/** \file
*/
//---------------------------------------------------------------------------

#ifndef SettingsH
#define SettingsH
//---------------------------------------------------------------------------
#include <System.hpp>
#include "UaConf.h"
#include "ButtonConf.h"
#include "HotKeyConf.h"
#include "PhoneConf.h"
#include <deque>
#include <list>


namespace Json
{
	struct Value;
}


class Settings
{
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
		struct _appVersion
		{
			unsigned int FileVersionMS;
			unsigned int FileVersionLS;
			bool operator<(const _appVersion &other) const
			{
			   if (FileVersionMS < other.FileVersionMS)
			   {
				   return true;
			   }
			   else if (other.FileVersionMS == FileVersionMS && FileVersionLS < other.FileVersionLS)
			   {
				   return true;
			   }
			   else
			   {
                   return false;
               }
			}
			_appVersion()
			{
				FileVersionMS = 0;
				FileVersionLS = 0;
			}
		} appVersion;					///< main executable version (from resources)
	} info;
	struct _frmMain
	{
		enum { MIN_HEIGHT = 200 };
		int iPosX, iPosY;				///< main window coordinates
		int iWidth, iHeight;			///< main window size
		bool bWindowMaximized;			///< is main window maximize?
		bool bAlwaysOnTop;
		bool bStartMinimizedToTray;
		bool bSpeedDialVisible;
		int iSpeedDialSize;				///< number of console columns minus 1 (0 = 1 column)
		enum {
			MIN_SPEED_DIAL_COL_WIDTH = 40
		};
		enum {
        	MAX_SPEED_DIAL_COL_WIDTH = 400
		};
		std::vector<int> speedDialWidth;///< width of each console column
		bool bSpeedDialOnly;
		bool bSpeedDialPopupMenu;       ///< enable/disable popup menu (editing) for speed dial / BLF panels
		bool bSpeedDialIgnorePresenceNote;	///< do not show "note" for presence
		bool bSpeedDialIgnoreDialogInfoRemoteIdentity;	///< do not show remote id as 2nd line for BLF
		bool bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing;		///< if dialog-info remote identity is missing (e.g. in state = confirmed) keep identity from previous notification 
		bool bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated;	///< ignore/clear remote id for BLF if state = terminated (idle)
		bool bXBtnMinimize;				///< minimize to tray when 'X' is pressed
		bool bRestoreOnIncomingCall;	///< restore minimized window on incoming call
		bool bSingleInstance;			///< check if another instance is running if started without parameters and bring it to front
		AnsiString dialpadBackgroundImage;
		bool bNoBeepOnEnterKey;			///< do not beep when [Enter] is pressed in dial or transfer edit
		AnsiString mainIcon;				///< .ico file replacing regular icon
		AnsiString trayNotificationImage;	///< .bmp file to be shown in tray on missing call (replacing regular image)
		bool bHideSettings;				///< hide settings menu from user
		bool bHideView;					///< hide "View" menu from user
		bool bHideTools;				///< hide "Tools" menu from user
		bool bHideHelp;					///< hide "Help" menu from user
		bool bKioskMode;
		bool bHideSpeedDialToggleButton;///< hide "<<"/">>" button
		bool bHideMouseCursor;
		bool bShowWhenAnsweringCall;
		bool bShowWhenMakingCall;
		bool bUseCustomCaption;				///< use customCaption
		AnsiString customCaption;			///< custom caption for main window
		bool bUseCustomApplicationTitle;       ///< use customApplicationTitle
		AnsiString customApplicationTitle;	///< custom title for application (taskbar text)
        bool bShowSettingsIfAccountSettingIsHidden;

		_frmMain(void);
	} frmMain;
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

        _Logging(void):
            bLogToFile(false),
            bFlush(false),
            iMaxFileSize(Settings::_Logging::DEF_MAX_FILE_SIZE),
            iLogRotate(Settings::_Logging::DEF_LOGROTATE),
            iMaxUiLogLines(5000)
        {
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
		bool filterUsingNote;
	} Contacts;
	struct _History
	{
		bool bNoStoreToFile;
	} History;
	struct _Scripts
	{
		AnsiString onMakeCall;		///< running when outgoing call is initiated
		AnsiString onCallState;
		AnsiString onStreamingState;
		AnsiString onRegistrationState;
		AnsiString onStartup;
		AnsiString onTimer;
		int timer;					///< ms
		AnsiString onDialogInfo;	///< on BLF (dialog-info) state change
		AnsiString onDial;			///< running when user presses one of the dialpad buttons or any key when main number edit is focused
									///< key value (char) is passed as srcId
		AnsiString onProgrammableButton;
		AnsiString onAudioDeviceError;
		AnsiString lastScriptWindowDir;
	} Scripts;

	std::list<PhoneConf> phoneConf;

	UaConf uaConf;

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

	Settings(void);

private:
	int UpdateFromJsonValue(const Json::Value &root);
};

extern Settings appSettings;

#endif

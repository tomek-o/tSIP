/** \file
 *  \brief Log display form module
 *
 *  Log display window is coupled using OnLog callback.
 *  Log callback enqueues supplied log line to private data containter.
 *  Data container size is limited - oldest log entries are discarded on
 *  overflow.
 *  OnLog callback is not coupled with VCL, so Synchronize() is not required.
 */

//---------------------------------------------------------------------------

#ifndef FormLogH
#define FormLogH
//---------------------------------------------------------------------------

#include "common/Mutex.h"
#include "Log.h"
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <deque>
#include <string>

//---------------------------------------------------------------------------
/** \brief Log display window
*/
class TfrmLog : public TForm
{
__published:	// IDE-managed Components
	TTimer *tmrUpdate;
	TRichEdit *redMain;
	TPanel *pnlBottom;
	TButton *btnClear;
	TCheckBox *chbAutoScroll;
	TPopupMenu *pmenuRichEdit;
	TMenuItem *miCopy;
	TCheckBox *chbLogMessages;
	TMenuItem *miSaveToFile;
	TSaveDialog *saveDialog;
	TCheckBox *chbLogToFile;
	TCheckBox *chbLogMessagesOnlyFirstLines;
	TButton *btnCopyAll;
	TButton *btnCopyLast;
	TButton *btnMore;
	TLabel *lblLevels;
	TCheckBox *chbLevelSip;
	TCheckBox *chbLevelError;
	TCheckBox *chbLevelWarning;
	TCheckBox *chbLevelInfo;
	TCheckBox *chbLevelDebug;
	TLabel *lblFind;
	TEdit *edSearch;
	TCheckBox *chbMatchCase;
	TComboBox *cbViewMode;
	TButton *btnOpenLog;
	TButton *btnLive;
	TLabel *lblSource;
	TOpenDialog *dlgOpenLog;
	TListView *lvLog;
	TMemo *memoLog;
	TSplitter *splLog;
	TPopupMenu *pmenuList;
	TMenuItem *miListCopySel;
	TMenuItem *miListCopy;
	TMenuItem *miListSelect;
	TLabel *lblViewType;
	void __fastcall chbLevelClick(TObject *Sender);
	void __fastcall searchFilterChange(TObject *Sender);
	void __fastcall btnOpenLogClick(TObject *Sender);
	void __fastcall btnLiveClick(TObject *Sender);
	void __fastcall cbViewModeChange(TObject *Sender);
	void __fastcall lvLogData(TObject *Sender, TListItem *Item);
	void __fastcall lvLogCustomDrawItem(TCustomListView *Sender, TListItem *Item, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall lvLogCustomDrawSubItem(TCustomListView *Sender, TListItem *Item, int SubItem, TCustomDrawState State, bool &DefaultDraw);
	void __fastcall lvLogKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall lvLogResize(TObject *Sender);
	void __fastcall lvLogSelectItem(TObject *Sender, TListItem *Item, bool Selected);
	void __fastcall miListCopySelectedClick(TObject *Sender);
	void __fastcall miListCopyAllClick(TObject *Sender);
	void __fastcall miListSelectAllClick(TObject *Sender);
	void __fastcall btnMoreClick(TObject *Sender);
	void __fastcall tmrUpdateTimer(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall btnClearClick(TObject *Sender);
	void __fastcall miCopyClick(TObject *Sender);
	void __fastcall chbLogMessagesClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall miSaveToFileClick(TObject *Sender);
	void __fastcall chbLogToFileClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall btnCopyAllClick(TObject *Sender);
	void __fastcall btnCopyLastClick(TObject *Sender);
private:	// User declarations
	Mutex mutex;
	typedef void (__closure *CallbackClose)(void);

	struct LogEntry
	{
		TColor color;
		int level;
		AnsiString asText;
	};
	std::deque<LogEntry> queDisplay;
	/// all entries kept for filtering (oldest dropped above iMaxUiLogLines)
	std::deque<LogEntry> history;
	enum { LevelCount = LogLevelDebug + 1 };
	bool levelShown[LevelCount];
	TCheckBox *chbLevel[LevelCount];
	bool ShowLevel(int level) const;
	bool ShowEntry(const LogEntry &entry) const;
	void RenderAll(void);
	void AppendEntry(const LogEntry &entry);
	/// true while a log file is shown instead of the live history
	bool viewingFile;
	std::deque<LogEntry> fileEntries;
	void OpenLogFile(const AnsiString &fileName);
	bool listMode;
	std::deque<LogEntry> listEntries;
	AnsiString VisibleText(void);
	void ScrollToEnd(void);
	void AddTextToLog(bool bUseTimestamp, int level, TColor color, AnsiString asText);

	unsigned int iMaxUiLogLines;
	bool updatingUi;
	/// filter, search, list and file controls are shown on request
	bool moreShown;
	void ApplyMore(void);
		
public:		// User declarations
	__fastcall TfrmLog(TComponent* Owner);
	__fastcall ~TfrmLog(void);
	void OnLog(int level, const char *module, const char *buf);                         ///< add text to log window
	void __fastcall redMainKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	AnsiString LastLines(int count);
	void SetLogLinesLimit(int lines) {
    	iMaxUiLogLines = lines;
	}
	CallbackClose callbackClose;                   ///< called when log window is closing
	void UpdateUi(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmLog *frmLog;
//---------------------------------------------------------------------------
#endif

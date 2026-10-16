/** \file
 *  \brief Log display form module
 */

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <ClipBrd.hpp>

#include "FormLog.h"
#include "Log.h"
#include "common/ScopedBool.h"
#include "common/ScopedLock.h"
#include "common/Mutex.h"
#include "Settings.h"
#include "Paths.h"
#include "ControlQueue.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmLog *frmLog;

//---------------------------------------------------------------------------
static AnsiString LevelName(int level);
__fastcall TfrmLog::TfrmLog(TComponent* Owner)
	: TForm(Owner),
	iMaxUiLogLines(1000),
	updatingUi(false)
{
	callbackClose = NULL;
	Width = appSettings.logging.windowWidth;
	Height = appSettings.logging.windowHeight;
	redMain->OnKeyDown = redMainKeyDown;
	// level filter row, below the existing controls of the bottom panel
	pnlBottom->Height = 100;
	listMode = appSettings.logging.listView;
	TLabel *lblLevels = new TLabel(this);
	lblLevels->Parent = pnlBottom;
	lblLevels->Caption = "Show levels:";
	lblLevels->Left = 5;
	lblLevels->Top = 52;
	for (int i = 0; i < LevelCount; i++)
	{
		levelShown[i] = ((appSettings.logging.levelFilter >> i) & 1) != 0;
		chbLevel[i] = new TCheckBox(this);
		chbLevel[i]->Parent = pnlBottom;
		chbLevel[i]->Left = 80 + i * 70;
		chbLevel[i]->Top = 50;
		chbLevel[i]->Width = 66;
		chbLevel[i]->Caption = LevelName(i);
		chbLevel[i]->Checked = levelShown[i];
		chbLevel[i]->OnClick = chbLevelClick;
	}

	// text search row
	TLabel *lblFind = new TLabel(this);
	lblFind->Parent = pnlBottom;
	lblFind->Caption = "Find:";
	lblFind->Left = 5;
	lblFind->Top = 80;
	edSearch = new TEdit(this);
	edSearch->Parent = pnlBottom;
	edSearch->Left = 80;
	edSearch->Top = 76;
	edSearch->Width = 260;
	edSearch->OnChange = searchFilterChange;
	chbMatchCase = new TCheckBox(this);
	chbMatchCase->Parent = pnlBottom;
	chbMatchCase->Left = 350;
	chbMatchCase->Top = 78;
	chbMatchCase->Width = 90;
	chbMatchCase->Caption = "Match case";
	chbMatchCase->OnClick = searchFilterChange;

	// list view: same entries, one row per line, shown instead of the rich edit
	chbListView = new TCheckBox(this);
	chbListView->Parent = pnlBottom;
	chbListView->Left = 470;
	chbListView->Top = 78;
	chbListView->Width = 90;
	chbListView->Caption = "List view";
	chbListView->OnClick = chbListViewClick;

	lvLog = new TListView(this);
	lvLog->Parent = this;
	lvLog->Align = alClient;
	lvLog->ViewStyle = vsReport;
	lvLog->ReadOnly = true;
	lvLog->RowSelect = true;
	lvLog->HideSelection = false;
	lvLog->Visible = false;
	lvLog->Columns->Add()->Caption = "Level";
	lvLog->Columns->Add()->Caption = "Message";
	lvLog->Columns->Items[0]->Width = 70;
	lvLog->Columns->Items[1]->Width = 500;
	lvLog->OwnerData = true;
	lvLog->OnData = lvLogData;
	lvLog->OnCustomDrawSubItem = lvLogCustomDrawSubItem;
	lvLog->MultiSelect = true;
	lvLog->OnResize = lvLogResize;
	lvLogResize(lvLog);
	lvLog->OnKeyDown = lvLogKeyDown;
	chbListView->Checked = listMode;
	lvLog->Visible = listMode;
	redMain->Visible = !listMode;

	// right-click menu of the list view
	pmenuList = new TPopupMenu(this);
	TMenuItem *miListCopySel = new TMenuItem(pmenuList);
	miListCopySel->Caption = "Copy selected";
	miListCopySel->OnClick = miListCopySelectedClick;
	pmenuList->Items->Add(miListCopySel);
	TMenuItem *miListCopy = new TMenuItem(pmenuList);
	miListCopy->Caption = "Copy all";
	miListCopy->OnClick = miListCopyAllClick;
	pmenuList->Items->Add(miListCopy);
	TMenuItem *miListSelect = new TMenuItem(pmenuList);
	miListSelect->Caption = "Select all";
	miListSelect->OnClick = miListSelectAllClick;
	pmenuList->Items->Add(miListSelect);
	lvLog->PopupMenu = pmenuList;
}
//---------------------------------------------------------------------------
__fastcall TfrmLog::~TfrmLog(void)
{
	// created last, so destroyed first - CLog must not keep a callback here
	CLog::Instance()->callbackLog = NULL;
}
//---------------------------------------------------------------------------

/** Color of a log line by its level (see LogLevel in Log.h) */
static TColor LevelColor(int level)
{
	// colors come from the settings, so a change applies on the next redraw
	if (level < 0 || level >= LogConf::LEVEL_COUNT)
		return clWindowText;
	return (TColor)appSettings.logging.richColors[level];
}

/** Last lines of the history, for scripts and the MCP plugin. Not affected by the filters,
 *  and available while the window is hidden (nothing is drawn then). */
AnsiString TfrmLog::LastLines(int count)
{
	ScopedLock<Mutex> lock(mutex);
	AnsiString text;
	int first = (int)history.size() - count;
	if (first < 0)
		first = 0;
	for (int i = first; i < (int)history.size(); i++)
		text += history[i].asText;
	return text;
}

void TfrmLog::OnLog(int level, const char *module, const char *buf)
{
	ScopedLock<Mutex> lock(mutex);
	/** \note DO NOT use any VCL code for safety */
	AddTextToLog(false, level, LevelColor(level), buf);
}

/** Name of a log level, used for the filter check boxes */
static AnsiString LevelName(int level)
{
	switch (level)
	{
	case LogLevelSip:	return "SIP";
	case LogLevelError:	return "Error";
	case LogLevelWarning:	return "Warning";
	case LogLevelInfo:	return "Info";
	case LogLevelDebug:	return "Debug";
	default:			return "?";
	}
}

/** Entry passes both the level filter and the text search */
bool TfrmLog::ShowEntry(const LogEntry &entry) const
{
	if (!ShowLevel(entry.level))
		return false;
	AnsiString search = edSearch->Text;
	if (search.Length() == 0)
		return true;
	if (chbMatchCase->Checked)
		return entry.asText.Pos(search) > 0;
	return UpperCase(entry.asText).Pos(UpperCase(search)) > 0;
}

bool TfrmLog::ShowLevel(int level) const
{
	return (level < 0 || level >= LevelCount) ? true : levelShown[level];
}

/** Append one line at the end of the rich edit, colored by its level.
 *  The text is inserted first and its range is colored afterwards: setting the
 *  color before the insert could be lost, e.g. on an empty control. */
void TfrmLog::AppendEntry(const LogEntry &entry)
{
	const int start = redMain->GetTextLen();
	redMain->SelStart = start;
	redMain->SelLength = 0;
	redMain->SelText = entry.asText;
	redMain->SelStart = start;
	redMain->SelLength = redMain->GetTextLen() - start;
	redMain->SelAttributes->Color = LevelColor(entry.level);
	redMain->SelAttributes->Name = appSettings.logging.consoleFont.name;
	redMain->SelLength = 0;
}

/** Rebuild the rich edit from the history, applying the level filter */
void TfrmLog::RenderAll(void)
{
	std::deque<LogEntry> all;
	{
		ScopedLock<Mutex> lock(mutex);
		all = history;
		// everything in the history is rendered below, so drop the pending copies
		queDisplay.clear();
	}

	if (listMode)
	{
		listEntries.clear();
		std::deque<LogEntry>::iterator it;
		for (it = all.begin(); it != all.end(); ++it)
			if (ShowEntry(*it))
				listEntries.push_back(*it);
		lvLog->Items->Count = (int)listEntries.size();
		lvLog->Invalidate();
		ScrollToEnd();
		return;
	}

	redMain->Lines->BeginUpdate();
	redMain->Clear();
	std::deque<LogEntry>::iterator iter;
	for (iter = all.begin(); iter != all.end(); ++iter)
	{
		if (!ShowEntry(*iter))
			continue;
		AppendEntry(*iter);
	}
	redMain->Lines->EndUpdate();

	if (chbAutoScroll->Checked)
	{
		redMain->SelStart = redMain->GetTextLen();
		redMain->Perform(EM_SCROLLCARET, 0, 0);
	}
}

void __fastcall TfrmLog::chbLevelClick(TObject *Sender)
{
	for (int i = 0; i < LevelCount; i++)
	{
		levelShown[i] = chbLevel[i]->Checked;
		appSettings.logging.levelFilter = (appSettings.logging.levelFilter & ~(1u << i)) | (levelShown[i] ? (1u << i) : 0u);
	}
	RenderAll();
}
//---------------------------------------------------------------------------

/** Search box or case option changed: rebuild the view */
void __fastcall TfrmLog::searchFilterChange(TObject *Sender)
{
	RenderAll();
}
//---------------------------------------------------------------------------

/** Text of the active view, for copy and save */
AnsiString TfrmLog::VisibleText(void)
{
	if (!listMode)
		return redMain->Text;
	AnsiString text;
	std::deque<LogEntry>::iterator it;
	for (it = listEntries.begin(); it != listEntries.end(); ++it)
		text += it->asText;
	return text;
}

/** Keep the newest row visible when auto scroll is on */
void TfrmLog::ScrollToEnd(void)
{
	if (chbAutoScroll->Checked && lvLog->Items->Count > 0)
		lvLog->Items->Item[lvLog->Items->Count - 1]->MakeVisible(false);
}

void __fastcall TfrmLog::lvLogData(TObject *Sender, TListItem *Item)
{
	if (Item->Index < 0 || Item->Index >= (int)listEntries.size())
		return;
	const LogEntry &entry = listEntries[Item->Index];
	// one row per entry: line breaks inside a message become spaces
	AnsiString text = entry.asText;
	for (int i = 1; i <= text.Length(); i++)
		if (text[i] == '\r' || text[i] == '\n')
			text[i] = ' ';
	Item->Caption = LevelName(entry.level);
	Item->SubItems->Clear();
	Item->SubItems->Add(text);
}

void __fastcall TfrmLog::lvLogCustomDrawSubItem(TCustomListView *Sender, TListItem *Item, int SubItem, TCustomDrawState State, bool &DefaultDraw)
{
	// text and background of the list view come from the settings, per level
	if (Item->Index < 0 || Item->Index >= (int)listEntries.size())
		return;
	const int lvl = listEntries[Item->Index].level;
	if (lvl < 0 || lvl >= LogConf::LEVEL_COUNT)
		return;
	Sender->Canvas->Font->Color = (TColor)appSettings.logging.listTextColors[lvl];
	Sender->Canvas->Brush->Color = (TColor)appSettings.logging.listBackgrounds[lvl];
}

/** The message column takes the width left by the level column */
void __fastcall TfrmLog::lvLogResize(TObject *Sender)
{
	const int rest = lvLog->ClientWidth - lvLog->Columns->Items[0]->Width;
	lvLog->Columns->Items[1]->Width = (rest > 100) ? rest : 100;
}
//---------------------------------------------------------------------------

/** Right-click menu of the list: copy the selected rows as plain text, in log order */
void __fastcall TfrmLog::miListCopySelectedClick(TObject *Sender)
{
	AnsiString text;
	for (int i = 0; i < lvLog->Items->Count && i < (int)listEntries.size(); i++)
		if (lvLog->Items->Item[i]->Selected)
			text += listEntries[i].asText;
	if (text.Length() > 0)
		Clipboard()->AsText = text;
}

void __fastcall TfrmLog::miListCopyAllClick(TObject *Sender)
{
	Clipboard()->AsText = VisibleText();
}

void __fastcall TfrmLog::miListSelectAllClick(TObject *Sender)
{
	for (int i = 0; i < lvLog->Items->Count; i++)
		lvLog->Items->Item[i]->Selected = true;
}
//---------------------------------------------------------------------------

/** Ctrl+C in the list copies the selected rows as plain text, in log order */
void __fastcall TfrmLog::lvLogKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if (Key != 'C' || !Shift.Contains(ssCtrl) || Shift.Contains(ssShift) || Shift.Contains(ssAlt))
		return;
	miListCopySelectedClick(Sender);
	Key = 0;
}

void __fastcall TfrmLog::chbListViewClick(TObject *Sender)
{
	listMode = chbListView->Checked;
	appSettings.logging.listView = listMode;
	lvLog->Visible = listMode;
	redMain->Visible = !listMode;
	RenderAll();
}
//---------------------------------------------------------------------------

/** Ctrl+C copies plain text only: the rich edit would also put RTF (with colors) on the clipboard */
void __fastcall TfrmLog::redMainKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	if (Key == 'C' && Shift.Contains(ssCtrl) && !Shift.Contains(ssShift) && !Shift.Contains(ssAlt))
	{
		Clipboard()->AsText = redMain->SelText;
		Key = 0;
	}
}
//---------------------------------------------------------------------------

/** Actual GUI update
 */
void __fastcall TfrmLog::tmrUpdateTimer(TObject *Sender)
{
	// hidden window: nothing to draw, the history keeps the lines and
	// UpdateUi() (called on show) renders them
	if (!Visible)
	{
		ScopedLock<Mutex> lock(mutex);
		queDisplay.clear();
		return;
	}
	// take the queued entries and release the lock at once: VCL code must not
	// run with the mutex held, or a LOG() from here would deadlock against a
	// thread that is already inside OnLog()
	std::deque<LogEntry> pending;
	{
		ScopedLock<Mutex> lock(mutex);
		if (queDisplay.empty())
			return;
		queDisplay.swap(pending);
	}

	if (listMode)
	{
		std::deque<LogEntry>::iterator it;
		for (it = pending.begin(); it != pending.end(); ++it)
		{
			if (!ShowEntry(*it))
				continue;
			listEntries.push_back(*it);
			if (listEntries.size() > iMaxUiLogLines)
				listEntries.pop_front();
		}
		lvLog->Items->Count = (int)listEntries.size();
		lvLog->Invalidate();
		ScrollToEnd();
		return;
	}

	// first save the current caret location
	int selStart = redMain->SelStart;
	int selLength = redMain->SelLength;
	// get the line number of the first visible line
	int firstLine = ::SendMessage(redMain->Handle, EM_GETFIRSTVISIBLELINE, 0 , 0);
	// disable screen redraws
	redMain->Lines->BeginUpdate();

	std::deque<LogEntry>::iterator iter;
	for(iter = pending.begin(); iter != pending.end(); ++iter)
	{
		if (!ShowEntry(*iter))
			continue;
		AppendEntry(*iter);
	}

#if 0
	while ((unsigned int)redMain->Lines->Count > iMaxUiLogLines)
	{
		// possibly causing memory leak (?)
		redMain->Lines->Delete(0);
	}
#else
	if ((unsigned int)redMain->Lines->Count > iMaxUiLogLines) {
		redMain->Clear();
	}
#endif

	// restore the caret location
	redMain->SelStart = selStart;
	redMain->SelLength = selLength;

	if (chbAutoScroll->Checked)
	{
		redMain->SelStart = redMain->GetTextLen();
		redMain->Perform( EM_SCROLLCARET, 0 , 0 );
	}
	else
	{
		// get the current first visible line,
		// compute the number of lines to scroll, and scroll...
		int currLine = ::SendMessage(redMain->Handle, EM_GETFIRSTVISIBLELINE, 0, 0);
		::SendMessage(redMain->Handle, EM_LINESCROLL, 0, firstLine - currLine);
	}
	redMain->Lines->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::FormClose(TObject *Sender, TCloseAction &Action)
{
	if (callbackClose)
		callbackClose();	
}
//---------------------------------------------------------------------------

void TfrmLog::AddTextToLog(bool bUseTimestamp, int level, TColor color, AnsiString asText)
{
	/** \note DO NOT use any VCL code for safety */
	struct LogEntry logEntry;
	logEntry.color = color;
	logEntry.level = level;
	logEntry.asText = asText;
	history.push_back(logEntry);
	if (history.size() > this->iMaxUiLogLines)
		history.pop_front();
	queDisplay.push_back(logEntry);
	if (queDisplay.size() > this->iMaxUiLogLines)
		queDisplay.pop_front();
}
void __fastcall TfrmLog::FormResize(TObject *Sender)
{
	redMain->Refresh();
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::btnClearClick(TObject *Sender)
{
	redMain->Clear();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::miCopyClick(TObject *Sender)
{
	Clipboard()->SetTextBuf( redMain->SelText.c_str() );	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::chbLogMessagesClick(TObject *Sender)
{
	if (updatingUi)
		return;
	appSettings.uaConf.logMessages = chbLogMessages->Checked;
	chbLogMessagesOnlyFirstLines->Visible = appSettings.uaConf.logMessages;
	appSettings.uaConf.logMessagesOnlyFirstLine = chbLogMessagesOnlyFirstLines->Checked;
	UA->SetMsgLogging(appSettings.uaConf.logMessages, appSettings.uaConf.logMessagesOnlyFirstLine);
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::FormShow(TObject *Sender)
{
	UpdateUi();
}
//---------------------------------------------------------------------------

void TfrmLog::UpdateUi(void)
{
	ScopedBool updating(&updatingUi);
	chbLogMessages->Checked = appSettings.uaConf.logMessages;
	chbLogMessagesOnlyFirstLines->Visible = appSettings.uaConf.logMessages;
	chbLogMessagesOnlyFirstLines->Checked = appSettings.uaConf.logMessagesOnlyFirstLine;
	chbLogToFile->Checked = appSettings.logging.logToFile;
	redMain->Font->Name = appSettings.logging.consoleFont.name;
	redMain->Font->Size = appSettings.logging.consoleFont.size;
	redMain->Font->Style = appSettings.logging.consoleFont.style;
	RenderAll();
}

void __fastcall TfrmLog::miSaveToFileClick(TObject *Sender)
{
	AnsiString fname;
	fname = saveDialog->InitialDir +
		((saveDialog->InitialDir!="")?("\\"):("")) +
		FormatDateTime("yyyymmdd_hhnnss", Now());
	saveDialog->FileName = fname;
	if (saveDialog->Execute())
	{
        {
			TStringList *lines = new TStringList();
			lines->Text = VisibleText();
			lines->SaveToFile(saveDialog->FileName);
			delete lines;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::chbLogToFileClick(TObject *Sender)
{
	// setting Checked from UpdateUi() fires this handler as well
	if (updatingUi)
		return;
	appSettings.logging.logToFile = chbLogToFile->Checked;
	if (appSettings.logging.logToFile)
		CLog::Instance()->SetFile((Paths::GetProfileDir() + "\\" + ChangeFileExt(ExtractFileName(Application->ExeName), ".log")).c_str());
	else
		CLog::Instance()->SetFile("");
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::btnCopyAllClick(TObject *Sender)
{
	Clipboard()->AsText = VisibleText();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::btnCopyLastClick(TObject *Sender)
{
	enum { LIMIT = 400 };

	AnsiString text = VisibleText();
	int newLineCnt = 0;
	for (unsigned int i = text.Length(); i>=1; i--)
	{
		if (text[i] == '\n')
		{
			newLineCnt++;
			if (newLineCnt >= LIMIT)
			{
				text = text.SubString(i+1, text.Length() - i);
				break;
			}
		}
	}

	Clipboard()->AsText = text;
}
//---------------------------------------------------------------------------


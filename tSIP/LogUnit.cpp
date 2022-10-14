/** \file
 *  \brief Log display form module
 */

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <ClipBrd.hpp>

#include "LogUnit.h"
#include "Log.h"
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
__fastcall TfrmLog::TfrmLog(TComponent* Owner)
	: TForm(Owner),
	iMaxUiLogLines(1000),
	updatingUi(false)
{
	callbackClose = NULL;
}
//---------------------------------------------------------------------------

void TfrmLog::OnLog(char *buf)
{
	ScopedLock<Mutex> lock(mutex);
	/** \note DO NOT use any VCL code for safety */
	AddTextToLog(false, clWindowText, buf);
}

/** Actual GUI update
 */
void __fastcall TfrmLog::tmrUpdateTimer(TObject *Sender)
{
// note: may cause freezing when opening log window after long time
//	if (!Visible)
//		return;
	ScopedLock<Mutex> lock(mutex);

	if (queDisplay.empty())
		return;

	// first save the current caret location
	int selStart = redMain->SelStart;
	int selLength = redMain->SelLength;
	// get the line number of the first visible line
	int firstLine = ::SendMessage(redMain->Handle, EM_GETFIRSTVISIBLELINE, 0 , 0);
	// disable screen redraws
	redMain->Lines->BeginUpdate();

	std::deque<LogEntry>::iterator iter;
	for(iter = queDisplay.begin(); iter != queDisplay.end(); ++iter)
	{
		redMain->SelStart = redMain->GetTextLen();
		redMain->SelLength = 0;
		redMain->SelAttributes->Color = iter->color;
		redMain->SelAttributes->Name = appSettings.Logging.consoleFont.name;
		redMain->SelText = iter->asText;
	}
	queDisplay.clear();

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

void TfrmLog::AddTextToLog(bool bUseTimestamp, TColor color, AnsiString asText)
{
	/** \note DO NOT use any VCL code for safety */
	struct LogEntry logEntry;
	logEntry.color = color;
	logEntry.asText = asText;
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
	updatingUi = true;
	chbLogMessages->Checked = appSettings.uaConf.logMessages;
	chbLogMessagesOnlyFirstLines->Visible = appSettings.uaConf.logMessages;
	chbLogMessagesOnlyFirstLines->Checked = appSettings.uaConf.logMessagesOnlyFirstLine;
	chbLogToFile->Checked = appSettings.Logging.bLogToFile;
	redMain->Font->Name = appSettings.Logging.consoleFont.name;
	redMain->Font->Size = appSettings.Logging.consoleFont.size;
	redMain->Font->Style = appSettings.Logging.consoleFont.style;
	updatingUi = false;	
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
        redMain->Lines->SaveToFile(saveDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::chbLogToFileClick(TObject *Sender)
{
	appSettings.Logging.bLogToFile = chbLogToFile->Checked;
	if (appSettings.Logging.bLogToFile)
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
	Clipboard()->AsText = redMain->Text;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLog::btnCopyLastClick(TObject *Sender)
{
	enum { LIMIT = 400 };

	AnsiString text = redMain->Text;
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


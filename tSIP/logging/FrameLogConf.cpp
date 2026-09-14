//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FrameLogConf.h"
#include "LogConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TfraLogConf::TfraLogConf(TComponent* Owner, LogConf &logging)
	: TFrame(Owner),
	logging(logging)
{
}
//---------------------------------------------------------------------------

void TfraLogConf::Load(void)
{
	chbLogToFile->Checked = logging.logToFile;
	chbLogFlush->Checked = logging.flush;
	chbLogTimestamps->Checked = logging.timestamps;
	cmbMaxUiLogLines->ItemIndex = -1;
	for (int i=0; i<cmbMaxUiLogLines->Items->Count; i++)
	{
		// 0 for a non numeric item: it never matches, so the item is skipped
		if ((unsigned int)StrToIntDef(cmbMaxUiLogLines->Items->Strings[i], 0) >= logging.maxUiLogLines)
		{
			cmbMaxUiLogLines->ItemIndex = i;
			break;
		}
	}
	if (cmbMaxUiLogLines->ItemIndex == -1)
	{
		cmbMaxUiLogLines->ItemIndex = cmbMaxUiLogLines->Items->Count - 1;
	}
	cbLogMaxFileSize->Text = logging.maxFileSize;
	cbLogRotate->ItemIndex = logging.logRotate;
	chbLogShowWindowAtStartup->Checked = logging.showWindowAtStartup;
	logging.consoleFont.ShowOnEdit(edLoggingConsoleFont);
}

void TfraLogConf::Apply(void)
{
	logging.logToFile = chbLogToFile->Checked;
	logging.flush = chbLogFlush->Checked;
	logging.maxFileSize = StrToIntDef(cbLogMaxFileSize->Text, logging.maxFileSize);
	if (logging.maxFileSize < LogConf::MIN_MAX_FILE_SIZE || logging.maxFileSize > LogConf::MAX_MAX_FILE_SIZE)
	{
		logging.maxFileSize = LogConf::DEF_MAX_FILE_SIZE;
	}
	logging.logRotate = cbLogRotate->ItemIndex;
	logging.maxUiLogLines = StrToIntDef(cmbMaxUiLogLines->Text, logging.maxUiLogLines);
	logging.timestamps = chbLogTimestamps->Checked;
	logging.showWindowAtStartup = chbLogShowWindowAtStartup->Checked;
}

void __fastcall TfraLogConf::btnLoggingConsoleFontSelectClick(TObject *Sender)
{
	struct FontConf *font = &logging.consoleFont;
	fontDialog->Font->Name = font->name;
	fontDialog->Font->Size = font->size;
	fontDialog->Font->Style = font->style;
	if (fontDialog->Execute())
	{
		font->name = fontDialog->Font->Name;
		font->size = fontDialog->Font->Size;
		font->style = fontDialog->Font->Style;
		font->ShowOnEdit(edLoggingConsoleFont);
	}
}
//---------------------------------------------------------------------------


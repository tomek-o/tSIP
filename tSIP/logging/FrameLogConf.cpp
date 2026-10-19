//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FrameLogConf.h"
#include "LogConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------

namespace {

/// color preset for the log colors grid; values are TColor (BGR)
struct LogColorTemplate
{
	const char *name;
	unsigned int richColors[LogConf::LEVEL_COUNT];
	unsigned int listTextColors[LogConf::LEVEL_COUNT];
	unsigned int listBackgrounds[LogConf::LEVEL_COUNT];
	unsigned int richBackground;
};

const LogColorTemplate logColorTemplates[] = {
	{
		"Classic",
		{0x800000, 0x0000C8, 0x0078A0, 0x000000, 0x808080},
		{0x800000, 0x0000C8, 0x0078A0, 0x000000, 0x808080},
		{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		0xFFFFFF
	},
	{
		"Dark",
		{0xFFAA78, 0x6E6EFF, 0x50C8F0, 0xDCDCDC, 0x8C8C8C},
		{0xFFAA78, 0x6E6EFF, 0x50C8F0, 0xDCDCDC, 0x8C8C8C},
		{0x1E1E1E, 0x1E1E1E, 0x1E1E1E, 0x1E1E1E, 0x1E1E1E},
		0x1E1E1E
	},
	{
		"High contrast",
		{0x800000, 0x0000FF, 0xA000A0, 0x000000, 0x5A5A5A},
		{0x800000, 0x0000FF, 0xA000A0, 0x000000, 0x5A5A5A},
		{0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF},
		0xFFFFFF
	}
};

const char *const levelCaptions[LogConf::LEVEL_COUNT] = {"SIP", "Error", "Warning", "Info", "Debug"};

}

__fastcall TfraLogConf::TfraLogConf(TComponent* Owner, LogConf &logging)
	: TFrame(Owner),
	logging(logging)
{
	CreateColorControls();
}
//---------------------------------------------------------------------------

// Rows below the existing controls: a grid of color cells, then the template row
void TfraLogConf::CreateColorControls(void)
{
	const int colLeft[CELLS] = {150, 260, 370};
	static const char *colCaptions[CELLS] = {"Text (log)", "Text (list)", "List background"};

	int top = 0;
	for (int i=0; i<ControlCount; i++)
	{
		int bottom = Controls[i]->Top + Controls[i]->Height;
		if (bottom > top)
			top = bottom;
	}
	top += 16;

	chbLogAubuf = new TCheckBox(this);
	chbLogAubuf->Parent = this;
	chbLogAubuf->Caption = "Logging for audio buffers";
	chbLogAubuf->Left = 8;
	chbLogAubuf->Top = top;
	chbLogAubuf->Width = 325;
	top += 28;

	TLabel *title = new TLabel(this);
	title->Parent = this;
	title->Caption = "Log window colors";
	title->Left = 8;
	title->Top = top;
	top += 24;

	// column headers
	for (int c=0; c<CELLS; c++)
	{
		TLabel *hdr = new TLabel(this);
		hdr->Parent = this;
		hdr->Caption = colCaptions[c];
		hdr->Left = colLeft[c];
		hdr->Top = top;
	}
	top += 20;

	dlgColor = new TColorDialog(this);
	for (int r=0; r<LEVELS; r++)
	{
		TLabel *lbl = new TLabel(this);
		lbl->Parent = this;
		lbl->Caption = levelCaptions[r];
		lbl->Left = 8;
		lbl->Top = top + (20 - lbl->Height) / 2;

		for (int c=0; c<CELLS; c++)
		{
			pnlCell[r][c] = new TPanel(this);
			pnlCell[r][c]->Parent = this;
			pnlCell[r][c]->Left = colLeft[c];
			pnlCell[r][c]->Top = top;
			pnlCell[r][c]->Width = 40;
			pnlCell[r][c]->Height = 20;
			pnlCell[r][c]->BevelOuter = bvLowered;
			pnlCell[r][c]->Caption = "";

			TButton *btn = new TButton(this);
			btn->Parent = this;
			btn->Caption = "...";
			btn->Left = colLeft[c] + 44;
			btn->Top = top;
			btn->Width = 30;
			btn->Tag = r * CELLS + c;
			btn->OnClick = btnColorClick;
		}
		top += 26;
	}

	// text view background: one color for the whole rich edit
	TLabel *rbLbl = new TLabel(this);
	rbLbl->Parent = this;
	rbLbl->Caption = "Text view background";
	rbLbl->Left = 8;
	rbLbl->Top = top + (20 - rbLbl->Height) / 2;
	pnlRichBackground = new TPanel(this);
	pnlRichBackground->Parent = this;
	pnlRichBackground->Left = 150;
	pnlRichBackground->Top = top;
	pnlRichBackground->Width = 40;
	pnlRichBackground->Height = 20;
	pnlRichBackground->BevelOuter = bvLowered;
	pnlRichBackground->Caption = "";
	btnRichBackground = new TButton(this);
	btnRichBackground->Parent = this;
	btnRichBackground->Caption = "...";
	btnRichBackground->Left = 194;
	btnRichBackground->Top = top;
	btnRichBackground->Width = 30;
	btnRichBackground->OnClick = btnRichBackgroundClick;
	top += 26;

	// preset row, a little lower than the grid
	top += 12;
	TLabel *tplLbl = new TLabel(this);
	tplLbl->Parent = this;
	tplLbl->Caption = "Color template:";
	tplLbl->Left = 8;

	cbColorTemplate = new TComboBox(this);
	cbColorTemplate->Parent = this;
	cbColorTemplate->Left = 90;
	cbColorTemplate->Top = top + 4;
	cbColorTemplate->Width = 160;
	tplLbl->Top = top + (cbColorTemplate->Height - tplLbl->Height) / 2;
	for (unsigned int i=0; i<sizeof(logColorTemplates)/sizeof(logColorTemplates[0]); i++)
		cbColorTemplate->Items->Add(logColorTemplates[i].name);
	cbColorTemplate->ItemIndex = 0;

	btnColorTemplate = new TButton(this);
	btnColorTemplate->Parent = this;
	btnColorTemplate->Caption = "Apply template";
	btnColorTemplate->Left = 270;
	btnColorTemplate->Top = top + 2;
	btnColorTemplate->Width = 110;
	btnColorTemplate->OnClick = btnColorTemplateClick;
	top += 28;

	// the frame is sized to its rows, so none of them is clipped
	Height = top;
}

void __fastcall TfraLogConf::btnColorClick(TObject *Sender)
{
	const int idx = static_cast<TComponent*>(Sender)->Tag;
	const int r = idx / CELLS;
	const int c = idx % CELLS;
	dlgColor->Color = pnlCell[r][c]->Color;
	if (dlgColor->Execute())
		pnlCell[r][c]->Color = dlgColor->Color;
}

void __fastcall TfraLogConf::btnRichBackgroundClick(TObject *Sender)
{
	dlgColor->Color = pnlRichBackground->Color;
	if (dlgColor->Execute())
		pnlRichBackground->Color = dlgColor->Color;
}

void __fastcall TfraLogConf::btnColorTemplateClick(TObject *Sender)
{
	const int idx = cbColorTemplate->ItemIndex;
	if (idx < 0)
		return;
	const LogColorTemplate &tpl = logColorTemplates[idx];
	for (int r=0; r<LEVELS; r++)
	{
		pnlCell[r][0]->Color = (TColor)tpl.richColors[r];
		pnlCell[r][1]->Color = (TColor)tpl.listTextColors[r];
		pnlCell[r][2]->Color = (TColor)tpl.listBackgrounds[r];
	}
	pnlRichBackground->Color = (TColor)tpl.richBackground;
}
//---------------------------------------------------------------------------

void TfraLogConf::Load(void)
{
	chbLogToFile->Checked = logging.logToFile;
	chbLogFlush->Checked = logging.flush;
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
	chbLogAubuf->Checked = logging.logAubuf;
	for (int r=0; r<LEVELS; r++)
	{
		pnlCell[r][0]->Color = (TColor)logging.richColors[r];
		pnlCell[r][1]->Color = (TColor)logging.listTextColors[r];
		pnlCell[r][2]->Color = (TColor)logging.listBackgrounds[r];
	}
	pnlRichBackground->Color = (TColor)logging.richBackground;
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
	logging.showWindowAtStartup = chbLogShowWindowAtStartup->Checked;
	logging.logAubuf = chbLogAubuf->Checked;
	for (int r=0; r<LEVELS; r++)
	{
		logging.richColors[r] = (unsigned int)pnlCell[r][0]->Color;
		logging.listTextColors[r] = (unsigned int)pnlCell[r][1]->Color;
		logging.listBackgrounds[r] = (unsigned int)pnlCell[r][2]->Color;
	}
	logging.richBackground = (unsigned int)pnlRichBackground->Color;
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

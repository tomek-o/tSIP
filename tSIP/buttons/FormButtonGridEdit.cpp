//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonGridEdit.h"
#include "ProgrammableButtons.h"
#include <assert.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonGridEdit *frmButtonGridEdit;
//---------------------------------------------------------------------------
__fastcall TfrmButtonGridEdit::TfrmButtonGridEdit(TComponent* Owner)
	: TForm(Owner),
	buttons(NULL),
	sourceBtnId(-1)
{
	for (int i=0; i<64; i++)
	{
    	cbColumns->Items->Add(i+1);
	}
	cbColumns->ItemIndex = 3;

	for (int i=0; i<64; i++)
	{
		cbRows->Items->Add(i+1);
	}
	cbRows->ItemIndex = 14;
}
//---------------------------------------------------------------------------

AnsiString TfrmButtonGridEdit::GetButtonDescription(int id)
{
	AnsiString caption;
	const ButtonConf &cfg = buttons->btnConf[id];
	if (cfg.caption != "")
		caption.sprintf("%d: %s", id, cfg.caption.c_str());
	else
		caption.sprintf("%d: [unnamed]", id);
	return caption;
}

void TfrmButtonGridEdit::SetButtons(ProgrammableButtons *buttons, int sourceBtnId)
{
	assert(buttons);
	this->sourceBtnId = sourceBtnId;
	this->buttons = buttons;

	const ButtonConf &btnSrc = buttons->btnConf[sourceBtnId];
	edColumnSpacing->Text = btnSrc.width;
	edRowSpacing->Text = btnSrc.height;	

	edSourceButton->Text = GetButtonDescription(sourceBtnId);

	int prevIndex = cbTargetTo->ItemIndex;
	cbTargetTo->Items->Clear();
	for (int i=sourceBtnId; i<buttons->btnConf.size(); i++)
	{
		cbTargetTo->Items->Add(GetButtonDescription(i));
	}
	if (prevIndex >= 0)
	{
		cbTargetTo->ItemIndex = prevIndex;
	}
	else
	{
		cbTargetTo->ItemIndex = 0;
	}
}

void __fastcall TfrmButtonGridEdit::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		Key = 0;
		Close();
	}	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonGridEdit::btnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------

int TfrmButtonGridEdit::IncrementLocation(int &columnId, int &rowId, int columnCount, int rowCount)
{
	if (cbButtonOrder->ItemIndex == 0)
	{
		rowId++;
		if (rowId >= rowCount)
		{
			rowId = 0;
			columnId++;
			if (columnId >= columnCount)
				return -1;
		}
	}
	else
	{
		columnId++;
		if (columnId >= columnCount)
		{
            columnId = 0;
			rowId++;
			if (rowId >= rowCount)
				return -1;
		}
	}
	return 0;
}

void __fastcall TfrmButtonGridEdit::btnApplyClick(TObject *Sender)
{
	const ButtonConf &btnSrc = buttons->btnConf[sourceBtnId];

	int end = sourceBtnId + cbTargetTo->ItemIndex;
	if (sourceBtnId < 0 || end < 0)
	{
		return;
	}

	int columnSpacing = StrToIntDef(edColumnSpacing->Text, -1);
	if (columnSpacing <= 0)
	{
		MessageBox(this->Handle, "Invalid column spacing value!", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}
	int rowSpacing = StrToIntDef(edRowSpacing->Text, -1);
	if (rowSpacing <= 0)
	{
		MessageBox(this->Handle, "Invalid row spacing value!", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}

	int columnId = 0, rowId = 0;
	int rowCount = cbRows->ItemIndex + 1;
	int columnCount = cbColumns->ItemIndex + 1;
	bool copyWidth = chbCopyButtonWidth->Checked;
	bool copyHeight = chbCopyButtonHeight->Checked;

	if (cbActionVisible->ItemIndex == 0)
		buttons->btnConf[sourceBtnId].visible = true;

	for (int i=sourceBtnId+1; i<=end; i++)
	{
		ButtonConf &btnDst = buttons->btnConf[i];
		if (cbActionVisible->ItemIndex == 1)
		{
			if (!btnDst.visible)
				continue;
		}
		if (cbActionContainer->ItemIndex == 0)
		{
			if (btnDst.parentId != btnSrc.parentId)
				continue;
		}
		
		if (IncrementLocation(columnId, rowId, columnCount, rowCount))
			break;
		int posX = btnSrc.left + (columnId * columnSpacing);
		int posY = btnSrc.top + (rowId * rowSpacing);
		btnDst.left = posX;
		btnDst.top = posY;
		if (copyWidth)
			btnDst.width = btnSrc.width;
		if (copyHeight)
			btnDst.height = btnSrc.height;
		if (cbActionVisible->ItemIndex == 0)
			btnDst.visible = true;
		if (cbActionContainer->ItemIndex == 0)
			btnDst.parentId = btnSrc.parentId;
	}

	buttons->Write();
	buttons->UpdateAll();	
}
//---------------------------------------------------------------------------


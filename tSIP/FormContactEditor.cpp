//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContactEditor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContactEditor *frmContactEditor;
//---------------------------------------------------------------------------
__fastcall TfrmContactEditor::TfrmContactEditor(TComponent* Owner)
	: TForm(Owner)
{
	//RichEdit: URL highlighting and OnClick event
	HANDLE handle = memoNote->Handle;
	unsigned mask = SendMessage(handle, EM_GETEVENTMASK, 0, 0);
	SendMessage(handle, EM_SETEVENTMASK, 0, mask | ENM_LINK);
	SendMessage(handle, EM_AUTOURLDETECT, true, 0);
}
//---------------------------------------------------------------------------
void __fastcall TfrmContactEditor::btnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmContactEditor::btnApplyClick(TObject *Sender)
{
	entry->description = edDescription->Text;
	entry->company = edCompany->Text;
	entry->uri1 = edNumber1->Text;
	entry->uri2 = edNumber2->Text;
	entry->uri3 = edNumber3->Text;
	entry->note = memoNote->Text;
	confirmed = true;
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmContactEditor::FormShow(TObject *Sender)
{
	confirmed = false;
	edDescription->SetFocus();	
}
//---------------------------------------------------------------------------

int __fastcall TfrmContactEditor::Start(Contacts::Entry *entry)
{
	this->entry = entry;

	edDescription->Text = entry->description;
	edCompany->Text = entry->company;
	edNumber1->Text = entry->uri1;
	edNumber2->Text = entry->uri2;
	edNumber3->Text = entry->uri3;
	memoNote->Text = entry->note;

	return ShowModal();
}

void __fastcall TfrmContactEditor::WndProc(Messages::TMessage &Message)
{
    if (Message.Msg == WM_NOTIFY)
    {
        // handle RichEdit link being clicked
        if (((LPNMHDR)Message.LParam)->code == EN_LINK)
        {
            ENLINK* p = (ENLINK *)Message.LParam;
            if (p->msg == WM_LBUTTONDOWN)
            {
				SendMessage(memoNote->Handle, EM_EXSETSEL, 0, (LPARAM)&(p->chrg));
                ShellExecute(Handle, "open", memoNote->SelText.c_str(), 0, 0, SW_SHOWNORMAL);
            }
        }
    }
	TForm::WndProc(Message);
}

void __fastcall TfrmContactEditor::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		this->Close();
	}	
}
//---------------------------------------------------------------------------


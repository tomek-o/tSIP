//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContactPopup.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContactPopup *frmContactPopup;
//---------------------------------------------------------------------------
__fastcall TfrmContactPopup::TfrmContactPopup(TComponent* Owner)
	: TForm(Owner),
	entry(NULL)
{
	//RichEdit: URL highlighting and OnClick event
	HANDLE handle = note->Handle;
	unsigned mask = SendMessage(handle, EM_GETEVENTMASK, 0, 0);
	SendMessage(handle, EM_SETEVENTMASK, 0, mask | ENM_LINK);
	SendMessage(handle, EM_AUTOURLDETECT, true, 0);
}
//---------------------------------------------------------------------------

void TfrmContactPopup::SetData(Contacts::Entry *entry)
{
	if (Visible && entry)
	{
		entry->note = note->Text;
	}
	this->entry = entry;
	AnsiString text;
	text.sprintf("%s, %s", entry->description.c_str(), entry->GetMainUri().c_str());
	this->Caption = text;
	note->Text = initialNote = entry->note;
}

bool TfrmContactPopup::isNoteModified(void)
{
	if (entry == NULL)
		return false;
	bool result = (initialNote != entry->note);
	return result;
}

void __fastcall TfrmContactPopup::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	if (entry)
	{
		entry->note = note->Text;
	}
}
//---------------------------------------------------------------------------

void TfrmContactPopup::InvalidateData(void)
{
	entry = NULL;
}

void __fastcall TfrmContactPopup::miRevertChangesClick(TObject *Sender)
{
    note->Text = initialNote;
}
//---------------------------------------------------------------------------

void __fastcall TfrmContactPopup::WndProc(Messages::TMessage &Message)
{
    if (Message.Msg == WM_NOTIFY)
    {
        // handle RichEdit link being clicked
        if (((LPNMHDR)Message.LParam)->code == EN_LINK)
        {
            ENLINK* p = (ENLINK *)Message.LParam;
            if (p->msg == WM_LBUTTONDOWN)
            {
				SendMessage(note->Handle, EM_EXSETSEL, 0, (LPARAM)&(p->chrg));
                ShellExecute(Handle, "open", note->SelText.c_str(), 0, 0, SW_SHOWNORMAL);
            }
        }
    }
	TForm::WndProc(Message);
}


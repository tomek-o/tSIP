//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContactEditor.h"
#include "Translate.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContactEditor *frmContactEditor = NULL;
//---------------------------------------------------------------------------

void TfrmContactEditor::TranslateForm(void* obj)
{
	TfrmContactEditor *frm = reinterpret_cast<TfrmContactEditor*>(obj);
	assert(frm);
	TRANSLATE_TMP("TfrmContactEditor.caption", frm->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblDescription", frm->lblDescription->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblCompany", frm->lblCompany->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblNumber1", frm->lblNumber1->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblNumber2", frm->lblNumber2->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblNumber3", frm->lblNumber3->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblNote", frm->lblNote->Caption);
	TRANSLATE_TMP("TfrmContactEditor.lblContactFile", frm->lblContactFile->Caption);

	TRANSLATE_TMP("TfrmContactEditor.btnApply", frm->btnApply->Caption);
	TRANSLATE_TMP("TfrmContactEditor.btnCancel", frm->btnCancel->Caption);
}

__fastcall TfrmContactEditor::TfrmContactEditor(TComponent* Owner)
	: TForm(Owner)
{
	RegisterTranslationCb(this, TranslateForm);

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
	entry->file = edFile->Text.Trim();

	if (storeNoteInSeparateFile == false)
	{
		entry->note = memoNote->Text;
	}
	else
	{
		if (noteChanged)
		{
			if (entry->file == "")
			{
				AnsiString msg;
				msg.sprintf("File for the note must be selected");
				MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
				return;
			}
			try
			{
				memoNote->Lines->SaveToFile(entry->file);
			}
			catch(...)
			{
				AnsiString msg;
				msg.sprintf("Could not save note to selected file");
				MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
				return;
			}
		}
	}
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

int __fastcall TfrmContactEditor::Start(Contacts::Entry *entry, bool storeNoteInSeparateFile)
{
	this->entry = entry;

	edDescription->Text = entry->description;
	edCompany->Text = entry->company;
	edNumber1->Text = entry->uri1;
	edNumber2->Text = entry->uri2;
	edNumber3->Text = entry->uri3;
	this->storeNoteInSeparateFile = storeNoteInSeparateFile;
	if (storeNoteInSeparateFile == false)
	{
		memoNote->Text = entry->note;
	}
	else
	{
		memoNote->Text = "";
		if (entry->file != "")
		{
			try
			{
				memoNote->Lines->LoadFromFile(entry->file);
			}
			catch(...)
			{
				AnsiString msg;
				msg.sprintf("Could not load note from file associated with contact");
				MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
			}
		}
	}
	noteChanged = false;
	edFile->Text = entry->file;

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

void __fastcall TfrmContactEditor::btnFileOpenClick(
      TObject *Sender)
{
	if (edFile->Text == "")
	{
		AnsiString msg = Translate2("TfrmContactEditor.msgFileNotSpecified", "Could not edit file, file is not specified");
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONINFORMATION);
	}
	else
	{
		ShellExecute(Handle, "open", edFile->Text.c_str(), 0, 0, SW_SHOWNORMAL);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContactEditor::btnFileSelectClick(TObject *Sender)
{
	openDialog->Filter = "";	
	if (FileExists(edFile->Text))
	{
		openDialog->FileName = edFile->Text;
	}
	else
	{
		openDialog->FileName = "";
		if (edFile->Text != "")
		{
			AnsiString dir = ExtractFileDir(edFile->Text);
			if (DirectoryExists(dir))
			{
            	openDialog->InitialDir = dir;
			}
		}
	}

	if (openDialog->Execute())
	{
    	edFile->Text = openDialog->FileName;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContactEditor::memoNoteChange(TObject *Sender)
{
	noteChanged = true;	
}
//---------------------------------------------------------------------------


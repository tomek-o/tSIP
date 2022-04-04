//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContacts.h"
#include "FormContactEditor.h"
#include "Contacts.h"
#include "SIMPLE_Messages.h"
#include "Translate.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContacts *frmContacts;
//---------------------------------------------------------------------------

namespace {

AnsiString singleItemCallText = "Call:";
AnsiString singleItemMessageText = "Message:";
AnsiString multiItemCallText = "Call...";
AnsiString multiItemMessageText = "Message...";
AnsiString noNumberUriText = "no number/URI";

}

void TfrmContacts::TranslateForm(void* obj)
{
	TfrmContacts *frm = reinterpret_cast<TfrmContacts*>(obj);
	assert(frm);
	TRANSLATE_TMP("TfrmContacts.miAdd", frm->miAdd->Caption);
	TRANSLATE_TMP("TfrmContacts.miEdit", frm->miEdit->Caption);
	TRANSLATE_TMP("TfrmContacts.miDelete", frm->miDelete->Caption);
	Translate("TfrmContacts.singleItemCallText", singleItemCallText);
	Translate("TfrmContacts.singleItemMessageText", singleItemMessageText);
	Translate("TfrmContacts.multiItemCallText", multiItemCallText);
	Translate("TfrmContacts.multiItemMessageText", multiItemMessageText);
	Translate("TfrmContacts.noNumberUriText", noNumberUriText);
}

__fastcall TfrmContacts::TfrmContacts(TComponent* Owner, Contacts *contacts, CallbackCall callbackCall)
	: TForm(Owner),
	contacts(contacts),
	callbackCall(callbackCall),
	filterUsingNote(false)
{
	RegisterTranslationCb(this, TranslateForm);
	assert(contacts);
	assert(callbackCall);
	contacts->addObserver(*this);

	FilterContacts();
}
//---------------------------------------------------------------------------

void TfrmContacts::FilterContacts(void)
{
	FilteredEntry fentry;
	AnsiString needle = UpperCase(edFilter->Text);
	filteredContacts.clear();
	std::vector<Contacts::Entry>& entries = contacts->GetEntries();
	if (needle == "")
	{
		for (int i=0; i<entries.size(); i++)
		{
			Contacts::Entry& entry = entries[i];
			fentry.id = i;
			fentry.entry = entry;
			filteredContacts.push_back(fentry);
		}
	}
	else
	{
		for (int i=0; i<entries.size(); i++)
		{
			Contacts::Entry& entry = entries[i];
			if (UpperCase(entry.description).Pos(needle) > 0 ||
				UpperCase(entry.company).Pos(needle) > 0 ||			
				UpperCase(entry.uri1).Pos(needle) > 0 ||
				UpperCase(entry.uri2).Pos(needle) > 0 ||
				UpperCase(entry.uri3).Pos(needle) > 0 ||
				(filterUsingNote?(UpperCase(entry.note).Pos(needle)):false)
				)
			{
				fentry.id = i;
				fentry.entry = entry;
				filteredContacts.push_back(fentry);
			}
		}
	}
	lvContacts->Items->Count = filteredContacts.size();
	lvContacts->Invalidate();
}

void TfrmContacts::obsUpdate(Observable* o, Argument * arg)
{
#if 0
	Contacts* contacts = dynamic_cast<Contacts*>(o);
	if (contacts)
	{
		//ContactsNotifyArgument* darg = dynamic_cast<ContactsNotifyArgument*>(arg);
		//assert(darg);
		const std::vector<Contacts::Entry>& entries = contacts->GetEntries();
		lvContacts->Items->Count = entries.size();
		lvContacts->Invalidate();
	}
#else
	FilterContacts();
#endif
}

void __fastcall TfrmContacts::lvContactsData(TObject *Sender, TListItem *Item)
{
	int id = Item->Index;
	Contacts::Entry &entry = filteredContacts[id].entry;
	Item->Caption = entry.description;
	//Item->SubItems->Add(entry.uri.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::lvContactsDblClick(TObject *Sender)
{
	TListItem *item = lvContacts->Selected;
	if (item == NULL)
	{
		return;
	}
	int id = item->Index;
	const Contacts::Entry &entry = filteredContacts[id].entry;
	callbackCall(entry.GetMainUri().c_str());
}
//---------------------------------------------------------------------------

TMenuItem* TfrmContacts::CreateCallItem(AnsiString uri, TPopupMenu *Owner)
{
	TMenuItem *item = new TMenuItem(Owner);
	item->Caption = uri;
	item->AutoHotkeys = maManual;
	item->OnClick = miCallItemClick;
	return item;
}

TMenuItem* TfrmContacts::CreateMessageItem(AnsiString uri, TPopupMenu *Owner)
{
	TMenuItem *item = new TMenuItem(Owner);
	item->Caption = uri;
	item->AutoHotkeys = maManual;
	item->OnClick = miMessageItemClick;
	return item;
}

void __fastcall TfrmContacts::miCallItemClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	if (item == NULL)
		return;
	callbackCall(item->Caption.c_str());
}

void __fastcall TfrmContacts::miMessageItemClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	if (item == NULL)
		return;
	SIMPLE_Messages::Send(item->Caption, "", false);
}

void __fastcall TfrmContacts::miCallSingleItemClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	if (item == NULL)
		return;
	AnsiString text = item->Caption;
	AnsiString needle = singleItemCallText + " ";
	if (strncmp(text.c_str(), needle.c_str(), needle.Length()) == 0)
	{
		callbackCall(text.c_str() + needle.Length());
	}
}

void __fastcall TfrmContacts::miMessageSingleItemClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	if (item == NULL)
		return;
	AnsiString text = item->Caption;
	AnsiString needle = singleItemMessageText + " ";
	if (strncmp(text.c_str(), needle.c_str(), needle.Length()) == 0)
	{
		SIMPLE_Messages::Send(text.c_str() + needle.Length(), "", false);
	}
}

void __fastcall TfrmContacts::popupContactListPopup(TObject *Sender)
{
	TListItem *item = lvContacts->Selected;
	miCall->Clear();
	miMessage->Clear();

	miCall->Visible = (item != NULL);
	miMessage->Visible = (item != NULL);
	miEdit->Visible = (item != NULL);
	miDelete->Visible = (item != NULL);

	if (item != NULL)
	{
		int id = filteredContacts[item->Index].id;		
		const Contacts::Entry &entry = contacts->GetEntries()[id];
		AnsiString uris[] = {entry.uri1, entry.uri2, entry.uri3};
		unsigned int validUriCount = 0;
		AnsiString validUri;
		for (unsigned int i=0; i<sizeof(uris)/sizeof(uris[0]); i++)
		{
			AnsiString uri = uris[i];
			if (uri != "")
			{
				validUriCount++;
				validUri = uri;
			}
		}
		miCall->Enabled = validUriCount;
		miMessage->Enabled = validUriCount;
		if (validUriCount == 0)
		{
			miCall->Caption = singleItemCallText + " " + noNumberUriText;
			miCall->OnClick = NULL;
			miMessage->Caption = singleItemMessageText + " " + noNumberUriText;
			miMessage->OnClick = NULL;
		}
		else if (validUriCount == 1)
		{
			miCall->Caption = singleItemCallText + " " + validUri;
			miCall->OnClick = miCallSingleItemClick;
			miMessage->Caption = singleItemMessageText + " " + validUri;
			miMessage->OnClick = miMessageSingleItemClick;
		}
		else
		{
			miCall->Caption = multiItemCallText;
			miCall->OnClick = NULL;
			miMessage->Caption = multiItemMessageText;
			miMessage->OnClick = NULL;
			for (unsigned int i=0; i<sizeof(uris)/sizeof(uris[0]); i++)
			{
				AnsiString uri = uris[i];
				if (uri != "")
				{
					TMenuItem *callItem = CreateCallItem(uri, popupContactList);
					miCall->Add(callItem);
					TMenuItem *messageItem = CreateMessageItem(uri, popupContactList);
					miMessage->Add(messageItem);
				}
			}
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::miAddClick(TObject *Sender)
{
    Contacts::Entry entry;
	frmContactEditor->Start(&entry, storeNoteInSeparateFile);
	if (frmContactEditor->isConfirmed())
	{
		contacts->GetEntries().push_back(entry);
		contacts->Sort();
		contacts->Update();
		contacts->Write();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::miEditClick(TObject *Sender)
{
	TListItem *item = lvContacts->Selected;
	if (item == NULL)
	{
		return;
	}
	int id = filteredContacts[item->Index].id;
	Contacts::Entry &entry = contacts->GetEntries()[id];
	frmContactEditor->Start(&entry, storeNoteInSeparateFile);
	if (frmContactEditor->isConfirmed())
	{
		contacts->Sort();
		contacts->Update();
		contacts->Write();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::miDeleteClick(TObject *Sender)
{
	TListItem *item = lvContacts->Selected;
	if (item == NULL)
	{
		return;
	}
	int id = filteredContacts[item->Index].id;
	std::vector<Contacts::Entry>& entries = contacts->GetEntries();
	entries.erase(entries.begin() + id);
	contacts->Update();
	contacts->Write();
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::edFilterChange(TObject *Sender)
{
	FilterContacts();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmContacts::edFilterKeyPress(TObject *Sender, char &Key)
{
	TListView *lv = lvContacts;
	if (Key == VK_RETURN)
	{
		if (lv->Items->Count > 0)
		{
			lv->Items->Item[0]->Selected = true;
			lv->Items->Item[0]->Focused = true;
			lv->SetFocus();
		}
    }	
}
//---------------------------------------------------------------------------

void TfrmContacts::Scale(int percentage)
{
	TListView *lv = lvContacts;
	for (int i=0; i<lv->Columns->Count; i++)
	{
        lv->Columns->Items[i]->Width = (float)lv->Columns->Items[i]->Width * percentage / 100.0f;
    }
}
void __fastcall TfrmContacts::lvContactsInfoTip(TObject *Sender,
      TListItem *Item, AnsiString &InfoTip)
{
	if (Item == NULL)
		return;
	int id = filteredContacts[Item->Index].id;
	Contacts::Entry &entry = contacts->GetEntries()[id];

	AnsiString tip;
	if (entry.company != "")
		tip.cat_printf("%s: %s\n", Translate2("TfrmContactEditor.lblCompany", "Company").c_str(), entry.company.c_str());
	if (entry.uri1 != "")
		tip.cat_printf("%s: %s\n", Translate2("TfrmContactEditor.lblNumber1", "Number #1").c_str(), entry.uri1.c_str());
	if (entry.uri2 != "")
		tip.cat_printf("%s: %s\n", Translate2("TfrmContactEditor.lblNumber2", "Number #2").c_str(), entry.uri2.c_str());
	if (entry.uri3 != "")
		tip.cat_printf("%s: %s\n", Translate2("TfrmContactEditor.lblNumber3", "Number #3").c_str(), entry.uri3.c_str());
	if (entry.note != "")
		tip.cat_printf("%s: %s", Translate2("TfrmContactEditor.lblNote", "Note").c_str(), entry.note.c_str());

	InfoTip = tip.Trim();
}
//---------------------------------------------------------------------------




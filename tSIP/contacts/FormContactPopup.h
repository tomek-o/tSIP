//---------------------------------------------------------------------------

#ifndef FormContactPopupH
#define FormContactPopupH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include "Contacts.h"
#include <Menus.hpp>
#include <ComCtrls.hpp>

class TfrmContactPopup : public TForm
{
__published:	// IDE-managed Components
	TPopupMenu *PopupMenu;
	TMenuItem *miRevertChanges;
	TRichEdit *note;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall miRevertChangesClick(TObject *Sender);
private:	// User declarations
	Contacts::Entry *entry;
	AnsiString initialNote;
	bool storeNoteInSeparateFile;
	void __fastcall WndProc(Messages::TMessage &Message);
public:		// User declarations
	void SetData(Contacts::Entry *entry, bool storeNoteInSeparateFile);
	__fastcall TfrmContactPopup(TComponent* Owner);
	bool isNoteModified(void);
	void InvalidateData(void);
	void AppendNoteText(AnsiString text);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmContactPopup *frmContactPopup;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#ifndef FormContactEditorH
#define FormContactEditorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include "Contacts.h"
#include <ComCtrls.hpp>

class TfrmContactEditor : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TLabel *lblDescription;
	TLabel *lblNumber1;
	TEdit *edDescription;
	TEdit *edNumber1;
	TLabel *lblNote;
	TRichEdit *memoNote;
	TLabel *lblNumber2;
	TEdit *edNumber2;
	TLabel *lblNumber3;
	TEdit *edNumber3;
	TLabel *lblCompany;
	TEdit *edCompany;
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:	// User declarations
	bool confirmed;
	Contacts::Entry *entry;
	void __fastcall WndProc(Messages::TMessage &Message);	
public:		// User declarations
	__fastcall TfrmContactEditor(TComponent* Owner);
	int __fastcall Start(Contacts::Entry *entry);
	bool isConfirmed(void) {
		return confirmed;
	}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmContactEditor *frmContactEditor;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#ifndef FormButtonGridEditH
#define FormButtonGridEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

class ProgrammableButtons;

class TfrmButtonGridEdit : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TComboBox *cbTargetTo;
	TLabel *lblTo;
	TLabel *lblColumns;
	TComboBox *cbColumns;
	TLabel *lblRows;
	TComboBox *cbRows;
	TLabel *lblFrom;
	TEdit *edSourceButton;
	TLabel *lblColumnSpacing;
	TEdit *edColumnSpacing;
	TLabel *lblRowSpacing;
	TEdit *edRowSpacing;
	TLabel *lblButtonOrder;
	TComboBox *cbButtonOrder;
	TLabel *lblDefaultWidth;
	TLabel *lblDefaultHeight;
	TCheckBox *chbCopyButtonWidth;
	TCheckBox *chbCopyButtonHeight;
	TComboBox *cbActionVisible;
	TComboBox *cbActionContainer;
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
private:	// User declarations
	ProgrammableButtons *buttons;
	int sourceBtnId;
	AnsiString GetButtonDescription(int id);
	int IncrementLocation(int &columnId, int &rowId, int columnCount, int rowCount);
public:		// User declarations
	__fastcall TfrmButtonGridEdit(TComponent* Owner);
	void SetButtons(ProgrammableButtons *buttons, int sourceBtnId);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonGridEdit *frmButtonGridEdit;
//---------------------------------------------------------------------------
#endif

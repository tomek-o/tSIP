//---------------------------------------------------------------------------

#ifndef FormButtonCopyH
#define FormButtonCopyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------

class ProgrammableButtons;

class TfrmButtonCopy : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TLabel *lblSource;
	TComboBox *cbSource;
	TLabel *lblTarget;
	TComboBox *cbTarget;
	TGroupBox *grboxCopy;
	TCheckBox *chbPosition;
	TCheckBox *chbCaption;
	TCheckBox *chbBehavior;
	TComboBox *cbTargetType;
	TCheckBox *chbFont;
	TCheckBox *chbSize;
	TCheckBox *chbColors;
	TCheckBox *chbVisible;
	TCheckBox *chbInactive;
	TCheckBox *chbImages;
	TCheckBox *chbFrame;
	TCheckBox *chbCenterTextHorizontally;
	TCheckBox *chbImageTransparent;
	TPopupMenu *popupCopy;
	TMenuItem *miSelectAll;
	TMenuItem *miDeselectAll;
	TCheckBox *chbImagePosition;
	TCheckBox *chbLabelPosition;
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbTargetTypeChange(TObject *Sender);
	void __fastcall miSelectAllClick(TObject *Sender);
	void __fastcall miDeselectAllClick(TObject *Sender);
private:	// User declarations
	ProgrammableButtons *buttons;
	void CopyButton(int src, int dst);
	void SetCheckboxes(bool state);
public:		// User declarations
	__fastcall TfrmButtonCopy(TComponent* Owner);
	void SetButtons(ProgrammableButtons *buttons);

};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonCopy *frmButtonCopy;
//---------------------------------------------------------------------------
#endif

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
	TCheckBox *chbBehavior;
	TComboBox *cbTargetType;
	TCheckBox *chbSize;
	TCheckBox *chbColors;
	TCheckBox *chbVisible;
	TCheckBox *chbInactive;
	TCheckBox *chbImages;
	TCheckBox *chbFrame;
	TCheckBox *chbImageTransparent;
	TPopupMenu *popupCopy;
	TMenuItem *miSelectAll;
	TMenuItem *miDeselectAll;
	TCheckBox *chbImagePosition;
	TLabel *lblFrom;
	TLabel *lblTo;
	TComboBox *cbTargetTo;
	TLabel *lblDirection;
	TCheckBox *chbParentContainer;
	TGroupBox *grboxCaption1;
	TCheckBox *chbCaption1;
	TCheckBox *chbFont1;
	TCheckBox *chbLabelPosition1;
	TCheckBox *chbCenterTextHorizontally1;
	TCheckBox *chbNumberOfCaptionLines;
	TCheckBox *chbSpaceLabelsYEqually;
	TCheckBox *chbCenterTextVertically1;
	TGroupBox *grboxCaption2;
	TCheckBox *chbCaption2;
	TCheckBox *chbFont2;
	TCheckBox *chbLabelPosition2;
	TCheckBox *chbCenterTextHorizontally2;
	TButton *btnSelectAll;
	TButton *btnDeselectAll;
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbTargetTypeChange(TObject *Sender);
	void __fastcall miSelectAllClick(TObject *Sender);
	void __fastcall miDeselectAllClick(TObject *Sender);
	void __fastcall btnSelectAllClick(TObject *Sender);
	void __fastcall btnDeselectAllClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:	// User declarations
	ProgrammableButtons *buttons;
	bool applied;
	void CopyButton(int src, int dst);
	void SetCheckboxes(bool state);
public:		// User declarations
	__fastcall TfrmButtonCopy(TComponent* Owner);
	void SetButtons(ProgrammableButtons *buttons, int sourceBtnId);
	bool IsApplied(void) const
	{
		return applied;
	}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonCopy *frmButtonCopy;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#ifndef FormButtonContainerConfH
#define FormButtonContainerConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------

class ButtonContainerConf;

class TfrmButtonContainerConf : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TLabel *lblBackgroundBitmap;
	TEdit *edBackgroundBitmap;
	TButton *btnSelectBackgroundBitmap;
	TOpenDialog *openDialog;
	TLabel *lblBackgroundColor;
	TComboBox *cbBackgroundColor;
	TButton *btnSelectBackgroundColor;
	TShape *shColorBackground;
	TLabel *lblColorPreview;
	TColorDialog *colorDialog;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnSelectBackgroundBitmapClick(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbBackgroundColorChange(TObject *Sender);
	void __fastcall btnSelectBackgroundColorClick(TObject *Sender);
private:	// User declarations
	bool confirmed;
	ButtonContainerConf *cfg;
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};
	void UpdateColorsPreview(void);
public:		// User declarations
	__fastcall TfrmButtonContainerConf(TComponent* Owner);
	void __fastcall ShowModal(ButtonContainerConf *cfg);
	bool isConfirmed(void) {
		return confirmed;
	}	
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonContainerConf *frmButtonContainerConf;
//---------------------------------------------------------------------------
#endif

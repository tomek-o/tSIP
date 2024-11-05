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

#include "ButtonContainerConf.h"

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
	TCheckBox *chbBackgroundImageTransparent;
	TLabel *lblContainer;
	TLabel *lblContainerName;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnSelectBackgroundBitmapClick(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbBackgroundColorChange(TObject *Sender);
	void __fastcall btnSelectBackgroundColorClick(TObject *Sender);
private:	// User declarations
	bool confirmed;
	ButtonContainerConf *config;
	ButtonContainerConf tmpConfig;
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};
	void UpdateColorsPreview(void);
public:		// User declarations
	__fastcall TfrmButtonContainerConf(TComponent* Owner);
	void __fastcall ShowModal(int containerId, ButtonContainerConf *cfg);
	bool isConfirmed(void) {
		return confirmed;
	}	
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonContainerConf *frmButtonContainerConf;
//---------------------------------------------------------------------------
#endif

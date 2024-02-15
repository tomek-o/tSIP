//---------------------------------------------------------------------------

#ifndef FormButtonContainerConfH
#define FormButtonContainerConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

class ButtonContainerConf;

class TfrmButtonContainerConf : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	void __fastcall FormShow(TObject *Sender);
private:	// User declarations
	bool confirmed;
	ButtonContainerConf *cfg;
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};	
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

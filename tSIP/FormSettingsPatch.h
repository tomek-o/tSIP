//---------------------------------------------------------------------------

#ifndef FormSettingsPatchH
#define FormSettingsPatchH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmSettingsPatch : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnApply;
	TMemo *edJson;
	TPanel *pnlTop;
	TLabel *lblInfo;
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:	// User declarations
	typedef int (__closure *CallbackUpdateSettings)(AnsiString json);
public:		// User declarations
	__fastcall TfrmSettingsPatch(TComponent* Owner);
	CallbackUpdateSettings onUpdateSettings;
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmSettingsPatch *frmSettingsPatch;
//---------------------------------------------------------------------------
#endif

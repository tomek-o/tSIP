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

class TfrmTextEditor;

class TfrmSettingsPatch : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnApply;
	TPanel *pnlTop;
	TLabel *lblInfo;
	TLabel *Label1;
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:	// User declarations
	typedef int (__closure *CallbackUpdateSettings)(AnsiString json);
	void __fastcall WmDropFiles(TWMDropFiles& Message);
	TfrmTextEditor *frmEditor;
public:		// User declarations
	__fastcall TfrmSettingsPatch(TComponent* Owner);
	CallbackUpdateSettings onUpdateSettings;
BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_DROPFILES, TWMDropFiles, WmDropFiles)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmSettingsPatch *frmSettingsPatch;
//---------------------------------------------------------------------------
#endif

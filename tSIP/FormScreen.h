//---------------------------------------------------------------------------

#ifndef FormScreenH
#define FormScreenH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmScreen : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlSpeedDial;
private:	// User declarations
	void __fastcall CreateParams(TCreateParams &Params);
public:		// User declarations
	__fastcall TfrmScreen(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmScreen *frmScreen;
//---------------------------------------------------------------------------
#endif

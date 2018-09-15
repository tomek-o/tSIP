//---------------------------------------------------------------------------

#ifndef HistoryUnitH
#define HistoryUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmHistory : public TForm
{
__published:	// IDE-managed Components
	TListView *lvHistory;
private:	// User declarations
public:		// User declarations
	__fastcall TfrmHistory(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmHistory *frmHistory;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#ifndef FormScriptPluginVariablesH
#define FormScriptPluginVariablesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#include <vector>

class TfrmScriptPluginVariables : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TListView *lvVariables;
	TButton *btnRead;
	TCheckBox *chbAutoRefresh;
	TTimer *tmrAutoRefresh;
	void __fastcall lvVariablesData(TObject *Sender, TListItem *Item);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnReadClick(TObject *Sender);
	void __fastcall tmrAutoRefreshTimer(TObject *Sender);
	void __fastcall chbAutoRefreshClick(TObject *Sender);
	void __fastcall chbAutoRefreshMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
private:	// User declarations
	struct Var
	{
		AnsiString name;
		AnsiString value;
	};
	std::vector<Var> variables;
	void Read(void);
public:		// User declarations
	__fastcall TfrmScriptPluginVariables(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmScriptPluginVariables *frmScriptPluginVariables;
//---------------------------------------------------------------------------
#endif

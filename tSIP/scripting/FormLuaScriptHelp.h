//---------------------------------------------------------------------------

#ifndef FormLuaScriptHelpH
#define FormLuaScriptHelpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#include "ScriptExec.h"
#include <Graphics.hpp>
#include <vector>

class TfrmLuaScriptHelp : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TListView *lvSymbols;
	TPanel *pnlBottom2;
	TImage *imgFilter;
	TEdit *edFilter;
	TRichEdit *memoDescription;
	TSplitter *Splitter1;
	void __fastcall lvSymbolsData(TObject *Sender, TListItem *Item);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall edFilterChange(TObject *Sender);
	void __fastcall lvSymbolsSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
private:	// User declarations
	std::vector<ScriptExec::Symbol> entries;
	void Filter(void);
public:		// User declarations
	__fastcall TfrmLuaScriptHelp(TComponent* Owner);
	typedef int (__closure *CallbackRunScript)(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled);
	static void SetCallbackRunScript(CallbackRunScript cb);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmLuaScriptHelp *frmLuaScriptHelp;
//---------------------------------------------------------------------------
#endif

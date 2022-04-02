//---------------------------------------------------------------------------

#ifndef FormHotkeysH
#define FormHotkeysH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
#include <list>
#include <vector>

class THotkeyCfgPanel;
struct HotKeyConf;

class TfrmHotkeys : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnAddHotkey;
	TScrollBox *ScrollBox1;
	TFlowPanel *panel;
	TPanel *pnlTop;
	TLabel *lblHeader;
	TTimer *timerRemove;
	void __fastcall btnAddHotkeyClick(TObject *Sender);
	void __fastcall btnRemoveClick(TObject *Sender);
	void __fastcall timerRemoveTimer(TObject *Sender);
private:	// User declarations
	std::list<THotkeyCfgPanel*> hkCfgPanels;
	THotkeyCfgPanel* removedPanel;
	std::list<HotKeyConf> *cfg;
public:		// User declarations
	__fastcall TfrmHotkeys(TComponent* Owner);
	void SetCfg(std::list<HotKeyConf> *hotkeyCfg);
};
//---------------------------------------------------------------------------
//extern PACKAGE TfrmHotkeys *frmHotkeys;
//---------------------------------------------------------------------------
#endif

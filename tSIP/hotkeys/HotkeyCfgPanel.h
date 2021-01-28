//---------------------------------------------------------------------------

#ifndef HotkeyCfgPanelH
#define HotkeyCfgPanelH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

struct HotKeyConf;

class PACKAGE THotkeyCfgPanel : public TPanel
{
private:
	HotKeyConf &cfg;
	void __fastcall UpdateCfg(TObject *Sender);
	void UpdateActionTypeView(void);
	int programmableButtonsCnt;
protected:
public:
	__fastcall THotkeyCfgPanel(TComponent* Owner, HotKeyConf &cfg, int programmableButtonsCnt);
	void Start(void);
__published:
	TCheckBox *chbShift;
	TCheckBox *chbAlt;
	TCheckBox *chbCtrl;
	TComboBox *cbKey;
	TComboBox *cbAction;
	TComboBox *cbId;
	TCheckBox *chbGlobal;
	TButton	  *btnRemove;
};
//---------------------------------------------------------------------------
#endif

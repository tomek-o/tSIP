//---------------------------------------------------------------------------

#ifndef FormPhonesH
#define FormPhonesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#include <list>

struct PhoneConf;

class TfrmPhones : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnToggleActivation;
	TListView *lvDlls;
	TButton *btnRefreshList;
	void __fastcall lvDllsData(TObject *Sender, TListItem *Item);
	void __fastcall btnToggleActivationClick(TObject *Sender);
	void __fastcall btnRefreshListClick(TObject *Sender);
	void __fastcall lvDllsDblClick(TObject *Sender);
private:	// User declarations
	std::list<PhoneConf> *cfg;
	PhoneConf* FindCfg(AnsiString dllName);
	void ToggleActivation(void);
public:		// User declarations
	__fastcall TfrmPhones(TComponent* Owner);
	void SetCfg(std::list<PhoneConf> *hotkeyCfg);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmPhones *frmPhones;
//---------------------------------------------------------------------------
#endif

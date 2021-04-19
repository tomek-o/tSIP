//---------------------------------------------------------------------------

#ifndef FormDialpadConfH
#define FormDialpadConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------

struct DialpadCfg;

class TfrmDialpadConf : public TForm
{
__published:	// IDE-managed Components
private:	// User declarations
	DialpadCfg *cfg;
public:		// User declarations
	__fastcall TfrmDialpadConf(TComponent* Owner);
	void SetCfg(DialpadCfg *cfg);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmDialpadConf *frmDialpadConf;
//---------------------------------------------------------------------------
#endif

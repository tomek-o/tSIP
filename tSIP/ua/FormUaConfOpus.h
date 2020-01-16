//---------------------------------------------------------------------------

#ifndef FormUaConfOpusH
#define FormUaConfOpusH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------

#include "UaConf.h"

class TfrmUaConfOpus : public TForm
{
__published:	// IDE-managed Components
	TCheckBox *chbStereo;
	TCheckBox *chbSpropStereo;
	TCheckBox *chbSetBitrate;
	TEdit *edBitrate;
	TCheckBox *chbSetSamplerate;
	TComboBox *cbSamplerate;
	TCheckBox *chbSetCbr;
	TCheckBox *chbCbr;
	TCheckBox *chbSetInbandFec;
	TCheckBox *chbInbandFec;
	TCheckBox *chbSetDtx;
	TCheckBox *chbDtx;
	TCheckBox *chbMirror;
	TLabel *lblComplexity;
	TComboBox *cbComplexity;
	TCheckBox *chbSetApplication;
	TComboBox *cbApplication;
	TCheckBox *chbSetPacketLoss;
	TEdit *edPacketLoss;
private:	// User declarations
	UaConf::Opus* opus;
public:		// User declarations
	__fastcall TfrmUaConfOpus(TComponent* Owner);
	void SetCfg(UaConf::Opus* opus);
	void Apply(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmUaConfOpus *frmUaConfOpus;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#ifndef FormDialpadConfH
#define FormDialpadConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------

struct DialpadConf;

class TfrmDialpadConf : public TForm
{
__published:	// IDE-managed Components
	TLabel *lblTransferEdit;
	TCheckBox *chbTransferEditVisible;
	TLabel *lblTransferEditLeft;
	TEdit *edTransferEditLeft;
	TLabel *lblEdTransferEditTop;
	TEdit *edTransferEditTop;
	TLabel *lblTransferEditWidth;
	TEdit *edTransferEditWidth;
	TLabel *lblTransferEditHeight;
	TEdit *edTransferEditHeight;
	TLabel *lblMicVolumeTrbar;
	TCheckBox *chbMicVolumeTrbarVisible;
	TLabel *lblMicVolumeTrbarLeft;
	TEdit *edMicVolumeTrbarLeft;
	TLabel *lblMicVolumeTrbarTop;
	TEdit *edMicVolumeTrbarTop;
	TLabel *lblMicVolumeTrbarWidth;
	TEdit *edMicVolumeTrbarWidth;
	TLabel *lblMicVolumeTrbarHeight;
	TEdit *edMicVolumeTrbarHeight;
	TLabel *lblMicVolumeResetBtn;
	TCheckBox *chbMicVolumeResetBtnVisible;
	TLabel *lblMicVolumeResetBtnLeft;
	TEdit *edMicVolumeResetBtnLeft;
	TLabel *lblMicVolumeResetBtnTop;
	TEdit *edMicVolumeResetBtnTop;
	TLabel *lblMicVolumeResetBtnWidth;
	TEdit *edMicVolumeResetBtnWidth;
	TLabel *lblMicVolumeResetBtnHeight;
	TEdit *edMicVolumeResetBtnHeight;
	TLabel *lblSpeakerVolumeTrbar;
	TCheckBox *chbSpeakerVolumeTrbarVisible;
	TLabel *lblSpeakerVolumeTrbarLeft;
	TEdit *edSpeakerVolumeTrbarLeft;
	TLabel *lblSpeakerVolumeTrbarTop;
	TEdit *edSpeakerVolumeTrbarTop;
	TLabel *lblSpeakerVolumeTrbarWidth;
	TEdit *edSpeakerVolumeTrbarWidth;
	TLabel *lblSpeakerVolumeTrbarHeight;
	TEdit *edSpeakerVolumeTrbarHeight;
	TLabel *lblSpeakerVolumeResetBtn;
	TCheckBox *chbSpeakerVolumeResetBtnVisible;
	TLabel *lblSpeakerVolumeResetBtnLeft;
	TEdit *edSpeakerVolumeResetBtnLeft;
	TLabel *lblSpeakerVolumeResetBtnTop;
	TEdit *edSpeakerVolumeResetBtnTop;
	TLabel *lblSpeakerVolumeResetBtnWidth;
	TEdit *edSpeakerVolumeResetBtnWidth;
	TLabel *lblSpeakerVolumeResetBtnHeight;
	TEdit *edSpeakerVolumeResetBtnHeight;
private:	// User declarations
	DialpadConf *cfg;
public:		// User declarations
	__fastcall TfrmDialpadConf(TComponent* Owner);
	void SetCfg(DialpadConf *cfg);
	void Apply(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmDialpadConf *frmDialpadConf;
//---------------------------------------------------------------------------
#endif

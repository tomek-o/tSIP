//---------------------------------------------------------------------------

#ifndef FormVideoConfH
#define FormVideoConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------

struct VideoConf;
struct UaConf;

class TfrmVideoConf : public TForm
{
__published:	// IDE-managed Components
	TCheckBox *chbEnabled;
	TLabel *lblDisplayParentType;
	TComboBox *cbDisplayParentType;
	TLabel *lblDisplayParentId;
	TEdit *edDisplayParentId;
	TCheckBox *chbSelfviewEnable;
	TCheckBox *chbSelfviewPip;
	TComboBox *cbInputDev;
	TLabel *lblInputDevice;
	TLabel *lblInputModule;
	TLabel *lblAudioInput;
	TComboBox *cbInputMod;
	TEdit *edInputFile;
	TButton *btnSelectInputFile;
	TLabel *lblTransmittedVideo;
	TLabel *lblWidth;
	TEdit *edWidth;
	TLabel *lblHeight;
	TEdit *edHeight;
	TLabel *lblBitrate;
	TEdit *edBitrate;
	TLabel *lblFps;
	TEdit *edFps;
	TCheckBox *chbDshowSkipReadingBackMediaFormat;
	TLabel *lblDisplayModule;
	TComboBox *cbOutputMod;
	TOpenDialog *dlgOpenDeviceFile;
	void __fastcall cbInputModChange(TObject *Sender);
	void __fastcall cbOutputModChange(TObject *Sender);
	void __fastcall btnSelectInputFileClick(TObject *Sender);
private:	// User declarations
	VideoConf *cfg;
	UaConf *uaCfg;
public:		// User declarations
	__fastcall TfrmVideoConf(TComponent* Owner);
	void SetCfg(VideoConf *cfg, UaConf *uaCfg);
	void Apply(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmVideoConf *frmVideoConf;
//---------------------------------------------------------------------------
#endif

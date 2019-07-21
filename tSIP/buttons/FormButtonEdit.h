//---------------------------------------------------------------------------

#ifndef FormButtonEditH
#define FormButtonEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include "ButtonType.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
class ButtonConf;

class TfrmButtonEdit : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TLabel *lblCaption;
	TEdit *edDescription;
	TLabel *lblNumber;
	TEdit *edNumber;
	TComboBox *cbType;
	TLabel *lblType;
	TMemo *memoHelp;
	TOpenDialog *openDialog;
	TButton *btnClear;
	TPageControl *pageControl;
	TTabSheet *tsVisual;
	TTabSheet *tsBehavior;
	TLabel *lblHeight;
	TEdit *edHeight;
	TLabel *lblMarginTop;
	TEdit *edMarginTop;
	TLabel *lblMarginBottom;
	TEdit *edMarginBottom;
	TGroupBox *grboxImage;
	TLabel *lblImgIdle;
	TLabel *lblImgTerminated;
	TLabel *lblImgEarly;
	TLabel *lblImgConfirmed;
	TCheckBox *chbNoIcon;
	TEdit *edImgIdle;
	TEdit *edImgTerminated;
	TEdit *edImgEarly;
	TEdit *edImgConfirmed;
	TButton *btnSelectImgTerminated;
	TButton *btnSelectImgEarly;
	TButton *btnSelectImgConfirmed;
	TButton *btnSelectImgIdle;
	TColorDialog *colorDialog;
	TLabel *lblBackgroundColor;
	TComboBox *cbBackgroundColor;
	TButton *btnSelectBackgroundColor;
	TPageControl *pcBehavior;
	TTabSheet *tsBehaviorBlf;
	TTabSheet *tsBehaviorPaging;
	TTabSheet *tsBehaviorScript;
	TGroupBox *grboxBlfOverride;
	TCheckBox *chbBlfOverrideIdle;
	TEdit *edBlfOverrideIdle;
	TCheckBox *chbBlfOverrideTerminated;
	TEdit *edBlfOverrideTerminated;
	TCheckBox *chbBlfOverrideEarly;
	TEdit *edBlfOverrideEarly;
	TCheckBox *chbBlfOverrideConfirmed;
	TEdit *edBlfOverrideConfirmed;
	TLabel *lblBlfInCallAction;
	TComboBox *cbBlfActionDuringCall;
	TLabel *lblBlfDtmfPrefixDuringCall;
	TEdit *edBlfDtmfPrefixDuringCall;
	TLabel *lblPagingTxWaveFile;
	TEdit *edPagingTxWaveFile;
	TButton *btnSelectWave;
	TLabel *lblScriptFile;
	TEdit *edScriptFile;
	TButton *btnSelectScript;
	TTabSheet *tsBehaviorExecute;
	TLabel *lblArg1;
	TEdit *edArg1;
	TLabel *lblPagingTxCodec;
	TComboBox *cbPagingTxCodec;
	TLabel *lblPagingTxPtime;
	TComboBox *cbPagingTxPtime;
	TTabSheet *tsBehaviorSwitchAudioSource;
	TTabSheet *tsBehaviorSwitchAudioPlayer;
	TLabel *lblSoundInputModule;
	TComboBox *cbSoundInputMod;
	TLabel *lblSoundInputDevice;
	TEdit *edSoundInputWave;
	TButton *btnSelectWaveFile;
	TLabel *Label3;
	TComboBox *cbSoundOutputMod;
	TLabel *lblSoundOutputDev;
	TComboBox *cbSoundOutputDev;
	TComboBox *cbSoundInputDev;
	TOpenDialog *dlgOpenRing;
	TLabel *Label1;
	TEdit *edDescription2;
	TComboBox *cbCaptionLines;
	TLabel *lblCaptionLines;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbTypeChange(TObject *Sender);
	void __fastcall SelectImgClick(TObject *Sender);
	void __fastcall btnClearClick(TObject *Sender);
	void __fastcall SelectColorClick(TObject *Sender);
	void __fastcall cbBackgroundColorChange(TObject *Sender);
	void __fastcall btnSelectWaveClick(TObject *Sender);
	void __fastcall btnSelectScriptClick(TObject *Sender);
	void __fastcall cbSoundInputModChange(TObject *Sender);
	void __fastcall btnSelectWaveFileClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall cbSoundOutputModChange(TObject *Sender);
private:	// User declarations
	bool confirmed;
	bool refreshAudioDevList;
	ButtonConf *cfg;
	void SetType(Button::Type type);
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};
	void ApplyConf(void);
public:		// User declarations
	__fastcall TfrmButtonEdit(TComponent* Owner);
	void __fastcall ShowModal(ButtonConf *cfg);
	bool isConfirmed(void) {
		return confirmed;
	}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonEdit *frmButtonEdit;
//---------------------------------------------------------------------------
#endif

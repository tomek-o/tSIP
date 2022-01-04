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
#include <Buttons.hpp>
class ButtonConf;

class TfrmButtonEdit : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TButton *btnCancel;
	TButton *btnApply;
	TLabel *lblNumber;
	TEdit *edNumber;
	TComboBox *cbType;
	TLabel *lblType;
	TMemo *memoHelp;
	TOpenDialog *openDialog;
	TButton *btnClear;
	TPageControl *pageControl;
	TTabSheet *tsBehavior;
	TColorDialog *colorDialog;
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
	TBitBtn *btnSelectedScriptOnButtonEdit;
	TTabSheet *tsBehaviorAutoAnswer;
	TLabel *lblAutoAnswerSipCode;
	TEdit *edAutoAnswerSipCode;
	TComboBox *cbCaptionLines;
	TLabel *lblCaption;
	TEdit *edCaption;
	TLabel *lblCaption2;
	TEdit *edCaption2;
	TFontDialog *fontDialog;
	TTabSheet *tsVisual;
	TCheckBox *chbVisible;
	TCheckBox *chbDown;
	TCheckBox *chbInactive;
	TCheckBox *chbCustomFrame;
	TLabel *lblBevelWidth;
	TEdit *edBevelWidth;
	TEdit *edHeight;
	TLabel *lblHeight;
	TEdit *edWidth;
	TLabel *lblWidth;
	TEdit *edTop;
	TEdit *edLeft;
	TLabel *lblLeft;
	TLabel *lblTop;
	TGroupBox *grboxCaption1;
	TLabel *lblSpeedDialFont;
	TLabel *lblLabelTop;
	TEdit *edSpeedDialFont;
	TButton *btnSpeedDialFontSelect;
	TCheckBox *chbLabelCenterHorizontally;
	TEdit *edLabelTop;
	TCheckBox *chbLabelCenterVertically;
	TGroupBox *grboxCaption2;
	TLabel *Label1;
	TLabel *lblLabel2Top;
	TLabel *Label6;
	TEdit *edCaption2Font;
	TButton *btnCaption2FontSelect;
	TEdit *edLabel2Top;
	TEdit *edLabel2Left;
	TCheckBox *chbLabel2CenterHorizontally;
	TGroupBox *grboxColors;
	TLabel *lblInactive;
	TLabel *lblColorIdle;
	TLabel *lblColorDown;
	TLabel *lblColorDownPressed;
	TLabel *lblColorInactiveDown;
	TComboBox *cbInactiveColor;
	TButton *btnSelectInactiveColor;
	TComboBox *cbIdleColor;
	TButton *btnSelectIdleColor;
	TComboBox *cbDownColor;
	TButton *btnSelectDownColor;
	TComboBox *cbDownPressedColor;
	TButton *btnSelectDownPressedColor;
	TComboBox *cbColorElement;
	TComboBox *cbInactiveDownColor;
	TButton *btnSelectInactiveDownColor;
	TGroupBox *grboxImage;
	TLabel *lblImgIdle;
	TLabel *lblImgTerminated;
	TLabel *lblImgEarly;
	TLabel *lblImgConfirmed;
	TLabel *lblImageLeft;
	TLabel *lblImageTop;
	TEdit *edImgIdle;
	TEdit *edImgTerminated;
	TEdit *edImgEarly;
	TEdit *edImgConfirmed;
	TButton *btnSelectImgTerminated;
	TButton *btnSelectImgEarly;
	TButton *btnSelectImgConfirmed;
	TButton *btnSelectImgIdle;
	TEdit *edImageLeft;
	TCheckBox *chbImageTransparent;
	TEdit *edImageTop;
	TCheckBox *chbImageCenterVertically;
	TCheckBox *chbSpaceLabelsYEqually;
	TLabel *lblLabelLeft;
	TEdit *edLabelLeft;
	TLabel *lblParent;
	TComboBox *cbParentId;
	TLabel *lblBlfExpires;
	TEdit *edBlfExpires;
	TLabel *lblAutoAnswerSipReason;
	TEdit *edAutoAnswerSipReason;
	TLabel *lblAutoAnswerSipReasonInfo;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnApplyClick(TObject *Sender);
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall cbTypeChange(TObject *Sender);
	void __fastcall SelectImgClick(TObject *Sender);
	void __fastcall btnClearClick(TObject *Sender);
	void __fastcall SelectColorClick(TObject *Sender);
	void __fastcall cbColorChange(TObject *Sender);
	void __fastcall btnSelectWaveClick(TObject *Sender);
	void __fastcall btnSelectScriptClick(TObject *Sender);
	void __fastcall cbSoundInputModChange(TObject *Sender);
	void __fastcall btnSelectWaveFileClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
	void __fastcall cbSoundOutputModChange(TObject *Sender);
	void __fastcall btnSelectedScriptOnButtonEditClick(TObject *Sender);
	void __fastcall btnSpeedDialFontSelectClick(TObject *Sender);
	void __fastcall cbColorElementChange(TObject *Sender);
	void __fastcall chbImageCenterVerticallyClick(TObject *Sender);
	void __fastcall chbLabelCenterVerticallyClick(TObject *Sender);
	void __fastcall chbLabel2CenterHorizontallyClick(TObject *Sender);
	void __fastcall chbSpaceLabelsYEquallyClick(TObject *Sender);
private:	// User declarations
	bool confirmed;
	bool refreshAudioDevList;
	ButtonConf *cfg;
	int btnId;
	void SetType(Button::Type type);
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};
	void ApplyConf(void);
	void UpdateColorView(void);
	void UpdateColors(void);
	void UpdateLabelsTopVisibility(void);
public:		// User declarations
	__fastcall TfrmButtonEdit(TComponent* Owner);
	void __fastcall ShowModal(ButtonConf *cfg, int btnId);
	bool isConfirmed(void) {
		return confirmed;
	}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonEdit *frmButtonEdit;
//---------------------------------------------------------------------------
#endif

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
	TLabel *lblSoundOutputMod;
	TComboBox *cbSoundOutputMod;
	TLabel *lblSoundOutputDev;
	TComboBox *cbSoundOutputDev;
	TComboBox *cbSoundInputDev;
	TOpenDialog *dlgOpenRing;
	TBitBtn *btnSelectedScriptOnButtonEdit;
	TTabSheet *tsBehaviorAutoAnswer;
	TLabel *lblAutoAnswerSipCode;
	TEdit *edAutoAnswerSipCode;
	TFontDialog *fontDialog;
	TTabSheet *tsColors;
	TLabel *lblBlfExpires;
	TEdit *edBlfExpires;
	TLabel *lblAutoAnswerSipReason;
	TEdit *edAutoAnswerSipReason;
	TLabel *lblAutoAnswerSipReasonInfo;
	TTabSheet *tsBehaviorSwitchVideoSource;
	TLabel *lblVideoInputModule;
	TLabel *lblVideoInputDevice;
	TEdit *edVideoInputFile;
	TComboBox *cbVideoInputMod;
	TButton *btnSelectVideoInputFile;
	TOpenDialog *dlgOpenDeviceFile;
	TComboBox *cbVideoInputDev;
	TLabel *lblNoVideo;
	TTreeView *tvSelector;
	TTabSheet *tsGeneral;
	TComboBox *cbCaptionLines;
	TLabel *lblLeft;
	TEdit *edLeft;
	TEdit *edTop;
	TCheckBox *chbDown;
	TLabel *lblTop;
	TLabel *lblWidth;
	TCheckBox *chbInactive;
	TEdit *edWidth;
	TLabel *lblHeight;
	TCheckBox *chbCustomFrame;
	TEdit *edHeight;
	TLabel *lblBevelWidth;
	TEdit *edBevelWidth;
	TGroupBox *grboxCaption1;
	TLabel *lblSpeedDialFont;
	TLabel *lblLabelTop;
	TLabel *lblLabelLeft;
	TEdit *edSpeedDialFont;
	TButton *btnSpeedDialFontSelect;
	TCheckBox *chbLabelCenterHorizontally;
	TEdit *edLabelTop;
	TCheckBox *chbLabelCenterVertically;
	TEdit *edLabelLeft;
	TGroupBox *grboxCaption2;
	TLabel *Label1;
	TLabel *lblLabel2Top;
	TLabel *Label6;
	TEdit *edCaption2Font;
	TButton *btnCaption2FontSelect;
	TEdit *edLabel2Top;
	TEdit *edLabel2Left;
	TCheckBox *chbLabel2CenterHorizontally;
	TCheckBox *chbSpaceLabelsYEqually;
	TLabel *lblCaption;
	TMemo *memoCaption1;
	TComboBox *cbColorElement;
	TLabel *lblInactive;
	TComboBox *cbInactiveColor;
	TButton *btnSelectInactiveColor;
	TLabel *lblColorDown;
	TComboBox *cbDownColor;
	TButton *btnSelectDownColor;
	TLabel *lblElements;
	TLabel *lblColorIdle;
	TLabel *lblColorInactiveDown;
	TLabel *lblColorDownPressed;
	TComboBox *cbDownPressedColor;
	TComboBox *cbInactiveDownColor;
	TComboBox *cbIdleColor;
	TButton *btnSelectIdleColor;
	TButton *btnSelectInactiveDownColor;
	TButton *btnSelectDownPressedColor;
	TTabSheet *tsBitmaps;
	TLabel *Label2;
	TLabel *lblImgIdle;
	TLabel *lblImgTerminated;
	TLabel *lblImgEarly;
	TLabel *lblImgConfirmed;
	TEdit *edImgConfirmed;
	TEdit *edImgEarly;
	TEdit *edImgTerminated;
	TEdit *edImgIdle;
	TButton *btnSelectImgIdle;
	TButton *btnSelectImgTerminated;
	TButton *btnSelectImgEarly;
	TButton *btnSelectImgConfirmed;
	TLabel *lblImageLeft;
	TLabel *lblImageTop;
	TEdit *edImageLeft;
	TEdit *edImageTop;
	TCheckBox *chbImageCenterVertically;
	TCheckBox *chbImageTransparent;
	TLabel *lblCaption2;
	TMemo *memoCaption2;
	TPanel *pnlTop;
	TLabel *lblType;
	TComboBox *cbType;
	TMemo *memoHelp;
	TCheckBox *chbVisible;
	TComboBox *cbParentId;
	TLabel *lblNumber;
	TEdit *edNumber;
	TShape *shColorInactive;
	TShape *shColorDown;
	TShape *shColorIdle;
	TShape *shColorInactiveDown;
	TShape *shColorDownPressed;
	TLabel *lblColorPreview;
	TImage *imgIdle;
	TImage *imgTerminated;
	TImage *imgEarly;
	TImage *imgConfirmed;
	TLabel *Label3;
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
	void __fastcall cbVideoInputModChange(TObject *Sender);
	void __fastcall tvSelectorChange(TObject *Sender, TTreeNode *Node);
private:	// User declarations
	bool confirmed;
	bool refreshAudioDevList;
	bool refreshVideoDevList;
	ButtonConf *cfg;
	int btnId;
	void SetType(Button::Type type);
	__fastcall int ShowModal(void) {
		return TForm::ShowModal();
	};
	void ApplyConf(void);
	void UpdateColorView(void);
	void UpdateColors(void);
	void UpdateColorsPreview(void);
	void UpdateLabelsTopVisibility(void);
	void UpdateImgPreview(void);
	TTabSheet *lastTab;
	TTreeNode* CreatePagesNode(TTreeNode *parent, TTabSheet *tab);
	void CreatePages(void);
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

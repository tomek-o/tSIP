//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonEdit.h"
#include "FormLuaScript.h"
#include "ButtonType.h"
#include "ButtonConf.h"
#include "ProgrammableButtons.h"
#include "ButtonContainers.h"
#include "Paths.h"
#include "common\Colors.h"
#include "AudioDevicesList.h"
#include "AudioModules.h"
#include "VideoModules.h"
#include "VideoDevicesList.h"
#include "UaConf.h"
#include "baresip_base_config.h"
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonEdit *frmButtonEdit = NULL;

namespace {
	ButtonConf::Color colors[ButtonConf::EL_LIMITER];
}
//---------------------------------------------------------------------------
__fastcall TfrmButtonEdit::TfrmButtonEdit(TComponent* Owner)
	: TForm(Owner),
	lastTab(NULL)
{
    pageControl->ActivePage = tsGeneral;
	for (int i=0; i<Button::TYPE_LIMITER; i++)
	{
		cbType->Items->Add(Button::TypeName((Button::Type)i));
	}
	cbParentId->Clear();
	for (int i=0; i<BUTTON_CONTAINER__COUNT; i++)
	{
    	cbParentId->Items->Add(GetButtonContainerName(static_cast<enum ButtonContainerId>(i)));
	}
	cbInactiveColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbInactiveColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
	cbInactiveDownColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbInactiveDownColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
	cbIdleColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbIdleColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
	cbDownColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbDownColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
	cbDownPressedColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbDownPressedColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}     
	colorDialog->Color = clGray;

	for (int i=0; i<pcBehavior->PageCount; i++)
	{
		pcBehavior->Pages[i]->TabVisible = false;
	}

	AudioModules::FillInputSelectorCb(cbSoundInputMod);
	AudioModules::FillOutputSelectorCb(cbSoundOutputMod);

	VideoModules::FillInputSelectorCb(cbVideoInputMod);

#ifndef USE_VIDEO
	lblNoVideo->Visible = true;
#endif

	for (int i=0; i<ButtonConf::BLF_REMOTE_IDENTITY_DISPLAY_LIMITER; i++)
	{
		cbBlfRemoteIdentityDisplay->Items->Add(ButtonConf::GetBlfRemoteIdentityDisplayDescription(static_cast<ButtonConf::BlfRemoteIdentityDisplay>(i)));
	}

	for (int i=0; i<pageControl->PageCount; i++)
	{
		pageControl->Pages[i]->TabVisible = false;
		pageControl->Pages[i]->Visible = false;
	}

	CreatePages();
}

TTreeNode* TfrmButtonEdit::CreatePagesNode(TTreeNode *parent, TTabSheet *tab)
{
	TTreeNode *node;
	if (parent == NULL)
		node = tvSelector->Items->Add(parent, tab->Caption);
	else
		node = tvSelector->Items->AddChild(parent, tab->Caption);
	node->Data = tab;

	return node;
}

void TfrmButtonEdit::CreatePages(void)
{
	tvSelector->Items->Clear();

	CreatePagesNode(NULL, tsGeneral);
	CreatePagesNode(NULL, tsBehavior);
	CreatePagesNode(NULL, tsColors);
	CreatePagesNode(NULL, tsBitmaps);


	if (tvSelector->Items->Count > 0)
	{
		tvSelector->Items->Item[0]->Selected = true;
	}
}


//---------------------------------------------------------------------------
void __fastcall TfrmButtonEdit::FormShow(TObject *Sender)
{
	confirmed = false;
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::ShowModal(ButtonConf *cfg, int btnId)
{
	this->cfg = cfg;
	this->btnId = btnId;
	refreshAudioDevList = true;	
	refreshVideoDevList = true;	
	ApplyConf();
	TForm::ShowModal();
}

void TfrmButtonEdit::ApplyConf(void)
{
	cbType->ItemIndex = cfg->type;
	if (cfg->parentId >= 0 && cfg->parentId < cbParentId->Items->Count)
	{
		cbParentId->ItemIndex = cfg->parentId;
	}
	else
	{
    	cbParentId->ItemIndex = BUTTON_CONTAINER__DEFAULT;
	}
	if (cfg->captionLines > 0 && cfg->captionLines <= cbCaptionLines->Items->Count)
	{
		cbCaptionLines->ItemIndex = cfg->captionLines - 1;
	}
	memoCaption1->Text = cfg->caption.c_str();
	memoCaption2->Text = cfg->caption2.c_str();
	edNumber->Text = cfg->number.c_str();
	chbVisible->Checked = cfg->visible;
	chbDown->Checked = cfg->down;
	chbInactive->Checked = cfg->inactive;
	chbCustomFrame->Checked = cfg->customFrame;
	chbLabelCenterHorizontally->Checked = cfg->labelCenterHorizontally;
	chbLabelCenterVertically->Checked = cfg->labelCenterVertically;
	edLabelLeft->Text = cfg->labelLeft;
	edLabelTop->Enabled = !chbLabelCenterVertically->Checked;
	edLabelTop->Text = cfg->labelTop;

	edLabel2Left->Text = cfg->label2Left;
	edLabel2Top->Text = cfg->label2Top;
	chbLabel2CenterHorizontally->Checked = cfg->label2CenterHorizontally;
	edLabel2Left->Enabled = !chbLabel2CenterHorizontally->Checked;

	chbSpaceLabelsYEqually->Checked = cfg->spaceLabelsYEqually;
	UpdateLabelsTopVisibility();	

	chbImageTransparent->Checked = cfg->imageTransparent;
	edLeft->Text = cfg->left;
	edTop->Text = cfg->top;
	edWidth->Text = cfg->width;
	edHeight->Text = cfg->height;
	edBevelWidth->Text = cfg->bevelWidth;

    memcpy(colors, cfg->colors, sizeof(cfg->colors));	
	UpdateColorView();

	edImgIdle->Text = cfg->imgIdle.c_str();
	edImgConfirmed->Text = cfg->imgConfirmed.c_str();
	edImgTerminated->Text = cfg->imgTerminated.c_str();
	edImgEarly->Text = cfg->imgEarly.c_str();
	edImageLeft->Text = cfg->imageLeft;
	edImageTop->Text = cfg->imageTop;
	chbImageCenterVertically->Checked = cfg->imageCenterVertically;
	edImageTop->Enabled = !chbImageCenterVertically->Checked;

	UpdateImgPreview();

	chbBlfOverrideIdle->Checked = cfg->blfOverrideIdle.active;
	edBlfOverrideIdle->Text = cfg->blfOverrideIdle.number.c_str();
	chbBlfOverrideTerminated->Checked = cfg->blfOverrideTerminated.active;
	edBlfOverrideTerminated->Text = cfg->blfOverrideTerminated.number.c_str();
	chbBlfOverrideEarly->Checked = cfg->blfOverrideEarly.active;
	edBlfOverrideEarly->Text = cfg->blfOverrideEarly.number.c_str();
	chbBlfOverrideConfirmed->Checked = cfg->blfOverrideConfirmed.active;
	edBlfOverrideConfirmed->Text = cfg->blfOverrideConfirmed.number.c_str();

	cbBlfActionDuringCall->ItemIndex = cfg->blfActionDuringCall;
	cbBlfRemoteIdentityDisplay->ItemIndex = cfg->blfRemoteIdentityDisplay;
	edBlfDtmfPrefixDuringCall->Text = cfg->blfDtmfPrefixDuringCall.c_str();

	edBlfExpires->Text = cfg->expires;

	edArg1->Text = cfg->arg1.c_str();

	edAutoAnswerSipCode->Text = cfg->sipCode;
	edAutoAnswerSipReason->Text = cfg->sipReason.c_str();

	edPagingTxWaveFile->Text = cfg->pagingTxWaveFile.c_str();
	cbPagingTxCodec->Text = cfg->pagingTxCodec.c_str();
	cbPagingTxPtime->Text = cfg->pagingTxPtime;

	edScriptFile->Text = cfg->script.c_str();

	edSpeedDialFont->Font->Name = cfg->font.name.c_str();
	edSpeedDialFont->Font->Size = cfg->font.size;
	TFontStyles style = TFontStyles();
	if (cfg->font.bold)
		style << fsBold;
	if (cfg->font.italic)
		style << fsItalic;
	if (cfg->font.underline)
		style << fsUnderline;
	if (cfg->font.strikeout)
		style << fsStrikeOut;
	edSpeedDialFont->Font->Style = style;

	edCaption2Font->Font->Name = cfg->fontLabel2.name.c_str();
	edCaption2Font->Font->Size = cfg->fontLabel2.size;
	style = TFontStyles();
	if (cfg->fontLabel2.bold)
		style << fsBold;
	if (cfg->fontLabel2.italic)
		style << fsItalic;
	if (cfg->fontLabel2.underline)
		style << fsUnderline;
	if (cfg->fontLabel2.strikeout)
		style << fsStrikeOut;
	edCaption2Font->Font->Style = style;

	cbSoundInputMod->ItemIndex = AudioModules::GetInputModuleCbIndex(cfg->audioRxMod.c_str());
	cbSoundOutputMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(cfg->audioTxMod.c_str());

	cbVideoInputMod->ItemIndex = VideoModules::GetInputModuleCbIndex(cfg->videoRxMod.c_str());
	cbVideoInputModChange(NULL);
	edVideoInputFile->Text = cfg->videoRxMod.c_str();

	SetType(cfg->type);
}

void __fastcall TfrmButtonEdit::btnApplyClick(TObject *Sender)
{
	confirmed = true;
	cfg->type = static_cast<Button::Type>(cbType->ItemIndex);
	cfg->parentId = cbParentId->ItemIndex;
	cfg->captionLines = cbCaptionLines->ItemIndex + 1;
	cfg->caption = memoCaption1->Text.c_str();
	cfg->caption2 = memoCaption2->Text.c_str();

	cfg->number = edNumber->Text.c_str();
	cfg->visible = chbVisible->Checked;
	cfg->down = chbDown->Checked;
	cfg->inactive = chbInactive->Checked;
	cfg->customFrame = chbCustomFrame->Checked;
	cfg->labelCenterHorizontally = chbLabelCenterHorizontally->Checked;
	cfg->labelCenterVertically = chbLabelCenterVertically->Checked;
	cfg->labelLeft = StrToIntDef(edLabelLeft->Text, cfg->labelLeft);
	cfg->labelTop = StrToIntDef(edLabelTop->Text, cfg->labelTop);

	cfg->label2Left = StrToIntDef(edLabel2Left->Text, 0);
	cfg->label2Top = StrToIntDef(edLabel2Top->Text, 0);
	cfg->label2CenterHorizontally = chbLabel2CenterHorizontally->Checked;

	cfg->spaceLabelsYEqually = chbSpaceLabelsYEqually->Checked;

	cfg->imageTransparent = chbImageTransparent->Checked;
	cfg->left = StrToIntDef(edLeft->Text, 0);
	cfg->top = StrToIntDef(edTop->Text, 0);
	cfg->width = StrToIntDef(edWidth->Text, 100);
	cfg->height = StrToIntDef(edHeight->Text, 32);
	cfg->bevelWidth = StrToIntDef(edBevelWidth->Text, cfg->bevelWidth);
	if (static_cast<int>(cfg->bevelWidth) < 0)
	{
		cfg->bevelWidth = 0;
	}

	UpdateColors();
    memcpy(cfg->colors, colors, sizeof(cfg->colors));

	cfg->imgIdle = edImgIdle->Text.c_str();
	cfg->imgConfirmed = edImgConfirmed->Text.c_str();
	cfg->imgTerminated = edImgTerminated->Text.c_str();
	cfg->imgEarly = edImgEarly->Text.c_str();
	cfg->imageLeft = StrToIntDef(edImageLeft->Text, 0);
	cfg->imageTop = StrToIntDef(edImageTop->Text, 0);
	cfg->imageCenterVertically = chbImageCenterVertically->Checked;

	cfg->blfOverrideIdle.active = chbBlfOverrideIdle->Checked;
	cfg->blfOverrideIdle.number = edBlfOverrideIdle->Text.c_str();
	cfg->blfOverrideTerminated.active = chbBlfOverrideTerminated->Checked;
	cfg->blfOverrideTerminated.number = edBlfOverrideTerminated->Text.c_str();
	cfg->blfOverrideEarly.active = chbBlfOverrideEarly->Checked;
	cfg->blfOverrideEarly.number = edBlfOverrideEarly->Text.c_str();
	cfg->blfOverrideConfirmed.active = chbBlfOverrideConfirmed->Checked;
	cfg->blfOverrideConfirmed.number = edBlfOverrideConfirmed->Text.c_str();

	cfg->blfActionDuringCall = static_cast<ButtonConf::BlfActionDuringCall>(cbBlfActionDuringCall->ItemIndex);
	cfg->blfRemoteIdentityDisplay = static_cast<ButtonConf::BlfRemoteIdentityDisplay>(cbBlfRemoteIdentityDisplay->ItemIndex);
	cfg->blfDtmfPrefixDuringCall = edBlfDtmfPrefixDuringCall->Text.c_str();

	{
		int tmp = StrToIntDef(edBlfExpires->Text, cfg->expires);
		if (tmp > 10 && tmp < 72*3600)
			cfg->expires = tmp;
	}

	cfg->arg1 = edArg1->Text.c_str();

	{
		int tmp = StrToIntDef(edAutoAnswerSipCode->Text, cfg->sipCode);
		if (tmp >= 100 && tmp <= 999)
			cfg->sipCode = tmp;
	}
	cfg->sipReason = edAutoAnswerSipReason->Text.c_str();

	cfg->pagingTxWaveFile = edPagingTxWaveFile->Text.c_str();
	cfg->pagingTxCodec = cbPagingTxCodec->Text.c_str();
	cfg->pagingTxPtime = StrToIntDef(cbPagingTxPtime->Text, 20);

	cfg->script = edScriptFile->Text.c_str();

	cfg->audioRxMod = AudioModules::GetInputModuleFromCbIndex(cbSoundInputMod->ItemIndex);
	if (cbSoundInputDev->Visible)
	{
		if (cbSoundInputDev->Tag == 0 || cbSoundInputDev->ItemIndex != cbSoundInputDev->Items->Count - 1)
		{
			cfg->audioRxDev = cbSoundInputDev->Text.c_str();
		}
	}
	else if (edSoundInputWave->Visible)
	{
		cfg->audioRxDev = edSoundInputWave->Text.c_str();
	}

	cfg->audioTxMod = AudioModules::GetOutputModuleFromCbIndex(cbSoundOutputMod->ItemIndex);
	if (cbSoundOutputDev->Tag == 0 || cbSoundOutputDev->ItemIndex != cbSoundOutputDev->Items->Count - 1)
	{
		cfg->audioTxDev = cbSoundOutputDev->Text.c_str();
	}

	cfg->videoRxMod = VideoModules::GetInputModuleFromCbIndex(cbVideoInputMod->ItemIndex);
	if (cfg->videoRxMod == VideoModules::avformat)
	{
		cfg->videoRxDev = edVideoInputFile->Text.c_str();
	}
	else
	{
		if (cbVideoInputDev->Tag == 0 || cbVideoInputDev->ItemIndex != cbVideoInputDev->Items->Count - 1)
		{
			cfg->videoRxDev = cbVideoInputDev->Text.c_str();
		}
	}

	Close();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbTypeChange(TObject *Sender)
{
	SetType(static_cast<Button::Type>(cbType->ItemIndex));	
}
//---------------------------------------------------------------------------

void TfrmButtonEdit::SetType(Button::Type type)
{
	memoHelp->Text = Button::TypeDescription(type);

	edNumber->Visible = true;
	lblNumber->Visible = true;

	for (int i=0; i<pcBehavior->PageCount; i++)
	{
        pcBehavior->Pages[i]->Visible = false;
    }

	switch (type)
	{
	case Button::DISABLED:
	case Button::CONTACT_NOTE:
	case Button::CONTACT_FILE:
	case Button::REDIAL:
	case Button::HOLD:
	case Button::HTTP_QUERY:
	case Button::SIP_ACCESS_URL:
	case Button::SHOW_SETTINGS:
	case Button::MINIMIZE:
	case Button::SHOW_LOG:
	case Button::EXIT:
	case Button::UA_RESTART:
	case Button::RECORD:
	case Button::RECORD_PAUSE:
	case Button::UNREGISTER:
	case Button::REREGISTER:
	case Button::MUTE_RING:
	case Button::MUTE:
	case Button::ATTENDED_TRANSFER:
	case Button::CONFERENCE_START:
	case Button::CALL_MAKE_OR_ANSWER:
	case Button::CALL_ANSWER:
		edNumber->Visible = false;
		lblNumber->Visible = false;
		break;

	case Button::LINE:
		edNumber->Visible = false;
		lblNumber->Visible = false;
		cbCaptionLines->ItemIndex = 1;	// 2 caption lines
		break;

	case Button::SPEED_DIAL:
		tsBehaviorBlf->Visible = true;
		break;
	case Button::BLF:
		//chbNoIcon->Checked = false;
		tsBehaviorBlf->Visible = true;
		break;
	case Button::MWI:
		tsBehaviorBlf->Visible = true;
		edNumber->Visible = false;
		lblNumber->Visible = false;			
		break;
	case Button::PRESENCE:
		tsBehaviorBlf->Visible = true;
		break;
	case Button::EXECUTE:
		tsBehaviorExecute->Visible = true;
		break;
	case Button::PAGING_TX:
		tsBehaviorPaging->Visible = true;
		break;
	case Button::SCRIPT:
		tsBehaviorScript->Visible = true;
		break;
	case Button::SWITCH_AUDIO_SOURCE:
		tsBehaviorSwitchAudioSource->Visible = true;
		edNumber->Visible = false;
		lblNumber->Visible = false;

		if (refreshAudioDevList)
		{
			refreshAudioDevList = false;
			AudioDevicesList::Instance().Refresh();
		}
		cbSoundInputMod->ItemIndex = AudioModules::GetInputModuleCbIndex(cfg->audioRxMod.c_str());
		cbSoundInputModChange(NULL);

		break;
	case Button::SWITCH_AUDIO_PLAYER:
		tsBehaviorSwitchAudioPlayer->Visible = true;
		edNumber->Visible = false;
		lblNumber->Visible = false;

		if (refreshAudioDevList)
		{
			refreshAudioDevList = false;
			AudioDevicesList::Instance().Refresh();
		}
		cbSoundOutputMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(cfg->audioTxMod.c_str());
		cbSoundOutputModChange(NULL);

		break;
		
	case Button::AUTO_ANSWER_DND:
	case Button::HANGUP:
	case Button::HANGUP_ALL:
		tsBehaviorAutoAnswer->Visible = true;
		edNumber->Visible = false;
		lblNumber->Visible = false;		
		break;

	case Button::SWITCH_VIDEO_SOURCE:
		tsBehaviorSwitchVideoSource->Visible = true;
		edNumber->Visible = false;
		lblNumber->Visible = false;

		if (refreshVideoDevList)
		{
			refreshVideoDevList = false;
			VideoDevicesList::Instance().Refresh();
		}
		cbVideoInputMod->ItemIndex = VideoModules::GetInputModuleCbIndex(cfg->videoRxMod.c_str());
		cbVideoInputModChange(NULL);	
		break;

	default:
		break;
	}
}

void __fastcall TfrmButtonEdit::SelectImgClick(TObject *Sender)
{
	TEdit *edit = NULL;
	TButton *btn = dynamic_cast<TButton*>(Sender);
	if (btn == btnSelectImgIdle)
		edit = edImgIdle;
	else if (btn == btnSelectImgTerminated)
		edit = edImgTerminated;
	else if (btn == btnSelectImgEarly)
		edit = edImgEarly;
	else if (btn == btnSelectImgConfirmed)
		edit = edImgConfirmed;

	if (!edit)
		return;
	AnsiString dir = Paths::GetProfileDir() + "\\img\\";
	openDialog->InitialDir = dir;
	openDialog->Filter = "";	
	if (FileExists(dir + edit->Text))
		openDialog->FileName = dir + edit->Text;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
    	edit->Text = ExtractFileName(openDialog->FileName);
	}

	UpdateImgPreview();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnClearClick(TObject *Sender)
{
	cfg->Reset();
	ProgrammableButtons::SetDefaultsForBtnId(btnId, *cfg);
	ApplyConf();
	cbSoundInputDev->ItemIndex = -1;
	cbSoundOutputDev->ItemIndex = -1;
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::SelectColorClick(TObject *Sender)
{
	int *col;
	ButtonConf::Color &color = colors[cbColorElement->ItemIndex];
	if (Sender == btnSelectInactiveColor)
	{
		col = &color.inactive;
	}
	else if (Sender == btnSelectInactiveDownColor)
	{
        col = &color.inactiveDown;
    }
	else if (Sender == btnSelectIdleColor)
	{
		col = &color.idle;
	}
	else if (Sender == btnSelectDownColor)
	{
		col = &color.down;
	}
	else if (Sender == btnSelectDownPressedColor)
	{
		col = &color.downPressed;
	}
	else
	{
		assert(!"Unhandled color type");
		return;
	}
	colorDialog->Color = static_cast<TColor>(*col);
	if (colorDialog->Execute())
	{
		*col = colorDialog->Color;
		UpdateColorsPreview();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbColorChange(TObject *Sender)
{
	btnSelectInactiveColor->Visible = (cbInactiveColor->ItemIndex == Color::clCustom);
    btnSelectInactiveDownColor->Visible = (cbInactiveDownColor->ItemIndex == Color::clCustom);
	btnSelectIdleColor->Visible = (cbIdleColor->ItemIndex == Color::clCustom);
	btnSelectDownColor->Visible = (cbDownColor->ItemIndex == Color::clCustom);
	btnSelectDownPressedColor->Visible = (cbDownPressedColor->ItemIndex == Color::clCustom);

	UpdateColors();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSelectWaveClick(TObject *Sender)
{
	TEdit *edit = edPagingTxWaveFile;
	AnsiString dir = Paths::GetProfileDir();
	ForceDirectories(dir);
	openDialog->Filter = "WAVE files (*.wav)|*.wav|All files|*.*";
	openDialog->InitialDir = dir;
	AnsiString fileName = dir + "\\" + edit->Text;
	if (FileExists(fileName))
		openDialog->FileName = fileName;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		if (UpperCase(dir) != UpperCase(ExtractFileDir(openDialog->FileName)))
		{
			MessageBox(this->Handle, "Entry was not updated.\nFor portability ring WAVE files must be placed in application subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		edit->Text = ExtractFileName(openDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSelectScriptClick(TObject *Sender)
{
	TEdit *edit = edScriptFile;
	AnsiString dir = Paths::GetProfileDir() + "\\scripts";
	ForceDirectories(dir);
	openDialog->InitialDir = dir;
	openDialog->Filter = "Lua files (*.lua)|*.lua|All files|*.*";
	AnsiString fileName = dir + "\\" + edit->Text;
	if (FileExists(fileName))
		openDialog->FileName = fileName;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		if (UpperCase(dir) != UpperCase(ExtractFileDir(openDialog->FileName)))
		{
			MessageBox(this->Handle, "Entry was not updated.\nFor portability script files must be placed in \"script\" subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		edit->Text = ExtractFileName(openDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbSoundInputModChange(TObject *Sender)
{
	AnsiString mod = AudioModules::GetInputModuleFromCbIndex(cbSoundInputMod->ItemIndex);
	if (mod == AudioModules::portaudio ||
		mod == AudioModules::winwave ||
		mod == AudioModules::winwave2)
	{
		btnSelectWaveFile->Visible = false;
		edSoundInputWave->Visible = false;
		cbSoundInputDev->Visible = true;
		lblSoundInputDevice->Visible = true;
		AudioDevicesList::FillComboBox(cbSoundInputDev, mod, false, cfg->audioRxDev.c_str());
	}
	else if (mod == AudioModules::aufile || mod == AudioModules::aufileMm || mod == AudioModules::avformat)
	{
		btnSelectWaveFile->Visible = true;
		edSoundInputWave->Visible = true;
		cbSoundInputDev->Visible = false;
		lblSoundInputDevice->Visible = true;
		edSoundInputWave->Text = cfg->audioRxDev.c_str();
	}
	else if (mod == AudioModules::nullaudio)
	{
		btnSelectWaveFile->Visible = false;
		edSoundInputWave->Visible = false;
		cbSoundInputDev->Visible = false;
		lblSoundInputDevice->Visible = false;
	}
	else
	{
		assert(!"Unhandled cbSoundInputMod index!");
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSelectWaveFileClick(TObject *Sender)
{
	dlgOpenRing->InitialDir = Paths::GetProfileDir();
	dlgOpenRing->Filter = "WAVE files (*.wav)|*.wav|All files|*.*";

	if (edSoundInputWave->Text != "")
	{
		if (FileExists(Paths::GetProfileDir() + "\\" + edSoundInputWave->Text))
		{
			dlgOpenRing->FileName = Paths::GetProfileDir() + "\\" + edSoundInputWave->Text;
		}
		else if (FileExists(edSoundInputWave->Text))
		{
			dlgOpenRing->FileName = edSoundInputWave->Text;
		}
	}
	if (dlgOpenRing->Execute())
	{
		if (UpperCase(Paths::GetProfileDir()) != UpperCase(ExtractFileDir(dlgOpenRing->FileName)))
		{
			edSoundInputWave->Text = dlgOpenRing->FileName;
		}
		else
		{
			edSoundInputWave->Text = ExtractFileName(dlgOpenRing->FileName);
		}
	}	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbSoundOutputModChange(TObject *Sender)
{
	AnsiString mod = AudioModules::GetOutputModuleFromCbIndex(cbSoundOutputMod->ItemIndex);
	if (mod == AudioModules::portaudio ||
		mod == AudioModules::winwave ||
		mod == AudioModules::winwave2)
	{
		if (mod == AudioModules::winwave)
			mod = AudioModules::winwave2;	// original "winwave" was buggy/obsoletes/removed
		cbSoundOutputDev->Visible = true;
		lblSoundOutputDev->Visible = true;
		AudioDevicesList::FillComboBox(cbSoundOutputDev, mod, true, cfg->audioTxDev.c_str());
	}
	else if (mod == AudioModules::nullaudio)
	{
		cbSoundOutputDev->Visible = false;
		lblSoundOutputDev->Visible = false;
	}
	else
	{
		assert(!"Unhandled cbSoundOutputMod index!");
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSpeedDialFontSelectClick(TObject *Sender)
{
	TButton *btn = dynamic_cast<TButton*>(Sender);
	assert(btn);

	ButtonConf::Font *font = NULL;
	TEdit *ed = NULL;

	if (btn == btnSpeedDialFontSelect)
	{
		font = &cfg->font;
		ed = edSpeedDialFont;
	}
	else if (btn == btnCaption2FontSelect)
	{
		font = &cfg->fontLabel2;
		ed = edCaption2Font;
	}
	else
	{
		ShowMessage("Unhandled button");
	}

	fontDialog->Font->Name = font->name.c_str();
	fontDialog->Font->Size = font->size;
	TFontStyles style = TFontStyles();
	if (font->bold)
		style << fsBold;
	if (font->italic)
		style << fsItalic;
	if (font->underline)
		style << fsUnderline;
	if (font->strikeout)
		style << fsStrikeOut;
	fontDialog->Font->Style = style;		
	fontDialog->Font->Color = clBlack;
	if (fontDialog->Execute())
	{
		if (fontDialog->Font->Color != clBlack)
		{
			MessageBox(this->Handle, "Warning: this dialog is not used for font color selection.\nUse \"Colors\" group below for this purpose.", "Font color selection", MB_ICONEXCLAMATION);
			fontDialog->Font->Color = clBlack;			
		}
		ed->Font = fontDialog->Font;
		font->name = fontDialog->Font->Name.c_str();
		font->size = fontDialog->Font->Size;
		font->bold = fontDialog->Font->Style.Contains(fsBold);
		font->italic = fontDialog->Font->Style.Contains(fsItalic);
		font->underline = fontDialog->Font->Style.Contains(fsUnderline);
		font->strikeout = fontDialog->Font->Style.Contains(fsStrikeOut);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbColorElementChange(TObject *Sender)
{
	UpdateColorView();
}
//---------------------------------------------------------------------------

void TfrmButtonEdit::UpdateColorView(void)
{
	if (cbColorElement->ItemIndex < 0)
		cbColorElement->ItemIndex = 0;
	ButtonConf::Color &color = colors[cbColorElement->ItemIndex];
	
	int colorId;

	colorId = Color::IntTColorToId(color.idle);
	cbIdleColor->ItemIndex = colorId;
	btnSelectIdleColor->Visible = (colorId == Color::clCustom);

	colorId = Color::IntTColorToId(color.inactive);
	cbInactiveColor->ItemIndex = colorId;
	btnSelectInactiveColor->Visible = (colorId == Color::clCustom);

	colorId = Color::IntTColorToId(color.inactiveDown);
	cbInactiveDownColor->ItemIndex = colorId;
	btnSelectInactiveDownColor->Visible = (colorId == Color::clCustom);

	colorId = Color::IntTColorToId(color.down);
	cbDownColor->ItemIndex = colorId;
	btnSelectDownColor->Visible = (colorId == Color::clCustom);

	colorId = Color::IntTColorToId(color.downPressed);
	cbDownPressedColor->ItemIndex = colorId;
	btnSelectDownPressedColor->Visible = (colorId == Color::clCustom);

	UpdateColorsPreview();
}

void TfrmButtonEdit::UpdateColors(void)
{
	ButtonConf::Color &color = colors[cbColorElement->ItemIndex];

	if (cbIdleColor->ItemIndex != Color::clCustom)
	{
		color.idle = Color::IdToIntTColor(static_cast<Color::Id>(cbIdleColor->ItemIndex));
	}

	if (cbInactiveColor->ItemIndex != Color::clCustom)
	{
		color.inactive = Color::IdToIntTColor(static_cast<Color::Id>(cbInactiveColor->ItemIndex));
	}

	if (cbInactiveDownColor->ItemIndex != Color::clCustom)
	{
		color.inactiveDown = Color::IdToIntTColor(static_cast<Color::Id>(cbInactiveDownColor->ItemIndex));
	}

	if (cbDownColor->ItemIndex != Color::clCustom)
	{
		color.down = Color::IdToIntTColor(static_cast<Color::Id>(cbDownColor->ItemIndex));
	}

	if (cbDownPressedColor->ItemIndex != Color::clCustom)
	{
		color.downPressed = Color::IdToIntTColor(static_cast<Color::Id>(cbDownPressedColor->ItemIndex));
	}

	UpdateColorsPreview();
}

void TfrmButtonEdit::UpdateColorsPreview(void)
{
	ButtonConf::Color &color = colors[cbColorElement->ItemIndex];
	shColorIdle->Brush->Color = static_cast<TColor>(color.idle);
	shColorInactive->Brush->Color = static_cast<TColor>(color.inactive);
	shColorInactiveDown->Brush->Color = static_cast<TColor>(color.inactiveDown);
	shColorDown->Brush->Color = static_cast<TColor>(color.down);
	shColorDownPressed->Brush->Color = static_cast<TColor>(color.downPressed);
}


void __fastcall TfrmButtonEdit::chbImageCenterVerticallyClick(TObject *Sender)
{
	edImageTop->Enabled = !chbImageCenterVertically->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::chbLabelCenterVerticallyClick(TObject *Sender)
{
	edLabelTop->Enabled = !chbLabelCenterVertically->Checked;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::chbLabel2CenterHorizontallyClick(TObject *Sender)
{
	edLabel2Left->Enabled = !chbLabel2CenterHorizontally->Checked;
}
//---------------------------------------------------------------------------



void __fastcall TfrmButtonEdit::btnSelectedScriptOnButtonEditClick(TObject *Sender)
{
	TEdit *edit = NULL;
	AnsiString eventName;
	if (Sender == btnSelectedScriptOnButtonEdit)
	{
		edit = edScriptFile;
		eventName = "on_btn";
	}
	else
	{
		assert(0);
		return;
	}
	AnsiString dir = Paths::GetProfileDir() + "\\scripts";
	ForceDirectories(dir);
	AnsiString file = dir + "\\";
	if (edit->Text != "")
	{
		file += edit->Text;
	}
	else
	{
		AnsiString name = eventName + "_" + FormatDateTime("yyyymmdd_hhnnss_zzz", Now()) + ".lua";
		file += name;
		edit->Text = name;
	}
	if (!FileExists(file))
	{
		FILE *fp = fopen(file.c_str(), "wb");
		if (fp)
		{
			fclose(fp);
		}
		else
		{
			MessageBox(this->Handle, "Could not create file in \"script\" subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
	}

	TfrmLuaScript *frmLuaScript = new TfrmLuaScript(NULL);
	frmLuaScript->Show();
	frmLuaScript->OpenFile(file);
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::chbSpaceLabelsYEquallyClick(TObject *Sender)
{
	UpdateLabelsTopVisibility();
}
//---------------------------------------------------------------------------

void TfrmButtonEdit::UpdateLabelsTopVisibility(void)
{
	bool state = !chbSpaceLabelsYEqually->Checked;
	lblLabelTop->Visible = state;
	edLabelTop->Visible = state;
	chbLabelCenterVertically->Visible = state;
	lblLabel2Top->Visible = state;
	edLabel2Top->Visible = state;
}

void __fastcall TfrmButtonEdit::cbVideoInputModChange(TObject *Sender)
{
	AnsiString mod = VideoModules::GetInputModuleFromCbIndex(cbVideoInputMod->ItemIndex);
	if (mod == VideoModules::dshow)
	{
		btnSelectVideoInputFile->Visible = false;
		edVideoInputFile->Visible = false;
		cbVideoInputDev->Visible = true;
		lblVideoInputDevice->Visible = true;
		VideoDevicesList::FillComboBox(cbVideoInputDev, mod, false, cfg->videoRxDev.c_str());
	}
	else if (mod == VideoModules::nullvideo ||
		mod == VideoModules::colorbar_generator ||
        mod == VideoModules::colorbar_generator_animated
		)
	{
		btnSelectVideoInputFile->Visible = false;
		edVideoInputFile->Visible = false;
		cbVideoInputDev->Visible = false;
		lblVideoInputDevice->Visible = false;
	}
	else if (mod == VideoModules::avformat)
	{
		btnSelectVideoInputFile->Visible = true;
		edVideoInputFile->Visible = true;
		cbVideoInputDev->Visible = false;
		lblVideoInputDevice->Visible = false;
	}
	else
	{
		assert(!"Unhandled cbVideoInputMod item index!");
		btnSelectVideoInputFile->Visible = false;
		edVideoInputFile->Visible = false;
		cbVideoInputDev->Visible = false;
		lblVideoInputDevice->Visible = false;
	}
}
//---------------------------------------------------------------------------


void __fastcall TfrmButtonEdit::tvSelectorChange(TObject *Sender,
      TTreeNode *Node)
{
	if (Node && Node->Selected)
	{
		TTabSheet *tab = reinterpret_cast<TTabSheet*>(Node->Data);
		assert(tab);
		tab->Visible = true;
		if (lastTab)
			lastTab->Visible = false;
		lastTab = tab;
	}	
}
//---------------------------------------------------------------------------

void TfrmButtonEdit::UpdateImgPreview(void)
{
	AnsiString idle = Paths::GetFullImgName(edImgIdle->Text);
	AnsiString terminated = Paths::GetFullImgName(edImgTerminated->Text);
	AnsiString early = Paths::GetFullImgName(edImgEarly->Text);
	AnsiString confirmed = Paths::GetFullImgName(edImgConfirmed->Text);

	try {
		if (FileExists(idle))
		{
			imgIdle->Picture->LoadFromFile(idle);
			imgIdle->Visible = true;
		}
	} catch (...) {
		imgIdle->Visible = false;
	}

	try {
		imgTerminated->Picture->LoadFromFile(terminated);
		imgTerminated->Visible = true;
	} catch (...) {
		imgTerminated->Visible = false;
	}

	try {
		imgEarly->Picture->LoadFromFile(early);
		imgEarly->Visible = true;
	} catch (...) {
		imgEarly->Visible = false;
	}

	try {
		imgConfirmed->Picture->LoadFromFile(confirmed);
		imgConfirmed->Visible = true;
	} catch (...) {
		imgConfirmed->Visible = false;
	}
}


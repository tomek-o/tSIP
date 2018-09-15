//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonEdit.h"
#include "ButtonType.h"
#include "ButtonConf.h"
#include "Colors.h"
#include "AudioDevicesList.h"
#include "UaConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonEdit *frmButtonEdit;

namespace {
	void FillDevList(TComboBox *target, int module, bool out, AnsiString selected)
	{
		target->Items->Clear();
		std::vector<AnsiString> *v = NULL;
		if (module == 0)
		{
			if (out)
				v = &AudioDevicesList::Instance().portaudioDevsOut;
			else
				v = &AudioDevicesList::Instance().portaudioDevsIn;
		}
		else if (module == 1)
		{
			if (out)
				v = &AudioDevicesList::Instance().winwaveDevsOut;
			else
				v = &AudioDevicesList::Instance().winwaveDevsIn;
		}
		else
		{
			return;
		}
		assert(v);
		for (int i=0; i<v->size(); i++)
		{
			AnsiString dev = v->at(i);
			target->Items->Add(dev);
			if (dev == selected)
			{
				target->ItemIndex = i;
			}
		}
	}
}

//---------------------------------------------------------------------------
__fastcall TfrmButtonEdit::TfrmButtonEdit(TComponent* Owner)
	: TForm(Owner)
{
    pageControl->ActivePage = tsVisual;
	for (int i=0; i<Button::TYPE_LIMITER; i++)
	{
		cbType->Items->Add(Button::TypeName((Button::Type)i));
	}
	cbBackgroundColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
        cbBackgroundColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
	colorDialog->Color = clGray;

	for (int i=0; i<pcBehavior->PageCount; i++)
	{
		pcBehavior->Pages[i]->TabVisible = false;
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmButtonEdit::FormShow(TObject *Sender)
{
	confirmed = false;
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::ShowModal(ButtonConf *cfg)
{
	this->cfg = cfg;
	refreshAudioDevList = true;	
	ApplyConf();
	TForm::ShowModal();
}

void TfrmButtonEdit::ApplyConf(void)
{
	cbType->ItemIndex = cfg->type;
	edDescription->Text = cfg->caption.c_str();
	edDescription2->Text = cfg->caption2.c_str();
	cbCaptionLines->ItemIndex = cfg->captionLines - 1;
	edNumber->Text = cfg->number.c_str();
	chbNoIcon->Checked = cfg->noIcon;
	edHeight->Text = cfg->height;
	edMarginTop->Text = cfg->marginTop;
	edMarginBottom->Text = cfg->marginBottom;
	int colorId = Color::IntTColorToId(cfg->backgroundColor);
	cbBackgroundColor->ItemIndex = colorId;
	if (colorId != Color::clCustom)
	{
		btnSelectBackgroundColor->Visible = false;
	}
	else
	{
		btnSelectBackgroundColor->Visible = true;
    }

	edImgIdle->Text = cfg->imgIdle.c_str();
	edImgConfirmed->Text = cfg->imgConfirmed.c_str();
	edImgTerminated->Text = cfg->imgTerminated.c_str();
	edImgEarly->Text = cfg->imgEarly.c_str();

	chbBlfOverrideIdle->Checked = cfg->blfOverrideIdle.active;
	edBlfOverrideIdle->Text = cfg->blfOverrideIdle.number.c_str();
	chbBlfOverrideTerminated->Checked = cfg->blfOverrideTerminated.active;
	edBlfOverrideTerminated->Text = cfg->blfOverrideTerminated.number.c_str();
	chbBlfOverrideEarly->Checked = cfg->blfOverrideEarly.active;
	edBlfOverrideEarly->Text = cfg->blfOverrideEarly.number.c_str();
	chbBlfOverrideConfirmed->Checked = cfg->blfOverrideConfirmed.active;
	edBlfOverrideConfirmed->Text = cfg->blfOverrideConfirmed.number.c_str();

	cbBlfActionDuringCall->ItemIndex = cfg->blfActionDuringCall;
	edBlfDtmfPrefixDuringCall->Text = cfg->blfDtmfPrefixDuringCall.c_str();

	edArg1->Text = cfg->arg1.c_str();

	edPagingTxWaveFile->Text = cfg->pagingTxWaveFile.c_str();
	cbPagingTxCodec->Text = cfg->pagingTxCodec.c_str();
	cbPagingTxPtime->Text = cfg->pagingTxPtime;

	edScriptFile->Text = cfg->script.c_str();

	SetType(cfg->type);
}

void __fastcall TfrmButtonEdit::btnApplyClick(TObject *Sender)
{
	confirmed = true;
	cfg->type = static_cast<Button::Type>(cbType->ItemIndex);
	cfg->caption = edDescription->Text.c_str();
	cfg->caption2 = edDescription2->Text.c_str();
	cfg->captionLines = cbCaptionLines->ItemIndex + 1;
	cfg->number = edNumber->Text.c_str();
	cfg->noIcon = chbNoIcon->Checked;
	cfg->height = StrToIntDef(edHeight->Text, 32);
	cfg->marginTop = StrToIntDef(edMarginTop->Text, 0);
	if (cfg->marginTop > 200)
		cfg->marginTop = 0;
	cfg->marginBottom = StrToIntDef(edMarginBottom->Text, 0);
	if (cfg->marginBottom > 200)
		cfg->marginBottom = 0;
	if (cbBackgroundColor->ItemIndex != Color::clCustom)
	{
		cfg->backgroundColor = Color::IdToIntTColor(static_cast<Color::Id>(cbBackgroundColor->ItemIndex));
	}
	else
	{
        cfg->backgroundColor = colorDialog->Color;
	}

	cfg->imgIdle = edImgIdle->Text.c_str();
	cfg->imgConfirmed = edImgConfirmed->Text.c_str();
	cfg->imgTerminated = edImgTerminated->Text.c_str();
	cfg->imgEarly = edImgEarly->Text.c_str();

	cfg->blfOverrideIdle.active = chbBlfOverrideIdle->Checked;
	cfg->blfOverrideIdle.number = edBlfOverrideIdle->Text.c_str();
	cfg->blfOverrideTerminated.active = chbBlfOverrideTerminated->Checked;
	cfg->blfOverrideTerminated.number = edBlfOverrideTerminated->Text.c_str();
	cfg->blfOverrideEarly.active = chbBlfOverrideEarly->Checked;
	cfg->blfOverrideEarly.number = edBlfOverrideEarly->Text.c_str();
	cfg->blfOverrideConfirmed.active = chbBlfOverrideConfirmed->Checked;
	cfg->blfOverrideConfirmed.number = edBlfOverrideConfirmed->Text.c_str();

	cfg->blfActionDuringCall = static_cast<ButtonConf::BlfActionDuringCall>(cbBlfActionDuringCall->ItemIndex);
	cfg->blfDtmfPrefixDuringCall = edBlfDtmfPrefixDuringCall->Text.c_str();

	cfg->arg1 = edArg1->Text.c_str();

	cfg->pagingTxWaveFile = edPagingTxWaveFile->Text.c_str();
	cfg->pagingTxCodec = cbPagingTxCodec->Text.c_str();
	cfg->pagingTxPtime = StrToIntDef(cbPagingTxPtime->Text, 20);

	cfg->script = edScriptFile->Text.c_str();

	if (cbSoundInputMod->ItemIndex == 0)
	{
		cfg->audioRxMod = UaConf::modPortaudio;
		cfg->audioRxDev = cbSoundInputDev->Text.c_str();
	}
	else if (cbSoundInputMod->ItemIndex == 1)
	{
		cfg->audioRxMod = UaConf::modWinwave;
		cfg->audioRxDev = cbSoundInputDev->Text.c_str();
	}
	else if (cbSoundInputMod->ItemIndex == 2)
	{
		cfg->audioRxMod = UaConf::modAufile;
		cfg->audioRxDev = edSoundInputWave->Text.c_str();
	}
	else
	{
        cfg->audioRxMod = UaConf::modNullaudio;
    }

	if (cbSoundOutputMod->ItemIndex == 0)
	{
		cfg->audioTxMod = UaConf::modPortaudio;
	}
	else if (cbSoundOutputMod->ItemIndex == 1)
	{
		cfg->audioTxMod = UaConf::modWinwave;
	}
	else
	{
        cfg->audioTxMod = UaConf::modNullaudio;
    }
	cfg->audioTxDev = cbSoundOutputDev->Text.c_str();

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
	chbNoIcon->Enabled = true;

	for (int i=0; i<pcBehavior->PageCount; i++)
	{
        pcBehavior->Pages[i]->Visible = false;
    }

	switch (type)
	{
	case Button::DISABLED:
		edNumber->Visible = false;
		lblNumber->Visible = false;
		break;
	case Button::SPEED_DIAL:
		tsBehaviorBlf->Visible = true;
		break;
	case Button::BLF:
		chbNoIcon->Checked = false;
		chbNoIcon->Enabled = false;
		tsBehaviorBlf->Visible = true;
		break;
	case Button::REDIAL:
		edNumber->Visible = false;
		lblNumber->Visible = false;
		break;
	case Button::TRANSFER:
	case Button::HOLD:
	case Button::HTTP_QUERY:
	case Button::SIP_ACCESS_URL:
	case Button::HANGUP:
	case Button::SHOW_SETTINGS:
	case Button::MINIMIZE:
	case Button::SHOW_LOG:
	case Button::EXIT:
		edNumber->Visible = false;
		lblNumber->Visible = false;
		break;
	case Button::MWI:
		tsBehaviorBlf->Visible = true;
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
		edNumber->Visible = false;
		lblNumber->Visible = false;		
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
		if (!strcmp(cfg->audioRxMod.c_str(), UaConf::modPortaudio))
		{
			cbSoundInputMod->ItemIndex = 0;
		}
		else if (!strcmp(cfg->audioRxMod.c_str(), UaConf::modWinwave))
		{
			cbSoundInputMod->ItemIndex = 1;
		}
		else if (!strcmp(cfg->audioRxMod.c_str(), UaConf::modAufile))
		{
			cbSoundInputMod->ItemIndex = 2;
		}
		else
		{
			cbSoundInputMod->ItemIndex = 3;
        }
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
		if (!strcmp(cfg->audioTxMod.c_str(), UaConf::modPortaudio))
		{
			cbSoundOutputMod->ItemIndex = 0;
		}
		else if (!strcmp(cfg->audioTxMod.c_str(), UaConf::modWinwave))
		{
			cbSoundOutputMod->ItemIndex = 1;
		}
		else
		{
			cbSoundOutputMod->ItemIndex = 2;
		}
		cbSoundOutputModChange(NULL);

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
	AnsiString dir = ExtractFileDir(Application->ExeName) + "\\img\\";
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
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnClearClick(TObject *Sender)
{
	cfg->Reset();
	ApplyConf();
	cbSoundInputDev->ItemIndex = -1;
	cbSoundOutputDev->ItemIndex = -1;
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::SelectColorClick(TObject *Sender)
{
	colorDialog->Color = static_cast<TColor>(cfg->backgroundColor);
	colorDialog->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::cbBackgroundColorChange(TObject *Sender)
{
	if (cbBackgroundColor->ItemIndex == Color::clCustom)
	{
		btnSelectBackgroundColor->Visible = true;
	}
	else
	{
        btnSelectBackgroundColor->Visible = false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSelectWaveClick(TObject *Sender)
{
	TEdit *edit = edPagingTxWaveFile;
	AnsiString dir = ExtractFileDir(Application->ExeName);
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
	AnsiString dir = ExtractFileDir(Application->ExeName) + "\\scripts";
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
	switch (cbSoundInputMod->ItemIndex)
	{
		case 0:	// portaudio
		case 1:	// winwave
			btnSelectWaveFile->Visible = false;
			edSoundInputWave->Visible = false;
			cbSoundInputDev->Visible = true;
			lblSoundInputDevice->Visible = true;
			FillDevList(cbSoundInputDev, cbSoundInputMod->ItemIndex, false, cfg->audioRxDev.c_str());
			break;
		case 2:	// aufile
			btnSelectWaveFile->Visible = true;
			edSoundInputWave->Visible = true;
			cbSoundInputDev->Visible = false;
			lblSoundInputDevice->Visible = true;
			edSoundInputWave->Text = cfg->audioRxDev.c_str();
			break;
		case 3:	// nullaudio
			btnSelectWaveFile->Visible = false;
			edSoundInputWave->Visible = false;
			cbSoundInputDev->Visible = false;
			lblSoundInputDevice->Visible = false;
			break;
		default:
			assert(!"Unhandled cbSoundInputMod index!");
			break;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonEdit::btnSelectWaveFileClick(TObject *Sender)
{
	dlgOpenRing->InitialDir = ExtractFileDir(Application->ExeName);
	if (edSoundInputWave->Text != "")
	{
		dlgOpenRing->FileName = ExtractFileDir(Application->ExeName) + "\\" + edSoundInputWave->Text;
	}
	if (dlgOpenRing->Execute())
	{
		if (UpperCase(ExtractFileDir(Application->ExeName)) != UpperCase(ExtractFileDir(dlgOpenRing->FileName)))
		{
			MessageBox(this->Handle, "File was not updated.\nFor portability source WAVE files must be placed in application directory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		edSoundInputWave->Text = ExtractFileName(dlgOpenRing->FileName);
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
	switch (cbSoundOutputMod->ItemIndex)
	{
		case 0:	// portaudio
		case 1:	// winwave
			cbSoundOutputDev->Visible = true;
			lblSoundOutputDev->Visible = true;
			FillDevList(cbSoundOutputDev, cbSoundOutputMod->ItemIndex, true, cfg->audioTxDev.c_str());
			break;
		case 2:	// nullaudio
			cbSoundOutputDev->Visible = false;
			lblSoundOutputDev->Visible = false;
			break;
		default:
			assert(!"Unhandled cbSoundOutputMod index!");
			break;
	}
}
//---------------------------------------------------------------------------


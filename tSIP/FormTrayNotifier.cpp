//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTrayNotifier.h"
#include "Settings.h"
#include "Paths.h"
#include "Log.h"
#include "Translate.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmTrayNotifier *frmTrayNotifier;
//---------------------------------------------------------------------------

void TfrmTrayNotifier::TranslateForm(void* obj)
{
	TfrmTrayNotifier *frm = reinterpret_cast<TfrmTrayNotifier*>(obj);
	assert(frm);
	TRANSLATE_TMP("TfrmTrayNotifier.btnAnswer", frm->btnAnswer->Caption);
	TRANSLATE_TMP("TfrmTrayNotifier.btnHangup", frm->btnHangup->Caption);
}


__fastcall TfrmTrayNotifier::TfrmTrayNotifier(TComponent* Owner)
	: TForm(Owner),
	OnHangup(NULL),
	OnAnswer(NULL)
{
	RegisterTranslationCb(this, TranslateForm);
	Width = appSettings.frmTrayNotifier.iWidth;
	Height = appSettings.frmTrayNotifier.iHeight;
	UpdateBackgroundImage();
	this->ActiveControl = btnStopFocus;
}
//---------------------------------------------------------------------------

void TfrmTrayNotifier::SetData(AnsiString description, AnsiString uri, bool incoming)
{
	lblDescription->Caption = description;
	lblUri->Caption = uri;
	btnAnswer->Visible = incoming;
	this->ActiveControl = btnStopFocus;	
}

void TfrmTrayNotifier::ShowWithoutFocus(void)
{
	frmTrayNotifier->Left = appSettings.frmTrayNotifier.iPosX;
	frmTrayNotifier->Top = appSettings.frmTrayNotifier.iPosY;
#if 0
	/** \todo frmTrayNotifier steals focus at first call */
	Visible = true;
#else
	/** \todo frmTrayNotifier: no focus stealed, but inconsistent window state */
	ShowWindow(Handle, SW_SHOWNOACTIVATE);
	SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//frmTrayNotifier->BringToFront();
	Visible = true;	
#endif
}

void TfrmTrayNotifier::HideWindow(void)
{
	if (appSettings.frmTrayNotifier.doNotChangePosition == false)
	{
		appSettings.frmTrayNotifier.iPosX = frmTrayNotifier->Left;
		appSettings.frmTrayNotifier.iPosY = frmTrayNotifier->Top;
	}
	Visible = false;
	ShowWindow(Handle, SW_HIDE);
}

void __fastcall TfrmTrayNotifier::btnHangupClick(TObject *Sender)
{
	if (OnHangup)
		OnHangup(486, "Busy Here");	
}
//---------------------------------------------------------------------------

void __fastcall TfrmTrayNotifier::btnAnswerClick(TObject *Sender)
{
	if (OnAnswer)
		OnAnswer();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmTrayNotifier::FormCreate(TObject *Sender)
{
	this->FormStyle = fsStayOnTop;
	Left = appSettings.frmTrayNotifier.iPosX;
	Top = appSettings.frmTrayNotifier.iPosY;
}
//---------------------------------------------------------------------------

void TfrmTrayNotifier::UpdateBackgroundImage(void)
{
	AnsiString asBackgroundFile;
	try
	{
		static AnsiString lastImage;
		AnsiString image = appSettings.frmTrayNotifier.backgroundImage;
		if (image != "" && image != lastImage)
		{
			asBackgroundFile = Paths::GetFullImgName(image);
			imgBackground->Picture->Bitmap->PixelFormat = pf24bit;
			imgBackground->Picture->LoadFromFile(asBackgroundFile);
			lastImage = image;
		}
	}
	catch (...)
	{
		LOG("Failed to load notifier window background (%s)\n", asBackgroundFile.c_str());
	}
}

void __fastcall TfrmTrayNotifier::CreateParams(TCreateParams &Params)
{
	TForm::CreateParams(Params);
	Params.ExStyle  |= WS_EX_APPWINDOW;	// Forces a top-level window onto the taskbar when the window is visible.
	Params.WndParent = GetDesktopWindow();
}

void TfrmTrayNotifier::SetCallState(Callback::ua_state_e state)
{
	if (state == Callback::CALL_STATE_ESTABLISHED)
	{
    	btnAnswer->Visible = false;
	}
}

void __fastcall TfrmTrayNotifier::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	if (appSettings.frmTrayNotifier.doNotChangePosition == false)
	{
		appSettings.frmTrayNotifier.iPosX = frmTrayNotifier->Left;
		appSettings.frmTrayNotifier.iPosY = frmTrayNotifier->Top;
	}
}
//---------------------------------------------------------------------------


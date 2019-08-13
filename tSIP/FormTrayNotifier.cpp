//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTrayNotifier.h"
#include "Settings.h"
#include "Paths.h"
#include "Log.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmTrayNotifier *frmTrayNotifier;
//---------------------------------------------------------------------------
__fastcall TfrmTrayNotifier::TfrmTrayNotifier(TComponent* Owner)
	: TForm(Owner),
	OnHangup(NULL),
	OnAnswer(NULL)
{
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
#if 1
	/** \todo frmTrayNotifier steals focus at first call */
	frmTrayNotifier->Visible = true;
#else
	/** \todo frmTrayNotifier: no focus stealed, but inconsistent window state */
	ShowWindow(frmTrayNotifier->Handle, SW_SHOWNOACTIVATE);
	SetWindowPos(frmTrayNotifier->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	//frmTrayNotifier->BringToFront();
#endif
}

void __fastcall TfrmTrayNotifier::btnHangupClick(TObject *Sender)
{
	if (OnHangup)
		OnHangup();	
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



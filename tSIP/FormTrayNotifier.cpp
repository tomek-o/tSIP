//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTrayNotifier.h"
#include "ua/Calls.h"
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
	OnAnswer(NULL),
	callUid(0)
{
	RegisterTranslationCb(this, TranslateForm);
	Width = appSettings.trayNotifier.iWidth;
	Height = appSettings.trayNotifier.iHeight;
	this->ActiveControl = btnStopFocus;
}
//---------------------------------------------------------------------------

void TfrmTrayNotifier::SetData(unsigned int callUid, AnsiString description, AnsiString uri, bool incoming)
{
	this->callUid = callUid;
	lblDescription->Caption = description;
	lblUri->Caption = uri;
	btnAnswer->Visible = incoming && appSettings.trayNotifier.elements.btnAnswer.visible;
	this->ActiveControl = btnStopFocus;	
}

void TfrmTrayNotifier::ShowWithoutFocus(void)
{
	Left = appSettings.trayNotifier.iPosX;
	Top = appSettings.trayNotifier.iPosY;
	Width = appSettings.trayNotifier.iWidth;
	Height = appSettings.trayNotifier.iHeight;

	bool monitorFound = false;
	for (int i=0; i<Screen->MonitorCount; i++)
	{
		TMonitor *monitor = Screen->Monitors[i];
		enum { MARGIN = 30 };
		if (
			(Left + Width + MARGIN >= monitor->Left) &&
			(Left - MARGIN <= monitor->Left + monitor->Width) &&
			(Top + 5 >= monitor->Top) &&
			(Top - MARGIN <= monitor->Top + monitor->Height)
			)
		{
			monitorFound = true;
			break;
		}
	}
	if (!monitorFound)
	{
		if (Screen->MonitorCount > 0)
		{
			TMonitor *monitor = Screen->Monitors[0];
			if (monitor)
			{
           		enum { MARGIN = 30 };
				LOG("Moving tray notifier window to the first monitor\n");
				Left = monitor->Left + monitor->Width - Width - MARGIN;
				Top = monitor->Top + monitor->Height - Height - MARGIN;
			}
		}
	}

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

	const TrayNotifierConf::Elements &el = appSettings.trayNotifier.elements;
	lblDescription->Visible = el.labelDescription.visible;
	lblDescription->Left = el.labelDescription.left;
	lblDescription->Top = el.labelDescription.top;
	lblDescription->Width = el.labelDescription.width;

	lblUri->Visible = el.labelUri.visible;
	lblUri->Left = el.labelUri.left;
	lblUri->Top = el.labelUri.top;
	lblUri->Width = el.labelUri.width;

	btnAnswer->Visible = el.btnAnswer.visible;
	btnAnswer->Left = el.btnAnswer.left;
	btnAnswer->Top = el.btnAnswer.top;
	btnAnswer->Width = el.btnAnswer.width;

	btnHangup->Visible = el.btnHangup.visible;
	btnHangup->Left = el.btnHangup.left;
	btnHangup->Top = el.btnHangup.top;
	btnHangup->Width = el.btnHangup.width;
}

void TfrmTrayNotifier::HideWindow(void)
{
	if (appSettings.trayNotifier.doNotChangePosition == false)
	{
		appSettings.trayNotifier.iPosX = frmTrayNotifier->Left;
		appSettings.trayNotifier.iPosY = frmTrayNotifier->Top;
	}
	Visible = false;
	ShowWindow(Handle, SW_HIDE);
}

void __fastcall TfrmTrayNotifier::btnHangupClick(TObject *Sender)
{
	if (OnHangup && callUid != 0)
		OnHangup(callUid, 486, "Busy Here");
	else
		HideWindow();
}
//---------------------------------------------------------------------------

void __fastcall TfrmTrayNotifier::btnAnswerClick(TObject *Sender)
{
	if (OnAnswer && callUid != 0)
	{
		if (appSettings.Calls.autoSwitchToCallAnsweredFromTray)
			Calls::SetCurrentCallUid(callUid);	// switch to new call
		OnAnswer(callUid);
	}
	else
		HideWindow();
}
//---------------------------------------------------------------------------

void __fastcall TfrmTrayNotifier::FormCreate(TObject *Sender)
{
	this->FormStyle = fsStayOnTop;
	Left = appSettings.trayNotifier.iPosX;
	Top = appSettings.trayNotifier.iPosY;
}
//---------------------------------------------------------------------------

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
	if (appSettings.trayNotifier.doNotChangePosition == false)
	{
		appSettings.trayNotifier.iPosX = frmTrayNotifier->Left;
		appSettings.trayNotifier.iPosY = frmTrayNotifier->Top;
	}
}
//---------------------------------------------------------------------------


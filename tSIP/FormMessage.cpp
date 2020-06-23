#include <vcl.h>
#pragma hdrstop

#include "FormMessage.h"
#include "SIMPLE_Messages.h"
#include "Settings.h"
#include "ControlQueue.h"

#include <stdio.h>
#include <time.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMessage *frmMessage;

namespace
{
	int nextRequestId = 0;
}

//---------------------------------------------------------------------------
__fastcall TfrmMessage::TfrmMessage(TComponent* Owner)
	: TForm(Owner),
	targetSet(false),
	incoming(false)
{
	SIMPLE_Messages::RegisterWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TfrmMessage::MyWndProc (Messages::TMessage &Msg)
{
    if (Msg.Msg == WM_NOTIFY)
    {
		if (((NMHDR*)Msg.LParam)->code == EN_LINK)
        {
            ENLINK *E = (ENLINK*)Msg.LParam;
            if (E->msg == WM_LBUTTONDBLCLK)
				ShellExecute (NULL, NULL, memoMain->Text.SubString(E->chrg.cpMin + 1, E->chrg.cpMax - E->chrg.cpMin).c_str(), NULL, NULL, SW_SHOW);
		}
    }
	else
	{
		WndProc (Msg);
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmMessage::memoInputKeyDown(TObject *Sender, WORD &Key,
        TShiftState Shift)
{
	if (Key == VK_RETURN && !Shift.Contains(ssCtrl) && !memoInput->WantReturns && memoInput->Text != "")
	{
		_SendMsg();
	}
}
//---------------------------------------------------------------------------
void TfrmMessage::_SendMsg()
{
	targetSet = true;
	edTarget->Enabled = false;
	AnsiString target = edTarget->Text;
	AnsiString msg = memoInput->Lines->Text;

	int requestId = nextRequestId;
	nextRequestId++;

	requestIds.insert(requestId);
	UA->SendMessage(requestId, 0, target, ::AnsiToUtf8(msg));

    memoMain->SelStart = memoMain->Lines->Text.Length();

    memoMain->SelAttributes->Size = 8;
    memoMain->SelAttributes->Style = TFontStyles() << fsBold;
    memoMain->SelAttributes->Color = clDkGray;
    memoMain->Paragraph->FirstIndent = 0;
    memoMain->Lines->Add ("Me: (" + TimeToStr (Time())+ ") ");

    memoMain->SelAttributes->Size = 10;
    memoMain->SelAttributes->Style = TFontStyles();
    memoMain->SelAttributes->Color = clBlue;
    memoMain->Paragraph->FirstIndent = 10;
    memoMain->Lines->Add (msg);

    SendMessage (memoMain->Handle, WM_VSCROLL, SB_BOTTOM, NULL);

    memoInput->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMessage::FormCreate(TObject *Sender)
{
    SetWindowLong (Handle, GWL_EXSTYLE, GetWindowLong (Handle, GWL_EXSTYLE) | WS_EX_APPWINDOW);

    SendMessage (memoMain->Handle, EM_AUTOURLDETECT, TRUE, 0);
    SendMessage (memoMain->Handle, EM_SETEVENTMASK, 0, ENM_LINK);
    WindowProc = MyWndProc;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMessage::btnSendOnEnterClick(TObject *Sender)
{
	memoInput->WantReturns = !btnSendOnEnter->Down;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMessage::btnSendClick(TObject *Sender)
{
	if (memoInput->Lines->Text != "")
	{
		_SendMsg();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmMessage::FormClose(TObject *Sender, TCloseAction &Action)
{
	SIMPLE_Messages::UnregisterWindow(this);
	Action = caFree;
}
//---------------------------------------------------------------------------

void TfrmMessage::SetTarget(AnsiString target)
{
	UpdateTarget(target);
	targetSet = true;
	edTarget->Text = target;
	edTarget->Enabled = false;
}

void TfrmMessage::AddIncomingMessage(AnsiString contentType, AnsiString body)
{
    targetSet = true;
	if (contentType == "text/plain")
	{
    	body = ::Utf8ToAnsi(body);
	}
    memoMain->SelStart = memoMain->Lines->Text.Length();
    memoMain->SelAttributes->Size = 8;
    memoMain->SelAttributes->Style = TFontStyles() << fsBold;
    memoMain->SelAttributes->Color = clBlack;
    memoMain->Paragraph->FirstIndent = 0;

    int current_time = time(NULL);
    char buf [30];
	tm *t;
	t = localtime ((long*)&current_time);
	strftime (buf, sizeof (buf), "%H:%M:%S", t);
	int TODO__PEER_DISPLAY;
	memoMain->Lines->Add ((AnsiString)"Peer: (" + buf + ") ");

    memoMain->SelAttributes->Size = 10;
    memoMain->SelAttributes->Style = TFontStyles();
    memoMain->SelAttributes->Color = clRed;
    memoMain->Paragraph->FirstIndent = 10;
    memoMain->Lines->Add (body);

    SendMessage (memoMain->Handle, WM_VSCROLL, SB_BOTTOM, NULL);

	FLASHWINFO f = {sizeof (f), Handle, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0};
	if (GetForegroundWindow() != Handle)
    {
        FlashWindowEx (&f);
//            PlaySound(..., NULL, SND_ASYNC | SND_FILENAME);
	}
}

void __fastcall TfrmMessage::edTargetChange(TObject *Sender)
{
	UpdateTarget(edTarget->Text);
}
//---------------------------------------------------------------------------

void TfrmMessage::UpdateTarget(AnsiString val)
{
	if (!targetSet)
	{
		target = val;
		AnsiString caption;
		caption.sprintf("Text: %s", target.c_str());
		this->Caption = caption;
	}
}

int TfrmMessage::HandleMessageStatus(int requestUid, int requestError, int sipCode, AnsiString reason)
{
	std::set<int>::iterator iter;
	iter = requestIds.find(requestUid);
	if (iter == requestIds.end())
		return -1;

	AnsiString statusText;
	if (requestError != 0)
	{
		statusText.sprintf("Error %d sending request", requestError);
	}
	else
	{
		if (sipCode != 200)
		{
			if (reason.Length() == 0)
			{
				statusText.sprintf("Received SIP/%d answer", sipCode);
			}
			else
			{
				statusText.sprintf("Received SIP/%d answer, %s", sipCode, reason.c_str());
			}
		}
	}

	if (statusText != "")
	{
		memoMain->SelStart = memoMain->Lines->Text.Length();
		memoMain->SelAttributes->Size = 8;
		memoMain->SelAttributes->Style = TFontStyles() << fsItalic;
		memoMain->SelAttributes->Color = clGray;
		memoMain->Paragraph->FirstIndent = 0;
		memoMain->Lines->Add (statusText);
	}

	requestIds.erase(iter);
	return 0;
}

void TfrmMessage::SetText(AnsiString text)
{
	memoInput->Text = text;
}

	


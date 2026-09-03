#include <vcl.h>
#pragma hdrstop

#include "FormMessage.h"
#include "SIMPLE_Messages.h"
#include "MessageHistory.h"
#include "Settings.h"
#include "ControlQueue.h"

#include <stdio.h>
#include <string.h>
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
	incoming(false),
	loadedHistoryCount(0),
	hasMoreHistory(false),
	loadingMoreHistory(false)
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
	bool firstFix = !targetSet;
	targetSet = true;
	edTarget->Enabled = false;
	AnsiString target = edTarget->Text;
	AnsiString msg = memoInput->Lines->Text;

	if (firstFix)
	{
		/* target was just fixed for the first time (new message window
		 * with a manually entered target); load its history now, before
		 * displaying the message about to be sent */
		LoadHistory();
	}

	int requestId = nextRequestId;
	nextRequestId++;

	requestIds.insert(requestId);
	AnsiString utf8Msg = ::AnsiToUtf8(msg);
	UA->SendMessage(requestId, 0, target, utf8Msg);
	MessageHistory::Append(target, false, "text/plain", utf8Msg);
	loadedHistoryCount++;

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
	LoadHistory();
}

namespace
{
	const unsigned int HISTORY_PAGE_SIZE = 20;
}

void TfrmMessage::LoadHistory(void)
{
	std::vector<MessageHistory::Entry> entries = MessageHistory::LoadLast(target, HISTORY_PAGE_SIZE);
	for (size_t i = 0; i < entries.size(); i++)
	{
		AppendHistoryEntry(entries[i].incoming, entries[i].time, entries[i].contentType, entries[i].body);
	}
	if (!entries.empty())
	{
		SendMessage (memoMain->Handle, WM_VSCROLL, SB_BOTTOM, NULL);
	}

	loadedHistoryCount = entries.size();
	hasMoreHistory = (entries.size() == HISTORY_PAGE_SIZE);
	tmrHistoryScroll->Enabled = hasMoreHistory;
}

void TfrmMessage::CheckHistoryScrollTop(void)
{
	if (loadingMoreHistory || !hasMoreHistory)
	{
		return;
	}

	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE;
	if (!GetScrollInfo(memoMain->Handle, SB_VERT, &si))
	{
		return;
	}

	if (si.nPos <= si.nMin)
	{
		LoadMoreHistory();
	}
}

void __fastcall TfrmMessage::tmrHistoryScrollTimer(TObject *Sender)
{
	CheckHistoryScrollTop();
}

void TfrmMessage::InsertHistoryEntryAtTop(int &insertPos, bool incoming, time_t t, AnsiString contentType, AnsiString utf8Body)
{
	AnsiString displayBody = (contentType == "text/plain") ? ::Utf8ToAnsi(utf8Body) : utf8Body;

	const tm *tv = localtime(&t);
	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tv);

	AnsiString headerLine = (incoming ? (AnsiString)"Peer: (" : (AnsiString)"Me: (") + buf + ") \r\n";
	memoMain->SelStart = insertPos;
	memoMain->SelLength = 0;
	memoMain->SelAttributes->Size = 8;
	memoMain->SelAttributes->Style = TFontStyles() << fsBold;
	memoMain->SelAttributes->Color = incoming ? clBlack : clDkGray;
	memoMain->Paragraph->FirstIndent = 0;
	memoMain->SelText = headerLine;
	insertPos += headerLine.Length();

	AnsiString bodyLine = displayBody + "\r\n";
	memoMain->SelStart = insertPos;
	memoMain->SelLength = 0;
	memoMain->SelAttributes->Size = 10;
	memoMain->SelAttributes->Style = TFontStyles();
	memoMain->SelAttributes->Color = incoming ? clRed : clBlue;
	memoMain->Paragraph->FirstIndent = 10;
	memoMain->SelText = bodyLine;
	insertPos += bodyLine.Length();
}

void TfrmMessage::LoadMoreHistory(void)
{
	if (loadingMoreHistory || !hasMoreHistory)
	{
		return;
	}
	loadingMoreHistory = true;

	std::vector<MessageHistory::Entry> entries = MessageHistory::LoadOlder(target, loadedHistoryCount, HISTORY_PAGE_SIZE);
	if (entries.empty())
	{
		hasMoreHistory = false;
		tmrHistoryScroll->Enabled = false;
		loadingMoreHistory = false;
		return;
	}

	int firstVisibleLineBefore = SendMessage(memoMain->Handle, EM_GETFIRSTVISIBLELINE, 0, 0);
	int lineCountBefore = SendMessage(memoMain->Handle, EM_GETLINECOUNT, 0, 0);

	int insertPos = 0;
	for (size_t i = 0; i < entries.size(); i++)
	{
		InsertHistoryEntryAtTop(insertPos, entries[i].incoming, entries[i].time, entries[i].contentType, entries[i].body);
	}

	int lineCountAfter = SendMessage(memoMain->Handle, EM_GETLINECOUNT, 0, 0);
	int linesInserted = lineCountAfter - lineCountBefore;

	int firstVisibleLineAfter = SendMessage(memoMain->Handle, EM_GETFIRSTVISIBLELINE, 0, 0);
	int desiredFirstVisibleLine = firstVisibleLineBefore + linesInserted;
	SendMessage(memoMain->Handle, EM_LINESCROLL, 0, desiredFirstVisibleLine - firstVisibleLineAfter);

	loadedHistoryCount += entries.size();
	hasMoreHistory = (entries.size() == HISTORY_PAGE_SIZE);
	tmrHistoryScroll->Enabled = hasMoreHistory;

	loadingMoreHistory = false;
}

void TfrmMessage::AppendHistoryEntry(bool incoming, time_t t, AnsiString contentType, AnsiString utf8Body)
{
	AnsiString displayBody = (contentType == "text/plain") ? ::Utf8ToAnsi(utf8Body) : utf8Body;

	const tm *tv = localtime(&t);
	char buf[32];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tv);

	memoMain->SelStart = memoMain->Lines->Text.Length();
	memoMain->SelAttributes->Size = 8;
	memoMain->SelAttributes->Style = TFontStyles() << fsBold;
	memoMain->SelAttributes->Color = incoming ? clBlack : clDkGray;
	memoMain->Paragraph->FirstIndent = 0;
	memoMain->Lines->Add ((incoming ? (AnsiString)"Peer: (" : (AnsiString)"Me: (") + buf + ") ");

	memoMain->SelAttributes->Size = 10;
	memoMain->SelAttributes->Style = TFontStyles();
	memoMain->SelAttributes->Color = incoming ? clRed : clBlue;
	memoMain->Paragraph->FirstIndent = 10;
	memoMain->Lines->Add (displayBody);
}

void TfrmMessage::AddIncomingMessage(AnsiString contentType, AnsiString body)
{
    targetSet = true;

	MessageHistory::Append(target, true, contentType, body);
	loadedHistoryCount++;

	if (contentType == "text/plain")
	{
    	body = ::Utf8ToAnsi(body);
	}
    memoMain->SelStart = memoMain->Lines->Text.Length();
    memoMain->SelAttributes->Size = 8;
    memoMain->SelAttributes->Style = TFontStyles() << fsBold;
    memoMain->SelAttributes->Color = clBlack;
    memoMain->Paragraph->FirstIndent = 0;

	time_t currentTime = time(NULL);
	const tm *t = localtime(&currentTime);
	char buf [30];
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

	


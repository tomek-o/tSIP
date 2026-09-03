#ifndef FormMessageH
#define FormMessageH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>

#include <set>
#include <time.h>
//---------------------------------------------------------------------------
class TfrmMessage : public TForm
{
__published:	// IDE-managed Components
	TRichEdit *memoMain;
	TPanel *pnlBottom;
    TMemo *memoInput;
	TPanel *pnlTop;
	TLabel *lblNumber;
	TEdit *edTarget;
	TPanel *pnlButtons;
	TButton *btnSend;
	TSpeedButton *btnSendOnEnter;
	TTimer *tmrHistoryScroll;
    void __fastcall memoInputKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall btnSendOnEnterClick(TObject *Sender);
    void __fastcall btnSendClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall edTargetChange(TObject *Sender);
	void __fastcall tmrHistoryScrollTimer(TObject *Sender);
private:	// User declarations
	AnsiString target;
	bool targetSet;
	bool incoming;
	void UpdateTarget(AnsiString val);
	std::set<int> requestIds;
	unsigned int loadedHistoryCount;
	bool hasMoreHistory;
	bool loadingMoreHistory;
	void LoadHistory(void);
	void LoadMoreHistory(void);
	void CheckHistoryScrollTop(void);
	void AppendHistoryEntry(bool incoming, time_t t, AnsiString contentType, AnsiString utf8Body);
	void InsertHistoryEntryAtTop(int &insertPos, bool incoming, time_t t, AnsiString contentType, AnsiString utf8Body);
public:		// User declarations
	__fastcall TfrmMessage(TComponent* Owner);
	void __fastcall MyWndProc (Messages::TMessage &Msg);
	void _SendMsg();
	void SetTarget(AnsiString target);
	AnsiString GetTarget(void) const {
		return target;
	}
	void SetIncoming(void) {
		incoming = true;
	}
	bool GetIncoming(void) const {
		return incoming;
	}
	void AddIncomingMessage(AnsiString contentType, AnsiString body);
	int HandleMessageStatus(int requestUid, int requestError, int sipCode, AnsiString reason);
	void SetText(AnsiString text);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMessage *frmMessage;
//---------------------------------------------------------------------------
#endif


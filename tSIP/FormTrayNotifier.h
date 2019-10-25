//---------------------------------------------------------------------------

#ifndef FormTrayNotifierH
#define FormTrayNotifierH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmTrayNotifier : public TForm
{
__published:	// IDE-managed Components
	TLabel *lblDescription;
	TLabel *lblUri;
	TButton *btnAnswer;
	TButton *btnHangup;
	TImage *imgBackground;
	TButton *btnStopFocus;
	void __fastcall btnHangupClick(TObject *Sender);
	void __fastcall btnAnswerClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
	typedef void(__closure *CallbackHangup)(void);
	typedef void(__closure *CallbackAnswer)(void);
	void __fastcall Close(void);
	void __fastcall CreateParams(TCreateParams &Params);	
public:		// User declarations
	__fastcall TfrmTrayNotifier(TComponent* Owner);
	void SetData(AnsiString description, AnsiString uri, bool incoming);
	void ShowWithoutFocus(void);
    void HideWindow(void);
	void UpdateBackgroundImage(void);
	CallbackHangup OnHangup;
	CallbackAnswer OnAnswer;
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmTrayNotifier *frmTrayNotifier;
//---------------------------------------------------------------------------
#endif

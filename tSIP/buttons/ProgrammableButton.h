//---------------------------------------------------------------------------
#ifndef ProgrammableButtonH
#define ProgrammableButtonH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include "baresip_dialog_info_status.h"
#include "baresip_dialog_info_direction.h"
#include "baresip_presence_status.h"
#include "common/Colors.h"
#include "ButtonConf.h"

class PACKAGE TProgrammableButton : public TPanel
{
private:
	TLabel *label;
	TLabel *label2;
	TImage *image;
	TImageList *imgList;
	AnsiString description;
	AnsiString user;
	enum dialog_info_status state;
	enum presence_status presence_state;
	bool down;
	bool inactive;
	bool visible;
	bool customFrame;
	bool labelCenterHorizontally;
	bool centerTextVertically;
	int labelTop;

    bool label2CenterHorizontally;

	bool centerImageVertically;
	int imageTop;
	int bevelWidth;
	int scalingPercentage;
	bool once;

	ButtonConf::Color colors[ButtonConf::EL_LIMITER];

	int configuredLines;
	bool spaceLabelsYEqually;
	AnsiString caption2;
	int label2Top;

    bool mouseDownState;

	bool raised;	// helps with flickering
	void Lower(void);
	void Raise(void);

	Graphics::TBitmap *bmpIdle;
	Graphics::TBitmap *bmpTerminated;
	Graphics::TBitmap *bmpEarly;
	Graphics::TBitmap *bmpConfirmed;

	void SetLines(int cnt);
	void SetImage(Graphics::TBitmap *bmp);
	void UpdateColors(void);
	void SetImageTop(void);
	void SetLabelTop(void);

	typedef void (__closure *CallbackMouseUpDown)(TProgrammableButton* btn);
	CallbackMouseUpDown onMouseUpDownCb;

protected:
	void __fastcall MouseEnter(TObject *Sender);
	void __fastcall MouseLeave(TObject *Sender);
	void __fastcall MouseUpHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall MouseDownHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
	void __fastcall Paint(void);
public:
	__fastcall TProgrammableButton(TComponent* Owner, TImageList* imgList, int scalingPercentage);
	__fastcall ~TProgrammableButton();
	void SetConfig(const ButtonConf &cfg);
	void SetCaption(AnsiString text);
	void SetCaption2(AnsiString text);
	void SetState(enum dialog_info_status state, bool updateRemoteIdentity, enum dialog_info_direction direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay);
	enum dialog_info_status GetState(void) {
		return state;
	}
	void SetDown(bool state);
	bool GetDown(void)
	{
    	return down;
	}
	bool GetMouseDown(void) const
	{
		return mouseDownState;
	}
	void SetInactive(bool state);
	bool GetInactive(void)
	{
		return inactive;
	}
	void SetVisible(bool state);
	bool GetVisible(void)
	{
		return visible;
	}
	void SetImage(AnsiString file);
	void SetMwiState(int newMsg, int oldMsg);
	void SetPresenceState(enum presence_status state, AnsiString note);
	void ClearPresenceState(void);
	void UpdateCallbacks(void);
	void SetScaling(int percentage) {
		if (scalingPercentage != percentage)
		{
			scalingPercentage = percentage;
		}
	}
	void SetMouseUpDownCallback(CallbackMouseUpDown cb)
	{
		onMouseUpDownCb = cb;
	}
__published:
};
//---------------------------------------------------------------------------
#endif

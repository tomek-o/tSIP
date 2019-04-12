//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "ProgrammableButton.h"
#include "ButtonConf.h"
#include "Paths.h"
#pragma package(smart_init)

// #define ON_MOUSE_ENTER_LEAVE

namespace
{
	void LoadBitmap(Graphics::TBitmap *bmp, AnsiString name)
	{
		try
		{
			AnsiString fname = Paths::GetFullImgName(name);
			if (FileExists(fname))
			{
				bmp->LoadFromFile(fname);
			}
		} catch(...) {}
	}
}

//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(TProgrammableButton *)
{
	new TProgrammableButton(NULL, new TImageList(NULL), 100);
}

__fastcall TProgrammableButton::TProgrammableButton(TComponent* Owner, TImageList* imgList, int scalingPercentage)
	: TPanel(Owner), imgList(imgList), state(DIALOG_INFO_UNKNOWN),
	down(false), scalingPercentage(scalingPercentage),
	once(false),
	configuredLines(1),
	raised(true),
	bmpIdle(NULL), bmpTerminated(NULL), bmpEarly(NULL), bmpConfirmed(NULL)
{
	assert(imgList);

    //this->Color = clYellow;

    this->DoubleBuffered = true;

	this->Height = 32;
	this->Margins->Left = 0;
	this->Margins->Right = 0;
	this->Margins->Top = 0;
	this->Margins->Bottom = 0;
	this->AlignWithMargins = true;

#ifdef ON_MOUSE_ENTER_LEAVE
	OnMouseEnter = MouseEnter;
	OnMouseLeave = MouseLeave;
#endif
	OnMouseUp = MouseUpHandler;
	OnMouseDown = MouseDownHandler;

	label = new TLabel(this);
	label->Parent = this;
	label->Left = 20;
	label->Top = 8;
	//label->AutoSize = false;
	//label->Width = this->Width - label->Left - 5;
#ifdef ON_MOUSE_ENTER_LEAVE
	label->OnMouseEnter = MouseEnter;
	label->OnMouseLeave = MouseLeave;
#endif
	label->OnMouseUp = MouseUpHandler;
	label->OnMouseDown = MouseDownHandler;
	label->BringToFront();
	//label->Transparent = true;

	label2 = new TLabel(this);
	label2->Parent = this;
	label2->Left = 20;
	label2->Top = 26;
	//label2->AutoSize = false;
	//label2->Width = this->Width - label->Left - 5;
#ifdef ON_MOUSE_ENTER_LEAVE
	label2->OnMouseEnter = MouseEnter;
	label2->OnMouseLeave = MouseLeave;
#endif
	label2->OnMouseUp = MouseUpHandler;
	label2->OnMouseDown = MouseDownHandler;
	label2->Visible = false;
	label2->BringToFront();
	//label2->Transparent = true;

	image = new TImage(this);
	image->Parent = this;
	image->Left = 2;
	image->Top = 8;
	image->Width = 0;
	image->Height = 0;
	image->Picture->Bitmap->PixelFormat = pf24bit;
	image->Transparent = true;
#ifdef ON_MOUSE_ENTER_LEAVE
	image->OnMouseEnter = MouseEnter;
	image->OnMouseLeave = MouseLeave;
#endif
	image->OnMouseUp = MouseUpHandler;
	image->OnMouseDown = MouseDownHandler;

	bmpIdle = new Graphics::TBitmap();
	bmpTerminated = new Graphics::TBitmap();
	bmpEarly = new Graphics::TBitmap();
	bmpConfirmed = new Graphics::TBitmap();
}

__fastcall TProgrammableButton::~TProgrammableButton()
{
	delete bmpIdle;
	delete bmpTerminated;
	delete bmpEarly;
	delete bmpConfirmed;
}

//---------------------------------------------------------------------------
namespace ProgrammableButton
{
	void __fastcall PACKAGE Register()
	{
		TComponentClass classes[1] = {__classid(TProgrammableButton)};
		RegisterComponents("Samples", classes, 0);
	}
}
//---------------------------------------------------------------------------

void __fastcall TProgrammableButton::MouseEnter(TObject *Sender)
{
	Lower();
}

void __fastcall TProgrammableButton::MouseLeave(TObject *Sender)
{
	Raise();
}

void __fastcall TProgrammableButton::MouseUpHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Raise();
}

void __fastcall TProgrammableButton::MouseDownHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Lower();
}


void TProgrammableButton::SetConfig(const ButtonConf &cfg)
{
    int percentage;
	if (!once)
	{
		percentage = 100;
	}
	else
	{
		percentage = scalingPercentage;
    }
	label->Caption = cfg.caption.c_str();
	label2->Caption = cfg.caption2.c_str();
	user = cfg.number.c_str();
	if (cfg.noIcon)
	{
		image->Width = 0 * percentage/100;
		label->Left = 2 * percentage/100;
	}
	else
	{
		image->Width = 0;
		label->Left = 20 * percentage/100;
	}
	Height = cfg.height * percentage/100;
	label->Top = (Height - label->Height)/2;
	label2->Left = label->Left;
	//image->Top = label->Top;

	this->Margins->Top = cfg.marginTop;
	this->Margins->Bottom = cfg.marginBottom;

	this->Color = static_cast<TColor>(cfg.backgroundColor);

	LoadBitmap(bmpIdle, cfg.imgIdle.c_str());
	LoadBitmap(bmpTerminated, cfg.imgTerminated.c_str());
	LoadBitmap(bmpEarly, cfg.imgEarly.c_str());
	LoadBitmap(bmpConfirmed, cfg.imgConfirmed.c_str());

	SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");
	//SetPresenceState(PRESENCE_UNKNOWN, "");
	presence_state = PRESENCE_UNKNOWN;

    // set inside SetState
	//image->Top = ((Height  * scalingPercentage/100) - image->Height)/2;
	configuredLines = cfg.captionLines;
	caption2 = cfg.caption2.c_str();
	SetLines(cfg.captionLines);

	once = true;	
}

void TProgrammableButton::UpdateCallbacks(void)
{
	image->OnClick = OnClick;
	label->OnClick = OnClick;

	image->OnDblClick = OnDblClick;
	label->OnDblClick = OnDblClick;
}

void TProgrammableButton::SetCaption(AnsiString text)
{
	label->Caption = text;

}
void TProgrammableButton::SetState(enum dialog_info_status state, bool updateRemoteIdentity, enum dialog_info_direction direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay)
{
	this->state = state;
	switch (state)
	{
	case DIALOG_INFO_TERMINATED:
		SetImage(bmpTerminated);
		break;
	case DIALOG_INFO_EARLY:
		SetImage(bmpEarly);
		break;
	case DIALOG_INFO_CONFIRMED:
		SetImage(bmpConfirmed);
		break;
	default: // DIALOG_INFO_UNKNOWN included
		SetImage(bmpIdle);
	}
	if (updateRemoteIdentity)
	{
		if (remoteIdentityDisplay.Length() || remoteIdentity.Length())
		{
			AnsiString ridCaption;
			if (direction == DIALOG_INFO_DIR_INITIATOR)
			{
				ridCaption = "-> ";
			}
			else if (direction == DIALOG_INFO_DIR_RECIPIENT)
			{
				ridCaption = "<- ";
			}
			if (remoteIdentityDisplay.Length())
			{
				ridCaption += remoteIdentityDisplay;
			}
			else
			{
				ridCaption += remoteIdentity;
			}
			label2->Caption = ridCaption;
			SetLines(2);
		}
		else
		{
			SetLines(configuredLines);
			if (configuredLines == 2)
			{
				label2->Caption = caption2;
			}
		}
	}
}

void TProgrammableButton::SetPresenceState(enum presence_status state, AnsiString note)
{
	this->presence_state = state;
	switch (state)
	{
	case PRESENCE_OPEN:	// "Online"
		SetImage(bmpTerminated);
		break;
	case PRESENCE_CLOSED:	// "Offline"
		SetImage(bmpEarly);
		break;
	case PRESENCE_BUSY:
		SetImage(bmpConfirmed);
		break;
	default: // PRESENCE_UNKNOWN included
		SetImage(bmpIdle);
	}

	label2->Caption = note;
	if (note != "")
	{
		SetLines(2);
	}
	else
	{
		SetLines(1);
	}
}

void TProgrammableButton::ClearPresenceState(void)
{
	this->presence_state = PRESENCE_UNKNOWN;
	SetImage(bmpIdle);
}

void TProgrammableButton::SetMwiState(int newMsg, int oldMsg)
{
	AnsiString caption;
	caption.sprintf("%d / %d", newMsg, oldMsg);
	label->Caption = caption;
	if (newMsg)
		SetImage(bmpEarly);
	else
		SetImage(bmpIdle);
}

void TProgrammableButton::SetDown(bool state)
{
	down = state;
	if (down)
	{
		Lower();
	}
	else
	{
		Raise();
	}
}

void TProgrammableButton::SetImage(AnsiString file)
{
	if (file == "")
	{
		image->Picture = NULL;	// clear picture
	}
	else
	{
		LoadBitmap(bmpIdle, file);
		SetImage(bmpIdle);
	}
}

void TProgrammableButton::Lower(void)
{
	if (raised == false)
		return;
	raised = false;			
	BevelOuter = bvLowered;
}

void TProgrammableButton::Raise(void)
{
	if (raised)
		return;
	if (down == false)
	{
        raised = true;
		BevelOuter = bvRaised;
	}		
}

void TProgrammableButton::SetLines(int cnt)
{
	if (cnt == 2)
	{
		//image->Top = (Height - image->Height)/2;

		//label->Top = (Height - label->Height)/3;
		//label2->Top = (Height - label->Height)*2/3;

		label->Top = (Height - label->Height - label2->Height)/3;
		label2->Top = label->Top + label->Height + label->Top;

		label2->Visible = true;
	}
	else
	{
		label->Top = (Height - label->Height)/2;
		//image->Top = (Height - image->Height)/2;
		label2->Visible = false;
	}
}

void TProgrammableButton::SetImage(Graphics::TBitmap *bmp)
{
	int percentage = 100;
	if (!once)
	{
		percentage = scalingPercentage;
    }
	image->Picture = NULL;	// clear picture
	if (bmp->Height > 0 && bmp->Width > 0)
	{
		image->Width = bmp->Width;
		image->Height = bmp->Height;
		//image->Top = (Height - image->Height)/2;// * percentage/100;
		image->Top = (Height - (image->Height * 100/percentage))/2;// * percentage/100;
	}
	image->Picture->Bitmap->PixelFormat = pf24bit;
	image->Picture->Bitmap = bmp;
	label->BringToFront();
	label->Transparent = true;
	label2->BringToFront();
	label2->Transparent = true;
}





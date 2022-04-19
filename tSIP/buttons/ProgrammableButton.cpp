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
			else
			{
            	//bmp->
			}
		}
		catch(...)
		{
			//bmp->
		}
	}
}

__fastcall TProgrammableButton::TProgrammableButton(TComponent* Owner, TImageList* imgList, int scalingPercentage)
	: TPanel(Owner), imgList(imgList), state(DIALOG_INFO_UNKNOWN),
	down(false),
	inactive(false), visible(false),
	label2CenterHorizontally(true),
	scalingPercentage(scalingPercentage),
	once(false),
	configuredLines(1),
	raised(true),
	onMouseUpDownCb(NULL),
	bmpIdle(NULL), bmpTerminated(NULL), bmpEarly(NULL), bmpConfirmed(NULL)
{
	assert(imgList);

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
	centerImageVertically = true;
	imageTop = 8;
	SetImageTop();

	SetLabelTop();
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
	if (inactive)
		return;
	Lower();
}

void __fastcall TProgrammableButton::MouseLeave(TObject *Sender)
{
	if (inactive)
		return;
	Raise();
}

void __fastcall TProgrammableButton::MouseUpHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	mouseDownState = false;
	if (onMouseUpDownCb)
		onMouseUpDownCb(this);
	if (inactive)
		return;
	Raise();
}

void __fastcall TProgrammableButton::MouseDownHandler(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	mouseDownState = true;
	if (onMouseUpDownCb)
		onMouseUpDownCb(this);
	if (inactive)
		return;
	Lower();
}

void __fastcall TProgrammableButton::Paint(void)
{
	TPanel::Paint();
	if (customFrame == false)
	{
		return;
	}
	if (raised == true)
	{
		if (inactive == false)
		{
			Canvas->Pen->Color = static_cast<TColor>(this->colors[ButtonConf::EL_FRAME].idle);
		}
		else
		{
			Canvas->Pen->Color = static_cast<TColor>(colors[ButtonConf::EL_FRAME].inactive);
		}
	}
	else
	{
		if (inactive == false)
		{
			Canvas->Pen->Color = static_cast<TColor>(colors[ButtonConf::EL_FRAME].down);
		}
		else
		{
			Canvas->Pen->Color = static_cast<TColor>(colors[ButtonConf::EL_FRAME].inactiveDown);
		}
	}
	Canvas->Pen->Width = bevelWidth;
	this->Canvas->Rectangle(0, 0, Width, Height);
}

void TProgrammableButton::SetConfig(const ButtonConf &cfg)
{
	int percentage;
	if (!once)
	{
		percentage = 100;
		SetDown(cfg.down);
		Visible = cfg.visible;
	}
	else
	{
		percentage = scalingPercentage;
	}
	SetInactive(cfg.inactive);
	SetVisible(cfg.visible);

	label->Caption = cfg.caption.c_str();
	label2->Caption = cfg.caption2.c_str();
	caption2 = cfg.caption2.c_str();

	user = cfg.number.c_str();

	label->Font->Name = cfg.font.name.c_str();
	label->Font->Size = cfg.font.size;
	label->Font->Style = TFontStyles();
	if (cfg.font.bold)
	{
		label->Font->Style << fsBold;
	}
	if (cfg.font.italic)
	{
		label->Font->Style << fsItalic;
	}
	if (cfg.font.underline)
	{
		label->Font->Style << fsUnderline;
	}


	label2->Left = cfg.label2Left;
	configuredLines = cfg.captionLines;
	spaceLabelsYEqually = cfg.spaceLabelsYEqually;
	label2->Font->Name = cfg.fontLabel2.name.c_str();
	label2->Font->Size = cfg.fontLabel2.size;
	label2->Font->Style = TFontStyles();
	if (cfg.fontLabel2.bold)
	{
		label2->Font->Style << fsBold;
	}
	if (cfg.fontLabel2.italic)
	{
		label2->Font->Style << fsItalic;
	}
	if (cfg.fontLabel2.underline)
	{
		label2->Font->Style << fsUnderline;
	}

	Left = cfg.left * percentage/100;
	Top = cfg.top * percentage/100;
	Width = cfg.width * percentage/100;
	Height = cfg.height * percentage/100;

	SetLines(cfg.captionLines);

	if (cfg.label2CenterHorizontally)
	{
		label2->Left = (Width - label2->Width)/2;
	}

	image->Transparent = cfg.imageTransparent;
	image->Left = cfg.imageLeft;
	image->Top = cfg.imageTop;

	labelCenterHorizontally = cfg.labelCenterHorizontally;
	if (cfg.labelCenterHorizontally)
	{
		label->Left = (Width - label->Width)/2;
		//label2->Left = label->Left;
	}
	else
	{
		label->Left = cfg.labelLeft;
	}
	centerTextVertically = cfg.labelCenterVertically;
	labelTop = cfg.labelTop;
	label2Top = cfg.label2Top; 

	centerImageVertically = cfg.imageCenterVertically;
	imageTop = cfg.imageTop;

	label2CenterHorizontally = cfg.label2CenterHorizontally;

	customFrame = cfg.customFrame;
	bevelWidth = cfg.bevelWidth;
	if (cfg.customFrame)
	{
		BorderWidth = 0;
		BevelWidth = 0;
		//BevelWidth = 5;
		BevelKind = bkNone;
		BevelInner = bvNone;
		BevelOuter = bvNone;
		BorderStyle = bsNone;
		//Ctl3D = false;
	}
	else
	{
		BevelKind = bkNone;
		BevelInner = bvNone;
		if (down)
		{
			Lower();
		}
		else
		{
			Raise();
		}
		BorderWidth = 0;
		BorderStyle = bsNone;
		BevelWidth = bevelWidth;
	}

	memcpy(colors, cfg.colors, sizeof(colors));
	UpdateColors();

	LoadBitmap(bmpIdle, cfg.imgIdle.c_str());
	LoadBitmap(bmpTerminated, cfg.imgTerminated.c_str());
	LoadBitmap(bmpEarly, cfg.imgEarly.c_str());
	LoadBitmap(bmpConfirmed, cfg.imgConfirmed.c_str());

	SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");
	//SetPresenceState(PRESENCE_UNKNOWN, "");
	presence_state = PRESENCE_UNKNOWN;

	//this->Repaint();
	//this->Invalidate();

	SetImageTop();

	once = true;	
}

void TProgrammableButton::UpdateCallbacks(void)
{
	image->OnClick = OnClick;
	label->OnClick = OnClick;
	label2->OnClick = OnClick;

	image->OnDblClick = OnDblClick;
	label->OnDblClick = OnDblClick;
	label2->OnDblClick = OnDblClick;
}

void TProgrammableButton::SetCaption(AnsiString text)
{
	label->Caption = text;
	if (labelCenterHorizontally)
	{
		label->Left = (Width - label->Width)/2;
		//label2->Left = label->Left;
	}
	//SetLabelTop();
}

void TProgrammableButton::SetCaption2(AnsiString text)
{
	label2->Caption = text;
	if (label2CenterHorizontally)
	{
		label2->Left = (Width - label2->Width)/2;
	}	
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
		SetLines(configuredLines);
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

void TProgrammableButton::SetInactive(bool state)
{
	if (inactive != state)
	{
		inactive = state;
		UpdateColors();
	}
}

void TProgrammableButton::SetVisible(bool state)
{
	if (visible != state)
	{
		visible = state;
		Visible = visible;
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
	UpdateColors();
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
	UpdateColors();
}

void TProgrammableButton::SetLines(int cnt)
{
	if (spaceLabelsYEqually == false)
	{
		if (cnt == 2)
		{
			SetLabelTop();
			label2->Top = label2Top;
			label2->Visible = true;
		}
		else
		{
			SetLabelTop();
			label2->Top = label2Top;
			label2->Visible = false;
		}
	}
	else
	{
		if (cnt == 2)
		{
			label->Top = (Height - label->Height - label2->Height)/3;
			label2->Top = label->Top + label->Height + label->Top;
			label2->Visible = true;
		}
		else
		{
			label->Top = (Height - label->Height)/2;
			label2->Top = label2Top;
			label2->Visible = false;
		}
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
	}
	SetImageTop();
	image->Picture->Bitmap->PixelFormat = pf24bit;
	image->Picture->Bitmap = bmp;
	label->BringToFront();
	label->Transparent = true;
	label2->BringToFront();
	label2->Transparent = true;
}


void TProgrammableButton::UpdateColors(void)
{
	TColor backgroundColor, fontColor;
	if (raised == true)
	{
		if (inactive == false)
		{
			backgroundColor = static_cast<TColor>(colors[ButtonConf::EL_BACKGROUND].idle);
			fontColor = static_cast<TColor>(colors[ButtonConf::EL_FONT].idle);
		}
		else
		{
			backgroundColor = static_cast<TColor>(colors[ButtonConf::EL_BACKGROUND].inactive);
			fontColor = static_cast<TColor>(colors[ButtonConf::EL_FONT].inactive);
        }
	}
	else
	{
		if (inactive == false)
		{
			backgroundColor = static_cast<TColor>(colors[ButtonConf::EL_BACKGROUND].down);
			fontColor = static_cast<TColor>(colors[ButtonConf::EL_FONT].down);
		}
		else
		{
			backgroundColor = static_cast<TColor>(colors[ButtonConf::EL_BACKGROUND].inactiveDown);
			fontColor = static_cast<TColor>(colors[ButtonConf::EL_FONT].inactiveDown);
        }
	}

	this->Color = backgroundColor;
	this->Font->Color = fontColor;
	label->Font->Color = fontColor;
	label2->Font->Color = fontColor;
}

void TProgrammableButton::SetImageTop(void)
{
	if (centerImageVertically)
	{
		//image->Top = (Height - image->Height)/2;
		image->Top = (Height - (image->Height * 100/scalingPercentage))/2;// * percentage/100;
	}
	else
	{
		image->Top = imageTop;
	}
}

void TProgrammableButton::SetLabelTop(void)
{
#if 0
	// center vertically
	if (label2->Visible == false)
	{
		label->Top = (Height - label->Height)/2;
	}
#endif
	if (centerTextVertically)
	{
		label->Top = (Height - label->Height)/2;
	}
	else
	{
		label->Top = labelTop;
	}
}




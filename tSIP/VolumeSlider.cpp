//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <math.h>
#include "VolumeSlider.h"

//---------------------------------------------------------------------------

namespace {

const double METER_FLOOR_DB = -90.0;
const double METER_RANGE_DB = 60.0;	// displayed range: -60..0 dBFS
const double METER_FAST_RELEASE_DB_PER_SEC = 40.0;
const double METER_SLOW_TAU_SEC = 0.3;
const int THUMB_HALF = 5;	// thumb height is 2 * THUMB_HALF + 1
const int TRACK_PAD = 6;	// keeps thumb inside the control at both ends

double ToDb(double value)
{
	if (value < 1.0)
		return METER_FLOOR_DB;
	return 20.0 * log10(value / 32768.0);
}

// Scale dB to 0..1 on displayed range
double ScalePos(double db)
{
	double pos = (db + METER_RANGE_DB) / METER_RANGE_DB;
	if (pos < 0.0)
		return 0.0;
	if (pos > 1.0)
		return 1.0;
	return pos;
}

// Low zone up to -12 dB, mid up to -3 dB, high above
TColor ZoneColor(double db, TColor low, TColor mid, TColor high)
{
	if (db > -3.0)
		return high;
	if (db > -12.0)
		return mid;
	return low;
}

}

//---------------------------------------------------------------------------

__fastcall VolumeSlider::VolumeSlider(TComponent* Owner)
	: TCustomControl(Owner),
	fMin(10),
	fMax(512),
	fPosition(384),
	fastDb(METER_FLOOR_DB),
	slowDb(METER_FLOOR_DB),
	dragging(false),
	cLow(clGreen),
	cMid(clYellow),
	cHigh(clRed),
	cPeak(clBlack),
	cTrack(clGray),
	cThumb(clSilver),
	buffer(new Graphics::TBitmap())
{
	// Paint covers the whole control; VCL must not erase it first (causes flicker)
	ControlStyle = ControlStyle << csOpaque;
	ParentColor = true;
	Width = 20;
	Height = 180;
}

__fastcall VolumeSlider::~VolumeSlider()
{
	delete buffer;
}

void VolumeSlider::SetPosition(int value)
{
	if (value < fMin)
		value = fMin;
	if (value > fMax)
		value = fMax;
	if (value == fPosition)
		return;
	fPosition = value;
	Invalidate();
	if (OnChange)
		OnChange(this);
}

void VolumeSlider::SetColors(TColor low, TColor mid, TColor high, TColor peak, TColor track, TColor thumb)
{
	cLow = low;
	cMid = mid;
	cHigh = high;
	cPeak = peak;
	cTrack = track;
	cThumb = thumb;
	Invalidate();
}

void VolumeSlider::SetMin(int value)
{
	fMin = value;
	SetPosition(fPosition);
}

void VolumeSlider::SetMax(int value)
{
	fMax = value;
	SetPosition(fPosition);
}

void VolumeSlider::PositionFromY(int y)
{
	int span = Height - 2 * TRACK_PAD;
	if (span < 1)
		return;
	double frac = (double)(y - TRACK_PAD) / span;
	if (frac < 0.0)
		frac = 0.0;
	if (frac > 1.0)
		frac = 1.0;
	SetPosition(fMin + (int)floor(frac * (fMax - fMin) + 0.5));
}

void VolumeSlider::SetLevels(int peak, float rms, double dtSec)
{
	double peakDb = ToDb(peak);
	double rmsDb = ToDb(rms);

	fastDb -= METER_FAST_RELEASE_DB_PER_SEC * dtSec;
	if (peakDb > fastDb)
		fastDb = peakDb;

	double k = 1.0 - exp(-dtSec / METER_SLOW_TAU_SEC);
	slowDb += (rmsDb - slowDb) * k;

	Invalidate();
}

bool VolumeSlider::IsSilent(void) const
{
	return fastDb <= METER_FLOOR_DB + 0.5 && slowDb <= METER_FLOOR_DB + 0.5;
}

void __fastcall VolumeSlider::Resize(void)
{
	TCustomControl::Resize();
	Invalidate();
}

void __fastcall VolumeSlider::Paint(void)
{
	int w = Width;
	int h = Height;
	if (w < 1 || h < 1)
		return;

	// draw into an off-screen bitmap, then copy: no flicker at meter rate
	buffer->Width = w;
	buffer->Height = h;
	TCanvas *c = buffer->Canvas;

	// background follows the parent, so the slider blends into its panel
	c->Brush->Color = Color;
	c->Pen->Style = psClear;
	c->FillRect(TRect(0, 0, w, h));

	// slow (RMS) level from the bottom
	int slowTop = h - (int)(ScalePos(slowDb) * h);
	if (slowTop < h)
	{
		c->Brush->Color = ZoneColor(slowDb, cLow, cMid, cHigh);
		c->FillRect(TRect(2, slowTop, w - 2, h));
	}

	// fast (peak) marker; hidden while at the floor (silence)
	if (fastDb > METER_FLOOR_DB + 0.5)
	{
		int fastY = h - (int)(ScalePos(fastDb) * h);
		c->Brush->Color = cPeak;
		c->FillRect(TRect(1, fastY - 1, w - 1, fastY + 1));
	}

	// track
	int cx = w / 2;
	c->Brush->Color = cTrack;
	c->FillRect(TRect(cx - 1, TRACK_PAD, cx + 2, h - TRACK_PAD));

	// thumb
	int span = h - 2 * TRACK_PAD;
	double frac = (fMax > fMin) ? (double)(fPosition - fMin) / (fMax - fMin) : 0.0;
	int thumbY = TRACK_PAD + (int)floor(frac * span + 0.5);
	c->Brush->Color = cThumb;
	c->Pen->Style = psSolid;
	c->Pen->Color = (TColor)RGB(0, 0, 0);
	c->Rectangle(TRect(1, thumbY - THUMB_HALF, w - 1, thumbY + THUMB_HALF + 1));

	Canvas->Draw(0, 0, buffer);
}

void __fastcall VolumeSlider::MouseDown(TMouseButton Button, Classes::TShiftState Shift, int X, int Y)
{
	if (Button == mbLeft)
	{
		dragging = true;
		PositionFromY(Y);
	}
	TCustomControl::MouseDown(Button, Shift, X, Y);
}

void __fastcall VolumeSlider::MouseMove(Classes::TShiftState Shift, int X, int Y)
{
	if (dragging)
		PositionFromY(Y);
	TCustomControl::MouseMove(Shift, X, Y);
}

void __fastcall VolumeSlider::MouseUp(TMouseButton Button, Classes::TShiftState Shift, int X, int Y)
{
	dragging = false;
	TCustomControl::MouseUp(Button, Shift, X, Y);
}

//---------------------------------------------------------------------------

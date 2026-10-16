//---------------------------------------------------------------------------

#ifndef VolumeSliderH
#define VolumeSliderH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <Graphics.hpp>

// Vertical volume slider with VU meter as its background. Created in code
// (not in .dfm), so no component registration is needed in the IDE.
// Position semantics as the former TTrackBar: Min at the top, Max at the bottom.
// Meter: fast (peak, instant attack, 40 dB/s release) and slow (RMS, 300 ms),
// on a logarithmic scale -60..0 dBFS. Levels: linear 0..32768.
class VolumeSlider : public TCustomControl
{
private:
	int fMin;
	int fMax;
	int fPosition;
	double fastDb;
	double slowDb;
	bool dragging;
	TColor cLow;
	TColor cMid;
	TColor cHigh;
	TColor cPeak;
	TColor cTrack;
	TColor cThumb;
	Graphics::TBitmap *buffer;
	void SetPosition(int value);
	void SetMin(int value);
	void SetMax(int value);
	void PositionFromY(int y);
	VolumeSlider(const VolumeSlider&);
	VolumeSlider& operator=(const VolumeSlider&);
protected:
	virtual void __fastcall Paint(void);
	DYNAMIC void __fastcall Resize(void);
	DYNAMIC void __fastcall MouseDown(TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
	DYNAMIC void __fastcall MouseMove(Classes::TShiftState Shift, int X, int Y);
	DYNAMIC void __fastcall MouseUp(TMouseButton Button, Classes::TShiftState Shift, int X, int Y);
public:
	__fastcall VolumeSlider(TComponent* Owner);
	__fastcall ~VolumeSlider();
	// Call periodically; dtSec is time elapsed since the previous call
	void SetLevels(int peak, float rms, double dtSec);
	// True when both fast and slow levels have decayed to the floor
	bool IsSilent(void) const;
	// Meter zones (low/mid/high), peak marker, track and thumb colors
	void SetColors(TColor low, TColor mid, TColor high, TColor peak, TColor track, TColor thumb);
	// Fired when Position changes (by mouse or by code)
	TNotifyEvent OnChange;
	__property int Position = {read=fPosition, write=SetPosition};
	__property int Min = {read=fMin, write=SetMin};
	__property int Max = {read=fMax, write=SetMax};
};
//---------------------------------------------------------------------------
#endif

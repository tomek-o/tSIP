//---------------------------------------------------------------------------

#ifndef FormAccountCodecsH
#define FormAccountCodecsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfrmAccountCodecs : public TForm
{
__published:	// IDE-managed Components
	TLabel *lblAudioCodecsAvailable;
	TListBox *lboxAudioCodecsAvailable;
	TButton *btnAudioCodecEnable;
	TButton *btnAudioCodecDisable;
	TListBox *lboxAudioCodecsEnabled;
	TLabel *lblAudioCodecsEnabled;
	TLabel *lblCodecsInfo;
	TLabel *lblCodecsInfo2;
	void __fastcall lboxAudioCodecsAvailableDblClick(TObject *Sender);
	void __fastcall lboxAudioCodecsEnabledDblClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall btnAudioCodecEnableClick(TObject *Sender);
	void __fastcall btnAudioCodecDisableClick(TObject *Sender);
	void __fastcall lboxAudioCodecsEnabledDragDrop(TObject *Sender,
          TObject *Source, int X, int Y);
	void __fastcall lboxAudioCodecsEnabledDragOver(TObject *Sender,
          TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall lboxAudioCodecsEnabledStartDrag(TObject *Sender,
          TDragObject *&DragObject);
private:	// User declarations
	int audioCodecsEnabledDraggedIndex;
public:		// User declarations
	__fastcall TfrmAccountCodecs(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAccountCodecs *frmAccountCodecs;
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormAccountCodecs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAccountCodecs *frmAccountCodecs;
//---------------------------------------------------------------------------
__fastcall TfrmAccountCodecs::TfrmAccountCodecs(TComponent* Owner)
	: TForm(Owner),
	audioCodecsEnabledDraggedIndex(-1)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::lboxAudioCodecsAvailableDblClick(
      TObject *Sender)
{
	AudioCodecEnableSelected();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::lboxAudioCodecsEnabledDblClick(
      TObject *Sender)
{
	AudioCodecDisableSelected();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::FormShow(TObject *Sender)
{
	lboxAudioCodecsAvailable->Clear();
	std::vector<AnsiString> codecs;
	if (Ua::Instance().GetAudioCodecList(codecs) == 0)
	{
		for (unsigned int i=0; i<codecs.size(); i++)
		{
			lboxAudioCodecsAvailable->Items->Add(codecs[i]);
		}
	}
	lboxAudioCodecsEnabled->Clear();

	/** \todo multiple accounts */
	for (unsigned int i=0; i<tmpSettings.uaConf.accounts[0].audio_codecs.size(); i++)
	{
		AnsiString name = tmpSettings.uaConf.accounts[0].audio_codecs[i].c_str();
		for (int codec = 0; codec < lboxAudioCodecsAvailable->Items->Count; codec++)
		{
			if (lboxAudioCodecsAvailable->Items->Strings[codec] == name)
			{
				lboxAudioCodecsAvailable->Items->Delete(codec);
				lboxAudioCodecsEnabled->Items->Add(name);
				break;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::btnAudioCodecEnableClick(TObject *Sender)
{
	AudioCodecEnableSelected();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::btnAudioCodecDisableClick(TObject *Sender)
{
	AudioCodecDisableSelected();	
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::lboxAudioCodecsEnabledDragDrop(
      TObject *Sender, TObject *Source, int X, int Y)
{
	TListBox *lbox = lboxAudioCodecsEnabled;
	int dropIndex = lbox->ItemAtPos(Point(X,Y), false);
	if (dropIndex >= 0)
	{
		lbox->Items->Move(audioCodecsEnabledDraggedIndex, dropIndex);
		lbox->ItemIndex = dropIndex;
    }	
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::lboxAudioCodecsEnabledDragOver(
      TObject *Sender, TObject *Source, int X, int Y, TDragState State,
      bool &Accept)
{
	Accept = true;		
}
//---------------------------------------------------------------------------
void __fastcall TfrmAccountCodecs::lboxAudioCodecsEnabledStartDrag(
      TObject *Sender, TDragObject *&DragObject)
{
	audioCodecsEnabledDraggedIndex = lboxAudioCodecsEnabled->ItemIndex;		
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#ifndef FormAccountH
#define FormAccountH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------

#include "UaConf.h"

class TfrmAccount : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *grboxAccount;
	TLabel *lblRegServer;
	TEdit *edRegServer;
	TComboBox *cbTransport;
	TLabel *lblUser;
	TLabel *lblPassword;
	TLabel *lblExpires;
	TEdit *edUser;
	TEdit *edPassword;
	TEdit *edExpires;
	TLabel *lblExpiresInfo;
	TLabel *lblExtra;
	TSpeedButton *btnExtra;
	TLabel *lblAuthUser;
	TEdit *edAuthUser;
	TLabel *lblAuthUserInfo;
	TLabel *lblStunServer;
	TEdit *edStunServer;
	TLabel *lblOutbound1;
	TEdit *edOutbound1;
	TCheckBox *chbAnswerAny;
	TLabel *lblPtime;
	TEdit *edPtime;
	TLabel *lblContactUser;
	TEdit *edCuser;
	TLabel *lblExpiresInfo2;
	TCheckBox *chbHideRegServer;
	TCheckBox *chbHideUser;
	TCheckBox *chbHidePassword;
	TCheckBox *chbHideAuthUser;
	TCheckBox *chbHideCuser;
	TComboBox *cbDtmfTxFormat;
	TLabel *lblDtmfTxFormat;
	TLabel *lblDisplayName;
	TEdit *edDisplayName;
	TCheckBox *chbHideDisplayName;
	TLabel *lblMediaEncryption;
	TComboBox *cbMediaEncryption;
	TComboBox *cbOutbound1Transport;
	TCheckBox *chbEnabled;
	TLabel *lblName;
	TEdit *edName;
	TLabel *lblNameInfo;
	TPanel *pnlBasic;
	TPanel *pnlExtraToggle;
	TPanel *pnlExtra;
	TPanel *pnlCodecs;
	TLabel *lblAudioCodecsAvailable;
	TLabel *lblAudioCodecsEnabled;
	TLabel *lblCodecsInfo;
	TListBox *lboxAudioCodecsAvailable;
	TButton *btnAudioCodecEnable;
	TButton *btnAudioCodecDisable;
	TListBox *lboxAudioCodecsEnabled;
	TLabel *lblVideoCodecsAvailable;
	TLabel *lblVideoCodecsEnabled;
	TListBox *lboxVideoCodecsAvailable;
	TButton *btnVideoCodecEnable;
	TButton *btnVideoCodecDisable;
	TListBox *lboxVideoCodecsEnabled;
	void __fastcall edRegServerChange(TObject *Sender);
	void __fastcall edUserChange(TObject *Sender);
	void __fastcall edPasswordChange(TObject *Sender);
	void __fastcall cbTransportChange(TObject *Sender);
	void __fastcall edExpiresChange(TObject *Sender);
	void __fastcall edStunServerChange(TObject *Sender);
	void __fastcall edAuthUserChange(TObject *Sender);
	void __fastcall edOutbound1Change(TObject *Sender);
	void __fastcall chbAnswerAnyClick(TObject *Sender);
	void __fastcall chbAnswerAnyKeyPress(TObject *Sender, char &Key);
	void __fastcall edPtimeChange(TObject *Sender);
	void __fastcall edCuserChange(TObject *Sender);
	void __fastcall chbHideRegServerClick(TObject *Sender);
	void __fastcall chbHideUserClick(TObject *Sender);
	void __fastcall chbHidePasswordClick(TObject *Sender);
	void __fastcall chbHideAuthUserClick(TObject *Sender);
	void __fastcall chbHideCuserClick(TObject *Sender);
	void __fastcall cbDtmfTxFormatChange(TObject *Sender);
	void __fastcall edDisplayNameChange(TObject *Sender);
	void __fastcall chbHideDisplayNameClick(TObject *Sender);
	void __fastcall cbMediaEncryptionChange(TObject *Sender);
	void __fastcall cbOutbound1TransportChange(TObject *Sender);
	void __fastcall chbEnabledClick(TObject *Sender);
	void __fastcall edNameChange(TObject *Sender);
	void __fastcall btnExtraClick(TObject *Sender);
	void __fastcall lboxAudioCodecsAvailableDblClick(TObject *Sender);
	void __fastcall lboxAudioCodecsEnabledDblClick(TObject *Sender);
	void __fastcall btnAudioCodecEnableClick(TObject *Sender);
	void __fastcall btnAudioCodecDisableClick(TObject *Sender);
	void __fastcall lboxAudioCodecsEnabledDragDrop(TObject *Sender,
		  TObject *Source, int X, int Y);
	void __fastcall lboxAudioCodecsEnabledDragOver(TObject *Sender,
		  TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall lboxAudioCodecsEnabledStartDrag(TObject *Sender,
		  TDragObject *&DragObject);
	void __fastcall lboxVideoCodecsAvailableDblClick(TObject *Sender);
	void __fastcall lboxVideoCodecsEnabledDblClick(TObject *Sender);
	void __fastcall btnVideoCodecEnableClick(TObject *Sender);
	void __fastcall btnVideoCodecDisableClick(TObject *Sender);
	void __fastcall lboxVideoCodecsEnabledDragDrop(TObject *Sender,
		  TObject *Source, int X, int Y);
	void __fastcall lboxVideoCodecsEnabledDragOver(TObject *Sender,
		  TObject *Source, int X, int Y, TDragState State, bool &Accept);
	void __fastcall lboxVideoCodecsEnabledStartDrag(TObject *Sender,
		  TDragObject *&DragObject);
private:	// User declarations
	UaConf::Account& acc;
	int audioCodecsEnabledDraggedIndex;
	int videoCodecsEnabledDraggedIndex;
	void LoadAudioCodecs(void);
	void StoreAudioCodecs(void);
	void AudioCodecEnableSelected(void);
	void AudioCodecDisableSelected(void);
	void LoadVideoCodecs(void);
	void StoreVideoCodecs(void);
	void VideoCodecEnableSelected(void);
	void VideoCodecDisableSelected(void);
	void SetExtraVisible(bool state);
	void UpdateHeight(void);
public:		// User declarations
	__fastcall TfrmAccount(TComponent* Owner, int id, UaConf::Account& acc);

};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAccount *frmAccount;
//---------------------------------------------------------------------------
#endif

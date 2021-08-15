//---------------------------------------------------------------------------

#ifndef FormAccountH
#define FormAccountH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
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
private:	// User declarations
	UaConf::Account& acc;
public:		// User declarations
	__fastcall TfrmAccount(TComponent* Owner, int id, UaConf::Account& acc);

};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAccount *frmAccount;
//---------------------------------------------------------------------------
#endif

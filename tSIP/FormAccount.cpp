//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormAccount.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAccount *frmAccount;
//---------------------------------------------------------------------------
__fastcall TfrmAccount::TfrmAccount(TComponent* Owner, int id, UaConf::Account& acc)
	: TForm(Owner),
	acc(acc)
{
	AnsiString title;
	title.sprintf("Account #%02d", id + 1);
	grboxAccount->Caption = title;

	chbHideRegServer->Checked = acc.hide_reg_server;
	chbHideUser->Checked = acc.hide_user;
	chbHideDisplayName->Checked = acc.hide_display_name;
	chbHideAuthUser->Checked = acc.hide_auth_user;
	chbHideCuser->Checked = acc.hide_cuser;
	chbHidePassword->Checked = acc.hide_pwd;

	if (!acc.hide_reg_server)
		edRegServer->Text = acc.reg_server.c_str();
	if (!acc.hide_user)
		edUser->Text = acc.user.c_str();
	if (!acc.hide_display_name)
		edDisplayName->Text = acc.display_name.c_str();
	if (!acc.hide_auth_user)
		edAuthUser->Text = acc.auth_user.c_str();
	if (!acc.hide_cuser)
		edCuser->Text = acc.cuser.c_str();
	if (!acc.hide_pwd)
		edPassword->Text = acc.pwd.c_str();
	cbTransport->ItemIndex = acc.transport;
	edExpires->Text = acc.reg_expires;

	chbAnswerAny->Checked = acc.answer_any;
	edPtime->Text = acc.ptime;
	edStunServer->Text = acc.stun_server.c_str();
	edOutbound1->Text = acc.outbound1.c_str();
	//edOutbound2->Text = acc.outbound2.c_str();
	if (acc.dtmf_tx_format < cbDtmfTxFormat->Items->Count)
	{
		cbDtmfTxFormat->ItemIndex = acc.dtmf_tx_format;
	}
	else
	{
		cbDtmfTxFormat->ItemIndex = 0;
	}
	cbMediaEncryption->ItemIndex = 0;
	for (int i=1; i<cbMediaEncryption->Items->Count; i++)
	{
		if (cbMediaEncryption->Items->Strings[i] == acc.mediaenc.c_str())
		{
			cbMediaEncryption->ItemIndex = i;
			break;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edRegServerChange(TObject *Sender)
{
	acc.reg_server = edRegServer->Text.c_str();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edUserChange(TObject *Sender)
{
	acc.user = edUser->Text.c_str();
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edPasswordChange(TObject *Sender)
{
	acc.pwd = edPassword->Text.c_str();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::cbTransportChange(TObject *Sender)
{
	acc.transport = (UaConf::Account::TRANSPORT_TYPE)cbTransport->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edExpiresChange(TObject *Sender)
{
	acc.reg_expires = StrToIntDef(edExpires->Text, 60);
	if (acc.reg_expires == 0)
		lblExpiresInfo2->Font->Style = TFontStyles() << fsBold;
	else
		lblExpiresInfo2->Font->Style = TFontStyles();
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edStunServerChange(TObject *Sender)
{
	acc.stun_server = edStunServer->Text.c_str();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edDisplayNameChange(TObject *Sender)
{
	acc.display_name = edDisplayName->Text.c_str();
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edAuthUserChange(TObject *Sender)
{
	acc.auth_user = edAuthUser->Text.c_str();
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edOutbound1Change(TObject *Sender)
{
	acc.outbound1 = edOutbound1->Text.c_str();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbAnswerAnyClick(TObject *Sender)
{
	acc.answer_any = chbAnswerAny->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbAnswerAnyKeyPress(TObject *Sender, char &Key)
{
	acc.answer_any = chbAnswerAny->Checked;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edPtimeChange(TObject *Sender)
{
	acc.ptime = StrToIntDef(edPtime->Text, UaConf::Account::DEF_PTIME);
	if (acc.ptime < UaConf::Account::MIN_PTIME || acc.ptime > UaConf::Account::MAX_PTIME)
	{
        acc.ptime = UaConf::Account::DEF_PTIME;
    } 
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::edCuserChange(TObject *Sender)
{
	acc.cuser = edCuser->Text.c_str();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHideRegServerClick(TObject *Sender)
{
	acc.hide_reg_server = chbHideRegServer->Checked;
	if (!chbHideRegServer->Checked)
		acc.reg_server = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHideUserClick(TObject *Sender)
{
	acc.hide_user = chbHideUser->Checked;
	if (!chbHideUser->Checked)
		acc.user = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHidePasswordClick(TObject *Sender)
{
	acc.hide_pwd = chbHidePassword->Checked;
	if (!chbHidePassword->Checked)
		acc.pwd = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHideDisplayNameClick(TObject *Sender)
{
	acc.hide_display_name = chbHideDisplayName->Checked;
	if (!chbHideDisplayName->Checked)
		acc.display_name = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHideAuthUserClick(TObject *Sender)
{
	acc.hide_auth_user = chbHideAuthUser->Checked;
	if (!chbHideAuthUser->Checked)
		acc.auth_user = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::chbHideCuserClick(TObject *Sender)
{
	acc.hide_cuser = chbHideCuser->Checked;
	if (!chbHideCuser->Checked)
		acc.cuser = "";
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::cbDtmfTxFormatChange(TObject *Sender)
{
	acc.dtmf_tx_format = cbDtmfTxFormat->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAccount::cbMediaEncryptionChange(TObject *Sender)
{
	if (cbMediaEncryption->ItemIndex == 0)
		acc.mediaenc = "";
	else
		acc.mediaenc = cbMediaEncryption->Text.c_str();
}
//---------------------------------------------------------------------------


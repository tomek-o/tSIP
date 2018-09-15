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
	edRegServer->Text = acc.reg_server.c_str();
	edUser->Text = acc.user.c_str();
	edAuthUser->Text = acc.auth_user.c_str();
	edCuser->Text = acc.cuser.c_str();
	edPassword->Text = acc.pwd.c_str();
	cbTransport->ItemIndex = acc.transport;
	edExpires->Text = acc.reg_expires;

	chbAnswerAny->Checked = acc.answer_any;
	edPtime->Text = acc.ptime;
	edStunServer->Text = acc.stun_server.c_str();
	edOutbound1->Text = acc.outbound1.c_str();
	//edOutbound2->Text = acc.outbound2.c_str();
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


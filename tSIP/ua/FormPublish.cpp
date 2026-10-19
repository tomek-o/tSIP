//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormPublish.h"
#include "Settings.h"
#include "Paths.h"
#include "UaGlobals.h"
#include "ControlQueue.h"
#include "UaPublish.h"
#include <vector>

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

namespace
{

/** presence expiry requested from the server; the document is published again before it expires */
const int PUBLISH_EXPIRES = 3600;

AnsiString XmlEscape(const AnsiString &s)
{
	AnsiString out;
	for (int i = 1; i <= s.Length(); i++)
	{
		switch (s[i])
		{
		case '&': out += "&amp;"; break;
		case '<': out += "&lt;"; break;
		case '>': out += "&gt;"; break;
		case '"': out += "&quot;"; break;
		case '\'': out += "&apos;"; break;
		default: out += s[i]; break;
		}
	}
	return out;
}

/** PIDF presence document (RFC 3863) */
AnsiString PresenceDocument(const AnsiString &entity, bool open, const AnsiString &note)
{
	AnsiString doc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	doc += "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"" + XmlEscape(entity) + "\">\n";
	doc += "  <tuple id=\"t1\">\n";
	doc += "    <status>\n";
	doc += AnsiString("      <basic>") + (open ? "open" : "closed") + "</basic>\n";
	doc += "    </status>\n";
	if (note.Length() > 0)
		doc += "    <note>" + XmlEscape(note) + "</note>\n";
	doc += "  </tuple>\n";
	doc += "</presence>\n";
	return doc;
}

/** queues a PUBLISH for the account from the state in UaPublish (new presence, refresh or unpublish) */
void PublishSend(int accountId)
{
	UaPublish::Presence p;
	if (!UaPublish::Get(accountId, p))
		return;
	const UaConf::Account &acc = appSettings.uaConf.accounts[accountId];
	AnsiString aor = AnsiString("sip:") + acc.user.c_str() + "@" + acc.reg_server.c_str();
	// withdrawn publication: empty body and Expires 0 remove the presence from the server
	AnsiString body = p.active ? PresenceDocument(aor, p.open, AnsiString(p.note.c_str())) : AnsiString();
	int expires = p.active ? PUBLISH_EXPIRES : 0;
	UA->PublishPresence(accountId, aor, body, expires, AnsiString(p.etag.c_str()));
	UaPublish::SentAt(accountId, GetTickCount());
}

void PublishNow(bool open, bool currentOnly, const AnsiString &note)
{
	// the note is restored from the settings the next time the dialog is opened (also after restart)
	appSettings.uaConf.publishNote = note.c_str();
	if (appSettings.Write(Paths::GetConfig()) != 0)
		ShowMessage("Could not save settings");

	std::vector<int> targets;
	if (currentOnly)
	{
		int accountId = appSettings.GetCurrentAccountId();
		if (accountId < 0 || accountId >= (int)appSettings.uaConf.accounts.size())
			return;
		if (!appSettings.uaConf.accounts[accountId].enabled)
		{
			ShowMessage("The current account is disabled");
			return;
		}
		targets.push_back(accountId);
	}
	else
	{
		for (unsigned int i = 0; i < appSettings.uaConf.accounts.size(); i++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[i];
			if (acc.enabled && acc.publish)
				targets.push_back(i);
		}
	}
	for (unsigned int i = 0; i < targets.size(); i++)
	{
		UaPublish::Request(targets[i], open, note.c_str());
		PublishSend(targets[i]);
	}
}

}	// namespace

//---------------------------------------------------------------------------

void PublishRefreshDue(void)
{
	std::vector<int> due = UaPublish::Due(GetTickCount());
	for (unsigned int i = 0; i < due.size(); i++)
		PublishSend(due[i]);
}

bool PublishWithdraw(int accountId)
{
	UaPublish::Presence p;
	if (!UaPublish::Get(accountId, p) || !p.active)
		return false;
	UaPublish::Withdraw(accountId);
	PublishSend(accountId);
	return true;
}

bool PublishWithdrawAll(void)
{
	bool sent = false;
	for (unsigned int i = 0; i < appSettings.uaConf.accounts.size(); i++)
		sent = PublishWithdraw(i) || sent;
	return sent;
}

__fastcall TfrmPublish::TfrmPublish(TComponent* Owner)
	: TForm(Owner)
{
	memo->Text = appSettings.uaConf.publishNote.c_str();
}

bool TfrmPublish::IsOpen(void) const
{
	return cbStatus->ItemIndex == 0;
}

bool TfrmPublish::CurrentAccountOnly(void) const
{
	return cbTarget->ItemIndex == 0;
}

AnsiString TfrmPublish::Note(void) const
{
	return memo->Text;
}

void ShowPublishDialog(TComponent *owner)
{
	TfrmPublish *frm = new TfrmPublish(owner);
	if (frm->ShowModal() == mrOk)
		PublishNow(frm->IsOpen(), frm->CurrentAccountOnly(), frm->Note());
	delete frm;
}
//---------------------------------------------------------------------------

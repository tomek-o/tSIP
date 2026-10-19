//---------------------------------------------------------------------------
#ifndef FormPublishH
#define FormPublishH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

/** \brief Dialog to publish presence (SIP PUBLISH, RFC 3903) to the current account
 *  or to all accounts with PUBLISH enabled. Layout is in PublishDialog.dfm.
 */
class TfrmPublish : public TForm
{
__published:	// IDE-managed Components
	TLabel *lblStatus;
	TComboBox *cbStatus;
	TLabel *lblTarget;
	TComboBox *cbTarget;
	TLabel *lblNote;
	TMemo *memo;
	TButton *btnPublish;
	TButton *btnCancel;
private:	// User declarations
public:		// User declarations
	__fastcall TfrmPublish(TComponent* Owner);
	bool IsOpen(void) const;
	bool CurrentAccountOnly(void) const;
	AnsiString Note(void) const;
};

/** Shows the dialog and publishes the presence if confirmed */
void ShowPublishDialog(TComponent *owner);
/** Refreshes the published presence of accounts before their expiry (called periodically) */
void PublishRefreshDue(void);
/** Unpublishes the presence of an account; true if an unpublish was queued */
bool PublishWithdraw(int accountId);
/** Unpublishes the presence of all accounts (on exit); true if anything was queued */
bool PublishWithdrawAll(void);

#endif

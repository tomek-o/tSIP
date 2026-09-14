//---------------------------------------------------------------------------
#ifndef FrameLogConfH
#define FrameLogConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>

struct LogConf;

class TfraLogConf : public TFrame
{
__published:	// IDE-managed Components
	TCheckBox *chbLogToFile;
	TCheckBox *chbLogFlush;
	TLabel *lblLogMaxFileSize;
	TLabel *lblLogRotate;
	TCheckBox *chbLogTimestamps;
	TLabel *lblUiCapacity;
	TLabel *lblLoggingConsoleFont;
	TCheckBox *chbLogShowWindowAtStartup;
	TEdit *edLoggingConsoleFont;
	TButton *btnLoggingConsoleFontSelect;
	TComboBox *cmbMaxUiLogLines;
	TComboBox *cbLogRotate;
	TComboBox *cbLogMaxFileSize;
	TFontDialog *fontDialog;
	void __fastcall btnLoggingConsoleFontSelectClick(TObject *Sender);
private:	// User declarations
	LogConf &logging;
public:		// User declarations
	__fastcall TfraLogConf(TComponent* Owner, LogConf &logging);
	void Load(void);
	void Apply(void);
};

#endif

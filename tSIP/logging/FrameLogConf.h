//---------------------------------------------------------------------------
#ifndef FrameLogConfH
#define FrameLogConfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>

struct LogConf;

class TfraLogConf : public TFrame
{
__published:	// IDE-managed Components
	TCheckBox *chbLogToFile;
	TCheckBox *chbLogFlush;
	TLabel *lblLogMaxFileSize;
	TLabel *lblLogRotate;
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
	/// grid: rows are log levels (SIP, error, warning, info, debug),
	/// columns are text in the rich edit, text in the list, list background
	enum { LEVELS = 5, CELLS = 3 };
	TPanel *pnlCell[LEVELS][CELLS];
	TComboBox *cbColorTemplate;
	TButton *btnColorTemplate;
	TPanel *pnlRichBackground;
	TButton *btnRichBackground;
	TColorDialog *dlgColor;
	TCheckBox *chbLogAubuf;
	void CreateColorControls(void);
	void __fastcall btnColorClick(TObject *Sender);
	void __fastcall btnColorTemplateClick(TObject *Sender);
	void __fastcall btnRichBackgroundClick(TObject *Sender);
public:		// User declarations
	__fastcall TfraLogConf(TComponent* Owner, LogConf &logging);
	void Load(void);
	void Apply(void);
};

#endif

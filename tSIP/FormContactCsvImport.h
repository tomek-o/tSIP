//---------------------------------------------------------------------------

#ifndef FormContactCsvImportH
#define FormContactCsvImportH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
#include <vector>

class TfrmContactCsvImport : public TForm
{
__published:	// IDE-managed Components
	TPageControl *pcWizard;
	TTabSheet *tsConfigureCsv;
	TTabSheet *tsReadCsv;
	TTabSheet *tsGenerateJson;
	TLabel *lblSeparator;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label4;
	TLabel *Label5;
	TEdit *edColumns;
	TEdit *edRows;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TComboBox *cbJsonStyle;
	TComboBox *cbDecimalSeparator;
	TPanel *pnlBottom;
	TPanel *pnlBottomRight;
	TButton *btnPrev;
	TButton *btnNext;
	TBevel *Bevel1;
	TCheckBox *chbExtractNumbers;
	TMemo *edFieldSeparator;
	TMemo *Memo1;
	TLabel *Label3;
	TLabel *lblInfo2;
	void __fastcall btnNextClick(TObject *Sender);
	void __fastcall btnPrevClick(TObject *Sender);
	void __fastcall chbExtractNumbersClick(TObject *Sender);
	void __fastcall chbExtractNumbersKeyPress(TObject *Sender, char &Key);
	void __fastcall lblInfo2Click(TObject *Sender);
private:	// User declarations
	typedef	std::vector<AnsiString> Fields;
	std::vector<Fields> csvRows;
	AnsiString fileName;
	AnsiString asJson;
	bool bUseStyledWriter;
	int pageIdx;
	void UpdateButtons(void);
	int ParseCsv(AnsiString asFileName, char separator, AnsiString& asMsg);
	int GenerateJson(int style, bool extractNumbers,
		char decimalSeparator, AnsiString& asJson, AnsiString& asMsg);
public:		// User declarations
	__fastcall TfrmContactCsvImport(TComponent* Owner);
	void Load(AnsiString fileName, bool bUseStyledWriter);
	AnsiString GetJson(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmContactCsvImport *frmContactCsvImport;
//---------------------------------------------------------------------------
#endif

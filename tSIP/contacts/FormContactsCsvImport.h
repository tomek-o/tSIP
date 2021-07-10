//---------------------------------------------------------------------------

#ifndef FormContactsCsvImportH
#define FormContactsCsvImportH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
#include <vector>

class Contacts;

class TfrmContactsCsvImport : public TForm
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
	TComboBox *cbDescriptionColumn;
	TPanel *pnlBottom;
	TPanel *pnlBottomRight;
	TButton *btnPrev;
	TButton *btnNext;
	TBevel *Bevel1;
	TCheckBox *chbSkipFirstRow;
	TMemo *edFieldSeparator;
	TMemo *Memo1;
	TLabel *Label3;
	TLabel *lblInfo2;
	TLabel *Label6;
	TComboBox *cbNumberColumn1;
	TCheckBox *chbSkipDuplicatedNumbers;
	TLabel *Label9;
	TComboBox *cbNumberColumn2;
	TLabel *Label10;
	TComboBox *cbNumberColumn3;
	TLabel *lblCompany;
	TComboBox *cbCompanyColumn;
	TLabel *lblNoteColumn;
	TComboBox *cbNoteColumn;
	void __fastcall btnNextClick(TObject *Sender);
	void __fastcall btnPrevClick(TObject *Sender);
	void __fastcall lblInfo2Click(TObject *Sender);
private:	// User declarations
	typedef	std::vector<AnsiString> Fields;
	std::vector<Fields> csvRows;
	AnsiString fileName;
	int pageIdx;
	void UpdateButtons(void);
	int ParseCsv(AnsiString asFileName, char separator, AnsiString& asMsg);
	int UpdateContacts(void);
	Contacts *contacts;
public:		// User declarations
	__fastcall TfrmContactsCsvImport(TComponent* Owner);
	void Load(AnsiString fileName, Contacts *contacts);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmContactsCsvImport *frmContactsCsvImport;
//---------------------------------------------------------------------------
#endif

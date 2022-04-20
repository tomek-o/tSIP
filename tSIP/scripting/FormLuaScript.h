//---------------------------------------------------------------------------

#ifndef FormLuaScriptH
#define FormLuaScriptH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ActnList.hpp>
#include <StdActns.hpp>

#include <vector>

class TfrmTextEditor;

class TfrmLuaScript : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TPanel *pnlBottom2;
	TButton *btnExecute;
	TButton *btnBreak;
	TMainMenu *MainMenu;
	TMenuItem *miHelp;
	TMenuItem *miFile;
	TMenuItem *miHelpExamples;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	TActionList *actionList;
	TMenuItem *Open1;
	TMenuItem *SaveAs1;
	TAction *actFileOpen;
	TAction *actFileSaveAs;
	TAction *actFileSave;
	TMenuItem *Save1;
	TMenuItem *miOpenRecent;
	TMenuItem *miCloseWindow;
	TButton *btnLuacheck;
	TListView *lvValidation;
	TMenuItem *miCustomLuaFunctions;
	TMenuItem *miFileNew;
	void __fastcall btnExecuteClick(TObject *Sender);
	void __fastcall btnBreakClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall actFileOpenExecute(TObject *Sender);
	void __fastcall actFileSaveAsExecute(TObject *Sender);
	void __fastcall actFileSaveExecute(TObject *Sender);
	void __fastcall miFileClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall miCloseWindowClick(TObject *Sender);
	void __fastcall btnLuacheckClick(TObject *Sender);
	void __fastcall lvValidationData(TObject *Sender, TListItem *Item);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lvValidationDblClick(TObject *Sender);
	void __fastcall miCustomLuaFunctionsClick(TObject *Sender);
	void __fastcall miFileNewClick(TObject *Sender);
private:	// User declarations
	bool breakRequest;
	volatile bool running;
	TfrmTextEditor *frmEditor;
	void UpdateExamplesMenu(void);
	void __fastcall LoadExample(TObject *Sender);
	AnsiString asCurrentFile;
	bool modified;
	void SetTitle(AnsiString filename, bool modified);
	void AddMruItem(AnsiString filename);
	void UpdateMruMenu(void);
	void __fastcall MruClick(TObject *Sender);
	void __fastcall RemoveObsoleteFilesClick(TObject *Sender);
	void __fastcall ClearMruClick(TObject *Sender);
	int CheckFileNotSavedDialog(void);
	void OnTextModified(void);
	void SetText(AnsiString text);
	void __fastcall WMDropFiles(TWMDropFiles &message);

	struct ValidationEntry {
		AnsiString file;
		int line;
		int position;	// position in line;
		AnsiString message;
		ValidationEntry(void):
			line(-1),
			position(-1)
		{}
	};
	std::vector<ValidationEntry> validationEntries;
public:		// User declarations
	__fastcall TfrmLuaScript(TComponent* Owner);
	typedef int (__closure *CallbackRunScript)(int srcType, int srcId, AnsiString script, bool &breakRequest, bool &handled);
	static void SetCallbackRunScript(CallbackRunScript cb);
	void SetScript(AnsiString asString);
	void OpenFile(AnsiString filename);
	BEGIN_MESSAGE_MAP
		MESSAGE_HANDLER(WM_DROPFILES, TWMDropFiles, WMDropFiles)
	END_MESSAGE_MAP(TForm);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmLuaScript *frmLuaScript;
//---------------------------------------------------------------------------
#endif

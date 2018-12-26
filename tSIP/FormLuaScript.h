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

#include "ScriptExec.h"
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ActnList.hpp>
#include <StdActns.hpp>

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
	void __fastcall btnExecuteClick(TObject *Sender);
	void __fastcall btnBreakClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall actFileOpenExecute(TObject *Sender);
	void __fastcall actFileSaveAsExecute(TObject *Sender);
	void __fastcall actFileSaveExecute(TObject *Sender);
	void __fastcall miFileClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
	typedef int (__closure *CallbackRunScript)(int srcType, int srcId, AnsiString script, bool &breakRequest);
	CallbackRunScript callbackRunScript;
	bool breakRequest;
	volatile bool running;
	TfrmTextEditor *frmEditor;
	void UpdateExamplesMenu(void);
	void __fastcall LoadExample(TObject *Sender);
	void OpenFile(AnsiString filename);
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
	int OnGetCurrentFileName(AnsiString &filename);
public:		// User declarations
	__fastcall TfrmLuaScript(TComponent* Owner,
		CallbackRunScript callbackRunScript
		);
	void SetScript(AnsiString asString);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmLuaScript *frmLuaScript;
//---------------------------------------------------------------------------
#endif

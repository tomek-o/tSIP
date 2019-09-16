//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormLuaScript.h"
#include "FormTextEditor.h"
#include "ScriptExec.h"
#include "common/BtnController.h"
#include "LuaExamples.h"
#include "Settings.h"
#include "ScriptSource.h"
#include <memory>
#include <assert.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmLuaScript *frmLuaScript = NULL;
//---------------------------------------------------------------------------

namespace
{

TfrmLuaScript::CallbackRunScript callbackRunScript = NULL;

}

void TfrmLuaScript::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	callbackRunScript = cb;	
}

__fastcall TfrmLuaScript::TfrmLuaScript(TComponent* Owner)
	:
	TForm(Owner),
	breakRequest(false),
	running(false),
	modified(false)
{
#if 0
	// disable 'X' system menu button
	HMENU hSysMenu = GetSystemMenu(this->Handle, false);
	if (hSysMenu != NULL)
	{
		EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
		DrawMenuBar(this->Handle);
	}
#endif

	frmEditor = new TfrmTextEditor(this);
	asCurrentFile = "";
	frmEditor->callbackTextChange = OnTextModified;
	frmEditor->BorderStyle = bsNone;
	frmEditor->Parent = this;
	frmEditor->Visible = true;

	/** \note Strange: style is not applied without this call (duplicating frmEditor constructor)
	*/
	frmEditor->Init(SC_STYLE_LUA);

	// inform OS that we accepting dropping files
	DragAcceptFiles(Handle, True);	
}
//---------------------------------------------------------------------------

void TfrmLuaScript::SetScript(AnsiString asString)
{
	if (CheckFileNotSavedDialog())
		return;
	if (asString != "")
	{
		modified = true;
		asCurrentFile.sprintf("NewArrayGenerator.lua");
	}
	else
	{
		modified = false;
		asCurrentFile.sprintf("NewScript.lua");
	}
	SetText(asString);
	SetTitle(asCurrentFile, modified);
}

void __fastcall TfrmLuaScript::btnExecuteClick(TObject *Sender)
{
	BtnController btnCtrl(dynamic_cast<TButton*>(Sender));
	breakRequest = false;
	running = true;
	bool handled = true;
	callbackRunScript(SCRIPT_SRC_SCRIPT_WINDOW, -1, frmEditor->GetText().c_str(), breakRequest, handled);
	running = false;
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::btnBreakClick(TObject *Sender)
{
	breakRequest = true;
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if (CheckFileNotSavedDialog())
	{
		CanClose = false;
		return;
	}
	breakRequest = true;
	while (running)
	{
		Application->ProcessMessages();
		Sleep(100);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::FormCreate(TObject *Sender)
{
	UpdateMruMenu();
	UpdateExamplesMenu();
}
//---------------------------------------------------------------------------

void TfrmLuaScript::UpdateExamplesMenu(void)
{
	miHelpExamples->Clear();
	miHelpExamples->AutoHotkeys = maManual;
	for (unsigned int i=0; i<sizeof(luaExamples)/sizeof(luaExamples[0]); i++)
	{
		TMenuItem* item = new TMenuItem(MainMenu);
		item->AutoHotkeys = maManual;
		item->Caption = luaExamples[i].name;
		item->Tag = i;
		item->OnClick = LoadExample;
		miHelpExamples->Add(item);
	}
}

void __fastcall TfrmLuaScript::LoadExample(TObject *Sender)
{
	if (CheckFileNotSavedDialog())
		return;
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	int id = item->Tag;
	if (id < 0 || (unsigned)id >= sizeof(luaExamples)/sizeof(luaExamples[0]))
	{
		ShowMessage("Tag out of range!");
		return;
	}
	modified = false;
	SetText(luaExamples[id].lua);
	asCurrentFile.sprintf("Example%d.lua", id+1);
	SetTitle(asCurrentFile, modified);
}

void __fastcall TfrmLuaScript::actFileOpenExecute(TObject *Sender)
{
	if (CheckFileNotSavedDialog())
		return;
	OpenDialog->InitialDir = appSettings.ScriptWindow.lastDir;
	if (OpenDialog->Execute())
	{
		appSettings.ScriptWindow.lastDir = ExtractFileDir(OpenDialog->FileName);
		OpenFile(OpenDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void TfrmLuaScript::OpenFile(AnsiString filename)
{
	AddMruItem(filename);
	std::auto_ptr<TStrings> strings(new TStringList());
	modified = false;
	if (FileExists(filename))
	{
		try
		{
			strings->LoadFromFile(filename);
		}
		catch(...)
		{
			MessageBox(this->Handle, "Failed to load data from file.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
	}
	else
	{
		MessageBox(this->Handle, "File doesn't exist.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}
	
	SetText(strings->Text);
	asCurrentFile = filename;
	SetTitle(filename, modified);
}

void __fastcall TfrmLuaScript::actFileSaveExecute(TObject *Sender)
{
	if (ExtractFileDir(asCurrentFile) != "")
	{
		TStrings *strings = new TStringList();
		strings->Text = frmEditor->GetText();
		strings->SaveToFile(asCurrentFile);
		delete strings;
		//frmJsonEditor->ClearFileChangedFlag();
		modified = false;
		SetTitle(asCurrentFile, modified);
	}
	else
	{
		actFileSaveAsExecute(Sender);
	}	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::actFileSaveAsExecute(TObject *Sender)
{
	SaveDialog->InitialDir = appSettings.ScriptWindow.lastDir;
	if (asCurrentFile != "")
	{
    	SaveDialog->FileName = asCurrentFile;
	}
	if (SaveDialog->Execute())
	{
		AnsiString asFileName;
		if (SaveDialog->FilterIndex == 1)
			asFileName = ChangeFileExt(SaveDialog->FileName, ".lua");
		else if (SaveDialog->FilterIndex == 2)
			asFileName = ChangeFileExt(SaveDialog->FileName, ".txt");

		AddMruItem(asFileName);
		appSettings.ScriptWindow.lastDir = ExtractFileDir(asFileName);
		if (FileExists(asFileName))
		{
			if (MessageBox(this->Handle, "File already exists, file will be overwritten.\r\nAre you sure you want to continue?",
				this->Caption.c_str(), MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDYES)
			{
				return;
			}
		}
		asCurrentFile = asFileName;
		SetTitle(asCurrentFile, false);
		TStrings *strings = new TStringList();
		strings->Text = frmEditor->GetText();
		strings->SaveToFile(asFileName);
		delete strings;
		//frmJsonEditor->ClearFileChangedFlag();
	}
}

void TfrmLuaScript::SetTitle(AnsiString filename, bool modified)
{
	AnsiString title;
	if (filename == "")
	{
		title.sprintf("[new file] - LuaScript");
	}
	else
	{
		if (modified)
		{
			title.sprintf("[%s *] - [%s] - LuaScript",
				ExtractFileName(filename).c_str(), ExtractFileDir(filename).c_str());
		}
		else
		{
			title.sprintf("[%s] - [%s] - LuaScript",
				ExtractFileName(filename).c_str(), ExtractFileDir(filename).c_str());
		}
	}
	//Application->Title = title;
	this->Caption = title;
	if (modified == false)
	{
    	//frmJsonEditor->ClearFileChangedFlag();
	}
}


void TfrmLuaScript::AddMruItem(AnsiString filename)
{
	appSettings.ScriptWindow.AddMru(filename);
	// re-create MRU items submenu
}

void TfrmLuaScript::UpdateMruMenu(void)
{
	miOpenRecent->Clear();
	TMenuItem *item;
	miOpenRecent->AutoHotkeys = maManual;
	if (appSettings.ScriptWindow.MRU.empty())
	{
		item = new TMenuItem(MainMenu);
		item->AutoHotkeys = maManual;
		item->Caption = "- empty -";
		item->Enabled = false;
		miOpenRecent->Insert(0, item);
	}
	for (unsigned int i=0; i<appSettings.ScriptWindow.MRU.size(); i++)
	{
		item = new TMenuItem(MainMenu);
		item->AutoHotkeys = maManual;
		item->Caption = appSettings.ScriptWindow.MRU[i];
		item->OnClick = MruClick;
		miOpenRecent->Insert(0, item);
	}
	item = new TMenuItem(MainMenu);
	item->Caption = "-";	// separator
	miOpenRecent->Add(item);
	item = new TMenuItem(MainMenu);
	item->Caption = "Remove obsolete files";
	item->OnClick = RemoveObsoleteFilesClick;
	miOpenRecent->Add(item);
	item = new TMenuItem(MainMenu);
	item->Caption = "Clear list";
	item->OnClick = ClearMruClick;
	miOpenRecent->Add(item);	
}

void __fastcall TfrmLuaScript::MruClick(TObject *Sender)
{
	if (CheckFileNotSavedDialog())
		return;
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	OpenFile(item->Caption);	
}

void __fastcall TfrmLuaScript::RemoveObsoleteFilesClick(TObject *Sender)
{
	std::deque<AnsiString>::iterator iter;
	for (iter = appSettings.ScriptWindow.MRU.begin(); iter != appSettings.ScriptWindow.MRU.end(); )
	{
		bool exist = FileExists(*iter);
		if (exist)
		{
			++iter;
		}
		else
		{
			appSettings.ScriptWindow.MRU.erase(iter++);
        }
	}
	UpdateMruMenu();	
}

void __fastcall TfrmLuaScript::ClearMruClick(TObject *Sender)
{
	appSettings.ScriptWindow.MRU.clear();
	UpdateMruMenu();
}

int TfrmLuaScript::CheckFileNotSavedDialog(void)
{
	if (modified)
	{
		int res = MessageBox(this->Handle, "Save current file?", "Current file not saved",
			MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONEXCLAMATION);
		if (res == ID_YES)
		{
			actFileSaveAs->Execute();
		}
		else if (res == ID_CANCEL)
		{
			return 1;
		}
	}
	return 0;
}


void TfrmLuaScript::OnTextModified(void)
{
	modified = true;
	SetTitle(asCurrentFile, modified);	
}

void TfrmLuaScript::SetText(AnsiString text)
{
	bool old = modified;
	frmEditor->SetText(text);
	modified = old;
}

void __fastcall TfrmLuaScript::miFileClick(TObject *Sender)
{
	UpdateMruMenu();
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::FormClose(TObject *Sender, TCloseAction &Action)
{
	Action = caFree;
}
//---------------------------------------------------------------------------

// fires an event when a file, or files are dropped onto the application.
void __fastcall TfrmLuaScript::WMDropFiles(TWMDropFiles &message)
{
    AnsiString FileName;
    FileName.SetLength(MAX_PATH);

	int Count = DragQueryFile((HDROP)message.Drop, 0xFFFFFFFF, NULL, MAX_PATH);

	// ignore all files but first one
	if (Count > 1)
		Count = 1;

	if (CheckFileNotSavedDialog() == 0)
	{
		// index through the files and query the OS for each file name...
		for (int index = 0; index < Count; ++index)
		{
			// the following code gets the FileName of the dropped file. it
			// looks cryptic but that's only because it is. Hey, Why do you think
			// Delphi and C++Builder are so popular anyway? Look up DragQueryFile
			// the Win32.hlp Windows API help file.
			FileName.SetLength(DragQueryFile((HDROP)message.Drop, index,FileName.c_str(), MAX_PATH));
            //appSettings.Editor.asDefaultDir = ExtractFileDir(FileName);
			OpenFile(FileName);
		}
	}
    // tell the OS that you're finished...
	DragFinish((HDROP) message.Drop);
}

void __fastcall TfrmLuaScript::miCloseWindowClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------


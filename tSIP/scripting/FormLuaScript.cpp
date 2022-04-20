//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormLuaScript.h"
#include "FormTextEditor.h"
#include "FormLuaScriptHelp.h"
#include "ScriptExec.h"
#include "common/BtnController.h"
#include "LuaExamples.h"
#include "Settings.h"
#include "ScriptSource.h"
#include "Log.h"
#include "Paths.h"
#include <memory>
#include <assert.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmLuaScript *frmLuaScript = NULL;
//---------------------------------------------------------------------------

namespace
{

TfrmLuaScript::CallbackRunScript callbackRunScript = NULL;

bool GetNextLine(AnsiString &Line, AnsiString &buf, char &lastchar)
{
	AnsiString res;
	int d, a;
	d = buf.Pos((char)13);
	a = buf.Pos((char)10);
	if ((d == 0) && (a == 0)) return false;		// no line-ending

	// Normal line ending: 13,10 (MingW) or 10 (CygWin)
	// Status-line ending: 13 (MingW and CygWin)

	// drawback: under MingW, every line has to be processed for statusline format,
	// because line type cannot be determined correctly by line ending

	if (a != 0) {
		// there is a 10
		if ((a == 1) && (lastchar == 13)) {
			// if a 13,10 ending was split
			// do not erase Line, which should contain previous line
			buf.Delete(1,1);
			lastchar = 10;
			return true;
		}
		if (d == a-1) {
			// line ending: 13,10
			Line = buf.SubString(1, a-2);
			buf.Delete(1, a);
			lastchar = 10;
			return true;
		}
		if ((d == 0) || (d > a)) {
			// line ending: 10
			Line = buf.SubString(1, a-1);
			buf.Delete(1, a);
			lastchar = 10;
			return true;
		}
	}
	if (d != 0) {
		// line ending: 13
		Line = buf.SubString(1, d-1);
		buf.Delete(1, d);
		lastchar = 13;
		return true;
	}
	return false;
}

}	// namespace

void TfrmLuaScript::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	callbackRunScript = cb;
	TfrmLuaScriptHelp::SetCallbackRunScript(cb);
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
	this->modified = modified;
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

void __fastcall TfrmLuaScript::btnLuacheckClick(TObject *Sender)
{
	AnsiString appName = ExtractFileDir(Application->ExeName) + "\\luacheck.exe";
	if (!FileExists(appName))
	{
		MessageBox(this->Handle, "Luacheck.exe must be placed in application directory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}
	AnsiString asFile = asCurrentFile;
	bool tmpFile = false;
	if (!FileExists(asFile) || modified)
	{
		asFile.sprintf("%s\\scripts\\%s", Paths::GetProfileDir().c_str(), "tmp_luacheck.lua");
		TStrings *strings = new TStringList();
		strings->Text = frmEditor->GetText();
		try
		{
			strings->SaveToFile(asFile);
			tmpFile = true;
		}
		catch(...)
		{
			MessageBox(this->Handle, "Failed to save temporary file for luacheck.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			delete strings;
			return;
		}
		delete strings;
	}

	BOOL ok = TRUE;
	HANDLE hStdInPipeRead = NULL;
	HANDLE hStdInPipeWrite = NULL;
	HANDLE hStdOutPipeRead = NULL;
	HANDLE hStdOutPipeWrite = NULL;

	// Create two pipes.
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	ok = CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0);
	if (ok == FALSE)
	{
		MessageBox(this->Handle, "Failed to create pipe.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		return;
	}
	ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
	if (ok == FALSE)
	{
		MessageBox(this->Handle, "Failed to create pipe.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		CloseHandle(hStdInPipeRead);
		return;
	}

	AnsiString command = (AnsiString)"\"" + appName + "\" \"" + asFile + "\" --formatter plain ";

	const std::vector<ScriptExec::Symbol> &globalsSet = ScriptExec::GetSymbols();
	if (globalsSet.empty())
	{
        // run once empty script to fill global function list
		BtnController btnCtrl(dynamic_cast<TButton*>(Sender));
		breakRequest = false;
		running = true;
		bool handled = true;
		callbackRunScript(SCRIPT_SRC_SCRIPT_WINDOW, -1, "", breakRequest, handled);
		running = false;
	}
	if (!globalsSet.empty())
	{
		command += "--globals";
		for (std::vector<ScriptExec::Symbol>::const_iterator iter = globalsSet.begin(); iter != globalsSet.end(); ++iter)
		{
			command += " ";
			command += iter->name;
		}
	}
    // Create the process.
    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdError = hStdOutPipeWrite;
    si.hStdOutput = hStdOutPipeWrite;
    si.hStdInput = hStdInPipeRead;
	PROCESS_INFORMATION pi = { 0 };
    LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
    LPSECURITY_ATTRIBUTES lpThreadAttribute = NULL;
    BOOL bInheritHandles = TRUE;
    DWORD dwCreationFlags = CREATE_NO_WINDOW;
    LPVOID lpEnvironment = NULL;
    const char* lpCurrentDirectory = NULL;
    ok = CreateProcess(
        appName.c_str(),
        command.c_str(),
        lpProcessAttributes,
        lpThreadAttribute,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        &si,
        &pi);
	if (ok == FALSE)
	{
		MessageBox(this->Handle, "Failed to start luacheck.exe.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		CloseHandle(hStdOutPipeRead);
		CloseHandle(hStdInPipeWrite);
		return;
	}

    // Close pipes we do not need.
    CloseHandle(hStdOutPipeWrite);
    CloseHandle(hStdInPipeRead);

	AnsiString outputText;

    // The main loop for reading output from the DIR command.
    char buf[1024 + 1] = { 0 };
    DWORD dwRead = 0;
    DWORD dwAvail = 0;
	ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
	LOG("Luacheck output:\n");
    while (ok == TRUE)
    {
        buf[dwRead] = '\0';
        LOG("%s", buf);
		outputText += buf;
        ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
    }
	LOG("End of luacheck output\n");

    // Clean up and exit.
	CloseHandle(hStdOutPipeRead);
	CloseHandle(hStdInPipeWrite);
    DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	validationEntries.clear();

	pnlBottom->Height = pnlBottom2->Height + lvValidation->Height;
	lvValidation->Visible = true;

	bool ok2;
	AnsiString Line;
	char lastChar = 0;
	do
	{
		ok2 = GetNextLine(Line, outputText, lastChar);
		if (ok2)
		{
			Line = Line.Trim();
			if (Line == "")
				continue;
			char* line = Line.c_str();
			if (
				strncmp(line, "Checking ", strlen("Checking ")) == 0 ||
				strncmp(line, "Total:", strlen("Total:")) == 0
				)
				continue;
			// "    Example12.lua: couldn't read: No such file or directory"
			// "    G:\\tools\\tSIP\\tSIP\\tSIP\\Debug_Build\\scripts\\Example12.lua:1:21: accessing undefined variable 'GetAudioErrorCount'"
			ValidationEntry entry;
			entry.message = Line;
			char *p = strchr(Line.c_str(), ':');
			if (p)
			{
				p = strchr(p+1, ':');
			}
			if (p)
			{
				*p = '\0';
				entry.file = Line.c_str();
				p++;
				entry.message = p;
				sscanf(p, "%d:%d:", &entry.line, &entry.position);
				p = strchr(p, ' ');
				if (p)
				{
					entry.message = p + 1;
				}
			}
			validationEntries.push_back(entry);
		}
	} while (ok2);

	lvValidation->Items->Count = validationEntries.size();
	lvValidation->Invalidate();
	if (tmpFile)
	{
		DeleteFile(asFile);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::lvValidationData(TObject *Sender,
      TListItem *Item)
{
	int id = Item->Index;
	const ValidationEntry &entry = validationEntries[id];
	Item->Caption = ExtractFileName(entry.file);
	if (entry.line >= 0)
	{
		Item->SubItems->Add(entry.line);
	}
	else
	{
		Item->SubItems->Add("");
	}
	if (entry.position >= 0)
	{
		Item->SubItems->Add(entry.position);
	}
	else
	{
		Item->SubItems->Add("");
	}
	Item->SubItems->Add(entry.message);	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::FormShow(TObject *Sender)
{
	lvValidation->Visible = false;
	pnlBottom->Height = pnlBottom2->Height;
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::lvValidationDblClick(TObject *Sender)
{
	TListItem *item = lvValidation->Selected;
	if (!item)
		return;
	int id = item->Index;
	const ValidationEntry &entry = validationEntries[id];
	if (entry.line > 0)
	{
		frmEditor->MarkError(entry.line, entry.position > 0 ? entry.position : 1);
		frmEditor->SetSciFocus();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::miCustomLuaFunctionsClick(TObject *Sender)
{
	frmLuaScriptHelp->Show();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScript::miFileNewClick(TObject *Sender)
{
	TfrmLuaScript *frmLuaScript = new TfrmLuaScript(this->Owner);
	frmLuaScript->Show();
	int left = Left + 20;
	int top = Top + 20;
	if (left >= Screen->Width - 60)
		left = 0;
	if (top >= Screen->Height - 60)
		top = 0;
	frmLuaScript->Left = left;
	frmLuaScript->Top = top;
	// would be destroyed with caFree
}
//---------------------------------------------------------------------------


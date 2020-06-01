//---------------------------------------------------------------------------


#pragma hdrstop

#include "ScEdit.h"
#define SCI_NAMESPACE	// required because of class name collision ("Action")
#include <Scintilla.h>
#include <SciLexer.h>
#include <assert.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)


void __fastcall TScEdit::CreateParams(Controls::TCreateParams &Params)
{
	TWinControl::CreateParams(Params);
	CreateSubClass(Params, "Scintilla");
}

void __fastcall TScEdit::WndProc(Messages::TMessage &Message)
{
	//LOG("TScEdit Msg = %d\n", Message.Msg);
	TWinControl::WndProc(Message);
	if(Message.Msg == WM_GETDLGCODE)
	{
		Message.Result = DLGC_WANTALLKEYS|DLGC_WANTARROWS|DLGC_WANTTAB;
	}
}

void TScEdit::setCppStyle(void)
{
	const char* szKeywords1=
		"true false null";
	//const char* szKeywords2=
	//	"bool char float int long short void wchar_t";

	SendEditor(SCI_STYLECLEARALL);

	SendEditor(SCI_SETLEXER, SCLEX_CPP);
	SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);
	//SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);

	SendEditor(SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_C_WORD2, 0x00800080);
	SendEditor(SCI_STYLESETBOLD, SCE_C_WORD2, TRUE);
	SendEditor(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3);
	SendEditor(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3);
	SendEditor(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x00808080);
	SendEditor(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00008000);
	SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00008000);
	SendEditor(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x00008000);

	SendEditor(SCI_STYLESETBACK, STYLE_BRACELIGHT, 0x00777777);
	SendEditor(SCI_STYLESETFORE, STYLE_BRACELIGHT, 0x00FFFFFF);
	SendEditor(SCI_STYLESETBACK, STYLE_BRACEBAD, 0x000000FF);

	SendEditor(SCI_SETCARETLINEVISIBLE, TRUE);
	SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
}

void TScEdit::setJsonStyle(void)
{
	const char* szKeywords1=
		"true false null";
	//const char* szKeywords2=
	//	"bool char float int long short void wchar_t";

	SendEditor(SCI_STYLECLEARALL);

	SendEditor(SCI_SETLEXER, SCLEX_JSON);
	SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);

	SendEditor(SCI_STYLESETFORE, SCE_JSON_DEFAULT, 0x00000000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_NUMBER, 0x00FF0000);	// blue
	SendEditor(SCI_STYLESETFORE, SCE_JSON_STRING, 0x00FF0080);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_STRINGEOL, 0x00FF0080);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_PROPERTYNAME, 0x00000080);	// maroon
	SendEditor(SCI_STYLESETFORE, SCE_JSON_ESCAPESEQUENCE, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_LINECOMMENT, 0x0000FF00);	// green
	SendEditor(SCI_STYLESETFORE, SCE_JSON_BLOCKCOMMENT, 0x0000FF00);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_OPERATOR, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_URI, 0x00FF0000);
	SendEditor(SCI_STYLESETUNDERLINE, SCE_JSON_URI, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_COMPACTIRI, 0x00000000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_KEYWORD, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_LDKEYWORD, 0x00FF0000);
	SendEditor(SCI_STYLESETFORE, SCE_JSON_ERROR, 0x000000FF);

	SendEditor(SCI_STYLESETBACK, STYLE_BRACELIGHT, 0x00777777);
	SendEditor(SCI_STYLESETFORE, STYLE_BRACELIGHT, 0x00FFFFFF);
	SendEditor(SCI_STYLESETBACK, STYLE_BRACEBAD, 0x000000FF);

	SendEditor(SCI_SETCARETLINEVISIBLE, TRUE);
	SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
}

void TScEdit::setLuaStyle(void)
{

	static const char lua_instre1[] = "and break do else elseif end false for function if in local nil not or repeat return then true until while";
	static const char lua_instre2[] = "_VERSION assert collectgarbage dofile error gcinfo loadfile loadstring print tonumber tostring type unpack _ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT _STDERR _STDIN _STDOUT call dostring foreach foreachi getn globals newtype rawget rawset require sort tinsert tremove _G getfenv getmetatable ipairs loadlib next pairs pcall rawegal rawget rawset require setfenv setmetatable xpcall string table math coroutine io os debug";
	static const char lua_type1[] = "abs acos asin atan atan2 ceil cos deg exp floor format frexp gsub ldexp log log10 max min mod rad random randomseed sin sqrt strbyte strchar strfind strlen strlower strrep strsub strupper tan string.byte string.char string.dump string.find string.len string.lower string.rep string.sub string.upper string.format string.gfind string.gsub table.concat table.foreach table.foreachi table.getn table.sort table.insert table.remove table.setn math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.deg math.exp math.floor math.frexp math.ldexp math.log math.log10 math.max math.min math.mod math.pi math.rad math.random math.randomseed math.sin math.sqrt math.tan";
	static const char lua_type2[] = "openfile closefile readfrom writeto appendto remove rename flush seek tmpfile tmpname read write clock date difftime execute exit getenv setlocale time coroutine.create coroutine.resume coroutine.status coroutine.wrap coroutine.yield io.close io.flush io.input io.lines io.open io.output io.read io.tmpfile io.type io.write io.stdin io.stdout io.stderr os.clock os.date os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale os.time os.tmpname";


	SendEditor(SCI_SETLEXER, SCLEX_LUA);
	SendEditor(SCI_SETSTYLEBITS, SendEditor(SCI_GETSTYLEBITSNEEDED));

	SendEditor(SCI_SETKEYWORDS, 0,
		reinterpret_cast<LPARAM>(lua_instre1));
	SendEditor(SCI_SETKEYWORDS, 1,
		reinterpret_cast<LPARAM>(lua_instre2));
	SendEditor(SCI_SETKEYWORDS, 2,
		reinterpret_cast<LPARAM>(lua_type1));
	SendEditor(SCI_SETKEYWORDS, 3,
		reinterpret_cast<LPARAM>(lua_type2));


	SendEditor(SCI_STYLECLEARALL);

	SendEditor(SCI_STYLESETFORE, SCE_LUA_DEFAULT,			(int)RGB(0x00, 0x00, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENT,			(int)RGB(0x00, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE,		(int)RGB(0x00, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC,		(int)RGB(0x00, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_LITERALSTRING,		(int)RGB(0x95, 0x00, 0x4A));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_PREPROCESSOR,		(int)RGB(0x80, 0x40, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD,				(int)RGB(0x00, 0x00, 0xFF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_NUMBER,			(int)RGB(0xFF, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_STRING,			(int)RGB(0x80, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_CHARACTER,			(int)RGB(0x80, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_OPERATOR,			(int)RGB(0x00, 0x00, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD2,				(int)RGB(0x00, 0x80, 0xCF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD3,				(int)RGB(0x80, 0x00, 0xFF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD4,				(int)RGB(0x00, 0x00, 0xA0));


	//SendEditor(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	//SendEditor(SCI_SETMARGINWIDTHN, 0, SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, "_9999"));		// TODO: do this dynamic, look for "SetLineNumberWidth" in scite code

	//SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
	//SendEditor(SCI_SETMARGINWIDTHN, 1, 16);

	SendEditor(SCI_MARKERDEFINE, 0, SC_MARK_ARROW);
#if 0
	const char* szKeywords1=
		"ShowMessage Sleep Call Dial Hangup CheckBreak ClearOutput";
	//const char* szKeywords2=
	//	"bool char float int long short void wchar_t";

	SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)fontName.c_str());
	SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
	SendEditor(SCI_STYLECLEARALL);

	SendEditor(SCI_SETLEXER, SCLEX_LUA);
	SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);
	//SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);

	SendEditor(SCI_STYLESETFORE, SCE_LUA_DEFAULT,			(int)RGB(0x00, 0x00, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENT,			(int)RGB(0x00, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE,		(int)RGB(0x00, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC,		(int)RGB(0x00, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_LITERALSTRING,		(int)RGB(0x95, 0x00, 0x4A));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_PREPROCESSOR,		(int)RGB(0x80, 0x40, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD,				(int)RGB(0x00, 0x00, 0xFF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_NUMBER,			(int)RGB(0xFF, 0x80, 0x00));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_STRING,			(int)RGB(0x80, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_CHARACTER,			(int)RGB(0x80, 0x80, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_OPERATOR,			(int)RGB(0x00, 0x00, 0x80));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD2,				(int)RGB(0x00, 0x80, 0xCF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD3,				(int)RGB(0x80, 0x00, 0xFF));
	SendEditor(SCI_STYLESETFORE, SCE_LUA_WORD4,				(int)RGB(0x00, 0x00, 0xA0));

	SendEditor(SCI_SETCARETLINEVISIBLE, TRUE);
	SendEditor(SCI_SETCARETLINEBACK, 0xb0ffff);
#endif		
}

void TScEdit::SetExtraStyle(void)
{
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.comment", (LPARAM)"1");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.preprocessor", (LPARAM)"1");

	//Now resize all the margins to zero
	//(This will be done in a RecalcLineMargin method...)
	SendEditor(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 0);

	//Then set the margin type and margin mask and resize it...
	SendEditor(SCI_SETMARGINTYPEN,  MARGIN_SCRIPT_FOLD_INDEX, SC_MARGIN_SYMBOL);
	SendEditor(SCI_SETMARGINMASKN, MARGIN_SCRIPT_FOLD_INDEX, SC_MASK_FOLDERS);
	SendEditor(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 20);

	//Set some visual preferences
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);

	SendEditor(SCI_SETFOLDFLAGS, 16, 0); // 16  	Draw line below if not expanded

	//Tell scintilla we want to be notified about mouse clicks in the margin
	SendEditor(SCI_SETMARGINSENSITIVEN, MARGIN_SCRIPT_FOLD_INDEX, 1);

	SendEditor(SCI_SETTABWIDTH, 4);

	SendEditor(SCI_SETMODEVENTMASK, SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT, 0);

	// Add line numbers
	SendEditor(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
#if 1
	char *text = "00";
	int width = SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (WPARAM)text);
	SendEditor(SCI_SETMARGINWIDTHN, 0, width+4);
#endif
	// Wider (2 pixels) caret
	SendEditor(SCI_SETCARETWIDTH, 2, 0);
}

void TScEdit::setStyle(enum ScEditStyle style)
{
	if (style == SC_STYLE_LUA)
	{
		setLuaStyle();
	}
	else if (style == SC_STYLE_CPP)
	{
		setCppStyle();
	}
	else if (style == SC_STYLE_JSON)
	{
		setJsonStyle();
	}
	else
	{
        assert(!"Unhandled SC style!");
	}
	SetExtraStyle();
}

void TScEdit::setFont(AnsiString name, int size)
{
//		// Set up the global default style. These attributes are used wherever no explicit choices are made.
//		SetAStyle(STYLE_DEFAULT, black, white, 10, "Courier New");
//		SendEditor(SCI_STYLECLEARALL);	// Copies global style to all others
	SendEditor(SCI_STYLESETFONT, STYLE_DEFAULT, (sptr_t)name.c_str());
	SendEditor(SCI_STYLESETSIZE, STYLE_DEFAULT, size);
}


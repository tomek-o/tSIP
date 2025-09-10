 //---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTextEditor.h"
#include "ScEdit.h"
#include "scripting/FormLuaScriptHelp.h"
#define SCI_NAMESPACE
#include <Scintilla.h>
#include <SciLexer.h>
#include "Log.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmTextEditor *frmTextEditor;

using namespace Scintilla;

//---------------------------------------------------------------------------
__fastcall TfrmTextEditor::TfrmTextEditor(TComponent* Owner)
	: TForm(Owner),
	callbackTextChange(NULL),
	m_se(NULL),
	fontName("Courier New"),
	fontSize(10),
	scStyle(SC_STYLE_LUA)	
{
	static bool once = false;
	if (!once)
	{
		once = true;
		if ( !Scintilla_RegisterClasses( GetModuleHandle(NULL) ) )
		{
			ShowMessage( "Failed to register Scintilla class" );
			return;
		}
	}
	m_se = new TScEdit (this);
	m_se-> Parent = this;
	m_se-> Align = alClient;
	m_se->setStyle(scStyle);
	m_se->setFont(fontName, fontSize);
}
//---------------------------------------------------------------------------

void TfrmTextEditor::Init(enum ScEditStyle style)
{
	m_se->setFont(fontName, fontSize);
	this->scStyle = style;
	m_se->setStyle(style);
}

void TfrmTextEditor::SetText(AnsiString text)
{
	m_se->SendEditor(SCI_SETTEXT, 0, (WPARAM)text.c_str());
	m_se->SendEditor(SCI_EMPTYUNDOBUFFER, 0, 0);	
}

AnsiString TfrmTextEditor::GetText(void)
{
	AnsiString ret;
	unsigned int nLength = m_se->SendEditor(SCI_GETLENGTH, 0, 0);
	ret.SetLength(nLength+1);	// +1 for null-termination required by Scintilla
	m_se->SendEditor(SCI_GETTEXT, nLength+1, (WPARAM)ret.data());
	ret.SetLength(nLength);		// remove trailing null-termination (not a part of AnsiString)
	return ret;
}

void TfrmTextEditor::MarkError(int line, int column)
{
	m_se->SendEditor(SCI_GOTOLINE, line - 1, 0);
	unsigned int pos = m_se->SendEditor(SCI_GETCURRENTPOS, 0, 0);
	m_se->SendEditor(SCI_SETSELECTION, pos + column - 1, pos + column);
}


__fastcall TfrmTextEditor::~TfrmTextEditor()
{
	delete m_se;
}

void __fastcall TfrmTextEditor::WMNotify(TMessage &Message)
{
	LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(Message.LParam);
	if (lpnmh->hwndFrom == m_se->Handle)
    {
        switch (lpnmh->code)
        {
		case SCN_MARGINCLICK:
		{
			SCNotification* notify = (SCNotification*)Message.LParam;

            //const int modifiers = notify->modifiers;
            const int position = notify->position;
            const int margin = notify->margin;
            const int line_number = m_se->SendEditor(SCI_LINEFROMPOSITION, position, 0);

            switch (margin)
            {
            case TScEdit::MARGIN_SCRIPT_FOLD_INDEX:
            {
                m_se->SendEditor(SCI_TOGGLEFOLD, line_number, 0);
            }
            break;
            }
		}
		break;

		case SCN_MODIFIED:	//SCEN_CHANGE:
		{
			if (callbackTextChange)
				callbackTextChange();
			m_se->updateMarginWidth(false);
		}
		break;

		case SCN_UPDATEUI:
		{
		#if 0
			if (braceAtCaret >= 0 && braceOpposite < 0)
			{
				SendScintilla(SCI_BRACEBADLIGHT,braceAtCaret);
						SendScintilla(SCI_SETHIGHLIGHTGUIDE,0UL);
			}
		#endif
			// do brace matching
			int lStart = m_se->SendEditor(SCI_GETCURRENTPOS, 0, 0);
			int lEnd = m_se->SendEditor(SCI_BRACEMATCH, lStart-1, 0);
			// if there is a matching brace highlight it
			if (lEnd >= 0)
				m_se->SendEditor(SCI_BRACEHIGHLIGHT, lStart-1, lEnd);
			// if there is NO matching brace erase old highlight
			else
				m_se->SendEditor(SCI_BRACEHIGHLIGHT, -1, -1);
		}
		break;

		case SCN_HOTSPOTRELEASECLICK:
		{
			SCNotification* notify = (SCNotification*)Message.LParam;
			int startPosition = m_se->SendEditor(SCI_WORDSTARTPOSITION, notify->position, false);
			int endPosition = m_se->SendEditor(SCI_WORDENDPOSITION, notify->position, false);

			// GetTextRange
			AnsiString word;
			unsigned int nLength = endPosition - startPosition;
			word.SetLength(nLength+1);	// +1 for null-termination required by Scintilla
			Sci_TextRange tr;
			tr.chrg.cpMin = startPosition;
			tr.chrg.cpMax = endPosition;
			tr.lpstrText = (char*)word.data();
			m_se->SendEditor(SCI_GETTEXTRANGE, 0, (WPARAM)&tr);
			word.SetLength(nLength);		// remove trailing null-termination (not a part of AnsiString)

			if (frmLuaScriptHelp == NULL)
			{
				Application->CreateForm(__classid(TfrmLuaScriptHelp), &frmLuaScriptHelp);
			}
			frmLuaScriptHelp->ShowForKeyword(word);
		}
		break;

		default:
		break;
		}
	}

	TForm::Dispatch(&Message);
}

void TfrmTextEditor::Search(AnsiString text, const TStringSearchOptions &so)
{
	if (so.Contains(soDown))
	{
		m_se->SendEditor(SCI_CHARRIGHT);
		m_se->SendEditor(SCI_SEARCHANCHOR);
		m_se->SendEditor(SCI_SEARCHNEXT,
			so.Contains(soMatchCase)?SCFIND_MATCHCASE:0 | so.Contains(soWholeWord)?SCFIND_WHOLEWORD:0,
			(LPARAM)text.c_str());
		m_se->SendEditor(SCI_SCROLLCARET);
	}
	else
	{
		m_se->SendEditor(SCI_SEARCHANCHOR);
		m_se->SendEditor(SCI_SEARCHPREV,
			so.Contains(soMatchCase)?SCFIND_MATCHCASE:0 | so.Contains(soWholeWord)?SCFIND_WHOLEWORD:0,
			(LPARAM)text.c_str());
		m_se->SendEditor(SCI_SCROLLCARET);
	}
}

int TfrmTextEditor::SetFont(AnsiString name, int size, TFontStyles style)
{
	fontName = name;
	fontSize = size;
	m_se->setFont(fontName, fontSize);
	//m_se->SetExtraStyle();
	return 0;
}

void TfrmTextEditor::SetSciFocus(void)
{
	m_se->SendEditor(SCI_GRABFOCUS, 1, 0);
}

void TfrmTextEditor::SetLineWrap(bool state)
{
	if (state)
	{
		// Line wrap
		m_se->SendEditor(SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
		m_se->SendEditor(SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_MARGIN, 0);
		m_se->SendEditor(SCI_SETWRAPINDENTMODE, SC_WRAPINDENT_SAME, 0);
	}
	else
	{
		m_se->SendEditor(SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
	}
}

void TfrmTextEditor::SetStyle(enum ScEditStyle style)
{
	m_se->setStyle(style);
	m_se->updateMarginWidth(true);
}

AnsiString TfrmTextEditor::GetSelectedText(void)
{
	// Find the length of the text
	int textLength = m_se->SendEditor(SCI_GETSELTEXT, 0, NULL);
	AnsiString text;
	if (textLength > 0)
	{
		text.SetLength(textLength);
		// Get text
		m_se->SendEditor(SCI_GETSELTEXT, 0, (LPARAM)text.data());
	}
	return text;
}






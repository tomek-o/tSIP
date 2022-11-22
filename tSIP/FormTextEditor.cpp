 //---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTextEditor.h"
#include "ScEdit.h"
#define SCI_NAMESPACE
#include <Scintilla.h>
#include <SciLexer.h>
#include "Log.h"
#include <math.h>
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
	lineCount(10),
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
	ret.SetLength(nLength+2);
	m_se->SendEditor(SCI_GETTEXT, nLength+1, (WPARAM)ret.data());
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
			UpdateMarginWidth(false);
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


		default:
		break;
		}
	}

	TForm::Dispatch(&Message);
}

void TfrmTextEditor::UpdateMarginWidth(bool force)
{
	int lines = m_se->SendEditor(SCI_GETLINECOUNT, 0, 0);
	if (lines < 10)
		lines = 10;
	if (force || lines/lineCount >= 10 || lineCount > lines)
	{
		int n = (int)log10(lines);
		lineCount = static_cast<int>(pow(10.0, n) + 0.5);
		char *text;
		switch (n)
		{
		case 0:
		case 1:
			text = "00";
			break;
		case 2:
			text = "000";
			break;
		case 3:
			text = "0000";
			break;
		case 4:
			text = "00000";
			break;
		default:
			text = "000000";
			break;
		}

		int width = m_se->SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (WPARAM)text);
		m_se->SendEditor(SCI_SETMARGINWIDTHN, 0, width + 4);
	}
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
	int TODO__MOVE_MARGIN_CALCULATION_TO_SC_EDIT;
	UpdateMarginWidth(true);
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






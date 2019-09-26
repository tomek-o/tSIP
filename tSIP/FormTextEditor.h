//---------------------------------------------------------------------------

#ifndef FormTextEditorH
#define FormTextEditorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include <StrUtils.hpp>	// TStringSearchOptions; required only on some TC installations?
#include "ScEditStyle.h"

class TScEdit;

class TfrmTextEditor : public TForm
{
__published:	// IDE-managed Components
private:	// User declarations
	TScEdit * m_se;
	typedef void (__closure *CallbackTextChange)(void);
	int lineCount;
	AnsiString fontName;
	int fontSize;
	void UpdateMarginWidth(bool force);
protected:
	void __fastcall WMNotify(TMessage &Message);
	BEGIN_MESSAGE_MAP
		VCL_MESSAGE_HANDLER(WM_NOTIFY, TMessage, WMNotify)
	END_MESSAGE_MAP(TForm);	
public:		// User declarations
	__fastcall TfrmTextEditor(TComponent* Owner);
	__fastcall ~TfrmTextEditor();
#if 0
	sptr_t SendEditor(unsigned int iMessage, uptr_t wParam = 0, sptr_t lParam = 0)
	{
		return m_fnDirect(m_ptrDirect, iMessage, wParam, lParam);
	}
#endif
	CallbackTextChange callbackTextChange;
	void SetText(AnsiString text);
	void SetSciFocus(void);
	AnsiString GetText(void);
	AnsiString GetSelectedText(void);
	void MarkError(int line, int column);
	void Search(AnsiString text, const TStringSearchOptions &so);
	int SetFont(AnsiString name, int size, TFontStyles style);
	void SetLineWrap(bool state);
	void SetStyle(enum ScEditStyle style);	


	void Init(enum ScEditStyle style);
private:
	enum ScEditStyle scStyle;
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmTextEditor *frmTextEditor;
//---------------------------------------------------------------------------
#endif

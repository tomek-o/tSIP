//---------------------------------------------------------------------------

#ifndef ScEditH
#define ScEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include "ScEditStyle.h"

class TScEdit : public TWinControl
{
protected:
	virtual void __fastcall CreateParams(Controls::TCreateParams &Params);
	virtual void __fastcall WndProc(Messages::TMessage &Message);
	void setCppStyle(void);
	void setJsonStyle(void);
	void setLuaStyle(void);
	void SetExtraStyle(void);
public:
	enum { MARGIN_SCRIPT_FOLD_INDEX = 1 };

	__fastcall TScEdit(Classes::TComponent* AOwner)
		:TWinControl(AOwner){;}

	LONG_PTR SendEditor(unsigned int iMessage, ULONG_PTR wParam = 0, LONG_PTR lParam = 0)
	{
		return SendMessage(Handle, iMessage, wParam, lParam);
	}
	/** \note must be called BEFORE setStyle */
	void setFont(AnsiString name, int size);
	/** \note must be called AFTER setFont */
	void setStyle(enum ScEditStyle style);
};


#endif

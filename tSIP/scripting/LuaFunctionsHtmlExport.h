//---------------------------------------------------------------------------

#ifndef LuaFunctionsHtmlExportH
#define LuaFunctionsHtmlExportH
//---------------------------------------------------------------------------

#include <System.hpp>

namespace LuaFunctionsHtmlExport
{
	/** \brief Export list of custom Lua functions (name, brief description, details) to a single, human-readable HTML file
		\return 0 on success, non-zero on failure
	*/
	int Write(AnsiString fileName);
}

#endif

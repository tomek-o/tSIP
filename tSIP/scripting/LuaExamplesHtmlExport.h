//---------------------------------------------------------------------------

#ifndef LuaExamplesHtmlExportH
#define LuaExamplesHtmlExportH
//---------------------------------------------------------------------------

#include <System.hpp>

namespace LuaExamplesHtmlExport
{
	/** \brief Export list of built-in Lua examples (name + source code) to a single, human-readable HTML file
		\return 0 on success, non-zero on failure
	*/
	int Write(AnsiString fileName);
}

#endif

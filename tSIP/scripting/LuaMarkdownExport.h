//---------------------------------------------------------------------------

#ifndef LuaMarkdownExportH
#define LuaMarkdownExportH
//---------------------------------------------------------------------------

#include <System.hpp>

namespace LuaMarkdownExport
{
	/** \brief Export custom Lua functions (name, brief, description) and built-in Lua examples
		(name + source code) to a single Markdown file (UTF-8)
		\return 0 on success, non-zero on failure
	*/
	int Write(AnsiString fileName);
}

#endif

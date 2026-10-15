//---------------------------------------------------------------------------

#pragma hdrstop

#include "LuaMarkdownExport.h"
#include "ScriptExec.h"
#include "LuaExamples.h"
#include "LuaHtmlExportCommon.h"
#include "Branding.h"
#include "common/Utilities.h"

#include <algorithm>
#include <fstream>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace LuaHtmlExportCommon;

int LuaMarkdownExport::Write(AnsiString fileName)
{
	std::vector<ScriptExec::Symbol> sorted = ScriptExec::GetSymbols();
	if (!sorted.empty())
	{
		// raw pointers, same as LuaFunctionsHtmlExport (STLport iterator warning)
		std::sort(&sorted[0], &sorted[0] + sorted.size());
	}
	unsigned int exampleCount = sizeof(luaExamples) / sizeof(luaExamples[0]);

	try
	{
		std::ofstream ofs(fileName.c_str(), std::ios::out | std::ios::binary);
		if (!ofs.is_open())
		{
			return 1;
		}

		// Markdown is written as UTF-8 without HTML escaping
		AnsiString title = AnsiToUtf8(Branding::appName + " " + GetAppVersion() + " - Lua reference");
		// already UTF-8 (and HTML-escaped, which leaves this line unchanged)
		AnsiString generatedBy = GetGeneratedByLine();

		ofs << "# " << title.c_str() << "\n\n";
		ofs << "_" << generatedBy.c_str() << "_\n\n";

		ofs << "## Contents\n\n";
		ofs << "- [Custom Lua functions](#custom-lua-functions)\n";
		ofs << "- [Examples](#examples)\n\n";

		ofs << "## Custom Lua functions\n\n";
		for (unsigned int i = 0; i < sorted.size(); i++)
		{
			const ScriptExec::Symbol &s = sorted[i];
			ofs << "### `" << AnsiToUtf8(s.name).c_str() << "`\n\n";
			if (s.brief != "")
			{
				ofs << AnsiToUtf8(s.brief).c_str() << "\n\n";
			}
			if (s.description != "")
			{
				ofs << "```\n" << AnsiToUtf8(s.description).c_str() << "\n```\n\n";
			}
		}

		ofs << "## Examples\n\n";
		ofs << "These examples are built into the " << AnsiToUtf8(Branding::appName).c_str()
			<< " executable. They are also available from the Script window's Help -> Examples menu.\n\n";
		for (unsigned int i = 0; i < exampleCount; i++)
		{
			ofs << "### " << AnsiToUtf8(luaExamples[i].name).c_str() << "\n\n";
			ofs << "```lua\n" << AnsiToUtf8(luaExamples[i].lua).c_str() << "\n```\n\n";
		}

		ofs.close();
	}
	catch(...)
	{
		return 1;
	}

	return 0;
}

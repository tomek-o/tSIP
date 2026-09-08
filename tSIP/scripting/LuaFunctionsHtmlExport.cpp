//---------------------------------------------------------------------------

#pragma hdrstop

#include "LuaFunctionsHtmlExport.h"
#include "ScriptExec.h"
#include "LuaHtmlExportCommon.h"
#include "Branding.h"

#include <algorithm>
#include <fstream>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace LuaHtmlExportCommon;

int LuaFunctionsHtmlExport::Write(AnsiString fileName)
{
	std::vector<ScriptExec::Symbol> sorted = ScriptExec::GetSymbols();
	if (!sorted.empty())
	{
		// using raw pointers rather than sorted.begin()/end() to avoid a W8091
		// (iterator category) warning from the classic compiler's STLport
		std::sort(&sorted[0], &sorted[0] + sorted.size());
	}

	try
	{
		std::ofstream ofs(fileName.c_str(), std::ios::out | std::ios::binary);
		if (!ofs.is_open())
		{
			return 1;
		}

		AnsiString title = PrepareText(Branding::appName + " - custom Lua functions");
		AnsiString generatedBy = GetGeneratedByLine();

		ofs <<
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<meta charset=\"utf-8\">\n"
			"<title>" << title.c_str() << "</title>\n"
			"<style>\n"
			"body { font-family: sans-serif; margin: 2em auto; max-width: 900px; }\n"
			"h2 { font-family: monospace; background: #eee; padding: 4px 8px; border-radius: 4px; }\n"
			"p.brief { font-style: italic; margin: 0.2em 0 0.6em 0; }\n"
			"p.meta { color: #666; font-size: 0.9em; margin-top: 0; }\n"
			"pre.description { white-space: pre-wrap; background: #f7f7f7; border: 1px solid #ddd; padding: 8px; border-radius: 4px; }\n"
			"nav { column-width: 16em; margin-bottom: 2em; }\n"
			"nav a { display: block; font-family: monospace; }\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<h1>" << title.c_str() << "</h1>\n"
			"<p class=\"meta\">" << generatedBy.c_str() << "</p>\n"
			"<nav>\n";

		for (unsigned int i = 0; i < sorted.size(); i++)
		{
			AnsiString name = PrepareText(sorted[i].name);
			ofs << "<a href=\"#" << name.c_str() << "\">" << name.c_str() << "</a>\n";
		}
		ofs << "</nav>\n";

		for (unsigned int i = 0; i < sorted.size(); i++)
		{
			const ScriptExec::Symbol &s = sorted[i];
			AnsiString name = PrepareText(s.name);
			ofs << "<h2 id=\"" << name.c_str() << "\">" << name.c_str() << "</h2>\n";
			if (s.brief != "")
			{
				ofs << "<p class=\"brief\">" << PrepareText(s.brief).c_str() << "</p>\n";
			}
			if (s.description != "")
			{
				ofs << "<pre class=\"description\">" << PrepareText(s.description).c_str() << "</pre>\n";
			}
		}

		ofs << "</body>\n</html>\n";
		ofs.close();
	}
	catch(...)
	{
		return 1;
	}

	return 0;
}

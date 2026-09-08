//---------------------------------------------------------------------------

#pragma hdrstop

#include "LuaExamplesHtmlExport.h"
#include "LuaExamples.h"
#include "LuaHtmlExportCommon.h"
#include "Branding.h"

#include <fstream>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace LuaHtmlExportCommon;

int LuaExamplesHtmlExport::Write(AnsiString fileName)
{
	unsigned int count = sizeof(luaExamples) / sizeof(luaExamples[0]);

	try
	{
		std::ofstream ofs(fileName.c_str(), std::ios::out | std::ios::binary);
		if (!ofs.is_open())
		{
			return 1;
		}

		AnsiString title = PrepareText(Branding::appName + " - built-in Lua examples");
		AnsiString generatedBy = GetGeneratedByLine();
		AnsiString note = PrepareText("These examples are built into the " + Branding::appName +
			" executable - they are also available from the Script window's Help -> Examples menu.");

		ofs <<
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<meta charset=\"utf-8\">\n"
			"<title>" << title.c_str() << "</title>\n"
			"<style>\n"
			"body { font-family: sans-serif; margin: 2em auto; max-width: 900px; }\n"
			"h2 { font-family: monospace; background: #eee; padding: 4px 8px; border-radius: 4px; }\n"
			"p.meta { color: #666; font-size: 0.9em; margin-top: 0; }\n"
			"p.note { background: #eef6ff; border: 1px solid #cfe3f7; padding: 8px; border-radius: 4px; }\n"
			"pre.code { white-space: pre-wrap; background: #f7f7f7; border: 1px solid #ddd; padding: 8px; border-radius: 4px; }\n"
			"nav { margin-bottom: 2em; }\n"
			"nav a { display: block; }\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<h1>" << title.c_str() << "</h1>\n"
			"<p class=\"meta\">" << generatedBy.c_str() << "</p>\n"
			"<p class=\"note\">" << note.c_str() << "</p>\n"
			"<nav>\n";

		for (unsigned int i = 0; i < count; i++)
		{
			AnsiString anchor;
			anchor.sprintf("example%u", i);
			ofs << "<a href=\"#" << anchor.c_str() << "\">" << PrepareText(luaExamples[i].name).c_str() << "</a>\n";
		}
		ofs << "</nav>\n";

		for (unsigned int i = 0; i < count; i++)
		{
			AnsiString anchor;
			anchor.sprintf("example%u", i);
			AnsiString name = PrepareText(luaExamples[i].name);
			ofs << "<h2 id=\"" << anchor.c_str() << "\">" << name.c_str() << "</h2>\n";
			ofs << "<pre class=\"code\">" << PrepareText(luaExamples[i].lua).c_str() << "</pre>\n";
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

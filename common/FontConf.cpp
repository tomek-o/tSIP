#include <vcl.h>
#pragma hdrstop

#include "FontConf.h"
#include <json/json.h>

FontConf::FontConf(void):
	name("Tahoma"),
	size(8),
	style(TFontStyles())
{
}

void FontConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;

	jv.getAString("name", name);
	jv.getIntInRange("size", size, MIN_SIZE, MAX_SIZE);
	style = TFontStyles();
	bool bold = jv.get("bold", false).asBool();
	if (bold)
	{
		style << fsBold;
	}
	bool italic = jv.get("italic", false).asBool();
	if (italic)
	{
		style << fsItalic;
	}
	bool underline = jv.get("underline", false).asBool();
	if (underline)
	{
		style << fsUnderline;
	}
}

void FontConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);

	jv["name"] = name;
	jv["size"] = size;
	jv["bold"] = style.Contains(fsBold);
	jv["italic"] = style.Contains(fsItalic);
	jv["underline"] = style.Contains(fsUnderline);
}

void FontConf::ShowOnEdit(Stdctrls::TEdit *ed)
{
	ed->Font->Name = name;
	ed->Font->Size = size;
	ed->Font->Style = style;
}


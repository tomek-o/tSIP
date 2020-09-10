//---------------------------------------------------------------------------


#pragma hdrstop

#include "ContactsXmlImport.h"
#include "Log.h"
#include <SysUtils.hpp>
#include <re_sxmlc.h>
#include <fstream> 

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace {

struct contactsContext {
	bool error;
	bool inDirectoryEntry;
	bool inName;
	bool inTelephone;

	Contacts::Entry entry;

	std::vector<Contacts::Entry> entries;
	contactsContext(void):
		error(false),
		inDirectoryEntry(false),
		inName(false),
		inTelephone(false)
	{}
};

/** \note return 0 to stop parsing */
static int contactsSxmlcCallback(XMLEvent evt, const XMLNode* node, SXML_CHAR* text, const int n, SAX_Data* sd)
{
	struct contactsContext* ctx = (struct contactsContext*)sd->user;

	switch(evt) {
		case XML_EVENT_START_NODE:
			if (!ctx->inDirectoryEntry) {
				if (!stricmp(node->tag, C2SX("DirectoryEntry"))) {
					ctx->inDirectoryEntry = true;
					Contacts::Entry defaultEntry;
					ctx->entry = defaultEntry;
				}
			} else {
				if (!stricmp(node->tag, C2SX("Name"))) {
					ctx->inName = true;
				} else if (!stricmp(node->tag, C2SX("Telephone"))) {
					ctx->inTelephone = true;
				}
			}
			break;

		case XML_EVENT_END_NODE:
			if (ctx->inDirectoryEntry) {
				if (!stricmp(node->tag, C2SX("DirectoryEntry"))) {
					ctx->entries.push_back(ctx->entry);
					ctx->inDirectoryEntry = false;
				} else if (!stricmp(node->tag, C2SX("Name"))) {
					ctx->inName = false;
				} else if (!stricmp(node->tag, C2SX("Telephone"))) {
					ctx->inTelephone = false;
				}
			}
			break;

		case XML_EVENT_TEXT:
			if (ctx->inName) {
				ctx->entry.description = ::Utf8ToAnsi(text);
			} else if (ctx->inTelephone) {
				if (ctx->entry.uri1 == "")
					ctx->entry.uri1 = ::Utf8ToAnsi(text);
				else if (ctx->entry.uri2 == "")
					ctx->entry.uri2 = ::Utf8ToAnsi(text);
				else if (ctx->entry.uri3 == "")
					ctx->entry.uri3 = ::Utf8ToAnsi(text);
			}
			break;

		case XML_EVENT_ERROR:
			ctx->error = true;
			return 0;	// stop parsing

		default:
			break;
	}

	return 1;	// continue parsing
}

}

int ContactsXmlImport(AnsiString filename, std::vector<Contacts::Entry> &entries)
{
	if (filename == "")
	{
		LOG("XML contacts import: not file name!\n");
		return -1;
	}
	if (!FileExists(filename))
	{
		LOG("XML contacts import: file %s does not exist!\n", filename.c_str());
		return -1;
	}

	try
	{
		std::ifstream ifs(filename.c_str());
		std::string strXml((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		if (strXml == "")
		{
            LOG("Error reading XML contacts file / XML file is empty (%s)\n", filename.c_str());
			return -3;
		}

		struct contactsContext ctx;
		SAX_Callbacks sax;
		SAX_Callbacks_init(&sax);
		sax.all_event = contactsSxmlcCallback;

		XMLDoc_parse_buffer_SAX_len(strXml.c_str(), strXml.size(), "root", &sax, &ctx);
		if (ctx.error)
		{
            LOG("Error parsing XML from contacts file (%s)\n", filename.c_str());
			return -2;
		}
		else
		{
            entries = ctx.entries;
			LOG("Found %u entries in XML contacts file\n", entries.size());
		}
		return 0;		
	}
	catch(...)
	{
        LOG("Error reading XML file (%s)\n", filename.c_str());
		return 1;
	}
}

//---------------------------------------------------------------------------
/** \file
	\brief Import of Cisco/Yealink/ATcom/...-style XML phonebook
*/

#ifndef ContactsXmlImportH
#define ContactsXmlImportH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <vector>
#include "Contacts.h"

int ContactsXmlImport(AnsiString filename, std::vector<Contacts::Entry> &entries);

#endif

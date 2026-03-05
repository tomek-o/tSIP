//---------------------------------------------------------------------------

#ifndef SettingsUtilsH
#define SettingsUtilsH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <string>

namespace SettingsUtils
{
	/** \brief Get the name of temporary file used for writing when updating */
	AnsiString GetNextFileName(AnsiString fileName);
	/** \brief Get the name of the backup file */
	AnsiString GetBakFileName(AnsiString fileName);
	/**	\brief Try to update file with new content in atomic way
		\return 0 on success
	*/
	int AtomicUpdateWithBackup(AnsiString fileName, const std::string &content);
}

#endif

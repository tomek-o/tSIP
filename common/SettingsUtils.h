//---------------------------------------------------------------------------

#ifndef SettingsUtilsH
#define SettingsUtilsH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <string>

namespace Json
{
	class Value;
}

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

	enum ReadStatus
	{
		READ_OK = 0,
		READ_IO_ERROR,
		READ_PARSE_ERROR,
		READ_FILE_NOT_FOUND,
		READ_EMPTY_FILE,
		READ_RECOVERED_FROM_BACKUP
	};

	enum ReadStatus ReadFileOrBackup(AnsiString fileName, Json::Value &root);
}

#endif

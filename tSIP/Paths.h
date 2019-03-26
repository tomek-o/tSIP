//---------------------------------------------------------------------------

#ifndef PathsH
#define PathsH
//---------------------------------------------------------------------------

#include <Classes.hpp>

namespace Paths
{
	void SetProfileDir(AnsiString dir);

	AnsiString GetProfileDir(void);

	AnsiString GetFullImgName(AnsiString name);

	AnsiString GetConfig(void);
};

#endif

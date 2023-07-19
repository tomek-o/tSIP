//---------------------------------------------------------------------------

#ifndef TelecomUtilsH
#define TelecomUtilsH
//---------------------------------------------------------------------------
#include <System.hpp>

AnsiString ExtractNumberFromUri(AnsiString uri);
AnsiString CleanNumber(AnsiString asNumber);
AnsiString GetClip(AnsiString uri, bool userOnly);
AnsiString CleanUri(AnsiString uri);

#endif

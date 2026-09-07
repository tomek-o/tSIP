//---------------------------------------------------------------------------

#ifndef TelecomUtilsH
#define TelecomUtilsH
//---------------------------------------------------------------------------
#include <System.hpp>

AnsiString ExtractNumberFromUri(AnsiString uri);
AnsiString CleanNumber(AnsiString asNumber);
AnsiString GetClip(AnsiString uri, bool userOnly);
AnsiString CleanUri(AnsiString uri);

/** \brief Self-test for ExtractNumberFromUri(); returns 0 if all cases pass,
 *  otherwise the 1-based index of the first failing case. */
int TestExtractNumberFromUri(void);

#endif

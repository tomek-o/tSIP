//---------------------------------------------------------------------------
#ifndef AppStatusH
#define AppStatusH
//---------------------------------------------------------------------------

#include <Classes.hpp>


/**	\brief Set application status
	Combining multiple texts into application status - text on status bar and
	hint for tray icon. Tray icon hint would contain sum of statuses, status bar
	text would contain status with highest priority (lowest priority number value),
	plugins can add their status text independently.
	\param id text identifying status
	\param priority number deciding about order of statuses; lower number goes before higher number
	\param text part of the status to be combined
*/
void SetAppStatus(AnsiString id, int priority, AnsiString text);

#endif

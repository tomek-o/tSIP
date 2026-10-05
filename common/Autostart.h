/** \file
	\brief Register/unregister a program to run at Windows login

	Uses the per-user HKEY_CURRENT_USER Run key - no elevation needed.
*/
#ifndef AutostartH
#define AutostartH

#include <System.hpp>

namespace Autostart
{
	/** \brief True only if `name` is registered with exactly `command`
		\note a stale entry (exe moved) reports false
	*/
	bool IsEnabled(const AnsiString &name, const AnsiString &command);

	/** \brief Add/overwrite the `name` entry to run `command` at login
		\return true on success
	*/
	bool Enable(const AnsiString &name, const AnsiString &command);

	/** \brief Remove the `name` entry, if present
		\return true on success (including when already absent)
	*/
	bool Disable(const AnsiString &name);
}

#endif

//---------------------------------------------------------------------------

#ifndef NetworkUtilsH
#define NetworkUtilsH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <vector>

/** \brief Use WinAPI GetAdaptersInfo() to get system IP addresses
	\return 0 on success
*/
int GetAdaptersInfoIp(std::vector<AnsiString> &ips);

#endif

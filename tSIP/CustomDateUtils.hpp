/** \file
	\brief Custom header to get rid of 8080 warnings from original VCL header
*/
#ifndef CustomDateUtilsHpp
#define CustomDateUtilsHpp

#include <DateUtils.hpp>

#define UNREFERENCED(x)  ((void)x)
static void FIX_DATEUTIL_WARNINGS()
{
	UNREFERENCED(NaN);
	UNREFERENCED(Infinity);
	UNREFERENCED(NegInfinity);
	UNREFERENCED(MinSingle);
	UNREFERENCED(MaxSingle);
	UNREFERENCED(MinDouble);
	UNREFERENCED(MaxDouble);
	UNREFERENCED(MinExtended);
	UNREFERENCED(MaxExtended);
	UNREFERENCED(MinComp);
	UNREFERENCED(MaxComp);
	UNREFERENCED(OneHour);
    UNREFERENCED(OneMinute);
    UNREFERENCED(OneSecond);
    UNREFERENCED(OneMillisecond);
}


#endif


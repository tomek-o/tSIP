//---------------------------------------------------------------------------

#ifndef CallsH
#define CallsH
//---------------------------------------------------------------------------

#include "Call.h"
#include <vector>

namespace Calls
{
	Call* Alloc(void);
	Call* FindByUid(int uid);
	void RemoveByUid(int uid);
	Call* GetFirstCall(void);
	Call* GetCurrentCall(void);
	Call* GetPreviousCall(void);
	void Clear(void);
	std::vector<int> GetUids(void);
};

#endif

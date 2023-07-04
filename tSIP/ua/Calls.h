//---------------------------------------------------------------------------

#ifndef CallsH
#define CallsH
//---------------------------------------------------------------------------

#include "Call.h"
#include <vector>

class TProgrammableButton;

namespace Calls
{
	Call* Alloc(void);
	Call* FindByUid(unsigned int uid);
	void RemoveByUid(unsigned int uid);
	Call* GetCurrentCall(void);
	Call& GetPreviousCall(void);
	void SetPreviousCall(const Call &call);
	void Clear(void);
	std::vector<unsigned int> GetUids(void);

	Recorder* FindRecorder(int recorderId);

	void OnLineButtonClick(int id, TProgrammableButton* btn);
	void OnButtonConfigChange(void);
};

#endif

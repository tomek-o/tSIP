//---------------------------------------------------------------------------

#ifndef CallsH
#define CallsH
//---------------------------------------------------------------------------

#include "Call.h"
#include <vector>
#include <map>

class TProgrammableButton;

namespace Calls
{
	Call* Alloc(void);
	Call* FindByUid(unsigned int uid);
	Call* FindByAutoAnswerTimer(Extctrls::TTimer *tmr);
	void RemoveByUid(unsigned int uid);
	Call* GetCurrentCall(void);
	unsigned int GetCurrentCallUid(void);
	int SetCurrentCallUid(unsigned int uid);
	Call& GetPreviousCall(void);
	void SetPreviousCall(const Call &call);
	void Clear(void);
	std::vector<unsigned int> GetUids(void);
	const std::map<unsigned int, Call> GetCalls(void);
	unsigned int Count(void);

	Recorder* FindRecorder(int recorderId);

	/** \return 0 on success
	*/
	int AssignLineButton(Call *call, bool outgoing, int &btnId);
	void OnLineButtonClick(int id, TProgrammableButton* btn);
	void OnButtonConfigChange(void);
};

#endif

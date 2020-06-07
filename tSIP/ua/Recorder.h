//---------------------------------------------------------------------------

#ifndef RecorderH
#define RecorderH
//---------------------------------------------------------------------------

#include "Callback.h"

struct Recorder
{
	int id;
	enum Callback::rec_state_e state;
	Recorder(void):
		id(-1),
		state(Callback::RECORDER_STATE_IDLE)
	{}
};

#endif

//---------------------------------------------------------------------------

#ifndef RecorderH
#define RecorderH
//---------------------------------------------------------------------------

#include "Callback.h"

struct Recorder
{
	enum Callback::rec_state_e state;
	Recorder(void):
		state(Callback::RECORDER_STATE_IDLE)
	{}
};

#endif

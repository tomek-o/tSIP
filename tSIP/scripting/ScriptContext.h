//---------------------------------------------------------------------------

#ifndef ScriptContextH
#define ScriptContextH
//---------------------------------------------------------------------------

#include "ScriptSource.h"
#include <System.hpp>

/** \brief Per-run context: describes why a script is being executed, plus
 *  any event-specific data associated with the trigger (e.g. an incoming
 *  SIP SIMPLE MESSAGE's sender/body). Extend with new fields as new event
 *  types need to expose more than a numeric id.
 */
struct ScriptContext
{
	enum ScriptSource srcType;
	int srcId;

	/** \name SCRIPT_SRC_ON_SIMPLE_MESSAGE_RX-specific fields */
	///@{
	AnsiString simpleMessageFrom;
	AnsiString simpleMessageBody;
	AnsiString simpleMessageContentType;
	///@}

	ScriptContext(enum ScriptSource srcType, int srcId);
};

#endif

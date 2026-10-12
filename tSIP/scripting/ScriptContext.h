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

	/** \brief If not NULL: execution error text is stored here instead of
		showing error dialog (e.g. script run by plugin - error is returned to plugin,
		modal dialog would block main thread)
	*/
	AnsiString *errorText;
	/** \brief If not NULL: print() output is also appended here (limited size) */
	AnsiString *outputText;
	/** \brief If not NULL: values returned by script (return ...) as text, tab-separated */
	AnsiString *returnText;

	ScriptContext(enum ScriptSource srcType, int srcId);
};

#endif

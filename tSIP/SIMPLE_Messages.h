//---------------------------------------------------------------------------

#ifndef SIMPLE_MessagesH
#define SIMPLE_MessagesH
//---------------------------------------------------------------------------

#include <System.hpp>
#include "scripting/ScriptContext.h"

/** \note Using just "Messages.h" for name creates some conflict (VCL?)
*/

class TfrmMessage;

namespace SIMPLE_Messages
{
	typedef void (__closure *CallbackRunScriptFile)(const ScriptContext &context, AnsiString filename, bool &handled, bool showLog);
	void SetCallbackRunScriptFile(CallbackRunScriptFile cb);

	void RegisterWindow(TfrmMessage *frmMessage);
	void UnregisterWindow(TfrmMessage *frmMessage);
	void CloseAllWindows(void);
	void OnIncomingMessage(AnsiString caller, AnsiString contentType, AnsiString body);
	void OnMessageStatus(int requestUid, int requestError, int sipCode, AnsiString reason);
	void Send(AnsiString target, AnsiString text, bool sendImmediately);
}

#endif

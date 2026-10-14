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
	/** \param accountId account that received message (-1: unknown) */
	void OnIncomingMessage(AnsiString caller, AnsiString contentType, AnsiString body, int accountId);
	void OnMessageStatus(int requestUid, int requestError, int sipCode, AnsiString reason);
	/** \param accountId account used for sending, -1: current account */
	void Send(AnsiString target, AnsiString text, bool sendImmediately, int accountId = -1);
}

#endif

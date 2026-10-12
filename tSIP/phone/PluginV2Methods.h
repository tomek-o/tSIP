/** \file
	\brief Plugin interface v2: Invoke() methods using application data directly
	(call history, contacts, call control) - kept separate from PhoneInterface and FormMain
*/
#ifndef PluginV2MethodsH
#define PluginV2MethodsH

#include <System.hpp>

class Call;

namespace Json
{
	class Value;
}

namespace PluginV2Methods
{
	/** \brief Execute Invoke() method - main thread only
		\return TSIP_OK, TSIP_ERR_* code, TSIP_ERR_UNKNOWN_METHOD if method is not handled here
	*/
	int Execute(const AnsiString &method, const Json::Value &params, Json::Value &result);

	/** \brief Resolve call uid from plugin request: 0 = current call
		\return false if there is no such call
	*/
	bool ResolveCallUid(unsigned int &callUid);

	/** \brief Start recording call using settings from configuration (implemented by application) */
	typedef void (__closure *CallbackStartRecording)(Call &call);
	void SetCallbackStartRecording(CallbackStartRecording cb);

	/** \brief Display name (as stored in call / history) converted to UTF-8 */
	AnsiString PeerNameToUtf8(const AnsiString &peerName);
}

#endif

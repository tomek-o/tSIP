//---------------------------------------------------------------------------
#ifndef UaPublishH
#define UaPublishH
//---------------------------------------------------------------------------
#include <string>
#include <vector>

/** \brief Presence published with SIP PUBLISH (RFC 3903), per account.
 *  Shared by the GUI thread (requests, refresh, withdraw) and the UA thread (replies); thread-safe.
 */
namespace UaPublish
{
	struct Presence
	{
		bool active;			///< publication wanted (false after Withdraw)
		bool open;				///< presence status: open (true) or closed
		std::string note;
		std::string etag;		///< SIP-ETag of the last successful PUBLISH, sent as SIP-If-Match
		unsigned int expires;	///< expiry granted by the server (or the requested one)
		unsigned long long lastSentMs;	///< 0 = not sent yet (or refresh due)
		Presence(void):
			active(false), open(true), expires(3600), lastSentMs(0)
		{}
	};

	/** publication requested: the next PUBLISH replaces the previous one */
	void Request(int accountId, bool open, const std::string &note);
	/** publication no longer wanted; the caller sends the unpublish (Expires: 0) */
	void Withdraw(int accountId);
	/** copies the state of an account; false if nothing was requested for it */
	bool Get(int accountId, Presence &out);
	/** a PUBLISH was queued at nowMs */
	void SentAt(int accountId, unsigned long long nowMs);
	/** accounts whose publication has to be refreshed (before the granted expiry) */
	std::vector<int> Due(unsigned long long nowMs);
	/** reply to a PUBLISH: scode 0 = no reply (error) */
	void Reply(int accountId, int scode, const std::string &etag, unsigned int expires);
}

#endif

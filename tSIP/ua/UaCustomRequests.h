//---------------------------------------------------------------------------

#ifndef UaCustomRequestsH
#define UaCustomRequestsH
//---------------------------------------------------------------------------

#include <System.hpp>


namespace UaCustomRequests
{

struct Request
{
	int uid;
	AnsiString method;
	AnsiString uri;
	AnsiString extraHeaderLines;

	// reply part
	bool haveReply;
	int error;
	int sipStatusCode;
	AnsiString replyText;

	Request(void):
		uid(-1),
		haveReply(false),
		error(0),
		sipStatusCode(0)
	{}
};

void Clear(void);

/**
	\param uid returned pseudo-unique id of created request
	\return 0 on success
*/
int Send(int &uid, AnsiString uri, AnsiString method, AnsiString extraHeaderLines);

/** \brief Send a SIP request of an arbitrary method within an existing call's dialog
	(unlike Send(), which always starts a new, separate SIP transaction/dialog to a
	given URI)
	\param uid returned pseudo-unique id of created request
	\return 0 on success
*/
int SendCustomCallRequest(int &uid, unsigned int callUid, AnsiString method, AnsiString extraHeaderLines);

/** \return 0 if request uid is found (it may be deleted before receving reply)
*/
int NotifyReply(int uid, int err, int sipStatusCode, AnsiString replyText);

void DeleteRequest(int uid);

/** \return pointer to request structure or NULL
*/
struct Request* GetRequest(int uid);

}	// namespace UaCustomRequests

#endif

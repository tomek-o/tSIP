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

void NotifyReply(int uid, int err, int sipStatusCode);

void DeleteRequest(int uid);

}	// namespace UaCustomRequests

#endif

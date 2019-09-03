//---------------------------------------------------------------------------


#pragma hdrstop

#include "UaCustomRequests.h"
#include "UaMain.h"
#include "ControlQueue.h"
#include "Log.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#include <map>

namespace
{

std::map<int, UaCustomRequests::Request> requests;
static int uid = 0;

int AllocUid(void)
{
	bool colliding = false;
	do
	{
		uid++;
		if (uid <= 0)
			uid = 1;
		if (requests.find(uid) != requests.end())
			colliding = true;
	} while (colliding);
	return uid;
}

}	// namespace

void UaCustomRequests::Clear(void)
{
	requests.clear();
}

void UaCustomRequests::DeleteRequest(int uid)
{
	requests.erase(uid);
}

struct UaCustomRequests::Request* UaCustomRequests::GetRequest(int uid)
{
	std::map<int, UaCustomRequests::Request>::iterator iter;
	iter = requests.find(uid);
	if (iter != requests.end())
	{
		return &iter->second;
	}
	return NULL;
}


int UaCustomRequests::Send(int &uid, AnsiString uri, AnsiString method, AnsiString extraHeaderLines)
{
	int status;

	int requestUid = AllocUid();

	Request req;
	req.uid = requestUid;
	req.method = method;
	req.uri = uri;
	req.extraHeaderLines = extraHeaderLines;

	status = UA->SendCustomRequest(requestUid, method, uri, extraHeaderLines);

	if (status == 0)
	{
		requests[req.uid] = req;
		uid = req.uid;		
	}

	return status;
}

int UaCustomRequests::NotifyReply(int uid, int err, int sipStatusCode, AnsiString replyText)
{
	LOG("UaCustomRequests: NotifyReply for %d: err = %d, SIP status code = %d\n", uid, err, sipStatusCode);
	std::map<int, UaCustomRequests::Request>::iterator iter;
	iter = requests.find(uid);
	if (iter != requests.end())
	{
		Request &req = iter->second;
		req.haveReply = true;
		req.error = err;
		req.sipStatusCode = sipStatusCode;
		req.replyText = replyText;
		return 0;
	}
	else
	{
		LOG("UaCustomRequests: request uid %d not found\n", uid);
		return -1;
	}
}




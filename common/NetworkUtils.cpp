//---------------------------------------------------------------------------


#pragma hdrstop

#include "NetworkUtils.h"
#include "StaticCheck.h"
#include <iphlpapi.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

int GetAdaptersInfoIp(std::vector<AnsiString> &ips)
{
	IP_ADAPTER_INFO info[32];
	ULONG ulOutBufLen = sizeof(info);
	DWORD ret;

	ips.clear();

	ret = GetAdaptersInfo(info, &ulOutBufLen);
	if (ret != ERROR_SUCCESS) {
		STATIC_CHECK(ERROR_SUCCESS == 0, UnexpectedStatusVal);
		return ret;
	}

	for (PIP_ADAPTER_INFO p = info; p; p = p->Next) {
		IP_ADDR_STRING *IpAddress = &p->IpAddressList;
		while (IpAddress)
		{
			ips.push_back(IpAddress->IpAddress.String);
			IpAddress = IpAddress->Next;
		}
	}
	return 0;
}

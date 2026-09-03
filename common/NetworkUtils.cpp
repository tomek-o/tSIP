//---------------------------------------------------------------------------


#pragma hdrstop

#include "NetworkUtils.h"
#include "StaticCheck.h"
#include <iphlpapi.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

int GetAdaptersInfoIp(std::vector<AnsiString> &ips)
{
	enum { ASSUMED_ADAPTER_COUNT = 16 };
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO) * ASSUMED_ADAPTER_COUNT;
	DWORD ret;

	ips.clear();

	std::vector<char> buf(ulOutBufLen);
	ret = GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO*>(&buf[0]), &ulOutBufLen);
	if (ret == ERROR_BUFFER_OVERFLOW)
	{
        // not enough space -> retry
		buf.resize(ulOutBufLen);
		ret = GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO*>(&buf[0]), &ulOutBufLen);
	}
	if (ret != ERROR_SUCCESS) {
		STATIC_CHECK(ERROR_SUCCESS == 0, UnexpectedStatusVal);
		return ret;
	}

	for (PIP_ADAPTER_INFO p = reinterpret_cast<IP_ADAPTER_INFO*>(&buf[0]); p; p = p->Next) {
		IP_ADDR_STRING *IpAddress = &p->IpAddressList;
		while (IpAddress)
		{
			ips.push_back(IpAddress->IpAddress.String);
			IpAddress = IpAddress->Next;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------


#pragma hdrstop

#include "NetInterfaces.h"
#include <re_net.h>
#include <re_sa.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{

bool if_handler(const char *ifname, const struct sa *sa, void *arg)
{
	std::vector<NetInterface> *ifaces = reinterpret_cast<std::vector<NetInterface> *>(arg);
	struct NetInterface ni;
	ni.name = ifname;
	char buf[128];
	int ret = net_inet_ntop(sa, buf, sizeof(buf));
	if (ret == 0)
	{
		ni.ip = buf;
	}
	ifaces->push_back(ni);
	return false;
}

}

int GetNetInterfaces(std::vector<NetInterface> &interfaces)
{
    interfaces.clear();
	int err = net_if_list(if_handler, &interfaces);
	return err;
}

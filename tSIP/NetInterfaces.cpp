//---------------------------------------------------------------------------


#pragma hdrstop

#include "NetInterfaces.h"
#include "Registry.hpp"
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

AnsiString GetNetAdapterDriverName(AnsiString adapterGuid)
{
	TRegistry *Registry = new TRegistry(KEY_READ);
	TStringList *sList = new TStringList;
	try {
		Registry->RootKey = HKEY_LOCAL_MACHINE;
		// False because we do not want to create it if it doesn’t exist
		AnsiString key = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}";
		if (Registry->OpenKey(key, false)) {
			Registry->GetKeyNames(sList);
			Registry->CloseKey();
			for (unsigned int i=0; i<sList->Count; i++) {
				AnsiString adapterKey = key + "\\" + sList->Strings[i];
				if (Registry->OpenKey(adapterKey, false)) {
					if (adapterGuid == Registry->ReadString("NetCfgInstanceId")) {
						return Registry->ReadString("DriverDesc");
					}
					Registry->CloseKey();
				}
			}
		}
	}
	__finally {
		delete Registry;
		delete sList;
	}
	return "";
}

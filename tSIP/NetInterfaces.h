//---------------------------------------------------------------------------

#ifndef NetInterfacesH
#define NetInterfacesH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <vector>

struct NetInterface
{
	AnsiString name;
	AnsiString ip;
};

int GetNetInterfaces(std::vector<NetInterface> &interfaces);
AnsiString GetNetAdapterDriverName(AnsiString adapterGuid);

#endif

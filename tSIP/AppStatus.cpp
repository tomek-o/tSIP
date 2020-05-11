//---------------------------------------------------------------------------


#pragma hdrstop

#include "AppStatus.h"
#include "FormMain.h"
#include <map>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{

struct OrderedStatusKey
{
	int priority;
	AnsiString id;
	bool operator<(const OrderedStatusKey &other) const
	{
		if (priority < other.priority)
			return true;
		if (id < other.id)
			return true;
		return false;
	}
};

std::map<OrderedStatusKey, AnsiString> orderedStatuses;

void UpdateStatus(void)
{
	std::map<OrderedStatusKey, AnsiString>::iterator iter;
	if (orderedStatuses.empty())
	{
		if (frmMain->StatusBar)
		{
			frmMain->StatusBar->SimpleText = "";
		}
		frmMain->SetTrayIconHint("");
		return;
	}
	AnsiString trayHint;
	if (frmMain->StatusBar)
	{
		frmMain->StatusBar->SimpleText = orderedStatuses.begin()->second;
	}
	iter = orderedStatuses.begin();
	trayHint = iter->second;
	++iter;
	for (; iter != orderedStatuses.end(); ++iter)
	{
    	trayHint.cat_printf("\r\n%s", iter->second.c_str());
	}
	frmMain->SetTrayIconHint(trayHint);
}

}	// namespace (anonymous)


void SetAppStatus(AnsiString id, int priority, AnsiString text)
{
	std::map<OrderedStatusKey, AnsiString>::iterator iter;
	for (iter = orderedStatuses.begin(); iter != orderedStatuses.end(); ++iter)
	{
		if (iter->first.id == id)
		{
			orderedStatuses.erase(iter);
			break;
		}
	}

	if (text != "")
	{
		OrderedStatusKey key;
		key.id = id;
		key.priority = priority;
		orderedStatuses[key] = text;
	}

	UpdateStatus();
}


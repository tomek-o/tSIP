//---------------------------------------------------------------------------


#pragma hdrstop

#include "TelecomUtils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString ExtractNumberFromUri(AnsiString uri)
{
	AnsiString res = "";
	int start = uri.Pos("sip:");
	if (start == 1)
		start += 4;
	int end = uri.Pos("@");
	if (end <= start)
		return "";
	res = uri.SubString(start, end-start);
	return res;
}

AnsiString CleanNumber(AnsiString asNumber)
{
	AnsiString nr = "";
	for(int i=1; i<=asNumber.Length(); i++)
	{
		if((asNumber[i] >= '0' && asNumber[i] <= '9') || asNumber[i]=='*' ||
			asNumber[i]=='#' || asNumber[i]=='+')
		{
			nr += asNumber[i];
		}
	}
	return nr;
}

AnsiString GetClip(AnsiString uri, bool userOnly)
{
	AnsiString res;
	if (userOnly)
	{
		res = ExtractNumberFromUri(uri);
		if (res != "")
		{
			return res;
		}
		else
		{
			return CleanUri(uri);
		}
	}
	else
	{
		return CleanUri(uri);
	}
}

AnsiString CleanUri(AnsiString uri)
{
	AnsiString res = uri;
	int start = uri.Pos("<");
	if (start == 0)
		start = 1;
	int end = uri.Pos(">");
	if (end == 0 || (uri.Pos(";") < end))
		end = uri.Pos(";");
	if (start && end && end > start)
	{
		res = uri.SubString(start, end-start);
	}
	return res;
}

//---------------------------------------------------------------------------


#pragma hdrstop

#include "TelecomUtils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString ExtractNumberFromUri(AnsiString uri)
{
	int start = uri.Pos("sips:");
	if (start == 1)
	{
		start += 5;
	}
	else
	{
		start = uri.Pos("sip:");
		if (start == 1)
			start += 4;
		else if (start == 0)
			start = 1;  // no "sip:"/"sips:" prefix - extract from the beginning of the string
	}

	int end = uri.Pos("@");
	if (end == 0)
	{
		// no '@' - there's no host part to strip, so the (scheme-stripped)
		// input is already just the number/user part; e.g. "300" -> "300"
		return uri.SubString(start, uri.Length() - start + 1);
	}
	if (end <= start)
		return "";
	return uri.SubString(start, end-start);
}

/** \brief Self-test for ExtractNumberFromUri() against various URI/number formats.
 *  \return 0 if all cases pass, otherwise the 1-based index of the first failing case.
 */
int TestExtractNumberFromUri(void)
{
	struct TestCase
	{
		const char *input;
		const char *expected;
	};
	static const TestCase cases[] = {
		{ "sip:300@myserver",             "300"      },
		{ "sip:300@myserver:5060",        "300"      },
		{ "sip:300@myserver;user=phone",  "300"      },
		{ "300@myserver",                 "300"      },   // no "sip:" prefix - was buggy (returned "300@")
		{ "300",                          "300"      },   // plain number, no '@' - already just the number
		{ "",                             ""         },   // empty input
		{ "myserver",                     "myserver" },   // no '@' - returned as-is, same rule as "300"
		{ "@myserver",                    ""         },   // empty user part
		{ "sip:@myserver",                ""         },   // empty user part, with scheme
		{ "sip:300",                      "300"      },   // scheme + user, no '@'/host - scheme stripped
		{ "sips:300@myserver",            "300"      },   // "sips:" scheme
		{ "sips:300",                     "300"      },   // "sips:" scheme, no '@'/host
		{ "sips:@myserver",               ""         },   // empty user part, with "sips:" scheme
	};

	for (unsigned int i = 0; i < sizeof(cases)/sizeof(cases[0]); i++)
	{
		AnsiString result = ExtractNumberFromUri(cases[i].input);
		if (result != AnsiString(cases[i].expected))
			return (int)(i + 1);
	}
	return 0;
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
	if (start > 0)
		start++;  // skip the '<' itself
	else
		start = 1;
	int end = uri.Pos(">");
	int semi = uri.Pos(";");
	if (end == 0 || (semi > 0 && semi < end))
		end = semi;
	if (end > start)
	{
		res = uri.SubString(start, end - start);
	}
	return res;
}

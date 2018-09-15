//---------------------------------------------------------------------------


#pragma hdrstop

#include "Utils.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//i.e. GetFileVer(Application->ExeName)
AnsiString GetFileVer(AnsiString FileName)
{
    AnsiString asVer="";
    VS_FIXEDFILEINFO *pVsInfo;
    unsigned int iFileInfoSize = sizeof( VS_FIXEDFILEINFO );
    
    int iVerInfoSize = GetFileVersionInfoSize(FileName.c_str(), NULL);
    if (iVerInfoSize!= 0)
    {
        char *pBuf = new char[iVerInfoSize];
        if (GetFileVersionInfo(FileName.c_str(),0, iVerInfoSize, pBuf ) )
        {
            if (VerQueryValue(pBuf, "\\",(void **)&pVsInfo,&iFileInfoSize))
            {
				asVer.sprintf("%d.%02d.%02d.%02d",
					HIWORD(pVsInfo->dwFileVersionMS), LOWORD(pVsInfo->dwFileVersionMS),
					HIWORD(pVsInfo->dwFileVersionLS), LOWORD(pVsInfo->dwFileVersionLS));
			}
        }
        delete [] pBuf;
    }
    return asVer;
}

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

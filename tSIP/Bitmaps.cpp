//---------------------------------------------------------------------------


#pragma hdrstop

#include "Bitmaps.h"
#include "Settings.h"
#include "Paths.h"
#include "FormMain.h"
#include "Log.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{

Graphics::TBitmap *bmpBackspace = NULL;
AnsiString asLastBackspaceFile;

Graphics::TBitmap *bmpResetMicVolume = NULL;
AnsiString asLastResetMicVolumeFile;

Graphics::TBitmap *bmpResetSpeakerVolume = NULL;
AnsiString asLastResetSpeakerVolumeFile;

Graphics::TBitmap *bmpConsoleHide = NULL;
AnsiString asLastConsoleHideFile;

Graphics::TBitmap *bmpConsoleShow = NULL;
AnsiString asLastConsoleShowFile;


/** \brief Update glyph keeping original bitmap as "backup"/default */
void UpdateBmp(Graphics::TBitmap *bmp, AnsiString asFile, Graphics::TBitmap *& originalBmp, AnsiString &asLastFile)
{
	assert(bmp);
	if (asFile == "")
	{
		if (originalBmp)
		{
			bmp->Assign(originalBmp);
		}
		asLastFile = asFile;
	}
	else
	{
		try
		{
			if (asFile != asLastFile)
			{
				if (originalBmp == NULL)
				{
					originalBmp = new Graphics::TBitmap();
					originalBmp->Assign(bmp);
					originalBmp->Dormant();
					originalBmp->FreeImage();
				}
				asLastFile = asFile;
				bmp->PixelFormat = pf24bit;
				bmp->LoadFromFile(Paths::GetFullImgName(asFile));
			}
		}
		catch (...)
		{
			LOG("Failed to load %s\n", asFile.c_str());
			if (originalBmp)
			{
				bmp->Assign(originalBmp);
			}
		}
	}
}

void LoadBitmap(Graphics::TBitmap *& bmp, AnsiString name, AnsiString &lastName)
{
	if (name == lastName)
		return;
	lastName = name;
	
	if (name == "")
	{
		if (bmp)
		{
			delete bmp;
			bmp = NULL;
		}
		return;
	}
	
	try
	{
		AnsiString fname = Paths::GetFullImgName(name);
		if (FileExists(fname))
		{
			if (bmp == NULL)
			{
				bmp = new Graphics::TBitmap();
			}
			bmp->LoadFromFile(fname);
		}
	} catch(...) {
		if (bmp)
		{
			delete bmp;
			bmp = NULL;
		}
	}
}


}	// namespace

void UpdateBitmaps(void)
{
	const Settings::_frmMain::_bitmaps &bitmaps = appSettings.frmMain.bitmaps;
	UpdateBmp(frmMain->btnBackspace->Glyph, bitmaps.bmpBtnBackspace, bmpBackspace, asLastBackspaceFile);
	UpdateBmp(frmMain->btnResetMicVolume->Glyph, bitmaps.bmpBtnResetMicVolume, bmpResetMicVolume, asLastResetMicVolumeFile);
	UpdateBmp(frmMain->btnResetSpeakerVolume->Glyph, bitmaps.bmpBtnResetSpeakerVolume, bmpResetSpeakerVolume, asLastResetSpeakerVolumeFile);
	LoadBitmap(bmpConsoleHide, bitmaps.bmpConsoleHide, asLastConsoleHideFile);
	LoadBitmap(bmpConsoleShow, bitmaps.bmpConsoleShow, asLastConsoleShowFile);
	UpdateBtnConsole();
}

void UpdateBtnConsole(void)
{
	TSpeedButton *btn = frmMain->btnSpeedDialPanel;
	if (appSettings.frmMain.bSpeedDialVisible)
	{
		if (bmpConsoleHide)
		{
			btn->Caption = "";
			btn->Glyph = bmpConsoleHide;
		}
		else
		{
			btn->Glyph = NULL;
			btn->Caption = "<<";
		}
	}
	else
	{
		if (bmpConsoleShow)
		{
			btn->Caption = "";
			btn->Glyph = bmpConsoleShow;
		}
		else
		{
			btn->Glyph = NULL;
			btn->Caption = ">>";
		}
	}
}

void FreeBitmapResources(void)
{
	if (bmpBackspace)
	{
		delete bmpBackspace;
		bmpBackspace = NULL;
	}
	if (bmpResetMicVolume)
	{
		delete bmpResetMicVolume;
		bmpResetMicVolume = NULL;
	}
	if (bmpResetSpeakerVolume)
	{
		delete bmpResetSpeakerVolume;
		bmpResetSpeakerVolume = NULL;
	}
	if (bmpConsoleHide)
	{
		delete bmpConsoleHide;
		bmpConsoleHide = NULL;
	}
	if (bmpConsoleShow)
	{
		delete bmpConsoleShow;
		bmpConsoleShow = NULL;
	}
}

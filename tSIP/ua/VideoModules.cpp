//---------------------------------------------------------------------------


#pragma hdrstop

#include "VideoModules.h"

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

using namespace VideoModules;

const char* VideoModules::dshow = "dshow";
const char* VideoModules::sdl = "sdl";

struct VideoModuleDef
{
	bool input;				///< handles video input?
	bool output;        	///< handles video output?
	const char* name;       ///< baresip name
	const char* description;///< UI name
};

const VideoModuleDef videoModuleDefs [] =
{
	{ true, false, dshow, "DirectShow" },
	{ false, true, sdl, "SDL" },
};
	

void VideoModules::FillInputSelectorCb(Stdctrls::TComboBox *cb)
{
	cb->Items->Clear();
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.input)
		{
			cb->Items->Add(mod.description);
		}
	}
}

void VideoModules::FillOutputSelectorCb(Stdctrls::TComboBox *cb)
{
	cb->Items->Clear();
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.output)
		{
			cb->Items->Add(mod.description);
		}
	}
}

const char* VideoModules::GetInputModuleFromCbIndex(int id)
{
	int idCounter = -1;
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.input)
		{
			idCounter++;
			if (idCounter == id)
			{
				return mod.name;
			}
		}
	}
	assert(!"Unhandled module index!");
	return dshow;
}

int VideoModules::GetInputModuleCbIndex(const std::string& name)
{
	int idCounter = -1;
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.input)
		{
			idCounter++;
			if (name == mod.name)
			{
				return idCounter;
			}
		}
	}
	assert(!"Unhandled module name!");
	return 0;
}


const char* VideoModules::GetOutputModuleFromCbIndex(int id)
{
	int idCounter = -1;
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.output)
		{
			idCounter++;
			if (idCounter == id)
			{
				return mod.name;
			}
		}
	}
	assert(!"Unhandled module index!");
	return dshow;
}

int VideoModules::GetOutputModuleCbIndex(const std::string& name)
{
	int idCounter = -1;
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.output)
		{
			idCounter++;
			if (name == mod.name)
			{
				return idCounter;
			}
		}
	}
	assert(!"Unhandled module name!");
	return 0;
}

bool VideoModules::IsInput(const std::string& name)
{
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.input)
		{
			if (name == mod.name)
			{
				return true;
			}
		}
	}
	return false;
}

bool VideoModules::IsOutput(const std::string& name)
{
	for (unsigned int i=0; i<ARRAY_SIZE(videoModuleDefs); i++)
	{
		const VideoModuleDef &mod = videoModuleDefs[i];
		if (mod.output)
		{
			if (name == mod.name)
			{
				return true;
			}
		}
	}
	return false;
}


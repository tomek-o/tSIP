//---------------------------------------------------------------------------
#pragma hdrstop

#include "AudioDevicesList.h"
#include "AudioModules.h"
#include <portaudio.h>
#include <mmsystem.h>

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

AudioDevicesList::AudioDevicesList(void)
{

}

void AudioDevicesList::Refresh(void)
{
	portaudioDevsIn.clear();
	portaudioDevsOut.clear();

	PaError error;
	error = Pa_Initialize();
	if (paNoError == error)
	{

		int n = Pa_GetDeviceCount();

		for (int i=0; i<n; i++)
		{
			const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
			if (info->maxInputChannels > 0)
			{
				portaudioDevsIn.push_back(info->name);
			}
			if (info->maxOutputChannels > 0)
			{
				portaudioDevsOut.push_back(info->name);
			}
		}

		Pa_Terminate();	// paired with Pa_Initialize()		
	}
	else
	{
		//DEBUG_WARNING("PortAudio init: %s\n", Pa_GetErrorText(error));
	}

	winwaveDevsIn.clear();
	winwaveDevsOut.clear();

	WAVEOUTCAPS woc;
	int nOutDevices = waveOutGetNumDevs();
	//if (nOutDevices > 0)
	//	winwaveDevsOut.push_back("WAVE mapper");
	for (int i=0; i<nOutDevices; i++)
	{
		if (waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS))==MMSYSERR_NOERROR)
		{
			winwaveDevsOut.push_back(woc.szPname);
		}
	}

	WAVEINCAPS wic;
	int nInDevices = waveInGetNumDevs();
	//if (nInDevices > 0)
	//	winwaveDevsIn.push_back("WAVE mapper");
	for (int i=0; i<nInDevices; i++)
	{
		if (waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS))==MMSYSERR_NOERROR)
		{
			winwaveDevsIn.push_back(wic.szPname);
		}
	}
}

void AudioDevicesList::FillComboBox(Stdctrls::TComboBox *target, AnsiString module, bool out, AnsiString selected)
{
    target->Tag = 0;
	target->Items->Clear();
	std::vector<AnsiString> *v = NULL;
	if (module == AudioModules::portaudio)
	{
		if (out)
			v = &AudioDevicesList::Instance().portaudioDevsOut;
		else
			v = &AudioDevicesList::Instance().portaudioDevsIn;
	}
	else if (module == AudioModules::winwave || module == AudioModules::winwave2)
	{
		if (out)
			v = &AudioDevicesList::Instance().winwaveDevsOut;
		else
			v = &AudioDevicesList::Instance().winwaveDevsIn;
	}
	else
	{
		return;
    }
	assert(v);
	for (int i=0; i<v->size(); i++)
	{
		AnsiString dev = v->at(i);
		target->Items->Add(dev);
		if (dev == selected)
		{
			target->ItemIndex = i;
		}
	}
	// convention: if selected device is not found - add it at last item with [NOT FOUND] text and set non-zero Tag
	if (target->ItemIndex < 0 && target->Items->Count > 0 && selected != "")
	{
		AnsiString text;
		text.sprintf("[NOT FOUND] %s", selected.c_str());
		target->Items->Add(text);
		target->ItemIndex = target->Items->Count - 1;
		target->Tag = 1;
	}
}

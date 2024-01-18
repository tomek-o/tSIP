//---------------------------------------------------------------------------

#ifndef AudioDevicesListH
#define AudioDevicesListH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <vector>

namespace Stdctrls
{
	class TComboBox;
}

class AudioDevicesList
{
private:
	AudioDevicesList();
	AudioDevicesList(const AudioDevicesList& source);
	AudioDevicesList& operator=(const AudioDevicesList&);
	bool filled;	///< set by Refresh();
public:
	static AudioDevicesList& Instance(void)
	{
		static AudioDevicesList audioDevicesList;
		return audioDevicesList;
	}
	std::vector<AnsiString> portaudioDevsIn;
	std::vector<AnsiString> portaudioDevsOut;
	std::vector<AnsiString> winwaveDevsIn;
	std::vector<AnsiString> winwaveDevsOut;
	/** \brief Fill/initialize device lists */
	void Refresh(void);
	/** \brief Check if Refresh() was called at least once */
	bool IsFilled(void) const
	{
    	return filled;
	}

	static void FillComboBox(Stdctrls::TComboBox *target, AnsiString module, bool out, AnsiString selected);
};

#endif


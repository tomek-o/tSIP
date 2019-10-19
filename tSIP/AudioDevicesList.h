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
	AudioDevicesList(const AudioDevicesList& source) {};
	AudioDevicesList& operator=(const AudioDevicesList&);
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
	void Refresh(void);

	static void FillComboBox(Stdctrls::TComboBox *target, AnsiString module, bool out, AnsiString selected);
};

#endif


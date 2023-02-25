//---------------------------------------------------------------------------

#ifndef VideoDevicesListH
#define VideoDevicesListH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <vector>

namespace Stdctrls
{
	class TComboBox;
}

class VideoDevicesList
{
private:
	VideoDevicesList();
	VideoDevicesList(const VideoDevicesList& source) {};
	VideoDevicesList& operator=(const VideoDevicesList&);
public:
	static VideoDevicesList& Instance(void)
	{
		static VideoDevicesList videoDevicesList;
		return videoDevicesList;
	}
	std::vector<AnsiString> dshowDevsIn;
	//std::vector<AnsiString> sdlDevsOut;
	void Refresh(void);

	static void FillComboBox(Stdctrls::TComboBox *target, AnsiString module, bool out, AnsiString selected);
};

#endif


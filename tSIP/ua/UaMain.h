//---------------------------------------------------------------------------

#ifndef UaMainH
#define UaMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <vector>

class Ua
{
private:
	Ua():
		thread(NULL)
	{
	};
	Ua(const Ua& source);
	Ua& operator=(const Ua&);
	TThread *thread;
public:
	static Ua& Instance(void)
	{
		static Ua ua;
		return ua;
	}
	int Start(void);
	void Restart(void);
	void Quit(void);
	int GetAudioCodecList(std::vector<AnsiString> &codecs);
	// RX peak of the current call, 0..32767 (0 if no call)
	unsigned int GetAudioRxSignalLevel(void);
	// audio levels (0..32768) of call; false if call has no audio
	bool GetCallAudioLevels(unsigned int callUid, int &rxPeak, float &rxRms, int &txPeak, float &txRms);

	int GetVideoCodecList(std::vector<AnsiString> &codecs);
};

/** Parent handle of the selfview window, set before a call starts (see baresip video_set_selfview_parent_handle) */
void UaSetSelfviewParentHandle(void *handle);

#endif

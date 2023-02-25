//---------------------------------------------------------------------------

#ifndef UaMainH
#define UaMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <vector>

class Ua
{
private:
	Ua() {
	};
	Ua(const Ua& source) {};
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
	unsigned int GetAudioRxSignalLevel(void);

	int GetVideoCodecList(std::vector<AnsiString> &codecs);
};

#endif

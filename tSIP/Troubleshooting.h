//---------------------------------------------------------------------------

#ifndef TroubleshootingH
#define TroubleshootingH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <vector>

namespace Troubleshooting {

	enum Level
	{
		LevelError = 0,
		LevelWarning,
		LevelHint,

		LevelLimiter
	};

	enum ItemType
	{
		NoAudioInputDevice = 0,
		NoAudioOutputDevice,
		WavFileMissing,
		IncorrectWavFormat,
		VirtualboxNetworkAdapter,
		InvalidFrameLength,
		UncommonFrameLength,
		PopularCodecsDisabled,
		NoRegistration,
		NoLocalPortAssigned,
		SpeexPreprocessingEnabled,
		NoCodecsEnabled,
		TooManyCodecsEnabled,

		ItemTypeLimiter
	};

	struct Item
	{
		enum ItemType type;
		AnsiString extraMsg;
		Item(void):
			type(ItemTypeLimiter)
		{}
		enum Level getLevel(void) const;
		const char* getName(void) const;
		const char* getDescription(void) const;
	};

	void AddItem(ItemType type, AnsiString extraMsg = "");

	const std::vector<Item>& getItems(void);

	void Update(void);
};


#endif

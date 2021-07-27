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

	struct Item
	{
		int typeId;
		AnsiString extraMsg;
		Item(void):
			typeId(-1)
		{}
		enum Level getLevel(void) const;
		const char* getName(void) const;
		const char* getDescription(void) const;
	};

	void AddItem(int typeId, AnsiString extraMsg = "");

	const std::vector<Item>& getItems(void);

	void Update(void);
};


#endif

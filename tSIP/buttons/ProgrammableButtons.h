//---------------------------------------------------------------------------

#ifndef ProgrammableButtonsH
#define ProgrammableButtonsH
//---------------------------------------------------------------------------

#include "ButtonConf.h"
#include "UaConf.h"
#include <vector>
#include <Classes.hpp>

namespace Json
{
	class Value;
}

class ProgrammableButtons
{
private:
	AnsiString filename;
	bool saveAllSettings;
	bool updated;
	int ReadFile(AnsiString name);
	int LoadFromJsonValue(const Json::Value &root);
public:
	static void SetDefaultsForBtnId(int id, ButtonConf& cfg);

	ProgrammableButtons(void);

	void SetFilename(AnsiString name)
	{
		filename = name;
	}
	int Read(void);
	int ReadFromString(AnsiString json);
	int Write(void);

	enum { EXT_CONSOLE_COLUMNS = 1 };
	enum { BASIC_PANEL_CONSOLE_BTNS = 15 };
	enum { CONSOLE_BTNS_PER_CONTAINER = 180 };

	static int GetTotalCnt(void)
	{
		return BASIC_PANEL_CONSOLE_BTNS + (EXT_CONSOLE_COLUMNS * CONSOLE_BTNS_PER_CONTAINER);
	}

	std::vector<ButtonConf> btnConf;

	void UpdateContacts(std::vector<UaConf::Contact> &contacts);

	void SetSaveAllSettings(bool state);

	void SetInitialSettings(void);	
};

#endif

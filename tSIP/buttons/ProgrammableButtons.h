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
	int ReadFile(AnsiString name);
	int LoadFromJsonValue(const Json::Value &root);	
public:
	ProgrammableButtons(void);
	void SetFilename(AnsiString name)
	{
		filename = name;
	}
	int Read(void);
	int ReadFromString(AnsiString json);
	int Write(void);

	enum { EXT_CONSOLE_COLUMNS = 12 };
	enum { CONSOLE_BTNS_PER_COLUMN = 15 };
	std::vector<ButtonConf> btnConf;

	void UpdateContacts(std::vector<UaConf::Contact> &contacts);	
};

#endif

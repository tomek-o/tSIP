//---------------------------------------------------------------------------

#ifndef AudioModulesH
#define AudioModulesH
//---------------------------------------------------------------------------

#include <string>

namespace Stdctrls
{
	class TComboBox;
}

namespace AudioModules
{

	extern const char* portaudio;
	extern const char* winwave;
	extern const char* winwave2;
	extern const char* aufile;
	extern const char* aufileMm;
	extern const char* nullaudio;

	void FillInputSelectorCb(Stdctrls::TComboBox *cb);
	void FillOutputSelectorCb(Stdctrls::TComboBox *cb);

	const char* GetInputModuleFromCbIndex(int id);
	int GetInputModuleCbIndex(const std::string& name);

	/** \brief Common combobox index -> output audio module name function
	*/
	const char* GetOutputModuleFromCbIndex(int id);
	int GetOutputModuleCbIndex(const std::string& name);

	bool IsInput(const std::string& name);
	bool IsOutput(const std::string& name);
}

#endif

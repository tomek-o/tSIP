//---------------------------------------------------------------------------

#ifndef VideoModulesH
#define VideoModulesH
//---------------------------------------------------------------------------

#include <string>

namespace Stdctrls
{
	class TComboBox;
}

namespace VideoModules
{

	extern const char* dshow;
	extern const char* sdl;

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

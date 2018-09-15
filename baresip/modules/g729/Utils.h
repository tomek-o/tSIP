/** \file
*/
//---------------------------------------------------------------------------

#ifndef UtilsH
#define UtilsH
//---------------------------------------------------------------------------
#include <string>
#include <windows.h>
#include <algorithm>

namespace Utils
{
	/** \brief Get current module handle
	*/
	HMODULE GetCurrentModule(void);

	/** \brief Get Dll or module path + name
		\return path + name
	*/
	std::string GetDllPath(void);

	/** \brief Replace extension of given filename.
		\param filename filename of filename with path
		\param ext new extension for filename (including period)
		\return filename with replaced extension

		Conditions used to replace extension:
		- period found in input filename
		- no slash or backslash after dot
	*/
	std::string ReplaceFileExtension(std::string filename, std::string ext);


	/** \brief Check if value is inside array
		\note Usage:
		int array[] = { 1, 2, 3, 4 };
		if( in_group(2, array) ) { ... }
		\note yay for n parameter!
	*/
	template<class Type, class Element, int n>
	bool in_group(const Type value, const Element (&array)[n])
	{
		return std::find(array, array+n, value) != array+n;
	}
}

#endif

#ifndef FontConfH
#define FontConfH

#include <System.hpp>
#include <Graphics.hpp>

namespace Json
{
	class Value;
}

namespace Stdctrls
{
	class TEdit;
}

struct FontConf
{
	enum {
		MIN_SIZE = 4,
		MAX_SIZE = 72
	};

	AnsiString name;
	int size;
	TFontStyles style;

	FontConf(void);
	
	bool operator==(const FontConf& right) const {
		return (
			name == right.name &&
			size == right.size &&
			style == right.style
		);
	}
	bool operator!=(const FontConf& right) const {
		return !(*this == right);
	}
	
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv) const;

	void ShowOnEdit(Stdctrls::TEdit *ed);
};

#endif

//---------------------------------------------------------------------------

#ifndef VideoConfH
#define VideoConfH
//---------------------------------------------------------------------------

#include <System.hpp>

namespace Json
{
	class Value;
}

struct VideoConf
{
	bool enabled;

	enum DisplayParentType {
		DISPLAY_PARENT_NONE = 0,	///< separate video window
		DISPLAY_PARENT_BUTTON,		///< video on a programmable button

		DISPLAY_PARENT__LIMITER
	} displayParentType;
	static const char* getDisplayParentTypeName(enum DisplayParentType type);

	int displayParentId;

	VideoConf(void);
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv);

	bool operator==(const VideoConf &right) const;
	bool operator!=(const VideoConf &right) const
	{
		return !(*this == right);
	}
};


#endif

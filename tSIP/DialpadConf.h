//---------------------------------------------------------------------------
#ifndef DialpadConfH
#define DialpadConfH
//---------------------------------------------------------------------------

namespace Json
{
	class Value;
}

struct DialpadConf
{
	enum ElementId
	{
		EL_ED_TRANSFER = 0,
		EL_TRBAR_MIC_VOLUME,
		EL_BTN_RESET_MIC_VOLUME,
		EL_TRBAR_SPEAKER_VOLUME,
		EL_BTN_RESET_SPEAKER_VOLUME,

		EL_CNT
	};

	struct ElementConf
	{
		bool visible;
		int left;
		int top;
		unsigned int width;
		unsigned int height;
		ElementConf(void):
			visible(true),
			left(0),
			top(0),
			width(30),
			height(30)
		{}
		void fromJson(const Json::Value &jv);
		void toJson(Json::Value &jv);
		bool operator==(const ElementConf &right) const;
		bool operator!=(const ElementConf &right) const
		{
			return !(*this == right);
		}
	} elements[EL_CNT];

	DialpadConf(void);
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv);

	bool operator==(const DialpadConf &right) const;
	bool operator!=(const DialpadConf &right) const
	{
		return !(*this == right);
	}
};

#endif

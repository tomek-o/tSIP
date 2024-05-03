//---------------------------------------------------------------------------

#ifndef TrayNotifierConfH
#define TrayNotifierConfH
//---------------------------------------------------------------------------

namespace Json
{
	class Value;
}

struct TrayNotifierConf
{
	int iPosX, iPosY;				///< coordinates
	unsigned int iHeight, iWidth;
	bool showOnIncoming;
	bool skipIfMainWindowVisible;
	bool showOnOutgoing;
	bool hideWhenAnsweringCall;		///< for manual call answer
	bool hideWhenAnsweringCallAutomatically;
	enum { SCALING_MIN = 50 };
	enum { SCALING_DEF = 100 };
	enum { SCALING_MAX = 500 };
	int scalingPct;					///< scaling (percentage)
	bool doNotChangePosition;

	struct ElementConf
	{
		bool visible;
		int left;
		int top;
		unsigned int width;
		ElementConf(void):
			visible(true),
			left(0),
			top(0),
			width(30)
		{}
		void fromJson(const Json::Value &jv);
		void toJson(Json::Value &jv) const;
		bool operator==(const ElementConf &right) const;
		bool operator!=(const ElementConf &right) const
		{
			return !(*this == right);
		}
	};

	struct Elements
	{
		ElementConf labelDescription, labelUri, btnAnswer, btnHangup;
	} elements;

	TrayNotifierConf(void);
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv) const;

private:
	int maxX, maxY;
};

#endif

//---------------------------------------------------------------------------

#ifndef ProgrammableButtonsH
#define ProgrammableButtonsH
//---------------------------------------------------------------------------

#include "ButtonConf.h"
#include "UaConf.h"
#include <vector>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>

namespace Json
{
	class Value;
}

class TProgrammableButton;
class TfrmButtonContainer;
class TdmButtons;

class ProgrammableButtons
{
private:
	// non-copyable: contains VCL components
	ProgrammableButtons(const ProgrammableButtons& source) {
	};
	ProgrammableButtons& operator=(const ProgrammableButtons&);

    TdmButtons *dmButtons;

	AnsiString filename;
	bool saveAllSettings;
	bool updated;
	int ReadFile(AnsiString name);
	int LoadFromJsonValue(const Json::Value &root);

	std::vector<TProgrammableButton*> btns;

	bool panelIsMoving;
	bool panelIsResizing;
	int editedPanelId;
	int scalingPercentage;

	typedef void (__closure *CallbackClick)(int id, TProgrammableButton* btn);
	typedef void (__closure *CallbackMouseUpDown)(int id, TProgrammableButton* btn);
	typedef void (__closure *CallbackSetKeepForeground)(bool disable);	
	typedef void (__closure *CallbackRestartUa)(void);
	CallbackClick callbackClick;
	CallbackMouseUpDown callbackMouseUpDown;
	CallbackSetKeepForeground callbackSetKeepForeground;
	CallbackRestartUa callbackRestartUa;

	void __fastcall containerBackgroundClick(TObject *Sender);	
	void __fastcall SpeedDialPanelClick(TObject *Sender);
	void OnPanelMouseUpDown(TProgrammableButton *btn);
	TfrmButtonContainer *GetBtnContainer(int btnId);
	void EndEditing(TfrmButtonContainer *container);

	void __fastcall miAddEditPanelClick(TObject *Sender);
	void __fastcall tmrMovingTimer(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);	

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
	enum { CONSOLE_BTNS_PER_CONTAINER = 200 };

	static int GetTotalCnt(void)
	{
		return BASIC_PANEL_CONSOLE_BTNS + (EXT_CONSOLE_COLUMNS * CONSOLE_BTNS_PER_CONTAINER);
	}

	std::vector<ButtonConf> btnConf;

	void UpdateContacts(std::vector<UaConf::Contact> &contacts);

	void SetSaveAllSettings(bool state);

	void SetInitialSettings(void);

	void SetInitialDialpad(void);

	void Create(TComponent* Owner,
		int scalingPercentage,
		CallbackClick callbackClick,
		CallbackMouseUpDown callbackMouseUpDown,
		CallbackSetKeepForeground callbackSetKeepForeground,
		CallbackRestartUa callbackRestartUa
		);

	void Destroy(void);

	void Edit(int id);

	void Move(int id);

	void Resize(int id);

	void UpdateAll(void);	

	TProgrammableButton* GetBtn(int id) {
		if (id >= 0 && id < btns.size())
			return btns[id];
		return NULL;
	}

	void UseContextMenu(bool state);
	void SetScalingPercentage(int val) {
    	scalingPercentage = val;
	}

	void ApplyButtonCfg(int id, const ButtonConf &cfg);

	void UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay);
	void UpdatePresenceState(int id, int state, AnsiString note);
	void UpdateBtnState(Button::Type type, bool state);
	void UpdateMwiState(int newMsg, int oldMsg);
	void UpdateAutoAnswer(bool enabled, int sipCode, AnsiString sipReason);

	void SetConfig(int btnId, const ButtonConf &conf);
};

#endif

//---------------------------------------------------------------------------

#ifndef FormButtonContainerH
#define FormButtonContainerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
#include <vector>

#include "ButtonType.h"

class TProgrammableButton;
class ProgrammableButtons;

class TfrmButtonContainer : public TForm
{
__published:	// IDE-managed Components
	TFlowPanel *flowPanel;
	TImageList *imgList;
	TPopupMenu *popupPanel;
	TMenuItem *miEditSpeedDial;
	void __fastcall miEditSpeedDialClick(TObject *Sender);
public:
	typedef void (__closure *CallbackClick)(int id, TProgrammableButton* btn);
	void UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay);
	void UpdatePresenceState(int id, int state, AnsiString note);
	void UpdateBtnState(Button::Type type, bool state);
	void UpdateBtnCaption(int id, AnsiString text);
	void UpdateBtnDown(int id, bool state);
	void UpdateBtnImage(int id, AnsiString file);
	void UpdateMwiState(int newMsg, int oldMsg);
	void UpdateSettings(void);
private:	// User declarations
	CallbackClick callbackClick;
	void __fastcall SpeedDialPanelClick(TObject *Sender);
	void EditContact(int id);
	std::vector<TProgrammableButton*> vpanels;
	int startBtnId;
	int btnCnt;
	bool useContextMenu;

	ProgrammableButtons &buttons;
public:		// User declarations
	__fastcall TfrmButtonContainer(TComponent* Owner, ProgrammableButtons &buttons,
		int width, int height, int scalingPercentage,
		int startBtnId, int btnCnt, CallbackClick callbackClick);
	TProgrammableButton* GetBtn(int id) {
		if (id >= 0 && id < vpanels.size())
			return vpanels[id];
		return NULL;
	}
	void SetScaling(int percentage);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonContainer *frmButtonContainer;
//---------------------------------------------------------------------------
#endif

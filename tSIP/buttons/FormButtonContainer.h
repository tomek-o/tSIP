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
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#include <vector>

#include "ButtonType.h"
#include "common/Observer.h"

class TProgrammableButton;
class ProgrammableButtons;
struct ButtonConf;

class TfrmButtonContainer : public TForm, Observer
{
__published:	// IDE-managed Components
	TImageList *imgList;
	TPopupMenu *popupPanel;
	TMenuItem *miEditSpeedDial;
	TPanel *panelMain;
	TPopupMenu *popupAddPanel;
	TMenuItem *miAddEditPanel;
	TMenuItem *miSetBackground;
	TOpenDialog *openDialog;
	TImage *imgBackground;
	TMenuItem *miClearBackground;
	TMenuItem *miMoveSpeedDial;
	TMenuItem *miCopyPanel;
	TPanel *pnlStatus;
	TListView *lvStatus;
	TImageList *imglistStatus;
	TMenuItem *miResizeSpeedDial;
	TTimer *tmrMoving;
	TPanel *movingFrame;
	TMenuItem *miContainerMenu;
	void __fastcall miEditSpeedDialClick(TObject *Sender);
	void __fastcall popupAddPanelPopup(TObject *Sender);
	void __fastcall miAddEditPanelClick(TObject *Sender);
	void __fastcall miSetBackgroundClick(TObject *Sender);
	void __fastcall miClearBackgroundClick(TObject *Sender);
	void __fastcall miMoveSpeedDialClick(TObject *Sender);
	void __fastcall imgBackgroundClick(TObject *Sender);
	void __fastcall miCopyPanelClick(TObject *Sender);
	void __fastcall lvStatusData(TObject *Sender, TListItem *Item);
	void __fastcall miResizeSpeedDialClick(TObject *Sender);
	void __fastcall tmrMovingTimer(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);
public:
	typedef void (__closure *CallbackClick)(int id, TProgrammableButton* btn);
	typedef void (__closure *CallbackMouseUpDown)(int id, TProgrammableButton* btn);
	typedef void (__closure *CallbackUpdateAll)(void);
	typedef void (__closure *CallbackSetKeepForeground)(bool disable);
	void UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay);
	void UpdatePresenceState(int id, int state, AnsiString note);
	void UpdateBtnState(Button::Type type, bool state);
	void UpdateMwiState(int newMsg, int oldMsg);
	void UpdateAutoAnswer(bool enabled, int sipCode);
	void UpdateSettings(void);
	void UpdateBackgroundImage(AnsiString file);
private:	// User declarations
	CallbackClick callbackClick;
	CallbackMouseUpDown callbackMouseUpDown;
	CallbackUpdateAll callbackUpdateAll;
	CallbackSetKeepForeground callbackSetKeepForeground;
	void __fastcall SpeedDialPanelClick(TObject *Sender);
	void OnPanelMouseUpDown(TProgrammableButton *btn);
	void EditContact(int id);
	void MovePanel(int id);
	void ResizePanel(int id);
	void ApplyButtonCfg(int id, const ButtonConf &cfg);
	std::vector<TProgrammableButton*> vpanels;
	int startBtnId;
	int btnCnt;
	bool useContextMenu;
	AnsiString lastImage;

	bool panelIsMoving;
	bool panelIsResizing;
	int editedPanelId;

	int scalingPercentage;

	bool showStatus;
	bool hideEmptyStatus;

	ProgrammableButtons &buttons;
public:		// User declarations
	__fastcall TfrmButtonContainer(TComponent* Owner, ProgrammableButtons &buttons,
		int width, int height, int scalingPercentage,
		int startBtnId, int btnCnt,
		CallbackClick callbackClick,
		CallbackMouseUpDown callbackMouseUpDown,
		CallbackUpdateAll callbackUpdateAll,
		CallbackSetKeepForeground callbackSetKeepForeground,
        bool showStatus, int statusPanelHeight, bool hideEmptyStatus
		);
	TProgrammableButton* GetBtn(int id) {
		if (id >= 0 && id < vpanels.size())
			return vpanels[id];
		return NULL;
	}
	void SetScaling(int percentage);
	int GetStartBtnId(void) const {
		return startBtnId;
	}
	int GetBtnCnt(void) const {
 		return btnCnt;
	}
	void UpdateBackgroundImage(void);
	void ShowStatusPanel(bool state);
	void obsUpdate(Observable* o, Argument * arg);
	void UpdateAll(void);	
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonContainer *frmButtonContainer;
//---------------------------------------------------------------------------
#endif

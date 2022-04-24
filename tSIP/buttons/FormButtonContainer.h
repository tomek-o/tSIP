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
	TPanel *panelMain;
	TPopupMenu *popupAddPanel;
	TMenuItem *miAddEditPanel;
	TMenuItem *miSetBackground;
	TOpenDialog *openDialog;
	TImage *imgBackground;
	TMenuItem *miClearBackground;
	TMenuItem *miCopyPanel;
	TPanel *pnlStatus;
	TListView *lvStatus;
	TImageList *imglistStatus;
	TTimer *tmrMoving;
	TPanel *movingFrame;
	void __fastcall popupAddPanelPopup(TObject *Sender);
	void __fastcall miAddEditPanelClick(TObject *Sender);
	void __fastcall miSetBackgroundClick(TObject *Sender);
	void __fastcall miClearBackgroundClick(TObject *Sender);
	void __fastcall miCopyPanelClick(TObject *Sender);
	void __fastcall lvStatusData(TObject *Sender, TListItem *Item);
	void __fastcall panelMainMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall imgBackgroundMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
public:
	typedef void (__closure *CallbackSetKeepForeground)(bool disable);
	void UpdateSettings(void);
	void UpdateBackgroundImage(AnsiString file);
	TPanel* GetButtonParent(void) {
    	return panelMain;
	}
private:	// User declarations
	unsigned int containerId;
	CallbackSetKeepForeground callbackSetKeepForeground;
	bool useContextMenu;
	AnsiString lastImage;

	int scalingPercentage;

	bool showStatus;
	bool hideEmptyStatus;
	
	ProgrammableButtons &buttons;
public:		// User declarations
	__fastcall TfrmButtonContainer(TComponent* Owner, ProgrammableButtons &buttons,
		unsigned int containerId,
		int width, int height, int scalingPercentage,
		CallbackSetKeepForeground callbackSetKeepForeground,
        bool showStatus, int statusPanelHeight, bool hideEmptyStatus
		);
	void UpdateBackgroundImage(void);
	void ShowStatusPanel(bool state);
	void obsUpdate(Observable* o, Argument * arg);
	void UpdateAll(void);	
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmButtonContainer *frmButtonContainer;
//---------------------------------------------------------------------------
#endif

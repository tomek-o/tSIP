//---------------------------------------------------------------------------

#ifndef ButtonsDataModuleH
#define ButtonsDataModuleH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TdmButtons : public TDataModule
{
__published:	// IDE-managed Components
	TPopupMenu *popupPanel;
	TMenuItem *miInfo;
	TMenuItem *N1;
	TMenuItem *miEditSpeedDial;
	TMenuItem *miMoveSpeedDial;
	TMenuItem *miResizeSpeedDial;
	TMenuItem *miContainerMenu;
	TImageList *imgList;
	TTimer *tmrMoving;
	TMenuItem *miCopyCaption1;
	TMenuItem *miCopyCaption2;
	TMenuItem *miCopyButtonProperties;
	TMenuItem *miMoveSpeedDialGroup;
	void __fastcall popupPanelPopup(TObject *Sender);
	void __fastcall miEditSpeedDialClick(TObject *Sender);
	void __fastcall miMoveSpeedDialClick(TObject *Sender);
	void __fastcall miResizeSpeedDialClick(TObject *Sender);
	void __fastcall miCopyCaption1Click(TObject *Sender);
	void __fastcall miCopyCaption2Click(TObject *Sender);
	void __fastcall miCopyButtonPropertiesClick(TObject *Sender);
	void __fastcall miMoveSpeedDialGroupClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TdmButtons(TComponent* Owner);
};
//---------------------------------------------------------------------------
#endif

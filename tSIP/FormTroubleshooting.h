//---------------------------------------------------------------------------

#ifndef FormTroubleshootingH
#define FormTroubleshootingH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
//---------------------------------------------------------------------------
class TfrmTroubleshooting : public TForm
{
__published:	// IDE-managed Components
	TPanel *pnlBottom;
	TListView *lvItems;
	TLabel *lblDetails;
	TMemo *memoDetails;
	TImageList *imgList;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lvItemsData(TObject *Sender, TListItem *Item);
	void __fastcall lvItemsSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
private:	// User declarations
	void SelectItem(TListItem *Item);
public:		// User declarations
	__fastcall TfrmTroubleshooting(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmTroubleshooting *frmTroubleshooting;
//---------------------------------------------------------------------------
#endif

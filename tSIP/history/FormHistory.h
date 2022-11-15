//---------------------------------------------------------------------------

#ifndef FormHistoryH
#define FormHistoryH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------

#include "common/Observer.h"
#include <ImgList.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>

#include "History.h"
#include <Menus.hpp>
#include <vector>

struct HistoryConf;

class TfrmHistory : public TForm, Observer
{
__published:	// IDE-managed Components
	TListView *lvHistory;
	TImageList *imgList;
	TPanel *pnlBottom;
	TImage *imgFilter;
	TEdit *edFilter;
	TPopupMenu *popupHistory;
	TMenuItem *miCopyNumber;
	TMenuItem *miAddEditPhonebook;
	TMenuItem *miHttpQuery;
	TMenuItem *miMessage;
	TMenuItem *miRecordFile;
	TMenuItem *miRecordFileOpen;
	TMenuItem *miOpenInExplorer;
	void __fastcall lvHistoryData(TObject *Sender, TListItem *Item);
	void __fastcall lvHistoryDblClick(TObject *Sender);
	void __fastcall edFilterChange(TObject *Sender);
	void __fastcall miCopyNumberClick(TObject *Sender);
	void __fastcall miAddEditPhonebookClick(TObject *Sender);
	void __fastcall miHttpQueryClick(TObject *Sender);
	void __fastcall edFilterKeyPress(TObject *Sender, char &Key);
	void __fastcall lvHistoryInfoTip(TObject *Sender, TListItem *Item,
          AnsiString &InfoTip);
	void __fastcall miMessageClick(TObject *Sender);
	void __fastcall miRecordFileOpenClick(TObject *Sender);
	void __fastcall miOpenInExplorerClick(TObject *Sender);
	void __fastcall popupHistoryPopup(TObject *Sender);
public:
	typedef void (__closure *CallbackCall)(AnsiString uri);
	typedef void (__closure *CallbackPhonebookEdit)(AnsiString uri);
	typedef void (__closure *CallbackHttpQuery)(AnsiString uri);
private:	// User declarations
	static void TranslateForm(void* obj);
	History *history;
	const HistoryConf &conf;
	CallbackCall callbackCall;
	CallbackPhonebookEdit callbackPhonebookEdit;
	CallbackHttpQuery callbackHttpQuery;
	struct FilteredEntry
	{
		int id;
		History::Entry entry;
	};
	std::vector<FilteredEntry> filteredEntries;
	bool updateNeeded;
	bool updating;					///< view needs to be updated on data change (window is visible)
	History::Entry* getSelectedEntry(void);
	AnsiString getDefaultUri(const History::Entry* entry);
	AnsiString GetHint(TListItem *item);
	void AddPaiToHint(AnsiString &hint, const History::Entry &entry);
	void SetColumnWidths(const std::vector<int>& widths);
	void FilterHistory(void);
public:		// User declarations
	__fastcall TfrmHistory(TComponent* Owner, History *history, const HistoryConf &historyConf,
		CallbackCall callbackCall,
		CallbackPhonebookEdit callbackPhonebookEdit,
		CallbackHttpQuery callbackHttpQuery
		);
	void obsUpdate(Observable* o, Argument * arg);
	/** Enable/disable immediate updating on data change.
		If window is not visible updating can be delayed to reduce CPU load.
	*/
	void SetUpdating(bool state);
	/** Scaling - manual scaling required for listview column width
	*/
	void Scale(int percentage);

	std::vector<int> GetColumnWidths(void);
	void UpdateShowHint(void);
	void UpdateConf(const HistoryConf &prev);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmHistory *frmHistory;
//---------------------------------------------------------------------------
#endif

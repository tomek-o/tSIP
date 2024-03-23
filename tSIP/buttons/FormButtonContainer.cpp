//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainer.h"
#include "FormButtonContainerConf.h"
#include "ProgrammableButton.h"
#include "ProgrammableButtons.h"
#include "SpeedDialStatus.h"
#include "Settings.h"
#include "UaMain.h"
#include "Log.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonContainer *frmButtonContainer;

namespace
{
	bool IsShiftPressed(void)
	{
		int res = GetAsyncKeyState(VK_SHIFT);
		if (res & 0x8000)
			return true;
		return false;
	}
}

//---------------------------------------------------------------------------
__fastcall TfrmButtonContainer::TfrmButtonContainer(TComponent* Owner,
	ProgrammableButtons &buttons,
	unsigned int containerId,
	int width, int height, int scalingPercentage,
	CallbackSetKeepForeground callbackSetKeepForeground,
	bool showStatus, int statusPanelHeight, bool hideEmptyStatus)
	:
	TForm(Owner),
	buttons(buttons), containerId(containerId),
	callbackSetKeepForeground(callbackSetKeepForeground),
	scalingPercentage(scalingPercentage),
	showStatus(showStatus),
	hideEmptyStatus(hideEmptyStatus)
{
	assert(callbackSetKeepForeground);
	if (width > 0)
	{
		this->Width = width;
	}
	if (height > 0)
	{
    	this->Height = height;
	}
	useContextMenu = appSettings.frmMain.bSpeedDialPopupMenu;

	if (useContextMenu == false)
	{
		panelMain->PopupMenu = NULL;
	}

	pnlStatus->Visible = showStatus && !hideEmptyStatus;
	pnlStatus->Height = statusPanelHeight;

	speedDialStatus.addObserver(*this);	
}

//---------------------------------------------------------------------------


void TfrmButtonContainer::UpdatePopupSettings(void)
{
	if (useContextMenu == appSettings.frmMain.bSpeedDialPopupMenu)
	{
        return;
	}
	useContextMenu = appSettings.frmMain.bSpeedDialPopupMenu;
	panelMain->PopupMenu = useContextMenu ? popupAddPanel : NULL;

}

void __fastcall TfrmButtonContainer::popupAddPanelPopup(TObject *Sender)
{
	miAddEditPanel->Clear();
	TMenuItem *item;
	miAddEditPanel->AutoHotkeys = maManual;
	for (int i=0; i<buttons.btnConf.size(); i++)
	{
		item = new TMenuItem(popupAddPanel);
		item->Tag = i;
	#if 0
		if (i > 0 && i % 50 == 0)
		{
            // scroll disappears if this is used
			item->Break = mbBarBreak;
		}
	#endif
		item->AutoHotkeys = maManual;
		AnsiString text;
		AnsiString caption = "[unnamed]";
		ButtonConf &cfg = buttons.btnConf[i];
		if (cfg.caption != "")
			caption = cfg.caption.c_str();		
		text.sprintf("Button #%02d: %s", i, caption.c_str());
		item->Caption = text;
		item->OnClick = miAddEditPanelClick;
		miAddEditPanel->Add(item);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::miAddEditPanelClick(TObject *Sender)
{
	TMenuItem *item = dynamic_cast<TMenuItem*>(Sender);
	assert(item);
	if (item == NULL)
		return;
	int id = item->Tag;
	buttons.Edit(id);
}

void TfrmButtonContainer::UpdateBackgroundImage(AnsiString file)
{
	AnsiString asBackgroundFile;
	try
	{
		if (file != "" && file != lastImage)
		{
			asBackgroundFile.sprintf("%s\\img\\%s", ExtractFileDir(Application->ExeName).c_str(), file.c_str());
			imgBackground->Picture->Bitmap->PixelFormat = pf24bit;
			imgBackground->Picture->LoadFromFile(asBackgroundFile);
			lastImage = file;
		}
		else if (appSettings.buttonContainers[containerId].backgroundImage == "")
		{
			imgBackground->Picture = NULL;
			lastImage = "";
		}
	}
	catch (...)
	{
		LOG("Failed to load background (%s)\n", asBackgroundFile.c_str());
	}
}

void __fastcall TfrmButtonContainer::miCopyPanelClick(TObject *Sender)
{
    buttons.CopyConfig(-1);
}
//---------------------------------------------------------------------------


void TfrmButtonContainer::obsUpdate(Observable* o, Argument * arg)
{
	const std::vector<struct SpeedDialStatus::Entry>& entries = speedDialStatus.GetEntries();
	lvStatus->Items->Count = entries.size();
	lvStatus->Invalidate();	
	if (hideEmptyStatus)
	{
		pnlStatus->Visible = showStatus && (entries.size() > 0);
	}
}

void __fastcall TfrmButtonContainer::lvStatusData(TObject *Sender,
      TListItem *Item)
{
	const std::vector<struct SpeedDialStatus::Entry>& entries = speedDialStatus.GetEntries();
	int id = Item->Index;
	const struct SpeedDialStatus::Entry& entry = entries[id];
	Item->ImageIndex = entry.type;
	Item->SubItems->Add(entry.msg);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::ShowStatusPanel(bool state)
{
	pnlStatus->Visible = state;
}


void __fastcall TfrmButtonContainer::panelMainMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	ShowMessage("mose down");	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::imgBackgroundMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if (appSettings.frmSpeedDial.dragApplicationWithButtonContainer && Button == mbLeft)
	{
		enum {SC_DRAGMOVE = 0xF012};
		ReleaseCapture();
		SendMessage(Application->MainForm->Handle, WM_SYSCOMMAND, SC_DRAGMOVE, 0);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::miConfigureButtonContainerClick(
      TObject *Sender)
{
	callbackSetKeepForeground(false);
	if (frmButtonContainerConf == NULL)
	{
		Application->CreateForm(__classid(TfrmButtonContainerConf), &frmButtonContainerConf);
	}
	ButtonContainerConf tmp = appSettings.buttonContainers[containerId];
	frmButtonContainerConf->ShowModal(&tmp);
	if (frmButtonContainerConf->isConfirmed())
	{
		if (tmp != appSettings.buttonContainers[containerId])
		{
			appSettings.buttonContainers[containerId] = tmp;
			ApplyConfig();
			AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
			appSettings.Write(asConfigFile);
		}
	}
	callbackSetKeepForeground(true);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::ApplyConfig(void)
{
	const ButtonContainerConf &cfg = appSettings.buttonContainers[containerId];
	UpdateBackgroundImage(cfg.backgroundImage);
	panelMain->Color = static_cast<TColor>(cfg.backgroundColor);
}

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainer.h"
#include "ProgrammableButton.h"
#include "FormButtonEdit.h"
#include "FormButtonCopy.h"
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
    int startBtnId, int btnCnt,
	CallbackClick callbackClick,
	CallbackMouseUpDown callbackMouseUpDown,
	CallbackUpdateAll callbackUpdateAll,
	CallbackSetKeepForeground callbackSetKeepForeground,	
	bool showStatus, int statusPanelHeight, bool hideEmptyStatus)
	:
	TForm(Owner),
	buttons(buttons), containerId(containerId), startBtnId(startBtnId), btnCnt(btnCnt),
	callbackClick(callbackClick),
	callbackMouseUpDown(callbackMouseUpDown),
	callbackUpdateAll(callbackUpdateAll),
	callbackSetKeepForeground(callbackSetKeepForeground),
	panelIsMoving(false),
	panelIsResizing(false),
	scalingPercentage(scalingPercentage),
	showStatus(showStatus),
	hideEmptyStatus(hideEmptyStatus)
{
	assert(startBtnId >= 0);
	assert(btnCnt >= 0);
	assert(callbackClick);
	assert(callbackMouseUpDown);
	assert(callbackUpdateAll);
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
	for (int i=0; i<btnCnt; i++)
	{
		TProgrammableButton *panel = new TProgrammableButton(panelMain, imgList, scalingPercentage);
		panel->Tag = i;
		panel->Parent = panelMain;
		//panel->AlignWithMargins = true;
		if (i < buttons.btnConf.size())
		{
			ButtonConf &cfg = buttons.btnConf[startBtnId + i];
			panel->SetConfig(cfg);
		}
		panel->OnClick = SpeedDialPanelClick;
		panel->OnDblClick = SpeedDialPanelClick;
		panel->SetMouseUpDownCallback(OnPanelMouseUpDown);
		panel->PopupMenu = useContextMenu ? popupPanel : NULL;
		panel->UpdateCallbacks();
		vpanels.push_back(panel);
	}

	if (useContextMenu == false)
	{
		panelMain->PopupMenu = NULL;
	}

	pnlStatus->Visible = showStatus && !hideEmptyStatus;
	pnlStatus->Height = statusPanelHeight;

	speedDialStatus.addObserver(*this);	
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::SetScaling(int percentage)
{
    scalingPercentage = percentage;
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		TProgrammableButton *panel = vpanels[i];
		panel->SetScaling(percentage);
	}
}


void __fastcall TfrmButtonContainer::SpeedDialPanelClick(TObject *Sender)
{
	if (panelIsMoving || panelIsResizing)
	{
		imgBackgroundClick(Sender);
	}
	else
	{
		TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(Sender);
		if (panel == NULL)
		{
			TComponent *component = dynamic_cast<TComponent*>(Sender);
			assert(component);
			panel = dynamic_cast<TProgrammableButton*>(component->Owner);
		}
		assert(panel);
		if (panel->GetInactive() == true)
		{
			return;
        }
		int id = startBtnId + panel->Tag;

		ButtonConf &cfg = buttons.btnConf[id];
		if (cfg.type == Button::HOLD || cfg.type == Button::MUTE || cfg.type == Button::MUTE_RING)
		{
			panel->SetDown(!panel->GetDown());
		}

		callbackClick(id, panel);
	}
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::OnPanelMouseUpDown(TProgrammableButton *btn)
{
	if (panelIsMoving || panelIsResizing)
	{
		return;
	}
	int id = startBtnId + btn->Tag;
	callbackMouseUpDown(id, btn);	
}


void __fastcall TfrmButtonContainer::miEditSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	callbackSetKeepForeground(false);
	EditContact(id);
	callbackSetKeepForeground(true);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::EditContact(int id)
{
	ButtonConf cfg = buttons.btnConf[startBtnId + id];	// copy
	AnsiString caption;
	caption.sprintf("Edit button #%02d", startBtnId + id);
	frmButtonEdit->Caption = caption;
	frmButtonEdit->ShowModal(&cfg, startBtnId + id);

	if (frmButtonEdit->isConfirmed())
	{
		ApplyButtonCfg(id, cfg);
		this->Repaint();
	}
}

void TfrmButtonContainer::UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;	
	vpanels[id]->SetState((enum dialog_info_status)state, updateRemoteIdentity, (enum dialog_info_direction)direction, remoteIdentity, remoteIdentityDisplay);
}

void TfrmButtonContainer::UpdatePresenceState(int id, int state, AnsiString note)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;	
	vpanels[id]->SetPresenceState((enum presence_status)state, note);
}

void TfrmButtonContainer::UpdateBtnState(Button::Type type, bool state)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		ButtonConf &cfg = buttons.btnConf[i + startBtnId];
		if (cfg.type == type)
		{
			vpanels[i]->SetDown(state);
		}
	}
}

void TfrmButtonContainer::UpdateMwiState(int newMsg, int oldMsg)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		ButtonConf &cfg = buttons.btnConf[i + startBtnId];
		if (cfg.type == Button::MWI)
		{
			vpanels[i]->SetMwiState(newMsg, oldMsg);
		}
	}
}

void TfrmButtonContainer::UpdateAutoAnswer(bool enabled, int sipCode)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		ButtonConf &cfg = buttons.btnConf[i + startBtnId];
		if (cfg.type == Button::AUTO_ANSWER_DND)
		{
			if (cfg.sipCode == sipCode && enabled)
			{
				vpanels[i]->SetDown(true);
			}
			else
			{
				vpanels[i]->SetDown(false);
			}
		}
	}
}

void TfrmButtonContainer::UpdateSettings(void)
{
	if (useContextMenu == appSettings.frmMain.bSpeedDialPopupMenu)
	{
        return;
	}
	useContextMenu = appSettings.frmMain.bSpeedDialPopupMenu;
	panelMain->PopupMenu = useContextMenu ? popupAddPanel : NULL;	
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		vpanels[i]->PopupMenu = useContextMenu ? popupPanel : NULL;
	}
}

void __fastcall TfrmButtonContainer::popupAddPanelPopup(TObject *Sender)
{
	miAddEditPanel->Clear();
	TMenuItem *item;
	miAddEditPanel->AutoHotkeys = maManual;
	for (int i=startBtnId; i<startBtnId+btnCnt; i++)
	{
		item = new TMenuItem(popupAddPanel);
		item->Tag = i;
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
	callbackSetKeepForeground(false);
	EditContact(id-startBtnId);
	callbackSetKeepForeground(true);
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::miSetBackgroundClick(TObject *Sender)
{
	AnsiString &fname = appSettings.buttonContainers[containerId].backgroundImage;
	openDialog->Filter = "Bitmaps (*.bmp)|*.bmp|All files|*.*";
	AnsiString dir = ExtractFileDir(Application->ExeName) + "\\img\\";
	openDialog->InitialDir = dir;
	if (FileExists(dir + fname))
		openDialog->FileName = dir + fname;
	else
		openDialog->FileName = "";
	callbackSetKeepForeground(false);
	if (openDialog->Execute())
	{
		fname = ExtractFileName(openDialog->FileName);
		UpdateBackgroundImage();
		AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
		appSettings.Write(asConfigFile);
	}
	callbackSetKeepForeground(true);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::UpdateBackgroundImage(void)
{
	UpdateBackgroundImage(appSettings.buttonContainers[containerId].backgroundImage);
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

void __fastcall TfrmButtonContainer::miClearBackgroundClick(TObject *Sender)
{
	appSettings.buttonContainers[containerId].backgroundImage = "";
	UpdateBackgroundImage();
	AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
	appSettings.Write(asConfigFile);
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::miMoveSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	MovePanel(id);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::MovePanel(int id)
{
	editedPanelId = id;
	panelIsMoving = true;
	imgBackground->Cursor = crCross;
	tmrMoving->Enabled = true;
}

void __fastcall TfrmButtonContainer::miResizeSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	ResizePanel(id);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::ResizePanel(int id)
{
	editedPanelId = id;
	panelIsResizing = true;
	imgBackground->Cursor = crCross;
	tmrMoving->Enabled = true;	
}

void __fastcall TfrmButtonContainer::imgBackgroundClick(TObject *Sender)
{
	if (panelIsMoving)
	{
		panelIsMoving = false;
		imgBackground->Cursor = crDefault;

		TPoint P = this->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = buttons.btnConf[startBtnId + editedPanelId];	// copy
		cfg.left = P.x * 100/scalingPercentage;
		cfg.top = P.y * 100/scalingPercentage;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = cfg.left % grid;
			if (modX >= grid/2)
				cfg.left += grid - modX;
			else
				cfg.left -= modX;
			int modY = cfg.top % grid;
			if (modY >= grid/2)
				cfg.top += grid - modY;
			else
				cfg.top -= modY;
		}

		ApplyButtonCfg(editedPanelId, cfg);
	}
	else if (panelIsResizing)
	{
		panelIsResizing = false;
		imgBackground->Cursor = crDefault;

		TPoint P = this->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = buttons.btnConf[startBtnId + editedPanelId];	// copy
		cfg.width = P.x * 100/scalingPercentage - cfg.left;
		cfg.height = P.y * 100/scalingPercentage - cfg.top;

		if (cfg.width < 10 || cfg.height < 10)
			return;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = cfg.width % grid;
			if (modX >= grid/2)
				cfg.width += grid - modX;
			else
				cfg.width -= modX;
			int modY = cfg.height % grid;
			if (modY >= grid/2)
				cfg.height += grid - modY;
			else
				cfg.height -= modY;
		}

		ApplyButtonCfg(editedPanelId, cfg);
	}
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::ApplyButtonCfg(int id, const ButtonConf &cfg)
{
	if (cfg != buttons.btnConf[startBtnId + id])
	{
		bool restartUa = false;
		if (cfg.UaRestartNeeded(buttons.btnConf[startBtnId + id]))
		{
			restartUa = true;
		}

		buttons.btnConf[startBtnId + id] = cfg;
		TProgrammableButton* panel = vpanels[id];
		panel->SetConfig(cfg);
		if (cfg.type != Button::BLF)
		{
			panel->SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");	// make sure BLF icon is cleared
		}
		if (cfg.type != Button::PRESENCE)
		{
			panel->ClearPresenceState();			// clear icon
		}
		buttons.Write();

		if (!appSettings.uaConf.disableUa && restartUa)
		{
			buttons.UpdateContacts(appSettings.uaConf.contacts);
			Ua::Instance().Restart();
		}
	}
	else
	{
		// assign anyway - some fields may be unintentionally omited from != operator
		buttons.btnConf[startBtnId + id] = cfg;
		buttons.Write();
	}
}

void __fastcall TfrmButtonContainer::miCopyPanelClick(TObject *Sender)
{
	frmButtonCopy->SetButtons(&buttons);
	callbackSetKeepForeground(false);
	frmButtonCopy->ShowModal();
	callbackSetKeepForeground(true);

	callbackUpdateAll();
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::UpdateAll(void)
{
	for (int id = startBtnId; id < buttons.btnConf.size(); id++)
	{
		if (id - startBtnId >= vpanels.size())
			break;
		TProgrammableButton* panel = vpanels[id - startBtnId];
		panel->SetConfig(buttons.btnConf[id]);
	}
	this->Repaint();
}

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


void __fastcall TfrmButtonContainer::tmrMovingTimer(TObject *Sender)
{
	if (!panelIsMoving && !panelIsResizing)
	{
		tmrMoving->Enabled = false;
		movingFrame->Visible = false;
		return;
	}
	movingFrame->Visible = true;
	TPoint P = this->ScreenToClient(Mouse->CursorPos);
	if (P.x < 0)
		P.x = 0;
	if (P.y < 0)
		P.y = 0;
	const ButtonConf &cfg = buttons.btnConf[startBtnId + editedPanelId];	// copy
	if (panelIsMoving)
	{
		int left = P.x * 100/scalingPercentage;
		int top = P.y * 100/scalingPercentage;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = left % grid;
			if (modX >= grid/2)
				left += grid - modX;
			else
				left -= modX;
			int modY = top % grid;
			if (modY >= grid/2)
				top += grid - modY;
			else
				top -= modY;
		}
		movingFrame->Top = top;
		movingFrame->Left = left;
		movingFrame->Width = cfg.width;
		movingFrame->Height = cfg.height;
		movingFrame->BringToFront();
	}
	else if (panelIsResizing)
	{
		int width = P.x * 100/scalingPercentage - cfg.left;
		int height = P.y * 100/scalingPercentage - cfg.top;

		if (width < 10 || height < 10)
			return;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = width % grid;
			if (modX >= grid/2)
				width += grid - modX;
			else
				width -= modX;
			int modY = height % grid;
			if (modY >= grid/2)
				height += grid - modY;
			else
				height -= modY;
		}
		movingFrame->Top = cfg.top;
		movingFrame->Left = cfg.left;
		movingFrame->Width = width;
		movingFrame->Height = height;
		movingFrame->BringToFront();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		if (panelIsMoving || panelIsResizing)
		{
			panelIsMoving = false;
			imgBackground->Cursor = crDefault;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::popupPanelPopup(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	AnsiString text;
	AnsiString typeText = Button::TypeName(buttons.btnConf[startBtnId + id].type);
	text.sprintf("Button #%d: %s", startBtnId + id, typeText.c_str());;
	miInfo->Caption = text;
}
//---------------------------------------------------------------------------


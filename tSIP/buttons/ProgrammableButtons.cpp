/** \note Configuration for buttons is seperated from main configuration
	to avoid rewriting it (~200kB for 180 buttons) every time application is closed.
	It can also be distributed separately (copied between instances or user) easier.
*/

#pragma hdrstop

#include "ProgrammableButtons.h"
#include "ProgrammableButton.h"
#include "ButtonsDataModule.h"
#include "ButtonContainers.h"
#include "FormButtonContainer.h"
#include "FormButtonEdit.h"
#include "FormButtonCopy.h"
#include "SettingsAppVersion.h"
#include "Settings.h"	// just for transition from column-based version < 0.2
#include "Sizes.h"
#include "ua/Calls.h"
#include "common/TimeCounter.h" 
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>
#include <Forms.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

namespace
{
	SettingsAppVersion appVersion;
	bool IsShiftPressed(void)
	{
		int res = GetAsyncKeyState(VK_SHIFT);
		if (res & 0x8000)
			return true;
		return false;
	}

	void UpdateCursorPos(int deltaX, int deltaY)
	{
		POINT pt;
		if (::GetCursorPos(&pt)) {
			pt.x += deltaX;
			pt.y += deltaY;
			::SetCursorPos(pt.x, pt.y);
			Sleep(200);	// simplified way to prevent too fast repetition
		}
	}

	enum
	{
		MIN_PANEL_WIDTH = 4,
		MIN_PANEL_HEIGHT = 4
	};
}

void ProgrammableButtons::SetDefaultsForBtnId(int id, ButtonConf& cfg)
{
	enum { HEIGHT = 32 };

	if (id < 0 || id >= GetTotalCnt())
		return;
	if (id < BASIC_PANEL_CONSOLE_BTNS)
	{
		cfg.left = 2;
		cfg.height = HEIGHT;
		cfg.width = 73;
		cfg.top = id * HEIGHT;
	}
	else
	{
		int offset = id - BASIC_PANEL_CONSOLE_BTNS;

		cfg.width = Sizes::COLUMN_WIDTH;
		int columnId = offset / BASIC_PANEL_CONSOLE_BTNS;
		cfg.left = Sizes::FIRST_COLUMN_LEFT + columnId * (Sizes::COLUMN_WIDTH + Sizes::COLUMN_SEPARATION);
		int offsetTop = (offset % BASIC_PANEL_CONSOLE_BTNS);
		cfg.top = Sizes::BUTTON_OFFSET_TOP + (offsetTop * HEIGHT);
		if (offsetTop > 10)
		{
			// some empty space before buttons that should not be visible to make aligning easier
			cfg.top += 10;
		}
	}
}

ProgrammableButtons::ProgrammableButtons(void):
	dmButtons(NULL),
	saveAllSettings(true),
	updated(false),
	panelIsMoving(false),
	panelMovingGroup(false),
	panelIsResizing(false),
	editedPanelId(-1),
	scalingPercentage(100)	
{
	btnConf.resize(GetTotalCnt());

	for (unsigned int i=0; i<btnConf.size(); i++)
	{
    	SetDefaultsForBtnId(i, btnConf[i]);
	}
}

int ProgrammableButtons::LoadFromJsonValue(const Json::Value &root)
{
	int status = -1;

	appVersion.FromJson(root["info"]);

	const Json::Value &btnConfJson = root["btnConf"];
	if (btnConfJson.type() == Json::arrayValue)
	{
        status = 0;
		for (int i=0; i<btnConfJson.size(); i++)
		{
			if (i >= btnConf.size())
			{
				break;
			}
			const Json::Value &btnJson = btnConfJson[i];
			ButtonConf &cfg = btnConf[i];
			cfg.FromJson(btnJson);
		}
	}

	{
		// dealing with transition to version 0.2.00.00 - new console configuration
		// trying to keep previous column width and number
		SettingsAppVersion ver0p2;
		ver0p2.FileVersionMS = 2;
		ver0p2.FileVersionLS = 0;

		// first version with programmable buttons on whole dialpad area
		SettingsAppVersion ver0p2p1;
		ver0p2p1.FileVersionMS = 2;
		ver0p2p1.FileVersionLS = 65535;

		if (appVersion < ver0p2)
		{
			if (appSettings.frmMain.pre0p2speedDialWidth.size() > 0)
			{
				int left = Sizes::FIRST_COLUMN_LEFT;
				int btnId = BASIC_PANEL_CONSOLE_BTNS;
				for (int widthId = 0; widthId < appSettings.frmMain.pre0p2speedDialWidth.size(); widthId++)
				{
					int width = appSettings.frmMain.pre0p2speedDialWidth[widthId];
					for (int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
					{
						btnConf[btnId].left = left;
						btnConf[btnId].width = width;
						btnId++;
					}
					left += width + Sizes::COLUMN_SEPARATION;
				}
				if (btnId < btnConf.size() - 1)
				{
					int offset = btnConf[btnId].left - left;
					for (btnId; btnId < btnConf.size(); btnId++)
					{
						btnConf[btnId].left -= offset;
					}
				}
				updated = true;
			}
		}

		if (appVersion < ver0p2p1)
		{
			SetInitialDialpad();
			for (unsigned int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
			{
				ButtonConf &cfg = btnConf[i];
				cfg.left += 184;
				cfg.parentId = 0;
			}
		}
	}

	return status;
}

int ProgrammableButtons::ReadFromString(AnsiString json)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	bool parsingSuccessful = reader.parse( json.c_str(), root );
	if ( !parsingSuccessful )
	{
		return 2;
	}

	if (root.type() != Json::objectValue)
	{
		return 3;
	}

	// assume this is newest configuration version if version is not present in text
	if (root["info"].type() == Json::nullValue)
	{
		SettingsAppVersion appVersion;
		appVersion.FromAppExe();
		appVersion.ToJson(root["info"]);
	}
	
	return LoadFromJsonValue(root);
}


int ProgrammableButtons::ReadFile(AnsiString name)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		std::ifstream ifs(name.c_str());
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strConfig, root );
		if ( !parsingSuccessful )
		{
			return 2;
		}
	}
	catch(...)
	{
		return 1;
	}
	return LoadFromJsonValue(root);
}

int ProgrammableButtons::Read(void)
{
	TimeCounter tc("Reading buttons configuration");
	assert(filename != "");

	if (ReadFile(filename) == 0)
	{
		{
			// regular log might not work yet - use OutputDebugString
			char s[100];
			snprintf(s, sizeof(s), "%s  %.3f ms\n", tc.getName(), tc.getTimeMs());
			OutputDebugString(s);
		}
		if (updated)
		{
			Write();
			updated = false;
		}

		return 0;
	}
	else
	{
		// earlier versions stored btn config in main file - try to read it
		AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
		int rc = ReadFile(asConfigFile);
		if (rc != 0)
		{
			SetInitialSettings();
		}
		// and write new, separate file (either default buttons or buttons from old "main" config,
		// if reading was successful)
		Write();
		return 0;
	}
	//notifyObservers();
}

int ProgrammableButtons::Write(void)
{
	TimeCounter tc("Writing buttons configuration");
	assert(filename != "");
    Json::Value root;
	Json::StyledWriter writer;

	{
		// update application version in settings
		appVersion.FromAppExe();
		appVersion.ToJson(root["info"]);
	}	

	// write buttons configuration
	Json::Value &jBtnConf = root["btnConf"];
	jBtnConf = Json::Value(Json::arrayValue);
	jBtnConf.resize(btnConf.size());
	for (unsigned int i=0; i<btnConf.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		Json::Value &jsonBtn = jBtnConf[i];
		cfg.ToJson(jsonBtn, saveAllSettings);
	}

	{
		//TimeCounter tc("Writing buttons configuration file");
		std::string outputConfig = writer.write( root );		// Debug: ~300 ms
		FILE *fp = fopen(filename.c_str(), "wb");
		if (fp)
		{
			int ret = fwrite(outputConfig.data(), outputConfig.size(), 1, fp);
			fclose(fp);
			if (ret != 1)
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}

	return 0;
}

void ProgrammableButtons::UpdateContacts(std::vector<UaConf::Contact> &contacts)
{
	contacts.clear();
	for (unsigned int btnId=0; btnId<btnConf.size(); btnId++)
	{
		const class ButtonConf &cfg = btnConf[btnId];
		if (cfg.number != "")
		{
			if (cfg.type == Button::BLF)
			{
				bool duplicate = false;
				for (unsigned int i=0; i<contacts.size(); i++)
				{
					if (contacts[i].user == cfg.number && contacts[i].sub_dialog_info)
					{
						contacts[i].btnIds.push_back(btnId);
						duplicate = true;
						break;
					}
				}
				if (duplicate)
				{
					continue;
				}

				UaConf::Contact contact;
				contact.user = cfg.number;
				contact.sub_dialog_info = true;
				contact.sub_dialog_info_expires = cfg.expires;
				contact.btnIds.push_back(btnId);
				contacts.push_back(contact);
			}
			else if (cfg.type == Button::PRESENCE)
			{
				bool duplicate = false;
				for (unsigned int i=0; i<contacts.size(); i++)
				{
					if (contacts[i].user == cfg.number && contacts[i].sub_presence)
					{
						contacts[i].btnIds.push_back(btnId);
						duplicate = true;
						break;
					}
				}
				if (duplicate)
				{
					continue;
				}

				UaConf::Contact contact;
				contact.user = cfg.number;
				contact.sub_presence = true;
				contact.sub_presence_expires = cfg.expires;				
				contact.btnIds.push_back(btnId);
				contacts.push_back(contact);
			}
		}
	}
}

void ProgrammableButtons::SetSaveAllSettings(bool state)
{
	if (saveAllSettings != state)
	{
		saveAllSettings = state;
		if (saveAllSettings && filename != "")
		{
        	Write();
		}
	}
}

void ProgrammableButtons::SetInitialSettings(void)
{
	ButtonConf *cfg;
	enum { LEFT = 190 };
	enum { WIDTH = 65 };
	enum { HEIGHT = 32 };

	for (unsigned int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
	{
		cfg = &btnConf[i];
		cfg->parentId = 0;
		cfg->left = LEFT;
		cfg->width = WIDTH;
		cfg->height = HEIGHT;
	}

	int top = 3;

	int btnId = 0;

	cfg = &btnConf[btnId++];
	cfg->caption = "    Redial";
	cfg->labelLeft = 0;
	cfg->type = Button::REDIAL;
	cfg->top = top;

	cfg = &btnConf[btnId++];
	cfg->caption = "    FLASH";
	cfg->labelLeft = 0;
	cfg->type = Button::DTMF;
	cfg->number = "R";
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[btnId++];
	cfg->caption = "Hold";
	cfg->type = Button::HOLD;
	cfg->imgIdle = "hold.bmp";
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[btnId++];
	cfg->caption = " Re-register";
	cfg->labelLeft = 4;
	cfg->type = Button::REREGISTER;
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[btnId++];
	cfg->captionLines = 2;
	cfg->caption = " Number";
	cfg->caption2 = " lookup";
	cfg->labelLeft = 4;
	cfg->labelTop = 3;
	cfg->label2Left = 4;
	cfg->label2Top = 17;	
	cfg->label2CenterHorizontally = false;	
	cfg->labelLeft = 4;
	cfg->type = Button::HTTP_QUERY;
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[btnId++];
	cfg->captionLines = 2;
	cfg->caption = " Right click";
	cfg->caption2 = " to edit btn";
	cfg->labelCenterVertically = false;
	cfg->labelLeft = 4;
	cfg->labelTop = 3;
	cfg->label2Left = 4;
	cfg->label2Top = 17;
	cfg->label2CenterHorizontally = false;
	top += HEIGHT;
	cfg->top = top;

	for (unsigned int i=btnId; i<BASIC_PANEL_CONSOLE_BTNS; i++)
	{
		cfg = &btnConf[i];
		//cfg->top = 5 * HEIGHT;
		cfg->visible = false;
	}

	SetInitialDialpad();
}

void ProgrammableButtons::SetInitialDialpad(void)
{
	enum { START_BTN = 200 };
	enum { BTN_CNT = 12 };
	enum { WIDTH = 40 };
	enum { HEIGHT = 40 };
	assert(START_BTN + BTN_CNT < GetTotalCnt());
	for (unsigned int i=START_BTN; i<START_BTN + BTN_CNT; i++)
	{
		ButtonConf &cfg = btnConf[i];
		cfg.parentId = 0;
		cfg.visible = true;
		cfg.width = WIDTH;
		cfg.height = HEIGHT;
		cfg.type = Button::DTMF;
		cfg.font.size = 22;
		cfg.labelCenterHorizontally = true;
		cfg.captionLines = 2;
		cfg.label2CenterHorizontally = true;
	}

	unsigned int id = START_BTN;
	ButtonConf *cfg;

	enum { LEFT1 = 4, LEFT2 = 51, LEFT3 = 99 };
	enum { TOP1 = 3, TOP2 = 46, TOP3 = 90, TOP4 = 134 };

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "1";
	cfg->left = LEFT1;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "2";
	cfg->caption2 = "ABC";
	cfg->left = LEFT2;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "3";
	cfg->caption2 = "DEF";
	cfg->left = LEFT3;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "4";
	cfg->caption2 = "GHI";
	cfg->left = LEFT1;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "5";
	cfg->caption2 = "JKL";
	cfg->left = LEFT2;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "6";
	cfg->caption2 = "MNO";
	cfg->left = LEFT3;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "7";
	cfg->caption2 = "PQRS";
	cfg->left = LEFT1;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "8";
	cfg->caption2 = "TUV";
	cfg->left = LEFT2;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "9";
	cfg->caption2 = "WXYZ";
	cfg->left = LEFT3;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "*";
	cfg->caption2 = "";
	cfg->left = LEFT1;
	cfg->top = TOP4;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "0";
	cfg->caption2 = "OPER";
	cfg->fontLabel2.size = cfg->fontLabel2.size - 1;
	cfg->left = LEFT2;
	cfg->top = TOP4;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "#";
	cfg->caption2 = "";
	cfg->left = LEFT3;
	cfg->top = TOP4;


}

TfrmButtonContainer* ProgrammableButtons::GetBtnContainer(int btnId)
{
    assert(btnId >= 0 && btnId < btnConf.size());
	const ButtonConf &cfg = btnConf[btnId];
	if (cfg.parentId <= ARRAY_SIZE(frmButtonContainers))
	{
		return frmButtonContainers[cfg.parentId];
	}
	else
	{
		assert(ARRAY_SIZE(frmButtonContainers) < BUTTON_CONTAINER_MAIN);
		return frmButtonContainers[BUTTON_CONTAINER_MAIN];
	}
}


void ProgrammableButtons::Create(TComponent* Owner,
		int scalingPercentage,
		CallbackClick callbackClick,
		CallbackMouseUpDown callbackMouseUpDown,
		CallbackSetKeepForeground callbackSetKeepForeground,
		CallbackRestartUa callbackRestartUa
		)
{
	assert(dmButtons == NULL);
	dmButtons = new TdmButtons(NULL);
	this->callbackClick = callbackClick;
	this->callbackSetKeepForeground = callbackSetKeepForeground;
	this->callbackMouseUpDown = callbackMouseUpDown;
	this->callbackRestartUa = callbackRestartUa;
	for (unsigned int i=0; i<ARRAY_SIZE(frmButtonContainers); i++)
	{
		TfrmButtonContainer *frm = frmButtonContainers[i];
		frm->OnKeyPress = FormKeyPress;
		frm->imgBackground->OnClick = containerBackgroundClick;
		frm->panelMain->OnClick = containerBackgroundClick;
		frm->movingFrame->OnClick = containerBackgroundClick;
		frm->lblCaption->OnClick = containerBackgroundClick;
	}
	for (unsigned int i=0; i<btnConf.size(); i++)
	{
		TProgrammableButton *panel = new TProgrammableButton(Owner, dmButtons->imgList, scalingPercentage);
		panel->Tag = i;
		//panel->AlignWithMargins = true;
		panel->Parent = GetBtnContainer(i)->GetButtonParent();
		const ButtonConf &cfg = btnConf[i];
		panel->SetConfig(cfg);
		panel->OnClick = SpeedDialPanelClick;
		panel->OnDblClick = SpeedDialPanelClick;
		panel->SetMouseUpDownCallback(OnPanelMouseUpDown);
		panel->UpdateCallbacks();
		btns.push_back(panel);
	}

	dmButtons->tmrMoving->OnTimer = this->tmrMovingTimer;
}

void ProgrammableButtons::Destroy(void)
{
#if 0	// buttons have own, different owners, possibly from different windows
	for (unsigned int i=0; i<btns.size(); i++)
	{
		delete btns[i];
	}
#endif
	btns.clear();
	if (dmButtons)
	{
		delete dmButtons;
		dmButtons = NULL;
	}
}

void __fastcall ProgrammableButtons::containerBackgroundClick(TObject *Sender)
{
	if (panelIsMoving || panelIsResizing)
	{
    	SpeedDialPanelClick(Sender);
	}
}

void __fastcall ProgrammableButtons::SpeedDialPanelClick(TObject *Sender)
{
	if (panelIsMoving)
	{
		panelIsMoving = false;

		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);

		container->imgBackground->Cursor = crDefault;

		TPoint P = container->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = btnConf[editedPanelId];	// copy
		ButtonConf initialCfg = cfg;
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

		if (panelMovingGroup)
		{
			int deltaX = cfg.left - initialCfg.left;
			int deltaY = cfg.top - initialCfg.top;
			for (int i=editedPanelId+1; i<btnConf.size(); i++)
			{
				const ButtonConf& other = btnConf[i];
				if (initialCfg.Contains(other))
				{
					ButtonConf newConf = other;
					newConf.left += deltaX;
					newConf.top += deltaY;
					ApplyButtonCfg(i, newConf);
				}
			}
		}
	}
	else if (panelIsResizing)
	{
		panelIsResizing = false;
		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
		container->imgBackground->Cursor = crDefault;

		TPoint P = container->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = btnConf[editedPanelId];	// copy
		cfg.width = P.x * 100/scalingPercentage - cfg.left;
		cfg.height = P.y * 100/scalingPercentage - cfg.top;

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

		if (cfg.width < MIN_PANEL_WIDTH)
			cfg.width = MIN_PANEL_WIDTH;
		if (cfg.height < MIN_PANEL_HEIGHT)
			cfg.height = MIN_PANEL_HEIGHT;		

		ApplyButtonCfg(editedPanelId, cfg);
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
		int id = panel->Tag;

		const ButtonConf &cfg = btnConf[id];
		if (cfg.type == Button::HOLD || cfg.type == Button::MUTE || cfg.type == Button::MUTE_RING)
		{
			panel->SetDown(!panel->GetDown());
		}

		callbackClick(id, panel);
	}
}

void ProgrammableButtons::OnPanelMouseUpDown(TProgrammableButton *btn)
{
	if (panelIsMoving || panelIsResizing)
	{
		return;
	}
	int id = btn->Tag;
	callbackMouseUpDown(id, btn);	
}

void ProgrammableButtons::UseContextMenu(bool state)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		btns[i]->PopupMenu = state ? dmButtons->popupPanel : NULL;
	}
}

void ProgrammableButtons::ApplyButtonCfg(int id, const ButtonConf &cfg)
{
	if (cfg != btnConf[id])
	{
		bool restartUa = false;
		if (cfg.UaRestartNeeded(btnConf[id]))
		{
			restartUa = true;
		}

		btnConf[id] = cfg;
		TProgrammableButton* panel = btns[id];
		panel->Parent = GetBtnContainer(id)->GetButtonParent();
		panel->SetConfig(cfg);
		if (cfg.type != Button::BLF)
		{
			panel->SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");	// make sure BLF icon is cleared
		}
		if (cfg.type != Button::PRESENCE)
		{
			panel->ClearPresenceState();			// clear icon
		}
		Write();

		if (restartUa && callbackRestartUa)
		{
        	callbackRestartUa();
		}

		Calls::OnButtonConfigChange();
	}
	else
	{
		// assign anyway - some fields may be unintentionally omited from != operator
		btnConf[id] = cfg;
		Write();
	}
}

void ProgrammableButtons::UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay)
{
	if (id < 0 || id >= btns.size())
		return;
	btns[id]->SetState((enum dialog_info_status)state, updateRemoteIdentity, (enum dialog_info_direction)direction, remoteIdentity, remoteIdentityDisplay);
}

void ProgrammableButtons::UpdatePresenceState(int id, int state, AnsiString note)
{
	if (id < 0 || id >= btns.size())
		return;
	btns[id]->SetPresenceState((enum presence_status)state, note);
}

void ProgrammableButtons::UpdateBtnState(Button::Type type, bool state)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == type)
		{
			btns[i]->SetDown(state);
		}
	}
}

void ProgrammableButtons::UpdateMwiState(int newMsg, int oldMsg)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == Button::MWI)
		{
			btns[i]->SetMwiState(newMsg, oldMsg);
		}
	}
}

void ProgrammableButtons::UpdateAutoAnswer(bool enabled, int sipCode, AnsiString sipReason)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == Button::AUTO_ANSWER_DND)
		{
			if (cfg.sipCode == sipCode && cfg.sipReason == sipReason.c_str() && enabled)
			{
				btns[i]->SetDown(true);
			}
			else
			{
				btns[i]->SetDown(false);
			}
		}
	}
}

void ProgrammableButtons::Edit(int id)
{
	ButtonConf cfg = btnConf[id];	// copy

	if (frmButtonEdit == NULL)
	{
		Application->CreateForm(__classid(TfrmButtonEdit), &frmButtonEdit);
	}

	AnsiString caption;
	caption.sprintf("Edit button #%02d", id);
	frmButtonEdit->Caption = caption;
	
	callbackSetKeepForeground(false);
	frmButtonEdit->ShowModal(&cfg, id);
	callbackSetKeepForeground(true);	

	if (frmButtonEdit->isConfirmed())
	{
		ApplyButtonCfg(id, cfg);
		TfrmButtonContainer *container = GetBtnContainer(id);
		container->Repaint();
	}
}

void ProgrammableButtons::Bring(int id, int left, int top, int containerId)
{
	ButtonConf cfg = btnConf[id];	// copy
	cfg.left = left;
	cfg.top = top;
	cfg.parentId = containerId;

	if (!cfg.visible)
	{
		if (Application->MessageBox("Make also button visible?", Application->Title.c_str(), MB_YESNO | MB_ICONQUESTION) == IDYES)
			cfg.visible = true;
	}

	ApplyButtonCfg(id, cfg);
	TfrmButtonContainer *container = GetBtnContainer(id);
	container->Repaint();
}

void ProgrammableButtons::Move(int id, bool moveGroup)
{
	editedPanelId = id;
	panelIsMoving = true;
	panelMovingGroup = moveGroup;
	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
	container->StartEditingButton(editedPanelId);

	// move mouse to top left button corner
	TPoint tp, tp2;
	tp.x = btnConf[id].left;
	tp.y = btnConf[id].top;
	tp2 = container->ClientToScreen(tp);
	::SetCursorPos(tp2.x, tp2.y);

	dmButtons->tmrMoving->Enabled = true;
}

void ProgrammableButtons::Resize(int id)
{
	editedPanelId = id;
	panelIsResizing = true;
	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
	container->StartEditingButton(editedPanelId);

	// move mouse to bottom right button corder
	TPoint tp, tp2;
	tp.x = btnConf[id].left + btnConf[id].width;
	tp.y = btnConf[id].top + btnConf[id].height;
	tp2 = container->ClientToScreen(tp);
	::SetCursorPos(tp2.x, tp2.y);

	dmButtons->tmrMoving->Enabled = true;
}

void ProgrammableButtons::UpdateAll(void)
{
	for (int id = 0; id < btnConf.size(); id++)
	{
		if (id >= btns.size())
			break;
		TProgrammableButton* panel = btns[id];
		panel->Parent = GetBtnContainer(id)->GetButtonParent();		
		panel->SetConfig(btnConf[id]);
	}
	for (unsigned int i=0; i<ARRAY_SIZE(frmButtonContainers); i++)
	{
		frmButtonContainers[i]->Repaint();
	}
}

void ProgrammableButtons::SetConfig(int btnId, const ButtonConf &conf)
{
	TProgrammableButton* btn = GetBtn(btnId);
	if (btn)
	{
		btn->Parent = GetBtnContainer(btnId)->GetButtonParent();		
		btn->SetConfig(conf);
	}
}

void __fastcall ProgrammableButtons::tmrMovingTimer(TObject *Sender)
{
	if (!panelIsMoving && !panelIsResizing)
	{
		dmButtons->tmrMoving->Enabled = false;
		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
		container->movingFrame->Visible = false;
		return;
	}

	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);

	{
		SHORT state = GetAsyncKeyState( VK_ESCAPE );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			EndEditing(container);
			return;
		}
	}

	{
		SHORT state = GetAsyncKeyState( VK_RETURN );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			SpeedDialPanelClick(NULL);
			return;
		}
	}

	{
		SHORT state = GetAsyncKeyState( VK_LEFT );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			UpdateCursorPos(-1, 0);
		}
	}
	{
		SHORT state = GetAsyncKeyState( VK_RIGHT );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			UpdateCursorPos(1, 0);
		}
	}
	{
		SHORT state = GetAsyncKeyState( VK_UP );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			UpdateCursorPos(0, -1);
		}
	}
	{
		SHORT state = GetAsyncKeyState( VK_DOWN );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			UpdateCursorPos(0, 1);
		}
	}


	const ButtonConf &cfg = btnConf[editedPanelId];

	TPoint P = container->ScreenToClient(Mouse->CursorPos);
	if (P.x < 0)
		P.x = 0;
	if (P.y < 0)
		P.y = 0;

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
		container->UpdateMovingFrame(left, top, cfg.width, cfg.height);
	}
	else if (panelIsResizing)
	{
		int width = P.x * 100/scalingPercentage - cfg.left;
		int height = P.y * 100/scalingPercentage - cfg.top;

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

		if (width < MIN_PANEL_WIDTH)
			width = MIN_PANEL_WIDTH;
		if (height < MIN_PANEL_HEIGHT)
			height = MIN_PANEL_HEIGHT;

		container->UpdateMovingFrame(cfg.left, cfg.top, width, height);
	}
}

void __fastcall ProgrammableButtons::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		TfrmButtonContainer *container = dynamic_cast<TfrmButtonContainer*>(Sender);
		EndEditing(container);
	}
}

void ProgrammableButtons::EndEditing(TfrmButtonContainer *container)
{
	if (panelIsMoving || panelIsResizing)
	{
		panelIsMoving = false;
		panelIsResizing = false;

		assert(container);
		container->imgBackground->Cursor = crDefault;
	}
}

void ProgrammableButtons::CopyConfig(int sourceBtnId)
{
	if (frmButtonCopy == NULL)
	{
		Application->CreateForm(__classid(TfrmButtonCopy), &frmButtonCopy);
	}
	frmButtonCopy->SetButtons(this, sourceBtnId);
	callbackSetKeepForeground(false);
	frmButtonCopy->ShowModal();
	callbackSetKeepForeground(true);

	if (frmButtonCopy->IsApplied())
	{
		UpdateAll();
	}
}

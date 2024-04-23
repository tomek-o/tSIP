//---------------------------------------------------------------------------
#pragma hdrstop

#include "ButtonContainers.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


TfrmButtonContainer* frmButtonContainers[BUTTON_CONTAINER__COUNT] = {0};

const char* GetButtonContainerName(enum ButtonContainerId id)
{
	switch (id)
	{
	case BUTTON_CONTAINER_DIALPAD:
		return "dialpad panel";
	case BUTTON_CONTAINER_MAIN:
		return "main area / console";
	case BUTTON_CONTAINER_CALL_PANEL:
		return "call panel (Call/Hangup)";
	case BUTTON_CONTAINER_TRAY_NOTIFIER:
		return "tray notifier window";
	default:
		return "???";
	}
}



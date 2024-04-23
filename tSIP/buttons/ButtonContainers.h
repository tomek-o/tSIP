//---------------------------------------------------------------------------

#ifndef ButtonContainersH
#define ButtonContainersH
//---------------------------------------------------------------------------

class TfrmButtonContainer;

enum ButtonContainerId {
	BUTTON_CONTAINER_DIALPAD = 0,
	BUTTON_CONTAINER_MAIN,
	BUTTON_CONTAINER_CALL_PANEL,
	BUTTON_CONTAINER_TRAY_NOTIFIER,

	BUTTON_CONTAINER__COUNT
};

static const enum ButtonContainerId BUTTON_CONTAINER__DEFAULT = BUTTON_CONTAINER_MAIN;

const char* GetButtonContainerName(enum ButtonContainerId id);

extern TfrmButtonContainer* frmButtonContainers[BUTTON_CONTAINER__COUNT];


#endif

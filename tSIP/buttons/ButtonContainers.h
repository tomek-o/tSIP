//---------------------------------------------------------------------------

#ifndef ButtonContainersH
#define ButtonContainersH
//---------------------------------------------------------------------------

#include "ProgrammableButtons.h"

class TfrmButtonContainer;
class TProgrammableButton;

extern TfrmButtonContainer* frmButtonContainers[1 + ProgrammableButtons::EXT_CONSOLE_COLUMNS];

TProgrammableButton* FindButton(int id);

#endif

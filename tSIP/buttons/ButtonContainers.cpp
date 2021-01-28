//---------------------------------------------------------------------------
#pragma hdrstop

#include "ButtonContainers.h"
#include "FormButtonContainer.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))


TfrmButtonContainer* frmButtonContainers[1 + ProgrammableButtons::EXT_CONSOLE_COLUMNS];

TProgrammableButton* FindButton(int id)
{
	for (int cid=0; cid<ARRAY_SIZE(frmButtonContainers); cid++)
	{
		TfrmButtonContainer *& container = frmButtonContainers[cid];
		if (container)
		{
			int start = container->GetStartBtnId();
			if (id < start)
				return NULL;
			if (id - start < container->GetBtnCnt())
				return container->GetBtn(id - start);
		}
	}
	return NULL;
}

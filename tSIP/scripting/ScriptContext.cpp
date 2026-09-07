//---------------------------------------------------------------------------

#pragma hdrstop

#include "ScriptContext.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

ScriptContext::ScriptContext(enum ScriptSource srcType, int srcId):
	srcType(srcType),
	srcId(srcId)
{
}

//---------------------------------------------------------------------------

#ifndef GlobalsH
#define GlobalsH
//---------------------------------------------------------------------------

class History;
extern History history;

class Contacts;
extern Contacts contacts;

class HotKeys;
extern HotKeys hotKeys;

class ProgrammableButtons;
extern ProgrammableButtons buttons;

/** \brief Process exit code, set via Lua's SetExitCode(), returned from WinMain()
*/
extern int applicationExitCode;

#endif

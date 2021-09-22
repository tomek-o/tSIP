/** \file
	\brief Command-line parameters handling
*/
#ifndef CommandLineH
#define CommandLineH

#include <System.hpp>

class CommandLine
{
private:
	CommandLine(void);
	CommandLine(const CommandLine& source) {};
	CommandLine& operator=(const CommandLine&);
	static CommandLine instance;
public:
	static CommandLine& Instance(void) {
		return instance;
	}
	/** \brief Process command-line parameters passed to executable
		\return non-zero if command-line was forwarded to another app instance
	*/
	int Process(void);
	enum { MAX_CMD_PARAM_LEN = 1024 };	
	void Execute(char* buf, int paramcnt);	
	enum Action {
		ACTION_NONE = 0,
		ACTION_CALL,
		ACTION_HANGUP,
		ACTION_ANSWER,
		ACTION_SHOWWINDOW,	///< bring application window to front (possibly from tray)
		ACTION_APP_QUIT,
		ACTION_PROGRAMMABLE_BTN,	///< simulating pressing one of the programmable buttons
		ACTION_RECORD_START,
		ACTION_RECORD_PAUSE,
		ACTION_SCRIPT,
		ACTION_SCRIPT_FILE
	} action;
	AnsiString asTarget;
	int programmableBtnId;
	AnsiString script;
	AnsiString scriptFile;

	static AnsiString GetProfileDir(void);
};

#endif

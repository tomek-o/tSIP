/** \file
	\brief Interface description for phone device

	This file should be shared by main application and device dll.
	You MUST declare _EXPORTING macro before including this file in dll sources.
*/

#ifndef PhoneH
#define PhoneH
//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#if defined(_MSC_VER)
#	define CALL_CONV
#else
#	define CALL_CONV __stdcall
#endif

/** \def DECLARE_FN
    \brief Declare function or consistent function + function pointer set depending on _EXPORTING
*/
#ifdef _EXPORTING
#define DECLARE_FN(type, fn, ...) __declspec(dllexport) type CALL_CONV fn(__VA_ARGS__)
#else
#define DECLARE_FN(type, fn, ...) __declspec(dllimport) type __stdcall fn(__VA_ARGS__); \
	typedef type (__stdcall *pf##fn)(__VA_ARGS__)
#endif

/** \brief Dll interface version, used to dismiss outdated libraries
*/
enum { DLL_INTERFACE_MAJOR_VERSION = 1 };
/** \brief Dll interface version, used to dismiss outdated libraries
*/
enum { DLL_INTERFACE_MINOR_VERSION = 0 };

/** \brief Holds Dll interface version
*/
struct S_PHONE_DLL_INTERFACE
{
	int majorVersion;   ///< major part of interface version
	int minorVersion;   ///< minor part of interface version
};

/** \brief State of connection between PC and data acquisition device
*/
enum E_CONNECTION_STATE
{
	DEVICE_DISCONNECTED = 0,
	DEVICE_CONNECTING,
	DEVICE_CONNECTED,
	DEVICE_DISCONNECTING
};

enum E_KEY
{
	KEY_0 = 0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_STAR,
	KEY_HASH,
	KEY_HOOK,
	KEY_OK,
	KEY_C,
	KEY_UP,
	KEY_DOWN
};

struct S_PHONE_CAPABILITIES;
struct S_PHONE_SETTINGS;

///////////////////////////////////////////////////////////////////////////////
// CALLBACKS
///////////////////////////////////////////////////////////////////////////////

/** \brief Called to generate log in parent application
    \param cookie Holds internal data of callee (upper layer)
    \param szText Text to add to log
*/
typedef void (__stdcall *CALLBACK_LOG)(void *cookie, const char *szText);
/** \brief Called on connection/disconnection of device
	\param cookie Holds internal data of callee (upper layer)
	\param state Current state of device (\sa E_CONNECTION_STATE)
	\param szMsgText Additional text (if applicable)
*/
typedef void (__stdcall *CALLBACK_CONNECT)(void *cookie, int state, const char *szMsgText);
/** \brief Update key state
	\param keyCode key type
	\param state key up (=0) / down (=1) state
*/
typedef void (__stdcall *CALLBACK_KEY)(void *cookie, int keyCode, int state);
/** \brief Start RTP streaming (paging), optional callback
	\param target IP address + port (e.g. 192.168.1.10:4000) for streaming
	\param filename name of the file to stream; if string is empty default audio source is used
	\param codecname name of the codec to be used in transmission (not implemented yet)
	\note uses cookie from "standard" callbacks
	\return 0 on success
*/
typedef int (__stdcall *CALLBACK_PAGING_TX)(void *cookie, const char* target, const char* filename, const char* codecname);
/** \brief Clear dial edit
	\note uses cookie from "standard" callbacks
*/
typedef void (__stdcall *CALLBACK_CLEAR_DIAL)(void *cookie);
/** \brief Get description for the number (phonebook, etc.)
	\param description buffer to store description (NULL-terminated, truncated if necessary)
	\param descriptionSize size of descripton buffer in bytes
	\return 0 on success
*/
typedef int (__stdcall *CALLBACK_GET_NUMBER_DESCRIPTION)(void *cookie, const char* number, char* description, int descriptionSize);

/** \brief Set value for variable with specified name; variables are shared by scripts and plugins
	\param name variable name
	\param value value to set (string)
*/
typedef int (__stdcall *CALLBACK_SET_VARIABLE)(void *cookie, const char* name, const char* value);
/** \brief Clear variable
	\param name variable name
*/
typedef int (__stdcall *CALLBACK_CLEAR_VARIABLE)(void *cookie, const char* name);

/** \brief Push value to queue
	\param name queue name
	\param value string value
	\return 0 on success
*/
typedef int (__stdcall *CALLBACK_QUEUE_PUSH)(void *cookie, const char* name, const char* value);
/** \brief Get and remove value from queue
	\param name queue name
	\param value pointer to buffer to receive value
	\param valueSize buffer size
	\return 0 on success (queue existed and queue not empty)
*/
typedef int (__stdcall *CALLBACK_QUEUE_POP)(void *cookie, const char* name, char* value, unsigned int valueSize);
/** \brief Clear queue completely (delete queue)
	\param name queue name
	\return 0 on success (queue existed)
*/
typedef int (__stdcall *CALLBACK_QUEUE_CLEAR)(void *cookie, const char* name);
/**	\brief Get number of elements in the queue
	\param name queue name
	\return number of elements in queue; 0 if queue does not exist
*/
typedef int (__stdcall *CALLBACK_QUEUE_GET_SIZE)(void *cookie, const char* name);
/** \brief Run Lua script - asynchronously, main/GUI thread of the application
	\param script script to execute
	\return 0 on success (script enqueued)
*/
typedef int (__stdcall *CALLBACK_RUN_SCRIPT_ASYNC)(void *cookie, const char* script);
/** \brief Application -> plugin (NOT plugin -> app like others!) callback declaration for function to be called when registered menu item is clicked
*/
typedef void (__stdcall *CALLBACK_MENU_ITEM_CLICK)(void *menuItemClickCookie);
/** \brief Add new item to tray menu from plugin
	\param parent opaque handle to menu item parent; NULL to add to root menu, can also used previously added menu item handle to create cascade menu
	\param caption text to put on new menu item
	\param lpMenuItemClickFn function to be called when menu item is clicked; NULL can be used when creating cascade menu structure
	\param menuItemClickCookie cookie to be passed back when menuItemClickCookie is called
	\return handle to new menu item
*/
typedef void* (__stdcall *CALLBACK_ADD_TRAY_MENU_ITEM)(void *cookie, void* parent, const char* caption, CALLBACK_MENU_ITEM_CLICK lpMenuItemClickFn, void *menuItemClickCookie);
/** \brief Set application status
	\param id status id to add or replace
	\param priority lower value -> higher priority
	\param text text to set
	\return 0 on success
*/
typedef int (__stdcall *CALLBACK_SET_APP_STATUS)(void *cookie, const char* id, int priority, const char* text);
///////////////////////////////////////////////////////////////////////////////
// EXPORTED/IMPORTED FUNCTION SET
///////////////////////////////////////////////////////////////////////////////

/** \brief Get dll interface version to check for compatibility
*/
DECLARE_FN(void, GetPhoneInterfaceDescription, struct S_PHONE_DLL_INTERFACE*);

/** \brief Get initial settings, either default or from config file/registry/etc.
*/
DECLARE_FN(int, GetPhoneSettings, struct S_PHONE_SETTINGS*);

/** \brief Save settings to config file/registry/etc.
*/
DECLARE_FN(int, SavePhoneSettings, struct S_PHONE_SETTINGS*);

/** \brief Set dll callbacks (to i.e. send new data from dll to main application)
*/
DECLARE_FN(void, SetCallbacks, void *cookie, CALLBACK_LOG lpLog, CALLBACK_CONNECT lpConnect, CALLBACK_KEY lpKey);

/** \brief Get detailed info about device capabilities
*/
DECLARE_FN(void, GetPhoneCapabilities, struct S_PHONE_CAPABILITIES **caps);

/** \brief Show dll specific settings window (if available)
*/
DECLARE_FN(void, ShowSettings, HANDLE parent);

/** \brief Connect to device
*/
DECLARE_FN(int, Connect, void);

/** \brief Disconnect from device
*/
DECLARE_FN(int, Disconnect, void);

DECLARE_FN(int, SetRegistrationState, int state);

DECLARE_FN(int, SetCallState, int state, const char* display);

DECLARE_FN(int, Ring, int state);

/*
 * PAGING (transmitter) - added 2016.01
 */
DECLARE_FN(void, SetPagingTxCallback, CALLBACK_PAGING_TX lpPagingTx);
DECLARE_FN(int, SetPagingTxState, int state);

DECLARE_FN(void, SetClearDialCallback, CALLBACK_CLEAR_DIAL lpClearDial);

DECLARE_FN(void, SetGetNumberDescriptionCallback, CALLBACK_GET_NUMBER_DESCRIPTION lpGetNumberDescription);

/** \brief Send message (text) to DLL
*/
DECLARE_FN(int, SendMessageText, const char* text);

DECLARE_FN(void, SetSetVariableCallback, CALLBACK_SET_VARIABLE lpFn);

DECLARE_FN(void, SetClearVariableCallback, CALLBACK_CLEAR_VARIABLE lpFn);

DECLARE_FN(void, SetQueuePushCallback, CALLBACK_QUEUE_PUSH lpFn);

DECLARE_FN(void, SetQueuePopCallback, CALLBACK_QUEUE_POP lpFn);

DECLARE_FN(void, SetQueueClearCallback, CALLBACK_QUEUE_CLEAR lpFn);

DECLARE_FN(void, SetQueueGetSizeCallback, CALLBACK_QUEUE_GET_SIZE lpFn);

DECLARE_FN(void, SetRunScriptAsyncCallback, CALLBACK_RUN_SCRIPT_ASYNC lpFn);

/** \note Tray menu items may be added only from GUI thread context, e.g. from Connect() function
*/
DECLARE_FN(void, SetAddTrayMenuItemCallback, CALLBACK_ADD_TRAY_MENU_ITEM lpFn);

DECLARE_FN(void, SetCallbackSetAppStatus, CALLBACK_SET_APP_STATUS lpFn);

/** \brief Called on audio device error (e.g. end of wave file)
*/
DECLARE_FN(void, SetAudioError, void);

/** \brief Pass information about profile directory to DLL
*/
DECLARE_FN(void, SetProfileDir, const char*);

#ifdef __cplusplus
}
#endif

#endif

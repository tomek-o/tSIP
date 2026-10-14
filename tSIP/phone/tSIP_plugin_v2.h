/** \file
	\brief tSIP plugin interface v2 - plain C header shared by tSIP and plugins

	See tSIP/_doc/plugin_interface_v2.md for design and compatibility rules.

	Summary:
	- plugin DLL exports single function: tSipPluginInit (TSIP_PLUGIN_INIT_NAME),
	- host passes table of host functions (TSIP_HOST_API) and table to be filled
	  by plugin (TSIP_PLUGIN_API),
	- both tables start with size field and may only grow at the end:
		- host zero-fills TSIP_PLUGIN_API and sets its size to the size known by
		  the host; plugin fills only entries that fit (TSIP_HAS) and does not
		  change the size,
		- plugin uses host entries only if they fit host->size (TSIP_HAS),
	- events (host -> plugin) and generic calls (plugin -> host) use UTF-8
	  JSON strings,
	- memory allocated by host is released by host (host->Free), never by
	  plugin's free()/delete (different C runtime / heap),
	- only 32-bit int and pointers in structures (no enums, no bool),
	- calling convention: __stdcall.

	Threading:
	- plugin functions (Start, Stop, Shutdown, OnEvent, ShowSettings) are
	  called from the main (GUI) thread,
	- host functions may be called from any thread; Invoke called from a thread
	  other than the main thread blocks until request is executed by the main
	  thread or until timeout,
	- plugin must not block in OnEvent / Start / Stop for long time.

	Lifecycle:
	- tSipPluginInit: fill plugin table only - no threads, no I/O-heavy work,
	- Start: called when plugin is enabled (and after application start),
	- Stop: plugin must stop and join its threads here,
	- Shutdown: called right before unloading DLL.

	Compatibility rules:
	- existing entries are never removed, reordered or changed,
	- changed signature = new entry appended at the end,
	- JSON: additive changes only, tolerant reader (ignore unknown fields).
*/

#ifndef TSIP_PLUGIN_V2_H
#define TSIP_PLUGIN_V2_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSIP_V2_CALL __stdcall

/** \brief Name of the function exported by v2 plugin */
#define TSIP_PLUGIN_INIT_NAME "tSipPluginInit"

/** \brief Check if table entry is present (fits table size known by other side)
	\param api pointer to TSIP_HOST_API / TSIP_PLUGIN_API
	\param member member name
*/
#define TSIP_HOST_HAS(api, member) \
	((api) != NULL && (api)->size >= (int)(offsetof(struct TSIP_HOST_API, member) + sizeof((api)->member)))
#define TSIP_PLUGIN_HAS(api, member) \
	((api) != NULL && (api)->size >= (int)(offsetof(struct TSIP_PLUGIN_API, member) + sizeof((api)->member)))

/** \brief Return codes */
enum {
	TSIP_OK = 0,
	TSIP_ERR_GENERIC = -1,
	TSIP_ERR_INVALID_ARG = -2,
	TSIP_ERR_UNKNOWN_METHOD = -3,
	TSIP_ERR_TIMEOUT = -4,
	TSIP_ERR_NOT_AVAILABLE = -5
};

/** \brief Functions provided by host (tSIP) to plugin

	Pass `cookie` as first argument to every host function.
*/
struct TSIP_HOST_API
{
	int size;			///< sizeof(TSIP_HOST_API) as known by host
	void *cookie;		///< opaque, identifies plugin instance in host

	/** \brief Add text to tSIP log */
	void (TSIP_V2_CALL *Log)(void *cookie, const char *text);

	/** \brief Generic synchronous call
		\param method method name, see list below
		\param jsonParams JSON object with parameters (UTF-8), may be NULL
		\param jsonResult receives JSON result allocated by host (UTF-8) or NULL;
			may be NULL if result is not needed; must be released with Free();
			may be set also on error - e.g. {"error":"..."} with details
		\return TSIP_OK or error code; TSIP_ERR_NOT_AVAILABLE if called from
			plugin thread while host executes plugin Stop() (call is cancelled
			instead of waiting for main thread, so Stop() can join plugin threads)

		Accounts: "accountId" is 0-based index of account in settings; methods with optional
		"accountId" use current account (selected by user / ACCOUNT button) if it is not given;
		not existing / disabled account -> TSIP_ERR_INVALID_ARG with {"error":"..."}

		Methods:
		- "GetAppInfo": {} -> {"exeName", "profileDir", "pluginInterface"}
		- "GetCalls": {} -> {"calls":[{"uid","incoming","state","stateName","peer","peerName",
			"accountId"}]}; accountId: account used by call (incoming: account that received it)
		- "Call": {"target":"...", "headers":{"Name":"value",...}, "accountId":N} -> {"uid":N};
			accountId (optional): account used for call; headers (optional):
			extra header lines for INVITE, added after lines from settings; header names generated
			by SIP stack (Via, From, To, Call-ID, CSeq, Contact, Route, Content-*, ...) are rejected
		- "Answer": {"uid":N} (uid optional: current call)
		- "Hangup": {"uid":N, "sipCode":486, "reason":"Busy Here"} (all optional)
		- "RunScript": {"script":"...", "async":false}
			executes Lua script in main thread; async = true: queue and return
			immediately; synchronous call: execution error (syntax / runtime) is
			not shown as dialog, returned as TSIP_ERR_GENERIC with {"error":"..."};
			synchronous result: {"return":"values returned by script (tab-separated)",
			"output":"print() output"} (fields present if not empty; "output" also on error)
		- "GetRegistration": {} -> {"accounts":[{"accountId","name","user","server","enabled",
			"current","registered", optional (if known): "state","stateName"}], "currentAccountId"}
			- all configured accounts
		- "SetCurrentAccount": {"accountId":N} -> {"currentAccountId"} - account used by default
			for outgoing calls / messages
		- "GetHistory": {"limit":20, "filter":"all|incoming|outgoing|missed", "search":"..."}
			(all optional) -> {"entries":[{"timestamp","incoming","missed","duration",
			"uri","peerName", optional: "contactName","paiUri","paiPeerName","lastSipCode","reason",
			"accountId"}]},
			newest first; limit up to 200
		- "RestartPlugin": {} - host calls plugin Stop() and Start() later (asynchronously,
			from main thread, after Invoke returns), e.g. to apply settings changed in
			ShowSettings(); Invoke calls pending at that time are cancelled as for normal Stop()
		- "SendDtmf": {"uid":N, "digits":"123#"} (uid optional; digits 0-9*#ABCD, R = flash, max 32)
		- "Hold": {"uid":N, "state":true} (uid optional) -> {"hold":bool} (actual state)
		- "Mute": {"uid":N, "state":true} (uid optional) -> {"mute":bool}
		- "Sidetone": {"uid":N, "state":true, "level":50} (uid, level optional) -> {"sidetone":bool,
			"level":N}; transmitted audio mixed into local playback (not recorded), level in percent 0..200
		- "StartRecording": {"uid":N} (uid optional) -> {"recording":bool, "file":"..."};
			uses recording settings from configuration (TSIP_ERR_NOT_AVAILABLE if disabled)
		- "SendMessage": {"target":"...", "text":"...", "accountId":N} - SIP MESSAGE, sent through message
			window (shown to user); delivery status is not returned
		- "GetMessages": {"peer":"...", "limit":20} -> {"messages":[{"timestamp","incoming",
			"contentType","body"}]}, oldest first
		- "ListAudioDevices": {} -> {"modules":[{"module","in":[...],"out":[...]}],
			"configured":{"input":{"module","device" or "file"},"output":{...}}}
		- "SetAudioInput" / "SetAudioOutput": {"uid":N, "module":"wasapi", "device":"..."}
			or {"uid":N, "configured":true} (uid optional) - switch audio of the call;
			device has to be on the list from ListAudioDevices
		- "PlayAudioFile": {"uid":N, "file":"name.wav"} (uid optional) - audio input of the
			call switched to WAV file from <profile>\audio or <profile> (name only, no path)
			-> {"uid","file","audioErrorCount"}; end of file increments audioErrorCount;
			end of this file never disconnects the call (regardless of settings)
		- "GetCallAudioState": {"uid":N} -> {"uid","audioErrorCount","rxLevel"}; rxLevel: peak
			absolute value of received audio samples over last ~100 ms (0..32767, linear)
		- "GetCallInvite": {"uid":N} (uid optional) -> {"uid","incoming","invite"}; invite: full text
			of initial received INVITE (raw SIP message), empty for outgoing call
		- "SendCustomRequest": {"method":"OPTIONS", "uri":"sip:..."} (out-of-dialog) or
			{"method":"INFO", "uid":N} (within call dialog, uid optional = current call), optional
			"headers":{"Name":"value"}, "accountId":N (out-of-dialog only; request within call uses
			account of the call) -> {"requestUid":N}; INVITE, ACK, CANCEL, BYE, PRACK, UPDATE
			are not allowed; requests are shared with Lua (GetCustomRequestReply etc.)
		- "GetCustomRequest": {"requestUid":N, "delete":true} -> {"requestUid", "haveReply",
			"transportError", "sipStatusCode", "reply"}; last 3 only if haveReply (transportError 0:
			SIP reply received, otherwise e.g. timeout); delete: forget request if reply is received
		- "DeleteCustomRequest": {"requestUid":N} - forget request (e.g. plugin stopped waiting)
		- "Transfer": {"uid":N, "target":"..."} (uid optional) - blind transfer
		- "SearchContacts": {"query":"...", "limit":20} -> {"contacts":[{"name",
			"numbers":[...], optional: "company","note"}]}; case-insensitive substring match
	*/
	int (TSIP_V2_CALL *Invoke)(void *cookie, const char *method, const char *jsonParams, char **jsonResult);

	/** \brief Release memory allocated by host (e.g. Invoke result) */
	void (TSIP_V2_CALL *Free)(void *cookie, void *ptr);

	/*
	 * Variables and queues - shared with Lua scripts and v1 plugins.
	 * Executed directly (thread-safe, no marshalling to main thread), suitable
	 * for frequent use.
	 * Values are passed unchanged as opaque byte strings (no UTF-8 / ANSI
	 * conversion), so they can carry e.g. JSON or other data without loss.
	 */

	/** \brief Set variable (visible in Lua as GetVariable) */
	int (TSIP_V2_CALL *SetVariable)(void *cookie, const char *name, const char *value);

	/** \brief Get variable
		\param value receives value allocated by host - release with Free()
		\return TSIP_OK or TSIP_ERR_NOT_AVAILABLE if variable is not set
	*/
	int (TSIP_V2_CALL *GetVariable)(void *cookie, const char *name, char **value);

	int (TSIP_V2_CALL *ClearVariable)(void *cookie, const char *name);

	/** \brief Add value to the end of the queue (queue is created if needed) */
	int (TSIP_V2_CALL *QueuePush)(void *cookie, const char *name, const char *value);

	/** \brief Take value from the front of the queue
		\param value receives value allocated by host - release with Free()
		\return TSIP_OK or TSIP_ERR_NOT_AVAILABLE if queue is empty / does not exist
	*/
	int (TSIP_V2_CALL *QueuePop)(void *cookie, const char *name, char **value);

	/** \return number of elements in queue (0 if queue does not exist) */
	int (TSIP_V2_CALL *QueueGetSize)(void *cookie, const char *name);

	/** \brief Delete queue */
	int (TSIP_V2_CALL *QueueClear)(void *cookie, const char *name);

	/* new entries: append only */
};

/** \brief Functions provided by plugin to host

	Host zero-fills this structure and sets size; plugin fills entries
	that fit (TSIP_PLUGIN_HAS). Entries left NULL are optional.
*/
struct TSIP_PLUGIN_API
{
	int size;			///< sizeof(TSIP_PLUGIN_API) as known by host - do not change

	/** \brief Plugin description as JSON (UTF-8), static string owned by plugin,
		e.g. {"name":"...","version":"...","description":"..."} */
	const char* (TSIP_V2_CALL *GetMetadata)(void);

	/** \brief Plugin enabled / application started */
	int (TSIP_V2_CALL *Start)(void);

	/** \brief Plugin disabled / application closing - stop and join threads */
	int (TSIP_V2_CALL *Stop)(void);

	/** \brief Called right before DLL is unloaded */
	void (TSIP_V2_CALL *Shutdown)(void);

	/** \brief Event from host
		\param type event type
		\param json event data as JSON object (UTF-8)

		Events:
		- "call_state": {"uid","state","stateName","incoming","peer","peerName"}
		- "registration": {"accountId","state","stateName"}
		- "mwi": {"accountId","newMsg","oldMsg"}
		More event types and fields may be added - ignore unknown ones.
	*/
	void (TSIP_V2_CALL *OnEvent)(const char *type, const char *json);

	/** \brief Show plugin settings window (optional) */
	void (TSIP_V2_CALL *ShowSettings)(void *parentHwnd);

	/* new entries: append only */
};

/** \brief Plugin entry point (exported as TSIP_PLUGIN_INIT_NAME)
	\return 0 if plugin accepts host / interface
*/
typedef int (TSIP_V2_CALL *pfTSipPluginInit)(const struct TSIP_HOST_API *host, struct TSIP_PLUGIN_API *plugin);

#ifdef __cplusplus
}
#endif

#endif /* TSIP_PLUGIN_V2_H */

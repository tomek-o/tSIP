/** \file
	\brief tSIP roadmap
*/

/** \page roadmap Roadmap

Bunch of things to do in future releases. No particular order:

- multiple accounts, even if only single active at the time
- UDP transport keepalive
- context help
- SIP message graphs

- some things from baresip
	- audio_loop_test
	- call_debug

- misleading "Failed to start application" status when no valid account is configured


- DTMF local playback


Call recording:
- fixme: possible hazards when starting recording
- other output formats, compression
- settings: record incoming/outgoing
- semi-automatic record mode: record call by default, delete file if not "saved" by user
- UID for file names to avoid conflicts with very short (<1 s) recordings


- new button type: auto answer / DND with specified answer code

- hotkeys: move focus to transfer textbox

- example: link handling from mail client
- ICE
- select reject code: 486 Busy (other devices keep ringing) or 603 Decline (all devices stop ringing)
- hotkeys or speed dial: volume
- encrypt password / password prompt
- contacts: master/detail view
- contact groups
- different ringtones for different contacts
- detect: no input snd dev
- handle "Call completed elsewhere" disconnection reason - do not add this call to history (or add to history with special mark)
- call history: master/detail view, copying number to clipboard, adding to phonebook
- number processing / "LCR" / dialplan

- user definable Call/Hangup buttons (horizontal configurable button container?)
- buttons: Dialpad/Contacts/History and equivalent hotkey actions replacing existing tab row
- combine regular number edit and transfer number edit into one 

- settings: time for hotkey anti-repeat timer

- separate icon for recorviewer

- BLF list

- BLF barge in

- visual appearance config for programmable button (text color, font, background color)

- gnome3 and wine: call notifications not working? (LS)

- warn user if wave file source is selected but source file does not exist

- translation mechanism

- remote phonebook (XML)

- supporting animated GIFs or frame-by-frame animations in BLFs

- CDR plugin: storing call details to CSV or some kind of database (mysql)

- contacts plugin: fetch number description from other data source

libre version 0.4.17 contains an improvement for the SIP-stack
when multiple SRV-records are present in the domain


> 3.  The voice quality is slightly fuzzy/muddy.  I've experienced this with
other open source SIP phones as well - everything except Zoiper.  Voices are
noticably clearer when using Zoiper - and all apps are configured to use
G711u, so it's an equal test across apps.  Do you have any idea why this is
and whether it can be addressed?
- test speex_pp module - add RX dir?

- blinking tray icon while in call

- aufile: accept mp3 files

- "BLF" p2p: subscription server

button: mouse down event

> I was wondering if it is possible to pause or stop the recording while still on the call?

Lua: get BLF button state => route incoming call to selected (idle) extension

- audible DTMF keypad tones during the call and while dialing: as opt-in options, PlaySound, resources, ASYNC?

- sine wave audio input device

- provisioning for buttons and phonebook

- more command-line control: show/hide, dtmf, hold/unhold, start recording

TXPManifest

ping plugin: add context menu to history


- ua_init: add option to disable udp/tcp transport (now both UDP/TCP are always enabled)

- 1000 BLFs (or BLFlist?), column paging

- RTP stats: call quality testing



DEBUG_INFO: w³¹cznik

aufile: required S16LE, 1ch

	
latency tracker

tSIP: help on button types (+ images)
tSIP: directory import from vcard
tSIP: verify wave file format when selecting for wave source audio device
tSIP: "zawieszenie" przy po³¹czeniu przychodz¹cym gdy brak urz¹dzenia audio (?)
tSIP: RecordViewer: play multiple files one by one (multiple selection list)
multiple accounts; disable / set as default for outgoing
lepsze informowanie o problemach: brak kompatybilnego kodeka, brak urz¹dzenia audio
RecordViewer: keep sorting column/dir after restart

tSIP scripting: help for commands (factory pattern)
tSIP scripting: Unregister, Reregister
luaopen_io(lua);


plugin interface: showsettings

tSIP plugin: logout from group on inactivity (mouse, keyboard)

tSIP scripting: examples for os: os.clock(), date(), difftime(), execute(), 
tSIP scripting: file I/O example (report call quality after each call)

BUG: halt if audio device (mic with autosensing) is detached during call - winwave


FIXME: btn reregister not always working correctly, depending on current registration state (?)

www: scripting with javascript

update Lua to 5.3.x, remove CVS tags from Lua source

- wrong BLF remote identity displayed with FreePBX after call confirmed + call waiting (two dialogs) info; call confirmed probably without remote identity
(=> multiple independent dialogs in dialog-info, handling of missing remote identity specific to dialog)

*/



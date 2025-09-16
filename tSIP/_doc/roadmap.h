/** \file
	\brief tSIP roadmap
*/

/** \page roadmap Roadmap

Bunch of things to do in future releases.
No particular order.
Outdated / not the only one TODO list.

- multiple accounts, even if only single active at the time
- UDP transport keepalive
- context help
- SIP message graphs

- some things from baresip
	- audio_loop_test
	- call_debug

- misleading "Failed to start application" status when no valid account is configured

- DTMF local playback

- example: link handling from mail client
- ICE
- hotkeys or speed dial: volume
- encrypt password
- contacts: master/detail view
- contact groups
- different ringtones for different contacts
- detect: no input snd dev
- call history: master/detail view, adding to phonebook
- number processing / "LCR" / dialplan

- buttons: Dialpad/Contacts/History and equivalent hotkey actions replacing existing tab row
- combine regular number edit and transfer number edit into one 

- settings: time for hotkey anti-repeat timer

- BLF list

- BLF barge in

- shared line appearance, RFC7463

- supporting animated GIFs or frame-by-frame animations in BLFs

- CDR plugin: storing call details to CSV or some kind of database (mysql)

- contacts plugin: fetch number description from other data source

libre version 0.4.17 contains an improvement for the SIP-stack
when multiple SRV-records are present in the domain


> 3.  The voice quality is slightly fuzzy/muddy.  I've experienced this with other open source SIP phones as well - everything except Zoiper.  Voices are
noticably clearer when using Zoiper - and all apps are configured to use G711u, so it's an equal test across apps.  Do you have any idea why this is
and whether it can be addressed?
- test speex_pp module - add RX dir?

- blinking tray icon while in call

- "BLF" p2p: subscription server

button: mouse down event

Lua: get BLF button state => route incoming call to selected (idle) extension

- audible DTMF keypad tones during the call and while dialing: as opt-in options, PlaySound, resources, ASYNC?

ping plugin: add context menu to history


- ua_init: add option to disable udp/tcp transport (now both UDP/TCP are always enabled)

- 1000 BLFs (or BLFlist?), column paging

- RTP stats: call quality testing

DEBUG_INFO: w��cznik

aufile: required 1ch

tSIP: help on button types (+ images)
tSIP: directory import from vcard
tSIP: verify wave file format when selecting for wave source audio device
tSIP: RecordViewer: play multiple files one by one (multiple selection list)
multiple accounts; disable / set as default for outgoing
RecordViewer: keep sorting column/dir after restart

plugin interface: showsettings

tSIP plugin: logout from group on inactivity (mouse, keyboard)

tSIP scripting: examples for os: os.clock(), date(), difftime(), execute(), 
tSIP scripting: file I/O example (report call quality after each call)

update Lua to 5.3.x, remove CVS tags from Lua source

- wrong BLF remote identity displayed with FreePBX after call confirmed + call waiting (two dialogs) info; call confirmed probably without remote identity
(=> multiple independent dialogs in dialog-info, handling of missing remote identity specific to dialog)

TODO: TLS not working (UDP used instead) when outbound proxy is specified?

CRM application communicating through TCPServer plugin

- H.264 config: profile
- video recording
- direction selection (sendonly/recvonly/sendrecv)
- VP8, VP9 - could ffmpeg be used?
- RTSP as video source - needs testing

Bug (untested yet, but quite likely): bad audio quality after changing codec
from G.711a to G.722 during the call after re-INVITE from FreePBX 16. Re-INVITE
was triggered by attended transfer on the PABX. Initial offer from FPBX contained
PCMA, G.722, second one same codecs in reverse order. Reported by A.T. 2023.03.31.

test multiple calls capacity / performance
test multiple calls recording
switch automatically to line with new incoming call if there is only one call
switch automatically to line with new outgoing call if there is only one call

support for multiple simultaneous RTP streaming - treat streaming as call

*/


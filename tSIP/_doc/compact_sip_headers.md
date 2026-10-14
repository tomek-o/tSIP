# Compact SIP header names

## What it does

SIP (RFC 3261, section 7.3.3 and extensions) defines one-letter compact forms
for some header names. When the option is enabled, tSIP uses them in outgoing
messages instead of full names, e.g.:

```
INVITE sip:200@pbx.example.com SIP/2.0
v: SIP/2.0/UDP 192.168.1.10:5060;branch=z9hG4bK...;rport
Max-Forwards: 70
t: <sip:200@pbx.example.com>
f: "John" <sip:100@pbx.example.com>;tag=...
i: 4f0c7a...
CSeq: 1 INVITE
m: <sip:100@192.168.1.10:5060>
c: application/sdp
l: 312
```

Main purpose: smaller messages. Over UDP this helps to keep requests (INVITE
with large SDP, e.g. with many codecs or video) below MTU and avoid IP
fragmentation, which is dropped by some routers, NATs and firewalls.

Headers with compact forms used by tSIP, in two groups that are enabled
separately:

| Full name        | Compact | Defined in | Group     |
|------------------|---------|------------|-----------|
| Via              | v       | RFC 3261   | RFC 3261  |
| From             | f       | RFC 3261   | RFC 3261  |
| To               | t       | RFC 3261   | RFC 3261  |
| Call-ID          | i       | RFC 3261   | RFC 3261  |
| Contact          | m       | RFC 3261   | RFC 3261  |
| Content-Type     | c       | RFC 3261   | RFC 3261  |
| Content-Length   | l       | RFC 3261   | RFC 3261  |
| Content-Encoding | e       | RFC 3261   | RFC 3261  |
| Supported        | k       | RFC 3261   | RFC 3261  |
| Subject          | s       | RFC 3261   | RFC 3261  |
| Event            | o       | RFC 6665   | extension |
| Allow-Events     | u       | RFC 6665   | extension |
| Refer-To         | r       | RFC 3515   | extension |
| Referred-By      | b       | RFC 3892   | extension |
| Session-Expires  | x       | RFC 4028   | extension |

Headers without compact form (e.g. `CSeq`, `Max-Forwards`, `Expires`,
`Accept`, `User-Agent`) are always sent with full names.

Receiving is not affected: compact headers in incoming messages were always
accepted, regardless of this option.

## How to enable

Settings -> **Network**:

- "Use compact SIP header names in outgoing messages (RFC 3261)" - RFC 3261
  group,
- "also for extension headers (...)" - extension group; available only when
  the first option is enabled.

Default: both disabled. Changing the options restarts the user agent (as
other network settings do).

Settings file (JSON):

```json
"uaConf" : {
   "sip" : {
      "compactHeadersRfc3261" : true,
      "compactHeadersExt" : false
   }
}
```

`compactHeadersExt` has no effect if `compactHeadersRfc3261` is false.

The options are global - they apply to all accounts.

## Interoperability

Compact forms of RFC 3261 headers are part of the base SIP specification and
every compliant implementation must accept them, but some older or simplified
PBXs, SBCs and SIP ALGs handle them incorrectly. Keep the option disabled
unless message size is a problem; if enabling it breaks registration or calls
with particular server, disable it again.

Compact forms of extension headers are recognized less often. For example
some pjsip versions register Event header parser without its compact form:
`o:` in SUBSCRIBE is then not recognized as Event header and subscription is
rejected with `489 Bad Event`; `Refer-To` / `Referred-By` may be looked up
only by full name as well. Enable the second option only if the other side is
known to handle these headers.

## What is not changed

- Headers that are copied to responses from received requests (`Via`, `From`,
  `To`, `Call-ID`, `CSeq`, `Record-Route`) keep the name used by the other
  side.
- Custom headers added by user (account settings, Lua scripts, custom
  requests) are sent exactly as entered.

## Implementation notes

- re: `sip_set_compact_headers(bool enabled, bool ext_enabled)` sets
  process-wide flags (RFC 3261 group, extension group);
  `sip_hname(enum sip_hdrid)` returns full or compact name
  (`re/src/sip/sip.c`, `re/include/re_sip.h`). The flags are process-wide
  (not part of `struct sip`) because many places creating headers (dialogs,
  sessions, subscriptions) have no direct access to `struct sip`, and tSIP
  uses single SIP stack anyway.
- Header names are selected where messages are created (re: `sip`,
  `sipsess`, `sipevent`, `sipreg`; baresip: `call.c`, `message.c`, `ua.c`,
  presence and dialog_info modules), e.g.

  ```c
  err |= mbuf_printf(mb, "%s: %r\r\n", sip_hname(SIP_HDR_VIA), &via);
  ```

  Messages are not rewritten after they are built, so what is logged
  (SIP message logging) is exactly what is created and sent.
- tSIP: `UaConf::Sip::compactHeadersRfc3261` and
  `UaConf::Sip::compactHeadersExt`, applied in `UaMain.cpp` before
  `ua_init()`.
- Per-account setting was not implemented: all accounts share one SIP stack
  and code creating headers does not know which account a message belongs
  to.

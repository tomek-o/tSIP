# Multiple SIP accounts

## What it does

tSIP can register and use several SIP accounts at the same time, e.g. an office
PBX extension and a SIP provider account. Calls can be received on any enabled
account, and outgoing calls and messages use the **current account**, unless
the account is given explicitly (button, contact, script or plugin).

Up to 16 accounts can be configured. With a single account, tSIP works and
looks the same as before: account selection controls are hidden.

## Configuration

Settings / SIP account:

- The account selector at the top of the page shows all configured accounts.
  **Add** creates a new account and **Remove** deletes the selected one.
- **Account enabled**: a disabled account keeps its configuration but is not
  registered and is not used for calls.
- **Name** (optional) is shown in menus, the history hint and the call state,
  e.g. "#2: Provider". Without a name the account is shown as `#2: user@server`.
- Audio codecs are configured per account on the same page. Rarely used settings
  are under **Show extra settings**. Opus settings are a separate page, and video
  codecs have their own account selector.
- Auto answer / DND can be set per account (see "Buttons" below).

Changing the auto answer settings of an account does not restart the SIP stack.
Other account changes restart it, which ends active calls.

### Removing an account

Buttons refer to accounts by number. If any button uses the removed account,
or an account after it, a dialog shows how many buttons are affected:

- **Yes**: remove the account and update the buttons. Buttons of the following
  accounts keep pointing to the same accounts (their numbers decrease by one).
  Buttons of the removed account are set to the default account.
- **No**: remove the account without changing the buttons. They now point to
  different accounts.
- **Cancel**: do not remove the account.

Button changes are saved when the settings are applied.

## Current account

The current account is used for outgoing calls and messages that don't specify
an account. It is saved on exit.

The status bar shows the registration state of the current account, e.g.
`Registered 100@pbx.example.com`, or `Registering... 100@pbx.example.com` before
the first registration result arrives. Hovering over the status bar shows the
state of all accounts. The tray icon shows an error if any enabled account
failed to register.

Ways to change the current account:

- **ACCOUNT** button, which also shows that account's registration state
- tray icon menu: **Account** submenu
- click on the status bar: the same account menu
- hotkeys: action **account #** (selects the chosen account) or **next account**
  (moves to the next enabled account and wraps around)
- Lua `SetCurrentAccount(id)` and plugin method `SetCurrentAccount`

The account menus are shown only when more than one account is configured.

## Incoming calls and messages

An incoming request is assigned to an account by the **user part of the request
URI**. This is the account's contact user (extra settings), or the user name if
the contact user is empty. Each enabled account should therefore have a
different contact user. Troubleshooting warns if two accounts share one.

If **Answer any call/message** is enabled for an account, that account receives
requests that don't match any account. Only one account should have this option.

- **Calls**: with more than one enabled account, the account that received the
  call is shown with the call state and in the tray notifier. It is stored in
  the call history. Calling back or redialing from history uses the same
  account, and the history entry hint shows it.
- **Text messages**: the message window replies through the account that
  received the message. With more than one account, the window caption shows
  the account, e.g. `Text: 200 [#2: Provider]`.

## Account selection for outgoing calls

The account is chosen in this order:

1. The account given explicitly: SPEED_DIAL / BLF / PRESENCE / MWI button
   account, history entry, Lua or plugin argument.
2. The account assigned to the called **contact** in the phonebook (contact
   editor: **Account**). The called number is matched against the contact's
   numbers, as for name lookup.
3. The current account.

If the selected account is missing or disabled, the current account is used.

## Buttons

Buttons whose type supports an account have an **Account** list in the button
editor. It is shown only when more than one account is configured, or when the
button already has an account assigned. The first item is the default:

| Button type     | Default (no account assigned)                  | With account assigned                                 |
|-----------------|------------------------------------------------|-------------------------------------------------------|
| ACCOUNT         | first account                                  | switches to and shows the state of that account       |
| SPEED_DIAL      | contact's account / current account           | calls using that account                              |
| BLF, PRESENCE   | first account                                  | subscription and calls use that account               |
| MWI             | first account                                  | shows that account's voicemail, calls through it      |
| AUTO_ANSWER_DND | global auto answer / DND setting               | toggles DND of that account only                      |

If an account has its own DND enabled, an incoming call on that account is
rejected with the code and reason set for that account. Otherwise the global
setting applies.

## Example button layout

`example_buttons_two_accounts.json` is a ready `tSIP_buttons.json` for two accounts
(#1 office, #2 provider), based on the standard dial pad layout. Close tSIP, back up
your own `tSIP_buttons.json` and copy the example over it. Edit numbers and captions
afterwards.

- first column: ACCOUNT buttons (#1, #2), MWI buttons (voicemail of each account),
  AUTO_ANSWER_DND buttons (DND of #1, DND of #2, DND for all accounts)
- second column: SPEED_DIAL through #1 (Reception) and #2 (Support), SPEED_DIAL without
  account (Mobile: contact's account or current account), BLF buttons: Alice and Bob on
  #1, Carol on #2

A second, grouped example is `example_buttons_two_accounts_grouped.json`: one coloured
block per account (blue: office PBX, orange: provider), each with a large ACCOUNT button,
voicemail and DND buttons, an "Extensions" group of BLF buttons, a "Quick dial" group of
large speed dial buttons, plus a grey column for buttons not tied to an account
(DND for all accounts, Mobile, Emergency).

## Lua

Account ids are 0-based. Old scripts work unchanged: all account arguments are
optional and default to the current account.

| Function | Description |
|----------|-------------|
| `GetAccountCount()` | number of configured accounts |
| `GetCurrentAccount()` | id of the current account |
| `SetCurrentAccount(id)` | select the current account (a disabled or missing account is ignored) |
| `GetAccountUserName(id)` | user name of an account |
| `GetCallAccountId([callUid])` | account of the current or given call (incoming: account that received it) |
| `GetRegistrationState([id])` | registration state (`Callback::reg_state_e`) of an account |
| `Call(target, [id])` | call using an account |
| `SendTextMessage(target, text, sendImmediately, [id])` | text message through an account |
| `SendCustomRequest(uri, method, extraHeaderLines, [id])` | out-of-dialog request through an account |

In the "on registration state" and "on SIMPLE message RX" scripts,
`GetExecSourceId()` returns the id of the account concerned.

## Plugins (interface v2) and MCP

- `GetRegistration` lists all accounts (`accountId`, `name`, `user`, `server`,
  `enabled`, `current`, registration state) and `currentAccountId`.
- `SetCurrentAccount` `{"accountId":N}`.
- Optional `accountId` for `Call`, `SendMessage` and out-of-dialog
  `SendCustomRequest`. A missing or disabled account returns an error.
- `accountId` is included in call info (`GetCalls`, the `call_state` event),
  `GetHistory` entries and the `registration` and `mwi` events.

The MCP plugin exposes the same features: an optional `accountId` for
`make_call`, `send_message` and `send_sip_request`; an account list in
`get_registration`; and a `set_current_account` tool.

See `phone/tSIP_plugin_v2.h` for details.

## Troubleshooting

Tools / Troubleshooting checks each enabled account and names the affected
accounts. Multi-account checks:

- no enabled account
- accounts with the same contact user
- more than one account with "Answer any call/message"
- a button assigned to an account that doesn't exist or is disabled

## Limitations

- Changing account settings (except auto answer / DND) restarts the SIP stack
  for all accounts and ends active calls.
- LINE buttons do not show the account of their call.
- BLF subscriptions are not renewed when only the button's account
  re-registers.
- Lua history access doesn't include the account.
- Attended transfer or conference between calls on different accounts depends
  on the servers and may not work.
- The settings, account and button edit windows are not translatable.

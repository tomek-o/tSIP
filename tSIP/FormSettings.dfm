object frmSettings: TfrmSettings
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Settings'
  ClientHeight = 429
  ClientWidth = 665
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 392
    Width = 665
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    object btnCancel: TButton
      Left = 584
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 503
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object pcGeneral: TPageControl
    Left = 117
    Top = 0
    Width = 548
    Height = 392
    ActivePage = tsScripts
    Align = alClient
    TabHeight = 10
    TabOrder = 1
    TabStop = False
    object tsGeneral: TTabSheet
      Caption = 'General'
      object lblGuiScaling: TLabel
        Left = 3
        Top = 143
        Width = 140
        Height = 13
        Caption = 'GUI scaling (restart required)'
      end
      object lblGuiScalingPct: TLabel
        Left = 226
        Top = 143
        Width = 11
        Height = 13
        Caption = '%'
      end
      object lblDialpadBackgroundImage: TLabel
        Left = 3
        Top = 232
        Width = 125
        Height = 13
        Caption = 'Dialpad background image'
      end
      object lblMainIconFile: TLabel
        Left = 3
        Top = 256
        Width = 128
        Height = 13
        Caption = 'Custom main icon file (.ico)'
      end
      object lblTrayNotificationIcon: TLabel
        Left = 3
        Top = 280
        Width = 168
        Height = 13
        Caption = 'Custom tray icon notification image'
      end
      object lblRestartRequired1: TLabel
        Left = 340
        Top = 256
        Width = 84
        Height = 13
        Caption = '(restart required)'
      end
      object lblRestartRequired2: TLabel
        Left = 340
        Top = 280
        Width = 84
        Height = 13
        Caption = '(restart required)'
      end
      object lblMainWindowHeight: TLabel
        Left = 3
        Top = 167
        Width = 94
        Height = 13
        Caption = 'Main window height'
      end
      object chbAlwaysOnTop: TCheckBox
        Left = 3
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Window always on top'
        TabOrder = 0
      end
      object chbStartMinimizedToTray: TCheckBox
        Left = 3
        Top = 26
        Width = 325
        Height = 17
        Caption = 'Start minimized to tray'
        TabOrder = 1
      end
      object edGuiScaling: TEdit
        Left = 178
        Top = 140
        Width = 42
        Height = 21
        TabOrder = 2
      end
      object chbXBtnMinimize: TCheckBox
        Left = 3
        Top = 48
        Width = 325
        Height = 17
        Caption = 'Minimize to tray when "X" button is pressed'
        TabOrder = 3
      end
      object chbRestoreMainWindowOnIncomingCall: TCheckBox
        Left = 3
        Top = 71
        Width = 382
        Height = 17
        Caption = 'Restore minimized or bring to front main window on incoming call'
        TabOrder = 4
      end
      object chbSingleInstance: TCheckBox
        Left = 3
        Top = 188
        Width = 382
        Height = 17
        Caption = 
          'Single instance (bring previous instance to front if already run' +
          'ning)'
        TabOrder = 5
      end
      object edDialpadBackgroundImage: TEdit
        Left = 178
        Top = 229
        Width = 121
        Height = 21
        TabOrder = 6
      end
      object btnSelectDialpadBackgroundImage: TButton
        Left = 305
        Top = 229
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 7
        OnClick = btnSelectImageClick
      end
      object chbNoBeepOnEnterKey: TCheckBox
        Left = 3
        Top = 209
        Width = 382
        Height = 17
        Caption = 
          'Do not beep when using [Enter] in main window to dial or transfe' +
          'r call'
        TabOrder = 8
      end
      object edMainIconFile: TEdit
        Left = 178
        Top = 253
        Width = 121
        Height = 21
        TabOrder = 9
      end
      object btnSelectMainIconFile: TButton
        Left = 305
        Top = 253
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 10
        OnClick = btnSelectImageClick
      end
      object edTrayNotificationImage: TEdit
        Left = 178
        Top = 277
        Width = 121
        Height = 21
        TabOrder = 11
      end
      object btnSelectTrayNotificationImage: TButton
        Left = 305
        Top = 277
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 12
        OnClick = btnSelectImageClick
      end
      object chbCustomUserAgent: TCheckBox
        Left = 3
        Top = 304
        Width = 157
        Height = 17
        Caption = 'Custom User-Agent value'
        TabOrder = 13
      end
      object edUserAgent: TEdit
        Left = 178
        Top = 302
        Width = 121
        Height = 21
        TabOrder = 14
      end
      object chbFrmMainShowWhenAnsweringCall: TCheckBox
        Left = 3
        Top = 94
        Width = 446
        Height = 17
        Caption = 
          'Show main form when answering call (using tray notifier, shortcu' +
          't, script, ...)'
        TabOrder = 15
      end
      object chbFrmMainUseCustomApplicationTitle: TCheckBox
        Left = 3
        Top = 327
        Width = 223
        Height = 17
        Caption = 'Custom application title (text in taskbar)'
        TabOrder = 16
      end
      object chbFrmMainUseCustomCaption: TCheckBox
        Left = 3
        Top = 351
        Width = 191
        Height = 17
        Caption = 'Custom caption for main window'
        TabOrder = 17
      end
      object edFrmMainCustomApplicationTitle: TEdit
        Left = 232
        Top = 325
        Width = 193
        Height = 21
        TabOrder = 18
      end
      object edFrmMainCustomCaption: TEdit
        Left = 232
        Top = 349
        Width = 193
        Height = 21
        TabOrder = 19
      end
      object edMainWindowHeight: TEdit
        Left = 178
        Top = 164
        Width = 42
        Height = 21
        TabOrder = 20
      end
      object chbFrmMainShowWhenMakingCall: TCheckBox
        Left = 3
        Top = 117
        Width = 478
        Height = 17
        Caption = 
          'Show main form when making call (using shortcut, script, command' +
          ' line / protocol handler...)'
        TabOrder = 21
      end
    end
    object tsNetwork: TTabSheet
      Caption = 'Network'
      ImageIndex = 14
      object lblLocalAddress: TLabel
        Left = 3
        Top = 3
        Width = 210
        Height = 13
        Caption = 'Local IP address (+ optional port to bind to)'
      end
      object lblRtpPortRange: TLabel
        Left = 3
        Top = 97
        Width = 130
        Height = 13
        Caption = 'RTP/RTCP local  port range'
      end
      object lblRtpPortMin: TLabel
        Left = 205
        Top = 97
        Width = 16
        Height = 13
        Caption = 'min'
      end
      object lblRtpPortMax: TLabel
        Left = 280
        Top = 97
        Width = 20
        Height = 13
        Caption = 'max'
      end
      object lblBindInfo: TLabel
        Left = 24
        Top = 22
        Width = 490
        Height = 13
        Caption = 
          'Binding is useful if you have multiple network interfaces and/or' +
          ' want to listen on non-random SIP port.'
      end
      object lblBindInfo2: TLabel
        Left = 24
        Top = 41
        Width = 408
        Height = 13
        Caption = 
          'To listen on specific port without specifying IP use e.g. (for p' +
          'ort 5090): 0.0.0.0:5090'
      end
      object lblRtpJitterBufDelay: TLabel
        Left = 3
        Top = 121
        Width = 151
        Height = 13
        Caption = 'RTP jitter buffer delay (frames)'
      end
      object lblJbufDelayMin: TLabel
        Left = 205
        Top = 121
        Width = 16
        Height = 13
        Caption = 'min'
      end
      object lblJbufDelayMax: TLabel
        Left = 280
        Top = 121
        Width = 20
        Height = 13
        Caption = 'max'
      end
      object lblIfName: TLabel
        Left = 3
        Top = 65
        Width = 135
        Height = 13
        Caption = 'Network interface (optional)'
      end
      object lblRtpTimeout: TLabel
        Left = 3
        Top = 145
        Width = 140
        Height = 13
        Caption = 'RTP timeout; 0 = disabled [s]'
      end
      object edLocalAddress: TEdit
        Left = 227
        Top = 0
        Width = 121
        Height = 21
        TabOrder = 0
      end
      object edRtpPortMin: TEdit
        Left = 227
        Top = 94
        Width = 42
        Height = 21
        TabOrder = 1
      end
      object edRtpPortMax: TEdit
        Left = 305
        Top = 94
        Width = 42
        Height = 21
        TabOrder = 2
      end
      object edJbufDelayMin: TEdit
        Left = 227
        Top = 118
        Width = 42
        Height = 21
        TabOrder = 3
      end
      object edJbufDelayMax: TEdit
        Left = 305
        Top = 118
        Width = 42
        Height = 21
        TabOrder = 4
      end
      object edIfName: TEdit
        Left = 226
        Top = 62
        Width = 231
        Height = 21
        TabOrder = 5
      end
      object edRtpTimeout: TEdit
        Left = 227
        Top = 142
        Width = 42
        Height = 21
        TabOrder = 6
        Text = '0'
      end
    end
    object tsAccount: TTabSheet
      Caption = 'SIP account'
      ImageIndex = 2
      object pnlAccountsBottom: TPanel
        Left = 0
        Top = 341
        Width = 540
        Height = 31
        Align = alBottom
        BevelOuter = bvNone
        TabOrder = 0
        object btnAddAccount: TButton
          Left = 464
          Top = 4
          Width = 75
          Height = 25
          Caption = 'Add new'
          TabOrder = 0
          Visible = False
          OnClick = btnAddAccountClick
        end
        object chbShowSettingsIfAnyAccountSettingsIsHidden: TCheckBox
          Left = 5
          Top = 8
          Width = 356
          Height = 17
          Caption = 'Show settings on startup if any account setting is hidden'
          TabOrder = 1
        end
      end
      object scrbAccounts: TScrollBox
        Left = 0
        Top = 0
        Width = 540
        Height = 341
        Align = alClient
        TabOrder = 1
      end
    end
    object tsSpeedDial: TTabSheet
      Caption = 'Speed Dial'
      ImageIndex = 12
      object lblSpeedDial: TLabel
        Left = 3
        Top = 3
        Width = 78
        Height = 13
        Caption = 'Speed dial panel'
      end
      object lblSpeedDialColumnWidth: TLabel
        Left = 3
        Top = 28
        Width = 220
        Height = 13
        Caption = 'Width of speed dial columns (restart required)'
      end
      object lblSpeedDialBlfSettings: TLabel
        Left = 3
        Top = 196
        Width = 122
        Height = 13
        Caption = 'BLF / dialog-info settings:'
      end
      object cbSpeedDialSize: TComboBox
        Left = 235
        Top = 0
        Width = 121
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 0
        Items.Strings = (
          'single column'
          '2 columns'
          '3 columns'
          '4 columns'
          '5 columns'
          '6 columns'
          '7 columns'
          '8 columns')
      end
      object chbShowSpeedDialOnly: TCheckBox
        Left = 3
        Top = 115
        Width = 325
        Height = 17
        Caption = 'Show only speed dial (console-only mode)'
        TabOrder = 1
      end
      object chbSpeedDialPopupMenu: TCheckBox
        Left = 3
        Top = 138
        Width = 325
        Height = 17
        Caption = 'Enable popup menu (editing) for programmable keys'
        TabOrder = 2
      end
      object chbSpeedDialIgnorePresenceNote: TCheckBox
        Left = 3
        Top = 161
        Width = 398
        Height = 17
        Caption = 
          'Ignore presence note (do not show second line for presence butto' +
          'n)'
        TabOrder = 3
      end
      object chbSpeedDialIgnoreDialogInfoRemoteIdentity: TCheckBox
        Left = 19
        Top = 218
        Width = 486
        Height = 17
        Caption = 
          'Ignore remote identity info (2nd line with caller/callee and cal' +
          'l direction, if present in notifications)'
        TabOrder = 4
      end
      object chbSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing: TCheckBox
        Left = 19
        Top = 241
        Width = 478
        Height = 17
        Caption = 
          'Keep previous remote identity info if remote identity is missing' +
          ' in notification'
        TabOrder = 5
      end
      object chbSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated: TCheckBox
        Left = 19
        Top = 264
        Width = 478
        Height = 17
        Caption = 
          'Ignore or clear remote identity if call state is set to terminat' +
          'ed'
        TabOrder = 6
      end
      object vleSpeedDialColWidths: TValueListEditor
        Left = 235
        Top = 23
        Width = 142
        Height = 86
        DefaultColWidth = 50
        ScrollBars = ssVertical
        Strings.Strings = (
          '=')
        TabOrder = 7
        TitleCaptions.Strings = (
          'Column'
          'Width')
        ColWidths = (
          50
          86)
      end
    end
    object tsCalls: TTabSheet
      Caption = 'Calls'
      ImageIndex = 4
      object lblSipAutoAnswerCode: TLabel
        Left = 22
        Top = 29
        Width = 243
        Height = 13
        Caption = 'Auto answer code (200 = accept, 486 = busy, ...)'
      end
      object lblAutoAnswerDelay: TLabel
        Left = 22
        Top = 55
        Width = 213
        Height = 13
        Caption = 'Auto answer delay, random from range [ms]'
      end
      object lblAutoAnswerDelayMin: TLabel
        Left = 146
        Top = 77
        Width = 16
        Height = 13
        Caption = 'min'
      end
      object lblAutoAnswerDelayMax: TLabel
        Left = 250
        Top = 77
        Width = 20
        Height = 13
        Caption = 'max'
      end
      object lblAutoAnswerCallInfoDelayMin: TLabel
        Left = 22
        Top = 146
        Width = 93
        Height = 13
        Caption = 'Minimum delay [ms]'
      end
      object lblCallsExtraHeaderLines: TLabel
        Left = 3
        Top = 242
        Width = 210
        Height = 13
        Caption = 'Extra/custom header lines for outgoing calls'
      end
      object edAutoAnswerCode: TEdit
        Left = 272
        Top = 26
        Width = 49
        Height = 21
        TabOrder = 0
      end
      object edAutoAnswerDelayMin: TEdit
        Left = 168
        Top = 74
        Width = 57
        Height = 21
        TabOrder = 1
      end
      object edAutoAnswerDelayMax: TEdit
        Left = 272
        Top = 74
        Width = 57
        Height = 21
        TabOrder = 2
      end
      object chbAutoAnswer: TCheckBox
        Left = 3
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Auto answer after time'
        TabOrder = 3
      end
      object chbAutoAnswerCallInfo: TCheckBox
        Left = 3
        Top = 123
        Width = 518
        Height = 17
        Caption = 
          'Auto answer when Call-Info with answer-after param or Answer-Aft' +
          'er=X value is received (intercom)'
        TabOrder = 4
      end
      object edAutoAnswerCallInfoDelayMin: TEdit
        Left = 272
        Top = 143
        Width = 57
        Height = 21
        TabOrder = 5
        Text = '0'
      end
      object chbAnswerOnEventTalk: TCheckBox
        Left = 3
        Top = 184
        Width = 363
        Height = 17
        Caption = 'Answer on receiving NOTIFY with Event: talk'
        TabOrder = 6
      end
      object chbHandleOodRefer: TCheckBox
        Left = 3
        Top = 207
        Width = 446
        Height = 17
        Caption = 'Handle incoming out-of-dialog REFER requests'
        TabOrder = 7
      end
      object memoCallsExtraHeaderLines: TMemo
        Left = 219
        Top = 239
        Width = 310
        Height = 44
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        TabOrder = 8
      end
      object chbDisconnectCallOnAudioError: TCheckBox
        Left = 3
        Top = 289
        Width = 462
        Height = 17
        Caption = 
          'Disconnect call on audio error (disable when using "on audio err' +
          'or" script event)'
        TabOrder = 9
      end
    end
    object tsDisplay: TTabSheet
      Caption = 'Display'
      ImageIndex = 15
      object chbUserOnlyClip: TCheckBox
        Left = 3
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Show only user part of URI as CLIP (if present)'
        TabOrder = 0
      end
      object chbDecodeUtfDisplayToAnsi: TCheckBox
        Left = 3
        Top = 26
        Width = 325
        Height = 17
        Caption = 'Decode Display Name (UTF8) to local (ANSI) code page'
        TabOrder = 1
      end
      object chbUsePAssertedIdentity: TCheckBox
        Left = 3
        Top = 49
        Width = 325
        Height = 17
        Caption = 'Use P-Asserted-Identity if present'
        TabOrder = 2
      end
    end
    object tsLocking: TTabSheet
      Caption = 'Locking'
      ImageIndex = 19
      object chbHideSettings: TCheckBox
        Left = 3
        Top = 3
        Width = 518
        Height = 17
        Caption = 
          'Hide settings menu; WARNING: to restore edit root["frmMain"]["Hi' +
          'deSettings"] in JSON config file'
        TabOrder = 0
      end
      object chbHideViewMenu: TCheckBox
        Left = 3
        Top = 24
        Width = 518
        Height = 17
        Caption = 'Hide "View" menu'
        TabOrder = 1
      end
      object chbHideHelpMenu: TCheckBox
        Left = 3
        Top = 64
        Width = 518
        Height = 17
        Caption = 'Hide "Help" menu'
        TabOrder = 2
      end
      object chbKioskMode: TCheckBox
        Left = 3
        Top = 84
        Width = 398
        Height = 17
        Caption = 'Kiosk mode (fullscreen, no main menu, status bar, border)'
        TabOrder = 3
      end
      object chbHideMouseCursor: TCheckBox
        Left = 3
        Top = 126
        Width = 398
        Height = 17
        Caption = 'Hide mouse cursor'
        TabOrder = 4
      end
      object chbHideToolsMenu: TCheckBox
        Left = 3
        Top = 44
        Width = 518
        Height = 17
        Caption = 'Hide "Tools" menu'
        TabOrder = 5
      end
      object chbHideSpeedDialToggleButton: TCheckBox
        Left = 3
        Top = 105
        Width = 398
        Height = 17
        Caption = 'Hide speed dial toggle button (button with "<<" / ">>")'
        TabOrder = 6
      end
    end
    object tsRing: TTabSheet
      Caption = 'Ring'
      ImageIndex = 8
      object lblRingDefault: TLabel
        Left = 5
        Top = 3
        Width = 56
        Height = 13
        Caption = 'Default ring'
      end
      object lblRingBellcoreDr1: TLabel
        Left = 5
        Top = 26
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr1'
      end
      object lblRingBellcoreDr2: TLabel
        Left = 5
        Top = 49
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr2'
      end
      object lblRingBellcoreDr3: TLabel
        Left = 5
        Top = 72
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr3'
      end
      object lblRingBellcoreDr4: TLabel
        Left = 5
        Top = 95
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr4'
      end
      object lblRingBellcoreDr5: TLabel
        Left = 5
        Top = 118
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr5'
      end
      object lblRingBellcoreDr6: TLabel
        Left = 5
        Top = 141
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr6'
      end
      object lblRingBellcoreDr7: TLabel
        Left = 5
        Top = 164
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr7'
      end
      object lblRingBellcoreDr8: TLabel
        Left = 5
        Top = 187
        Width = 57
        Height = 13
        Caption = 'Bellcore-dr8'
      end
      object edRingDefault: TEdit
        Left = 96
        Top = 0
        Width = 121
        Height = 21
        TabOrder = 0
      end
      object btnRingSelectDefault: TButton
        Left = 221
        Top = 0
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 1
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr1: TEdit
        Left = 96
        Top = 23
        Width = 121
        Height = 21
        TabOrder = 2
      end
      object btnRingSelectBellcoreDr1: TButton
        Left = 221
        Top = 23
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 3
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr2: TEdit
        Left = 96
        Top = 46
        Width = 121
        Height = 21
        TabOrder = 4
      end
      object btnRingSelectBellcoreDr2: TButton
        Left = 221
        Top = 46
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 5
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr3: TEdit
        Left = 96
        Top = 69
        Width = 121
        Height = 21
        TabOrder = 6
      end
      object btnRingSelectBellcoreDr3: TButton
        Left = 221
        Top = 69
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 7
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr4: TEdit
        Left = 96
        Top = 92
        Width = 121
        Height = 21
        TabOrder = 8
      end
      object btnRingSelectBellcoreDr4: TButton
        Left = 221
        Top = 92
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 9
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr5: TEdit
        Left = 96
        Top = 115
        Width = 121
        Height = 21
        TabOrder = 10
      end
      object btnRingSelectBellcoreDr5: TButton
        Left = 221
        Top = 115
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 11
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr6: TEdit
        Left = 96
        Top = 138
        Width = 121
        Height = 21
        TabOrder = 12
      end
      object btnRingSelectBellcoreDr6: TButton
        Left = 221
        Top = 138
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 13
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr7: TEdit
        Left = 96
        Top = 161
        Width = 121
        Height = 21
        TabOrder = 14
      end
      object btnRingSelectBellcoreDr7: TButton
        Left = 221
        Top = 161
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 15
        OnClick = btnRingSelectClick
      end
      object edRingBellcoreDr8: TEdit
        Left = 96
        Top = 184
        Width = 121
        Height = 21
        TabOrder = 16
      end
      object btnRingSelectBellcoreDr8: TButton
        Left = 221
        Top = 184
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 17
        OnClick = btnRingSelectClick
      end
    end
    object tsAudioIO: TTabSheet
      Caption = 'Audio I/O'
      ImageIndex = 5
      object lblSoundInputModule: TLabel
        Left = 43
        Top = 22
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblAudioInput: TLabel
        Left = 3
        Top = 4
        Width = 26
        Height = 13
        Caption = 'Input'
      end
      object lblSoundInputDevice: TLabel
        Left = 43
        Top = 47
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioOutput: TLabel
        Left = 3
        Top = 67
        Width = 34
        Height = 13
        Caption = 'Output'
      end
      object Label3: TLabel
        Left = 43
        Top = 86
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundOutputDev: TLabel
        Left = 43
        Top = 111
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioAlertOutput: TLabel
        Left = 3
        Top = 131
        Width = 241
        Height = 13
        Caption = 'Alert output (call progress, ringback, busy signals)'
      end
      object Label6: TLabel
        Left = 43
        Top = 150
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundAlertOutputDev: TLabel
        Left = 43
        Top = 175
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioOutputIntercom: TLabel
        Left = 3
        Top = 257
        Width = 131
        Height = 13
        Caption = 'Output for paging/intercom'
      end
      object Label2: TLabel
        Left = 43
        Top = 276
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundOutputIntercomDev: TLabel
        Left = 43
        Top = 301
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioRingOutput: TLabel
        Left = 3
        Top = 193
        Width = 141
        Height = 13
        Caption = 'Ring output (on incoming call)'
      end
      object Label9: TLabel
        Left = 43
        Top = 212
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundRingOutputDev: TLabel
        Left = 43
        Top = 237
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object cbSoundInputMod: TComboBox
        Left = 121
        Top = 17
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 0
        Text = 'PortAudio / DirectSound'
        OnChange = cbSoundInputModChange
        Items.Strings = (
          'PortAudio / DirectSound'
          'WaveIn, WaveOut'
          'Wave file'
          'Null (silence)')
      end
      object cbSoundInputDev: TComboBox
        Left = 121
        Top = 44
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 1
      end
      object cbSoundOutputMod: TComboBox
        Left = 121
        Top = 83
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 2
        Text = 'PortAudio / DirectSound'
        OnChange = cbSoundOutputModChange
        Items.Strings = (
          'PortAudio / DirectSound'
          'WaveIn, WaveOut'
          'Null')
      end
      object cbSoundOutputDev: TComboBox
        Left = 121
        Top = 108
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 3
      end
      object cbSoundAlertOutputMod: TComboBox
        Left = 121
        Top = 147
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 4
        Text = 'PortAudio / DirectSound'
        OnChange = cbSoundAlertOutputModChange
        Items.Strings = (
          'PortAudio / DirectSound'
          'WaveIn, WaveOut'
          'Null')
      end
      object cbSoundAlertOutputDev: TComboBox
        Left = 121
        Top = 172
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 5
      end
      object cbSoundOutputIntercomMod: TComboBox
        Left = 121
        Top = 273
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 6
        Text = 'PortAudio / DirectSound'
        OnChange = cbSoundOutputIntercomModChange
        Items.Strings = (
          'PortAudio / DirectSound'
          'WaveIn, WaveOut'
          'Null')
      end
      object cbSoundOutputIntercomDev: TComboBox
        Left = 121
        Top = 298
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 7
      end
      object btnSelectWaveFile: TButton
        Left = 367
        Top = 45
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 8
        OnClick = btnSelectWaveFileClick
      end
      object edSoundInputWave: TEdit
        Left = 121
        Top = 44
        Width = 240
        Height = 21
        TabOrder = 9
      end
      object cbSoundRingOutputDev: TComboBox
        Left = 121
        Top = 234
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 10
      end
      object cbSoundRingOutputMod: TComboBox
        Left = 121
        Top = 209
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 11
        Text = 'PortAudio / DirectSound'
        OnChange = cbSoundRingOutputModChange
        Items.Strings = (
          'PortAudio / DirectSound'
          'WaveIn, WaveOut'
          'Null')
      end
    end
    object tsAudioProcessing: TTabSheet
      Caption = 'Audio Processing'
      ImageIndex = 6
      object lblAec: TLabel
        Left = 3
        Top = 7
        Width = 113
        Height = 13
        Caption = 'AEC (echo cancellation)'
      end
      object lblWebRtcAec: TLabel
        Left = 3
        Top = 37
        Width = 65
        Height = 13
        Caption = 'WebRTC AEC'
      end
      object Label1: TLabel
        Left = 82
        Top = 37
        Width = 101
        Height = 13
        Caption = 'ms in snd card buffer'
      end
      object lblWebRtcAecClockSkew: TLabel
        Left = 290
        Top = 37
        Width = 50
        Height = 13
        Caption = 'clock skew'
      end
      object lblAudioPreprocessingTxAgcLevel: TLabel
        Left = 135
        Top = 125
        Width = 22
        Height = 13
        Caption = 'level'
      end
      object cbAec: TComboBox
        Left = 122
        Top = 4
        Width = 206
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 2
        TabOrder = 0
        Text = 'WebRTC'
        Items.Strings = (
          'none'
          'Speex'
          'WebRTC')
      end
      object edWebRtcAecMsInSndCardBuf: TEdit
        Left = 190
        Top = 34
        Width = 75
        Height = 21
        TabOrder = 1
      end
      object edWebRtcAecSkew: TEdit
        Left = 346
        Top = 34
        Width = 75
        Height = 21
        TabOrder = 2
      end
      object chbAudioPreprocessingTxEnabled: TCheckBox
        Left = 2
        Top = 82
        Width = 494
        Height = 17
        Caption = 
          'Enable audio preprocessing for microphone path (transmitted audi' +
          'o) [UNSTABLE/EXPERIMENTAL]'
        TabOrder = 3
        OnClick = chbAudioPreprocessingTxEnabledClick
      end
      object chbAudioPreprocessingTxDenoiseEnabled: TCheckBox
        Left = 18
        Top = 103
        Width = 494
        Height = 17
        Caption = 'Enable denoise'
        TabOrder = 4
      end
      object chbAudioPreprocessingTxAgcEnabled: TCheckBox
        Left = 18
        Top = 124
        Width = 105
        Height = 17
        Caption = 'Enable AGC'
        TabOrder = 5
      end
      object edAudioPreprocessingTxAgcLevel: TEdit
        Left = 175
        Top = 122
        Width = 49
        Height = 21
        TabOrder = 6
      end
      object chbAudioPreprocessingTxVadEnabled: TCheckBox
        Left = 18
        Top = 145
        Width = 105
        Height = 17
        Caption = 'Enable VAD'
        TabOrder = 7
      end
      object chbAudioPreprocessingTxDereverbEnabled: TCheckBox
        Left = 18
        Top = 166
        Width = 105
        Height = 17
        Caption = 'Enable dereverb'
        TabOrder = 8
      end
    end
    object tsRecording: TTabSheet
      Caption = 'Recording'
      ImageIndex = 11
      object lblRecDirType: TLabel
        Left = 3
        Top = 37
        Width = 94
        Height = 13
        Caption = 'Recording directory'
      end
      object lblCustomRecDir: TLabel
        Left = 3
        Top = 64
        Width = 152
        Height = 13
        Caption = 'Custom directory for recordings'
      end
      object lblRecordingMode: TLabel
        Left = 3
        Top = 92
        Width = 93
        Height = 13
        Caption = 'Recording channels'
      end
      object lblRecordingStartMode: TLabel
        Left = 3
        Top = 121
        Width = 53
        Height = 13
        Caption = 'Start mode'
      end
      object lblRecordedSide: TLabel
        Left = 3
        Top = 148
        Width = 68
        Height = 13
        Caption = 'Recorded side'
      end
      object chbRecordingEnabled: TCheckBox
        Left = 3
        Top = 3
        Width = 97
        Height = 17
        Caption = 'Enable recording'
        TabOrder = 0
      end
      object cbRecDirType: TComboBox
        Left = 167
        Top = 34
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 1
        Text = '\recordings\ subdirectory (relative)'
        OnChange = cbRecDirTypeChange
        Items.Strings = (
          '\recordings\ subdirectory (relative)'
          'custom (absolute)')
      end
      object edCustomRecDir: TEdit
        Left = 167
        Top = 61
        Width = 247
        Height = 21
        TabOrder = 2
      end
      object btnSelectCustomRecDir: TButton
        Left = 420
        Top = 61
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 3
        OnClick = btnSelectCustomRecDirClick
      end
      object cbRecordingChannels: TComboBox
        Left = 167
        Top = 89
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 4
        Text = 'mono (both sides mixed or one side only)'
        OnChange = cbRecordingChannelsChange
        Items.Strings = (
          'mono (both sides mixed or one side only)'
          'stereo (both sides, separated, larger size)')
      end
      object cbRecordingStart: TComboBox
        Left = 167
        Top = 118
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 1
        TabOrder = 5
        Text = 'automatic - call confirmed state'
        Items.Strings = (
          'manual (Lua script)'
          'automatic - call confirmed state'
          'automatic - call early media or call confirmed')
      end
      object cbRecordedSide: TComboBox
        Left = 167
        Top = 145
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 6
        Text = 'both local and remote (mixed)'
        OnChange = cbRecDirTypeChange
        Items.Strings = (
          'both local and remote (mixed)'
          'local party only (your microphone)'
          'remote party only')
      end
    end
    object tsCodecs: TTabSheet
      Caption = 'Codecs'
      ImageIndex = 3
      object lblAudioCodecsAvailable: TLabel
        Left = 15
        Top = 5
        Width = 108
        Height = 13
        Caption = 'Available audio codecs'
      end
      object lblAudioCodecsEnabled: TLabel
        Left = 207
        Top = 5
        Width = 151
        Height = 13
        Caption = 'Enabled, in order of preference'
      end
      object lboxAudioCodecsAvailable: TListBox
        Left = 15
        Top = 24
        Width = 150
        Height = 256
        ItemHeight = 13
        TabOrder = 0
        OnDblClick = lboxAudioCodecsAvailableDblClick
      end
      object lboxAudioCodecsEnabled: TListBox
        Left = 207
        Top = 24
        Width = 150
        Height = 256
        DragMode = dmAutomatic
        ItemHeight = 13
        TabOrder = 1
        OnDblClick = lboxAudioCodecsEnabledDblClick
        OnDragDrop = lboxAudioCodecsEnabledDragDrop
        OnDragOver = lboxAudioCodecsEnabledDragOver
        OnStartDrag = lboxAudioCodecsEnabledStartDrag
      end
      object btnAudioCodecEnable: TButton
        Left = 171
        Top = 113
        Width = 30
        Height = 25
        Hint = 'Enable selected codec'
        Caption = '->'
        TabOrder = 2
        OnClick = btnAudioCodecEnableClick
      end
      object btnAudioCodecDisable: TButton
        Left = 171
        Top = 160
        Width = 30
        Height = 25
        Hint = 'Disable selected codec'
        Caption = '<-'
        TabOrder = 3
        OnClick = btnAudioCodecDisableClick
      end
    end
    object tsIntegration: TTabSheet
      Caption = 'Integration'
      ImageIndex = 7
      object lblProtocolName: TLabel
        Left = 26
        Top = 22
        Width = 68
        Height = 13
        Caption = 'Protocol name'
      end
      object lblProtoInfo1: TLabel
        Left = 127
        Top = 46
        Width = 349
        Height = 13
        Caption = 
          'Action requires administrator rights (running application as adm' +
          'inistrator)'
      end
      object lblProtoInfo2: TLabel
        Left = 127
        Top = 65
        Width = 332
        Height = 13
        Caption = 'Creates entries in HKEY_CLASSES_ROOT, key name = protocol name'
      end
      object lblIntegrationFromBrowser: TLabel
        Left = 3
        Top = 3
        Width = 247
        Height = 13
        Caption = 'Browser or e-mail client -> softphone ("click to call")'
      end
      object lblIntegrationSoftphoneToBrowser: TLabel
        Left = 3
        Top = 184
        Width = 446
        Height = 13
        Caption = 
          'Softphone -> browser (opening page specified in incoming INVITE ' +
          'with "Access-URL" header)'
      end
      object lblIntegrationSoftphoneToBrowserMode: TLabel
        Left = 26
        Top = 203
        Width = 26
        Height = 13
        Caption = 'Mode'
      end
      object chAddFilterWMCopyData: TCheckBox
        Left = 26
        Top = 83
        Width = 511
        Height = 17
        Caption = 
          'Allow WM_COPYDATA to be passed to when elevated (e.g. running as' +
          ' admin under debugger)'
        TabOrder = 0
      end
      object btnCreateIntegrationProtocol: TButton
        Left = 214
        Top = 19
        Width = 302
        Height = 21
        Caption = 'Associate application with protocol (browser integration)'
        TabOrder = 1
        OnClick = btnCreateIntegrationProtocolClick
      end
      object edProtocolName: TEdit
        Left = 127
        Top = 19
        Width = 81
        Height = 21
        TabOrder = 2
        Text = 'tel'
      end
      object cbSipAccessUrlMode: TComboBox
        Left = 127
        Top = 200
        Width = 322
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 3
        Text = 'always passive (only using "Access-URL" button)'
        Items.Strings = (
          'always passive (only using "Access-URL" button)'
          'as specified by "mode" parameter in received message'
          'always active (always automatic)')
      end
      object chbDoNotUseSipPrefixForDirectIpCalls: TCheckBox
        Left = 26
        Top = 102
        Width = 511
        Height = 17
        Caption = 
          'Do not use "sip:" prefix for direct IP calls (clean passed text ' +
          'even if starts with "sip:")'
        TabOrder = 4
      end
    end
    object tsHotkeys: TTabSheet
      Caption = 'Hotkeys'
      ImageIndex = 9
    end
    object tsContacts: TTabSheet
      Caption = 'Contacts'
      ImageIndex = 9
      object lblContactPopup: TLabel
        Left = 7
        Top = 7
        Width = 233
        Height = 13
        Caption = 'Show contact popup if contact note is not empty'
      end
      object lblContactHttpQuery: TLabel
        Left = 7
        Top = 95
        Width = 490
        Height = 13
        Caption = 
          'HTTP query (opens browser with last incoming phone number passed' +
          ' as parameter on action execute)'
      end
      object lblHttpQueryStartMode: TLabel
        Left = 7
        Top = 144
        Width = 82
        Height = 13
        Caption = 'HTTP query start'
      end
      object chbContactPopupShowOnIncoming: TCheckBox
        Left = 18
        Top = 27
        Width = 438
        Height = 17
        Caption = 'on incoming call'
        TabOrder = 0
      end
      object chbContactPopupShowOnOutgoing: TCheckBox
        Left = 18
        Top = 50
        Width = 325
        Height = 17
        Caption = 'on outgoing call'
        TabOrder = 1
      end
      object edContactHttpQuery: TEdit
        Left = 114
        Top = 114
        Width = 385
        Height = 21
        TabOrder = 2
        Text = 'https://www.google.com/search?q=[number]'
      end
      object chbContactFilterUsingNote: TCheckBox
        Left = 7
        Top = 186
        Width = 325
        Height = 17
        Caption = 'Include "Note" field when filtering contacts'
        TabOrder = 3
      end
      object cbHttpQueryOpenMode: TComboBox
        Left = 114
        Top = 141
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 4
        Text = 'Manual only (HTTP query button)'
        Items.Strings = (
          'Manual only (HTTP query button)'
          'Automatic: on incoming call'
          'Automatic: on accepting incoming call')
      end
    end
    object tsHistory: TTabSheet
      Caption = 'History'
      ImageIndex = 16
      object chbHistoryNoStoreToFile: TCheckBox
        Left = 3
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Do not store history to file'
        TabOrder = 0
      end
      object chbHistoryUsePaiForDisplayIfAvailable: TCheckBox
        Left = 3
        Top = 26
        Width = 446
        Height = 17
        Caption = 
          'Prefer P-Asserted-Identity information for display if available ' +
          '(see also: "Display" tab)'
        TabOrder = 1
      end
      object chbHistoryUsePaiForDialIfAvailable: TCheckBox
        Left = 3
        Top = 49
        Width = 446
        Height = 17
        Caption = 
          'Prefer P-Asserted-Identity information for dialing if available ' +
          '(see also: "Display" tab)'
        TabOrder = 2
      end
      object chbHistoryShowHint: TCheckBox
        Left = 3
        Top = 72
        Width = 446
        Height = 17
        Caption = 'Show hint with additional information for history entries'
        TabOrder = 3
      end
      object chbHistoryFormatCallDurationAsHourMinSec: TCheckBox
        Left = 2
        Top = 118
        Width = 446
        Height = 17
        Caption = 'Format call duration time as hours:minutes:seconds'
        TabOrder = 4
      end
      object chbHistoryShowCodecNameInHint: TCheckBox
        Left = 21
        Top = 95
        Width = 446
        Height = 17
        Caption = 'Show codec name in hint'
        TabOrder = 5
      end
    end
    object tsPhones: TTabSheet
      Caption = 'Phones'
      ImageIndex = 13
    end
    object tsTrayNotifier: TTabSheet
      Caption = 'Tray Notifier'
      ImageIndex = 16
      object lblTrayNotifierBackgroundImage: TLabel
        Left = 3
        Top = 72
        Width = 87
        Height = 13
        Caption = 'Background image'
      end
      object Label4: TLabel
        Left = 3
        Top = 96
        Width = 53
        Height = 13
        Caption = 'GUI scaling'
      end
      object Label5: TLabel
        Left = 275
        Top = 96
        Width = 11
        Height = 13
        Caption = '%'
      end
      object chbTrayNotifierShowOnIncoming: TCheckBox
        Left = 2
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Show tray notifier on incoming call'
        TabOrder = 0
      end
      object chbSkipTrayNotifierIfMainWindowVisible: TCheckBox
        Left = 19
        Top = 26
        Width = 325
        Height = 17
        Caption = 'Skip if main window is visible'
        TabOrder = 1
      end
      object chbTrayNotifierShowOnOutgoing: TCheckBox
        Left = 2
        Top = 49
        Width = 502
        Height = 17
        Caption = 
          'Show tray notifier on outgoing call if application is hidden to ' +
          'tray (e.g. when using hotkeys)'
        TabOrder = 2
      end
      object edTrayNotifierBackgroundImage: TEdit
        Left = 166
        Top = 69
        Width = 121
        Height = 21
        TabOrder = 3
      end
      object btnTrayNotifierBackgroundImage: TButton
        Left = 293
        Top = 69
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 4
        OnClick = btnSelectImageClick
      end
      object edTrayNotifierGuiScaling: TEdit
        Left = 227
        Top = 93
        Width = 42
        Height = 21
        TabOrder = 5
      end
      object chbTrayNotifierHideWhenAnsweringCall: TCheckBox
        Left = 3
        Top = 115
        Width = 502
        Height = 17
        Caption = 'Hide tray notifier when (manually) answering call'
        TabOrder = 6
      end
      object chbTrayNotifierHideWhenAnsweringCallAutomatically: TCheckBox
        Left = 3
        Top = 136
        Width = 502
        Height = 17
        Caption = 'Hide tray notifier when call is auto-answered'
        TabOrder = 7
      end
    end
    object tsScripts: TTabSheet
      Caption = 'Scripts'
      ImageIndex = 18
      ExplicitLeft = 6
      ExplicitTop = 20
      object lblScriptOnCallStateFile: TLabel
        Left = 5
        Top = 59
        Width = 96
        Height = 13
        Caption = 'on call state change'
      end
      object lblScriptEvents: TLabel
        Left = 5
        Top = 5
        Width = 254
        Height = 13
        Caption = 'Lua scripts executed on events (\script subdirectory)'
      end
      object lblScriptOnStreamingStateFile: TLabel
        Left = 5
        Top = 85
        Width = 128
        Height = 13
        Caption = 'on streaming state change'
      end
      object lblScriptOnMakeCall: TLabel
        Left = 5
        Top = 33
        Width = 66
        Height = 13
        Caption = 'on making call'
      end
      object lblScriptOnTimer: TLabel
        Left = 5
        Top = 162
        Width = 39
        Height = 13
        Caption = 'on timer'
      end
      object lblScriptTimerPeriod: TLabel
        Left = 423
        Top = 162
        Width = 30
        Height = 13
        Caption = 'Period'
      end
      object lblScriptOnRegistrationStateChange: TLabel
        Left = 5
        Top = 112
        Width = 136
        Height = 13
        Caption = 'on registration state change'
      end
      object lblScriptOnStartup: TLabel
        Left = 5
        Top = 137
        Width = 50
        Height = 13
        Caption = 'on startup'
      end
      object lblScriptOnBlf: TLabel
        Left = 5
        Top = 214
        Width = 131
        Height = 13
        Caption = 'on BLF (dialog-info) change'
      end
      object lblScriptOnDial: TLabel
        Left = 5
        Top = 240
        Width = 77
        Height = 13
        Caption = 'on dial (keypad)'
      end
      object lblScriptOnProgrammableButton: TLabel
        Left = 17
        Top = 266
        Width = 118
        Height = 13
        Caption = 'on programmable button'
      end
      object lblScriptOnAudioError: TLabel
        Left = 5
        Top = 292
        Width = 102
        Height = 13
        Caption = 'on audio device error'
      end
      object lblScriptPeriodMs: TLabel
        Left = 519
        Top = 162
        Width = 13
        Height = 13
        Caption = 'ms'
      end
      object lblScriptOnTimer2: TLabel
        Left = 5
        Top = 188
        Width = 48
        Height = 13
        Caption = 'on timer 2'
      end
      object lblScriptTimer2Period: TLabel
        Left = 423
        Top = 188
        Width = 30
        Height = 13
        Caption = 'Period'
      end
      object lblScriptTimer2Ms: TLabel
        Left = 519
        Top = 188
        Width = 13
        Height = 13
        Caption = 'ms'
      end
      object edScriptOnCallStateChangeFile: TEdit
        Left = 145
        Top = 56
        Width = 212
        Height = 21
        TabOrder = 0
      end
      object btnSelectedScriptOnCallStateChange: TButton
        Left = 363
        Top = 56
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 1
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnStreamingStateChangeFile: TEdit
        Left = 145
        Top = 82
        Width = 212
        Height = 21
        TabOrder = 2
      end
      object btnSelectedScriptOnStreamingStateChange: TButton
        Left = 363
        Top = 82
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 3
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnMakeCallFile: TEdit
        Left = 145
        Top = 30
        Width = 212
        Height = 21
        TabOrder = 4
      end
      object btnSelectedScriptOnMakeCallChange: TButton
        Left = 363
        Top = 30
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 5
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnTimerFile: TEdit
        Left = 145
        Top = 158
        Width = 212
        Height = 21
        TabOrder = 6
      end
      object btnSelectedScriptOnTimerChange: TButton
        Left = 363
        Top = 159
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 7
        OnClick = btnSelectedScriptClick
      end
      object edScriptTimer: TEdit
        Left = 459
        Top = 159
        Width = 54
        Height = 21
        TabOrder = 8
      end
      object edScriptOnRegistrationStateChangeFile: TEdit
        Left = 145
        Top = 109
        Width = 212
        Height = 21
        TabOrder = 9
      end
      object btnSelectedScriptOnRegistrationStateChange: TButton
        Left = 363
        Top = 109
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 10
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnStartupFile: TEdit
        Left = 145
        Top = 134
        Width = 212
        Height = 21
        TabOrder = 12
      end
      object btnSelectedScriptOnStartupChange: TButton
        Left = 363
        Top = 134
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 11
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnDialogInfoFile: TEdit
        Left = 145
        Top = 212
        Width = 212
        Height = 21
        TabOrder = 13
      end
      object btnSelectedScriptOnDialogInfoChange: TButton
        Left = 363
        Top = 212
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 14
        OnClick = btnSelectedScriptClick
      end
      object edScriptOnDialFile: TEdit
        Left = 145
        Top = 238
        Width = 212
        Height = 21
        TabOrder = 15
      end
      object btnSelectedScriptOnDialChange: TButton
        Left = 363
        Top = 238
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 16
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnMakeCallEdit: TBitBtn
        Left = 390
        Top = 30
        Width = 22
        Height = 21
        TabOrder = 17
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnCallStateEdit: TBitBtn
        Left = 390
        Top = 56
        Width = 22
        Height = 21
        TabOrder = 18
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnStreamingStateEdit: TBitBtn
        Left = 390
        Top = 82
        Width = 22
        Height = 21
        TabOrder = 19
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnRegistrationStateEdit: TBitBtn
        Left = 390
        Top = 109
        Width = 22
        Height = 21
        TabOrder = 20
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnStartupEdit: TBitBtn
        Left = 390
        Top = 134
        Width = 22
        Height = 21
        TabOrder = 21
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnTimerEdit: TBitBtn
        Left = 390
        Top = 159
        Width = 22
        Height = 21
        TabOrder = 22
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnDialogInfoEdit: TBitBtn
        Left = 390
        Top = 212
        Width = 22
        Height = 21
        TabOrder = 23
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object btnSelectedScriptOnDialEdit: TBitBtn
        Left = 390
        Top = 238
        Width = 22
        Height = 21
        TabOrder = 24
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object edScriptOnProgrammableButtonFile: TEdit
        Left = 145
        Top = 263
        Width = 212
        Height = 21
        TabOrder = 25
      end
      object btnSelectedScriptOnProgrammableButtonChange: TButton
        Left = 363
        Top = 263
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 26
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnProgrammableButtonEdit: TBitBtn
        Left = 390
        Top = 263
        Width = 22
        Height = 21
        TabOrder = 27
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object edScriptOnAudioErrorFile: TEdit
        Left = 145
        Top = 290
        Width = 212
        Height = 21
        TabOrder = 28
      end
      object btnSelectedScriptOnAudioErrorChange: TButton
        Left = 363
        Top = 290
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 29
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnAudioErrorEdit: TBitBtn
        Left = 390
        Top = 290
        Width = 22
        Height = 21
        TabOrder = 30
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object edScriptOnTimer2File: TEdit
        Left = 145
        Top = 185
        Width = 212
        Height = 21
        TabOrder = 31
      end
      object btnSelectedScriptOnTimer2Change: TButton
        Left = 363
        Top = 185
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 32
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnTimer2Edit: TBitBtn
        Left = 390
        Top = 185
        Width = 22
        Height = 21
        TabOrder = 33
        OnClick = btnSelectedScriptEditClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
          000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
          D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
      end
      object edScriptTimer2: TEdit
        Left = 459
        Top = 185
        Width = 54
        Height = 21
        TabOrder = 34
      end
    end
    object tsLogging: TTabSheet
      Caption = 'Logging'
      ImageIndex = 1
      object lblUiCapacity: TLabel
        Left = 5
        Top = 122
        Width = 194
        Height = 13
        Caption = 'Visible (buffered) log lines in log window:'
      end
      object lblLogMaxFileSize: TLabel
        Left = 5
        Top = 48
        Width = 354
        Height = 13
        Caption = 
          'Log file size limit (0 = no limit); file would be reset/rotated ' +
          'when exceeded'
      end
      object lblLogRotate: TLabel
        Left = 5
        Top = 72
        Width = 208
        Height = 13
        Caption = 'Number of old log files to keep ("logrotate")'
      end
      object lblLoggingConsoleFont: TLabel
        Left = 5
        Top = 152
        Width = 79
        Height = 13
        Caption = 'Log window font'
      end
      object cmbMaxUiLogLines: TComboBox
        Left = 365
        Top = 119
        Width = 111
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 0
        Items.Strings = (
          '100'
          '200'
          '500'
          '1000'
          '2000'
          '5000'
          '10000')
      end
      object chbLogToFile: TCheckBox
        Left = 5
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Log to file'
        TabOrder = 1
      end
      object chbLogMessages: TCheckBox
        Left = 5
        Top = 96
        Width = 325
        Height = 17
        Caption = 'Log received and transmitted SIP messages'
        TabOrder = 2
      end
      object chbLogFlush: TCheckBox
        Left = 5
        Top = 25
        Width = 325
        Height = 17
        Caption = 'Flush log file on each line added'
        TabOrder = 3
      end
      object cbLogMaxFileSize: TComboBox
        Left = 365
        Top = 45
        Width = 111
        Height = 21
        ItemHeight = 13
        TabOrder = 4
        Items.Strings = (
          '1048576'
          '10485760'
          '104857600')
      end
      object cbLogRotate: TComboBox
        Left = 365
        Top = 69
        Width = 111
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 1
        TabOrder = 5
        Text = '1'
        Items.Strings = (
          '0'
          '1'
          '2'
          '3'
          '4'
          '5')
      end
      object edLoggingConsoleFont: TEdit
        Left = 98
        Top = 149
        Width = 201
        Height = 21
        TabOrder = 6
        Text = 'abcABC 123 () [] <>,.'
      end
      object btnLoggingConsoleFontSelect: TButton
        Left = 305
        Top = 147
        Width = 26
        Height = 25
        Caption = '...'
        TabOrder = 7
        OnClick = btnLoggingConsoleFontSelectClick
      end
    end
  end
  object tvSelector: TTreeView
    Left = 0
    Top = 0
    Width = 117
    Height = 392
    Align = alLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    HideSelection = False
    Indent = 19
    ParentFont = False
    ReadOnly = True
    RowSelect = True
    ShowRoot = False
    TabOrder = 0
    OnChange = tvSelectorChange
    Items.NodeData = {
      0114000000270000000000000000000000FFFFFFFFFFFFFFFF00000000000000
      0007470065006E006500720061006C00270000000000000000000000FFFFFFFF
      FFFFFFFF0000000000000000074E006500740077006F0072006B002F00000000
      00000000000000FFFFFFFFFFFFFFFF00000000000000000B5300490050002000
      6100630063006F0075006E0074002D0000000000000000000000FFFFFFFFFFFF
      FFFF00000000000000000A5300700065006500640020004400690061006C0023
      0000000000000000000000FFFFFFFFFFFFFFFF00000000000000000543006100
      6C006C007300270000000000000000000000FFFFFFFFFFFFFFFF000000000000
      00000744006900730070006C0061007900270000000000000000000000FFFFFF
      FFFFFFFFFF0000000000000000074C006F0063006B0069006E00670021000000
      0000000000000000FFFFFFFFFFFFFFFF000000000000000004520069006E0067
      002B0000000000000000000000FFFFFFFFFFFFFFFF0000000000000000094100
      7500640069006F00200049002F004F00390000000000000000000000FFFFFFFF
      FFFFFFFF00000000000000001041007500640069006F002000500072006F0063
      0065007300730069006E0067002B0000000000000000000000FFFFFFFFFFFFFF
      FF0000000000000000095200650063006F007200640069006E00670025000000
      0000000000000000FFFFFFFFFFFFFFFF00000000000000000643006F00640065
      00630073002F0000000000000000000000FFFFFFFFFFFFFFFF00000000000000
      000B49006E0074006500670072006100740069006F006E002700000000000000
      00000000FFFFFFFFFFFFFFFF00000000000000000748006F0074006B00650079
      007300290000000000000000000000FFFFFFFFFFFFFFFF000000000000000008
      43006F006E0074006100630074007300270000000000000000000000FFFFFFFF
      FFFFFFFF00000000000000000748006900730074006F00720079003900000000
      00000000000000FFFFFFFFFFFFFFFF000000000000000010500068006F006E00
      650073002000280070006C007500670069006E00730029003300000000000000
      00000000FFFFFFFFFFFFFFFF00000000000000000D540072006100790020004E
      006F00740069006600690065007200270000000000000000000000FFFFFFFFFF
      FFFFFF0000000000000000075300630072006900700074007300270000000000
      000000000000FFFFFFFFFFFFFFFF0000000000000000074C006F006700670069
      006E006700}
  end
  object dlgOpenRing: TOpenDialog
    Filter = 'WAVE files (*.wav)|*.wav'
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 320
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 48
    Top = 320
  end
  object fontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [fdEffects, fdForceFontExist]
    Left = 88
    Top = 320
  end
end

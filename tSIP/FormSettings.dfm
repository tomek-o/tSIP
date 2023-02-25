object frmSettings: TfrmSettings
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Settings'
  ClientHeight = 505
  ClientWidth = 700
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
    Top = 468
    Width = 700
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 2
    object btnCancel: TButton
      Left = 593
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 512
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object pcGeneral: TPageControl
    Left = 128
    Top = 0
    Width = 572
    Height = 468
    ActivePage = tsVideoCodecs
    Align = alClient
    TabHeight = 10
    TabOrder = 1
    TabStop = False
    object tsGeneral: TTabSheet
      Caption = 'General'
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblGuiScaling: TLabel
        Left = 3
        Top = 191
        Width = 140
        Height = 13
        Caption = 'GUI scaling (restart required)'
      end
      object lblGuiScalingPct: TLabel
        Left = 226
        Top = 191
        Width = 11
        Height = 13
        Caption = '%'
      end
      object lblTranslation: TLabel
        Left = 3
        Top = 7
        Width = 53
        Height = 13
        Caption = 'Translation'
      end
      object lblTranslateInfo: TLabel
        Left = 320
        Top = 7
        Width = 101
        Height = 13
        Caption = '(may require restart)'
      end
      object chbAlwaysOnTop: TCheckBox
        Left = 3
        Top = 51
        Width = 325
        Height = 17
        Caption = 'Window always on top'
        TabOrder = 0
      end
      object chbStartMinimizedToTray: TCheckBox
        Left = 3
        Top = 74
        Width = 325
        Height = 17
        Caption = 'Start minimized to tray'
        TabOrder = 1
      end
      object edGuiScaling: TEdit
        Left = 178
        Top = 188
        Width = 42
        Height = 21
        TabOrder = 2
      end
      object chbXBtnMinimize: TCheckBox
        Left = 3
        Top = 96
        Width = 325
        Height = 17
        Caption = 'Minimize to tray when "X" button is pressed'
        TabOrder = 3
      end
      object chbRestoreMainWindowOnIncomingCall: TCheckBox
        Left = 3
        Top = 119
        Width = 382
        Height = 17
        Caption = 'Restore minimized or bring to front main window on incoming call'
        TabOrder = 4
      end
      object chbSingleInstance: TCheckBox
        Left = 3
        Top = 236
        Width = 382
        Height = 17
        Caption = 
          'Single instance (bring previous instance to front if already run' +
          'ning)'
        TabOrder = 5
      end
      object chbNoBeepOnEnterKey: TCheckBox
        Left = 3
        Top = 257
        Width = 382
        Height = 17
        Caption = 
          'Do not beep when using [Enter] in main window to dial or transfe' +
          'r call'
        TabOrder = 6
      end
      object chbFrmMainShowWhenAnsweringCall: TCheckBox
        Left = 3
        Top = 142
        Width = 446
        Height = 17
        Caption = 
          'Show main form when answering call (using tray notifier, shortcu' +
          't, script, ...)'
        TabOrder = 7
      end
      object chbFrmMainShowWhenMakingCall: TCheckBox
        Left = 3
        Top = 165
        Width = 478
        Height = 17
        Caption = 
          'Show main form when making call (using shortcut, script, command' +
          ' line / protocol handler...)'
        TabOrder = 8
      end
      object chbNoTaskbarButtonRestore: TCheckBox
        Left = 3
        Top = 280
        Width = 526
        Height = 17
        Caption = 
          'Do not restore taskbar button when restoring application from tr' +
          'ay'
        TabOrder = 9
      end
      object chbNoTrayIcon: TCheckBox
        Left = 3
        Top = 213
        Width = 526
        Height = 17
        Caption = 'Do not create tray icon (restart required)'
        TabOrder = 10
      end
      object cbTranslation: TComboBox
        Left = 75
        Top = 4
        Width = 230
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 11
      end
      object chbTranslationLogMissingKeys: TCheckBox
        Left = 75
        Top = 31
        Width = 325
        Height = 17
        Caption = 'Log keys missing in translation file'
        TabOrder = 12
      end
    end
    object tsNetwork: TTabSheet
      Caption = 'Network'
      ImageIndex = 14
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblLocalAddress: TLabel
        Left = 3
        Top = 3
        Width = 210
        Height = 13
        Caption = 'Local IP address (+ optional port to bind to)'
      end
      object lblRtpPortRange: TLabel
        Left = 3
        Top = 152
        Width = 130
        Height = 13
        Caption = 'RTP/RTCP local  port range'
      end
      object lblRtpPortMin: TLabel
        Left = 205
        Top = 152
        Width = 16
        Height = 13
        Caption = 'min'
      end
      object lblRtpPortMax: TLabel
        Left = 280
        Top = 152
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
        Top = 176
        Width = 151
        Height = 13
        Caption = 'RTP jitter buffer delay (frames)'
      end
      object lblJbufDelayMin: TLabel
        Left = 205
        Top = 176
        Width = 16
        Height = 13
        Caption = 'min'
      end
      object lblJbufDelayMax: TLabel
        Left = 280
        Top = 176
        Width = 20
        Height = 13
        Caption = 'max'
      end
      object lblIfName: TLabel
        Left = 3
        Top = 65
        Width = 435
        Height = 13
        Caption = 
          'Network interface (optional) - binding to network interface inst' +
          'ead of binding to IP address'
      end
      object lblRtpTimeout: TLabel
        Left = 3
        Top = 200
        Width = 140
        Height = 13
        Caption = 'RTP timeout; 0 = disabled [s]'
      end
      object lblNetworkInterfaceInfo: TLabel
        Left = 24
        Top = 107
        Width = 186
        Height = 13
        Caption = 'Interface name (GUID) or empty string'
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
        Top = 149
        Width = 42
        Height = 21
        TabOrder = 1
      end
      object edRtpPortMax: TEdit
        Left = 305
        Top = 149
        Width = 42
        Height = 21
        TabOrder = 2
      end
      object edJbufDelayMin: TEdit
        Left = 227
        Top = 173
        Width = 42
        Height = 21
        TabOrder = 3
      end
      object edJbufDelayMax: TEdit
        Left = 305
        Top = 173
        Width = 42
        Height = 21
        TabOrder = 4
      end
      object edIfName: TEdit
        Left = 227
        Top = 104
        Width = 334
        Height = 21
        TabOrder = 5
      end
      object edRtpTimeout: TEdit
        Left = 227
        Top = 197
        Width = 42
        Height = 21
        TabOrder = 6
        Text = '0'
      end
      object cbNetworkInterfaces: TComboBox
        Left = 24
        Top = 81
        Width = 537
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 7
        OnChange = cbNetworkInterfacesChange
      end
    end
    object tsAccount: TTabSheet
      Caption = 'SIP account'
      ImageIndex = 2
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object pnlAccountsBottom: TPanel
        Left = 0
        Top = 417
        Width = 564
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
        Width = 564
        Height = 417
        Align = alClient
        TabOrder = 1
      end
    end
    object tsTls: TTabSheet
      Caption = 'TLS'
      ImageIndex = 25
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblTlsClientCertificate: TLabel
        Left = 5
        Top = 3
        Width = 78
        Height = 13
        Caption = 'Client certificate'
      end
      object Label8: TLabel
        Left = 5
        Top = 26
        Width = 31
        Height = 13
        Caption = 'CA file'
      end
      object chbTlsVerifyServerCertificate: TCheckBox
        Left = 5
        Top = 66
        Width = 526
        Height = 17
        Caption = 'Verify server certificate'
        TabOrder = 0
      end
      object edTlsCertificate: TEdit
        Left = 96
        Top = 0
        Width = 180
        Height = 21
        TabOrder = 1
      end
      object edTlsCaFile: TEdit
        Left = 96
        Top = 23
        Width = 180
        Height = 21
        TabOrder = 2
      end
      object btnSelectTlsCaFile: TButton
        Left = 287
        Top = 23
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 3
        OnClick = btnSelectTlsClick
      end
      object btnSelectTlsCertificate: TButton
        Left = 287
        Top = 0
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 4
        OnClick = btnSelectTlsClick
      end
      object chbTlsUseWindowsRootCaStore: TCheckBox
        Left = 5
        Top = 47
        Width = 526
        Height = 17
        Caption = 'Use CA certificates from Windows root store'
        TabOrder = 5
      end
    end
    object tsMainWindow: TTabSheet
      Caption = 'Main window'
      ImageIndex = 23
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblMainWindowCollapsedSize: TLabel
        Left = 3
        Top = 3
        Width = 230
        Height = 13
        Caption = 'Main window size in collapsed (no console) state'
      end
      object lblMainWindowExpandedSize: TLabel
        Left = 3
        Top = 24
        Width = 172
        Height = 13
        Caption = 'Main window size in expanded state'
      end
      object lblFrmMainMainPanelPositionCollapsed: TLabel
        Left = 2
        Top = 238
        Width = 310
        Height = 13
        Caption = 
          'Main panel (dialpad, contacts, history) position in collapsed st' +
          'ate'
      end
      object lblFrmMainMainlPanelPositionExpanded: TLabel
        Left = 2
        Top = 259
        Width = 181
        Height = 13
        Caption = 'Main panel position in expanded state'
      end
      object lblCollapsedWidth: TLabel
        Left = 336
        Top = 5
        Width = 26
        Height = 13
        Caption = 'width'
      end
      object lblExpandedWidth: TLabel
        Left = 336
        Top = 24
        Width = 26
        Height = 13
        Caption = 'width'
      end
      object lblMainPanelCollapsedLeft: TLabel
        Left = 335
        Top = 238
        Width = 16
        Height = 13
        Caption = 'left'
      end
      object lblMainPanelExpandedLeft: TLabel
        Left = 335
        Top = 259
        Width = 16
        Height = 13
        Caption = 'left'
      end
      object lblCollapsedHeight: TLabel
        Left = 416
        Top = 3
        Width = 30
        Height = 13
        Caption = 'height'
      end
      object lblExpandedHeight: TLabel
        Left = 416
        Top = 24
        Width = 30
        Height = 13
        Caption = 'height'
      end
      object lblMainPanelCollapsedTop: TLabel
        Left = 415
        Top = 238
        Width = 16
        Height = 13
        Caption = 'top'
      end
      object lblMainPanelExpandedTop: TLabel
        Left = 415
        Top = 259
        Width = 16
        Height = 13
        Caption = 'top'
      end
      object lblFrmMainLayout: TLabel
        Left = 3
        Top = 121
        Width = 33
        Height = 13
        Caption = 'Layout'
      end
      object lblDialComboboxSortOrder: TLabel
        Left = 3
        Top = 304
        Width = 134
        Height = 13
        Caption = 'Dialing combobox sort order'
      end
      object lblFrmMainCallPanelPositionCollapsed: TLabel
        Left = 3
        Top = 168
        Width = 305
        Height = 13
        Caption = 
          'Call panel (dialpad, contacts, history) position in collapsed st' +
          'ate'
      end
      object lblFrmMainCallPanelPositionExpanded: TLabel
        Left = 3
        Top = 189
        Width = 176
        Height = 13
        Caption = 'Call panel position in expanded state'
      end
      object lblCallPanelCollapsedLeft: TLabel
        Left = 336
        Top = 168
        Width = 16
        Height = 13
        Caption = 'left'
      end
      object lblCallPanelExpandedLeft: TLabel
        Left = 336
        Top = 189
        Width = 16
        Height = 13
        Caption = 'left'
      end
      object lblCallPanelCollapsedTop: TLabel
        Left = 416
        Top = 168
        Width = 16
        Height = 13
        Caption = 'top'
      end
      object lblCallPanelExpandedTop: TLabel
        Left = 416
        Top = 189
        Width = 16
        Height = 13
        Caption = 'top'
      end
      object lblMainWindowPosOffset: TLabel
        Left = 3
        Top = 71
        Width = 215
        Height = 13
        Caption = 'Main window position offset when expanding'
      end
      object lblExpandingPosLeftOffset: TLabel
        Left = 336
        Top = 71
        Width = 16
        Height = 13
        Caption = 'left'
      end
      object lblExpandingPosTopOffset: TLabel
        Left = 416
        Top = 71
        Width = 30
        Height = 13
        Caption = 'height'
      end
      object chbFrmMainHideCallPanel: TCheckBox
        Left = 2
        Top = 147
        Width = 486
        Height = 17
        Caption = 
          'Hide call panel (dialing combobox, call state labels, Call and H' +
          'angup buttons)'
        TabOrder = 0
      end
      object edMainPanelExpandedLeft: TEdit
        Left = 365
        Top = 256
        Width = 35
        Height = 21
        TabOrder = 1
      end
      object edMainPanelCollapsedLeft: TEdit
        Left = 365
        Top = 235
        Width = 35
        Height = 21
        TabOrder = 2
      end
      object edExpandedWidth: TEdit
        Left = 366
        Top = 21
        Width = 35
        Height = 21
        TabOrder = 3
      end
      object edCollapsedWidth: TEdit
        Left = 366
        Top = 0
        Width = 35
        Height = 21
        TabOrder = 4
      end
      object edMainPanelExpandedTop: TEdit
        Left = 451
        Top = 256
        Width = 35
        Height = 21
        TabOrder = 5
      end
      object edMainPanelCollapsedTop: TEdit
        Left = 451
        Top = 235
        Width = 35
        Height = 21
        TabOrder = 6
      end
      object edExpandedHeight: TEdit
        Left = 452
        Top = 21
        Width = 35
        Height = 21
        TabOrder = 7
      end
      object edCollapsedHeight: TEdit
        Left = 452
        Top = 0
        Width = 35
        Height = 21
        TabOrder = 8
      end
      object cbFrmMainLayout: TComboBox
        Left = 152
        Top = 118
        Width = 337
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 9
        Text = 
          'default, call panel above main panel (Dialpad, Contacts, History' +
          ')'
        Items.Strings = (
          
            'default, call panel above main panel (Dialpad, Contacts, History' +
            ')'
          'call panel inside Dialpad tab')
      end
      object cbDialComboboxOrder: TComboBox
        Left = 152
        Top = 301
        Width = 145
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 10
        Text = 'by number'
        Items.Strings = (
          'by number'
          'by date/time')
      end
      object chbFrmMainHideMainPanel: TCheckBox
        Left = 2
        Top = 217
        Width = 486
        Height = 17
        Caption = 'Hide main panel (Dialpad/Contacts/History)'
        TabOrder = 11
      end
      object edCallPanelCollapsedLeft: TEdit
        Left = 366
        Top = 165
        Width = 35
        Height = 21
        TabOrder = 12
      end
      object edCallPanelExpandedLeft: TEdit
        Left = 366
        Top = 186
        Width = 35
        Height = 21
        TabOrder = 13
      end
      object edCallPanelCollapsedTop: TEdit
        Left = 452
        Top = 165
        Width = 35
        Height = 21
        TabOrder = 14
      end
      object edCallPanelExpandedTop: TEdit
        Left = 452
        Top = 186
        Width = 35
        Height = 21
        TabOrder = 15
      end
      object chbFrmMainUseClientAreaSizes: TCheckBox
        Left = 3
        Top = 48
        Width = 486
        Height = 17
        Caption = 'Use client area sizes instead of absolute sizes'
        TabOrder = 16
      end
      object edExpandingPosLeftOffset: TEdit
        Left = 366
        Top = 68
        Width = 35
        Height = 21
        TabOrder = 17
      end
      object edExpandingPosTopOffset: TEdit
        Left = 452
        Top = 68
        Width = 35
        Height = 21
        TabOrder = 18
      end
    end
    object tsDialpad: TTabSheet
      Margins.Left = 0
      Margins.Top = 0
      Margins.Right = 0
      Margins.Bottom = 0
      Caption = 'Dialpad'
      ImageIndex = 24
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
    end
    object tsSpeedDial: TTabSheet
      Caption = 'Speed Dial'
      ImageIndex = 12
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblSpeedDialBlfSettings: TLabel
        Left = 2
        Top = 56
        Width = 122
        Height = 13
        Caption = 'BLF / dialog-info settings:'
      end
      object lblDialogInfoPreferredState: TLabel
        Left = 18
        Top = 147
        Width = 335
        Height = 13
        Caption = 
          'Preferred state if multiple dialog elements are received in NOTI' +
          'FY XML'
      end
      object chbSpeedDialPopupMenu: TCheckBox
        Left = 2
        Top = 3
        Width = 325
        Height = 17
        Caption = 'Enable popup menu (editing) for programmable keys'
        TabOrder = 0
      end
      object chbSpeedDialIgnorePresenceNote: TCheckBox
        Left = 2
        Top = 26
        Width = 398
        Height = 17
        Caption = 
          'Ignore presence note (do not show second line for presence butto' +
          'n)'
        TabOrder = 1
      end
      object chbSpeedDialIgnoreDialogInfoRemoteIdentity: TCheckBox
        Left = 18
        Top = 78
        Width = 486
        Height = 17
        Caption = 
          'Ignore remote identity info (2nd line with caller/callee and cal' +
          'l direction, if present in notifications)'
        TabOrder = 2
      end
      object chbSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing: TCheckBox
        Left = 18
        Top = 101
        Width = 478
        Height = 17
        Caption = 
          'Keep previous remote identity info if remote identity is missing' +
          ' in notification'
        TabOrder = 3
      end
      object chbSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated: TCheckBox
        Left = 18
        Top = 124
        Width = 478
        Height = 17
        Caption = 
          'Ignore or clear remote identity if call state is set to terminat' +
          'ed'
        TabOrder = 4
      end
      object chbSpeedDialUseGrid: TCheckBox
        Left = 3
        Top = 176
        Width = 401
        Height = 17
        Caption = 
          'Snap to grid when moving/resizing buttons (hold Shift to overrid' +
          'e)     grid size:'
        TabOrder = 5
      end
      object edSpeedDialGridSize: TEdit
        Left = 400
        Top = 174
        Width = 31
        Height = 21
        TabOrder = 6
      end
      object chbSpeedDialSaveAllSettings: TCheckBox
        Left = 3
        Top = 199
        Width = 526
        Height = 17
        Caption = 
          'Save all button settings, even same as default (larger JSON file' +
          ' but possibly easier to edit or compare)'
        TabOrder = 7
      end
      object cbDialogInfoPreferredState: TComboBox
        Left = 359
        Top = 144
        Width = 138
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 8
        Text = 'first one'
        Items.Strings = (
          'first one'
          'early (ringing)'
          'confirmed')
      end
      object chbSpeedDialDragApplicationWithButtonContainer: TCheckBox
        Left = 3
        Top = 222
        Width = 526
        Height = 17
        Caption = 'Move application window when dragging over button container'
        TabOrder = 9
      end
    end
    object tsCalls: TTabSheet
      Caption = 'Calls'
      ImageIndex = 4
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
      object lblSipAutoAnswerReason: TLabel
        Left = 342
        Top = 29
        Width = 33
        Height = 13
        Caption = 'reason'
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
      object edAutoAnswerReason: TEdit
        Left = 381
        Top = 26
        Width = 148
        Height = 21
        TabOrder = 10
      end
    end
    object tsMessages: TTabSheet
      Caption = 'Messages'
      ImageIndex = 22
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblMessagesRing: TLabel
        Left = 5
        Top = 3
        Width = 136
        Height = 13
        Caption = 'Sound for incoming message'
      end
      object lbMessagesReply: TLabel
        Left = 5
        Top = 35
        Width = 202
        Height = 13
        Caption = 'Reply to incoming messages with SIP code'
      end
      object lblMessagesReply2: TLabel
        Left = 263
        Top = 35
        Width = 85
        Height = 13
        Caption = 'and reason (text)'
      end
      object edMessagesRing: TEdit
        Left = 160
        Top = 0
        Width = 121
        Height = 21
        TabOrder = 0
      end
      object btnMessagesSelectRing: TButton
        Left = 285
        Top = 0
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 1
        OnClick = btnMessagesSelectRingClick
      end
      object edMessagesReplyCode: TEdit
        Left = 213
        Top = 32
        Width = 44
        Height = 21
        TabOrder = 2
      end
      object edMessagesReplyReason: TEdit
        Left = 354
        Top = 32
        Width = 175
        Height = 21
        TabOrder = 3
      end
      object chbMessagesDoNotReply: TCheckBox
        Left = 5
        Top = 64
        Width = 524
        Height = 17
        Caption = 
          'Do not reply to incoming messages  (ONLY for SIP testing purpose' +
          's)'
        TabOrder = 4
      end
    end
    object tsDisplay: TTabSheet
      Caption = 'Display'
      ImageIndex = 15
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Caption = 'Kiosk mode (fullscreen, no main menu, no status bar, no border)'
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
      object chbHideStatusBar: TCheckBox
        Left = 3
        Top = 147
        Width = 398
        Height = 17
        Caption = 'Hide status bar'
        TabOrder = 7
      end
      object chbHideMainMenu: TCheckBox
        Left = 3
        Top = 168
        Width = 398
        Height = 17
        Caption = 'Hide main menu'
        TabOrder = 8
      end
      object chbHideDialpad: TCheckBox
        Left = 3
        Top = 214
        Width = 398
        Height = 17
        Caption = 'Hide Dialpad'
        TabOrder = 9
      end
      object chbFrmMainBorderless: TCheckBox
        Left = 3
        Top = 191
        Width = 398
        Height = 17
        Caption = 'Make main window borderless; this removes also main menu'
        TabOrder = 10
      end
    end
    object tsLockingSettingsPages: TTabSheet
      Caption = 'Settings pages'
      ImageIndex = 26
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblSettingsPagesToHide: TLabel
        Left = 3
        Top = 3
        Width = 357
        Height = 13
        Caption = 
          'Settings pages to hide (edit JSON directly to unhide "Locking" o' +
          'r this page)'
      end
      object chbBoxLockingSettingsPages: TCheckListBox
        Left = 3
        Top = 22
        Width = 534
        Height = 291
        Columns = 2
        ItemHeight = 13
        TabOrder = 0
      end
    end
    object tsBranding: TTabSheet
      Caption = 'Branding, bitmaps'
      ImageIndex = 20
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblMainIconFile: TLabel
        Left = 3
        Top = 6
        Width = 128
        Height = 13
        Caption = 'Custom main icon file (.ico)'
      end
      object lblRestartRequired1: TLabel
        Left = 393
        Top = 6
        Width = 84
        Height = 13
        Caption = '(restart required)'
      end
      object lblRestartRequired2: TLabel
        Left = 393
        Top = 30
        Width = 84
        Height = 13
        Caption = '(restart required)'
      end
      object lblTrayNotificationIcon: TLabel
        Left = 3
        Top = 30
        Width = 168
        Height = 13
        Caption = 'Custom tray icon notification image'
      end
      object lblBmpBtnBackspace: TLabel
        Left = 3
        Top = 128
        Width = 128
        Height = 13
        Caption = '"Backspace" button bitmap'
      end
      object lblBmpConsoleHide: TLabel
        Left = 3
        Top = 178
        Width = 131
        Height = 13
        Caption = 'Console hide button bitmap'
      end
      object lblBmpConsoleShow: TLabel
        Left = 3
        Top = 153
        Width = 136
        Height = 13
        Caption = 'Console show button bitmap'
      end
      object lblBmpBtnResetMicVolume: TLabel
        Left = 3
        Top = 204
        Width = 153
        Height = 13
        Caption = 'Reset mic volume button bitmap'
      end
      object lblBmpBtnResetSpeakerVolume: TLabel
        Left = 3
        Top = 230
        Width = 176
        Height = 13
        Caption = 'Reset speaker volume button bitmap'
      end
      object lblBrandingAppUrl: TLabel
        Left = 3
        Top = 256
        Width = 109
        Height = 13
        Caption = 'URL in "About" window'
      end
      object edMainIconFile: TEdit
        Left = 231
        Top = 3
        Width = 121
        Height = 21
        TabOrder = 0
      end
      object btnSelectMainIconFile: TButton
        Left = 358
        Top = 3
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 1
        OnClick = btnSelectImageClick
      end
      object btnSelectTrayNotificationImage: TButton
        Left = 358
        Top = 27
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 2
        OnClick = btnSelectImageClick
      end
      object edTrayNotificationImage: TEdit
        Left = 231
        Top = 27
        Width = 121
        Height = 21
        TabOrder = 3
      end
      object chbCustomUserAgent: TCheckBox
        Left = 3
        Top = 54
        Width = 157
        Height = 17
        Caption = 'Custom User-Agent value'
        TabOrder = 4
      end
      object chbFrmMainUseCustomApplicationTitle: TCheckBox
        Left = 3
        Top = 77
        Width = 223
        Height = 17
        Caption = 'Custom application title (text in taskbar)'
        TabOrder = 5
      end
      object chbFrmMainUseCustomCaption: TCheckBox
        Left = 3
        Top = 101
        Width = 191
        Height = 17
        Caption = 'Custom caption for main window'
        TabOrder = 6
      end
      object edUserAgent: TEdit
        Left = 231
        Top = 52
        Width = 121
        Height = 21
        TabOrder = 7
      end
      object edFrmMainCustomApplicationTitle: TEdit
        Left = 231
        Top = 75
        Width = 193
        Height = 21
        TabOrder = 8
      end
      object edFrmMainCustomCaption: TEdit
        Left = 231
        Top = 99
        Width = 193
        Height = 21
        TabOrder = 9
      end
      object edBmpBtnBackspace: TEdit
        Left = 231
        Top = 125
        Width = 121
        Height = 21
        TabOrder = 10
      end
      object btnSelectBmpBtnBackspace: TButton
        Left = 358
        Top = 125
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 11
        OnClick = btnSelectImageClick
      end
      object edBmpBtnConsoleHide: TEdit
        Left = 231
        Top = 175
        Width = 121
        Height = 21
        TabOrder = 12
      end
      object btnSelectBmpBtnConsoleHide: TButton
        Left = 358
        Top = 175
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 13
        OnClick = btnSelectImageClick
      end
      object edBmpBtnConsoleShow: TEdit
        Left = 231
        Top = 150
        Width = 121
        Height = 21
        TabOrder = 14
      end
      object btnSelectBmpBtnConsoleShow: TButton
        Left = 358
        Top = 150
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 15
        OnClick = btnSelectImageClick
      end
      object edBmpBtnResetMicVolume: TEdit
        Left = 231
        Top = 201
        Width = 121
        Height = 21
        TabOrder = 16
      end
      object btnSelectBmpBtnResetMicVolume: TButton
        Left = 358
        Top = 201
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 17
        OnClick = btnSelectImageClick
      end
      object edBmpBtnResetSpeakerVolume: TEdit
        Left = 231
        Top = 227
        Width = 121
        Height = 21
        TabOrder = 18
      end
      object btnSelectBmpBtnResetSpeakerVolume: TButton
        Left = 358
        Top = 227
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 19
        OnClick = btnSelectImageClick
      end
      object edBrandingAppUrl: TEdit
        Left = 231
        Top = 253
        Width = 193
        Height = 21
        TabOrder = 20
      end
    end
    object tsRing: TTabSheet
      Caption = 'Ring'
      ImageIndex = 8
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Width = 180
        Height = 21
        TabOrder = 0
      end
      object btnRingSelectDefault: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 2
      end
      object btnRingSelectBellcoreDr1: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 4
      end
      object btnRingSelectBellcoreDr2: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 6
      end
      object btnRingSelectBellcoreDr3: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 8
      end
      object btnRingSelectBellcoreDr4: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 10
      end
      object btnRingSelectBellcoreDr5: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 12
      end
      object btnRingSelectBellcoreDr6: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 14
      end
      object btnRingSelectBellcoreDr7: TButton
        Left = 287
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
        Width = 180
        Height = 21
        TabOrder = 16
      end
      object btnRingSelectBellcoreDr8: TButton
        Left = 287
        Top = 184
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 17
        OnClick = btnRingSelectClick
      end
      object chbLoopRingWithoutSilence: TCheckBox
        Left = 3
        Top = 224
        Width = 238
        Height = 17
        Caption = 'Loop ring without silence'
        TabOrder = 18
      end
    end
    object tsAudioIO: TTabSheet
      Caption = 'Audio I/O'
      ImageIndex = 5
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblSoundInputModule: TLabel
        Left = 30
        Top = 20
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
        Left = 30
        Top = 47
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioOutput: TLabel
        Left = 3
        Top = 65
        Width = 34
        Height = 13
        Caption = 'Output'
      end
      object Label3: TLabel
        Left = 30
        Top = 84
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundOutputDev: TLabel
        Left = 30
        Top = 108
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioAlertOutput: TLabel
        Left = 3
        Top = 128
        Width = 241
        Height = 13
        Caption = 'Alert output (call progress, ringback, busy signals)'
      end
      object Label6: TLabel
        Left = 30
        Top = 147
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundAlertOutputDev: TLabel
        Left = 30
        Top = 171
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioOutputIntercom: TLabel
        Left = 3
        Top = 252
        Width = 131
        Height = 13
        Caption = 'Output for paging/intercom'
      end
      object Label2: TLabel
        Left = 30
        Top = 271
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundOutputIntercomDev: TLabel
        Left = 30
        Top = 295
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblAudioRingOutput: TLabel
        Left = 3
        Top = 189
        Width = 141
        Height = 13
        Caption = 'Ring output (on incoming call)'
      end
      object Label9: TLabel
        Left = 30
        Top = 208
        Width = 34
        Height = 13
        Caption = 'Module'
      end
      object lblSoundRingOutputDev: TLabel
        Left = 30
        Top = 232
        Width = 32
        Height = 13
        Caption = 'Device'
      end
      object lblPortaudio: TLabel
        Left = 3
        Top = 344
        Width = 138
        Height = 13
        Caption = 'PortAudio suggested latency'
      end
      object lblPoraudioIn: TLabel
        Left = 184
        Top = 344
        Width = 28
        Height = 13
        Caption = 'in [s]:'
      end
      object lblPortaudioOut: TLabel
        Left = 288
        Top = 344
        Width = 36
        Height = 13
        Caption = 'out [s]:'
      end
      object cbSoundInputMod: TComboBox
        Left = 121
        Top = 17
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 0
        OnChange = cbSoundInputModChange
      end
      object cbSoundInputDev: TComboBox
        Left = 121
        Top = 41
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 1
      end
      object cbSoundOutputMod: TComboBox
        Left = 121
        Top = 81
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 2
        OnChange = cbSoundOutputModChange
      end
      object cbSoundOutputDev: TComboBox
        Left = 121
        Top = 105
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 3
      end
      object cbSoundAlertOutputMod: TComboBox
        Left = 121
        Top = 144
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 4
        OnChange = cbSoundAlertOutputModChange
      end
      object cbSoundAlertOutputDev: TComboBox
        Left = 121
        Top = 168
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 5
      end
      object cbSoundOutputIntercomMod: TComboBox
        Left = 121
        Top = 268
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 6
        OnChange = cbSoundOutputIntercomModChange
      end
      object cbSoundOutputIntercomDev: TComboBox
        Left = 121
        Top = 292
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 7
      end
      object btnSelectWaveFile: TButton
        Left = 367
        Top = 43
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 8
        OnClick = btnSelectWaveFileClick
      end
      object edSoundInputWave: TEdit
        Left = 121
        Top = 41
        Width = 240
        Height = 21
        TabOrder = 9
      end
      object cbSoundRingOutputDev: TComboBox
        Left = 121
        Top = 229
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 10
      end
      object cbSoundRingOutputMod: TComboBox
        Left = 121
        Top = 205
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        TabOrder = 11
        OnChange = cbSoundRingOutputModChange
      end
      object edPortaudioInSuggestedLatency: TEdit
        Left = 218
        Top = 341
        Width = 49
        Height = 21
        TabOrder = 12
      end
      object edPortaudioOutSuggestedLatency: TEdit
        Left = 330
        Top = 341
        Width = 49
        Height = 21
        TabOrder = 13
      end
      object chbStartAudioSourceAtCallStart: TCheckBox
        Left = 3
        Top = 367
        Width = 534
        Height = 17
        Caption = 
          'Start audio input (e.g. microphone) before call is confirmed (mi' +
          'ght help with some Bluetooth devices delay)'
        TabOrder = 14
      end
    end
    object tsAudioProcessing: TTabSheet
      Caption = 'Audio Processing'
      ImageIndex = 6
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Left = 95
        Top = 37
        Width = 101
        Height = 13
        Caption = 'ms in snd card buffer'
      end
      object lblWebRtcAecClockSkew: TLabel
        Left = 309
        Top = 37
        Width = 50
        Height = 13
        Caption = 'clock skew'
      end
      object lblAudioPreprocessingTxAgcLevel: TLabel
        Left = 171
        Top = 114
        Width = 22
        Height = 13
        Caption = 'level'
      end
      object lblAudioAgcRxLevel: TLabel
        Left = 18
        Top = 333
        Width = 55
        Height = 13
        Caption = 'target level'
      end
      object lblAudioAgcRxMaxGain: TLabel
        Left = 18
        Top = 355
        Width = 43
        Height = 13
        Caption = 'max gain'
      end
      object lblAudioAgcRxAttackRate: TLabel
        Left = 18
        Top = 377
        Width = 53
        Height = 13
        Caption = 'attack rate'
      end
      object lblAudioAgcRxReleaseRate: TLabel
        Left = 18
        Top = 399
        Width = 58
        Height = 13
        Caption = 'release rate'
      end
      object lblAudioTxGateCloseThreshold: TLabel
        Left = 18
        Top = 213
        Width = 72
        Height = 13
        Caption = 'close threshold'
      end
      object lblAudioTxGateHoldMs: TLabel
        Left = 18
        Top = 235
        Width = 67
        Height = 13
        Caption = 'hold time (ms)'
      end
      object lblAudioTxGateAttackMs: TLabel
        Left = 18
        Top = 257
        Width = 77
        Height = 13
        Caption = 'attack time (ms)'
      end
      object lblAudioTxGateReleaseMs: TLabel
        Left = 18
        Top = 279
        Width = 82
        Height = 13
        Caption = 'release time (ms)'
      end
      object cbAec: TComboBox
        Left = 122
        Top = 4
        Width = 206
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 2
        TabOrder = 0
        Text = 'WebRTC'
        Items.Strings = (
          'none'
          'Speex [not recommended]'
          'WebRTC')
      end
      object edWebRtcAecMsInSndCardBuf: TEdit
        Left = 206
        Top = 34
        Width = 75
        Height = 21
        TabOrder = 1
      end
      object edWebRtcAecSkew: TEdit
        Left = 370
        Top = 34
        Width = 75
        Height = 21
        TabOrder = 2
      end
      object chbAudioPreprocessingTxEnabled: TCheckBox
        Left = 2
        Top = 71
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
        Top = 92
        Width = 494
        Height = 17
        Caption = 'Enable denoise'
        TabOrder = 4
      end
      object chbAudioPreprocessingTxAgcEnabled: TCheckBox
        Left = 18
        Top = 113
        Width = 105
        Height = 17
        Caption = 'Enable AGC'
        TabOrder = 5
      end
      object edAudioPreprocessingTxAgcLevel: TEdit
        Left = 206
        Top = 111
        Width = 49
        Height = 21
        TabOrder = 6
      end
      object chbAudioPreprocessingTxVadEnabled: TCheckBox
        Left = 18
        Top = 134
        Width = 105
        Height = 17
        Caption = 'Enable VAD'
        TabOrder = 7
      end
      object chbAudioPreprocessingTxDereverbEnabled: TCheckBox
        Left = 18
        Top = 155
        Width = 105
        Height = 17
        Caption = 'Enable dereverb'
        TabOrder = 8
      end
      object chbAudioRxAgcEnabled: TCheckBox
        Left = 3
        Top = 310
        Width = 262
        Height = 17
        Caption = 'Enable AGC for RX (received audio) direction'
        TabOrder = 9
      end
      object edAudioRxAgcTarget: TEdit
        Left = 206
        Top = 330
        Width = 49
        Height = 21
        TabOrder = 10
      end
      object edAudioRxAgcMaxGain: TEdit
        Left = 206
        Top = 352
        Width = 49
        Height = 21
        TabOrder = 11
      end
      object edAudioRxAgcAttackRate: TEdit
        Left = 206
        Top = 374
        Width = 49
        Height = 21
        TabOrder = 12
      end
      object edAudioRxAgcReleaseRate: TEdit
        Left = 206
        Top = 396
        Width = 49
        Height = 21
        TabOrder = 13
      end
      object chbAudioTxGateEnabled: TCheckBox
        Left = 3
        Top = 190
        Width = 398
        Height = 17
        Caption = 'Enable audio gate microphone path (transmitted audio) direction'
        TabOrder = 14
      end
      object edAudioTxGateReleaseMs: TEdit
        Left = 206
        Top = 276
        Width = 49
        Height = 21
        TabOrder = 15
      end
      object edAudioTxGateAttackMs: TEdit
        Left = 206
        Top = 254
        Width = 49
        Height = 21
        TabOrder = 16
      end
      object edAudioTxGateHoldMs: TEdit
        Left = 206
        Top = 232
        Width = 49
        Height = 21
        TabOrder = 17
      end
      object edAudioTxGateCloseThreshold: TEdit
        Left = 206
        Top = 210
        Width = 49
        Height = 21
        TabOrder = 18
      end
    end
    object tsRecording: TTabSheet
      Caption = 'Recording'
      ImageIndex = 11
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Top = 173
        Width = 53
        Height = 13
        Caption = 'Start mode'
      end
      object lblRecordedSide: TLabel
        Left = 3
        Top = 200
        Width = 68
        Height = 13
        Caption = 'Recorded side'
      end
      object lblRecordingFileFormat: TLabel
        Left = 3
        Top = 120
        Width = 100
        Height = 13
        Caption = 'Recording file format'
      end
      object lblRecordingBitrate: TLabel
        Left = 3
        Top = 147
        Width = 142
        Height = 13
        Caption = 'Recording bitrate (Opus only)'
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
        ItemHeight = 0
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
        ItemHeight = 0
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
        Top = 170
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 1
        TabOrder = 5
        Text = 'automatic - call confirmed state'
        Items.Strings = (
          'manual (Lua script or button)'
          'automatic - call confirmed state'
          'automatic - call early media or call confirmed')
      end
      object cbRecordedSide: TComboBox
        Left = 167
        Top = 197
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 6
        Text = 'both local and remote (mixed)'
        OnChange = cbRecDirTypeChange
        Items.Strings = (
          'both local and remote (mixed)'
          'local party only (your microphone)'
          'remote party only')
      end
      object btnOpenRecordingFolder: TButton
        Left = 447
        Top = 34
        Width = 75
        Height = 21
        Caption = 'Open'
        TabOrder = 7
        OnClick = btnOpenRecordingFolderClick
      end
      object chbRecordingNoNumberB64Encoding: TCheckBox
        Left = 2
        Top = 232
        Width = 534
        Height = 17
        Caption = 
          'Do not encode number/URI with filesystem-safe base64; not compat' +
          'ible with RecordViewer.exe'
        TabOrder = 8
      end
      object cbRecordingFileFormat: TComboBox
        Left = 167
        Top = 117
        Width = 274
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 9
        Text = 'WAVE'
        OnChange = cbRecordingChannelsChange
        Items.Strings = (
          'WAVE'
          'Opus/OGG')
      end
      object edRecordingBitrate: TEdit
        Left = 168
        Top = 144
        Width = 121
        Height = 21
        TabOrder = 10
      end
    end
    object tsCodecs: TTabSheet
      Caption = 'Audio codecs'
      ImageIndex = 3
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
      object lblCodecsInfo: TLabel
        Left = 15
        Top = 352
        Width = 218
        Height = 13
        Caption = 'Double-click codec to move between the lists.'
      end
      object lblCodecsInfo2: TLabel
        Left = 15
        Top = 371
        Width = 307
        Height = 13
        Caption = 
          'Drag-and-drop items on enabled codecs list to reorder priorities' +
          '.'
      end
      object lboxAudioCodecsAvailable: TListBox
        Left = 15
        Top = 24
        Width = 150
        Height = 313
        ItemHeight = 13
        TabOrder = 0
        OnDblClick = lboxAudioCodecsAvailableDblClick
      end
      object lboxAudioCodecsEnabled: TListBox
        Left = 207
        Top = 24
        Width = 150
        Height = 313
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
        Top = 129
        Width = 30
        Height = 25
        Hint = 'Enable selected codec'
        Caption = '->'
        TabOrder = 2
        OnClick = btnAudioCodecEnableClick
      end
      object btnAudioCodecDisable: TButton
        Left = 171
        Top = 192
        Width = 30
        Height = 25
        Hint = 'Disable selected codec'
        Caption = '<-'
        TabOrder = 3
        OnClick = btnAudioCodecDisableClick
      end
    end
    object tsVideo: TTabSheet
      Caption = 'Video'
      ImageIndex = 28
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
    end
    object tsVideoCodecs: TTabSheet
      Caption = 'Video codecs'
      ImageIndex = 27
      object lblVideoCodecsAvailable: TLabel
        Left = 15
        Top = 5
        Width = 108
        Height = 13
        Caption = 'Available video codecs'
      end
      object lblVideoCodecsEnabled: TLabel
        Left = 207
        Top = 5
        Width = 151
        Height = 13
        Caption = 'Enabled, in order of preference'
      end
      object lblVideoCodecsInfo: TLabel
        Left = 15
        Top = 352
        Width = 218
        Height = 13
        Caption = 'Double-click codec to move between the lists.'
      end
      object lblVideoCodecsInfo2: TLabel
        Left = 15
        Top = 371
        Width = 307
        Height = 13
        Caption = 
          'Drag-and-drop items on enabled codecs list to reorder priorities' +
          '.'
      end
      object lboxVideoCodecsAvailable: TListBox
        Left = 15
        Top = 24
        Width = 150
        Height = 313
        ItemHeight = 13
        TabOrder = 0
        OnDblClick = lboxVideoCodecsAvailableDblClick
      end
      object btnVideoCodecEnable: TButton
        Left = 171
        Top = 129
        Width = 30
        Height = 25
        Hint = 'Enable selected codec'
        Caption = '->'
        TabOrder = 1
        OnClick = btnVideoCodecEnableClick
      end
      object btnVideoCodecDisable: TButton
        Left = 171
        Top = 192
        Width = 30
        Height = 25
        Hint = 'Disable selected codec'
        Caption = '<-'
        TabOrder = 2
        OnClick = btnVideoCodecDisableClick
      end
      object lboxVideoCodecsEnabled: TListBox
        Left = 207
        Top = 24
        Width = 150
        Height = 313
        DragMode = dmAutomatic
        ItemHeight = 13
        TabOrder = 3
        OnDblClick = lboxVideoCodecsEnabledDblClick
        OnDragDrop = lboxVideoCodecsEnabledDragDrop
        OnDragOver = lboxVideoCodecsEnabledDragOver
        OnStartDrag = lboxVideoCodecsEnabledStartDrag
      end
    end
    object tsIntegration: TTabSheet
      Caption = 'Integration'
      ImageIndex = 7
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Width = 364
        Height = 13
        Caption = 
          'Browser or e-mail client -> softphone ("click to call" / command' +
          ' line handling)'
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
        ItemHeight = 0
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
      object chbDoNotPassParametersToPreviousInstance: TCheckBox
        Left = 26
        Top = 122
        Width = 511
        Height = 17
        Caption = 
          'Do not pass parameters to previous instance (run new instance on' +
          ' each click / command line call)'
        TabOrder = 5
      end
      object chbAcceptCommandLineScript: TCheckBox
        Left = 26
        Top = 142
        Width = 511
        Height = 17
        Caption = 'Accept scripts from command line'
        TabOrder = 6
      end
    end
    object tsHotkeys: TTabSheet
      Caption = 'Hotkeys'
      ImageIndex = 9
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
    end
    object tsContacts: TTabSheet
      Caption = 'Contacts'
      ImageIndex = 9
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblContactPopup: TLabel
        Left = 2
        Top = 87
        Width = 233
        Height = 13
        Caption = 'Show contact popup if contact note is not empty'
      end
      object lblContactHttpQuery: TLabel
        Left = 2
        Top = 223
        Width = 490
        Height = 13
        Caption = 
          'HTTP query (opens browser with last incoming phone number passed' +
          ' as parameter on action execute)'
      end
      object lblHttpQueryStartMode: TLabel
        Left = 2
        Top = 272
        Width = 82
        Height = 13
        Caption = 'HTTP query start'
      end
      object lblContactOpenFile: TLabel
        Left = 2
        Top = 153
        Width = 186
        Height = 13
        Caption = 'Open file associated with contact if set'
      end
      object Label7: TLabel
        Left = 3
        Top = 3
        Width = 378
        Height = 13
        Caption = 
          'Contacts/phonebook JSON file (if empty: file from application di' +
          'rectory is used)'
      end
      object lblContactsCheckIfFileUpdated: TLabel
        Left = 231
        Top = 48
        Width = 47
        Height = 13
        Caption = 'second(s)'
      end
      object chbContactPopupShowOnIncoming: TCheckBox
        Left = 13
        Top = 104
        Width = 438
        Height = 17
        Caption = 'on incoming call'
        TabOrder = 0
      end
      object chbContactPopupShowOnOutgoing: TCheckBox
        Left = 13
        Top = 124
        Width = 325
        Height = 17
        Caption = 'on outgoing call'
        TabOrder = 1
      end
      object edContactHttpQuery: TEdit
        Left = 109
        Top = 242
        Width = 385
        Height = 21
        TabOrder = 2
        Text = 'https://www.google.com/search?q=[number]'
      end
      object chbContactFilterUsingNote: TCheckBox
        Left = 2
        Top = 314
        Width = 325
        Height = 17
        Caption = 'Include "Note" field when filtering contacts'
        TabOrder = 3
      end
      object cbHttpQueryOpenMode: TComboBox
        Left = 109
        Top = 269
        Width = 240
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
        ItemIndex = 0
        TabOrder = 4
        Text = 'Manual only (HTTP query button)'
        Items.Strings = (
          'Manual only (HTTP query button)'
          'Automatic: on incoming call'
          'Automatic: on accepting incoming call')
      end
      object chbContactOpenFileOnIncoming: TCheckBox
        Left = 13
        Top = 169
        Width = 438
        Height = 17
        Caption = 'on incoming call'
        TabOrder = 5
      end
      object chbContactOpenFileOnOutgoing: TCheckBox
        Left = 13
        Top = 189
        Width = 325
        Height = 17
        Caption = 'on outgoing call'
        TabOrder = 6
      end
      object edContactsFile: TEdit
        Left = 13
        Top = 20
        Width = 454
        Height = 21
        TabOrder = 7
      end
      object btnSelectContactsFile: TButton
        Left = 473
        Top = 20
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 8
        OnClick = btnSelectContactsFileClick
      end
      object chbContactsCheckIfFileUpdated: TCheckBox
        Left = 13
        Top = 47
        Width = 164
        Height = 17
        Caption = 'Check if file is updated every'
        TabOrder = 9
      end
      object edContactsCheckIfFileUpdated: TEdit
        Left = 183
        Top = 45
        Width = 42
        Height = 21
        TabOrder = 10
        Text = '60'
      end
      object chbContactStoreNoteInSeparateFile: TCheckBox
        Left = 3
        Top = 337
        Width = 502
        Height = 17
        Caption = 'Store contact note in file associated with contact entry'
        TabOrder = 11
      end
    end
    object tsHistory: TTabSheet
      Caption = 'History'
      ImageIndex = 16
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Left = 3
        Top = 181
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
      object chbHistoryShowLastReplyCodeInHint: TCheckBox
        Left = 21
        Top = 117
        Width = 446
        Height = 17
        Caption = 'Show last SIP reply code code in hint'
        TabOrder = 6
      end
      object chbHistoryShowLastReplyLineInHint: TCheckBox
        Left = 21
        Top = 139
        Width = 446
        Height = 17
        Caption = 'Show last SIP reply line in hint'
        TabOrder = 7
      end
      object chbHistoryShowRecordFileInHint: TCheckBox
        Left = 21
        Top = 160
        Width = 446
        Height = 17
        Caption = 'Show record file in hint'
        TabOrder = 8
      end
    end
    object tsPhones: TTabSheet
      Caption = 'Plugins / phones'
      ImageIndex = 13
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
    end
    object tsTrayNotifier: TTabSheet
      Caption = 'Tray Notifier'
      ImageIndex = 16
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
      object chbTrayNotifierDoNotChangePosition: TCheckBox
        Left = 3
        Top = 158
        Width = 406
        Height = 17
        Caption = 'Do not change position (always show at same, current position)'
        TabOrder = 8
      end
    end
    object tsScripts: TTabSheet
      Caption = 'Scripts'
      ImageIndex = 18
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
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
        Width = 259
        Height = 13
        Caption = 'Lua scripts executed on events (\scripts subdirectory)'
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
      object lblScriptOnProgrammableButtonClick: TLabel
        Left = 5
        Top = 266
        Width = 140
        Height = 13
        Caption = 'on programmable button click'
      end
      object lblScriptOnAudioError: TLabel
        Left = 5
        Top = 316
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
      object lblOnCustomRequestReply: TLabel
        Left = 5
        Top = 343
        Width = 116
        Height = 13
        Caption = 'on custom request reply'
      end
      object lblOnContactNoteOpen: TLabel
        Left = 5
        Top = 370
        Width = 103
        Height = 13
        Caption = 'on contact note open'
      end
      object lblOnRecorderState: TLabel
        Left = 5
        Top = 397
        Width = 84
        Height = 13
        Caption = 'on recorder state'
      end
      object lblScriptOnProgrammableButtonMouseUpDown: TLabel
        Left = 5
        Top = 291
        Width = 126
        Height = 13
        Caption = 'on button mouse down/up'
      end
      object lblOnEncryptionState: TLabel
        Left = 5
        Top = 425
        Width = 94
        Height = 13
        Caption = 'on encryption state'
      end
      object edScriptOnCallStateChangeFile: TEdit
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
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
        Left = 148
        Top = 314
        Width = 212
        Height = 21
        TabOrder = 28
      end
      object btnSelectedScriptOnAudioErrorChange: TButton
        Left = 363
        Top = 314
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 29
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnAudioErrorEdit: TBitBtn
        Left = 390
        Top = 314
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
        Left = 148
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
      object edScriptOnCustomRequestReplyFile: TEdit
        Left = 148
        Top = 340
        Width = 212
        Height = 21
        TabOrder = 35
      end
      object btnSelectedScriptOnCustomRequestReplyChange: TButton
        Left = 363
        Top = 340
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 36
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnCustomRequestReplyEdit: TBitBtn
        Left = 390
        Top = 340
        Width = 22
        Height = 21
        TabOrder = 37
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
      object edScriptOnContactNoteOpenFile: TEdit
        Left = 148
        Top = 367
        Width = 212
        Height = 21
        TabOrder = 38
      end
      object btnSelectedScriptOnContactNoteOpenChange: TButton
        Left = 363
        Top = 367
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 39
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnContactNoteOpenEdit: TBitBtn
        Left = 390
        Top = 367
        Width = 22
        Height = 21
        TabOrder = 40
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
      object edScriptOnRecorderStateFile: TEdit
        Left = 148
        Top = 394
        Width = 212
        Height = 21
        TabOrder = 41
      end
      object btnSelectedScriptOnRecorderStateChange: TButton
        Left = 363
        Top = 394
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 42
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnRecorderStateEdit: TBitBtn
        Left = 390
        Top = 394
        Width = 22
        Height = 21
        TabOrder = 43
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
      object edScriptOnProgrammableButtonMouseUpDownFile: TEdit
        Left = 148
        Top = 288
        Width = 212
        Height = 21
        TabOrder = 44
      end
      object btnSelectedScriptOnProgrammableButtonMouseUpDownChange: TButton
        Left = 363
        Top = 288
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 45
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnProgrammableButtonMouseUpDownEdit: TBitBtn
        Left = 390
        Top = 288
        Width = 22
        Height = 21
        TabOrder = 46
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
      object edScriptOnEncryptionStateFile: TEdit
        Left = 148
        Top = 422
        Width = 212
        Height = 21
        TabOrder = 47
      end
      object btnSelectedScriptOnEncryptionStateChange: TButton
        Left = 363
        Top = 422
        Width = 22
        Height = 21
        Caption = '...'
        TabOrder = 48
        OnClick = btnSelectedScriptClick
      end
      object btnSelectedScriptOnEncryptionStateEdit: TBitBtn
        Left = 390
        Top = 422
        Width = 22
        Height = 21
        TabOrder = 49
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
    end
    object tsLogging: TTabSheet
      Caption = 'Logging'
      ImageIndex = 1
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
      object lblUiCapacity: TLabel
        Left = 5
        Top = 172
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
        Top = 202
        Width = 79
        Height = 13
        Caption = 'Log window font'
      end
      object cmbMaxUiLogLines: TComboBox
        Left = 365
        Top = 169
        Width = 111
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
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
        ItemHeight = 0
        TabOrder = 4
        Items.Strings = (
          '1048576'
          '2097152'
          '5242880'
          '10485760'
          '104857600')
      end
      object cbLogRotate: TComboBox
        Left = 365
        Top = 69
        Width = 111
        Height = 21
        Style = csDropDownList
        ItemHeight = 0
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
        Top = 199
        Width = 201
        Height = 21
        TabOrder = 6
        Text = 'abcABC 123 () [] <>,.'
      end
      object btnLoggingConsoleFontSelect: TButton
        Left = 305
        Top = 197
        Width = 26
        Height = 25
        Caption = '...'
        TabOrder = 7
        OnClick = btnLoggingConsoleFontSelectClick
      end
      object chbLogAubuf: TCheckBox
        Left = 5
        Top = 143
        Width = 325
        Height = 17
        Caption = 'Logging for audio buffers'
        TabOrder = 8
      end
      object chbLogMessagesOnlyFirstLines: TCheckBox
        Left = 21
        Top = 119
        Width = 325
        Height = 17
        Caption = 'Log only first lines (Request-Line/Status-Line) from messages'
        TabOrder = 9
      end
    end
    object tsUaConfOpus: TTabSheet
      Caption = 'Opus'
      ImageIndex = 21
      ExplicitLeft = 0
      ExplicitTop = 0
      ExplicitWidth = 0
      ExplicitHeight = 0
    end
  end
  object tvSelector: TTreeView
    Left = 0
    Top = 0
    Width = 128
    Height = 468
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
    TabOrder = 0
    OnChange = tvSelectorChange
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

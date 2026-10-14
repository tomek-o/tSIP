object frmAccount: TfrmAccount
  Left = 0
  Top = 0
  Align = alTop
  BorderStyle = bsNone
  Caption = 'Account, codecs'
  ClientHeight = 581
  ClientWidth = 605
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBasic: TPanel
    Left = 0
    Top = 0
    Width = 605
    Height = 176
    Align = alTop
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 0
    ExplicitWidth = 532
    object lblName: TLabel
      Left = 9
      Top = 30
      Width = 27
      Height = 13
      Caption = 'Name'
    end
    object lblNameInfo: TLabel
      Left = 294
      Top = 30
      Width = 206
      Height = 13
      Caption = 'optional, displayed instead of user@server'
    end
    object lblRegServer: TLabel
      Left = 9
      Top = 56
      Width = 109
      Height = 13
      Caption = 'SIP/registration server'
    end
    object lblUser: TLabel
      Left = 9
      Top = 80
      Width = 22
      Height = 13
      Caption = 'User'
    end
    object lblPassword: TLabel
      Left = 9
      Top = 104
      Width = 46
      Height = 13
      Caption = 'Password'
    end
    object lblExpires: TLabel
      Left = 9
      Top = 128
      Width = 96
      Height = 13
      Caption = 'Registration Expires'
    end
    object lblExpiresInfo: TLabel
      Left = 182
      Top = 128
      Width = 35
      Height = 13
      Caption = 'e.g. 60'
    end
    object lblExpiresInfo2: TLabel
      Left = 233
      Top = 128
      Width = 106
      Height = 13
      Caption = '0 = no registration'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object lblPtime: TLabel
      Left = 9
      Top = 152
      Width = 26
      Height = 13
      Caption = 'ptime'
    end
    object chbEnabled: TCheckBox
      Left = 7
      Top = 6
      Width = 200
      Height = 17
      Caption = 'Account enabled'
      TabOrder = 9
      OnClick = chbEnabledClick
    end
    object edName: TEdit
      Left = 126
      Top = 27
      Width = 162
      Height = 21
      TabOrder = 10
      OnChange = edNameChange
    end
    object edRegServer: TEdit
      Left = 126
      Top = 53
      Width = 162
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnChange = edRegServerChange
    end
    object cbTransport: TComboBox
      Left = 294
      Top = 53
      Width = 50
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 2
      Text = 'UDP'
      OnChange = cbTransportChange
      Items.Strings = (
        'UDP'
        'TCP'
        'TLS')
    end
    object chbHideRegServer: TCheckBox
      Left = 380
      Top = 55
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 3
      OnClick = chbHideRegServerClick
    end
    object edUser: TEdit
      Left = 126
      Top = 77
      Width = 98
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      OnChange = edUserChange
    end
    object chbHideUser: TCheckBox
      Left = 380
      Top = 79
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 5
      OnClick = chbHideUserClick
    end
    object edPassword: TEdit
      Left = 126
      Top = 101
      Width = 98
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      PasswordChar = '*'
      TabOrder = 6
      OnChange = edPasswordChange
    end
    object chbHidePassword: TCheckBox
      Left = 380
      Top = 103
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 7
      OnClick = chbHidePasswordClick
    end
    object edExpires: TEdit
      Left = 126
      Top = 125
      Width = 50
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 8
      Text = '0'
      OnChange = edExpiresChange
    end
    object edPtime: TEdit
      Left = 126
      Top = 149
      Width = 50
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      Text = '20'
      OnChange = edPtimeChange
    end
    object grboxAccount: TGroupBox
      Left = 440
      Top = 6
      Width = 86
      Height = 25
      Align = alCustom
      Caption = 'Account #xxx'
      TabOrder = 0
      Visible = False
    end
  end
  object pnlExtraToggle: TPanel
    Left = 0
    Top = 176
    Width = 605
    Height = 31
    Align = alTop
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 1
    ExplicitWidth = 532
    object btnExtra: TSpeedButton
      Left = 7
      Top = 3
      Width = 140
      Height = 25
      AllowAllUp = True
      GroupIndex = 1
      Caption = 'Show extra settings'
      OnClick = btnExtraClick
    end
    object lblExtra: TLabel
      Left = 156
      Top = 9
      Width = 311
      Height = 13
      Caption = '(typically can be left empty or with default values)'
    end
  end
  object pnlExtra: TPanel
    Left = 0
    Top = 207
    Width = 605
    Height = 186
    Align = alTop
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 2
    Visible = False
    ExplicitWidth = 532
    object lblDisplayName: TLabel
      Left = 8
      Top = 5
      Width = 63
      Height = 13
      Caption = 'Display name'
    end
    object lblAuthUser: TLabel
      Left = 8
      Top = 28
      Width = 94
      Height = 13
      Caption = 'Authentication user'
    end
    object lblAuthUserInfo: TLabel
      Left = 232
      Top = 28
      Width = 122
      Height = 13
      Caption = '(fill if different than User)'
    end
    object lblContactUser: TLabel
      Left = 8
      Top = 51
      Width = 70
      Height = 13
      Caption = '"Contact" user'
    end
    object lblStunServer: TLabel
      Left = 8
      Top = 74
      Width = 60
      Height = 13
      Caption = 'STUN server'
    end
    object lblOutbound1: TLabel
      Left = 8
      Top = 97
      Width = 79
      Height = 13
      Caption = 'Outbound proxy'
    end
    object lblDtmfTxFormat: TLabel
      Left = 8
      Top = 120
      Width = 81
      Height = 13
      Caption = 'DTMF TX method'
    end
    object lblMediaEncryption: TLabel
      Left = 8
      Top = 164
      Width = 82
      Height = 13
      Caption = 'Media encryption'
    end
    object edDisplayName: TEdit
      Left = 126
      Top = 2
      Width = 160
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 7
      OnChange = edDisplayNameChange
    end
    object chbHideDisplayName: TCheckBox
      Left = 379
      Top = 4
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 9
      OnClick = chbHideDisplayNameClick
    end
    object edAuthUser: TEdit
      Left = 126
      Top = 25
      Width = 98
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      OnChange = edAuthUserChange
    end
    object chbHideAuthUser: TCheckBox
      Left = 379
      Top = 27
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 0
      OnClick = chbHideAuthUserClick
    end
    object edCuser: TEdit
      Left = 126
      Top = 48
      Width = 98
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnChange = edCuserChange
    end
    object chbHideCuser: TCheckBox
      Left = 379
      Top = 50
      Width = 145
      Height = 17
      Caption = 'Hide/do not store in file'
      TabOrder = 2
      OnClick = chbHideCuserClick
    end
    object edStunServer: TEdit
      Left = 126
      Top = 71
      Width = 162
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 3
      OnChange = edStunServerChange
    end
    object edOutbound1: TEdit
      Left = 126
      Top = 94
      Width = 162
      Height = 22
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 4
      OnChange = edOutbound1Change
    end
    object cbOutbound1Transport: TComboBox
      Left = 294
      Top = 94
      Width = 50
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 5
      Text = 'UDP'
      OnChange = cbOutbound1TransportChange
      Items.Strings = (
        'UDP'
        'TCP'
        'TLS')
    end
    object cbDtmfTxFormat: TComboBox
      Left = 126
      Top = 117
      Width = 131
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 6
      Text = 'RFC2833'
      OnChange = cbDtmfTxFormatChange
      Items.Strings = (
        'RFC2833'
        'SIP INFO / dtmf-relay'
        'inband')
    end
    object chbAnswerAny: TCheckBox
      Left = 7
      Top = 141
      Width = 346
      Height = 17
      Caption = 
        'Answer any call/message with this account (use as "local account' +
        '")'
      TabOrder = 8
      OnClick = chbAnswerAnyClick
      OnKeyPress = chbAnswerAnyKeyPress
    end
    object cbMediaEncryption: TComboBox
      Left = 126
      Top = 161
      Width = 131
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 10
      Text = '[none]'
      OnChange = cbMediaEncryptionChange
      Items.Strings = (
        '[none]'
        'srtp'
        'srtp-mand'
        'srtp-mandf'
        'zrtp')
    end
  end
  object pnlCodecs: TPanel
    Left = 0
    Top = 393
    Width = 605
    Height = 196
    Align = alTop
    BevelOuter = bvNone
    ParentBackground = False
    TabOrder = 3
    ExplicitWidth = 532
    object lblAudioCodecsAvailable: TLabel
      Left = 9
      Top = 6
      Width = 108
      Height = 13
      Caption = 'Available audio codecs'
    end
    object lblAudioCodecsEnabled: TLabel
      Left = 200
      Top = 6
      Width = 151
      Height = 13
      Caption = 'Enabled, in order of preference'
    end
    object lblCodecsInfo: TLabel
      Left = 9
      Top = 178
      Width = 419
      Height = 13
      Caption = 
        'Double-click codec to move between the lists, drag-and-drop on e' +
        'nabled list to reorder.'
    end
    object lboxAudioCodecsAvailable: TListBox
      Left = 9
      Top = 22
      Width = 150
      Height = 150
      ItemHeight = 13
      TabOrder = 0
      OnDblClick = lboxAudioCodecsAvailableDblClick
    end
    object btnAudioCodecEnable: TButton
      Left = 164
      Top = 66
      Width = 30
      Height = 25
      Hint = 'Enable selected codec'
      Caption = '->'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = btnAudioCodecEnableClick
    end
    object btnAudioCodecDisable: TButton
      Left = 164
      Top = 104
      Width = 30
      Height = 25
      Hint = 'Disable selected codec'
      Caption = '<-'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 2
      OnClick = btnAudioCodecDisableClick
    end
    object lboxAudioCodecsEnabled: TListBox
      Left = 200
      Top = 22
      Width = 150
      Height = 150
      DragMode = dmAutomatic
      ItemHeight = 13
      TabOrder = 3
      OnDblClick = lboxAudioCodecsEnabledDblClick
      OnDragDrop = lboxAudioCodecsEnabledDragDrop
      OnDragOver = lboxAudioCodecsEnabledDragOver
      OnStartDrag = lboxAudioCodecsEnabledStartDrag
    end
  end
end

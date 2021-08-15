object frmAccount: TfrmAccount
  Left = 0
  Top = 0
  Align = alTop
  BorderStyle = bsNone
  Caption = 'Account'
  ClientHeight = 346
  ClientWidth = 532
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lblRegServer: TLabel
    Left = 9
    Top = 8
    Width = 109
    Height = 13
    Caption = 'SIP/registration server'
  end
  object lblUser: TLabel
    Left = 9
    Top = 32
    Width = 22
    Height = 13
    Caption = 'User'
  end
  object lblPassword: TLabel
    Left = 9
    Top = 56
    Width = 46
    Height = 13
    Caption = 'Password'
  end
  object lblExpires: TLabel
    Left = 9
    Top = 80
    Width = 96
    Height = 13
    Caption = 'Registration Expires'
  end
  object lblExpiresInfo: TLabel
    Left = 182
    Top = 80
    Width = 35
    Height = 13
    Caption = 'e.g. 60'
  end
  object lblExtra: TLabel
    Left = 9
    Top = 122
    Width = 311
    Height = 13
    Caption = 
      'Extra settings (typically can be left empty or with default valu' +
      'es)'
  end
  object lblAuthUser: TLabel
    Left = 8
    Top = 172
    Width = 94
    Height = 13
    Caption = 'Authentication user'
  end
  object lblAuthUserInfo: TLabel
    Left = 232
    Top = 172
    Width = 122
    Height = 13
    Caption = '(fill if different than User)'
  end
  object lblStunServer: TLabel
    Left = 8
    Top = 218
    Width = 60
    Height = 13
    Caption = 'STUN server'
  end
  object lblOutbound1: TLabel
    Left = 8
    Top = 241
    Width = 79
    Height = 13
    Caption = 'Outbound proxy'
  end
  object lblPtime: TLabel
    Left = 8
    Top = 302
    Width = 26
    Height = 13
    Caption = 'ptime'
  end
  object lblContactUser: TLabel
    Left = 7
    Top = 195
    Width = 70
    Height = 13
    Caption = '"Contact" user'
  end
  object lblExpiresInfo2: TLabel
    Left = 233
    Top = 80
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
  object lblDtmfTxFormat: TLabel
    Left = 7
    Top = 264
    Width = 81
    Height = 13
    Caption = 'DTMF TX method'
  end
  object lblDisplayName: TLabel
    Left = 8
    Top = 149
    Width = 63
    Height = 13
    Caption = 'Display name'
  end
  object lblMediaEncryption: TLabel
    Left = 8
    Top = 326
    Width = 82
    Height = 13
    Caption = 'Media encryption'
  end
  object grboxAccount: TGroupBox
    Left = 379
    Top = 293
    Width = 86
    Height = 25
    Align = alCustom
    Caption = 'Account #xxx'
    TabOrder = 0
    Visible = False
  end
  object edRegServer: TEdit
    Left = 126
    Top = 5
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
    Top = 5
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
  object edUser: TEdit
    Left = 126
    Top = 29
    Width = 98
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 3
    OnChange = edUserChange
  end
  object edPassword: TEdit
    Left = 126
    Top = 53
    Width = 98
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    PasswordChar = '*'
    TabOrder = 4
    OnChange = edPasswordChange
  end
  object edExpires: TEdit
    Left = 126
    Top = 77
    Width = 50
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 5
    Text = '0'
    OnChange = edExpiresChange
  end
  object edAuthUser: TEdit
    Left = 126
    Top = 169
    Width = 98
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 6
    OnChange = edAuthUserChange
  end
  object edStunServer: TEdit
    Left = 126
    Top = 215
    Width = 162
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 7
    OnChange = edStunServerChange
  end
  object edOutbound1: TEdit
    Left = 126
    Top = 238
    Width = 162
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 8
    OnChange = edOutbound1Change
  end
  object chbAnswerAny: TCheckBox
    Left = 7
    Top = 282
    Width = 346
    Height = 17
    Caption = 
      'Answer any call/message with this account (use as "local account' +
      '")'
    TabOrder = 9
    OnClick = chbAnswerAnyClick
    OnKeyPress = chbAnswerAnyKeyPress
  end
  object edPtime: TEdit
    Left = 126
    Top = 299
    Width = 50
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 10
    Text = '20'
    OnChange = edPtimeChange
  end
  object edCuser: TEdit
    Left = 126
    Top = 192
    Width = 98
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 11
    OnChange = edCuserChange
  end
  object chbHideRegServer: TCheckBox
    Left = 380
    Top = 7
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 12
    OnClick = chbHideRegServerClick
  end
  object chbHideUser: TCheckBox
    Left = 380
    Top = 31
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 13
    OnClick = chbHideUserClick
  end
  object chbHidePassword: TCheckBox
    Left = 380
    Top = 55
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 14
    OnClick = chbHidePasswordClick
  end
  object chbHideAuthUser: TCheckBox
    Left = 379
    Top = 171
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 15
    OnClick = chbHideAuthUserClick
  end
  object chbHideCuser: TCheckBox
    Left = 379
    Top = 194
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 16
    OnClick = chbHideCuserClick
  end
  object cbDtmfTxFormat: TComboBox
    Left = 126
    Top = 261
    Width = 131
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 17
    Text = 'RFC2833'
    OnChange = cbDtmfTxFormatChange
    Items.Strings = (
      'RFC2833'
      'SIP INFO / dtmf-relay'
      'inband')
  end
  object edDisplayName: TEdit
    Left = 126
    Top = 146
    Width = 160
    Height = 22
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Courier New'
    Font.Style = []
    ParentFont = False
    TabOrder = 18
    OnChange = edDisplayNameChange
  end
  object chbHideDisplayName: TCheckBox
    Left = 379
    Top = 148
    Width = 145
    Height = 17
    Caption = 'Hide/do not store in file'
    TabOrder = 19
    OnClick = chbHideDisplayNameClick
  end
  object cbMediaEncryption: TComboBox
    Left = 126
    Top = 323
    Width = 131
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 20
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

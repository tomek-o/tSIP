object frmAccount: TfrmAccount
  Left = 0
  Top = 0
  Align = alTop
  BorderStyle = bsNone
  Caption = 'Account'
  ClientHeight = 274
  ClientWidth = 418
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
    Top = 117
    Width = 311
    Height = 13
    Caption = 
      'Extra settings (typically can be left empty or with default valu' +
      'es)'
  end
  object lblAuthUser: TLabel
    Left = 9
    Top = 139
    Width = 94
    Height = 13
    Caption = 'Authentication user'
  end
  object lblAuthUserInfo: TLabel
    Left = 233
    Top = 139
    Width = 122
    Height = 13
    Caption = '(fill if different than User)'
  end
  object lblStunServer: TLabel
    Left = 9
    Top = 186
    Width = 60
    Height = 13
    Caption = 'STUN server'
  end
  object lblOutbound1: TLabel
    Left = 9
    Top = 209
    Width = 79
    Height = 13
    Caption = 'Outbound proxy'
  end
  object lblPtime: TLabel
    Left = 9
    Top = 249
    Width = 26
    Height = 13
    Caption = 'ptime'
  end
  object lblContactUser: TLabel
    Left = 8
    Top = 163
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
  object grboxAccount: TGroupBox
    Left = 314
    Top = 158
    Width = 95
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
      'TCP')
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
    Top = 136
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
    Top = 183
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
    Top = 206
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
    Left = 9
    Top = 229
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
    Top = 246
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
    Top = 160
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
end

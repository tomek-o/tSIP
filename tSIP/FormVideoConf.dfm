object frmVideoConf: TfrmVideoConf
  Left = 0
  Top = 0
  Align = alClient
  BorderStyle = bsNone
  Caption = 'frmVideoConf'
  ClientHeight = 386
  ClientWidth = 618
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lblDisplayParentType: TLabel
    Left = 8
    Top = 226
    Width = 94
    Height = 13
    Caption = 'Display parent type'
  end
  object lblDisplayParentId: TLabel
    Left = 8
    Top = 250
    Width = 80
    Height = 13
    Caption = 'Display parent id'
  end
  object lblInputDevice: TLabel
    Left = 7
    Top = 55
    Width = 67
    Height = 13
    Caption = 'Source device'
  end
  object lblInputModule: TLabel
    Left = 7
    Top = 31
    Width = 70
    Height = 13
    Caption = 'Source module'
  end
  object lblAudioInput: TLabel
    Left = 3
    Top = 4
    Width = 26
    Height = 13
    Caption = 'Input'
  end
  object lblTransmittedVideo: TLabel
    Left = 8
    Top = 96
    Width = 86
    Height = 13
    Caption = 'Transmitted video'
  end
  object lblWidth: TLabel
    Left = 100
    Top = 115
    Width = 26
    Height = 13
    Caption = 'width'
  end
  object lblHeight: TLabel
    Left = 244
    Top = 115
    Width = 30
    Height = 13
    Caption = 'height'
  end
  object lblBitrate: TLabel
    Left = 100
    Top = 139
    Width = 32
    Height = 13
    Caption = 'bitrate'
  end
  object lblFps: TLabel
    Left = 100
    Top = 163
    Width = 15
    Height = 13
    Caption = 'fps'
  end
  object lblDisplayModule: TLabel
    Left = 8
    Top = 202
    Width = 71
    Height = 13
    Caption = 'Display module'
  end
  object chbEnabled: TCheckBox
    Left = 3
    Top = 3
    Width = 325
    Height = 17
    Caption = 'enabled'
    TabOrder = 0
  end
  object cbDisplayParentType: TComboBox
    Left = 152
    Top = 223
    Width = 320
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 1
  end
  object edDisplayParentId: TEdit
    Left = 152
    Top = 247
    Width = 62
    Height = 21
    TabOrder = 2
  end
  object chbSelfviewEnable: TCheckBox
    Left = 8
    Top = 281
    Width = 325
    Height = 17
    Caption = 'enable selfview (show video from local source)'
    TabOrder = 3
  end
  object chbSelfviewPip: TCheckBox
    Left = 19
    Top = 304
    Width = 325
    Height = 17
    Caption = 'show selfview as picture-in-picture'
    TabOrder = 4
  end
  object cbInputDev: TComboBox
    Left = 152
    Top = 52
    Width = 320
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 5
  end
  object cbInputMod: TComboBox
    Left = 152
    Top = 28
    Width = 320
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 6
    OnChange = cbInputModChange
  end
  object edInputFile: TEdit
    Left = 152
    Top = 52
    Width = 320
    Height = 21
    TabOrder = 7
    Visible = False
  end
  object btnSelectInputFile: TButton
    Left = 478
    Top = 53
    Width = 21
    Height = 19
    Caption = '...'
    TabOrder = 8
    Visible = False
    OnClick = btnSelectInputFileClick
  end
  object edWidth: TEdit
    Left = 152
    Top = 112
    Width = 49
    Height = 21
    TabOrder = 9
  end
  object edHeight: TEdit
    Left = 296
    Top = 112
    Width = 49
    Height = 21
    TabOrder = 10
  end
  object edBitrate: TEdit
    Left = 152
    Top = 136
    Width = 73
    Height = 21
    TabOrder = 11
  end
  object edFps: TEdit
    Left = 152
    Top = 160
    Width = 49
    Height = 21
    TabOrder = 12
  end
  object chbDshowSkipReadingBackMediaFormat: TCheckBox
    Left = 8
    Top = 336
    Width = 521
    Height = 17
    Caption = 
      'DirectShow camera input: skip reading back set media format (wor' +
      'karound for possible OBS-camera bug)'
    TabOrder = 13
  end
  object cbOutputMod: TComboBox
    Left = 152
    Top = 199
    Width = 320
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 14
    OnChange = cbOutputModChange
  end
  object dlgOpenDeviceFile: TOpenDialog
    Options = [ofEnableSizing]
    Left = 504
    Top = 48
  end
end

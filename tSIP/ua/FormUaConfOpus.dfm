object frmUaConfOpus: TfrmUaConfOpus
  Left = 0
  Top = 0
  Align = alClient
  BorderStyle = bsNone
  Caption = 'Opus'
  ClientHeight = 324
  ClientWidth = 442
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lblComplexity: TLabel
    Left = 8
    Top = 196
    Width = 53
    Height = 13
    Caption = 'Complexity'
  end
  object chbStereo: TCheckBox
    Left = 8
    Top = 8
    Width = 97
    Height = 17
    Caption = 'stereo'
    TabOrder = 0
  end
  object chbSpropStereo: TCheckBox
    Left = 8
    Top = 31
    Width = 97
    Height = 17
    Caption = 'sprop-stereo'
    TabOrder = 1
  end
  object chbSetBitrate: TCheckBox
    Left = 8
    Top = 54
    Width = 97
    Height = 17
    Caption = 'Set bitrate'
    TabOrder = 2
  end
  object edBitrate: TEdit
    Left = 175
    Top = 52
    Width = 74
    Height = 21
    TabOrder = 3
  end
  object chbSetSamplerate: TCheckBox
    Left = 8
    Top = 79
    Width = 97
    Height = 17
    Caption = 'Set sample rate'
    TabOrder = 4
  end
  object cbSamplerate: TComboBox
    Left = 175
    Top = 77
    Width = 74
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 4
    TabOrder = 5
    Text = '48000'
    Items.Strings = (
      '8000'
      '12000'
      '16000'
      '24000'
      '48000')
  end
  object chbSetCbr: TCheckBox
    Left = 8
    Top = 104
    Width = 97
    Height = 17
    Caption = 'Set CBR'
    TabOrder = 6
  end
  object chbCbr: TCheckBox
    Left = 175
    Top = 104
    Width = 97
    Height = 17
    Caption = 'CBR'
    TabOrder = 7
  end
  object chbSetInbandFec: TCheckBox
    Left = 8
    Top = 127
    Width = 97
    Height = 17
    Caption = 'Set inband FEC'
    TabOrder = 8
  end
  object chbInbandFec: TCheckBox
    Left = 175
    Top = 127
    Width = 97
    Height = 17
    Caption = 'inband FEC'
    TabOrder = 9
  end
  object chbSetDtx: TCheckBox
    Left = 8
    Top = 150
    Width = 97
    Height = 17
    Caption = 'Set DTX'
    TabOrder = 10
  end
  object chbDtx: TCheckBox
    Left = 175
    Top = 150
    Width = 97
    Height = 17
    Caption = 'DTX'
    TabOrder = 11
  end
  object chbMirror: TCheckBox
    Left = 8
    Top = 173
    Width = 265
    Height = 17
    Caption = 'Mirror 2nd party parameters if available'
    TabOrder = 12
  end
  object cbComplexity: TComboBox
    Left = 175
    Top = 193
    Width = 74
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 9
    TabOrder = 13
    Text = '10'
    Items.Strings = (
      '1'
      '2'
      '3'
      '4'
      '5'
      '6'
      '7'
      '8'
      '9'
      '10')
  end
  object chbSetApplication: TCheckBox
    Left = 8
    Top = 218
    Width = 97
    Height = 17
    Caption = 'Set application'
    TabOrder = 14
  end
  object cbApplication: TComboBox
    Left = 175
    Top = 216
    Width = 74
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 15
    Text = 'audio'
    Items.Strings = (
      'audio'
      'VoIP')
  end
  object chbSetPacketLoss: TCheckBox
    Left = 8
    Top = 241
    Width = 153
    Height = 17
    Caption = 'Set packet loss percentage'
    TabOrder = 16
  end
  object edPacketLoss: TEdit
    Left = 175
    Top = 239
    Width = 74
    Height = 21
    TabOrder = 17
    Text = '10'
  end
end

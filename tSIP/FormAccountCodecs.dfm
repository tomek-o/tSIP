object frmAccountCodecs: TfrmAccountCodecs
  Left = 0
  Top = 0
  Caption = 'frmAccountCodecs'
  ClientHeight = 687
  ClientWidth = 385
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object lblAudioCodecsAvailable: TLabel
    Left = 8
    Top = 8
    Width = 108
    Height = 13
    Caption = 'Available audio codecs'
  end
  object lblAudioCodecsEnabled: TLabel
    Left = 200
    Top = 8
    Width = 151
    Height = 13
    Caption = 'Enabled, in order of preference'
  end
  object lblCodecsInfo: TLabel
    Left = 8
    Top = 343
    Width = 218
    Height = 13
    Caption = 'Double-click codec to move between the lists.'
  end
  object lblCodecsInfo2: TLabel
    Left = 8
    Top = 358
    Width = 307
    Height = 13
    Caption = 
      'Drag-and-drop items on enabled codecs list to reorder priorities' +
      '.'
  end
  object lboxAudioCodecsAvailable: TListBox
    Left = 8
    Top = 27
    Width = 150
    Height = 313
    ItemHeight = 13
    TabOrder = 0
    OnDblClick = lboxAudioCodecsAvailableDblClick
  end
  object btnAudioCodecEnable: TButton
    Left = 164
    Top = 132
    Width = 30
    Height = 25
    Hint = 'Enable selected codec'
    Caption = '->'
    TabOrder = 1
    OnClick = btnAudioCodecEnableClick
  end
  object btnAudioCodecDisable: TButton
    Left = 164
    Top = 195
    Width = 30
    Height = 25
    Hint = 'Disable selected codec'
    Caption = '<-'
    TabOrder = 2
    OnClick = btnAudioCodecDisableClick
  end
  object lboxAudioCodecsEnabled: TListBox
    Left = 200
    Top = 27
    Width = 150
    Height = 313
    DragMode = dmAutomatic
    ItemHeight = 13
    TabOrder = 3
    OnDblClick = lboxAudioCodecsEnabledDblClick
    OnDragDrop = lboxAudioCodecsEnabledDragDrop
    OnDragOver = lboxAudioCodecsEnabledDragOver
    OnStartDrag = lboxAudioCodecsEnabledStartDrag
  end
end

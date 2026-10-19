object frmPublish: TfrmPublish
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = 'Publish presence'
  ClientHeight = 227
  ClientWidth = 331
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object lblStatus: TLabel
    Left = 12
    Top = 14
    Width = 35
    Height = 13
    Caption = 'Status:'
  end
  object lblTarget: TLabel
    Left = 12
    Top = 46
    Width = 41
    Height = 13
    Caption = 'Send to:'
  end
  object lblNote: TLabel
    Left = 12
    Top = 78
    Width = 27
    Height = 13
    Caption = 'Note:'
  end
  object cbStatus: TComboBox
    Left = 70
    Top = 11
    Width = 250
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 0
    Text = 'Open (available)'
    Items.Strings = (
      'Open (available)'
      'Closed')
  end
  object cbTarget: TComboBox
    Left = 70
    Top = 43
    Width = 250
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 1
    Text = 'Current account'
    Items.Strings = (
      'Current account'
      'All accounts with PUBLISH enabled')
  end
  object memo: TMemo
    Left = 70
    Top = 75
    Width = 250
    Height = 110
    ScrollBars = ssVertical
    TabOrder = 2
  end
  object btnPublish: TButton
    Left = 152
    Top = 191
    Width = 80
    Height = 25
    Caption = 'Publish'
    Default = True
    ModalResult = 1
    TabOrder = 3
  end
  object btnCancel: TButton
    Left = 240
    Top = 191
    Width = 80
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 4
  end
end

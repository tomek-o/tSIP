object frmButtonCopy: TfrmButtonCopy
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Copy button'
  ClientHeight = 242
  ClientWidth = 445
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lblSource: TLabel
    Left = 8
    Top = 8
    Width = 33
    Height = 13
    Caption = 'Source'
  end
  object lblTarget: TLabel
    Left = 240
    Top = 8
    Width = 32
    Height = 13
    Caption = 'Target'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 205
    Width = 445
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnCancel: TButton
      Left = 360
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Close'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 279
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Copy'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object cbSource: TComboBox
    Left = 56
    Top = 5
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    TabOrder = 1
  end
  object cbTarget: TComboBox
    Left = 286
    Top = 30
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    TabOrder = 2
  end
  object grboxCopy: TGroupBox
    Left = 8
    Top = 63
    Width = 425
    Height = 136
    Caption = 'Copy'
    PopupMenu = popupCopy
    TabOrder = 3
    object chbPosition: TCheckBox
      Left = 16
      Top = 85
      Width = 73
      Height = 17
      Caption = 'position'
      TabOrder = 0
    end
    object chbCaption: TCheckBox
      Left = 144
      Top = 62
      Width = 73
      Height = 17
      Caption = 'caption'
      TabOrder = 1
    end
    object chbBehavior: TCheckBox
      Left = 248
      Top = 62
      Width = 105
      Height = 17
      Caption = 'type and behavior'
      TabOrder = 2
    end
    object chbFont: TCheckBox
      Left = 144
      Top = 16
      Width = 73
      Height = 17
      Caption = 'font'
      TabOrder = 3
    end
    object chbSize: TCheckBox
      Left = 144
      Top = 39
      Width = 73
      Height = 17
      Caption = 'size'
      TabOrder = 4
    end
    object chbColors: TCheckBox
      Left = 16
      Top = 16
      Width = 73
      Height = 17
      Caption = 'colors'
      TabOrder = 5
    end
    object chbVisible: TCheckBox
      Left = 248
      Top = 16
      Width = 73
      Height = 17
      Caption = 'visible'
      TabOrder = 6
    end
    object chbInactive: TCheckBox
      Left = 16
      Top = 39
      Width = 73
      Height = 17
      Caption = 'inactive'
      TabOrder = 7
    end
    object chbImages: TCheckBox
      Left = 16
      Top = 62
      Width = 73
      Height = 17
      Caption = 'images'
      TabOrder = 8
    end
    object chbFrame: TCheckBox
      Left = 248
      Top = 39
      Width = 122
      Height = 17
      Caption = 'frame, bevel width'
      TabOrder = 9
    end
    object chbCenterTextHorizontally: TCheckBox
      Left = 144
      Top = 85
      Width = 73
      Height = 17
      Caption = 'center text'
      TabOrder = 10
    end
    object chbImageTransparent: TCheckBox
      Left = 248
      Top = 85
      Width = 129
      Height = 17
      Caption = 'image: transparent'
      TabOrder = 11
    end
    object chbImagePosition: TCheckBox
      Left = 248
      Top = 108
      Width = 129
      Height = 17
      Caption = 'image: position'
      TabOrder = 12
    end
    object chbLabelPosition: TCheckBox
      Left = 16
      Top = 108
      Width = 129
      Height = 17
      Caption = 'label: position'
      TabOrder = 13
    end
  end
  object cbTargetType: TComboBox
    Left = 286
    Top = 5
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 4
    Text = 'single button'
    OnChange = cbTargetTypeChange
    Items.Strings = (
      'single button'
      'all visible buttons'
      'all buttons')
  end
  object popupCopy: TPopupMenu
    Left = 16
    Top = 208
    object miSelectAll: TMenuItem
      Caption = 'Select all'
      OnClick = miSelectAllClick
    end
    object miDeselectAll: TMenuItem
      Caption = 'DeselectAll'
      OnClick = miDeselectAllClick
    end
  end
end

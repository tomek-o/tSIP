object frmButtonCopy: TfrmButtonCopy
  Left = 0
  Top = 0
  Anchors = [akTop, akRight]
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Copy button'
  ClientHeight = 437
  ClientWidth = 604
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  OnShow = FormShow
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
    Left = 231
    Top = 8
    Width = 32
    Height = 13
    Caption = 'Target'
  end
  object lblFrom: TLabel
    Left = 231
    Top = 32
    Width = 22
    Height = 13
    Caption = 'from'
  end
  object lblTo: TLabel
    Left = 435
    Top = 32
    Width = 10
    Height = 13
    Caption = 'to'
  end
  object lblDirection: TLabel
    Left = 207
    Top = 8
    Width = 16
    Height = 13
    Caption = '=>'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 400
    Width = 604
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 5
    DesignSize = (
      604
      37)
    object btnCancel: TButton
      Left = 524
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Close'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 443
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
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
    TabOrder = 0
  end
  object cbTarget: TComboBox
    Left = 277
    Top = 29
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    TabOrder = 2
  end
  object grboxCopy: TGroupBox
    Left = 8
    Top = 56
    Width = 588
    Height = 338
    Caption = 'Attributes to copy'
    PopupMenu = popupCopy
    TabOrder = 4
    object chbPosition: TCheckBox
      Left = 16
      Top = 97
      Width = 73
      Height = 17
      Caption = 'position'
      TabOrder = 6
    end
    object chbBehavior: TCheckBox
      Left = 16
      Top = 51
      Width = 105
      Height = 17
      Caption = 'type and behavior'
      TabOrder = 2
    end
    object chbSize: TCheckBox
      Left = 144
      Top = 97
      Width = 73
      Height = 17
      Caption = 'size'
      TabOrder = 7
    end
    object chbColors: TCheckBox
      Left = 16
      Top = 269
      Width = 73
      Height = 17
      Caption = 'colors'
      TabOrder = 12
    end
    object chbVisible: TCheckBox
      Left = 16
      Top = 74
      Width = 73
      Height = 17
      Caption = 'visible'
      TabOrder = 3
    end
    object chbInactive: TCheckBox
      Left = 336
      Top = 74
      Width = 73
      Height = 17
      Caption = 'inactive'
      TabOrder = 5
    end
    object chbImages: TCheckBox
      Left = 16
      Top = 292
      Width = 73
      Height = 17
      Caption = 'images'
      TabOrder = 13
    end
    object chbFrame: TCheckBox
      Left = 16
      Top = 314
      Width = 122
      Height = 17
      Caption = 'frame, bevel width'
      TabOrder = 16
    end
    object chbImageTransparent: TCheckBox
      Left = 336
      Top = 292
      Width = 124
      Height = 17
      Caption = 'image: transparent'
      TabOrder = 15
    end
    object chbImagePosition: TCheckBox
      Left = 144
      Top = 292
      Width = 129
      Height = 17
      Caption = 'image: position'
      TabOrder = 14
    end
    object chbParentContainer: TCheckBox
      Left = 144
      Top = 74
      Width = 129
      Height = 17
      Caption = 'parent container'
      TabOrder = 4
    end
    object grboxCaption1: TGroupBox
      Left = 8
      Top = 159
      Width = 571
      Height = 44
      Caption = 'Caption #1'
      TabOrder = 10
      object chbCaption1: TCheckBox
        Left = 9
        Top = 18
        Width = 46
        Height = 17
        Caption = 'text'
        TabOrder = 0
      end
      object chbFont1: TCheckBox
        Left = 70
        Top = 18
        Width = 42
        Height = 17
        Caption = 'font'
        TabOrder = 1
      end
      object chbLabelPosition1: TCheckBox
        Left = 136
        Top = 18
        Width = 60
        Height = 17
        Caption = 'position'
        TabOrder = 2
      end
      object chbCenterTextHorizontally1: TCheckBox
        Left = 228
        Top = 18
        Width = 135
        Height = 17
        Caption = 'center text horizontally'
        TabOrder = 3
      end
      object chbCenterTextVertically1: TCheckBox
        Left = 383
        Top = 18
        Width = 125
        Height = 17
        Caption = 'center text vertically'
        TabOrder = 4
      end
    end
    object chbNumberOfCaptionLines: TCheckBox
      Left = 17
      Top = 140
      Width = 158
      Height = 17
      Caption = 'number of caption lines'
      TabOrder = 8
    end
    object chbSpaceLabelsYEqually: TCheckBox
      Left = 181
      Top = 140
      Width = 158
      Height = 17
      Caption = 'space labels equally in Y axis'
      TabOrder = 9
    end
    object grboxCaption2: TGroupBox
      Left = 8
      Top = 202
      Width = 571
      Height = 44
      Caption = 'Caption #2'
      TabOrder = 11
      object chbCaption2: TCheckBox
        Left = 9
        Top = 18
        Width = 46
        Height = 17
        Caption = 'text'
        TabOrder = 0
      end
      object chbFont2: TCheckBox
        Left = 70
        Top = 18
        Width = 42
        Height = 17
        Caption = 'font'
        TabOrder = 1
      end
      object chbLabelPosition2: TCheckBox
        Left = 136
        Top = 18
        Width = 60
        Height = 17
        Caption = 'position'
        TabOrder = 2
      end
      object chbCenterTextHorizontally2: TCheckBox
        Left = 228
        Top = 18
        Width = 135
        Height = 17
        Caption = 'center text horizontally'
        TabOrder = 3
      end
    end
    object btnSelectAll: TButton
      Left = 16
      Top = 18
      Width = 75
      Height = 23
      Caption = 'Select all'
      TabOrder = 0
      OnClick = btnSelectAllClick
    end
    object btnDeselectAll: TButton
      Left = 97
      Top = 18
      Width = 75
      Height = 23
      Caption = 'Deselect all'
      TabOrder = 1
      OnClick = btnDeselectAllClick
    end
  end
  object cbTargetType: TComboBox
    Left = 277
    Top = 5
    Width = 319
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 1
    Text = 'single button'
    OnChange = cbTargetTypeChange
    Items.Strings = (
      'single button'
      'all visible buttons'
      'button range'
      'all buttons from the same container'
      'all buttons')
  end
  object cbTargetTo: TComboBox
    Left = 451
    Top = 29
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 0
    TabOrder = 3
  end
  object popupCopy: TPopupMenu
    Left = 8
    Top = 32
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

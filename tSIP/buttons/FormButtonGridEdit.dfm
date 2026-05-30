object frmButtonGridEdit: TfrmButtonGridEdit
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Arrange buttons in a grid, starting from base button'
  ClientHeight = 281
  ClientWidth = 449
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnKeyPress = FormKeyPress
  PixelsPerInch = 96
  TextHeight = 13
  object lblTo: TLabel
    Left = 247
    Top = 11
    Width = 10
    Height = 13
    Caption = 'to'
  end
  object lblColumns: TLabel
    Left = 8
    Top = 52
    Width = 40
    Height = 13
    Caption = 'Columns'
  end
  object lblRows: TLabel
    Left = 8
    Top = 76
    Width = 26
    Height = 13
    Caption = 'Rows'
  end
  object lblFrom: TLabel
    Left = 8
    Top = 11
    Width = 59
    Height = 13
    Caption = 'From button'
  end
  object lblColumnSpacing: TLabel
    Left = 152
    Top = 52
    Width = 74
    Height = 13
    Caption = 'Column spacing'
  end
  object lblRowSpacing: TLabel
    Left = 152
    Top = 76
    Width = 60
    Height = 13
    Caption = 'Row spacing'
  end
  object lblButtonOrder: TLabel
    Left = 8
    Top = 108
    Width = 61
    Height = 13
    Caption = 'Button order'
  end
  object lblDefaultWidth: TLabel
    Left = 288
    Top = 52
    Width = 139
    Height = 13
    Caption = '(default = first button width)'
  end
  object lblDefaultHeight: TLabel
    Left = 288
    Top = 76
    Width = 143
    Height = 13
    Caption = '(default = first button height)'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 244
    Width = 449
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 11
    ExplicitTop = 165
    ExplicitWidth = 432
    DesignSize = (
      449
      37)
    object btnCancel: TButton
      Left = 369
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Close'
      TabOrder = 1
      OnClick = btnCancelClick
      ExplicitLeft = 352
    end
    object btnApply: TButton
      Left = 288
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
      ExplicitLeft = 271
    end
  end
  object cbTargetTo: TComboBox
    Left = 267
    Top = 8
    Width = 160
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 1
  end
  object cbColumns: TComboBox
    Left = 60
    Top = 49
    Width = 53
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 2
  end
  object cbRows: TComboBox
    Left = 60
    Top = 73
    Width = 53
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 4
  end
  object edSourceButton: TEdit
    Left = 73
    Top = 8
    Width = 160
    Height = 21
    Color = clBtnFace
    ReadOnly = True
    TabOrder = 0
  end
  object edColumnSpacing: TEdit
    Left = 232
    Top = 49
    Width = 41
    Height = 21
    TabOrder = 3
    Text = '100'
  end
  object edRowSpacing: TEdit
    Left = 232
    Top = 73
    Width = 41
    Height = 21
    TabOrder = 5
    Text = '30'
  end
  object cbButtonOrder: TComboBox
    Left = 73
    Top = 105
    Width = 354
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 6
    Text = 'starting with a column (top to bottom, then next column)'
    Items.Strings = (
      'starting with a column (top to bottom, then next column)'
      'starting with a row (left to right, then next row)')
  end
  object chbCopyButtonWidth: TCheckBox
    Left = 8
    Top = 142
    Width = 419
    Height = 17
    Caption = 'Copy button width from the first button to others'
    TabOrder = 7
  end
  object chbCopyButtonHeight: TCheckBox
    Left = 8
    Top = 161
    Width = 419
    Height = 17
    Caption = 'Copy button height from the first button to others'
    TabOrder = 8
  end
  object cbActionVisible: TComboBox
    Left = 8
    Top = 184
    Width = 225
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 9
    Text = 'make all buttons visible'
    Items.Strings = (
      'make all buttons visible'
      'skip buttons not visible')
  end
  object cbActionContainer: TComboBox
    Left = 8
    Top = 211
    Width = 225
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 10
    Text = 'skip buttons assigned to other containers'
    Items.Strings = (
      'skip buttons assigned to other containers'
      'bring all buttons to the same container')
  end
end

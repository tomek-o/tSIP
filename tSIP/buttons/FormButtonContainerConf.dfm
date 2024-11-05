object frmButtonContainerConf: TfrmButtonContainerConf
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Button container configuration'
  ClientHeight = 144
  ClientWidth = 407
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
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object lblBackgroundBitmap: TLabel
    Left = 8
    Top = 32
    Width = 91
    Height = 13
    Caption = 'Background bitmap'
  end
  object lblBackgroundColor: TLabel
    Left = 8
    Top = 80
    Width = 82
    Height = 13
    Caption = 'Background color'
  end
  object shColorBackground: TShape
    Left = 351
    Top = 77
    Width = 41
    Height = 21
  end
  object lblColorPreview: TLabel
    Left = 298
    Top = 80
    Width = 38
    Height = 13
    Caption = 'preview'
  end
  object lblContainer: TLabel
    Left = 8
    Top = 8
    Width = 47
    Height = 13
    Caption = 'Container'
  end
  object lblContainerName: TLabel
    Left = 105
    Top = 8
    Width = 100
    Height = 13
    Caption = 'lblContainerName'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 107
    Width = 407
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 4
    DesignSize = (
      407
      37)
    object btnCancel: TButton
      Left = 323
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 234
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object edBackgroundBitmap: TEdit
    Left = 105
    Top = 29
    Width = 256
    Height = 21
    TabOrder = 0
  end
  object btnSelectBackgroundBitmap: TButton
    Left = 367
    Top = 29
    Width = 25
    Height = 21
    Caption = '...'
    TabOrder = 1
    OnClick = btnSelectBackgroundBitmapClick
  end
  object cbBackgroundColor: TComboBox
    Left = 105
    Top = 77
    Width = 136
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnChange = cbBackgroundColorChange
  end
  object btnSelectBackgroundColor: TButton
    Left = 247
    Top = 76
    Width = 21
    Height = 21
    Caption = '...'
    TabOrder = 3
    OnClick = btnSelectBackgroundColorClick
  end
  object chbBackgroundImageTransparent: TCheckBox
    Left = 8
    Top = 56
    Width = 390
    Height = 17
    Caption = 
      'Make background image transparent (transparent color from left-t' +
      'op pixel)'
    TabOrder = 5
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 8
    Top = 104
  end
  object colorDialog: TColorDialog
    Left = 40
    Top = 104
  end
end

object frmButtonContainerConf: TfrmButtonContainerConf
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Button container configuration'
  ClientHeight = 121
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
    Top = 8
    Width = 91
    Height = 13
    Caption = 'Background bitmap'
  end
  object lblBackgroundColor: TLabel
    Left = 8
    Top = 56
    Width = 82
    Height = 13
    Caption = 'Background color'
  end
  object shColorBackground: TShape
    Left = 357
    Top = 53
    Width = 41
    Height = 21
  end
  object lblColorPreview: TLabel
    Left = 299
    Top = 56
    Width = 38
    Height = 13
    Caption = 'preview'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 84
    Width = 407
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 4
    ExplicitTop = 71
    ExplicitWidth = 386
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
      ExplicitLeft = 302
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
      ExplicitLeft = 213
    end
  end
  object edBackgroundBitmap: TEdit
    Left = 105
    Top = 5
    Width = 261
    Height = 21
    TabOrder = 0
  end
  object btnSelectBackgroundBitmap: TButton
    Left = 373
    Top = 5
    Width = 25
    Height = 21
    Caption = '...'
    TabOrder = 1
    OnClick = btnSelectBackgroundBitmapClick
  end
  object cbBackgroundColor: TComboBox
    Left = 105
    Top = 53
    Width = 144
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnChange = cbBackgroundColorChange
  end
  object btnSelectBackgroundColor: TButton
    Left = 258
    Top = 53
    Width = 21
    Height = 21
    Caption = '...'
    TabOrder = 3
    OnClick = btnSelectBackgroundColorClick
  end
  object chbBackgroundImageTransparent: TCheckBox
    Left = 8
    Top = 32
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
    Top = 88
  end
  object colorDialog: TColorDialog
    Left = 40
    Top = 88
  end
end

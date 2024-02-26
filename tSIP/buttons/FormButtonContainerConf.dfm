object frmButtonContainerConf: TfrmButtonContainerConf
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Button container configuration'
  ClientHeight = 108
  ClientWidth = 386
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
    Caption = 'background bitmap'
  end
  object lblBackgroundColor: TLabel
    Left = 10
    Top = 45
    Width = 82
    Height = 13
    Caption = 'background color'
  end
  object shColorBackground: TShape
    Left = 336
    Top = 42
    Width = 41
    Height = 21
  end
  object lblColorPreview: TLabel
    Left = 292
    Top = 45
    Width = 38
    Height = 13
    Caption = 'preview'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 71
    Width = 386
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 4
    DesignSize = (
      386
      37)
    object btnCancel: TButton
      Left = 302
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 213
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
    Top = 5
    Width = 242
    Height = 21
    TabOrder = 0
  end
  object btnSelectBackgroundBitmap: TButton
    Left = 352
    Top = 5
    Width = 25
    Height = 21
    Caption = '...'
    TabOrder = 1
    OnClick = btnSelectBackgroundBitmapClick
  end
  object cbBackgroundColor: TComboBox
    Left = 105
    Top = 42
    Width = 140
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnChange = cbBackgroundColorChange
  end
  object btnSelectBackgroundColor: TButton
    Left = 251
    Top = 42
    Width = 21
    Height = 21
    Caption = '...'
    TabOrder = 3
    OnClick = btnSelectBackgroundColorClick
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 8
    Top = 72
  end
  object colorDialog: TColorDialog
    Left = 40
    Top = 72
  end
end

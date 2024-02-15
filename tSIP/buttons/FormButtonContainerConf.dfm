object frmButtonContainerConf: TfrmButtonContainerConf
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Button container configuration'
  ClientHeight = 145
  ClientWidth = 339
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
  object lblBackgroundBitmap: TLabel
    Left = 8
    Top = 8
    Width = 91
    Height = 13
    Caption = 'Background bitmap'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 108
    Width = 339
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitTop = 236
    ExplicitWidth = 544
    DesignSize = (
      339
      37)
    object btnCancel: TButton
      Left = 255
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
      ExplicitLeft = 454
    end
    object btnApply: TButton
      Left = 166
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
      ExplicitLeft = 204
    end
  end
  object edBackgroundBitmap: TEdit
    Left = 105
    Top = 5
    Width = 192
    Height = 21
    TabOrder = 1
  end
  object btnSelectBackgroundBitmap: TButton
    Left = 303
    Top = 5
    Width = 25
    Height = 21
    Caption = '...'
    TabOrder = 2
    OnClick = btnSelectBackgroundBitmapClick
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 8
    Top = 48
  end
end

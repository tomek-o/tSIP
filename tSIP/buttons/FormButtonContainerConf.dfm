object frmButtonContainerConf: TfrmButtonContainerConf
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Button container configuration'
  ClientHeight = 273
  ClientWidth = 544
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
  object pnlBottom: TPanel
    Left = 0
    Top = 236
    Width = 544
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitLeft = -208
    ExplicitTop = 209
    ExplicitWidth = 657
    object btnCancel: TButton
      Left = 454
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
    end
    object btnApply: TButton
      Left = 362
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
    end
  end
end

object frmSettingsPatch: TfrmSettingsPatch
  Left = 0
  Top = 0
  Caption = 'Patch/update settings'
  ClientHeight = 282
  ClientWidth = 430
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnKeyDown = FormKeyDown
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 244
    Width = 430
    Height = 38
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitTop = 224
    ExplicitWidth = 358
    DesignSize = (
      430
      38)
    object btnApply: TButton
      Left = 350
      Top = 9
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
      ExplicitLeft = 278
    end
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 430
    Height = 25
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 1
    ExplicitWidth = 358
    object lblInfo: TLabel
      Left = 8
      Top = 6
      Width = 132
      Height = 13
      Caption = 'Update settings with JSON:'
    end
  end
end

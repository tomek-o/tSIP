object frmSettingsPatch: TfrmSettingsPatch
  Left = 0
  Top = 0
  Caption = 'Patch/update settings'
  ClientHeight = 262
  ClientWidth = 358
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
    Top = 224
    Width = 358
    Height = 38
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    DesignSize = (
      358
      38)
    object btnApply: TButton
      Left = 278
      Top = 9
      Width = 75
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object edJson: TMemo
    Left = 0
    Top = 25
    Width = 358
    Height = 199
    Align = alClient
    TabOrder = 1
    ExplicitLeft = 80
    ExplicitTop = 80
    ExplicitWidth = 185
    ExplicitHeight = 89
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 358
    Height = 25
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    object lblInfo: TLabel
      Left = 8
      Top = 6
      Width = 132
      Height = 13
      Caption = 'Update settings with JSON:'
    end
  end
end

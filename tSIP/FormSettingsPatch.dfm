object frmSettingsPatch: TfrmSettingsPatch
  Left = 0
  Top = 0
  Caption = 'Patch/update settings'
  ClientHeight = 282
  ClientWidth = 422
  Color = clBtnFace
  Constraints.MinHeight = 250
  Constraints.MinWidth = 430
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
    Width = 422
    Height = 38
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitTop = 224
    ExplicitWidth = 358
    DesignSize = (
      422
      38)
    object btnApply: TButton
      Left = 342
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
    Width = 422
    Height = 42
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 1
    object lblInfo: TLabel
      Left = 8
      Top = 4
      Width = 401
      Height = 13
      Caption = 
        'Update settings with JSON - text may contain only selected confi' +
        'guration elements.'
    end
    object Label1: TLabel
      Left = 8
      Top = 23
      Width = 271
      Height = 13
      Caption = 'File with JSON "patch" may be dropped into this window.'
    end
  end
end

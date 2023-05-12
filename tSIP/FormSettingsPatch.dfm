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
    end
  end
  object memo: TMemo
    Left = 0
    Top = 0
    Width = 422
    Height = 49
    Align = alTop
    Color = clBtnFace
    Lines.Strings = (
      
        'Update settings with JSON - text may contain whole configuration' +
        ' or only selected '
      'configuration elements.'
      'File with JSON "patch" may be dropped into this window.')
    ReadOnly = True
    TabOrder = 1
    ExplicitLeft = 8
    ExplicitTop = 8
    ExplicitWidth = 393
  end
end

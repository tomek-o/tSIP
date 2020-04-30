object frmMessage: TfrmMessage
  Left = 479
  Top = 165
  Caption = 'Text messages'
  ClientHeight = 273
  ClientWidth = 318
  Color = clBtnFace
  Constraints.MinHeight = 250
  Constraints.MinWidth = 250
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object memoMain: TRichEdit
    Left = 0
    Top = 31
    Width = 318
    Height = 135
    TabStop = False
    Align = alClient
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    HideScrollBars = False
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 1
    WantTabs = True
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 166
    Width = 318
    Height = 107
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object memoInput: TMemo
      Left = 0
      Top = 25
      Width = 318
      Height = 82
      Align = alClient
      MaxLength = 1989
      ScrollBars = ssVertical
      TabOrder = 0
      WantReturns = False
      OnKeyDown = memoInputKeyDown
    end
    object pnlButtons: TPanel
      Left = 0
      Top = 0
      Width = 318
      Height = 25
      Align = alTop
      BevelOuter = bvNone
      TabOrder = 1
      DesignSize = (
        318
        25)
      object btnSendOnEnter: TSpeedButton
        Left = 292
        Top = 3
        Width = 26
        Height = 21
        Hint = 'Send on [Enter]'
        AllowAllUp = True
        Anchors = [akTop, akRight]
        GroupIndex = 1
        Down = True
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          1800000000000003000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000
          0000000000000000000000000000000000000000000000000000000000000000
          00000000FFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFFFFFFFF}
        OnClick = btnSendOnEnterClick
      end
      object btnSend: TButton
        Left = 231
        Top = 3
        Width = 57
        Height = 21
        Anchors = [akTop, akRight]
        Caption = 'Send'
        TabOrder = 0
        OnClick = btnSendClick
      end
    end
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 318
    Height = 31
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 2
    DesignSize = (
      318
      31)
    object lblNumber: TLabel
      Left = 5
      Top = 7
      Width = 61
      Height = 13
      Caption = 'Number/URI'
    end
    object edTarget: TEdit
      Left = 80
      Top = 3
      Width = 238
      Height = 24
      Anchors = [akLeft, akTop, akRight]
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Courier New'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnChange = edTargetChange
    end
  end
end

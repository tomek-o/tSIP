object frmHotkeys: TfrmHotkeys
  Left = 0
  Top = 0
  Align = alClient
  BorderStyle = bsNone
  Caption = 'Hotkeys'
  ClientHeight = 311
  ClientWidth = 567
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 273
    Width = 567
    Height = 38
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnAddHotkey: TButton
      Left = 8
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Add hotkey'
      TabOrder = 0
      OnClick = btnAddHotkeyClick
    end
  end
  object ScrollBox1: TScrollBox
    Left = 0
    Top = 27
    Width = 567
    Height = 246
    Align = alClient
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    TabOrder = 1
    object panel: TFlowPanel
      Left = 0
      Top = 0
      Width = 551
      Height = 289
      Align = alTop
      AutoSize = True
      AutoWrap = False
      BevelOuter = bvNone
      FlowStyle = fsTopBottomLeftRight
      TabOrder = 0
    end
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 567
    Height = 27
    Align = alTop
    BevelWidth = 2
    TabOrder = 2
    object lblHeader: TLabel
      Left = 8
      Top = 7
      Width = 482
      Height = 13
      Caption = 
        'Global    Ctrl Shift  Alt                        Key            ' +
        '                              Action                     Paramet' +
        'er       '
    end
  end
  object timerRemove: TTimer
    Enabled = False
    Interval = 100
    OnTimer = timerRemoveTimer
    Left = 88
    Top = 280
  end
end

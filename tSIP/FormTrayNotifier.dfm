object frmTrayNotifier: TfrmTrayNotifier
  Left = 0
  Top = 0
  BorderStyle = bsToolWindow
  Caption = 'tSIP'
  ClientHeight = 83
  ClientWidth = 207
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  ScreenSnap = True
  OnClose = FormClose
  OnCreate = FormCreate
  DesignSize = (
    207
    83)
  PixelsPerInch = 96
  TextHeight = 13
  object lblDescription: TLabel
    Left = 8
    Top = 8
    Width = 191
    Height = 13
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Caption = 'lblDescription'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object lblUri: TLabel
    Left = 8
    Top = 27
    Width = 191
    Height = 13
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Caption = 'lblUri'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
    Transparent = True
  end
  object btnAnswer: TButton
    Left = 8
    Top = 50
    Width = 75
    Height = 25
    Caption = 'Answer'
    TabOrder = 1
    OnClick = btnAnswerClick
  end
  object btnHangup: TButton
    Left = 124
    Top = 50
    Width = 75
    Height = 25
    Caption = 'Hangup'
    TabOrder = 2
    OnClick = btnHangupClick
  end
  object btnStopFocus: TButton
    Left = 89
    Top = 72
    Width = 0
    Height = 0
    TabOrder = 0
    TabStop = False
  end
end

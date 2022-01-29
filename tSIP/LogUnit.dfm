object frmLog: TfrmLog
  Left = 0
  Top = 0
  Caption = 'Log'
  ClientHeight = 294
  ClientWidth = 635
  Color = clBtnFace
  Constraints.MinHeight = 80
  Constraints.MinWidth = 200
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  ScreenSnap = True
  OnClose = FormClose
  OnKeyPress = FormKeyPress
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object redMain: TRichEdit
    Left = 0
    Top = 0
    Width = 635
    Height = 248
    Align = alClient
    PopupMenu = pmenuRichEdit
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 248
    Width = 635
    Height = 46
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object btnClear: TButton
      Left = 5
      Top = 6
      Width = 41
      Height = 35
      Caption = 'Clear'
      TabOrder = 0
      OnClick = btnClearClick
    end
    object chbAutoScroll: TCheckBox
      Left = 151
      Top = 6
      Width = 90
      Height = 17
      Caption = 'Auto Scroll'
      Checked = True
      State = cbChecked
      TabOrder = 3
    end
    object chbLogMessages: TCheckBox
      Left = 244
      Top = 6
      Width = 251
      Height = 17
      Caption = 'Log received and transmitted SIP messages'
      TabOrder = 5
      OnClick = chbLogMessagesClick
    end
    object chbLogToFile: TCheckBox
      Left = 151
      Top = 22
      Width = 86
      Height = 17
      Caption = 'Log to file'
      TabOrder = 4
      OnClick = chbLogToFileClick
    end
    object chbLogMessagesOnlyFirstLines: TCheckBox
      Left = 260
      Top = 22
      Width = 235
      Height = 17
      Caption = 'only first lines (Request-Line/Status-Line)'
      TabOrder = 6
      OnClick = chbLogMessagesClick
    end
    object btnCopyAll: TButton
      Left = 45
      Top = 6
      Width = 92
      Height = 18
      Caption = 'Copy all'
      TabOrder = 1
      OnClick = btnCopyAllClick
    end
    object btnCopyLast: TButton
      Left = 45
      Top = 23
      Width = 92
      Height = 18
      Caption = 'Copy last lines'
      TabOrder = 2
      OnClick = btnCopyLastClick
    end
  end
  object tmrUpdate: TTimer
    Interval = 500
    OnTimer = tmrUpdateTimer
    Left = 8
    Top = 8
  end
  object pmenuRichEdit: TPopupMenu
    Left = 8
    Top = 40
    object miCopy: TMenuItem
      Caption = '&Copy'
      OnClick = miCopyClick
    end
    object miSaveToFile: TMenuItem
      Caption = 'Save to file'
      OnClick = miSaveToFileClick
    end
  end
  object saveDialog: TSaveDialog
    DefaultExt = 'log'
    Filter = 'Log files (*.log)|*.log|Text files (*.txt)|*.txt|Any files|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 8
    Top = 72
  end
end

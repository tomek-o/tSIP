object frmLog: TfrmLog
  Left = 0
  Top = 0
  Caption = 'Log'
  ClientHeight = 406
  ClientWidth = 640
  Color = clBtnFace
  Constraints.MinHeight = 400
  Constraints.MinWidth = 600
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
  object splLog: TSplitter
    Left = 0
    Top = 195
    Width = 640
    Height = 5
    Cursor = crVSplit
    Align = alBottom
    Visible = False
    ExplicitTop = 0
  end
  object redMain: TRichEdit
    Left = 0
    Top = 0
    Width = 640
    Height = 195
    Align = alClient
    PopupMenu = pmenuRichEdit
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 18
    ExplicitHeight = 190
  end
  object lvLog: TListView
    Left = 0
    Top = 0
    Width = 640
    Height = 195
    Align = alClient
    Columns = <
      item
        Caption = 'Level'
        Width = 70
      end
      item
        Caption = 'Message'
        Width = 500
      end>
    HideSelection = False
    MultiSelect = True
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    PopupMenu = pmenuList
    TabOrder = 19
    ViewStyle = vsReport
    Visible = False
    OnCustomDrawItem = lvLogCustomDrawItem
    OnCustomDrawSubItem = lvLogCustomDrawSubItem
    OnData = lvLogData
    OnKeyDown = lvLogKeyDown
    OnResize = lvLogResize
    OnSelectItem = lvLogSelectItem
    ExplicitHeight = 190
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 280
    Width = 640
    Height = 126
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 22
    object lblLevels: TLabel
      Left = 5
      Top = 76
      Width = 60
      Height = 13
      Caption = 'Show levels:'
    end
    object lblFind: TLabel
      Left = 5
      Top = 104
      Width = 24
      Height = 13
      Caption = 'Find:'
    end
    object lblSource: TLabel
      Left = 335
      Top = 50
      Width = 19
      Height = 13
      Caption = 'Live'
    end
    object lblViewType: TLabel
      Left = 5
      Top = 50
      Width = 47
      Height = 13
      Caption = 'View type'
    end
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
    object chbLevelSip: TCheckBox
      Left = 80
      Top = 74
      Width = 66
      Height = 17
      Caption = 'SIP'
      Checked = True
      State = cbChecked
      TabOrder = 8
      OnClick = chbLevelClick
    end
    object chbLevelError: TCheckBox
      Left = 150
      Top = 74
      Width = 66
      Height = 17
      Caption = 'Error'
      Checked = True
      State = cbChecked
      TabOrder = 9
      OnClick = chbLevelClick
    end
    object chbLevelWarning: TCheckBox
      Left = 220
      Top = 74
      Width = 66
      Height = 17
      Caption = 'Warning'
      Checked = True
      State = cbChecked
      TabOrder = 10
      OnClick = chbLevelClick
    end
    object chbLevelInfo: TCheckBox
      Left = 290
      Top = 74
      Width = 66
      Height = 17
      Caption = 'Info'
      Checked = True
      State = cbChecked
      TabOrder = 11
      OnClick = chbLevelClick
    end
    object chbLevelDebug: TCheckBox
      Left = 360
      Top = 74
      Width = 66
      Height = 17
      Caption = 'Debug'
      Checked = True
      State = cbChecked
      TabOrder = 12
      OnClick = chbLevelClick
    end
    object edSearch: TEdit
      Left = 80
      Top = 100
      Width = 260
      Height = 21
      TabOrder = 13
      OnChange = searchFilterChange
    end
    object chbMatchCase: TCheckBox
      Left = 350
      Top = 102
      Width = 90
      Height = 17
      Caption = 'Match case'
      TabOrder = 14
      OnClick = searchFilterChange
    end
    object cbViewMode: TComboBox
      Left = 80
      Top = 47
      Width = 64
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      ItemIndex = 0
      TabOrder = 15
      Text = 'Text'
      OnChange = cbViewModeChange
      Items.Strings = (
        'Text'
        'List')
    end
    object btnOpenLog: TButton
      Left = 150
      Top = 47
      Width = 110
      Height = 21
      Caption = 'Open log file...'
      TabOrder = 16
      OnClick = btnOpenLogClick
    end
    object btnLive: TButton
      Left = 265
      Top = 47
      Width = 60
      Height = 21
      Caption = 'Live'
      Enabled = False
      TabOrder = 17
      OnClick = btnLiveClick
    end
    object btnMore: TButton
      Left = 489
      Top = 6
      Width = 48
      Height = 35
      Caption = 'More...'
      TabOrder = 7
      OnClick = btnMoreClick
    end
  end
  object memoLog: TMemo
    Left = 0
    Top = 200
    Width = 640
    Height = 80
    Align = alBottom
    ReadOnly = True
    ScrollBars = ssVertical
    TabOrder = 20
    Visible = False
    ExplicitTop = 195
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
  object pmenuList: TPopupMenu
    Left = 8
    Top = 104
    object miListCopySel: TMenuItem
      Caption = 'Copy selected'
      OnClick = miListCopySelectedClick
    end
    object miListCopy: TMenuItem
      Caption = 'Copy all'
      OnClick = miListCopyAllClick
    end
    object miListSelect: TMenuItem
      Caption = 'Select all'
      OnClick = miListSelectAllClick
    end
  end
  object dlgOpenLog: TOpenDialog
    Filter = 'Log files (*.log)|*.log|All files (*.*)|*.*'
    Left = 8
    Top = 136
  end
end

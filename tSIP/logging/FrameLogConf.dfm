object fraLogConf: TfraLogConf
  Left = 0
  Top = 0
  Width = 451
  Height = 304
  Align = alClient
  TabOrder = 0
  TabStop = True
  object lblLogMaxFileSize: TLabel
    Left = 5
    Top = 48
    Width = 354
    Height = 13
    Caption = 
      'Log file size limit (0 = no limit); file would be reset/rotated ' +
      'when exceeded'
  end
  object lblLogRotate: TLabel
    Left = 5
    Top = 72
    Width = 208
    Height = 13
    Caption = 'Number of old log files to keep ("logrotate")'
  end
  object lblUiCapacity: TLabel
    Left = 5
    Top = 114
    Width = 194
    Height = 13
    Caption = 'Visible (buffered) log lines in log window:'
  end
  object lblLoggingConsoleFont: TLabel
    Left = 5
    Top = 144
    Width = 79
    Height = 13
    Caption = 'Log window font'
  end
  object chbLogToFile: TCheckBox
    Left = 5
    Top = 3
    Width = 325
    Height = 17
    Caption = 'Log to file'
    TabOrder = 0
  end
  object chbLogFlush: TCheckBox
    Left = 5
    Top = 25
    Width = 325
    Height = 17
    Caption = 'Flush log file on each line added'
    TabOrder = 1
  end
  object chbLogTimestamps: TCheckBox
    Left = 5
    Top = 91
    Width = 325
    Height = 17
    Caption = 'Add timestamps'
    TabOrder = 2
  end
  object chbLogShowWindowAtStartup: TCheckBox
    Left = 5
    Top = 168
    Width = 325
    Height = 17
    Caption = 'Show log window at startup'
    TabOrder = 3
  end
  object edLoggingConsoleFont: TEdit
    Left = 98
    Top = 141
    Width = 201
    Height = 21
    TabOrder = 4
    Text = 'abcABC 123 () [] <>,.'
  end
  object btnLoggingConsoleFontSelect: TButton
    Left = 305
    Top = 139
    Width = 26
    Height = 25
    Caption = '...'
    TabOrder = 5
    OnClick = btnLoggingConsoleFontSelectClick
  end
  object cmbMaxUiLogLines: TComboBox
    Left = 365
    Top = 111
    Width = 111
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 6
    Items.Strings = (
      '100'
      '200'
      '500'
      '1000'
      '2000'
      '5000'
      '10000')
  end
  object cbLogRotate: TComboBox
    Left = 365
    Top = 69
    Width = 111
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 7
    Text = '1'
    Items.Strings = (
      '0'
      '1'
      '2'
      '3'
      '4'
      '5')
  end
  object cbLogMaxFileSize: TComboBox
    Left = 365
    Top = 45
    Width = 111
    Height = 21
    ItemHeight = 13
    TabOrder = 8
    Items.Strings = (
      '1048576'
      '2097152'
      '5242880'
      '10485760'
      '104857600')
  end
  object fontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [fdEffects, fdForceFontExist]
    Left = 344
    Top = 137
  end
end

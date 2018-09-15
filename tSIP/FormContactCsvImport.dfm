object frmContactCsvImport: TfrmContactCsvImport
  Left = 0
  Top = 0
  Caption = 'Import contacts from CSV file'
  ClientHeight = 244
  ClientWidth = 318
  Color = clBtnFace
  DefaultMonitor = dmMainForm
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object pcWizard: TPageControl
    Left = 0
    Top = 0
    Width = 318
    Height = 205
    ActivePage = tsConfigureCsv
    Align = alClient
    Style = tsFlatButtons
    TabOrder = 0
    object tsConfigureCsv: TTabSheet
      Caption = 'tsConfigureCsv'
      object lblSeparator: TLabel
        Left = 80
        Top = 40
        Width = 72
        Height = 13
        Caption = 'Field separator'
      end
      object Label1: TLabel
        Left = 8
        Top = 8
        Width = 119
        Height = 13
        Caption = 'Configure CSV reader'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label3: TLabel
        Left = 8
        Top = 135
        Width = 160
        Height = 13
        Caption = 'For CSV format description go to:'
      end
      object lblInfo2: TLabel
        Left = 27
        Top = 154
        Width = 258
        Height = 13
        Cursor = crHandPoint
        Caption = 'http://en.wikipedia.org/wiki/Comma-separated_values'
        Color = clBtnFace
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clBlue
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsUnderline]
        ParentColor = False
        ParentFont = False
        OnClick = lblInfo2Click
      end
      object edFieldSeparator: TMemo
        Left = 166
        Top = 35
        Width = 34
        Height = 20
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Tahoma'
        Font.Style = [fsBold]
        MaxLength = 1
        ParentFont = False
        TabOrder = 0
        WantTabs = True
      end
      object Memo1: TMemo
        Left = 5
        Top = 72
        Width = 301
        Height = 57
        BevelEdges = []
        BevelInner = bvNone
        BevelOuter = bvNone
        BorderStyle = bsNone
        Color = clBtnFace
        Lines.Strings = (
          'Notes:'
          '- incorrect separator would give incorrect parsing result or '
          'file would not be parsed at all'
          '')
        TabOrder = 1
      end
    end
    object tsReadCsv: TTabSheet
      Caption = 'tsReadCsv'
      ImageIndex = 1
      object Label2: TLabel
        Left = 8
        Top = 8
        Width = 139
        Height = 13
        Caption = 'Verify CSV reader output'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label4: TLabel
        Left = 50
        Top = 43
        Width = 44
        Height = 13
        Caption = 'Columns:'
      end
      object Label5: TLabel
        Left = 170
        Top = 43
        Width = 30
        Height = 13
        Caption = 'Rows:'
      end
      object edColumns: TEdit
        Left = 100
        Top = 40
        Width = 48
        Height = 21
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 0
        Text = '-'
      end
      object edRows: TEdit
        Left = 206
        Top = 40
        Width = 46
        Height = 21
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 1
        Text = '-'
      end
    end
    object tsGenerateJson: TTabSheet
      Caption = 'tsGenerateJson'
      ImageIndex = 2
      object Label7: TLabel
        Left = 8
        Top = 8
        Width = 84
        Height = 13
        Caption = 'Generate JSON'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label8: TLabel
        Left = 44
        Top = 40
        Width = 24
        Height = 13
        Caption = 'Style'
      end
      object Label9: TLabel
        Left = 44
        Top = 90
        Width = 86
        Height = 13
        Caption = 'Decimal separator'
      end
      object cbJsonStyle: TComboBox
        Left = 77
        Top = 37
        Width = 203
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 0
        Text = 'Properties (first CSV line = header)'
        Items.Strings = (
          'Properties (first CSV line = header)'
          'Column arrays (first CSV line = header)'
          'Row arrays')
      end
      object cbDecimalSeparator: TComboBox
        Left = 185
        Top = 87
        Width = 35
        Height = 21
        Style = csDropDownList
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ItemHeight = 13
        ItemIndex = 0
        ParentFont = False
        TabOrder = 1
        Text = '.'
        Items.Strings = (
          '.'
          ',')
      end
      object chbExtractNumbers: TCheckBox
        Left = 44
        Top = 67
        Width = 236
        Height = 17
        Caption = 'Extract numbers from CSV fields'
        Checked = True
        State = cbChecked
        TabOrder = 2
        OnClick = chbExtractNumbersClick
        OnKeyPress = chbExtractNumbersKeyPress
      end
    end
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 205
    Width = 318
    Height = 39
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object Bevel1: TBevel
      Left = 4
      Top = 0
      Width = 309
      Height = 3
    end
    object pnlBottomRight: TPanel
      AlignWithMargins = True
      Left = 72
      Top = 3
      Width = 243
      Height = 33
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 0
      object btnPrev: TButton
        Left = 76
        Top = 6
        Width = 75
        Height = 25
        Caption = '< Previous'
        TabOrder = 0
        OnClick = btnPrevClick
      end
      object btnNext: TButton
        Left = 160
        Top = 6
        Width = 75
        Height = 25
        Caption = 'Next >'
        TabOrder = 1
        OnClick = btnNextClick
      end
    end
  end
end

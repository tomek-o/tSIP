object frmContactEditor: TfrmContactEditor
  Left = 0
  Top = 0
  Caption = 'Add/edit contact'
  ClientHeight = 349
  ClientWidth = 434
  Color = clBtnFace
  Constraints.MinHeight = 376
  Constraints.MinWidth = 442
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poScreenCenter
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  DesignSize = (
    434
    349)
  PixelsPerInch = 96
  TextHeight = 13
  object lblDescription: TLabel
    Left = 8
    Top = 8
    Width = 53
    Height = 13
    Caption = 'Description'
  end
  object lblNumber1: TLabel
    Left = 8
    Top = 55
    Width = 54
    Height = 13
    Caption = 'Number #1'
  end
  object lblNote: TLabel
    Left = 8
    Top = 128
    Width = 23
    Height = 13
    Caption = 'Note'
  end
  object lblNumber2: TLabel
    Left = 8
    Top = 78
    Width = 54
    Height = 13
    Caption = 'Number #2'
  end
  object lblNumber3: TLabel
    Left = 8
    Top = 101
    Width = 54
    Height = 13
    Caption = 'Number #3'
  end
  object lblCompany: TLabel
    Left = 8
    Top = 31
    Width = 45
    Height = 13
    Caption = 'Company'
  end
  object lblContactFile: TLabel
    Left = 8
    Top = 288
    Width = 16
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'File'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 312
    Width = 434
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 6
    DesignSize = (
      434
      37)
    object btnCancel: TButton
      Left = 348
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akRight, akBottom]
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 257
      Top = 6
      Width = 75
      Height = 25
      Anchors = [akRight, akBottom]
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
  end
  object edDescription: TEdit
    Left = 96
    Top = 5
    Width = 177
    Height = 21
    TabOrder = 0
  end
  object edNumber1: TEdit
    Left = 96
    Top = 52
    Width = 177
    Height = 21
    TabOrder = 2
  end
  object memoNote: TRichEdit
    Left = 48
    Top = 125
    Width = 375
    Height = 158
    Anchors = [akLeft, akTop, akRight, akBottom]
    PlainText = True
    TabOrder = 5
    OnChange = memoNoteChange
  end
  object edNumber2: TEdit
    Left = 96
    Top = 75
    Width = 177
    Height = 21
    TabOrder = 3
  end
  object edNumber3: TEdit
    Left = 96
    Top = 98
    Width = 177
    Height = 21
    TabOrder = 4
  end
  object edCompany: TEdit
    Left = 96
    Top = 28
    Width = 177
    Height = 21
    TabOrder = 1
  end
  object edFile: TEdit
    Left = 48
    Top = 285
    Width = 324
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 7
  end
  object btnFileSelect: TButton
    Left = 377
    Top = 285
    Width = 22
    Height = 21
    Anchors = [akRight, akBottom]
    Caption = '...'
    TabOrder = 8
    OnClick = btnFileSelectClick
  end
  object btnFileOpen: TBitBtn
    Left = 401
    Top = 285
    Width = 22
    Height = 21
    Anchors = [akRight, akBottom]
    TabOrder = 9
    OnClick = btnFileOpenClick
    Glyph.Data = {
      36030000424D3603000000000000360000002800000010000000100000000100
      18000000000000030000C40E0000C40E00000000000000000000FFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000
      000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFF000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000
      D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8
      FF000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000D8FF000000FFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FF000000000000000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
      FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000FFFFFF}
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 272
  end
end

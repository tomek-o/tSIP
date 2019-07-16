object frmContactEditor: TfrmContactEditor
  Left = 0
  Top = 0
  Caption = 'Add/edit contact'
  ClientHeight = 319
  ClientWidth = 423
  Color = clBtnFace
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
    Width = 46
    Height = 13
    Caption = 'Number 1'
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
    Width = 46
    Height = 13
    Caption = 'Number 2'
  end
  object lblNumber3: TLabel
    Left = 8
    Top = 101
    Width = 46
    Height = 13
    Caption = 'Number 3'
  end
  object lblCompany: TLabel
    Left = 8
    Top = 31
    Width = 45
    Height = 13
    Caption = 'Company'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 282
    Width = 423
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 3
    object btnCancel: TButton
      Left = 330
      Top = 7
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 249
      Top = 7
      Width = 75
      Height = 25
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
    TabOrder = 5
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
end

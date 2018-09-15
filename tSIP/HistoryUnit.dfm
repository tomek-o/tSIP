object frmHistory: TfrmHistory
  Left = 0
  Top = 0
  Align = alClient
  BorderStyle = bsNone
  Caption = 'History'
  ClientHeight = 311
  ClientWidth = 318
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lvHistory: TListView
    Left = 0
    Top = 0
    Width = 318
    Height = 311
    Align = alClient
    Columns = <
      item
        Caption = 'Timestamp'
        Width = 100
      end
      item
        Caption = 'Number'
        Width = 100
      end>
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 0
    ViewStyle = vsReport
    ExplicitLeft = 24
    ExplicitTop = 64
    ExplicitWidth = 250
    ExplicitHeight = 150
  end
end

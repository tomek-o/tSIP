object frmScriptPluginVariables: TfrmScriptPluginVariables
  Left = 0
  Top = 0
  Caption = 'Scripts and plugins variables'
  ClientHeight = 313
  ClientWidth = 448
  Color = clBtnFace
  Constraints.MinHeight = 100
  Constraints.MinWidth = 200
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 272
    Width = 448
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnRead: TButton
      Left = 16
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Refresh'
      TabOrder = 0
      OnClick = btnReadClick
    end
    object chbAutoRefresh: TCheckBox
      Left = 104
      Top = 10
      Width = 97
      Height = 17
      Caption = 'auto refresh'
      TabOrder = 1
      OnClick = chbAutoRefreshClick
      OnMouseDown = chbAutoRefreshMouseDown
    end
  end
  object lvVariables: TListView
    Left = 0
    Top = 0
    Width = 448
    Height = 272
    Align = alClient
    Columns = <
      item
        Caption = 'Name'
        Width = 150
      end
      item
        AutoSize = True
        Caption = 'Value'
      end>
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 1
    ViewStyle = vsReport
    OnData = lvVariablesData
  end
  object tmrAutoRefresh: TTimer
    Enabled = False
    Interval = 500
    OnTimer = tmrAutoRefreshTimer
    Left = 208
    Top = 280
  end
end

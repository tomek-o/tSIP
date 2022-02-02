object frmLuaScriptHelp: TfrmLuaScriptHelp
  Left = 0
  Top = 0
  Caption = 'Lua help - custom functions'
  ClientHeight = 382
  ClientWidth = 600
  Color = clBtnFace
  Constraints.MinHeight = 200
  Constraints.MinWidth = 200
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Splitter: TSplitter
    Left = 0
    Top = 267
    Width = 600
    Height = 3
    Cursor = crVSplit
    Align = alBottom
    MinSize = 50
    ExplicitTop = 0
    ExplicitWidth = 270
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 270
    Width = 600
    Height = 112
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object pnlBottom2: TPanel
      Left = 0
      Top = 83
      Width = 600
      Height = 29
      Align = alBottom
      BevelOuter = bvNone
      TabOrder = 0
      object imgFilter: TImage
        Left = 6
        Top = 6
        Width = 17
        Height = 19
        Picture.Data = {
          07544269746D617036040000424D360400000000000036000000280000001000
          0000100000000100200000000000000400000000000000000000000000000000
          0000FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF0000000000FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF0000000000000000000000
          0000FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00000000000000000000000000FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF00000000000000000000000000FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00000000000000000000000000FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF007F7F7F000000000000000000000000007F7F
          7F00FF00FF0000FFFF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00000000007F7F7F007F7F7F007F7F7F007F7F7F007F7F
          7F00000000000000000000FFFF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00000000007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF007F7F7F007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF00FFFFFF007F7F7F007F7F7F00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00000000007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF00FFFFFF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00000000007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF00FFFFFF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00000000007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF00FFFFFF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF007F7F7F007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF00FFFFFF007F7F7F007F7F7F00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00000000007F7F7F00FFFFFF00FFFFFF00FFFFFF00FFFFFF00FFFF
          FF007F7F7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00000000007F7F7F007F7F7F007F7F7F007F7F7F007F7F
          7F0000000000FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00FF00FF00FF00FF00FF00FF007F7F7F000000000000000000000000007F7F
          7F00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00FF00
          FF00}
        Transparent = True
      end
      object edFilter: TEdit
        Left = 29
        Top = 4
        Width = 140
        Height = 21
        TabOrder = 0
        OnChange = edFilterChange
      end
    end
    object memoDescription: TRichEdit
      Left = 0
      Top = 0
      Width = 600
      Height = 83
      Align = alClient
      PlainText = True
      ReadOnly = True
      ScrollBars = ssVertical
      TabOrder = 1
    end
  end
  object lvSymbols: TListView
    Left = 0
    Top = 0
    Width = 600
    Height = 267
    Align = alClient
    Columns = <
      item
        Caption = 'Name'
        Width = 150
      end
      item
        AutoSize = True
        Caption = 'Brief'
      end>
    HideSelection = False
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    PopupMenu = popupList
    TabOrder = 1
    ViewStyle = vsReport
    OnData = lvSymbolsData
    OnSelectItem = lvSymbolsSelectItem
  end
  object popupList: TPopupMenu
    Left = 16
    Top = 112
    object miCopyFunctionName: TMenuItem
      Caption = 'Copy function name'
      OnClick = miCopyFunctionNameClick
    end
    object miCopyBriefDescription: TMenuItem
      Caption = 'Copy brief + description'
      OnClick = miCopyBriefDescriptionClick
    end
  end
end

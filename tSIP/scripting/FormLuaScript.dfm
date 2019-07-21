object frmLuaScript: TfrmLuaScript
  Left = 0
  Top = 0
  Caption = 'Script'
  ClientHeight = 341
  ClientWidth = 493
  Color = clBtnFace
  Constraints.MinHeight = 300
  Constraints.MinWidth = 300
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 299
    Width = 493
    Height = 42
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object pnlBottom2: TPanel
      Left = 0
      Top = 0
      Width = 493
      Height = 41
      Align = alTop
      BevelOuter = bvNone
      TabOrder = 0
      object btnExecute: TButton
        Left = 8
        Top = 8
        Width = 75
        Height = 25
        Caption = 'Execute'
        TabOrder = 0
        OnClick = btnExecuteClick
      end
      object btnBreak: TButton
        Left = 98
        Top = 8
        Width = 75
        Height = 25
        Caption = 'Break'
        TabOrder = 1
        OnClick = btnBreakClick
      end
    end
  end
  object MainMenu: TMainMenu
    Left = 8
    Top = 208
    object miFile: TMenuItem
      Caption = 'File'
      OnClick = miFileClick
      object Open1: TMenuItem
        Action = actFileOpen
      end
      object miOpenRecent: TMenuItem
        Caption = 'Open Recent'
      end
      object Save1: TMenuItem
        Action = actFileSave
      end
      object SaveAs1: TMenuItem
        Action = actFileSaveAs
      end
    end
    object miHelp: TMenuItem
      Caption = 'Help'
      object miHelpExamples: TMenuItem
        Caption = 'Examples'
      end
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 
      'Lua files (*.lua)|*.lua|Text files (*.txt)|*.txt|All files (*.*)' +
      '|*.*'
    Left = 40
    Top = 208
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '*.lua'
    Filter = 
      'Lua files (*.lua)|*.lua|Text files (*.txt)|*.txt|All files (*.*)' +
      '|*.*'
    Options = [ofHideReadOnly, ofExtensionDifferent, ofEnableSizing]
    Left = 72
    Top = 208
  end
  object actionList: TActionList
    Left = 104
    Top = 208
    object actFileOpen: TAction
      Category = 'File'
      Caption = '&Open...'
      ShortCut = 16463
      OnExecute = actFileOpenExecute
    end
    object actFileSave: TAction
      Category = 'File'
      Caption = '&Save'
      ShortCut = 16467
      OnExecute = actFileSaveExecute
    end
    object actFileSaveAs: TAction
      Category = 'File'
      Caption = 'Save &As...'
      ShortCut = 24659
      OnExecute = actFileSaveAsExecute
    end
  end
end

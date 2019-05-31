object frmPhones: TfrmPhones
  Left = 0
  Top = 0
  Align = alClient
  BorderStyle = bsNone
  Caption = 'Phones'
  ClientHeight = 260
  ClientWidth = 530
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object pnlBottom: TPanel
    Left = 0
    Top = 227
    Width = 530
    Height = 33
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnToggleActivation: TButton
      Left = 8
      Top = 4
      Width = 161
      Height = 25
      Caption = 'Activate/deactivate selected'
      TabOrder = 0
      OnClick = btnToggleActivationClick
    end
    object btnRefreshList: TButton
      Left = 175
      Top = 4
      Width = 161
      Height = 25
      Caption = 'Refresh DLL list'
      TabOrder = 1
      OnClick = btnRefreshListClick
    end
  end
  object lvDlls: TListView
    Left = 0
    Top = 0
    Width = 530
    Height = 227
    Align = alClient
    Columns = <
      item
        Caption = 'Active'
        Width = 46
      end
      item
        Caption = 'Name'
        Width = 150
      end
      item
        Caption = 'Version'
        Width = 70
      end>
    HideSelection = False
    OwnerData = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 1
    ViewStyle = vsReport
    OnData = lvDllsData
    OnDblClick = lvDllsDblClick
  end
end

object frmContactPopup: TfrmContactPopup
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'Contact'
  ClientHeight = 113
  ClientWidth = 392
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object note: TRichEdit
    Left = 0
    Top = 0
    Width = 392
    Height = 113
    Align = alClient
    PlainText = True
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object PopupMenu: TPopupMenu
    Left = 8
    Top = 80
    object miRevertChanges: TMenuItem
      Caption = 'Revert changes'
      OnClick = miRevertChangesClick
    end
  end
end

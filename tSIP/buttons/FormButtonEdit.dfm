object frmButtonEdit: TfrmButtonEdit
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Edit button'
  ClientHeight = 482
  ClientWidth = 408
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object lblCaption: TLabel
    Left = 8
    Top = 88
    Width = 37
    Height = 13
    Caption = 'Caption'
  end
  object lblNumber: TLabel
    Left = 197
    Top = 88
    Width = 37
    Height = 13
    Caption = 'Number'
  end
  object lblType: TLabel
    Left = 8
    Top = 11
    Width = 24
    Height = 13
    Caption = 'Type'
  end
  object Label1: TLabel
    Left = 197
    Top = 113
    Width = 54
    Height = 13
    Caption = 'Caption #2'
  end
  object lblCaptionLines: TLabel
    Left = 8
    Top = 113
    Width = 61
    Height = 13
    Caption = 'Caption lines'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 445
    Width = 408
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnCancel: TButton
      Left = 326
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 245
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
    object btnClear: TButton
      Left = 8
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Reset all'
      TabOrder = 2
      OnClick = btnClearClick
    end
  end
  object edDescription: TEdit
    Left = 51
    Top = 85
    Width = 110
    Height = 21
    TabOrder = 1
  end
  object edNumber: TEdit
    Left = 240
    Top = 85
    Width = 153
    Height = 21
    TabOrder = 2
  end
  object cbType: TComboBox
    Left = 51
    Top = 8
    Width = 342
    Height = 21
    Style = csDropDownList
    DropDownCount = 12
    ItemHeight = 13
    TabOrder = 3
    OnChange = cbTypeChange
  end
  object memoHelp: TMemo
    Left = 51
    Top = 34
    Width = 342
    Height = 48
    TabStop = False
    BorderStyle = bsNone
    Color = clBtnFace
    ReadOnly = True
    TabOrder = 4
  end
  object pageControl: TPageControl
    Left = 8
    Top = 140
    Width = 393
    Height = 305
    ActivePage = tsBehavior
    TabOrder = 5
    object tsVisual: TTabSheet
      Caption = 'Visual'
      object lblHeight: TLabel
        Left = 3
        Top = 9
        Width = 31
        Height = 13
        Caption = 'Height'
      end
      object lblMarginTop: TLabel
        Left = 98
        Top = 9
        Width = 53
        Height = 13
        Caption = 'Top margin'
      end
      object lblMarginBottom: TLabel
        Left = 212
        Top = 9
        Width = 69
        Height = 13
        Caption = 'Bottom margin'
      end
      object lblBackgroundColor: TLabel
        Left = 3
        Top = 40
        Width = 82
        Height = 13
        Caption = 'Background color'
      end
      object edHeight: TEdit
        Left = 46
        Top = 6
        Width = 38
        Height = 21
        TabOrder = 0
      end
      object edMarginTop: TEdit
        Left = 159
        Top = 6
        Width = 40
        Height = 21
        TabOrder = 1
      end
      object edMarginBottom: TEdit
        Left = 287
        Top = 6
        Width = 40
        Height = 21
        TabOrder = 2
      end
      object grboxImage: TGroupBox
        Left = 3
        Top = 73
        Width = 370
        Height = 129
        Caption = 'Bitmaps (from "img" subdirectory)'
        TabOrder = 3
        object lblImgIdle: TLabel
          Left = 7
          Top = 41
          Width = 95
          Height = 13
          Caption = 'Idle/invalid/non-BLF'
        end
        object lblImgTerminated: TLabel
          Left = 7
          Top = 61
          Width = 76
          Height = 13
          Caption = 'BLF: terminated'
        end
        object lblImgEarly: TLabel
          Left = 7
          Top = 81
          Width = 48
          Height = 13
          Caption = 'BLF: early'
        end
        object lblImgConfirmed: TLabel
          Left = 7
          Top = 101
          Width = 71
          Height = 13
          Caption = 'BLF: confirmed'
        end
        object chbNoIcon: TCheckBox
          Left = 7
          Top = 18
          Width = 97
          Height = 17
          Caption = 'Do not show'
          TabOrder = 0
        end
        object edImgIdle: TEdit
          Left = 113
          Top = 38
          Width = 117
          Height = 21
          TabOrder = 1
        end
        object edImgTerminated: TEdit
          Left = 113
          Top = 58
          Width = 117
          Height = 21
          TabOrder = 2
        end
        object edImgEarly: TEdit
          Left = 113
          Top = 78
          Width = 117
          Height = 21
          TabOrder = 3
        end
        object edImgConfirmed: TEdit
          Left = 113
          Top = 98
          Width = 117
          Height = 21
          TabOrder = 4
        end
        object btnSelectImgTerminated: TButton
          Left = 232
          Top = 59
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 5
          OnClick = SelectImgClick
        end
        object btnSelectImgEarly: TButton
          Left = 232
          Top = 79
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 6
          OnClick = SelectImgClick
        end
        object btnSelectImgConfirmed: TButton
          Left = 232
          Top = 99
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 7
          OnClick = SelectImgClick
        end
        object btnSelectImgIdle: TButton
          Left = 232
          Top = 38
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 8
          OnClick = SelectImgClick
        end
      end
      object cbBackgroundColor: TComboBox
        Left = 116
        Top = 37
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 4
        OnChange = cbBackgroundColorChange
      end
      object btnSelectBackgroundColor: TButton
        Left = 259
        Top = 38
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 5
        OnClick = SelectColorClick
      end
    end
    object tsBehavior: TTabSheet
      Caption = 'Behavior'
      ImageIndex = 1
      object pcBehavior: TPageControl
        Left = 0
        Top = 0
        Width = 385
        Height = 277
        ActivePage = tsBehaviorAutoAnswer
        Align = alClient
        Style = tsFlatButtons
        TabOrder = 0
        object tsBehaviorBlf: TTabSheet
          Caption = 'tsBehaviorBlf'
          object lblBlfInCallAction: TLabel
            Left = 3
            Top = 125
            Width = 199
            Height = 13
            Caption = 'BLF/Presence/SpeedDial action during call'
          end
          object lblBlfDtmfPrefixDuringCall: TLabel
            Left = 3
            Top = 149
            Width = 228
            Height = 13
            Caption = 'BLF/Presence/SpeedDial DTMF prefix during call'
          end
          object grboxBlfOverride: TGroupBox
            Left = 3
            Top = 0
            Width = 370
            Height = 108
            Caption = 'BLF: override dialed number depending on state'
            TabOrder = 0
            object chbBlfOverrideIdle: TCheckBox
              Left = 7
              Top = 18
              Width = 76
              Height = 17
              Caption = 'idle/invalid'
              TabOrder = 0
            end
            object edBlfOverrideIdle: TEdit
              Left = 113
              Top = 16
              Width = 117
              Height = 21
              TabOrder = 1
            end
            object chbBlfOverrideTerminated: TCheckBox
              Left = 7
              Top = 39
              Width = 76
              Height = 17
              Caption = 'terminated'
              TabOrder = 2
            end
            object edBlfOverrideTerminated: TEdit
              Left = 113
              Top = 37
              Width = 117
              Height = 21
              TabOrder = 3
            end
            object chbBlfOverrideEarly: TCheckBox
              Left = 7
              Top = 61
              Width = 76
              Height = 17
              Caption = 'early'
              TabOrder = 4
            end
            object edBlfOverrideEarly: TEdit
              Left = 113
              Top = 59
              Width = 117
              Height = 21
              TabOrder = 5
            end
            object chbBlfOverrideConfirmed: TCheckBox
              Left = 7
              Top = 83
              Width = 76
              Height = 17
              Caption = 'confirmed'
              TabOrder = 6
            end
            object edBlfOverrideConfirmed: TEdit
              Left = 113
              Top = 81
              Width = 117
              Height = 21
              TabOrder = 7
            end
          end
          object cbBlfActionDuringCall: TComboBox
            Left = 223
            Top = 122
            Width = 145
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            ItemIndex = 2
            TabOrder = 1
            Text = 'blind transfer'
            Items.Strings = (
              'none'
              'send digits (DTMF)'
              'blind transfer')
          end
          object edBlfDtmfPrefixDuringCall: TEdit
            Left = 251
            Top = 146
            Width = 117
            Height = 21
            TabOrder = 2
          end
        end
        object tsBehaviorPaging: TTabSheet
          Caption = 'tsBehaviorPaging'
          ImageIndex = 1
          object lblPagingTxWaveFile: TLabel
            Left = 3
            Top = 3
            Width = 145
            Height = 13
            Caption = 'Wave file (from app directory)'
          end
          object lblPagingTxCodec: TLabel
            Left = 3
            Top = 26
            Width = 30
            Height = 13
            Caption = 'Codec'
          end
          object lblPagingTxPtime: TLabel
            Left = 3
            Top = 49
            Width = 50
            Height = 13
            Caption = 'Ptime [ms]'
          end
          object edPagingTxWaveFile: TEdit
            Left = 162
            Top = 0
            Width = 191
            Height = 21
            TabOrder = 0
          end
          object btnSelectWave: TButton
            Left = 354
            Top = 0
            Width = 22
            Height = 21
            Caption = '...'
            TabOrder = 1
            OnClick = btnSelectWaveClick
          end
          object cbPagingTxCodec: TComboBox
            Left = 162
            Top = 23
            Width = 145
            Height = 21
            ItemHeight = 13
            TabOrder = 2
            Text = 'PCMA'
            Items.Strings = (
              'PCMA'
              'PCMU'
              'G722'
              'L16/44100')
          end
          object cbPagingTxPtime: TComboBox
            Left = 162
            Top = 46
            Width = 71
            Height = 21
            ItemHeight = 13
            ItemIndex = 1
            TabOrder = 3
            Text = '20'
            Items.Strings = (
              '40'
              '20'
              '10')
          end
        end
        object tsBehaviorScript: TTabSheet
          Caption = 'tsBehaviorScript'
          ImageIndex = 2
          object lblScriptFile: TLabel
            Left = 3
            Top = 3
            Width = 117
            Height = 13
            Caption = 'Script file (\script subdir)'
          end
          object edScriptFile: TEdit
            Left = 130
            Top = 0
            Width = 194
            Height = 21
            TabOrder = 0
          end
          object btnSelectScript: TButton
            Left = 330
            Top = 0
            Width = 22
            Height = 21
            Caption = '...'
            TabOrder = 1
            OnClick = btnSelectScriptClick
          end
          object btnSelectedScriptOnButtonEdit: TBitBtn
            Left = 355
            Top = 0
            Width = 22
            Height = 21
            TabOrder = 2
            OnClick = btnSelectedScriptOnButtonEditClick
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
        end
        object tsBehaviorExecute: TTabSheet
          Caption = 'tsBehaviorExecute'
          ImageIndex = 3
          object lblArg1: TLabel
            Left = 3
            Top = 3
            Width = 56
            Height = 13
            Caption = 'Argument 1'
          end
          object edArg1: TEdit
            Left = 124
            Top = 0
            Width = 252
            Height = 21
            TabOrder = 0
          end
        end
        object tsBehaviorSwitchAudioSource: TTabSheet
          Caption = 'tsBehaviorSwitchAudioSource'
          ImageIndex = 4
          object lblSoundInputModule: TLabel
            Left = 3
            Top = 3
            Width = 34
            Height = 13
            Caption = 'Module'
          end
          object lblSoundInputDevice: TLabel
            Left = 3
            Top = 28
            Width = 32
            Height = 13
            Caption = 'Device'
          end
          object cbSoundInputMod: TComboBox
            Left = 81
            Top = 0
            Width = 240
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 0
            OnChange = cbSoundInputModChange
          end
          object edSoundInputWave: TEdit
            Left = 81
            Top = 25
            Width = 240
            Height = 21
            TabOrder = 1
          end
          object btnSelectWaveFile: TButton
            Left = 328
            Top = 26
            Width = 21
            Height = 19
            Caption = '...'
            TabOrder = 2
            OnClick = btnSelectWaveFileClick
          end
          object cbSoundInputDev: TComboBox
            Left = 81
            Top = 25
            Width = 240
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 3
          end
        end
        object tsBehaviorSwitchAudioPlayer: TTabSheet
          Caption = 'tsBehaviorSwitchAudioPlayer'
          ImageIndex = 5
          object Label3: TLabel
            Left = 3
            Top = 3
            Width = 34
            Height = 13
            Caption = 'Module'
          end
          object lblSoundOutputDev: TLabel
            Left = 3
            Top = 28
            Width = 32
            Height = 13
            Caption = 'Device'
          end
          object cbSoundOutputMod: TComboBox
            Left = 81
            Top = 0
            Width = 240
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 0
            OnChange = cbSoundOutputModChange
          end
          object cbSoundOutputDev: TComboBox
            Left = 81
            Top = 25
            Width = 240
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 1
          end
        end
        object tsBehaviorAutoAnswer: TTabSheet
          Caption = 'tsBehaviorAutoAnswer'
          ImageIndex = 6
          ExplicitLeft = -12
          ExplicitTop = 29
          object lblAutoAnswerSipCode: TLabel
            Left = 3
            Top = 3
            Width = 42
            Height = 13
            Caption = 'SIP code'
          end
          object edAutoAnswerSipCode: TEdit
            Left = 64
            Top = 0
            Width = 65
            Height = 21
            TabOrder = 0
            Text = '200'
          end
        end
      end
    end
  end
  object edDescription2: TEdit
    Left = 283
    Top = 110
    Width = 110
    Height = 21
    TabOrder = 6
  end
  object cbCaptionLines: TComboBox
    Left = 120
    Top = 110
    Width = 41
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 7
    Text = '1'
    Items.Strings = (
      '1'
      '2')
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 96
    Top = 408
  end
  object colorDialog: TColorDialog
    Left = 128
    Top = 408
  end
  object dlgOpenRing: TOpenDialog
    Filter = 'WAVE files (*.wav)|*.wav'
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 352
    Top = 368
  end
end

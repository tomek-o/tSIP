object frmButtonEdit: TfrmButtonEdit
  Left = 0
  Top = 0
  BorderStyle = bsSingle
  Caption = 'Edit button'
  ClientHeight = 531
  ClientWidth = 675
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
  object lblNumber: TLabel
    Left = 469
    Top = 57
    Width = 37
    Height = 13
    Caption = 'Number'
  end
  object lblType: TLabel
    Left = 8
    Top = 7
    Width = 24
    Height = 13
    Caption = 'Type'
  end
  object lblCaption: TLabel
    Left = 126
    Top = 57
    Width = 37
    Height = 13
    Caption = 'Caption'
  end
  object lblCaption2: TLabel
    Left = 270
    Top = 57
    Width = 54
    Height = 13
    Caption = 'Caption #2'
  end
  object lblParent: TLabel
    Left = 9
    Top = 31
    Width = 80
    Height = 13
    Caption = 'Parent container'
  end
  object pnlBottom: TPanel
    Left = 0
    Top = 494
    Width = 675
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btnCancel: TButton
      Left = 592
      Top = 7
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 511
      Top = 7
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 0
      OnClick = btnApplyClick
    end
    object btnClear: TButton
      Left = 8
      Top = 6
      Width = 137
      Height = 25
      Caption = 'Reset button settings'
      TabOrder = 2
      OnClick = btnClearClick
    end
  end
  object edNumber: TEdit
    Left = 514
    Top = 54
    Width = 153
    Height = 21
    TabOrder = 1
  end
  object cbType: TComboBox
    Left = 39
    Top = 4
    Width = 225
    Height = 21
    Style = csDropDownList
    DropDownCount = 12
    ItemHeight = 13
    TabOrder = 2
    OnChange = cbTypeChange
  end
  object memoHelp: TMemo
    Left = 264
    Top = 8
    Width = 403
    Height = 42
    TabStop = False
    BorderStyle = bsNone
    Color = clBtnFace
    ReadOnly = True
    TabOrder = 3
  end
  object pageControl: TPageControl
    Left = 8
    Top = 81
    Width = 659
    Height = 408
    ActivePage = tsVisual
    TabOrder = 4
    object tsVisual: TTabSheet
      Caption = 'Visual'
      ImageIndex = 1
      object lblBevelWidth: TLabel
        Left = 407
        Top = 8
        Width = 55
        Height = 13
        Caption = 'Bevel width'
      end
      object lblHeight: TLabel
        Left = 277
        Top = 33
        Width = 31
        Height = 13
        Caption = 'Height'
      end
      object lblWidth: TLabel
        Left = 173
        Top = 33
        Width = 28
        Height = 13
        Caption = 'Width'
      end
      object lblLeft: TLabel
        Left = 3
        Top = 33
        Width = 19
        Height = 13
        Caption = 'Left'
      end
      object lblTop: TLabel
        Left = 89
        Top = 33
        Width = 18
        Height = 13
        Caption = 'Top'
      end
      object chbVisible: TCheckBox
        Left = 3
        Top = 7
        Width = 110
        Height = 17
        Caption = 'Visible (show)'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 0
      end
      object chbDown: TCheckBox
        Left = 132
        Top = 7
        Width = 60
        Height = 17
        Caption = 'Down'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
      end
      object chbInactive: TCheckBox
        Left = 198
        Top = 7
        Width = 71
        Height = 17
        Caption = 'Inactive'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
      end
      object chbCustomFrame: TCheckBox
        Left = 275
        Top = 7
        Width = 95
        Height = 17
        Caption = 'custom frame'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
      end
      object edBevelWidth: TEdit
        Left = 468
        Top = 5
        Width = 38
        Height = 21
        TabOrder = 4
      end
      object edHeight: TEdit
        Left = 318
        Top = 30
        Width = 38
        Height = 21
        TabOrder = 5
      end
      object edWidth: TEdit
        Left = 217
        Top = 30
        Width = 38
        Height = 21
        TabOrder = 6
      end
      object edTop: TEdit
        Left = 113
        Top = 30
        Width = 38
        Height = 21
        TabOrder = 7
      end
      object edLeft: TEdit
        Left = 28
        Top = 30
        Width = 38
        Height = 21
        TabOrder = 8
      end
      object grboxCaption1: TGroupBox
        Left = 0
        Top = 81
        Width = 321
        Height = 72
        Caption = 'Caption #1'
        TabOrder = 9
        object lblSpeedDialFont: TLabel
          Left = 10
          Top = 17
          Width = 22
          Height = 13
          Caption = 'Font'
        end
        object lblLabelTop: TLabel
          Left = 159
          Top = 39
          Width = 18
          Height = 13
          Caption = 'Top'
        end
        object lblLabelLeft: TLabel
          Left = 159
          Top = 17
          Width = 19
          Height = 13
          Caption = 'Left'
        end
        object edSpeedDialFont: TEdit
          Left = 38
          Top = 13
          Width = 75
          Height = 52
          AutoSize = False
          TabOrder = 0
          Text = 'abcABC 123 () [] <>,.'
        end
        object btnSpeedDialFontSelect: TButton
          Left = 119
          Top = 14
          Width = 26
          Height = 22
          Caption = '...'
          TabOrder = 1
          OnClick = btnSpeedDialFontSelectClick
        end
        object chbLabelCenterHorizontally: TCheckBox
          Left = 245
          Top = 15
          Width = 71
          Height = 17
          Caption = 'centered'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 2
        end
        object edLabelTop: TEdit
          Left = 192
          Top = 36
          Width = 38
          Height = 21
          TabOrder = 3
        end
        object chbLabelCenterVertically: TCheckBox
          Left = 245
          Top = 38
          Width = 73
          Height = 17
          Caption = 'centered'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = chbLabelCenterVerticallyClick
        end
        object edLabelLeft: TEdit
          Left = 192
          Top = 14
          Width = 38
          Height = 21
          TabOrder = 5
        end
      end
      object grboxCaption2: TGroupBox
        Left = 327
        Top = 81
        Width = 321
        Height = 72
        Caption = 'Caption #2'
        TabOrder = 10
        object Label1: TLabel
          Left = 10
          Top = 17
          Width = 22
          Height = 13
          Caption = 'Font'
        end
        object lblLabel2Top: TLabel
          Left = 159
          Top = 40
          Width = 18
          Height = 13
          Caption = 'Top'
        end
        object Label6: TLabel
          Left = 159
          Top = 17
          Width = 19
          Height = 13
          Caption = 'Left'
        end
        object edCaption2Font: TEdit
          Left = 38
          Top = 13
          Width = 75
          Height = 52
          AutoSize = False
          TabOrder = 0
          Text = 'abcABC 123 () [] <>,.'
        end
        object btnCaption2FontSelect: TButton
          Left = 119
          Top = 14
          Width = 26
          Height = 22
          Caption = '...'
          TabOrder = 1
          OnClick = btnSpeedDialFontSelectClick
        end
        object edLabel2Top: TEdit
          Left = 192
          Top = 37
          Width = 38
          Height = 21
          TabOrder = 2
        end
        object edLabel2Left: TEdit
          Left = 192
          Top = 14
          Width = 38
          Height = 21
          TabOrder = 3
        end
        object chbLabel2CenterHorizontally: TCheckBox
          Left = 240
          Top = 16
          Width = 68
          Height = 17
          Caption = 'centered'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 4
          OnClick = chbLabel2CenterHorizontallyClick
        end
      end
      object grboxColors: TGroupBox
        Left = 0
        Top = 162
        Width = 648
        Height = 95
        Caption = 'Colors'
        TabOrder = 11
        object lblInactive: TLabel
          Left = 10
          Top = 45
          Width = 37
          Height = 13
          Caption = 'inactive'
        end
        object lblColorIdle: TLabel
          Left = 242
          Top = 18
          Width = 16
          Height = 13
          Caption = 'idle'
        end
        object lblColorDown: TLabel
          Left = 10
          Top = 69
          Width = 26
          Height = 13
          Caption = 'down'
        end
        object lblColorDownPressed: TLabel
          Left = 242
          Top = 69
          Width = 71
          Height = 13
          Caption = 'down, pressed'
        end
        object lblColorInactiveDown: TLabel
          Left = 242
          Top = 45
          Width = 70
          Height = 13
          Caption = 'inactive, down'
        end
        object cbInactiveColor: TComboBox
          Left = 53
          Top = 42
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          ParentShowHint = False
          ShowHint = True
          TabOrder = 0
          OnChange = cbColorChange
        end
        object btnSelectInactiveColor: TButton
          Left = 196
          Top = 43
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 1
          OnClick = SelectColorClick
        end
        object cbIdleColor: TComboBox
          Left = 317
          Top = 15
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 2
          OnChange = cbColorChange
        end
        object btnSelectIdleColor: TButton
          Left = 460
          Top = 16
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 3
          OnClick = SelectColorClick
        end
        object cbDownColor: TComboBox
          Left = 53
          Top = 66
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 4
          OnChange = cbColorChange
        end
        object btnSelectDownColor: TButton
          Left = 196
          Top = 67
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 5
          OnClick = SelectColorClick
        end
        object cbDownPressedColor: TComboBox
          Left = 317
          Top = 66
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 6
          OnChange = cbColorChange
        end
        object btnSelectDownPressedColor: TButton
          Left = 460
          Top = 67
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 7
          OnClick = SelectColorClick
        end
        object cbColorElement: TComboBox
          Left = 8
          Top = 15
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          ParentShowHint = False
          ShowHint = True
          TabOrder = 8
          OnChange = cbColorElementChange
          Items.Strings = (
            'Background'
            'Font'
            'Custom frame')
        end
        object cbInactiveDownColor: TComboBox
          Left = 317
          Top = 42
          Width = 140
          Height = 21
          Style = csDropDownList
          ItemHeight = 13
          TabOrder = 9
          OnChange = cbColorChange
        end
        object btnSelectInactiveDownColor: TButton
          Left = 460
          Top = 43
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 10
          OnClick = SelectColorClick
        end
      end
      object grboxImage: TGroupBox
        Left = -1
        Top = 268
        Width = 649
        Height = 109
        Caption = 'Bitmaps (from "img" subdirectory)'
        TabOrder = 12
        object lblImgIdle: TLabel
          Left = 8
          Top = 20
          Width = 95
          Height = 13
          Caption = 'Idle/invalid/non-BLF'
        end
        object lblImgTerminated: TLabel
          Left = 8
          Top = 40
          Width = 76
          Height = 13
          Caption = 'BLF: terminated'
        end
        object lblImgEarly: TLabel
          Left = 8
          Top = 60
          Width = 48
          Height = 13
          Caption = 'BLF: early'
        end
        object lblImgConfirmed: TLabel
          Left = 8
          Top = 80
          Width = 71
          Height = 13
          Caption = 'BLF: confirmed'
        end
        object lblImageLeft: TLabel
          Left = 297
          Top = 20
          Width = 54
          Height = 13
          Caption = 'image: Left'
        end
        object lblImageTop: TLabel
          Left = 297
          Top = 41
          Width = 53
          Height = 13
          Caption = 'image: Top'
        end
        object edImgIdle: TEdit
          Left = 114
          Top = 17
          Width = 117
          Height = 21
          TabOrder = 0
        end
        object edImgTerminated: TEdit
          Left = 114
          Top = 37
          Width = 117
          Height = 21
          TabOrder = 1
        end
        object edImgEarly: TEdit
          Left = 114
          Top = 57
          Width = 117
          Height = 21
          TabOrder = 2
        end
        object edImgConfirmed: TEdit
          Left = 114
          Top = 77
          Width = 117
          Height = 21
          TabOrder = 3
        end
        object btnSelectImgTerminated: TButton
          Left = 233
          Top = 38
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 4
          OnClick = SelectImgClick
        end
        object btnSelectImgEarly: TButton
          Left = 233
          Top = 58
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 5
          OnClick = SelectImgClick
        end
        object btnSelectImgConfirmed: TButton
          Left = 233
          Top = 78
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 6
          OnClick = SelectImgClick
        end
        object btnSelectImgIdle: TButton
          Left = 233
          Top = 17
          Width = 21
          Height = 19
          Caption = '...'
          TabOrder = 7
          OnClick = SelectImgClick
        end
        object edImageLeft: TEdit
          Left = 366
          Top = 14
          Width = 38
          Height = 21
          TabOrder = 8
        end
        object chbImageTransparent: TCheckBox
          Left = 297
          Top = 81
          Width = 154
          Height = 17
          Caption = 'transparent bitmap image'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 9
        end
        object edImageTop: TEdit
          Left = 366
          Top = 38
          Width = 38
          Height = 21
          TabOrder = 10
        end
        object chbImageCenterVertically: TCheckBox
          Left = 413
          Top = 40
          Width = 60
          Height = 17
          Caption = 'centered'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'Tahoma'
          Font.Style = []
          ParentFont = False
          TabOrder = 11
          OnClick = chbImageCenterVerticallyClick
        end
      end
      object chbSpaceLabelsYEqually: TCheckBox
        Left = 3
        Top = 58
        Width = 632
        Height = 17
        Caption = 
          'space caption lines equally in Y axis (automatically for either ' +
          '1 or 2 caption lines)'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 13
        OnClick = chbSpaceLabelsYEquallyClick
      end
    end
    object tsBehavior: TTabSheet
      Caption = 'Behavior'
      ImageIndex = 1
      object pcBehavior: TPageControl
        Left = 0
        Top = 0
        Width = 651
        Height = 380
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
          object lblBlfExpires: TLabel
            Left = 3
            Top = 181
            Width = 51
            Height = 13
            Caption = 'Expires [s]'
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
          object edBlfExpires: TEdit
            Left = 116
            Top = 178
            Width = 45
            Height = 21
            TabOrder = 3
            Text = '600'
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
          object lblAutoAnswerSipCode: TLabel
            Left = 3
            Top = 3
            Width = 42
            Height = 13
            Caption = 'SIP code'
          end
          object lblAutoAnswerSipReason: TLabel
            Left = 3
            Top = 30
            Width = 52
            Height = 13
            Caption = 'SIP reason'
          end
          object lblAutoAnswerSipReasonInfo: TLabel
            Left = 283
            Top = 30
            Width = 152
            Height = 13
            Caption = '(e.g. "Busy Here" for 486 code)'
          end
          object edAutoAnswerSipCode: TEdit
            Left = 64
            Top = 0
            Width = 65
            Height = 21
            TabOrder = 0
            Text = '200'
          end
          object edAutoAnswerSipReason: TEdit
            Left = 64
            Top = 27
            Width = 201
            Height = 21
            TabOrder = 1
            Text = 'OK'
          end
        end
      end
    end
  end
  object cbCaptionLines: TComboBox
    Left = 8
    Top = 54
    Width = 100
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 5
    Text = '1 caption line'
    Items.Strings = (
      '1 caption line'
      '2 caption lines')
  end
  object edCaption: TEdit
    Left = 169
    Top = 54
    Width = 89
    Height = 21
    TabOrder = 6
  end
  object edCaption2: TEdit
    Left = 328
    Top = 54
    Width = 89
    Height = 21
    TabOrder = 7
  end
  object cbParentId: TComboBox
    Left = 95
    Top = 28
    Width = 169
    Height = 21
    Style = csDropDownList
    DropDownCount = 12
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 8
    Text = 'main / console'
    Items.Strings = (
      'dialpad'
      'main / console')
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
    Left = 192
    Top = 408
  end
  object fontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [fdForceFontExist]
    Left = 160
    Top = 408
  end
end

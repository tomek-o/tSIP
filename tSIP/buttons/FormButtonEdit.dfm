object frmButtonEdit: TfrmButtonEdit
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'Edit button'
  ClientHeight = 437
  ClientWidth = 657
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
  object pnlBottom: TPanel
    Left = 0
    Top = 400
    Width = 657
    Height = 37
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 3
    object btnCancel: TButton
      Left = 574
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 2
      OnClick = btnCancelClick
    end
    object btnApply: TButton
      Left = 482
      Top = 6
      Width = 75
      Height = 25
      Caption = 'Apply'
      TabOrder = 1
      OnClick = btnApplyClick
    end
    object btnClear: TButton
      Left = 8
      Top = 6
      Width = 137
      Height = 25
      Caption = 'Reset button settings'
      TabOrder = 0
      OnClick = btnClearClick
    end
  end
  object pageControl: TPageControl
    Left = 128
    Top = 78
    Width = 529
    Height = 322
    ActivePage = tsGeneral
    Align = alClient
    TabHeight = 10
    TabOrder = 2
    object tsGeneral: TTabSheet
      Caption = 'General'
      ImageIndex = 2
      object lblLeft: TLabel
        Left = 5
        Top = 10
        Width = 19
        Height = 13
        Caption = 'Left'
      end
      object lblTop: TLabel
        Left = 91
        Top = 10
        Width = 18
        Height = 13
        Caption = 'Top'
      end
      object lblWidth: TLabel
        Left = 175
        Top = 10
        Width = 28
        Height = 13
        Caption = 'Width'
      end
      object lblHeight: TLabel
        Left = 279
        Top = 10
        Width = 31
        Height = 13
        Caption = 'Height'
      end
      object lblBevelWidth: TLabel
        Left = 3
        Top = 241
        Width = 55
        Height = 13
        Caption = 'Bevel width'
      end
      object cbCaptionLines: TComboBox
        Left = 5
        Top = 49
        Width = 100
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 0
        TabOrder = 4
        Text = '1 caption line'
        Items.Strings = (
          '1 caption line'
          '2 caption lines')
      end
      object edLeft: TEdit
        Left = 30
        Top = 7
        Width = 38
        Height = 21
        TabOrder = 0
      end
      object edTop: TEdit
        Left = 115
        Top = 7
        Width = 38
        Height = 21
        TabOrder = 1
      end
      object chbDown: TCheckBox
        Left = 3
        Top = 265
        Width = 60
        Height = 17
        Caption = 'Down'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 9
      end
      object chbInactive: TCheckBox
        Left = 69
        Top = 265
        Width = 71
        Height = 17
        Caption = 'Inactive'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 10
      end
      object edWidth: TEdit
        Left = 219
        Top = 7
        Width = 38
        Height = 21
        TabOrder = 2
      end
      object chbCustomFrame: TCheckBox
        Left = 146
        Top = 265
        Width = 95
        Height = 17
        Caption = 'custom frame'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 11
      end
      object edHeight: TEdit
        Left = 320
        Top = 7
        Width = 38
        Height = 21
        TabOrder = 3
      end
      object edBevelWidth: TEdit
        Left = 64
        Top = 238
        Width = 38
        Height = 21
        TabOrder = 8
      end
      object grboxCaption1: TGroupBox
        Left = 3
        Top = 76
        Width = 510
        Height = 72
        Caption = 'Caption #1'
        TabOrder = 6
        object lblSpeedDialFont: TLabel
          Left = 196
          Top = 16
          Width = 22
          Height = 13
          Caption = 'Font'
        end
        object lblLabelTop: TLabel
          Left = 345
          Top = 39
          Width = 18
          Height = 13
          Caption = 'Top'
        end
        object lblLabelLeft: TLabel
          Left = 345
          Top = 17
          Width = 19
          Height = 13
          Caption = 'Left'
        end
        object lblCaption: TLabel
          Left = 7
          Top = 17
          Width = 22
          Height = 13
          Caption = 'Text'
        end
        object edSpeedDialFont: TEdit
          Left = 224
          Top = 13
          Width = 75
          Height = 52
          AutoSize = False
          TabOrder = 1
          Text = 'abcABC 123 () [] <>,.'
        end
        object btnSpeedDialFontSelect: TButton
          Left = 305
          Top = 14
          Width = 26
          Height = 22
          Caption = '...'
          TabOrder = 2
          OnClick = btnSpeedDialFontSelectClick
        end
        object chbLabelCenterHorizontally: TCheckBox
          Left = 431
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
          TabOrder = 4
        end
        object edLabelTop: TEdit
          Left = 378
          Top = 36
          Width = 38
          Height = 21
          TabOrder = 5
        end
        object chbLabelCenterVertically: TCheckBox
          Left = 431
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
          TabOrder = 6
          OnClick = chbLabelCenterVerticallyClick
        end
        object edLabelLeft: TEdit
          Left = 378
          Top = 14
          Width = 38
          Height = 21
          TabOrder = 3
        end
        object memoCaption1: TMemo
          Left = 35
          Top = 14
          Width = 142
          Height = 51
          TabOrder = 0
        end
      end
      object grboxCaption2: TGroupBox
        Left = 3
        Top = 150
        Width = 510
        Height = 72
        Caption = 'Caption #2'
        TabOrder = 7
        object Label1: TLabel
          Left = 194
          Top = 16
          Width = 22
          Height = 13
          Caption = 'Font'
        end
        object lblLabel2Top: TLabel
          Left = 343
          Top = 40
          Width = 18
          Height = 13
          Caption = 'Top'
        end
        object Label6: TLabel
          Left = 343
          Top = 17
          Width = 19
          Height = 13
          Caption = 'Left'
        end
        object lblCaption2: TLabel
          Left = 7
          Top = 17
          Width = 22
          Height = 13
          Caption = 'Text'
        end
        object edCaption2Font: TEdit
          Left = 222
          Top = 13
          Width = 75
          Height = 52
          AutoSize = False
          TabOrder = 1
          Text = 'abcABC 123 () [] <>,.'
        end
        object btnCaption2FontSelect: TButton
          Left = 303
          Top = 14
          Width = 26
          Height = 22
          Caption = '...'
          TabOrder = 2
          OnClick = btnSpeedDialFontSelectClick
        end
        object edLabel2Top: TEdit
          Left = 376
          Top = 37
          Width = 38
          Height = 21
          TabOrder = 5
        end
        object edLabel2Left: TEdit
          Left = 376
          Top = 14
          Width = 38
          Height = 21
          TabOrder = 3
        end
        object chbLabel2CenterHorizontally: TCheckBox
          Left = 431
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
        object memoCaption2: TMemo
          Left = 35
          Top = 14
          Width = 140
          Height = 51
          TabOrder = 0
        end
      end
      object chbSpaceLabelsYEqually: TCheckBox
        Left = 111
        Top = 51
        Width = 407
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
        TabOrder = 5
        OnClick = chbSpaceLabelsYEquallyClick
      end
    end
    object tsBehavior: TTabSheet
      Caption = 'Behavior'
      ImageIndex = 1
      object pcBehavior: TPageControl
        Left = 0
        Top = 0
        Width = 521
        Height = 302
        ActivePage = tsBehaviorBlf
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
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 0
            OnChange = cbSoundInputModChange
          end
          object edSoundInputWave: TEdit
            Left = 81
            Top = 25
            Width = 320
            Height = 21
            TabOrder = 1
          end
          object btnSelectWaveFile: TButton
            Left = 407
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
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 3
          end
        end
        object tsBehaviorSwitchAudioPlayer: TTabSheet
          Caption = 'tsBehaviorSwitchAudioPlayer'
          ImageIndex = 5
          object lblSoundOutputMod: TLabel
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
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 0
            OnChange = cbSoundOutputModChange
          end
          object cbSoundOutputDev: TComboBox
            Left = 81
            Top = 25
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 1
          end
        end
        object tsBehaviorSwitchVideoSource: TTabSheet
          Caption = 'tsBehaviorSwitchVideoSource'
          ImageIndex = 7
          object lblVideoInputModule: TLabel
            Left = 3
            Top = 3
            Width = 34
            Height = 13
            Caption = 'Module'
          end
          object lblVideoInputDevice: TLabel
            Left = 3
            Top = 27
            Width = 32
            Height = 13
            Caption = 'Device'
          end
          object lblNoVideo: TLabel
            Left = 3
            Top = 88
            Width = 594
            Height = 13
            Caption = 
              'This build does not support video, video settings are left only ' +
              'to avoid issues when swapping executables.'
            Font.Charset = DEFAULT_CHARSET
            Font.Color = clWindowText
            Font.Height = -11
            Font.Name = 'Tahoma'
            Font.Style = [fsBold]
            ParentFont = False
            Visible = False
          end
          object edVideoInputFile: TEdit
            Left = 148
            Top = 24
            Width = 320
            Height = 21
            TabOrder = 0
            Visible = False
          end
          object cbVideoInputMod: TComboBox
            Left = 148
            Top = 0
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 1
            OnChange = cbVideoInputModChange
          end
          object btnSelectVideoInputFile: TButton
            Left = 474
            Top = 25
            Width = 21
            Height = 19
            Caption = '...'
            TabOrder = 2
            Visible = False
          end
          object cbVideoInputDev: TComboBox
            Left = 148
            Top = 24
            Width = 320
            Height = 21
            Style = csDropDownList
            ItemHeight = 13
            TabOrder = 3
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
    object tsColors: TTabSheet
      Caption = 'Colors'
      ImageIndex = 1
      object lblInactive: TLabel
        Left = 10
        Top = 45
        Width = 37
        Height = 13
        Caption = 'inactive'
      end
      object lblColorDown: TLabel
        Left = 10
        Top = 69
        Width = 26
        Height = 13
        Caption = 'down'
      end
      object lblElements: TLabel
        Left = 10
        Top = 6
        Width = 38
        Height = 13
        Caption = 'Element'
      end
      object lblColorIdle: TLabel
        Left = 10
        Top = 95
        Width = 16
        Height = 13
        Caption = 'idle'
      end
      object lblColorInactiveDown: TLabel
        Left = 10
        Top = 120
        Width = 70
        Height = 13
        Caption = 'inactive, down'
      end
      object lblColorDownPressed: TLabel
        Left = 10
        Top = 145
        Width = 71
        Height = 13
        Caption = 'down, pressed'
      end
      object shColorInactive: TShape
        Left = 280
        Top = 42
        Width = 41
        Height = 21
      end
      object shColorDown: TShape
        Left = 280
        Top = 67
        Width = 41
        Height = 21
      end
      object shColorIdle: TShape
        Left = 280
        Top = 92
        Width = 41
        Height = 21
      end
      object shColorInactiveDown: TShape
        Left = 280
        Top = 117
        Width = 41
        Height = 21
      end
      object shColorDownPressed: TShape
        Left = 280
        Top = 142
        Width = 41
        Height = 21
      end
      object lblColorPreview: TLabel
        Left = 280
        Top = 23
        Width = 38
        Height = 13
        Caption = 'preview'
      end
      object cbColorElement: TComboBox
        Left = 94
        Top = 3
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ParentShowHint = False
        ShowHint = True
        TabOrder = 0
        OnChange = cbColorElementChange
        Items.Strings = (
          'Background'
          'Font'
          'Custom frame')
      end
      object cbInactiveColor: TComboBox
        Left = 94
        Top = 42
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        OnChange = cbColorChange
      end
      object btnSelectInactiveColor: TButton
        Left = 239
        Top = 42
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 2
        OnClick = SelectColorClick
      end
      object cbDownColor: TComboBox
        Left = 94
        Top = 66
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 3
        OnChange = cbColorChange
      end
      object btnSelectDownColor: TButton
        Left = 239
        Top = 66
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 4
        OnClick = SelectColorClick
      end
      object cbDownPressedColor: TComboBox
        Left = 94
        Top = 142
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 9
        OnChange = cbColorChange
      end
      object cbInactiveDownColor: TComboBox
        Left = 94
        Top = 117
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 7
        OnChange = cbColorChange
      end
      object cbIdleColor: TComboBox
        Left = 94
        Top = 92
        Width = 140
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 5
        OnChange = cbColorChange
      end
      object btnSelectIdleColor: TButton
        Left = 239
        Top = 92
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 6
        OnClick = SelectColorClick
      end
      object btnSelectInactiveDownColor: TButton
        Left = 239
        Top = 117
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 8
        OnClick = SelectColorClick
      end
      object btnSelectDownPressedColor: TButton
        Left = 239
        Top = 142
        Width = 21
        Height = 21
        Caption = '...'
        TabOrder = 10
        OnClick = SelectColorClick
      end
    end
    object tsBitmaps: TTabSheet
      Caption = 'Bitmaps'
      ImageIndex = 3
      object Label2: TLabel
        Left = 2
        Top = 100
        Width = 213
        Height = 13
        Caption = 'Bitmaps must be placed in "img" subdirectory'
      end
      object lblImgIdle: TLabel
        Left = 3
        Top = 146
        Width = 95
        Height = 13
        Caption = 'Idle/invalid/non-BLF'
      end
      object lblImgTerminated: TLabel
        Left = 3
        Top = 176
        Width = 76
        Height = 13
        Caption = 'BLF: terminated'
      end
      object lblImgEarly: TLabel
        Left = 3
        Top = 206
        Width = 48
        Height = 13
        Caption = 'BLF: early'
      end
      object lblImgConfirmed: TLabel
        Left = 3
        Top = 238
        Width = 71
        Height = 13
        Caption = 'BLF: confirmed'
      end
      object lblImageLeft: TLabel
        Left = 3
        Top = 10
        Width = 54
        Height = 13
        Caption = 'image: Left'
      end
      object lblImageTop: TLabel
        Left = 3
        Top = 33
        Width = 53
        Height = 13
        Caption = 'image: Top'
      end
      object imgIdle: TImage
        Left = 272
        Top = 137
        Width = 57
        Height = 31
        Center = True
      end
      object imgTerminated: TImage
        Left = 272
        Top = 168
        Width = 57
        Height = 31
        Center = True
      end
      object imgEarly: TImage
        Left = 272
        Top = 199
        Width = 57
        Height = 31
        Center = True
      end
      object imgConfirmed: TImage
        Left = 272
        Top = 230
        Width = 57
        Height = 31
        Center = True
      end
      object Label3: TLabel
        Left = 280
        Top = 118
        Width = 38
        Height = 13
        Caption = 'preview'
      end
      object edImgConfirmed: TEdit
        Left = 109
        Top = 235
        Width = 117
        Height = 21
        TabOrder = 10
      end
      object edImgEarly: TEdit
        Left = 109
        Top = 203
        Width = 117
        Height = 21
        TabOrder = 8
      end
      object edImgTerminated: TEdit
        Left = 109
        Top = 173
        Width = 117
        Height = 21
        TabOrder = 6
      end
      object edImgIdle: TEdit
        Left = 109
        Top = 143
        Width = 117
        Height = 21
        TabOrder = 4
      end
      object btnSelectImgIdle: TButton
        Left = 229
        Top = 143
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 5
        OnClick = SelectImgClick
      end
      object btnSelectImgTerminated: TButton
        Left = 229
        Top = 174
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 7
        OnClick = SelectImgClick
      end
      object btnSelectImgEarly: TButton
        Left = 229
        Top = 204
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 9
        OnClick = SelectImgClick
      end
      object btnSelectImgConfirmed: TButton
        Left = 229
        Top = 236
        Width = 21
        Height = 19
        Caption = '...'
        TabOrder = 11
        OnClick = SelectImgClick
      end
      object edImageLeft: TEdit
        Left = 72
        Top = 7
        Width = 38
        Height = 21
        TabOrder = 0
      end
      object edImageTop: TEdit
        Left = 72
        Top = 30
        Width = 38
        Height = 21
        TabOrder = 1
      end
      object chbImageCenterVertically: TCheckBox
        Left = 119
        Top = 32
        Width = 60
        Height = 17
        Caption = 'centered'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = chbImageCenterVerticallyClick
      end
      object chbImageTransparent: TCheckBox
        Left = 3
        Top = 67
        Width = 154
        Height = 17
        Caption = 'make bitmap transparent'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Tahoma'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
      end
    end
  end
  object tvSelector: TTreeView
    Left = 0
    Top = 78
    Width = 128
    Height = 322
    Align = alLeft
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Tahoma'
    Font.Style = []
    HideSelection = False
    Indent = 19
    ParentFont = False
    ReadOnly = True
    RowSelect = True
    TabOrder = 1
    OnChange = tvSelectorChange
  end
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 657
    Height = 78
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object lblType: TLabel
      Left = 5
      Top = 8
      Width = 24
      Height = 13
      Caption = 'Type'
    end
    object lblNumber: TLabel
      Left = 6
      Top = 56
      Width = 65
      Height = 13
      Caption = 'Number / URI'
    end
    object cbType: TComboBox
      Left = 36
      Top = 5
      Width = 225
      Height = 21
      Style = csDropDownList
      DropDownCount = 12
      ItemHeight = 13
      TabOrder = 0
      OnChange = cbTypeChange
    end
    object memoHelp: TMemo
      Left = 267
      Top = 5
      Width = 383
      Height = 69
      TabStop = False
      BorderStyle = bsNone
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 4
    end
    object chbVisible: TCheckBox
      Left = 6
      Top = 31
      Width = 115
      Height = 17
      Caption = 'Visible (show) on'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 1
    end
    object cbParentId: TComboBox
      Left = 122
      Top = 29
      Width = 139
      Height = 21
      Style = csDropDownList
      DropDownCount = 12
      ItemHeight = 13
      ItemIndex = 1
      TabOrder = 2
      Text = 'main area / console'
      Items.Strings = (
        'dialpad'
        'main area / console')
    end
    object edNumber: TEdit
      Left = 122
      Top = 53
      Width = 139
      Height = 21
      TabOrder = 3
    end
  end
  object openDialog: TOpenDialog
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 152
  end
  object colorDialog: TColorDialog
    Left = 40
    Top = 152
  end
  object dlgOpenRing: TOpenDialog
    Filter = 'WAVE files (*.wav)|*.wav'
    Options = [ofHideReadOnly, ofNoChangeDir, ofFileMustExist, ofEnableSizing]
    Left = 40
    Top = 184
  end
  object fontDialog: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    Options = [fdForceFontExist]
    Left = 8
    Top = 184
  end
  object dlgOpenDeviceFile: TOpenDialog
    Options = [ofEnableSizing]
    Left = 8
    Top = 216
  end
end

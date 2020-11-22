object FormOption: TFormOption
  Left = 402
  Top = 230
  BorderStyle = bsDialog
  Caption = #36873#39033#21450#35774#32622
  ClientHeight = 252
  ClientWidth = 348
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object LabelBoxName: TLabel
    Left = 12
    Top = 15
    Width = 58
    Height = 13
    Caption = '      '#25991#20214#21517':'
  end
  object ButtonBrowse: TButton
    Left = 258
    Top = 10
    Width = 75
    Height = 25
    Caption = #27983#35272'...'
    TabOrder = 0
    OnClick = ButtonBrowseClick
  end
  object EditBoxName: TEdit
    Left = 77
    Top = 12
    Width = 180
    Height = 21
    TabOrder = 1
  end
  object GroupBoxChpass: TGroupBox
    Left = 12
    Top = 39
    Width = 321
    Height = 95
    Caption = #26356#25913#23494#30721
    TabOrder = 2
    object LabelOldPass: TLabel
      Left = 102
      Top = 19
      Width = 67
      Height = 13
      Caption = '     '#24403#21069#23494#30721':'
    end
    object LabelNewPass1: TLabel
      Left = 97
      Top = 46
      Width = 70
      Height = 13
      Caption = '          '#26032#23494#30721':'
    end
    object LabelNewPass2: TLabel
      Left = 97
      Top = 67
      Width = 70
      Height = 13
      Caption = '  '#37325#22797#26032#23494#30721':'
    end
    object EditOldPass: TEdit
      Left = 176
      Top = 16
      Width = 121
      Height = 21
      PasswordChar = '*'
      TabOrder = 0
    end
    object EditPass1: TEdit
      Left = 176
      Top = 43
      Width = 121
      Height = 21
      PasswordChar = '*'
      TabOrder = 1
    end
    object EditPass2: TEdit
      Left = 176
      Top = 64
      Width = 121
      Height = 21
      PasswordChar = '*'
      TabOrder = 2
    end
    object ButtonChpass: TButton
      Left = 11
      Top = 41
      Width = 75
      Height = 25
      Caption = #26356#25913
      TabOrder = 3
      OnClick = ButtonChpassClick
    end
  end
  object ButtonClose: TButton
    Left = 177
    Top = 211
    Width = 75
    Height = 25
    Cancel = True
    Caption = #20851#38381
    TabOrder = 3
    OnClick = ButtonCloseClick
  end
  object GroupBox1: TGroupBox
    Left = 12
    Top = 142
    Width = 321
    Height = 53
    TabOrder = 4
    object LabelAutoMount: TLabel
      Left = 3
      Top = 21
      Width = 58
      Height = 13
      Caption = '      '#25991#20214#21517':'
    end
    object CheckBoxAutoMount: TCheckBox
      Left = 8
      Top = -1
      Width = 78
      Height = 17
      Caption = #33258#21160#21152#36733
      TabOrder = 0
      OnClick = CheckBoxAutoMountClick
    end
    object ButtonBrowseAuto: TButton
      Left = 240
      Top = 16
      Width = 75
      Height = 25
      Caption = #27983#35272'...'
      TabOrder = 1
      OnClick = ButtonBrowseClick
    end
    object EditAutoMount: TEdit
      Left = 65
      Top = 18
      Width = 174
      Height = 21
      TabOrder = 2
    end
  end
  object ButtonOK: TButton
    Left = 85
    Top = 211
    Width = 75
    Height = 25
    Caption = #30830#23450
    TabOrder = 5
    OnClick = ButtonOKClick
  end
end

object FormMount: TFormMount
  Left = 402
  Top = 384
  BorderStyle = bsDialog
  Caption = #25171#24320#19968#20010#21152#23494#23481#22120#25991#20214'...'
  ClientHeight = 131
  ClientWidth = 293
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object LabelPassword: TLabel
    Left = 12
    Top = 11
    Width = 42
    Height = 13
    Caption = '      '#23494#30721
  end
  object PanelExpert: TPanel
    Left = 12
    Top = 38
    Width = 269
    Height = 58
    BevelOuter = bvNone
    TabOrder = 4
    object LabelDrivers: TLabel
      Left = 0
      Top = 3
      Width = 28
      Height = 13
      Caption = #30424#31526':'
    end
    object ComboBoxDrivers: TComboBox
      Left = 52
      Top = 0
      Width = 77
      Height = 21
      ItemHeight = 13
      TabOrder = 0
    end
    object CheckBoxRO: TCheckBox
      Left = 172
      Top = 2
      Width = 97
      Height = 17
      Caption = #21152#36733#20026#21482#35835
      TabOrder = 1
    end
    object CheckBoxMountExit: TCheckBox
      Left = 0
      Top = 29
      Width = 129
      Height = 17
      Caption = #21152#36733#21518#36864#20986#26412#31243#24207
      TabOrder = 2
    end
  end
  object ButtonMount: TButton
    Left = 24
    Top = 95
    Width = 75
    Height = 25
    Caption = #25171#24320
    TabOrder = 0
    OnClick = ButtonMountClick
  end
  object ButtonCancel: TButton
    Left = 200
    Top = 95
    Width = 75
    Height = 25
    Cancel = True
    Caption = #21462#28040
    TabOrder = 1
    OnClick = ButtonCancelClick
  end
  object EditPassword: TEdit
    Left = 64
    Top = 8
    Width = 217
    Height = 21
    PasswordChar = '*'
    TabOrder = 2
    OnKeyPress = EditPasswordKeyPress
  end
  object ButtonExpert: TButton
    Left = 112
    Top = 95
    Width = 75
    Height = 25
    Caption = #39640#32423
    TabOrder = 3
    OnClick = ButtonExpertClick
  end
end

object FormNewBox: TFormNewBox
  Left = 329
  Top = 346
  BorderStyle = bsDialog
  Caption = #21019#24314#19968#20010#21152#23494#23481#22120#25991#20214'...'
  ClientHeight = 208
  ClientWidth = 387
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
  object LabelFileName: TLabel
    Left = 12
    Top = 15
    Width = 46
    Height = 13
    Caption = '  '#25991#20214#21517':'
  end
  object LabelPassword: TLabel
    Left = 12
    Top = 42
    Width = 43
    Height = 13
    Caption = '     '#23494#30721':'
  end
  object LabelSize: TLabel
    Left = 12
    Top = 69
    Width = 43
    Height = 13
    Caption = '     '#23610#23544':'
  end
  object LabelM: TLabel
    Left = 168
    Top = 69
    Width = 44
    Height = 13
    Caption = #20806'('#23383#33410')'
  end
  object Label1: TLabel
    Left = 195
    Top = 42
    Width = 54
    Height = 13
    Caption = '  '#20877#27425#36755#20837
  end
  object EditFileName: TEdit
    Left = 68
    Top = 12
    Width = 229
    Height = 21
    TabOrder = 0
  end
  object ButtonBrowse: TButton
    Left = 299
    Top = 10
    Width = 75
    Height = 25
    Caption = #27983#35272'...'
    TabOrder = 1
    OnClick = ButtonBrowseClick
  end
  object EditPassword1: TEdit
    Left = 68
    Top = 39
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 2
  end
  object EditPassword2: TEdit
    Left = 253
    Top = 39
    Width = 121
    Height = 21
    PasswordChar = '*'
    TabOrder = 3
  end
  object CSpinEditSize: TCSpinEdit
    Left = 68
    Top = 66
    Width = 97
    Height = 22
    TabOrder = 4
    Value = 32
  end
  object ButtonCreate: TButton
    Left = 36
    Top = 175
    Width = 75
    Height = 25
    Caption = #21019#24314
    TabOrder = 5
    OnClick = ButtonCreateClick
  end
  object ButtonExpert: TButton
    Left = 160
    Top = 175
    Width = 75
    Height = 25
    Caption = #39640#32423
    TabOrder = 6
    OnClick = ButtonExpertClick
  end
  object ButtonCancel: TButton
    Left = 279
    Top = 175
    Width = 75
    Height = 25
    Cancel = True
    Caption = #21462#28040
    TabOrder = 7
    OnClick = ButtonCancelClick
  end
  object RadioGroupAlgo: TRadioGroup
    Left = 12
    Top = 96
    Width = 362
    Height = 70
    Caption = #36873#25321#21152#23494#31639#27861
    Columns = 3
    ItemIndex = 2
    Items.Strings = (
      #19981#21152#23494
      '2Fish'
      'AES 128'
      'AES 192'
      'AES 256')
    TabOrder = 8
  end
end

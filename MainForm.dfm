object FormMain: TFormMain
  Left = 341
  Top = 335
  BorderStyle = bsDialog
  Caption = 'EZCrypt'#26131#23494
  ClientHeight = 238
  ClientWidth = 384
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnClose = FormClose
  OnCreate = FormCreate
  OnHide = FormHide
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object BitBtnCreate: TBitBtn
    Left = 0
    Top = 0
    Width = 64
    Height = 64
    Caption = #21019#24314
    TabOrder = 0
    OnClick = BitBtnCreateClick
  end
  object BitBtnMount: TBitBtn
    Left = 64
    Top = 0
    Width = 64
    Height = 64
    Caption = #25171#24320
    TabOrder = 1
    OnClick = BitBtnMountClick
  end
  object BitBtnClose: TBitBtn
    Left = 128
    Top = 0
    Width = 64
    Height = 64
    Caption = #20851#38381
    TabOrder = 2
    OnClick = BitBtnCloseClick
  end
  object BitBtnExit: TBitBtn
    Left = 320
    Top = 0
    Width = 64
    Height = 64
    Cancel = True
    Caption = #38544#34255
    TabOrder = 3
    OnClick = BitBtnExitClick
  end
  object BitBtnMode: TBitBtn
    Left = 192
    Top = 0
    Width = 64
    Height = 64
    Caption = #39640#32423
    TabOrder = 4
    OnClick = BitBtnModeClick
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 217
    Width = 384
    Height = 21
    Panels = <
      item
        Width = 300
      end
      item
        Width = 50
      end>
  end
  object BitBtnOption: TBitBtn
    Left = 256
    Top = 0
    Width = 64
    Height = 64
    Caption = #35774#32622
    TabOrder = 6
    OnClick = BitBtnOptionClick
  end
  object StringGridDrvInfo: TStringGrid
    Left = 0
    Top = 62
    Width = 384
    Height = 157
    FixedCols = 0
    RowCount = 1
    FixedRows = 0
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goRowSelect]
    PopupMenu = VolsPopupMenu
    TabOrder = 7
    OnContextPopup = StringGridDrvInfoContextPopup
  end
  object OpenDialog: TOpenDialog
    Left = 4
    Top = 68
  end
  object TrayIcon: TTrayIcon
    PopupMenu = TrayPopupMenu
    Visible = True
    OnDblClick = TrayIconDblClick
    Left = 32
    Top = 68
  end
  object TrayPopupMenu: TPopupMenu
    Left = 60
    Top = 68
    object mnuShowMainForm: TMenuItem
      Caption = #26174#31034#30028#38754
      OnClick = mnuShowMainFormClick
    end
    object mnuCloseAll: TMenuItem
      Caption = #20840#37096#20851#38381
      OnClick = mnuCloseAllClick
    end
    object mnuAbout: TMenuItem
      Caption = #20851#20110#26131#23494
      OnClick = mnuAboutClick
    end
    object mnuSpliter0: TMenuItem
      Caption = '-'
    end
    object mnuExit: TMenuItem
      Caption = #36864#20986#26131#23494
      OnClick = mnuExitClick
    end
  end
  object VolsPopupMenu: TPopupMenu
    Left = 88
    Top = 68
    object mnuCloseit: TMenuItem
      Caption = #20851#38381#36825#20010
      OnClick = mnuCloseitClick
    end
    object mnuCloseAllVol: TMenuItem
      Caption = #20851#38381#20840#37096
      OnClick = mnuCloseAllVolClick
    end
  end
end

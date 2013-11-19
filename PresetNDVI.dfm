object PresetFormNDVI: TPresetFormNDVI
  Left = 673
  Top = 669
  BorderStyle = bsDialog
  Caption = 'NDVI'
  ClientHeight = 221
  ClientWidth = 315
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object TitleLabel: TLabel
    Left = 12
    Top = 8
    Width = 27
    Height = 12
    Caption = 'NDVI'
  end
  object Label1: TLabel
    Left = 64
    Top = 128
    Width = 12
    Height = 12
    Caption = #36196
  end
  object Label2: TLabel
    Left = 40
    Top = 156
    Width = 36
    Height = 12
    Caption = #36817#36196#22806
  end
  object BtnOk: TButton
    Left = 152
    Top = 188
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    TabOrder = 2
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 232
    Top = 188
    Width = 75
    Height = 25
    Caption = #12461#12515#12531#12475#12523
    TabOrder = 3
    OnClick = BtnCancelClick
  end
  object DetailMemo: TMemo
    Left = 24
    Top = 28
    Width = 277
    Height = 81
    TabStop = False
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Color = cl3DLight
    Lines.Strings = (
      #26893#29289#12398#20803#27671#20855#21512#12434#21512#25104#12375#12414#12377#12290
      #30333#12369#12428#12400#30333#12356#12411#12393#12289#12381#12398#37096#20998#12398#26893#29289#12364#20803#27671#12391#12354#12427#12371#12392
      #12434#34920#12375#12390#12356#12414#12377#12290
      ''
      #20197#19979#12398#65298#12388#12398#12496#12531#12489#12434#36984#25246#12375#12390#12367#12384#12373#12356)
    ReadOnly = True
    TabOrder = 4
  end
  object RedComboBox: TComboBox
    Left = 84
    Top = 124
    Width = 209
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
  end
  object NIRComboBox: TComboBox
    Left = 84
    Top = 152
    Width = 209
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
  end
end

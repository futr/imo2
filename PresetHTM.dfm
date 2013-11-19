object PresetFormTHM: TPresetFormTHM
  Left = 191
  Top = 575
  BorderStyle = bsDialog
  Caption = #12469#12540#12514#12464#12521#12501#12451#12540#39080
  ClientHeight = 313
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
    Width = 98
    Height = 12
    Caption = #12469#12540#12514#12464#12521#12501#12451#12540#39080
  end
  object Label1: TLabel
    Left = 52
    Top = 192
    Width = 24
    Height = 12
    Caption = #23550#35937
  end
  object Label2: TLabel
    Left = 64
    Top = 252
    Width = 12
    Height = 12
    Caption = #24335
  end
  object BtnOk: TButton
    Left = 152
    Top = 282
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    TabOrder = 4
    OnClick = BtnOkClick
  end
  object BtnCancel: TButton
    Left = 232
    Top = 282
    Width = 75
    Height = 25
    Caption = #12461#12515#12531#12475#12523
    TabOrder = 5
    OnClick = BtnCancelClick
  end
  object DetailMemo: TMemo
    Left = 24
    Top = 28
    Width = 277
    Height = 125
    TabStop = False
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Color = cl3DLight
    Lines.Strings = (
      #12402#12392#12388#12398#12496#12531#12489#12434#12469#12540#12514#12464#12521#12501#12451#12540#39080#12395#33394#21512#25104#12375#12414#12377#12290
      #20516#12364#22823#12365#12356#12411#12393#12381#12398#37096#20998#12364#36196#12395#36817#12389#12365#12289#23567#12373#12356#12411#12393#40658
      #12395#36817#12389#12365#12414#12377#12290
      ''
      #20197#19979#12398#12402#12392#12388#12398#12496#12531#12489#12289#12418#12375#12367#12399#24335#12434#36984#25246#12375#12390#12367#12384#12373#12356#12290
      #12383#12384#12375#12289'X, Y, Z'#12399#20869#37096#12391#35336#31639#29992#12395#20351#29992#12375#12390#12356#12427#12398#12391#36984
      #25246#12420#20351#29992#12434#12375#12394#12356#12391#12367#12384#12373#12356#12290
      ''
      #65288#20966#29702#12395#33509#24178#12398#26178#38291#12364#12363#12363#12426#12414#12377#65289)
    ReadOnly = True
    TabOrder = 6
  end
  object BandComboBox: TComboBox
    Left = 84
    Top = 188
    Width = 209
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
  end
  object BandRadioButton: TRadioButton
    Left = 12
    Top = 164
    Width = 113
    Height = 17
    Caption = #12496#12531#12489#12395#20966#29702'(&B)'
    Checked = True
    TabOrder = 0
    TabStop = True
    OnClick = BandRadioButtonClick
  end
  object ExpRadioButton: TRadioButton
    Left = 12
    Top = 224
    Width = 113
    Height = 17
    Caption = #24335#12395#20966#29702'(&E)'
    TabOrder = 2
    OnClick = ExpRadioButtonClick
  end
  object ExpComboBox: TComboBox
    Left = 84
    Top = 248
    Width = 209
    Height = 20
    Enabled = False
    ItemHeight = 12
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
  end
end

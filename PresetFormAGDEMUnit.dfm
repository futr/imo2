object PresetFormAGDEM: TPresetFormAGDEM
  Left = 287
  Top = 630
  BorderStyle = bsDialog
  Caption = 'ASTER'#12288'GDEM'#39640#24230#20998#24067
  ClientHeight = 195
  ClientWidth = 349
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 12
  object TitleLabel: TLabel
    Left = 12
    Top = 8
    Width = 125
    Height = 12
    Caption = 'ASTER'#12288'GDEM'#39640#24230#20998#24067
  end
  object Label1: TLabel
    Left = 76
    Top = 96
    Width = 46
    Height = 12
    Caption = #12496#12531#12489'(&B)'
    FocusControl = BandComboBox
  end
  object Label2: TLabel
    Left = 188
    Top = 128
    Width = 65
    Height = 12
    Caption = #26368#39640#39640#24230'[m]'
  end
  object Label3: TLabel
    Left = 24
    Top = 128
    Width = 65
    Height = 12
    Caption = #26368#20302#39640#24230'[m]'
  end
  object DetailMemo: TMemo
    Left = 24
    Top = 28
    Width = 313
    Height = 49
    TabStop = False
    BevelInner = bvNone
    BevelOuter = bvNone
    BorderStyle = bsNone
    Color = cl3DLight
    Lines.Strings = (
      'ASTER GEDM'#12398#39640#24230#12487#12540#12479#12434#20351#12387#12390#39640#24230#20998#24067#12434#20316#25104#12375#12414#12377#12290
      ''
      'ASTER GDEM'#12395#23550#24540#12377#12427#12496#12531#12489#12434#36984#25246#12375#12390#12367#12384#12373#12356)
    ReadOnly = True
    TabOrder = 0
  end
  object BandComboBox: TComboBox
    Left = 128
    Top = 92
    Width = 209
    Height = 20
    Style = csDropDownList
    ItemHeight = 12
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
  end
  object BtnCancel: TButton
    Left = 264
    Top = 160
    Width = 75
    Height = 25
    Caption = #38281#12376#12427
    TabOrder = 7
    OnClick = BtnCancelClick
  end
  object BtnOk: TButton
    Left = 184
    Top = 160
    Width = 75
    Height = 25
    Caption = #23455#34892'(&E)'
    Default = True
    TabOrder = 6
    OnClick = BtnOkClick
  end
  object MaxEdit: TEdit
    Left = 260
    Top = 124
    Width = 61
    Height = 20
    TabOrder = 4
    Text = '1,500'
  end
  object MaxUpDown: TUpDown
    Left = 321
    Top = 124
    Width = 15
    Height = 20
    Associate = MaxEdit
    Min = -32768
    Max = 32767
    Position = 1500
    TabOrder = 5
    Wrap = False
  end
  object MinEdit: TEdit
    Left = 96
    Top = 124
    Width = 61
    Height = 20
    TabOrder = 2
    Text = '0'
  end
  object MinUpDown: TUpDown
    Left = 157
    Top = 124
    Width = 15
    Height = 20
    Associate = MinEdit
    Min = -32768
    Max = 32767
    Position = 0
    TabOrder = 3
    Wrap = False
  end
end

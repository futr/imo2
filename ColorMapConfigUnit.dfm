object ColorMapConfigForm: TColorMapConfigForm
  Left = 549
  Top = 194
  BorderStyle = bsDialog
  Caption = #12524#12505#12523#12473#12521#12452#12473
  ClientHeight = 615
  ClientWidth = 455
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poMainFormCenter
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 12
  object Label2: TLabel
    Left = 12
    Top = 8
    Width = 351
    Height = 12
    Caption = #12300#33394#12398#21106#12426#24403#12390#12301#12398#20013#12398#25968#23383#12434#12463#12522#12483#12463#12377#12427#12392#33394#12392#12524#12505#12523#12434#35373#23450#12391#12365#12414#12377
  end
  object Label6: TLabel
    Left = 16
    Top = 80
    Width = 52
    Height = 12
    Caption = #26368#23567#20516'(&B)'
  end
  object Label9: TLabel
    Left = 12
    Top = 28
    Width = 407
    Height = 12
    Caption = #12300#33394#12398#21106#12426#24403#12390#12301#20869#12398#26368#12418#22823#12365#12356#20516#20197#19978#12398#20516#12398#33394#12399#12289#26368#12418#22823#12365#12356#20516#12398#33394#12392#21516#12376#12391#12377
  end
  object LevelImageBox: TGroupBox
    Left = 12
    Top = 52
    Width = 149
    Height = 325
    Caption = #33394#12398#21106#12426#24403#12390
    TabOrder = 0
    object LevelImage: TImage
      Left = 16
      Top = 20
      Width = 61
      Height = 273
    end
    object SmoothCheckBox: TCheckBox
      Left = 16
      Top = 300
      Width = 121
      Height = 17
      Hint = #12394#12417#12425#12363#12395#12524#12505#12523#12473#12521#12452#12473#12434#34892#12358
      Caption = #12394#12417#12425#12363#12395#12377#12427'(&S)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      OnClick = SmoothCheckBoxClick
    end
  end
  object LevelEditorGroupBox: TGroupBox
    Left = 172
    Top = 228
    Width = 273
    Height = 149
    Caption = #33394#12392#20516#12398#35373#23450
    TabOrder = 2
    object Label1: TLabel
      Left = 138
      Top = 52
      Width = 113
      Height = 12
      Caption = #12414#12391#12434#20197#19979#12398#33394#12391#22615#12427
    end
    object Bevel1: TBevel
      Left = 8
      Top = 96
      Width = 257
      Height = 14
      Shape = bsBottomLine
    end
    object LevelEdit: TEdit
      Left = 56
      Top = 48
      Width = 53
      Height = 20
      Hint = #12524#12505#12523#12398#20516
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      Text = '0'
    end
    object AddLevelButton: TButton
      Left = 186
      Top = 116
      Width = 75
      Height = 25
      Hint = #12524#12505#12523#12434#36861#21152
      Caption = #36861#21152'(&A)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 5
      OnClick = AddLevelButtonClick
    end
    object DeleteLevelButton: TButton
      Left = 106
      Top = 116
      Width = 75
      Height = 25
      Hint = #12524#12505#12523#12434#21066#38500
      Caption = #21066#38500'(&D)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 6
      OnClick = DeleteLevelButtonClick
    end
    object LevelEditUpDown: TUpDown
      Left = 109
      Top = 48
      Width = 16
      Height = 20
      Hint = #12524#12505#12523#12398#20516#12434#65297#22793#26356#12377#12427
      Min = -32768
      Max = 32767
      ParentShowHint = False
      Position = 0
      ShowHint = True
      TabOrder = 2
      Wrap = False
      OnClick = LevelEditUpDownClick
    end
    object LevelColorPanel: TPanel
      Left = 16
      Top = 76
      Width = 165
      Height = 25
      Hint = #12524#12505#12523#12398#33394#12434#36984#12406
      ParentShowHint = False
      ShowHint = True
      TabOrder = 3
      OnClick = LevelColorPanelClick
    end
    object SelectColorButton: TButton
      Left = 186
      Top = 76
      Width = 75
      Height = 25
      Hint = #12524#12505#12523#12398#33394#12434#36984#12406
      Caption = #33394#12434#36984#12406'(&I)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 4
      OnClick = SelectColorButtonClick
    end
    object StaticRatioCheckBox: TCheckBox
      Left = 16
      Top = 20
      Width = 213
      Height = 17
      Hint = #19968#30058#20302#12356#12363#39640#12356#12524#12505#12523#12434#22793#26356#12377#12427#12392#20182#12398#12524#12505#12523#12364#27604#29575#12434#23432#12387#12390#36861#24467#12377#12427
      Caption = #26368#19978#19979#12434#22793#26356#12375#12383#22580#21512#27604#29575#12434#20445#12388'(&R)'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
    end
    object SetButton: TButton
      Left = 16
      Top = 48
      Width = 37
      Height = 21
      Hint = #12524#12505#12523#12398#20516#12434#35373#23450#12377#12427
      Caption = #36969#29992
      Default = True
      Enabled = False
      ParentShowHint = False
      ShowHint = True
      TabOrder = 7
      OnClick = SetButtonClick
    end
  end
  object PreViewGroupBox: TGroupBox
    Left = 172
    Top = 52
    Width = 273
    Height = 169
    Caption = #12503#12524#12499#12517#12540
    TabOrder = 1
    object PreviewImage: TImage
      Left = 2
      Top = 14
      Width = 269
      Height = 153
      Align = alClient
    end
  end
  object ExpGroupBox: TGroupBox
    Left = 12
    Top = 516
    Width = 345
    Height = 89
    Caption = #24335#12392#21336#20301
    TabOrder = 3
    object Label3: TLabel
      Left = 56
      Top = 24
      Width = 29
      Height = 12
      Caption = #24335'(&E):'
      FocusControl = ExpEdit
    end
    object Label4: TLabel
      Left = 8
      Top = 56
      Width = 78
      Height = 12
      Caption = #21336#20301#12398#25991#23383'(&U):'
      FocusControl = UnitStrEdit
    end
    object ExpEdit: TEdit
      Left = 92
      Top = 20
      Width = 237
      Height = 20
      Hint = #30011#32032#20516#12434#22793#25563#12377#12427#24335
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      Text = 'a'
      OnChange = ExpEditChange
    end
    object UnitStrEdit: TEdit
      Left = 92
      Top = 52
      Width = 237
      Height = 20
      Hint = #24335#12391#35336#31639#12373#12428#12427#20516#12398#21336#20301#12398#25991#23383
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      Text = #21336#20301
      OnChange = UnitStrEditChange
    end
  end
  object CancelButton: TButton
    Left = 368
    Top = 580
    Width = 79
    Height = 25
    Hint = #38281#12376#12427
    Caption = 'OK'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 4
    OnClick = CancelButtonClick
  end
  object ClearButton: TButton
    Left = 368
    Top = 520
    Width = 79
    Height = 25
    Hint = #35373#23450#12434#26368#21021#12398#29366#24907#12395#25147#12377
    Caption = #12463#12522#12450'(&C)'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 5
    OnClick = ClearButtonClick
  end
  object GroupBox1: TGroupBox
    Left = 12
    Top = 384
    Width = 433
    Height = 125
    Caption = #12503#12522#12475#12483#12488
    TabOrder = 6
    object Label5: TLabel
      Left = 16
      Top = 68
      Width = 54
      Height = 12
      Caption = #26368#23567#20516'(&B):'
      FocusControl = MinLevelEdit
    end
    object Label7: TLabel
      Left = 284
      Top = 68
      Width = 53
      Height = 12
      Caption = #26368#22823#20516'(&T):'
      FocusControl = MaxLevelEdit
    end
    object Label8: TLabel
      Left = 12
      Top = 100
      Width = 289
      Height = 12
      Caption = #12503#12522#12475#12483#12488#12398#35373#23450#12434#22793#26356#12377#12427#12392#29694#22312#12398#35373#23450#12399#30772#26820#12373#12428#12414#12377
    end
    object PreSetComboBox: TComboBox
      Left = 12
      Top = 20
      Width = 409
      Height = 36
      Style = csOwnerDrawFixed
      ItemHeight = 30
      TabOrder = 0
      OnDrawItem = PreSetComboBoxDrawItem
      OnSelect = PreSetComboBoxSelect
    end
    object MinLevelEdit: TEdit
      Left = 76
      Top = 64
      Width = 61
      Height = 20
      TabOrder = 1
      Text = '0'
      OnChange = MinLevelEditChange
    end
    object MaxLevelEdit: TEdit
      Left = 344
      Top = 64
      Width = 61
      Height = 20
      TabOrder = 2
      Text = '255'
      OnChange = MaxLevelEditChange
    end
    object SetPresetButton: TButton
      Left = 344
      Top = 92
      Width = 75
      Height = 25
      Caption = #36969#29992'(&P)'
      TabOrder = 3
      Visible = False
      OnClick = SetPresetButtonClick
    end
    object MaxLevelUpDown: TUpDown
      Left = 405
      Top = 64
      Width = 15
      Height = 20
      Associate = MaxLevelEdit
      Min = -32768
      Max = 32767
      Position = 255
      TabOrder = 4
      Wrap = False
    end
    object MinLevelUpDown: TUpDown
      Left = 137
      Top = 64
      Width = 15
      Height = 20
      Associate = MinLevelEdit
      Min = -32768
      Max = 32767
      Position = 0
      TabOrder = 5
      Wrap = False
    end
  end
  object CloseButton: TButton
    Left = 368
    Top = 552
    Width = 79
    Height = 25
    Caption = #12461#12515#12531#12475#12523'(&X)'
    TabOrder = 7
    OnClick = CloseButtonClick
  end
  object LevelColorDialog: TColorDialog
    Ctl3D = True
    Left = 184
    Top = 68
  end
end

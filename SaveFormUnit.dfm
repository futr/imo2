object SaveForm: TSaveForm
  Left = 1007
  Top = 126
  BorderStyle = bsDialog
  Caption = #20445#23384
  ClientHeight = 154
  ClientWidth = 343
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object TimeLabel: TLabel
    Left = 12
    Top = 112
    Width = 4
    Height = 12
  end
  object SaveModeRadioGroup: TRadioGroup
    Left = 12
    Top = 12
    Width = 321
    Height = 73
    Caption = #20445#23384#26041#27861
    ItemIndex = 0
    Items.Strings = (
      #29694#22312#12398#30011#38754' ( '#29694#22312#35211#12360#12390#12356#12427#29366#24907#12391#20445#23384#12375#12414#12377' )'
      #30011#20687#20840#20307' ( '#22823#12365#12394#30011#20687#12391#12399#20445#23384#12395#25968#20998#20197#19978#12363#12363#12426#12414#12377' )')
    TabOrder = 0
  end
  object CancelButton: TButton
    Left = 248
    Top = 120
    Width = 83
    Height = 25
    Caption = #12461#12515#12531#12475#12523'(&C)'
    TabOrder = 1
    OnClick = CancelButtonClick
  end
  object SaveButton: TButton
    Left = 160
    Top = 120
    Width = 83
    Height = 25
    Caption = #20445#23384'(&S)'
    Default = True
    TabOrder = 2
    OnClick = SaveButtonClick
  end
  object SaveProgressBar: TProgressBar
    Left = 12
    Top = 92
    Width = 321
    Height = 16
    Min = 0
    Max = 100
    TabOrder = 3
  end
  object ImgSaveDialog: TSaveDialog
    DefaultExt = 'bmp'
    Filter = #12499#12483#12488#12510#12483#12503'|*.bmp'
    Left = 296
    Top = 8
  end
end

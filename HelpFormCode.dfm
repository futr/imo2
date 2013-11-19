object HelpForm: THelpForm
  Left = 287
  Top = 178
  Width = 581
  Height = 456
  Caption = #31777#26131#12504#12523#12503
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 12
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 573
    Height = 390
    Align = alClient
    BevelOuter = bvNone
    Caption = 'Panel1'
    TabOrder = 0
    object Memo1: TMemo
      Left = 0
      Top = 0
      Width = 573
      Height = 390
      Align = alClient
      Lines.Strings = (
        #9632' SatelliteEye'
        '(c) 2011 futr'
        'ilce.ma@gmail.com'
        ''
        #9632' '#23550#24540#24418#24335
        'ALOS'#12398'CEOS'#24418#24335#12398#12487#12540#12479#12395#23550#24540#12375#12390#12356#12414#12377#12290
        #29694#22312#12398#12392#12371#12429#12289'PRISM, AVNIR2'#12398#12487#12540#12479#12434#35501#12415#36796#12416#12371#12392#12364#12391#12365#12414#12377#12290
        #12381#12398#20182#12398#12487#12540#12479#12418#12289#19968#37096#35501#12415#36796#12417#12427#12371#12392#12364#12354#12426#12414#12377#12364#12289#22522#26412#30340#12395#12399#19978#35352#12398#12487#12540#12479#23554#29992#12391#12377#12290
        #12414#12383#12289'summary.txt'#12363#12425#24231#27161#12487#12540#12479#12434#35501#12415#36796#12417#12414#12377#12290
        ''
        #19968#37096#12398'TIFF'#24418#24335#12398#30011#20687#12395#23550#24540#12375#12390#12356#12414#12377#12290
        'GeoTIFF'#12418#35501#12415#36796#12416#12371#12392#12399#12391#12365#12414#12377#12364#12289#24231#27161#31561#12434#34920#12377#24773#22577#12399#35501#12415#36796#12414#12428#12414#12379#12435#12290
        'LANDSAT'#12398#12487#12540#12479#12398#22580#21512#12289#25313#24373#23376#12364#12300'met'#12301#12398#12486#12461#12473#12488#12487#12540#12479#12434#34907#26143#12487#12540#12479#12398)
      ReadOnly = True
      ScrollBars = ssVertical
      TabOrder = 0
    end
  end
  object Panel2: TPanel
    Left = 0
    Top = 390
    Width = 573
    Height = 39
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    DesignSize = (
      573
      39)
    object Button1: TButton
      Left = 489
      Top = 7
      Width = 75
      Height = 25
      Anchors = [akRight, akBottom]
      Caption = #38281#12376#12427'(&C)'
      TabOrder = 0
      OnClick = Button1Click
    end
  end
end

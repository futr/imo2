//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetLS8_THM.h"
#include "Main.h"
#include "ColorBar.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormLS8_THM *PresetFormLS8_THM;
//---------------------------------------------------------------------------
__fastcall TPresetFormLS8_THM::TPresetFormLS8_THM(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS8_THM::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void TPresetFormLS8_THM::readSetting( void )
{
	// 設定を読み込む
    if ( TypeRadioButton10->Checked ) {
    	ALEdit->Text = FloatToStr( rad_add_10 );
        MLEdit->Text = FloatToStr( rad_mul_10 );
    	k1Edit->Text = FloatToStr( k1_10 );
    	k2Edit->Text = FloatToStr( k2_10 );
    } else {
    	ALEdit->Text = FloatToStr( rad_add_11 );
        MLEdit->Text = FloatToStr( rad_mul_11 );
    	k1Edit->Text = FloatToStr( k1_11 );
    	k2Edit->Text = FloatToStr( k2_11 );
    }
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS8_THM::BtnOkClick(TObject *Sender)
{
	/* 温度分布用カラーマップを作成 */
    ColorMap *map;

    AnsiString unit_str;
    AnsiString cb;

    char *fmt_r;
    char *fmt_g;
    char *fmt_b;
    char *fmt_thm;
    char *fmt_cb_r;
    char *fmt_cb_g;
	char *fmt_cb_b;


    char thm;
    char band;

    int temp_width = TempUpDown->Position - TempMinUpDown->Position;
    int max_temp = TempUpDown->Position;
    int min_temp = TempMinUpDown->Position;

    float k1 = StrToFloat( k1Edit->Text );
    float k2 = StrToFloat( k2Edit->Text );
    float ml = StrToFloat( MLEdit->Text );
    float al = StrToFloat( ALEdit->Text );


    map = SatViewMainForm->color_map;

    /* コンボボックスをキャラクタへ */
    thm  = BandComboBox->ItemIndex + 0x61;
    band = 'w';

    // 温度計算式
    fmt_thm = "%c = %f / ln( %f / ( %c * %f + %f ) + 1 ) - 273.15";
	unit_str = unit_str.sprintf( fmt_thm, band, k2, k1, thm, ml, al );

    // 式を登録
    map->setExpression( unit_str );
    map->setUnitString( "℃" );
    map->setRange( min_temp, max_temp );

    // 色を作成
    map->deleteAllColorLevel();

    map->addColorLevel( new ColorLevel( max_temp, 255, 0, 0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 4 / 5.0 ) + min_temp, 255, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 3 / 5.0 ) + min_temp,   0, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 2 / 5.0 ) + min_temp,   0, 255, 255 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 1 / 5.0 ) + min_temp,   0,   0, 255 ) );
    map->addColorLevel( new ColorLevel( ( max_temp - min_temp ) * ( 0 / 5.0 ) + min_temp,   0,   0,   0 ) );

    map->setSmooth();

    // メインフォームをカラーマップモードに
    SatViewMainForm->LevelDrawRadioButton->Checked = true;

    /* ヒストグラム設定解除 */
    SatViewMainForm->disableBandRange( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] );

	// ボタン無効
    //this->Visible = false;
    this->BtnOk->Enabled = false;

    /* ついでに書かせる */
    SatViewMainForm->DrawImg();

    // カラーバー表示
    ColorBarForm->viewNowValue( true );
    ColorBarForm->setColorMap( SatViewMainForm->color_map );
    ColorBarForm->setExpMode( false );
    ColorBarForm->setUnitString( "a", "℃" );
    ColorBarForm->Show();
    ColorBarForm->draw();

    /* ボタン復帰 */
    this->BtnOk->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS8_THM::FormShow(TObject *Sender)
{
	BtnOk->Enabled = true;	
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS8_THM::TypeRadioButton10Click(TObject *Sender)
{
	readSetting();	
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormLS8_THM::TypeRadioButton11Click(TObject *Sender)
{
	readSetting();
}
//---------------------------------------------------------------------------

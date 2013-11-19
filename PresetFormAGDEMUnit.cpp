//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetFormAGDEMUnit.h"
#include "Main.h"
#include "ColorBar.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormAGDEM *PresetFormAGDEM;
//---------------------------------------------------------------------------
__fastcall TPresetFormAGDEM::TPresetFormAGDEM(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TPresetFormAGDEM::BtnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormAGDEM::BtnOkClick(TObject *Sender)
{
	// 高さ用カラーマップ作成
    ColorMap *map;

    int maxh;
    int minh;

    char band6;

    maxh = MaxUpDown->Position;
    minh = MinUpDown->Position;

    map = SatViewMainForm->color_map;

    /* コンボボックスをキャラクタへ */
    band6 = BandComboBox->ItemIndex + 0x61;

    // 式を登録
    map->setExpression( band6 );
    map->setUnitString( "m" );
    map->setRange( minh, maxh );

    // 色を作成
    map->deleteAllColorLevel();

    map->addColorLevel( new ColorLevel( minh + 0.9,                             0,   0, 255 ) );
    map->addColorLevel( new ColorLevel( minh + 1,                             116, 116, 255 ) );
    map->addColorLevel( new ColorLevel( ( maxh - minh ) * ( 1 / 4.0 ) + minh,   0, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( maxh - minh ) * ( 2 / 4.0 ) + minh, 255, 255,   0 ) );
    map->addColorLevel( new ColorLevel( ( maxh - minh ) * ( 3 / 4.0 ) + minh, 160, 100,   0 ) );
    map->addColorLevel( new ColorLevel( ( maxh - minh ) * ( 4 / 4.0 ) + minh,  20,  20,   0 ) );

    map->setSmooth();

    // メインフォームをカラーマップモードに
    SatViewMainForm->LevelDrawRadioButton->Checked = true;

    /* ヒストグラム設定解除 */
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_bottom->Position = 0;
    ( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] )->updown_top->Position = 255;

    SatViewMainForm->UpdateBandBox( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[BandComboBox->ItemIndex] );


	// ボタン無効
    //this->Visible = false;
    this->BtnOk->Enabled = false;

    /* ついでに書かせる */
    SatViewMainForm->DrawImg();

    // カラーバー表示
    // ColorBarForm->Caption = "高さ";
    ColorBarForm->viewNowValue( true );
    ColorBarForm->setColorMap( SatViewMainForm->color_map );
    ColorBarForm->setExpMode( false );
    ColorBarForm->setUnitString( "a", "m" );
    ColorBarForm->Show();
    ColorBarForm->draw();

    /* ボタン復帰 */
    this->BtnOk->Enabled = true;

}
//---------------------------------------------------------------------------

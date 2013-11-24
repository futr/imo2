//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetNDVI.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormNDVI *PresetFormNDVI;
//---------------------------------------------------------------------------
__fastcall TPresetFormNDVI::TPresetFormNDVI(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::BtnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::BtnOkClick(TObject *Sender)
{
	/* メインフォームのバンドエディットを書き換え */
    AnsiString r;
    AnsiString g;
    AnsiString b;

    ColorMap *map;

    char *fmt_r;

    char band_red;
    char band_ired;

    // map設定
    map = SatViewMainForm->color_map;

    /* バンド初期値 */
    // fmt_r = "( %c - %c ) / ( %c + %c ) * 128 + 128";
    fmt_r = "( %c - %c ) / ( %c + %c ) * 100";

    /* コンボボックスをキャラクタへ */
    band_red  = RedComboBox->ItemIndex + 0x61;
    band_ired = NIRComboBox->ItemIndex + 0x61;

    /* sprintfで式を作る */
    r = r.sprintf( fmt_r, band_ired, band_red, band_ired, band_red );
    g = r;
    b = r;

    /* メインへ */
    /*
    SatViewMainForm->ExpEditR->Text = r;
    SatViewMainForm->ExpEditG->Text = g;
    SatViewMainForm->ExpEditB->Text = b;
    */

    // 式を登録
    map->setExpression( r );
    map->setUnitString( "[NDVI×100]" );
    map->setRange( -100, 100 );

    // 色を作成
    map->deleteAllColorLevel();

    map->addColorLevel( new ColorLevel(  100, 255, 255, 255 ) );
    map->addColorLevel( new ColorLevel( -100,   0,   0,   0 ) );

    map->setSmooth();

    // メインフォームをカラーマップモードに
    SatViewMainForm->LevelDrawRadioButton->Checked = true;

    /* ヒストグラム設定解除 */
    SatViewMainForm->disableBandRange( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[RedComboBox->ItemIndex] );
    SatViewMainForm->disableBandRange( (struct REMOS_FRONT_BAND *)SatViewMainForm->list_band->Items[NIRComboBox->ItemIndex] );

    /* 見えなくなる */
    this->Visible = false;
    this->BtnOk->Enabled = false;

    // メインフォームを式モードに
    // SatViewMainForm->ExpDrawRadioButton->Checked = true;

    /* ついでに書かせる */
    SatViewMainForm->DrawImg();

    Close();
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormNDVI::FormShow(TObject *Sender)
{
	BtnOk->Enabled = true;
}
//---------------------------------------------------------------------------

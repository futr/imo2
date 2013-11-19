//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PresetHTM.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPresetFormTHM *PresetFormTHM;
//---------------------------------------------------------------------------
__fastcall TPresetFormTHM::TPresetFormTHM(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormTHM::BtnOkClick(TObject *Sender)
{
	/* メインフォームのバンドエディットを書き換え */
    AnsiString r;
    AnsiString g;
    AnsiString b;
    AnsiString exp;

    char c_band;

    char *fmt_r;
    char *fmt_g;
    char *fmt_b;
    char* band;

    exp = "";

    /* バンド初期値 */
    fmt_r = "( w = %s ), ( w < ( 255 / 5 * 4 ) )if( x = 0 ), ( w > ( 255 / 5 * 4 ) )if( x = 255 ), ( ( w < ( 255 / 5 * 4 ) ) * ( w > ( 255 / 5 * 3 ) ) )if( x = 255 * ( ( w - ( 255 / 5 * 3 ) ) / ( 255 / 5 ) ) ), x";
    fmt_g = "( w = %s ), ( w < ( 255 / 5 * 2 ) )if( y = 0 ), ( w > ( 255 / 5 * 2 ) )if( y = 255 ), ( ( w > ( 255 / 5 ) ) * ( w < ( 255 / 5 * 2 ) ) )if( y = 255 * ( ( w - 255 / 5 ) / ( 255 / 5 ) ) ), ( ( w > ( 255 / 5 * 4 ) ) * ( w < ( 256 ) ) )if( y = 255 - 255 * ( ( w - ( 255 / 5 * 4 ) ) / ( 255 / 5 ) ) ), y";
    fmt_b = "( w = %s ), ( w < ( 255 / 5 * 3 ) )if( z = 255 ), ( w > ( 255 / 5 * 3 ) )if( z = 0 ), ( w < ( 255 / 5 ) )if( z = 255 * ( w / ( 255 / 5 ) ) ), ( ( w > ( 255 / 5 * 2 ) ) * ( w < ( 255 / 5 * 3 ) ) )if( z = 255 - 255 * ( ( w - ( 255 / 5 * 2 ) ) / ( 255 / 5 ) ) ), z";

    /* 式作成 */
    if ( BandRadioButton->Checked ) {
    	/* バンドだった */

        /* コンボボックスをキャラクタへ */
        c_band = BandComboBox->ItemIndex + 0x61;

        exp = exp.sprintf( "%c", c_band );

    } else {
    	/* 式だった */
        exp = ExpComboBox->Text;
    }

    /* かっこ */
    exp = "( " + exp + " )";

    /* アドレス確保 */
    band = exp.c_str();

    /* sprintfで式を作る */
    r = r.sprintf( fmt_r, band );
    g = g.sprintf( fmt_g, band );
    b = b.sprintf( fmt_b, band );

    /* メインへ */
    SatViewMainForm->ExpEditR->Text = r;
    SatViewMainForm->ExpEditG->Text = g;
    SatViewMainForm->ExpEditB->Text = b;

    /* 見えなくなる */
    this->Visible = false;
    this->BtnOk->Enabled = false;

    /* ついでに書かせる */
    SatViewMainForm->DrawImg();

    Close();
}
//---------------------------------------------------------------------------

void __fastcall TPresetFormTHM::FormShow(TObject *Sender)
{
	BtnOk->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TPresetFormTHM::BtnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------


void __fastcall TPresetFormTHM::BandRadioButtonClick(TObject *Sender)
{
	BandComboBox->Enabled = true;
    ExpComboBox->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TPresetFormTHM::ExpRadioButtonClick(TObject *Sender)
{
	BandComboBox->Enabled = false;
    ExpComboBox->Enabled = true;
}
//---------------------------------------------------------------------------


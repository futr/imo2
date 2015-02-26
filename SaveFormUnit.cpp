//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SaveFormUnit.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSaveForm *SaveForm;
//---------------------------------------------------------------------------
__fastcall TSaveForm::TSaveForm(TComponent* Owner)
	: TForm(Owner)
{
	saving = false;
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::CancelButtonClick(TObject *Sender)
{
	// 保存中なら強制停止するか確認する
    if ( saving ) {
    	if ( Application->MessageBoxA( "保存を中止しますか？\n保存を中止した場合画像が正しく保存されません。", "保存を中止", MB_YESNO | MB_ICONQUESTION ) == ID_YES ) {
        	saving = false;
            CancelButton->Enabled = false;
        }
    } else {
    	Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::SaveButtonClick(TObject *Sender)
{
	/* 保存する */
    SaveButton->Enabled = false;
    CancelButton->Enabled = true;
    SaveModeRadioGroup->Enabled = false;

    saving = true;

    if ( ImgSaveDialog->Execute() ) {
    	// 上書き確認
        if ( FileExists( ImgSaveDialog->FileName ) ) {
        	if( Application->MessageBoxA( "ファルがすでに存在しています。\n上書きしますか？", "上書き確認", MB_YESNO | MB_ICONWARNING ) == ID_NO ) {
            	goto L_EXIT_SAVE;
            }
        }

        if ( SaveModeRadioGroup->ItemIndex == 0 ) {
            /* そのまま */
            SatViewMainForm->SatImage->Picture->SaveToFile( ImgSaveDialog->FileName );
        } else {
            /* 等倍 ( DEBUG : 二番目の引数は倍率の準備 ) */
			SaveToBmp( ImgSaveDialog->FileName, 0 );
        }
    }

L_EXIT_SAVE:
	saving = false;
    CancelButton->Enabled = true;
    SaveButton->Enabled = true;
    SaveModeRadioGroup->Enabled = true;

    SaveProgressBar->Position = 0;

    Close();
}
//---------------------------------------------------------------------------
bool __fastcall TSaveForm::SaveToBmp( AnsiString filename, int zpos )
{
	/* MainFormのDrawを一部コピー */

	/* 描画関数 */
    TSatViewMainForm *mf;
    EBMP_FILE ebmp;

    struct REMOS_FRONT_BAND *band;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];

    /* 電卓ユニット */
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;

    int zoom_pos;
    AnsiString stime;
    unsigned int start;
    unsigned int now;

    // MainFormから持ってくる
    mf = SatViewMainForm;
    ColorMap *cmap = mf->color_map;
    TList *list_band = mf->list_band;
    bool drawModeExp = mf->b_draw_mode_exp;

    int i, j, k, l;
    int skip;
    int blockSize;
    RGBTRIPLE *rgb;
    unsigned char red;
    unsigned char blue;
    unsigned char green;
    unsigned char *buf;
    double mag;
    int img_start_x;
    int img_start_y;
    int img_read_xc;
    int img_read_yc;
    int img_use_xc;
    int img_use_yc;
    int draw_w;
    int draw_h;
    int draw_x;
    int draw_y;

    /* 時間測定開始 */
    start = GetTickCount();

    /* メインフォームのポインタ */
    mf = SatViewMainForm;

    tok_r = NULL;
    tok_b = NULL;
    tok_g = NULL;

    /* 開いてなければ実行しない */
    if ( !mf->b_open ) {
    	return false;
    }

    /* 描画中なら実行しない */
    if ( mf->b_drawing ) {
    	return false;
    }

    /* 描画許可がなければ描画しない */
    if ( !mf->b_draw ) {
    	return false;
    }

    mf->b_drawing = true;

    // ズームは一倍固定
    zoom_pos = ZOOM_MAX / 2;
    mag = mf->ZoomPosToMag( zoom_pos );

    /* 変数ポインタ登録 */
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* 式ユニット初期化 */
	ecalc_free_token( tok_b );
    tok_b = ecalc_make_token( mf->ExpEditB->Text.c_str() );
	tok_b = ecalc_make_tree( tok_b );

	ecalc_free_token( tok_g );
    tok_g = ecalc_make_token( mf->ExpEditG->Text.c_str() );
	tok_g = ecalc_make_tree( tok_g );

	ecalc_free_token( tok_r );
    tok_r = ecalc_make_token( mf->ExpEditR->Text.c_str() );
	tok_r = ecalc_make_tree( tok_r );

    /* 座標計算 */
    draw_x = 0;
    draw_y = 0;
    draw_w = mf->img_w;
    draw_h = mf->img_h;

    // skip（読み込み飛ばし量）と画素ブロックサイズ決定

    if ( mag >= 1 ) {
        skip = 1;
        blockSize = mag;
    } else {
        skip = 1 / mag;
        blockSize = 1;
    }

    /* 画像読み込み開始地点決定 */
    img_start_x = 0;
    img_start_y = 0;

	/* 読み込み個数（範囲）決定 */
	img_read_xc = ( draw_w - draw_x ) / mag;
	img_read_yc = ( draw_h - draw_y ) / mag;

    /* 実際に使われる個数 */
    img_use_xc = img_read_xc / skip;
    img_use_yc = img_read_yc / skip;

    // 画面行バッファ作成 ( R, G, B )
    buf = (unsigned char *)malloc( img_read_xc * sizeof( RGBTRIPLE ) * blockSize );

    /* プログレスバー設定 */
    SaveProgressBar->Max = img_read_yc;
    SaveProgressBar->Position = 0;

    /* BMP作成 */
    ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc, img_read_yc );

    // 読み込みと描画ループ
    for ( i = 0; i < img_use_yc; i++ ) {
        /* 各バンド読み込み */
        for ( j = 0; j < list_band->Count; j++ ) {
            /* バンド指定、データ読み込み */
            band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

            // バンドのラインバッファ読み込み
            mf->ReadBandLineBuf( band, img_start_y + i * skip, img_start_x, img_read_xc );
        }

        /* 列データ読み出しループ */
        for ( j = 0; j < img_use_xc; j++ ) {
            /* 色作成 */
            for ( k = 0; k < list_band->Count; k++ ) {
            	/* バンド取得 */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[k]);

                /* 値登録 */
                if ( band->canvas_mode ) {
                    // キャンバスモード
                	var[k] = remos_get_ranged_pixel( band->band, band->line_buf[j * skip] );
                } else {
                    var[k] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, j * skip ) );
                }
            }

            // 式モードかカラーバーモードか
            if ( drawModeExp ) {
            	// 式モード
                red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
            } else {
            	// カラーバーモード
                cmap->evalExpression( vars, 0 );
                cmap->makeColor();

                red   = mf->GetUCharValue( cmap->getR() );
                green = mf->GetUCharValue( cmap->getG() );
                blue  = mf->GetUCharValue( cmap->getB() );
            }

            // バッファに保存
            rgb = (RGBTRIPLE *)buf;

            // 行を作る
            for ( k = 0; k < blockSize; k++ ) {
                rgb[j * blockSize + k].rgbtBlue  = blue;
                rgb[j * blockSize + k].rgbtGreen = green;
                rgb[j * blockSize + k].rgbtRed   = red;
            }
        }

        // 行blockSize分描画ループ
        for ( j = 0; j < blockSize; j++ ) {
            /* 画像書き出し */
            ebmp_write_line( &ebmp, j + draw_y + i * blockSize, buf, ebmp.line_size );
        }

        // UI処理

        /* プログレス */
        SaveProgressBar->Position = i;

        /* 時間計測 */
        now = GetTickCount();

        if ( i % 10 == 0 && i ) {
            stime.sprintf( "処理時間 : %d秒 / %d秒", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        // 脱出処理
        if ( !saving ) {
            break;
        }
    }

    // バッファ解放
    free( buf );

    /* BMPファイル閉じる */
    ebmp_create_file_close( &ebmp );

    mf->b_drawing = false;

    return true;
}
//---------------------------------------------------------------------------
void __fastcall TSaveForm::FormShow(TObject *Sender)
{
	saving = false;
    CancelButton->Enabled = true;
    SaveButton->Enabled = true;
    SaveModeRadioGroup->Enabled = true;
    TimeLabel->Caption = "";
}
//---------------------------------------------------------------------------



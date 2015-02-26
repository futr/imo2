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
    unsigned int start;
    unsigned int now;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];
    AnsiString str;

    int i, j, k, l;
    int skip;
    int zoom_pos;
    unsigned char red;
    unsigned char blue;
    unsigned char green;

    /* 電卓ユニット */
    struct ECALC_TOKEN *tok_r;
    struct ECALC_TOKEN *tok_g;
    struct ECALC_TOKEN *tok_b;

    int img_read_xc;
    int img_read_yc;

    double mag;


    unsigned char *buf;

    AnsiString stime;

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

	/* DEBUG : ここでは強制等倍 */
    zoom_pos = ZOOM_MAX / 2;

    /* 倍率設定 */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* 縮小 */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
        skip = 1.0 / mag;
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* 拡大 */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
        skip = mag;
    } else {
    	/* 等倍 */
    	mag  = 1;
        skip = 1;
    }

    /* 拡大か縮小化で描画方法変更 */
    if ( mag >= 1 ) {
    	/* 拡大 */

    	/* 読み込み個数決定 */
    	img_read_xc = mf->img_w;
    	img_read_yc = mf->img_h;

        /* プログレスバー設定 */
        SaveProgressBar->Max = img_read_yc;
        SaveProgressBar->Position = 0;

        /* BMP作成 */
        ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc * skip, img_read_yc * skip );

    	/* バッファー確保 */
    	buf = (unsigned char *)malloc( ebmp.line_size );

        /* 読み込み描画ループ */
        for ( i = 0; i < img_read_yc; i++ ) {
        	/* 強制停止されたら止まる */
            if ( !saving ) {
            	break;
            }

            /* 各バンド読み込み */
            for ( j = 0; j < mf->list_band->Count; j++ ) {
                /* バンド指定、データ読み込み */
                band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[j]);

                /* モードで方法が違う */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, 0 + i, 0, img_read_xc );
                } else {
                	mf->GetLineData( band, band->line_buf, 0 + i, 0, img_read_xc );
                }
            }

            /* 画面に書き込み */

            /* 列データ読み出しループ */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* 色作成 */
                for ( l = 0; l < mf->list_band->Count; l++ ) {
                    /* バンド取得 */
                    band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[l]);

                    /* 値登録 */
                    if ( band->canvas_mode ) {
                        // キャンバスモード
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k] );
                    } else {
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, k ) );
                    }
                }

                // 式モードかカラーバーモードか
                if ( mf->b_draw_mode_exp ) {
                	// 式モード
                    red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// カラーバーモード
                    mf->color_map->evalExpression( vars, 0 );
                    mf->color_map->makeColor();

                    red   = mf->GetUCharValue( mf->color_map->getR() );
                    green = mf->GetUCharValue( mf->color_map->getG() );
                    blue  = mf->GetUCharValue( mf->color_map->getB() );
                }

                for ( l = 0; l < skip; l++ ) {
                    buf[l * 3 + k * 3 * skip + 0] = blue;
                    buf[l * 3 + k * 3 * skip + 1] = green;
                    buf[l * 3 + k * 3 * skip + 2] = red;
                }
            }

            /* 行ループ */
            for ( j = 0; j < skip; j++ ) {
                /* 画像書き出し */
                ebmp_write_line( &ebmp, i * skip + j, buf, ebmp.line_size );
            }

            /* プログレス */
            SaveProgressBar->Position = i;

            /* 時間計測 */
            now = GetTickCount();

            if ( i ) {
            	stime.sprintf( "処理時間 : %d秒 / %d秒", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            }

            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        /* BMPファイル閉じる */
        ebmp_create_file_close( &ebmp );
    } else {
    	/* 縮小 */

    	/* 読み込み個数決定 */
        img_read_xc = ( mf->img_w ) / skip;
        img_read_yc = ( mf->img_h ) / skip;

        /* プログレスバー設定 */
        SaveProgressBar->Max = img_read_yc;
        SaveProgressBar->Position = 0;

        /* BMP作成 */
        ebmp_create_file_open( &ebmp, filename.c_str(), img_read_xc, img_read_yc );

    	/* バッファー確保 */
    	buf = (unsigned char *)malloc( ebmp.line_size );

        /* 読み込み描画ループ */
        for ( i = 0; i < img_read_yc; i++ ) {
        	// 強制停止されたら止まる
            if ( !saving ) {
            	break;
            }

            /* 各バンド読み込み */
            for ( j = 0; j < mf->list_band->Count; j++ ) {
                /* バンド指定、データ読み込み */
                band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[j]);

               /* モードで方法が違う */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, i * skip, 0, img_read_xc * skip );
                } else {
                	mf->GetLineData( band, band->line_buf, i * skip, 0, img_read_xc * skip );
                }
            }

            /* 画面に書き込み */

            /* 列データ読み出しループ */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* 色作成 */
                for ( l = 0; l < mf->list_band->Count; l++ ) {
                	/* バンド取得 */
                    band = (struct REMOS_FRONT_BAND *)(mf->list_band->Items[l]);

                    /* 値登録 */
                    if ( band->canvas_mode ) {
                        // キャンバスモード
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    } else {
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf, k * skip ) );
                    }
                }

                // 式モードかカラーバーモードか
                if ( mf->b_draw_mode_exp ) {
                	// 式モード
                    red   = mf->GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = mf->GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = mf->GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// カラーバーモード
                    mf->color_map->evalExpression( vars, 0 );
                    mf->color_map->makeColor();

                    red   = mf->GetUCharValue( mf->color_map->getR() );
                    green = mf->GetUCharValue( mf->color_map->getG() );
                    blue  = mf->GetUCharValue( mf->color_map->getB() );
                }

                buf[k * 3 + 0] = blue;
                buf[k * 3 + 1] = green;
                buf[k * 3 + 2] = red;
            }

            /* 画像書き出し */
            ebmp_write_line( &ebmp, i, buf, ebmp.line_size );

            /* プログレス */
            SaveProgressBar->Position = i;

            /* 時間計測 */
            now = GetTickCount();

            if ( i ) {
            	stime.sprintf( "処理時間 : %d秒 / %d秒", ( now - start ) / 1000, (int)( ( now - start ) * ( (float)img_read_yc / i ) / 1000 ) );
            }

            TimeLabel->Caption = stime;

            Application->ProcessMessages();
        }

        /* BMPファイル閉じる */
        ebmp_create_file_close( &ebmp );
    }

    /* バッファ開放 */
    free( buf );

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



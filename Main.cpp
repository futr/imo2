#include "ColorBar.h"
#include "SaveFormUnit.h"
#include "PresetLS_THM.h"
#include "PresetHTM.h"
#include "PresetNDVI.h"
#include "About.h"
#include "PixValue.h"

/* ヒストグラムの処理を大幅に書き換え中、どんなビット数でも256レベルに分解する */
/* ヒストグラムの問題は修正したはず */
/* Canvasモードのヒストグラム間違ってない？ */
/* Canvasモードの排他処理が適当 */
/* b_canvasで一応排他処理をしているが、一応なのでちゃんと作るときは書き換える */

/* 場所値ダイアログでfloatとかusintとかをむちゃくちゃにしてしまった */
/* remosに不完全なpal1.1が付いている */
/* 場所値ダイアログで拡大モード時位置がおかしい、特に拡大して左に空白がある場合 */

/* 描画関数のコピーが大量にあるのと、remosから値を取り出す方法が一致してない上に分散していること */
/* 座標値取得のコードも分散していることなどの問題がある。 */
/* レンジの設定もほとんど機能していない */

#include "Status.h"
//---------------------------------------------------------------------------

#include <vcl.h>
#include <Jpeg.hpp>
#pragma hdrstop

#include "Main.h"

#include <math.h>
#include <Math.hpp>

#include "ecalc.h"
#include "remos.h"
#include "econf.h"
#include "ColorMapConfigUnit.h"
#include "PresetFormAGDEMUnit.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TSatViewMainForm *SatViewMainForm;

/* 画像起点 */
int img_x_start = 0;
int img_y_start = 0;

//---------------------------------------------------------------------------
__fastcall TSatViewMainForm::TSatViewMainForm(TComponent* Owner)
	: TForm(Owner)
{
	/* ヒント設定 */
	Application->OnHint = ShowHint;

	/* リスト確保 */
    list_band = new TList();
    list_file = new TList();

    /* バックバッファ作成 */
    img_back = new Graphics::TBitmap();

    /* Canvasモード用 */

    /* 値初期化 */
    b_open = false;
    b_click = false;
    b_drawing = false;
    b_zoom_click = false;
    b_canvas = false;
    b_sync = false;
    b_draw = true;
    b_config = false;
    b_config_land = false;   					/* これは経度緯度用 */
    b_config_resolution = false;
    b_mode_length = false;
    b_mode_stamp_cb = false;
    b_draw_mode_exp = true;

    // 設定初期化
    clearSetting();

    tok_r = NULL;
    tok_g = NULL;
    tok_b = NULL;
    tok_color_bar = NULL;

    color_bar_exp = "a";

    img_w = 1;
    img_h = 1;

    mag = 1;

    zoom_pos = ZOOM_MAX / 2;

	/* メモリマネージャ初期化 */
	ecalc_memman_init();

    /* タイトル */
    Application->Title = "SatelliteEye";

    /* ヒント処理登録 */
    Application->OnHint = DisplayHint;

    /* ドラッグ受け取り */
    DragAcceptFiles( Handle, true );

    /* メッセージ処理関数設定 */
    Application->OnMessage = AppMessage;

    // ホイールイベントハンドラー設定 ( 多分ウィンドウメッセージを処理しないと無理 )
    // SatImage->OnMouseWheel = ImageMouseWheelEvent;
    SatViewMainForm->OnMouseWheel = ImageMouseWheelEvent;

    /* ブロードキャストメッセージ登録 */
    umsg_x       = RegisterWindowMessage( "SATIMGVIEW_SCR_X_9145" );
    umsg_y       = RegisterWindowMessage( "SATIMGVIEW_SCR_Y_9145" );
    umsg_mag     = RegisterWindowMessage( "SATIMGVIEW_SCR_MAG_9145" );
    umsg_connect = RegisterWindowMessage( "SATIMGVIEW_CONNECT_9145" );
    umsg_lat     = RegisterWindowMessage( "SATIMGVIEW_LAT_9145" );
    umsg_lon     = RegisterWindowMessage( "SATIMGVIEW_LON_9145" );
    umsg_draw    = RegisterWindowMessage( "SATIMGVIEW_DRAW_9145" );

    // カラーマップ初期化
    color_map = new ColorMap;

    // カラーマップに白黒を初期値として設定
    color_map->deleteAllColorLevel();
    color_map->addColorLevel( new ColorLevel(   0,   0,   0,   0 ) );
    color_map->addColorLevel( new ColorLevel( 255, 255, 255, 255 ) );
    color_map->setSmooth();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	/* リスト解放 */
    int i;

    /* ファイル開放 */
    for ( i = 0; i < list_file->Count; i++ ) {
    	remos_close( (struct REMOS_FILE_CONTAINER *)list_file->Items[i] );
        free( list_file->Items[i] );
    }

    /* バンドボックス開放 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	FreeBandBox( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
        free( list_band->Items[i] );
    }

    // カラーマップ破棄
    delete color_map;

    delete list_band;
    delete list_file;
    delete img_back;

    ecalc_free_token( tok_r );
    ecalc_free_token( tok_g );
    ecalc_free_token( tok_b );
    ecalc_free_token( tok_color_bar );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ShowHint( TObject *sender )
{
	/* ヒントを表示 */
    StatusBar->SimpleText = Application->Hint;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBOpenClick(TObject *Sender)
{
	/* ファイルを開く */

    if ( OpenDialog->Execute() ) {
    	OpenFiles();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBSaveClick(TObject *Sender)
{
    /* 開いてなければ実行しない */
    if ( !b_open ) {
    	return;
    }

	/* 画像保存 ( 変に操作されると怖いのでModal ) */
    SaveForm->ShowModal();

    /*
    if ( SaveDialog->Execute() ) {
    	SatImage->Picture->SaveToFile( SaveDialog->FileName );
    }
	*/
}
//---------------------------------------------------------------------------
struct REMOS_FRONT_BAND *TSatViewMainForm::MakeBandBox( struct REMOS_BAND *band, AnsiString fln, int index )
{
    /* バンドボックス作成 */
    char num[2];
    struct REMOS_FRONT_BAND *box;

    /* バンド番号作成 */
    num[0] = 0x61 + index;
    num[1] = '\0';

    /* 確保 */
    box = (struct REMOS_FRONT_BAND *)malloc( sizeof(struct REMOS_FRONT_BAND) );

    box->img_hist    = new TImage( SatViewMainForm );
    box->label_name  = new TLabel( SatViewMainForm );
    box->panel_cont  = new TPanel( SatViewMainForm );

    box->btn_close = new TBitBtn( SatViewMainForm );
    box->btn_auto  = new TBitBtn( SatViewMainForm );
    box->btn_max   = new TBitBtn( SatViewMainForm );
    box->btn_red   = new TBitBtn( SatViewMainForm );
    box->btn_green = new TBitBtn( SatViewMainForm );
    box->btn_blue  = new TBitBtn( SatViewMainForm );
    box->btn_wb    = new TBitBtn( SatViewMainForm );

    box->label_bottom = new TLabel( SatViewMainForm );
    box->label_top = new TLabel( SatViewMainForm );
    box->edit_bottom = new TEdit( SatViewMainForm );
    box->updown_bottom = new TUpDown( SatViewMainForm );
    box->edit_top = new TEdit( SatViewMainForm );
    box->updown_top = new TUpDown( SatViewMainForm );

    /* 設定 */
    box->index = index;
    box->band = band;
    box->fln = new AnsiString( fln );

    box->panel_cont->Align  = alTop;
    box->panel_cont->Parent = BandList;
    box->panel_cont->Height = 100;
    box->panel_cont->Tag    = (int)box;

    box->btn_close->Parent     = box->panel_cont;
    box->btn_auto->Parent      = box->panel_cont;
    box->btn_max->Parent       = box->panel_cont;
    box->btn_wb->Parent        = box->panel_cont;
    box->btn_red->Parent       = box->panel_cont;
    box->btn_green->Parent     = box->panel_cont;
    box->btn_blue->Parent      = box->panel_cont;
    box->img_hist->Parent      = box->panel_cont;
    box->label_name->Parent    = box->panel_cont;
    box->label_bottom->Parent  = box->panel_cont;
    box->label_top->Parent     = box->panel_cont;
    box->edit_bottom->Parent   = box->panel_cont;
    box->updown_bottom->Parent = box->panel_cont;
    box->edit_top->Parent      = box->panel_cont;
    box->updown_top->Parent    = box->panel_cont;

    box->btn_close->TabOrder   = 0;
    box->edit_bottom->TabOrder = 1;
    box->edit_top->TabOrder    = 2;
    box->btn_auto->TabOrder    = 3;
    box->btn_max->TabOrder     = 4;
    box->btn_red->TabOrder     = 5;
    box->btn_green->TabOrder   = 6;
    box->btn_blue->TabOrder    = 7;
    box->btn_wb->TabOrder      = 8;


    box->img_hist->OnMouseDown = HistMouseDown;
    box->img_hist->OnMouseMove = HistMouseMove;
    box->img_hist->OnMouseUp   = HistMouseUp;

    box->btn_close->OnClick = BandCloseBtnClick;

    box->edit_bottom->OnChange = BandEditDownChange;
    box->edit_top->OnChange    = BandEditUpChange;

    box->btn_blue->OnClick  = BlueBtnClick;
    box->btn_red->OnClick   = RedBtnClick;
    box->btn_green->OnClick = GreenBtnClick;
    box->btn_auto->OnClick  = AutoBtnClick;
    box->btn_wb->OnClick    = WBBtnClick;
    box->btn_max->OnClick   = MaxBtnClick;

    box->img_hist->Top  = 24;
    box->img_hist->Left = 4;

    box->img_hist->Height = 45;
    box->img_hist->Width  = 256;

    box->img_hist->Canvas->Brush->Color = clWhite;
    box->img_hist->Canvas->FillRect( Rect( 0, 0, 177, 45 ) );

    box->label_bottom->Left = 8;
    box->label_bottom->Top = 76;
    box->label_bottom->Caption = "下";

    box->label_top->Left = 68;
    box->label_top->Top = 76;
    box->label_top->Caption = "上";

    box->edit_bottom->Left = 24;
    box->edit_bottom->Top = 72;
    box->edit_bottom->Width = 40;

    box->edit_top->Top = 72;
    box->edit_top->Left = 84;
    box->edit_top->Width = 40;

    // box->updown_bottom->Associate = box->edit_bottom;
    // box->updown_top->Associate = box->edit_top;
    box->updown_bottom->Visible = false;
    box->updown_top->Visible    = false;

    box->updown_bottom->Max = 255;
    box->updown_top->Max = 255;
    box->updown_top->Position = 255;

    box->label_name->Top = 8;
    box->label_name->Left = 28;
    // box->label_name->Font->Color = clBlue;
    box->label_name->Caption = num;
    box->label_name->Caption = box->label_name->Caption + " : " + fln;;

    box->btn_close->Top         = 4;
    box->btn_close->Left        = 4;
    box->btn_close->Height      = 17;
    box->btn_close->Width       = 21;
    box->btn_close->Caption     = "×";
    box->btn_close->Font->Color = clRed;

    box->btn_blue->Top         = 72;
    box->btn_blue->Left        = 224;
    box->btn_blue->Height      = 21;
    box->btn_blue->Width       = 17;
    box->btn_blue->Caption     = "■";
    box->btn_blue->Font->Color = clBlue;
    box->btn_blue->Font->Size  = 10;

    box->btn_red->Top         = 72;
    box->btn_red->Left        = 184;
    box->btn_red->Height      = 21;
    box->btn_red->Width       = 17;
    box->btn_red->Caption     = "■";
    box->btn_red->Font->Color = clRed;
    box->btn_red->Font->Size  = 10;

    box->btn_green->Top         = 72;
    box->btn_green->Left        = 204;
    box->btn_green->Height      = 21;
    box->btn_green->Width       = 17;
    box->btn_green->Caption     = "■";
    box->btn_green->Font->Color = clGreen;
    box->btn_green->Font->Size  = 10;

    box->btn_wb->Top         = 72;
    box->btn_wb->Left        = 244;
    box->btn_wb->Height      = 21;
    box->btn_wb->Width       = 17;
    box->btn_wb->Caption     = "■";
    box->btn_wb->Font->Color = clBlack;
    box->btn_wb->Font->Size  = 10;

    box->btn_auto->Top         = 72;
    box->btn_auto->Left        = 128;
    box->btn_auto->Height      = 21;
    box->btn_auto->Width       = 29;
    box->btn_auto->Caption     = "自動";
    box->btn_auto->Font->Size  = 8;

    box->btn_max->Top         = 72;
    box->btn_max->Left        = 156;
    box->btn_max->Height      = 21;
    box->btn_max->Width       = 21;
    box->btn_max->Caption     = "<>";
    box->btn_max->Font->Size  = 8;

    box->band->range_bottom = box->band->range_bottom;
    box->band->range_top    = box->band->range_top;

    box->hist_click = false;

    /* Canvasモード用 */
    box->img_canvas = NULL;

    /* ラインバッファ作成 */
    box->line_buf = (unsigned char *)malloc( box->band->line_img_width * box->band->byte_per_sample * box->band->sample_per_pix );	// DEBUG バグを招く可能性が高い

    return box;
}
//---------------------------------------------------------------------------
void TSatViewMainForm::FreeBandBox( struct REMOS_FRONT_BAND *box )
{
	/* バンドボックス開放 */
    delete box->fln;

	delete box->img_hist;
    delete box->label_name;
    delete box->btn_blue;
    delete box->btn_red;
    delete box->btn_green;
    delete box->btn_auto;
    delete box->btn_wb;
    delete box->btn_max;

    // DEBUG プログラム終了時に削除してくれるのでほっとくdelete box->btn_close;
    box->btn_close->Visible = false;
    box->btn_close->Parent = SatViewMainForm;

    delete box->panel_cont;

    free( box->line_buf );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawImg( TImage *screen, Graphics::TBitmap *back_screen, bool drawModeExp, int zoompos, int img_cent_x, int img_cent_y, ColorMap *cmap )
{
	/* 描画関数 */
    struct REMOS_FRONT_BAND *band;
    unsigned char *line_buf;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];
    AnsiString str;

    int i, j, k, l;
    int skip;
    int xpos;
    RGBTRIPLE *rgb;
    unsigned char red;
    unsigned char blue;
    unsigned char green;

    unsigned char *buf;

    double mag;	// グローバルとかぶっているの強制ローカル

    int sc_w;
    int sc_h;
    int sc_x;
    int sc_y;

    int img_start_x;
    int img_start_y;
    int img_read_xc;
    int img_read_yc;
    int img_read_xw;
    int img_read_yw;

    int draw_w;
    int draw_h;
    int draw_x;
    int draw_y;
    double draw_x_b;
    double draw_y_b;

    int bb_w;
    int bb_h;

    /* 開いてなければ実行しない */
    if ( !b_open ) {
    	return;
    }

    /* 描画中なら実行しない */
    if ( b_drawing ) {
    	return;
    }

    /* 描画許可がなければ描画しない */
    if ( !b_draw ) {
    	return;
    }

    b_drawing = true;

    /* 変数ポインタ登録 */
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* 式ユニット初期化 */
	ecalc_free_token( tok_b );
    tok_b = ecalc_make_token( ExpEditB->Text.c_str() );
	tok_b = ecalc_make_tree( tok_b );

	ecalc_free_token( tok_g );
    tok_g = ecalc_make_token( ExpEditG->Text.c_str() );
	tok_g = ecalc_make_tree( tok_g );

	ecalc_free_token( tok_r );
    tok_r = ecalc_make_token( ExpEditR->Text.c_str() );
	tok_r = ecalc_make_tree( tok_r );

    /* スクリーンサイズ取得 */
    sc_w = screen->Width;
    sc_h = screen->Height;

	/* キャンバスを作らせる */
    screen->Canvas->Brush->Color = clWhite;
    screen->Picture->Bitmap->Width = sc_w;
    screen->Picture->Bitmap->Height = sc_h;

    screen->Picture->Bitmap->PixelFormat = pf24bit;
    screen->Picture->Bitmap->HandleType = bmDIB;

    /* 処理中 */
    str = "描画中";
    screen->Canvas->Font->Size   = 12;
    screen->Canvas->Font->Color  = clRed;
    screen->Canvas->Brush->Color = clWhite;
    screen->Canvas->TextOutA( sc_w / 2 - screen->Canvas->TextWidth( str ) / 2, sc_h / 2 - screen->Canvas->TextHeight( str ) - 10, str );
    Application->ProcessMessages();

    /* 座標計算 */

    /* 倍率設定 */
    if ( zoompos > ZOOM_MAX / 2 ) {
    	/* 縮小 */
        mag  = 1.0 / Power( 2, zoompos - ZOOM_MAX / 2 );
        skip = 1.0 / mag;
    } else if ( zoompos < ZOOM_MAX / 2 ) {
    	/* 拡大 */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoompos );
        skip = mag;
    } else {
    	/* 等倍 */
    	mag  = 1;
        skip = 1;
    }

    /* 実質描画空間設定 */
    draw_w = sc_w;
    draw_h = sc_h;

    /* 画像読み込み開始地点決定 */
    img_start_x = img_cent_x - ( draw_w / 2.0 ) * ( 1.0 / mag );
    img_start_y = img_cent_y - ( draw_h / 2.0 ) * ( 1.0 / mag );

    /* 線書き用に保存 */
    img_start_x_line = img_start_x;
    img_start_y_line = img_start_y;

    /* 描画開始点設定 */
    if ( img_start_x < 0 ) {
    	draw_x_b = fabsl( img_cent_x - ( draw_w / 2.0 ) * ( 1.0 / mag ) ) * mag;
        draw_x = draw_x_b;
        img_start_x = 0;
        line_add_x = 0;
    } else {
    	draw_x = 0;
        line_add_x = ( sc_w / 2 ) - ( img_cent_x - img_start_x ) * mag;
    }

    if ( img_start_y < 0 ) {
    	draw_y_b = fabsl( img_cent_y - ( draw_h / 2.0 ) * ( 1.0 / mag ) ) * mag;
        draw_y = draw_y_b;
        img_start_y = 0;
        line_add_y = 0;
    } else {
    	draw_y = 0;
        line_add_y = ( sc_h / 2 ) - ( img_cent_y - img_start_y ) * mag;
    }

    /* キャンパスクリア */
    screen->Canvas->Brush->Color = clWhite;
    screen->Canvas->FillRect( Rect( 0, 0, sc_w, sc_h ) );

    /* 拡大か縮小化で描画方法変更 */
    if ( mag >= 1 ) {
    	/* 拡大 */

    	/* 読み込み個数決定 */
    	img_read_xc = ( draw_w - draw_x ) / skip;
    	img_read_yc = ( draw_h - draw_y ) / skip;

        /* 範囲補正 */
        if ( img_start_x + img_read_xc > img_w ) {
            /* xの読み込み範囲がオーバー */
            img_read_xc = img_w - img_start_x;
        }

        if ( img_start_y + img_read_yc > img_h ) {
            /* yの読み込み範囲がオーバー */
            img_read_yc = img_h - img_start_y;
        }

        /* 画像バッファ作成 */
        buf = (unsigned char *)malloc( img_read_xc * 3 );

        /* 読み込み描画ループ */
        for ( i = 0; i < img_read_yc; i++ ) {
            /* 各バンド読み込み */
            for ( j = 0; j < list_band->Count; j++ ) {
                /* バンド指定、データ読み込み */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

                /* モードで方法が違う */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, img_start_y + i, img_start_x, img_read_xc );
                } else {
                	GetLineData( band, band->line_buf, img_start_y + i, img_start_x, img_read_xc );
                }

                /* レンジ適用 */
                remos_get_ranged_pixels( band->band, band->line_buf, img_read_xc );
            }

            /* 画面に書き込み */

            /* 列データ読み出しループ */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* 色作成 */
                for ( l = 0; l < list_band->Count; l++ ) {
                    /* バンド取得 */
                    band = (struct REMOS_FRONT_BAND *)(list_band->Items[l]);

                    /* 値登録 */
                    // var[l] = band->line_buf[k];
                    if ( band->canvas_mode ) {
                        // キャンバスモード
                    	var[l] = band->line_buf[k];
                    } else {
                    	// remosモード DEBUG : エンディアンを決め打ちにしているのでバグるかも
                        // var[l] = remos_data_to_value( band->line_buf + band->band->bits / 8 * k, band->band->bits / 8, REMOS_ENDIAN_LITTLE );

                    	// DEBUG : 値の取得方法を変更 ( マイナスに対応させるため )
                        var[l] = remos_data_to_value_band( band->band, band->line_buf + band->band->bits / 8 * k );
                    }
                }

                // 式モードかカラーバーモードか
                if ( drawModeExp ) {
                	// 式モード
                    red   = GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// カラーバーモード
                    cmap->evalExpression( vars, 0 );
                    cmap->makeColor();

                    red   = GetUCharValue( cmap->getR() );
                    green = GetUCharValue( cmap->getG() );
                    blue  = GetUCharValue( cmap->getB() );
                }

                buf[k * 3 + 0] = red;
                buf[k * 3 + 1] = green;
                buf[k * 3 + 2] = blue;
            }

            /* 行ループ */
            for ( j = 0; j < skip; j++ ) {
                /* 行ポインタ取得 */
                rgb = (RGBTRIPLE *)screen->Picture->Bitmap->ScanLine[j + draw_y + i * skip];

            	for ( k = 0; k < img_read_xc; k++ ) {
                    for ( l = 0; l < skip; l++ ) {
                        rgb[l + draw_x + k * skip].rgbtBlue  = buf[k * 3 + 2];
                        rgb[l + draw_x + k * skip].rgbtRed   = buf[k * 3 + 0];
                        rgb[l + draw_x + k * skip].rgbtGreen = buf[k * 3 + 1];
                    }
                }
            }
        }

        /* バッファ開放 */
        free( buf );
    } else {
    	/* 縮小 */

    	/* 読み込み個数決定 */
    	img_read_xc = draw_w - draw_x;
    	img_read_yc = draw_h - draw_y;

        /* 範囲補正 */
        if ( img_start_x + img_read_xc * skip > img_w ) {
            /* xの読み込み範囲がオーバー */
            img_read_xc = ( img_w - img_start_x ) / skip;
        }

        if ( img_start_y + img_read_yc * skip > img_h ) {
            /* yの読み込み範囲がオーバー */
            img_read_yc = ( img_h - img_start_y ) / skip;
        }

        /* 読み込み描画ループ */
        for ( i = 0; i < img_read_yc; i++ ) {
            /* 各バンド読み込み */
            for ( j = 0; j < list_band->Count; j++ ) {
                /* バンド指定、データ読み込み */
                band = (struct REMOS_FRONT_BAND *)(list_band->Items[j]);

               /* モードで方法が違う */
                if ( !band->canvas_mode ) {
                	remos_get_line_pixels( band->band, band->line_buf, img_start_y + i * skip, img_start_x, img_read_xc * skip );
                } else {
                	GetLineData( band, band->line_buf, img_start_y + i * skip, img_start_x, img_read_xc * skip );
                }

                /* レンジ適用 */
                // remos_get_ranged_pixels( band->band, band->line_buf, img_read_xc * skip );
            }

            /* 画面に書き込み */

            /* 行ポインタ取得 */
            rgb = (RGBTRIPLE *)screen->Picture->Bitmap->ScanLine[draw_y + i];

            /* 列データ読み出しループ */
            for ( k = 0; k < img_read_xc; k++ ) {
                /* 色作成 */
                for ( l = 0; l < list_band->Count; l++ ) {
                	/* バンド取得 */
                    band = (struct REMOS_FRONT_BAND *)(list_band->Items[l]);

                    /* 値登録 ( レンジの処理はここで行う ) */
                    // var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    /* 値登録 */
                    if ( band->canvas_mode ) {
                        // キャンバスモード
                    	var[l] = remos_get_ranged_pixel( band->band, band->line_buf[k * skip] );
                    } else {
                    	// remosモード DEBUG : エンディアンを決め打ちにしているのでバグるかも
                        // var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value( band->line_buf + band->band->bits / 8 * k * skip, band->band->bits / 8, REMOS_ENDIAN_LITTLE ) );
                    	// DEBUG : 値の取得方法を変更 ( マイナスに対応させるため )
                        var[l] = remos_get_ranged_pixel( band->band, remos_data_to_value_band( band->band, band->line_buf + band->band->bits / 8 * k * skip ) );
                    }
                }

                // 式モードかカラーバーモードか
                if ( drawModeExp ) {
                	// 式モード
                    red   = GetUCharValue( ecalc_get_tree_value( tok_r, vars, 0 ) );
                    green = GetUCharValue( ecalc_get_tree_value( tok_g, vars, 0 ) );
                    blue  = GetUCharValue( ecalc_get_tree_value( tok_b, vars, 0 ) );
                } else {
                	// カラーバーモード
                    cmap->evalExpression( vars, 0 );
                    cmap->makeColor();

                    red   = GetUCharValue( cmap->getR() );
                    green = GetUCharValue( cmap->getG() );
                    blue  = GetUCharValue( cmap->getB() );
                }

                rgb[draw_x + k].rgbtBlue  = blue;
                rgb[draw_x + k].rgbtRed   = red;
                rgb[draw_x + k].rgbtGreen = green;
            }
        }
    }

    // バッファへコピー
    if ( back_screen != NULL ) {
    	back_screen->Canvas->CopyRect( Rect( 0, 0, screen->Width, screen->Height ), screen->Canvas, Rect( 0, 0, screen->Width, screen->Height ) );
	}

    b_drawing = false;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawImg( void )
{
    /* 汎用描画関数を使ってメイン画像を描画 */

    /* magがグローバル変数なので何が起こるかわからないから一応倍率計算だけしておく */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* 縮小 */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* 拡大 */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
    } else {
    	/* 等倍 */
    	mag  = 1;
    }

    /* 描画 */
    DrawImg( SatImage, img_back, b_draw_mode_exp, zoom_pos, ScrImgHor->Position, ScrImgVert->Position, color_map );
}
//---------------------------------------------------------------------------
unsigned char TSatViewMainForm::GetUCharValue( double value )
{
	/* unsigned char に丸め込む */
    if ( value < 0 ) {
    	return 0;
    } else if ( value > 255 ) {
    	return 255;
    } else {
    	return (unsigned char)value;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBCloseClick(TObject *Sender)
{
	/* 開いていたものを全て閉じる */
    int i;
    struct REMOS_FRONT_BAND *box;
    TList *del_list;
    void *del;

    del_list = new TList();

    if ( b_open ) {
        /* バンドボックス開放 */
        for ( i = 0; i < list_band->Count; i++ ) {
        	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        	/* Canvasモードなら削除要求に登録 */
            if ( box->canvas_mode == true ) {
            	del_list->Add( box->img_canvas );
            }

            FreeBandBox( box );
            free( list_band->Items[i] );
        }

        list_band->Clear();

        /* ファイル開放 */
        for ( i = 0; i < list_file->Count; i++ ) {
            remos_close( (struct REMOS_FILE_CONTAINER *)list_file->Items[i] );
            free( list_file->Items[i] );
        }

        list_file->Clear();

        /* canvasモードのcanvasを破棄 */
        for ( i = 0; i < del_list->Count; i++ ) {
        	/* 保存 */
        	del = del_list->Items[i];

            /* NULLならなにもしない */
            if ( del == NULL ) {
            	continue;
            }

            /* 破棄 */
        	delete del_list->Items[i];

            /* 自分と同じものを削除 */
        	while ( del_list->IndexOf( del ) != -1 ) {
            	del_list->Items[del_list->IndexOf( del )] = NULL;
            }
        }
    }

    /* キャンバスクリア */
    SatImage->Canvas->Brush->Color = clWhite;
    SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    /* フラグ解除 */
    b_open = false;

    clearSetting();

    /* リスト破棄 */
    delete del_list;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	TPoint point;

    // DEBUG : フォーカスを設定してみる
    PanelImage->SetFocus();

	/* カーソル移動初期化 */
	b_move = false;

    /* ひらいてなければなにもしない */
    if ( !b_open ) {
    	return;
    }

	/* 左ボタン */
    if ( Button == mbLeft ) {
        cp_x = X;
        cp_y = Y;

        cp_vert = ScrImgVert->Position;
        cp_hor  = ScrImgHor->Position;

        /* バッファ初期化 */
    	img_back->Width  = SatImage->Width;
   		img_back->Height = SatImage->Height;

    	img_back->PixelFormat = pf24bit;
    	img_back->HandleType = bmDIB;

        /* バッファにコピー */
        img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    	b_click = true;
    } else {
    	int vert;
    	int hor;
    	int i;
    	int cent_c_x;
    	int cent_c_y;
        AnsiString str;
        unsigned int val;
    	struct REMOS_FRONT_BAND *box;
        char band[2];
        TPixForm *pix_form;

        // クリック解除
    	b_click = false;

        // 座標値フォーム作成
        pix_form = new TPixForm( this );
        pix_form->do_delete = true;

        /* カーソル位置算定 */
        vert = ScrImgVert->Position;
        hor  = ScrImgHor->Position;

        /* トライアンドエラーでこうなった */
        cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
        cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

        if ( cent_c_x < 0 ) {
            cent_c_x = 0;
        }

        if ( cent_c_y < 0 ) {
            cent_c_y = 0;
        }

        if ( cent_c_y >= img_h ) {
            cent_c_y = img_h - 1;
        }

        if ( cent_c_x >= img_w ) {
            cent_c_x = img_w - 1;
        }

        str = "";
        str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

        // 経度緯度表示
        if ( b_config ) {
            str = str + "経度, 緯度 : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
        }

        /* 各バンド生データ取得 */
        for ( i = 0; i < list_band->Count; i++ ) {
            box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            /* モードで取得方法が違う */
            if ( box->canvas_mode ) {
                val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            } else {
                val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            }

            band[0] = tolower( (char)( box->index + 0x41 ) );
            band[1] = '\0';

            str = str + band;
            str = str + " : " + IntToStr( val ) + "\n";
        }

        pix_form->ValueLabel->Caption = str;

        if ( pix_form->ValueLabel->Height + 30 > pix_form->Height ) {
            pix_form->Height = pix_form->ValueLabel->Height + 30;
        }

        /* カーソル位置にピクセル情報を移動 */
        GetCursorPos( &point );

        pix_form->Left = point.x + 10;
        pix_form->Top  = point.y + 10;

        pix_form->Show();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	int lon;
    int lat;

    if ( b_click ) {
    	// 距離測定モードならなにもしない
        if ( b_mode_length ) {
        	img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        	b_click = false;

        	return;
        }

        // カラーバースタンプモードなら描画してバッファへコピー
        if ( b_mode_stamp_cb ) {
        	b_click = false;

            // バッファコピー
        	//SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            // カラーバー転送
            //SatImage->Canvas->CopyRect( Rect( X + 10, Y + 10, X + 10 + ColorBarForm->ColorBarImage->Width, Y + 10 + ColorBarForm->ColorBarImage->Height ), ColorBarForm->ColorBarImage->Canvas, ColorBarForm->ColorBarImage->Canvas->ClipRect );

            // バッファへ転送
        	//img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            return;
        }

		/* 簡易描画してみる */
		SatImage->Canvas->Brush->Color = clWhite;
    	SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    	SatImage->Canvas->CopyRect( Rect( - 1 * ( cp_x - X ), - 1 * ( cp_y - Y ), SatImage->Width - ( ( cp_x - X ) ), SatImage->Height - ( ( cp_y - Y ) ) ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    	b_click = false;

        /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
        if ( b_sync ) {
            if ( b_config ) {
            	/* 経度緯度可能 */
                lon = 10000 * GetLon( ScrImgHor->Position, ScrImgVert->Position ).ToDouble();
                lat = 10000 * GetLat( ScrImgHor->Position, ScrImgVert->Position ).ToDouble();

                PostMessage( HWND_BROADCAST, umsg_lat, lat, 0 );
                PostMessage( HWND_BROADCAST, umsg_lon, lon, 0 );
            } else {
            	/* XYモード */
        		PostMessage( HWND_BROADCAST, umsg_x, ScrImgHor->Position, 0 );
            	PostMessage( HWND_BROADCAST, umsg_y, ScrImgVert->Position, 0 );
            }

            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandCloseBtnClick(TObject *Sender)
{
	/* バンドを閉じる */
    /* ボタンだけは削除せず、プログラム終了時に削除してもらう : DEBUG */
    struct REMOS_FRONT_BAND *box;
    struct REMOS_FILE_CONTAINER *fc;
    int i;
    bool b_canvas_mode;
    Graphics::TBitmap *bmp;
    AnsiString fln;
    AnsiString fc_fln;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    fln = *box->fln;
    bmp = box->img_canvas;

    /* リスト削除 */
    list_band->Delete( box->index );

    /* バンド削除 */
    FreeBandBox( box );
    free( box );

    /* ボックス更新 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	UpdateBandBox( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
    }

    /* バンド内に同一ファイル名が存在するかチェック */
    for ( i = 0; i < list_band->Count; i++ ) {
        box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        /* 同じファイルがあれば問題なし */
    	if ( fln == *box->fln ) {
        	return;
        }
    }

    /* 同一のものがなかったので閉じる */
    for ( i = 0; i < list_file->Count; i++ ) {
    	fc = (struct REMOS_FILE_CONTAINER *)list_file->Items[i];

        fc_fln = fc->file_name;

    	if ( fc_fln == fln ) {
    		remos_close( fc );
        	free( fc );
        }

		list_file->Delete( i );

        /* DEBUG : CanvasモードだったらBitmap破棄 */
        delete bmp;
    }

    /* 全部閉じた */
    if ( list_file->Count == 0 ) {
    	// DEBUG : ボタンを消さなければならない関係から、削除コードをここにコピー ( 関数にすべき )

        /* キャンバスクリア */
        SatImage->Canvas->Brush->Color = clWhite;
        SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        /* フラグ解除 */
        b_open = false;

       	clearSetting();
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::UpdateBandBox( struct REMOS_FRONT_BAND *box )
{
	/* ボックス内部値更新 */
    char num[2];

    /* 自分のインデックス再取得 */
    box->index = list_band->IndexOf( box );

    /* バンド番号作成 */
    num[0] = 0x61 + box->index;
    num[1] = '\0';

    /* 名前書きなおし */
    box->label_name->Caption = num;
    box->label_name->Caption = box->label_name->Caption + " : " + *box->fln;
    box->label_name->Caption = box->label_name->Caption + " - " + IntToStr( box->band->band_num );

    /* Canvasモードか、remosモードでもカラーなら色を書く */
    if ( box->canvas_mode || box->band->color == REMOS_BAND_COLOR_RGB ) {
    	switch ( box->band->band_num ) {
        	case 0:
            	box->label_name->Caption = box->label_name->Caption + " - R";
                box->label_name->Font->Color = clRed;
                break;

        	case 1:
            	box->label_name->Caption = box->label_name->Caption + " - G";
                box->label_name->Font->Color = clGreen;
                break;

        	case 2:
            	box->label_name->Caption = box->label_name->Caption + " - B";
                box->label_name->Font->Color = clBlue;
                break;
        }
    }

    /* Hint設定 */
    box->label_name->Hint = box->label_name->Caption;

    /* レンジ読み込み */
    box->edit_bottom->Text = FloatToStr( box->band->range_bottom );
    box->edit_top->Text    = FloatToStr( box->band->range_top );

    /* ヒストグラム描画 */
    DrawHist( box );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* ヒストグラムクリックイベント */
    struct REMOS_FRONT_BAND *box;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    box->hist_click  = true;
    box->hist_button = Button;

    HistMouseMove( Sender, Shift, X, Y );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* ヒストグラムクリックイベント */
    struct REMOS_FRONT_BAND *box;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    box->hist_click = false;

    /* 更新 */
    DrawHist( box );

    /* 描画させる */
    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::HistMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	/* ヒストグラムポインタ移動イベント */
    int w, h;
    int y;
    int i;
    float val;
    struct REMOS_FRONT_BAND *box;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    if ( X < 0 ) {
    	X = 0;
    } else if ( X > 255 ) {
    	X = 255;
    }

    /* クリックされてなかったらスキップ */
    if ( !box->hist_click ) {
    	return;
    }

    /* Xの位置から値作成 */
    val = X / 255.0 * ( box->band->range_max - box->band->range_min ) + box->band->range_min;

    /* レンジ設定 */
    if ( box->hist_button == mbLeft ) {
    	if ( val < box->band->range_top ) {
            box->band->range_bottom = val;
        } else {
            box->band->range_bottom = box->band->range_top;

            X = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
        }
    } else if ( box->hist_button == mbRight ) {
    	if ( val > box->band->range_bottom ) {
        	box->band->range_top = val;
        } else {
            box->band->range_top = box->band->range_bottom;

            X = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
        }
    }

    /* 値をエディットに読み込む */
    box->edit_bottom->Text = FloatToStr( box->band->range_bottom );
    box->edit_top->Text    = FloatToStr( box->band->range_top );

    /* 数字を描く */
    box->img_hist->Canvas->Font->Color = clBlack;
    box->img_hist->Canvas->TextOutA( 2 + X, 2, IntToStr( box->band->hist[X] ) );
}
//---------------------------------------------------------------------------
void TSatViewMainForm::DrawHist( struct REMOS_FRONT_BAND *box )
{
	/* ヒストグラム描画 */
    int h;
    int y;
    int i;
    int pos_l;
    int pos_r;
    double max;

    h = box->img_hist->Height;

    /* 0除算防止 */
    if ( box->band->hist_max == 0 ) {
    	box->band->hist_max = 1;
    }

    if ( box->band->hist_max_reduce_topbottom == 0 ) {
    	box->band->hist_max_reduce_topbottom = 1;
    }

    /* 描画位置計算 */
    pos_l = ( box->band->range_bottom - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;
    pos_r = ( box->band->range_top - box->band->range_min ) / ( box->band->range_max - box->band->range_min ) * 255;

    /* ヒストグラム描画 */
    for ( i = 0; i <= pos_l; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 50, 50, 130 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 80, 80, 80 );
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    for ( i = pos_l; i <= pos_r; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 30, 30, 255 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = clWhite;
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    for ( i = pos_r; i < 256; i++ ) {
        if ( HistReduceMaxCheckBox->Checked ) {
            max = box->band->hist[i] / (double)box->band->hist_max_reduce_topbottom;
        } else {
            max = box->band->hist[i] / (double)box->band->hist_max;
        }

    	y = h * ( max );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 50, 50, 130 );
        box->img_hist->Canvas->MoveTo( i, h );
        box->img_hist->Canvas->LineTo( i, h - y );

        box->img_hist->Canvas->Pen->Color = (TColor)RGB( 80, 80, 80 );
        box->img_hist->Canvas->MoveTo( i, h - y );
        box->img_hist->Canvas->LineTo( i, 0 );
    }

    /* ヒストグラムが作られてなければ警告 */
    if ( !box->hist_maked ) {
        box->img_hist->Canvas->Font->Size   = 9;
        box->img_hist->Canvas->Font->Color  = clRed;
        box->img_hist->Canvas->Brush->Color = clWhite;
        box->img_hist->Canvas->TextOutA( 1, 1, "ヒストグラムが作成されていません" );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandEditUpChange(TObject *Sender)
{
	/* ヒストグラム変更 */
    struct REMOS_FRONT_BAND *box;
    float val;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* 現在の値 */
    try {
    	val = box->edit_top->Text.ToDouble();
    } catch( ... ) {
    	// 数字でなければ何もしない
        return;
    }

    /* オーバーしていれば最小と等しくする */
    if ( box->band->range_bottom > val ) {
    	val = box->band->range_bottom;

        // エディットはいじらない
        // box->edit_top->Text = FloatToStr( val );
    }

    /* 値更新 */
    box->band->range_top = val;

    /* 画面更新 */
    DrawHist( box );

	PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BandEditDownChange(TObject *Sender)
{
	/* ヒストグラム変更 */
    struct REMOS_FRONT_BAND *box;
    float val;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* 現在の値 */
    try {
    	val = box->edit_bottom->Text.ToDouble();
    } catch( ... ) {
    	// 数字でなければ何もしない
        return;
    }

    /* オーバーしていれば最大と等しくする */
    if ( box->band->range_top < val ) {
    	val = box->band->range_top;

        // エディットはいじらない
    	// box->edit_bottom->Text  = FloatToStr( val );
    }

    /* 値更新 */
    box->band->range_bottom = val;

    /* 画面更新 */
    DrawHist( box );

	PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::PleaseClick( void )
{
    /* 更新メッセージ */
    AnsiString str;

    str = "再描画してください";
    SatImage->Canvas->Font->Size   = 9;
    SatImage->Canvas->Font->Color  = clRed;
    SatImage->Canvas->Brush->Color = clWhite;
    SatImage->Canvas->TextOutA( 10, 10, str );
}
//---------------------------------------------------------------------------
AnsiString TSatViewMainForm::GetLon( int x, int y )
{
	/* 経度を文字で */
    AnsiString ret;
    double lon;
    double lat;
    double utm_n;
    double utm_e;

    // UTMモードであればUTMで計算する
    if ( b_config_utm ) {
    	lon = config_utm.getLatLon( Point2D( x, y ) ).x;
    } else {
    	// もしだいちならだいち用の設定を使う
        if ( b_config_alos ) {
        	lon = conf_alos.getLon( x, y );
        } else {
        	// LatLonモード
        	lon = conf_lt_lon + ( ( conf_rt_lon - conf_lt_lon ) / ( img_w - 1 ) ) * x + ( ( conf_lb_lon - conf_lt_lon ) / ( img_h - 1 ) ) * y + ( ( conf_rb_lon - conf_lb_lon - conf_rt_lon + conf_lt_lon ) / ( ( img_w - 1 ) * ( img_h - 1 ) ) ) * ( x * y );
        }
    }

    ret = ret.sprintf( "%.4lf", lon );

    return ret;
}
//---------------------------------------------------------------------------
AnsiString TSatViewMainForm::GetLat( int x, int y )
{
	/* 緯度を文字で */
    AnsiString ret;
    double lat;
    double lon;
    double utm_n;
    double utm_e;

    // UTMモードであればUTMで計算する
    if ( b_config_utm ) {
    	lat = config_utm.getLatLon( Point2D( x, y ) ).y;
    } else {
    	// もしだいちならだいち用の設定を使う
        if ( b_config_alos ) {
        	lat = conf_alos.getLat( x, y );
        } else {
        	// LatLonモード
        	lat = conf_lt_lat + ( ( conf_rt_lat - conf_lt_lat ) / ( img_w - 1 ) ) * x + ( ( conf_lb_lat - conf_lt_lat ) / ( img_h - 1 ) ) * y + ( ( conf_rb_lat - conf_lb_lat - conf_rt_lat + conf_lt_lat ) / ( ( img_w - 1 ) * ( img_h - 1 ) ) ) * ( x * y );
        }
    }

    ret = ret.sprintf( "%.4lf", lat );

    return ret;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::SatImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int vert;
    int hor;
    int i;
    int cent_c_x;
    int cent_c_y;
    int line_x;
    int line_y;
    float val;
    struct REMOS_FRONT_BAND *box;
    TColor cl;
    AnsiString str;
    char band[2];
    double lon;
    double lat;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];


    /* ひらいてなければなにもしない */
    if ( !b_open ) {
    	return;
    }

    // 式用ポインター登録
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
    }

    /* カーソル移動 */
    b_move = true;

    sc_h = SatImage->Height;
    sc_w = SatImage->Width;

	/* クリックされている？ */
	if ( b_click ) {
    	// 距離測定モードか
        if ( b_mode_length ) {
        	// 距離測定モード

        	// バッファ描画
            SatImage->Canvas->CopyRect( Rect( 0, 0, img_back->Width, img_back->Height ), img_back->Canvas, Rect( 0, 0, img_back->Width, img_back->Height ) );

            // 線描画
            SatImage->Canvas->Pen->Color = clRed;
            SatImage->Canvas->Pen->Width = 5;
            SatImage->Canvas->MoveTo( cp_x, cp_y );
            SatImage->Canvas->LineTo( X, Y );

            // 倍率計算
            double real_mag;

            if ( zoom_pos > ZOOM_MAX / 2 ) {
                /* 縮小 */
                real_mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
            } else if ( zoom_pos < ZOOM_MAX / 2 ) {
                /* 拡大 */
                real_mag = Power( 2, ZOOM_MAX / 2 - zoom_pos );
            } else {
                /* 等倍 */
                real_mag = 1;
            }

            // 距離描画
            double length = sqrt( ( X - cp_x ) * ( X - cp_x ) * conf_resolution_x * conf_resolution_x / ( real_mag * real_mag ) + ( Y - cp_y ) * ( Y - cp_y ) * conf_resolution_y * conf_resolution_y / ( real_mag * real_mag ) );

            SatImage->Canvas->Font->Color = clRed;
            SatImage->Canvas->TextOutA( X + 5, Y - 5 - SatImage->Canvas->TextHeight( "height" ), IntToStr( (int)length ) + "[m]" );
        } else if ( b_mode_stamp_cb ) {
        	// スタンプモードならなにもしない
        } else {
        	// 移動モード
            SatImage->Canvas->Pen->Width = 1;

            /* 画像移動 */
            ScrImgHor->Position  = cp_hor  + ( cp_x - X ) / mag;
            ScrImgVert->Position = cp_vert + ( cp_y - Y ) / mag;

            vert = ScrImgVert->Position;
            hor  = ScrImgHor->Position;

            /* 簡易描画 */
            SatImage->Canvas->Brush->Color = clWhite;
            SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
            SatImage->Canvas->CopyRect( Rect( - 1 * ( cp_x - X ), - 1 * ( cp_y - Y ), SatImage->Width - ( ( cp_x - X ) ), SatImage->Height - ( ( cp_y - Y ) ) ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            /* ミニマップ描画 */
            mm_s_x = 100 * ( hor / (float)img_w );
            mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );

            SatImage->Canvas->Pen->Color   = clRed;
            SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
            SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
            SatImage->Canvas->Brush->Color = clRed;

            SatImage->Canvas->MoveTo( mm_s_x, 0 );
            SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
            SatImage->Canvas->MoveTo( 0, mm_s_y );
            SatImage->Canvas->LineTo( 100, mm_s_y );

            /* 十字線 補正項 : line_add_ : DEBUG */
            SatImage->Canvas->MoveTo( sc_w / 2 - line_add_x, 0 );
            SatImage->Canvas->LineTo( sc_w / 2 - line_add_x, sc_h );
            SatImage->Canvas->MoveTo( 0, sc_h / 2 - line_add_y );
            SatImage->Canvas->LineTo( sc_w, sc_h / 2 - line_add_y );

            str = "X, Y : " + IntToStr( hor ) + ", " + IntToStr( vert );

            SatImage->Canvas->Font->Size = 9;
            SatImage->Canvas->Font->Color = clRed;
            SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
            SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 - line_add_y + 5, str );

            /* 経度緯度を計算出来れば表示 */
            if ( b_config ) {
                str = "経度, 緯度 :  " + GetLon( hor, vert ) + ", " + GetLat( hor, vert );
                SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 - line_add_y + 5 + SatImage->Canvas->TextHeight( str ), str );
            }

            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

                /* モードで取得方法が違う */
                if ( box->canvas_mode ) {
                    val = GetPixel( box, img_w * vert + hor  );
                } else {
                    val = remos_get_pixel( box->band, img_w * vert + hor );
                }

                band[0] = tolower( (char)( box->index + 0x41 ) );
                band[1] = '\0';

                str = band;
                str = str + " : " + IntToStr( (int)val );

                SatImage->Canvas->Font->Size = 9;
                SatImage->Canvas->Font->Color = clRed;
                SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
                SatImage->Canvas->TextOutA( sc_w / 2 - line_add_x + 5, sc_h / 2 + 5 + ( i + 2 ) * SatImage->Canvas->TextHeight( str ) - line_add_y, str );
            }
        }

    } else {
    	/* 座標値更新 */
        cl = SatImage->Canvas->Pixels[X][Y];

        r = ( 0x0000FF & cl );
        g = ( 0x00FF00 & cl ) >> 8;
        b = ( 0xFF0000 & cl ) >> 16;

        str = "";
        str = str + "( R, G, B ) = " + "( " + IntToStr( r ) + ", "
    	                    + IntToStr( g ) + ", " + IntToStr( b ) + " )";

    	StatusBar->Panels->Items[2]->Text = str;

        /* カーソル位置算定 */
        vert = ScrImgVert->Position;
        hor  = ScrImgHor->Position;

        /* トライアンドエラーでこうなった */
        cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
        cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

        if ( cent_c_x < 0 ) {
            cent_c_x = 0;
        }

        if ( cent_c_y < 0 ) {
            cent_c_y = 0;
        }

        if ( cent_c_y >= img_h ) {
            cent_c_y = img_h - 1;
        }

        if ( cent_c_x >= img_w ) {
            cent_c_x = img_w - 1;
        }

        // 経度緯度表示
        if ( b_config ) {
        	StatusBar->Panels->Items[3]->Text = "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + " " + "経度, 緯度 : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y );
        }

        /* カーソル位置フォームに表示 */
        if ( PixForm->Visible ) {
            // cent_c_x = img_start_x_line + X / mag;
            // cent_c_y = img_start_y_line + Y / mag;

        	str = "";
            str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

            // 経度緯度表示
            if ( b_config ) {
            	str = str + "経度, 緯度 : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
			}

			/* 各バンド生データ取得 */
            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            	/* モードで取得方法が違う */
            	if ( box->canvas_mode ) {
              		val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            	} else {
            		val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            	}

                band[0] = tolower( (char)( box->index + 0x41 ) );
                band[1] = '\0';

                str = str + band;
                str = str + " : " + IntToStr( (int)val ) + "\n";
        	}

        	PixForm->ValueLabel->Caption = str;

            if ( PixForm->ValueLabel->Height + 30 >PixForm->Height ) {
            	PixForm->Height = PixForm->ValueLabel->Height + 30;
            }
        }

        // カラーバー表示
        if ( ColorBarForm->Visible || b_mode_stamp_cb ) {
        	/* カーソル位置算定 */
        	vert = ScrImgVert->Position;
    		hor  = ScrImgHor->Position;

            /* トライアンドエラーでこうなった */
            cent_c_x = hor + ( X + line_add_x - SatImage->Width / 2 ) / mag;
            cent_c_y = vert + ( Y + line_add_y - SatImage->Height / 2 ) / mag;

            if ( cent_c_x < 0 ) {
            	cent_c_x = 0;
            }

            if ( cent_c_y < 0 ) {
            	cent_c_y = 0;
            }

            // 式作成
            ecalc_free_token( tok_color_bar );
    		tok_color_bar = ecalc_make_token( color_bar_exp.c_str() );
			tok_color_bar = ecalc_make_tree( tok_color_bar );

			/* 各バンド生データ取得 */
            for ( i = 0; i < list_band->Count; i++ ) {
                box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

            	/* モードで取得方法が違う */
            	if ( box->canvas_mode ) {
              		val = GetPixel( box, img_w * cent_c_y + cent_c_x );
            	} else {
            		val = remos_get_pixel( box->band, img_w * cent_c_y + cent_c_x );
            	}

            	// 式変数に登録
                var[i] = val;
        	}

            // カラーマップが有効であればカラーマップの現在値計算
            if ( color_map != NULL ) {
            	color_map->evalExpression( vars, 0 );
            }

            // カラーバーのモードによって値の送り方を決定
            if ( ColorBarForm->getExpMode() ) {
            	// 式から取得した値を設定し再描画
                ColorBarForm->setNowValue( ecalc_get_tree_value( tok_color_bar, vars, 0 ) );
                ColorBarForm->draw();
            } else {
            	// カラーマップから取得した値を設定し再描画
            	ColorBarForm->setNowValue( color_map->getValue() );
            	ColorBarForm->draw();
            }
        }

        // カラーバースタンプモード
        if ( b_mode_stamp_cb ) {
        	// バッファコピー
        	SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

            // 可能なら座標を書く
        	str = "";
            // str = str + "X, Y : " + IntToStr( cent_c_x ) + ", " + IntToStr( cent_c_y ) + "\n";

            if ( b_config ) {
            	str = str + "経度, 緯度 : " + GetLon( cent_c_x, cent_c_y ) + ", " + GetLat( cent_c_x, cent_c_y ) + "\n\n";
			}

            int size_buf = SatImage->Canvas->Font->Size;
            SatImage->Canvas->Font->Size = 10;
            SatImage->Canvas->TextOutA( X + 10, Y + 10 + ColorBarForm->ColorBarImage->Height, str );
            SatImage->Canvas->Font->Size = size_buf;

            // 十字を書く
            SatImage->Canvas->Pen->Color = clRed;
            SatImage->Canvas->Pen->Width = 2;
            SatImage->Canvas->MoveTo( X - 10, Y );
            SatImage->Canvas->LineTo( X - 5,  Y );
            SatImage->Canvas->MoveTo( X + 10, Y );
            SatImage->Canvas->LineTo( X + 5,  Y );
            SatImage->Canvas->MoveTo( X, Y - 10 );
            SatImage->Canvas->LineTo( X, Y -  5 );
            SatImage->Canvas->MoveTo( X, Y + 10 );
            SatImage->Canvas->LineTo( X, Y +  5 );

            // カラーバー転送
            SatImage->Canvas->CopyRect( Rect( X + 10, Y + 10, X + 10 + ColorBarForm->ColorBarImage->Width, Y + 10 + ColorBarForm->ColorBarImage->Height ), ColorBarForm->ColorBarImage->Canvas, ColorBarForm->ColorBarImage->Canvas->ClipRect );

        }

		/* バッファをコピー */
		// SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ImageMouseWheelEvent( TObject *Sender, TShiftState Shift,
	int WheelDelta, const TPoint &MousePos, bool &Handled )
{
	// Image用のホイールイベント

    // WheelDaltaの符号で向きを判断
    if ( WheelDelta >= 1 ) {
    	// ズームアップ
    	zoom_pos--;

    	if ( zoom_pos < 0 ) {
    		zoom_pos = 0;
    	}

    	DrawZoomBox( 0, 0 );
		DrawImg();
    } else if ( WheelDelta <= -1 ) {
        // ズームダウン
        zoom_pos++;

    	if ( zoom_pos > ZOOM_MAX ) {
    		zoom_pos = ZOOM_MAX;
    	}

    	DrawZoomBox( 0, 0 );
		DrawImg();
    }

    // 処理した
    Handled = true;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ScrImgHorScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int mm_e_x;
    int mm_e_y;
    int vert;
    int hor;
    AnsiString str;

    /* ひらいてなければなにもしない */
    if ( !b_open ) {
    	return;
    }

	if ( ScrollCode == scEndScroll || ScrCheckBox->Checked == true ) {
        /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
        if ( b_sync ) {
        	PostMessage( HWND_BROADCAST, umsg_x, ScrImgHor->Position, 0 );
            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    } else if ( ScrollCode == scTrack || ScrollCode == scLineUp || ScrollCode == scLineDown || ScrollCode == scPageDown || ScrollCode == scPageUp ) {
    	/* ミニマップ描画 */
        sc_h = SatImage->Height;
        sc_w = SatImage->Width;
        vert = ScrImgVert->Position;
        hor  = ScrollPos;

        mm_s_x = 100 * ( hor / (float)img_w );
        mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );
        mm_e_x = mm_s_x + 1;
        mm_e_y = mm_s_y + 1;

        SatImage->Canvas->Pen->Width   = 1;
        SatImage->Canvas->Pen->Color   = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->Brush->Color = clRed;
        SatImage->Canvas->Rectangle( Rect( mm_s_x, mm_s_y, mm_e_x, mm_e_y ) );
        SatImage->Canvas->MoveTo( mm_s_x, 0 );
        SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->MoveTo( 0, mm_s_y );
        SatImage->Canvas->LineTo( 100, mm_s_y );

        SatImage->Canvas->MoveTo( sc_w / 2, 0 );
        SatImage->Canvas->LineTo( sc_w / 2, sc_h );
        SatImage->Canvas->MoveTo( 0, sc_h / 2 );
        SatImage->Canvas->LineTo( sc_w, sc_h / 2 );

        str = IntToStr( hor ) + ", " + IntToStr( vert ) + "          ";

        SatImage->Canvas->Font->Size = 9;
        SatImage->Canvas->Font->Color = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->TextOutA( sc_w / 2 + 5, sc_h / 2 + 5, str );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ScrImgVertScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
	int r;
    int g;
    int b;
    int sc_h;
    int sc_w;
    int mm_s_x;
    int mm_s_y;
    int mm_e_x;
    int mm_e_y;
    int vert;
    int hor;
    AnsiString str;

    /* ひらいてなければなにもしない */
    if ( !b_open ) {
    	return;
    }

	if ( ScrollCode == scEndScroll || ScrCheckBox->Checked == true ) {
        /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
        if ( b_sync ) {
        	PostMessage( HWND_BROADCAST, umsg_y, ScrImgVert->Position, 0 );
            PostMessage( HWND_BROADCAST, umsg_draw, ScrImgVert->Position, 0 );
        } else {
        	DrawImg();
        }
    } else if ( ScrollCode == scTrack || ScrollCode == scLineUp || ScrollCode == scLineDown || ScrollCode == scPageDown || ScrollCode == scPageUp ) {
    	/* ミニマップ描画 */
        sc_h = SatImage->Height;
        sc_w = SatImage->Width;
        vert = ScrollPos;
        hor  = ScrImgHor->Position;

        mm_s_x = 100 * ( hor / (float)img_w );
        mm_s_y = ( ( img_h / (float)img_w ) * 100 ) * ( vert / (float)img_h );
        mm_e_x = mm_s_x + 1;
        mm_e_y = mm_s_y + 1;

        SatImage->Canvas->Pen->Width   = 1;
        SatImage->Canvas->Pen->Color   = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->Rectangle( 0, 0, 100, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->Brush->Color = clRed;
        SatImage->Canvas->Rectangle( Rect( mm_s_x, mm_s_y, mm_e_x, mm_e_y ) );
        SatImage->Canvas->MoveTo( mm_s_x, 0 );
        SatImage->Canvas->LineTo( mm_s_x, ( img_h / (float)img_w ) * 100 );
        SatImage->Canvas->MoveTo( 0, mm_s_y );
        SatImage->Canvas->LineTo( 100, mm_s_y );

        SatImage->Canvas->MoveTo( sc_w / 2, 0 );
        SatImage->Canvas->LineTo( sc_w / 2, sc_h );
        SatImage->Canvas->MoveTo( 0, sc_h / 2 );
        SatImage->Canvas->LineTo( sc_w, sc_h / 2 );

        str = IntToStr( hor ) + ", " + IntToStr( vert ) + "          ";

        SatImage->Canvas->Font->Size = 9;
        SatImage->Canvas->Font->Color = clRed;
        SatImage->Canvas->Brush->Color = (TColor)RGB( 255, 200, 200 );
        SatImage->Canvas->TextOutA( sc_w / 2 + 5, sc_h / 2 + 5, str );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditRChange(TObject *Sender)
{
	/* メモリ解放 */
	ecalc_free_token( tok_r );

	/* 式を更新する */
    tok_r = ecalc_make_token( ExpEditR->Text.c_str() );

	/* ツリーに変更 */
	tok_r = ecalc_make_tree( tok_r );

    if ( tok_r == NULL ) {
    	ExpEditR->Color = clBlack;
    } else {
    	ExpEditR->Color = clRed;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditBChange(TObject *Sender)
{
	/* メモリ解放 */
	ecalc_free_token( tok_b );

	/* 式を更新する */
    tok_b = ecalc_make_token( ExpEditB->Text.c_str() );

	/* ツリーに変更 */
	tok_b = ecalc_make_tree( tok_b );

    if ( tok_b == NULL ) {
    	ExpEditB->Color = clBlack;
    } else {
    	ExpEditB->Color = clBlue;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ExpEditGChange(TObject *Sender)
{
	/* メモリ解放 */
	ecalc_free_token( tok_g );

	/* 式を更新する */
    tok_g = ecalc_make_token( ExpEditG->Text.c_str() );

	/* ツリーに変更 */
	tok_g = ecalc_make_tree( tok_g );

    if ( tok_g == NULL ) {
    	ExpEditG->Color = clBlack;
    } else {
    	ExpEditG->Color = clGreen;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BlueBtnClick(TObject *Sender)
{
	/* 自分を青に */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditB->Text = band;

    // 必要なら式モードへ
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::GreenBtnClick(TObject *Sender)
{
	/* 自分を緑に */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditG->Text = band;

    // 必要なら式モードへ
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::RedBtnClick(TObject *Sender)
{
	/* 自分を赤に */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditR->Text = band;

    // 必要なら式モードへ
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::WBBtnClick(TObject *Sender)
{
	/* 白黒に */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    band[0] = tolower( (char)( box->index + 0x41 ) );
    band[1] = '\0';

    ExpEditR->Text = band;
    ExpEditG->Text = band;
	ExpEditB->Text = band;

    // 必要なら式モードへ
    if ( !ExpDrawRadioButton->Checked ) {
    	ExpDrawRadioButton->Checked = true;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MaxBtnClick(TObject *Sender)
{
	/* レンジ設定解除 */
    struct REMOS_FRONT_BAND *box;
    char band[2];

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* 最大最小に */
    box->band->range_bottom = box->band->range_min;
    box->band->range_top    = box->band->range_max;

    /* 更新 */
    UpdateBandBox( box );

    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::AutoBtnClick(TObject *Sender)
{
	/* オートレンジ */
    struct REMOS_FRONT_BAND *box;
    double range;
    int ret;

    /* BNAD特定 */
    box = (struct REMOS_FRONT_BAND *)( ( (TBitBtn *)Sender )->Parent->Tag );

    /* ヒストグラム生成済み？ */
    if ( !box->hist_maked ) {
    	ret = Application->MessageBoxA( "ヒストグラムが作成されていません。作成しますか？", "確認", MB_YESNO | MB_ICONINFORMATION );

    	if ( ret == IDYES ) {
        	box->hist_maked = true;
        	remos_make_hist( box->band );
        }
    }

    range = AutoRangeEdit->Text.ToDouble();

    /* オートレンジ適用 */
    if ( AutoRangeCheckBox->Checked ) {
    	/* 上下を除く */
        remos_calc_auto_range( box->band, range, 1 );
    } else {
    	/* のぞかない */
    	remos_calc_auto_range( box->band, range, 0 );
    }

    /* 更新 */
    UpdateBandBox( box );

    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
	TPoint points[3];
    int skip;
    int cent_y;
    int track_pos;
    int i;
    int zoom_change;
    double zoom_mag;

    /* 簡易拡大 */
    if ( !b_zoom_click ) {
    	return;
    }

    /* 範囲調整 */
    if ( X < 0 ) {
    	X = 0;
    }

    if ( X > ZoomImage->Width ) {
    	X = ZoomImage->Width - 1;
    }

    if ( Y < 0 ) {
    	Y = 0;
    }

    if ( Y > ZoomImage->Height ) {
    	Y = ZoomImage->Height - 1;
    }

    skip = ( ZoomImage->Height - 30 ) / ZOOM_MAX;

    /* ズーム調整 */
    zoom_pos = ( Y - 15.0 + skip / 2 ) / ( ZoomImage->Height - 30 ) * ZOOM_MAX;

    if ( zoom_pos < 0 ) {
       	zoom_pos = 0;
    } else if ( zoom_pos > ZOOM_MAX ) {
    	zoom_pos = ZOOM_MAX;
    }

    /* 倍率設定 */
    if ( zoom_pos > ZOOM_MAX / 2 ) {
    	/* 縮小 */
        mag  = 1.0 / Power( 2, zoom_pos - ZOOM_MAX / 2 );
    } else if ( zoom_pos < ZOOM_MAX / 2 ) {
    	/* 拡大 */
    	mag  = Power( 2, ZOOM_MAX / 2 - zoom_pos );
    } else {
    	/* 等倍 */
    	mag  = 1;
    }

    /* 描画 */
    DrawZoomBox( X, Y );

    zoom_change = zoom_pos_click - zoom_pos + ZOOM_MAX / 2;

    /* 倍率設定 */
    if ( zoom_change > ( ZOOM_MAX / 2 ) ) {
    	/* 縮小 */
        zoom_mag  = 1.0 / Power( 2, zoom_change - ( ZOOM_MAX / 2 ) );
    } else if ( zoom_change < ( ZOOM_MAX / 2 ) ) {
    	/* 拡大 */
    	zoom_mag  = Power( 2, ( ZOOM_MAX / 2 ) - zoom_change );
    } else {
    	/* 等倍 */
    	zoom_mag  = 1;
    }

    /* 必要なら描画 */
    if ( zoom_pos_bef != zoom_pos ) {
    	SatImage->Canvas->Brush->Color = clWhite;
    	SatImage->Canvas->FillRect( Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    	SatImage->Canvas->CopyRect( Rect( SatImage->Width / 2 - SatImage->Width / 2 / zoom_mag,
    	                            	SatImage->Height / 2 - SatImage->Height / 2 / zoom_mag,
    	                                SatImage->Width / 2 + SatImage->Width / 2 / zoom_mag,
    	                                SatImage->Height / 2 + SatImage->Height / 2 / zoom_mag ),
    	                            img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );
    }

    /* ズーム位置保存 */
    zoom_pos_bef = zoom_pos;

}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DrawZoomBox( int X, int Y )
{
	TPoint points[3];
    int skip;
    int cent_y;
    int track_pos;
    int i;
    int zoom_change;
    double zoom_mag;

    /* 範囲調整 */
    if ( X < 0 ) {
    	X = 0;
    }

    if ( X > ZoomImage->Width ) {
    	X = ZoomImage->Width - 1;
    }

    if ( Y < 0 ) {
    	Y = 0;
    }

    if ( Y > ZoomImage->Height ) {
    	Y = ZoomImage->Height - 1;
    }

    skip = ( ZoomImage->Height - 30 ) / ZOOM_MAX;

    track_pos = 15 + zoom_pos * skip;

    ZoomImage->Picture->Bitmap->Width = ZoomImage->Width;
    ZoomImage->Picture->Bitmap->Height = ZoomImage->Height;

    points[0] = Point( 12, 5 );
    points[1] = Point( ZoomImage->Width - 12, 5 );
    points[2] = Point( ZoomImage->Width / 2, ZoomImage->Height - 5 );

	ZoomImage->Canvas->Brush->Color = clWhite;
    ZoomImage->Canvas->Pen->Color = clBlue;
	ZoomImage->Canvas->Rectangle( Rect( 0, 0, ZoomImage->Width, ZoomImage->Height ) );

	ZoomImage->Canvas->Brush->Color = (TColor)RGB( 180, 180, 255 );
    ZoomImage->Canvas->Pen->Color = clBlue;
    ZoomImage->Canvas->Polygon( points, 3 );

    for ( i = 0; i < ZOOM_MAX + 1; i++ ) {
    	if ( i != ZOOM_MAX / 2 ) {
    		ZoomImage->Canvas->MoveTo( 10, 15 + i * skip );
    		ZoomImage->Canvas->LineTo( ZoomImage->Width - 10, 15 + i * skip );
        } else {
    		ZoomImage->Canvas->MoveTo( 0, 15 + i * skip );
    		ZoomImage->Canvas->LineTo( ZoomImage->Width, 15 + i * skip );
        }
    }

    /* ハコ描画 */

    /* クリックしてればなめらかに動かす */
    if ( b_zoom_click ) {
    	track_pos = Y;
    }

	ZoomImage->Canvas->Brush->Color = (TColor)RGB( 100, 100, 255 );
    ZoomImage->Canvas->Pen->Color = clBlue;

    ZoomImage->Canvas->Rectangle( 5, track_pos - 10, ZoomImage->Width - 5, track_pos + 10 );
    ZoomImage->Canvas->Pen->Color = (TColor)RGB( 200, 200, 255 );
    ZoomImage->Canvas->MoveTo( 5, track_pos - 10 );
    ZoomImage->Canvas->LineTo( ZoomImage->Width - 5, track_pos - 10 );
    ZoomImage->Canvas->MoveTo( 5, track_pos - 10 );
    ZoomImage->Canvas->LineTo( 5, track_pos + 10 );

    /* ギザギザ */
    for ( i = 0; i < 4; i++ ) {
    	ZoomImage->Canvas->Pen->Color = (TColor)RGB( 50, 50, 200 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) );
    	ZoomImage->Canvas->LineTo( ZoomImage->Width - 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) );

    	ZoomImage->Canvas->Pen->Color = (TColor)RGB( 190, 190, 255 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->LineTo( ZoomImage->Width - 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->MoveTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) - 1 );
    	ZoomImage->Canvas->LineTo( 8, track_pos - 10 + ( i + 1 ) * ( 20 / 5 ) + 1 );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ZoomImageMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if ( Button == mbLeft ) {
    	b_zoom_click = true;

        zoom_pos_click = zoom_pos;
        zoom_pos_bef   = zoom_pos;

        /* バッファ初期化 */
    	img_back->Width  = SatImage->Width;
   		img_back->Height = SatImage->Height;

    	img_back->PixelFormat = pf24bit;
    	img_back->HandleType = bmDIB;

        /* バッファにコピー */
        img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

        /* マウス移動イベント発生 */
        ZoomImageMouseMove( Sender, Shift, X, Y );
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	b_zoom_click = false;

    DrawZoomBox( X, Y );

    /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom同期外した */
    DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::FormShow(TObject *Sender)
{
    /* ズーム書く */
    DrawZoomBox( 0, 0 );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBSideClick(TObject *Sender)
{
	if ( !TBSide->Down ) {
    	BandPanel->Visible = false;
        MI_V_BAND->Checked = false;
    } else {
    	BandPanel->Visible = true;
        MI_V_BAND->Checked = true;
    }

    // 正しいサイズで再描画させるため
    Application->ProcessMessages();

    DrawImg();
    // PleaseClick();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_QUITClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_OPENClick(TObject *Sender)
{
	TBOpen->OnClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_SAVEClick(TObject *Sender)
{
	TBSave->OnClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_CLOSEClick(TObject *Sender)
{
	TBClose->OnClick( Sender );	
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BtnDrawClick(TObject *Sender)
{
	/* フラグが禁止でも強制的に描画 */

	if ( !b_draw ) {
    	b_draw = true;

    	DrawImg();

        b_draw = false;
    } else {
    	DrawImg();
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_BANDClick(TObject *Sender)
{
	/* バンドリストON/OFF */
    if ( BandPanel->Visible ) {
    	TBSide->Down = false;
        BandPanel->Visible = false;
    } else {
    	TBSide->Down = true;
        BandPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_TOOLClick(TObject *Sender)
{
	/* ツールバーON/OFF */
    if ( ButtonPanel->Visible ) {
        ButtonPanel->Visible = false;
    } else {
        ButtonPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::V1Click(TObject *Sender)
{
    if ( BandPanel->Visible == true ) {
        MI_V_BAND->Checked = true;

        if ( ConfigPanel->Visible == true ) {
        	MI_V_CONF->Checked = true;
        } else {
        	MI_V_CONF->Checked = false;
        }
    } else {
        MI_V_BAND->Checked = false;
        MI_V_CONF->Checked = false;
    }

    if ( ButtonPanel->Visible == true ) {
        MI_V_TOOL->Checked = true;
    } else {
        MI_V_TOOL->Checked = false;
    }

    if ( ZoomPanel->Visible == true ) {
        MI_V_ZOOM->Checked = true;
    } else {
        MI_V_ZOOM->Checked = false;
    }

    if ( StatusBar->Visible == true ) {
        MI_V_STATUS->Checked = true;
    } else {
        MI_V_STATUS->Checked = false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_V_ZOOMClick(TObject *Sender)
{
	/* ズームバーON/OFF */
    if ( ZoomPanel->Visible ) {
        ZoomPanel->Visible = false;
    } else {
        ZoomPanel->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_V_STATUSClick(TObject *Sender)
{
	/* ズームバーON/OFF */
    if ( StatusBar->Visible ) {
        StatusBar->Visible = false;
    } else {
        StatusBar->Visible = true;
    }

    PleaseClick();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomUpImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* ズームアップ */
    zoom_pos--;

    if ( zoom_pos < 0 ) {
    	zoom_pos = 0;
    }

    DrawZoomBox( X, Y );
    // ZoomImage->OnMouseMove( Sender, Shift, 0, 0 );

    /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom同期外した */
	DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::Image3MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	/* ズームダウン */
    zoom_pos++;

    if ( zoom_pos > ZOOM_MAX ) {
    	zoom_pos = ZOOM_MAX;
    }

    DrawZoomBox( 0, 0 );

    /* シンクロが必要ならシンクロする ( Syncでドローがかかるのでドローしない ) */
    /*
    if ( b_sync ) {
    	PostMessage( HWND_BROADCAST, umsg_mag, zoom_pos, 0 );
    } else {
       	DrawImg();
    }
    */

    /* Zoom同期外した */
	DrawImg();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_W_LTClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RTClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = 0;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_LBClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RBClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::ZoomPanelResize(TObject *Sender)
{
	DrawZoomBox( 0, 0 );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::DisplayHint(TObject *Sender)
{
    StatusBar->Panels->Items[0]->Text=GetLongHint(Application->Hint);
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_V_CONFClick(TObject *Sender)
{
	if ( BandPanel->Visible == false ) {
    	BandPanel->Visible   = true;
        ConfigPanel->Visible = true;
        TBSide->Down = true;

        PleaseClick();
    } else if ( ConfigPanel->Visible == true ) {
    	ConfigPanel->Visible = false;
    } else {
    	ConfigPanel->Visible = true;
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::MakeHist( struct REMOS_FRONT_BAND *box )
{
	/* Canvasモード用ヒストグラム作成 */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i, j;
    unsigned int max;

    max = 0;

    /* ポインタ短縮 */
    bmp = box->img_canvas;

	/* メモリクリア */
	for ( i = 0; i < 256; i++ ) {
		box->band->hist[i] = 0;
	}

    for ( i = 0; i < box->band->line_count; i++ ) {
    	rgb = (RGBTRIPLE *)bmp->ScanLine[i];

    	for ( j = 0; j < box->band->line_img_width; j++ ) {
            if ( box->band->band_num == 0 ) {
                box->band->hist[rgb[j].rgbtRed]++;
            }

            if ( box->band->band_num == 1 ) {
                box->band->hist[rgb[j].rgbtGreen]++;
            }

            if ( box->band->band_num == 2 ) {
                box->band->hist[rgb[j].rgbtBlue]++;
            }
        }
    }

	/* 最大値計測 */
	for ( i = 0; i < 256; i++ ) {
		if ( max < box->band->hist[i] ) {
			max = box->band->hist[i];
		}
	}

	/* 最大値保存 */
	box->band->hist_max = max;

	/* 最大値計測 */
	for ( i = 1, max = 0; i < 255; i++ ) {
		if ( max < box->band->hist[i] ) {
			max = box->band->hist[i];
		}
	}

	/* 最大値保存 */
	box->band->hist_max_reduce_topbottom = max;

    /* 0だと困るので値を入れておく */
    if ( box->band->hist_max_reduce_topbottom == 0 ) {
    	box->band->hist_max_reduce_topbottom = 1;
    }

    if ( box->band->hist_max == 0 ) {
    	box->band->hist_max = 1;
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::GetLineData( struct REMOS_FRONT_BAND *box, unsigned char *buf, int line, int from, int count )
{
	/* キャンバスモード用ゲットライン */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i;

    /* ポインタ短縮 */
    bmp = box->img_canvas;

    /* 範囲外にあれば強制範囲内 */
    if ( line >= box->band->line_count ) {
    	line = box->band->line_count - 1;
    }

    if ( box->band->line_img_width <= from ) {
    	from = box->band->line_img_width - 1;
    }

    if ( count + from >= box->band->line_img_width ) {
    	count = box->band->line_img_width - from;
    }

    /* 行ポインタ取得 */
    rgb = (RGBTRIPLE *)bmp->ScanLine[line];

    for ( i = from; i < count + from; i++ ) {
    	if ( box->band->band_num == 0 ) {
        	buf[i - from] = rgb[i].rgbtRed;
        }

    	if ( box->band->band_num == 1 ) {
        	buf[i - from] = rgb[i].rgbtGreen;
        }

    	if ( box->band->band_num == 2 ) {
        	buf[i - from] = rgb[i].rgbtBlue;
        }
    }
}
//---------------------------------------------------------------------------
unsigned char TSatViewMainForm::GetPixel( struct REMOS_FRONT_BAND *box, int pos )
{
	/* Canvasモードでの1pix取得関数 */
    Graphics::TBitmap *bmp;
    RGBTRIPLE *rgb;
    int i;
    int x;
    int line;

    /* 0除算対策 */
    if ( pos < 1 ) {
    	pos = 0;
    	x   = 0;
    } else {
    	x = pos % box->band->line_img_width % pos;
    }

    if ( pos >= box->band->line_img_width * box->band->line_count ) {
    	pos = box->band->line_img_width * box->band->line_count - 1;
    }

    /* ポインタ短縮 */
    bmp = box->img_canvas;

    /* 行ポインタ取得 */
    rgb = (RGBTRIPLE *)bmp->ScanLine[pos / box->band->line_img_width];

    if ( box->band->band_num == 0 ) {
        return rgb[x].rgbtRed;
    }

    if ( box->band->band_num == 1 ) {
        return rgb[x].rgbtGreen;
    }

    if ( box->band->band_num == 2 ) {
        return rgb[x].rgbtBlue;
    } else {
    	return 0;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_H_ABOUTClick(TObject *Sender)
{
	AboutForm->ShowModal();
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::HistReduceMaxCheckBoxClick(
      TObject *Sender)
{
	/* ヒストグラム更新 */
    int i;

    for ( i = 0; i < list_band->Count; i++ ) {
    	DrawHist( (struct REMOS_FRONT_BAND *)list_band->Items[i] );
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::UpDown3MouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	Application->MessageBoxA( "", "", MB_OK );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::FormResize(TObject *Sender)
{
	if ( b_open ) {
		PleaseClick();
    }
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBScrClick(TObject *Sender)
{
	/* スクロールを同期 */
	if ( b_sync ) {
        b_sync = false;
    } else {
        b_sync = true;
    }
}
//---------------------------------------------------------------------------





void __fastcall TSatViewMainForm::TBLtClick(TObject *Sender)
{
	MI_W_LTClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBRtClick(TObject *Sender)
{
	MI_W_RTClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBLbClick(TObject *Sender)
{
	MI_W_LBClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBRbClick(TObject *Sender)
{
	MI_W_RBClick( Sender );

}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_TClick(TObject *Sender)
{
	/* ウインドウを移動 */
    HWND h_wnd_desktop;
	TSize desktop;
    TRect rect;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

	h_wnd_desktop = GetDesktopWindow();
	GetWindowRect( h_wnd_desktop, &rect );

    /* 移動 */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_BClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = desktop.cy / 2;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx;
    SatViewMainForm->Height = desktop.cy / 2;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_LClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = 0;
    SatViewMainForm->Top  = 0;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_W_RClick(TObject *Sender)
{
	/* ウインドウを移動 */
	TSize desktop;

    /* デスクトップサイズ取得 */
    desktop.cx = GetSystemMetrics(SM_CXFULLSCREEN);
    desktop.cy = GetSystemMetrics(SM_CYFULLSCREEN);

    /* 移動 */
    SatViewMainForm->Left = desktop.cx / 2;
    SatViewMainForm->Top  = 0;

    /* サイズ変更 */
    SatViewMainForm->Width  = desktop.cx / 2;
    SatViewMainForm->Height = desktop.cy;
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton3Click(TObject *Sender)
{
	MI_W_TClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton5Click(TObject *Sender)
{
	MI_W_BClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton6Click(TObject *Sender)
{
	MI_W_LClick( Sender );	
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ToolButton9Click(TObject *Sender)
{
	MI_W_RClick( Sender );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::BtnColorClick(TObject *Sender)
{
    TPoint pos;
    pos.x = 0;
    pos.y = 0;
    pos = BtnColor->ClientToScreen( pos );
    pos.x += BtnColor->Width;
    ColorMenu->Popup( pos.x, pos.y );
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::TBUpdateClick(TObject *Sender)
{
	/* 描画許可 */
	if ( !TBUpdate->Down ) {
    	b_draw = false;
    } else {
    	b_draw = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::MI_C_NDVIClick(TObject *Sender)
{
	/* NDVIプリセットを表示 */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* ファイルが開かれて無ければ開かない */
    if ( !b_open ) {
    	return;
    }

    /* 初期設定 */
    PresetFormNDVI->NIRComboBox->Clear();
    PresetFormNDVI->RedComboBox->Clear();

    /* バンド登録 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormNDVI->NIRComboBox->Items->Add( box->label_name->Caption );
        PresetFormNDVI->RedComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormNDVI->NIRComboBox->ItemIndex = 0;
    PresetFormNDVI->RedComboBox->ItemIndex = 0;

    PresetFormNDVI->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormNDVI->Width / 2;
    PresetFormNDVI->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormNDVI->Height / 2;

	PresetFormNDVI->Show();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_TMClick(TObject *Sender)
{
	/* NDVIプリセットを表示 */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* ファイルが開かれて無ければ開かない */
    if ( !b_open ) {
    	return;
    }

    /* 初期設定 */
    PresetFormTHM->BandComboBox->Clear();
    PresetFormTHM->ExpComboBox->Clear();

    /* バンド登録 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormTHM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    /* 式登録 */
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditR->Text );
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditG->Text );
    PresetFormTHM->ExpComboBox->Items->Add( ExpEditB->Text );

    PresetFormTHM->BandComboBox->ItemIndex = 0;
    PresetFormTHM->ExpComboBox->ItemIndex = 0;

    PresetFormTHM->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormTHM->Width / 2;
    PresetFormTHM->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormTHM->Height / 2;

	PresetFormTHM->Show();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_LS_THMClick(TObject *Sender)
{
	/* LANDSAT_THMプリセットを表示 */
    int i;
    struct REMOS_FRONT_BAND *box;

    /* ファイルが開かれて無ければ開かない */
    if ( !b_open ) {
    	return;
    }

    /* 初期設定 */
    PresetFormLS_THM->BandComboBox->Clear();

    /* バンド登録 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormLS_THM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormLS_THM->BandComboBox->ItemIndex = 0;

    // LMAXとLMINが読み込めれば設定、読み込めなければデフォルト値
    if ( b_config_land ) {
    	PresetFormLS_THM->lmax_61 = land_lmax;
        PresetFormLS_THM->lmin_61 = land_lmin;

    	PresetFormLS_THM->lmax_62 = land_lmax_2;
        PresetFormLS_THM->lmin_62 = land_lmin_2;
    } else {
    	// デフォルト値
        PresetFormLS_THM->lmax_61 = 17.04;
        PresetFormLS_THM->lmin_61 = 0;

        PresetFormLS_THM->lmax_62 = 12.65;
        PresetFormLS_THM->lmin_62 = 3.2;
    }

    // 設定を読みこませる
    PresetFormLS_THM->readSetting();

    // 警告を出す
    if ( b_config_ls7 == false ) {
    	AnsiString msg;

        msg = "Landsat7の設定データーが読み込まれていません。\n温度表示を正確に行えない可能性があります。\n各設定値はデフォルト値が使用されます。";

    	Application->MessageBoxA( msg.c_str(), "情報", MB_ICONINFORMATION|MB_OK );
    }

    // 表示
    PresetFormLS_THM->Left = SatViewMainForm->Left + SatViewMainForm->Width / 2 - PresetFormTHM->Width / 2;
    PresetFormLS_THM->Top = SatViewMainForm->Top + SatViewMainForm->Height / 2 - PresetFormTHM->Height / 2;

	PresetFormLS_THM->Show();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::OnDropFiles( TWMDropFiles msg )
{
	/* ドロップイベント受け取り */
    char filename[256];												/* ファイル名バッファ */
    int files;														/* 受け取り数 */
    int i;
    TStringList *sl;

    sl = new TStringList();

    files = DragQueryFile( (HDROP)msg.Drop, 0xFFFFFFFF, NULL, 0 );	/* ファイル数を受け取る */

    /* OpenDialogを利用 */
    OpenDialog->Files->Clear();

	for ( i = 0; i < files; i++ ) {
    	DragQueryFile( (HDROP)msg.Drop, i, filename, 255 );			/* ファイル名受け取り */
        sl->Add( filename );
    }

    DragFinish( (HDROP)msg.Drop );									/* ドロップ完了 */

    sl->Sort();
    OpenDialog->Files->Clear();

    for ( i = 0; i < sl->Count; i++ ) {
    	OpenDialog->Files->Add( sl->Strings[i] );
    }

    /* 開く */
    OpenFiles();

    delete sl;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::AppMessage( tagMSG &msg, bool &handled )
{
	/* D&D用, メッセージ処理 */
    AnsiString dbg;

    if ( msg.message == WM_DROPFILES ) {
    	/* ドロップ指令だったら */
        TWMDropFiles msg_drop;

        msg_drop.Drop = msg.wParam;
        OnDropFiles( msg_drop );
        handled = msg_drop.Result;
    } else if ( b_sync ) {
    	// もし同期モードなら
        if ( msg.message == umsg_x ) {
            msg_buf_x = msg.wParam;
            msg_buf_mode = 0;
            // DrawImg();
        } else if ( msg.message == umsg_y ) {
            msg_buf_y = msg.wParam;
            msg_buf_mode = 0;
            // DrawImg();
        } else if ( msg.message == umsg_mag ) {
            zoom_pos = msg.wParam;
            DrawZoomBox( 0, 0 );
            DrawImg();
        } else if ( msg.message == umsg_lat ) {
            /* 緯度 経度では描画せず */
            msg_buf_mode = 1;
            msg_buf_lat = msg.wParam / 10000.0;
        } else if ( msg.message == umsg_lon ) {
            /* 経度 */
            msg_buf_mode = 1;
            msg_buf_lon = msg.wParam / 10000.0;
        } else if ( msg.message == umsg_draw ) {
            /* 描画指令 */
            /* DEBUG : 条件がおかしかった msg_buf_mode = 1 */
            if ( msg_buf_mode == 1 ) {
                /* 経度緯度モードだった */
                // dbg = "lat " + FloatToStr( msg_buf_lat ) + " lon " + FloatToStr( msg_buf_lon );
                // Application->MessageBoxA( dbg.c_str(), "", MB_OK );

                /* 経度緯度モードに対応出来れば経度緯度モードでリンク */
                if ( b_config ) {
                	// UTMモードならUTMで取得
                    if ( b_config_utm ) {
                    	msg_buf_x = config_utm.getXY( Point2D( msg_buf_lon, msg_buf_lat ) ).x;
                        msg_buf_y = config_utm.getXY( Point2D( msg_buf_lon, msg_buf_lat ) ).y;
                    } else {
                    	// だいちならだいち用の設定を使う
                        if ( b_config_alos ) {
                        	msg_buf_x = conf_alos.getI( msg_buf_lat, msg_buf_lon );
                            msg_buf_y = conf_alos.getJ( msg_buf_lat, msg_buf_lon );
                    	} else {
                        	remos_latlon_to_xy( &msg_buf_x, &msg_buf_y, img_w, img_h, msg_buf_lat, msg_buf_lon, conf_lt_lon, conf_lt_lat, conf_lb_lon, conf_lb_lat, conf_rt_lon, conf_rt_lat, conf_rb_lon, conf_rb_lat );
                        }
                	}
                } else {
                    /* 対応していない場合は何もしない */
                    msg_buf_x = ScrImgHor->Position;
                    msg_buf_y = ScrImgVert->Position;
                }
            }

            // dbg = "x " + IntToStr( msg_buf_x ) + " y " + IntToStr( msg_buf_y );
            // Application->MessageBoxA( dbg.c_str(), "", MB_OK );

            ScrImgHor->Position  = msg_buf_x;
            ScrImgVert->Position = msg_buf_y;

            DrawImg();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::OpenFiles( void )
{
	/* ファイル開く関数 */
	/* ファイルを開く */
    struct REMOS_FILE_CONTAINER *remos;
    struct REMOS_FRONT_BAND *box;
    struct ECONF_FILE_CONTAINER ef;
    bool b_hist_calc;
    bool b_new_open = false;
    Graphics::TBitmap *img_bmp;
    TJPEGImage *img_jpg;
    int ret;
    AnsiString msg, fln, str_buf;
    AnsiString fln_0, fln_1, fln_2;
    char num[2];
    TSearchRec sr;
    TSearchRec sr_conf, sr_conf_2;

    /* メッセージ表示 */
    StatusForm->MessageLabel->Caption = "画像を読み込んでいます";
    StatusForm->Show();
    Application->ProcessMessages();

    for ( int i = 0; i < OpenDialog->Files->Count; i++ ) {
        /* 対象ファイルが大きすぎて、かつヒストグラムが必要な場合に警告 */
        FindFirst( OpenDialog->Files->Strings[i], faAnyFile, sr );

        if ( HistCheckBox->Checked ) {
            b_hist_calc = true;
        } else {
            b_hist_calc = false;
        }

        if ( ( sr.Size > 100000000 ) && ( b_hist_calc == true ) ) {
            if ( Application->MessageBoxA( "ファイルサイズが100MBを超えているためヒストグラム作成に時間がかかる可能性があります。\nヒストグラムを作成しますか？", "情報", MB_ICONINFORMATION|MB_YESNO ) == IDNO ) {
                b_hist_calc = false;
            }
        }

        FindClose( sr );

        /* rmeos確保 */
        remos = (struct REMOS_FILE_CONTAINER *)malloc( sizeof(struct REMOS_FILE_CONTAINER) );

        /* BMP,JPGかチェック */
        if ( ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".BMP" || ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".JPG" ) {
            /* BMP, JPGモード */

            /* Canvasモードではremosをうまく使えないので、ファイル名をコピーさせた後、強制的に閉じる */
            remos_open( remos, OpenDialog->Files->Strings[i].c_str(), REMOS_FILE_TYPE_NOT_RECOG );

            /* ファイル、バンド設定を手動で */
            remos->band_count = 3;
            remos->bands = (struct REMOS_BAND*)malloc( sizeof(struct REMOS_BAND) * 3 );

            /* バンドを設定させる */
            for ( int j = 0; j < 3; j++ ) {
                remos->bands[j].band_num      = j;
                remos->bands[j].band_count    = 3;
                remos->bands[j].bits          = 8;
                remos->bands[j].range_bottom  = 0;
                remos->bands[j].range_top     = 255;
                remos->bands[j].range_max     = 255;
                remos->bands[j].range_min     = 0;
                remos->bands[j].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;		// DEBUG : 20130511これがないためにレンジ指定が機能しなかった
            }

            /* 画像をロード */
            img_bmp = new Graphics::TBitmap();

            if ( ExtractFileExt( OpenDialog->Files->Strings[i] ).UpperCase() == ".BMP" ) {
                /* bmpの場合 */
                img_bmp->LoadFromFile( OpenDialog->Files->Strings[i].c_str() );
            } else {
                /* jpgの場合 */
                img_jpg = new TJPEGImage();
                img_jpg->LoadFromFile( OpenDialog->Files->Strings[i].c_str() );
                img_bmp->Assign( img_jpg );
                delete img_jpg;
            }

            /* 既に開かれていた場合で、サイズが以前のと違うとエラー */
            if ( b_open ) {
                if ( ( img_w != img_bmp->Width ) || ( img_h != img_bmp->Height ) ) {
                    Application->MessageBoxA( "画像の大きさが違うバンド同士は同時に開けません", "失敗", MB_OK|MB_ICONINFORMATION );

                    remos_close( remos );
                    free( remos );
                    delete img_bmp;

                    /* メッセージを消す */
                    StatusForm->Close();

                    return ;
                }
            }

            /* 成功 */
            b_open = true;
            b_new_open = true;

            /* 登録 */
            list_file->Add( remos );

            /* 設定 */
            img_w = img_bmp->Width;
            img_h = img_bmp->Height;

            ScrImgHor->Max  = img_w - 1;
            ScrImgVert->Max = img_h - 1;

            StatusBar->Panels->Items[1]->Text = "縦 : " + IntToStr( img_h ) + " 横 : " + IntToStr( img_w );

            /* バンドも登録 */
            for ( int j = 0; j < remos->band_count; j++ ) {
                /* バンドボックスの生成と登録 */
                remos->bands[j].line_count     = img_h;
                remos->bands[j].line_img_width = img_w;
                remos->bands[j].line_width     = img_w;
                remos->bands[j].line_footer    = 0;
                remos->bands[j].line_header    = 0;

                remos->bands[j].hist_max                  = 1;	/* 0だと危険なので */
                remos->bands[j].hist_max_reduce_topbottom = 1;  /* 0だと危険なので */

                remos->bands[j].hist_max = 255;

                // ビット数などを指定
                remos->bands[j].byte_per_sample = 1;
                remos->bands[j].sample_per_pix  = 1;
                remos->bands[j].bits = 8;

                if ( !b_hist_calc ) {
                    for ( int k = 0; k < 256; k++ ) {
                        remos->bands[j].hist[k] = 0;
                    }
                }

                /* バンドボックスを作る */
                list_band->Add( MakeBandBox( &remos->bands[j], ExtractFileName( remos->file_name ), list_band->Count ) );

                box = (struct REMOS_FRONT_BAND *)list_band->Items[list_band->Count - 1];

                /* Canvasモードに */
                box->canvas_mode = true;
                box->canvas_color = box->index;
                box->img_canvas  = img_bmp;

                /* ヒストグラム作成フラグ */
                if ( b_hist_calc ) {
                    box->hist_maked = true;
                    MakeHist( box );
                } else {
                    box->hist_maked = false;

                    /* ヒストグラムクリア */
                    for ( int k = 0; k < 256; k++ ) {
                    	box->band->hist[i] = 0;
                    }
                }

                /* 更新 */
                UpdateBandBox( box );

                /* メッセージ処理 */
                Application->ProcessMessages();
            }
        } else {
            /* 衛星データを開く remosモード */
            if ( 0 != ( ret = (int)remos_open( remos, OpenDialog->Files->Strings[i].c_str(), REMOS_FILE_TYPE_AUTO ) ) ) {
                if ( ret == REMOS_RET_READ_FAILED ) {
                    /* 読めたけど識別失敗 */
                    msg = "\"" + ExtractFileName( OpenDialog->Files->Strings[i] ) + "\"\n"
                        + "は不明な衛星データ形式です";

                    Application->MessageBoxA( msg.c_str(), "識別失敗", MB_OK|MB_ICONINFORMATION );

                    remos_close ( remos );

                    free( remos );
                } else {
                    /* 既に開かれていた場合で、サイズが以前のと違うとエラー */
                    if ( b_open ) {
                        if ( ( img_w != remos->img_width ) || ( img_h != remos->img_height ) ) {
                            Application->MessageBoxA( "画像の大きさが違うバンド同士は同時に開けません", "失敗", MB_OK|MB_ICONINFORMATION );

                            remos_close( remos );
                            free( remos );

                            /* メッセージを消す */
                            StatusForm->Close();

                            return ;
                        }
                    }

                    /* 成功 */
                    b_open = true;
                    b_new_open = true;

                    /* 登録 */
                    list_file->Add( remos );

                    /* 設定 */
                    img_w = remos->img_width;
                    img_h = remos->img_height;

                    ScrImgHor->Max  = img_w - 1;
                    ScrImgVert->Max = img_h - 1;

                    StatusBar->Panels->Items[1]->Text = "縦 : " + IntToStr( img_h ) + " 横 : " + IntToStr( img_w );

                    /* バンドも登録 */
                    for ( int j = 0; j < remos->band_count; j++ ) {
                        /* バンドボックスの生成と登録 */

                        /* 登録時、ヒストグラムも作成 */
                        if ( b_hist_calc ) {
                            remos_make_hist( &remos->bands[j] );
                        } else {
                            /* 0除算防止などのため適当な初期値を代入 */
                            remos->bands[j].hist_max_reduce_topbottom = 255;
                            remos->bands[j].hist_max = 255;

                            /* ヒストグラムクリア */
                            for ( int k = 0; k < 256; k++ ) {
                                remos->bands[j].hist[k] = 0;
                            }
                        }

                        list_band->Add( MakeBandBox( &remos->bands[j], ExtractFileName( remos->file_name ), list_band->Count ) );

                        box = (struct REMOS_FRONT_BAND *)list_band->Items[list_band->Count - 1];

                        /* ヒストグラム作成フラグ */
                        if ( b_hist_calc ) {
                            box->hist_maked = true;
                        } else {
                            box->hist_maked = false;
                        }

                        /* キャンバスモードではない */
                        box->canvas_mode = false;

                        /* 更新 */
                        UpdateBandBox( box );

                        /* メッセージ処理 */
                        Application->ProcessMessages();
                    }
                }
            } else {
                /* 読み込み失敗 */
                free( remos );

                // 勝手に閉じてはいけない
                // b_open = false;
            }
        }
    }

    /* メッセージを消す */
    StatusForm->Close();

    /* 設定ファイルの読み込みを試みる ( 面倒くさいので毎回する ) */
    if ( b_new_open ) {
    	// 設定読み込み開始

    	// 開けたので一度設定をクリアする
    	clearSetting();

        fln_0 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "summary.txt";
        fln_1 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "*.met";
        fln_2 = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "*.txt";

        if ( econf_open( &ef, fln_0.c_str() ) ) {
            /* だいち用認識 */
            // だいちはレベル1B2R_Uしか今のところ処理できないようにしている
            if ( econf_recog( &ef ) ) {
            	if ( econf_search( &ef, "Pds_PixelSpacing" ) != NULL ) {
                	b_config_land = false;
                	b_config_resolution = true;

                    conf_resolution_x = econf_as_double( econf_search( &ef, "Pds_PixelSpacing" ) );
                    conf_resolution_y = conf_resolution_x;
                } else {
                	b_config_land = false;
                	b_config_resolution = false;
                }

				// レベル1B2なら
                if ( econf_search( &ef, "Pds_ProductID" ) != NULL ) {
                	AnsiString plevel = econf_as_str( econf_search( &ef, "Pds_ProductID" ) );

                	if ( plevel.Pos( "O1B2R_U" ) != 0 || plevel.Pos( "O1B2G_U" ) != 0 ) {
						// ALOSのUTM投影1B2だった
                        AnsiString fln_led;
                        TSearchRec sr_led;

                        // リーダーファイル読み込み
                        fln_led = ExtractFilePath( OpenDialog->Files->Strings[0] ) + "LED*";

                        if ( FindFirst( fln_led, faAnyFile, sr_led ) == 0 ) {
                        	// 発見したので開いてみる
                            if ( conf_alos.loadLeader( sr_led.Name.c_str() ) ) {
                            	// 成功したので設定有効,座標を一応保存
                                conf_lt_lat = conf_alos.getLat( 0, 0 );
                                conf_lt_lon = conf_alos.getLon( 0, 0 );
                                conf_rt_lat = conf_alos.getLat( img_w, 0 );
                                conf_rt_lon = conf_alos.getLon( img_w, 0 );
                                conf_lb_lat = conf_alos.getLat( 0, img_h );
                                conf_lb_lon = conf_alos.getLon( 0, img_h );
                                conf_rb_lat = conf_alos.getLat( img_w, img_h );
                                conf_rb_lon = conf_alos.getLon( img_w, img_h );

                                b_config = true;
                                b_config_alos = true;
                                b_config_land = false;
                                b_config_latlon_utm_proj = false;
                            }
                        }

                        // レコードを閉じる
                        FindClose( sr_led );
                    }
                }

                // 日付の設定
                if ( econf_search( &ef, "Lbi_ObservationDate" ) ) {
                	b_config_date = true;

                    config_date_str = econf_as_str( econf_search( &ef, "Lbi_ObservationDate" ) );
                } else {
                	b_config_date = false;
                }
            }

            econf_close( &ef );
        } else {
        	/* LANDSAT用認識 */
            int met = FindFirst( fln_1, faAnyFile, sr_conf );
            int txt = FindFirst( fln_2, faAnyFile, sr_conf_2 );
            AnsiString current_name;
            bool end_find = false;

            // すべて失敗すれば何もしない
            if ( met != 0 && txt != 0 ) {
            	end_find = true;
            }

            /* met優先でmetがなければtxtで実行 */
            while ( !end_find ) {
                if ( met == 0 ) {
                	current_name = sr_conf.Name;

                    if ( FindNext( sr_conf ) != 0 ) {
                    	end_find = true;
                    }
                } else {
                	current_name = sr_conf_2.Name;

                    if ( FindNext( sr_conf_2 ) != 0 ) {
                    	end_find = true;
                    }
                }

                if ( econf_open( &ef, current_name.c_str() ) ) {
                    /* 認識 */
                    if ( econf_recog( &ef ) ) {
                        if ( econf_search( &ef, "LMAX_BAND61" ) != NULL ) {
                            /* LANDSAT用発見 */
                            b_config_land = true;

                            // 温度設定読み込み
                            land_lmax = econf_as_double( econf_search( &ef, "LMAX_BAND61" ) );
                            land_lmin = econf_as_double( econf_search( &ef, "LMIN_BAND61" ) );

                            land_qcal_min = econf_as_double( econf_search( &ef, "QCALMIN_BAND61" ) );
                            land_qcal_max = econf_as_double( econf_search( &ef, "QCALMAX_BAND61" ) );

                            land_lmax_2 = econf_as_double( econf_search( &ef, "LMAX_BAND62" ) );
    						land_lmin_2 = econf_as_double( econf_search( &ef, "LMIN_BAND62" ) );
                            land_qcal_min_2 = econf_as_double( econf_search( &ef, "QCALMIN_BAND62" ) );
                            land_qcal_max_2 = econf_as_double( econf_search( &ef, "QCALMAX_BAND62" ) );

                            AnsiString fn_8, fn_61, fn_62;
                            AnsiString fnbuf;

                            // ファイル名を取得
                            fn_8  = econf_as_str( econf_search( &ef, "BAND8_FILE_NAME" ) );
                            fn_61 = econf_as_str( econf_search( &ef, "BAND61_FILE_NAME" ) );
                            fn_62 = econf_as_str( econf_search( &ef, "BAND62_FILE_NAME" ) );

                            // ファイル名比較
                            for ( int i = 0; i < OpenDialog->Files->Count; i++ ) {
                                if ( fn_8.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) ) {
                                    // 8ならパンクロ
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_PAN" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                } else if ( fn_61.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) || fn_62.Pos( ExtractFileName( OpenDialog->Files->Strings[i] ) ) ) {
                                    // 6162なら温度
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_THM" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                } else {
                                    conf_resolution_x = econf_as_double( econf_search( &ef, "GRID_CELL_SIZE_REF" ) );
                                    conf_resolution_y = conf_resolution_x;
                                    b_config_resolution = true;

                                    break;
                                }
                            }
                        } else {
                            b_config_land = false;
                            b_config_resolution = false;
                        }

                        // Landsat7かどうか
                        if ( AnsiString( econf_as_str( econf_search( &ef, "SPACECRAFT_ID" ) ) ) == "\"Landsat7\"" ) {
                        	b_config_ls7 = true;
                        } else {
                        	b_config_ls7 = false;
                        }

                        // 日付文字列
                        if ( econf_search( &ef, "ACQUISITION_DATE" ) ) {
                        	b_config_date = true;

                            config_date_str = econf_as_str( econf_search( &ef, "ACQUISITION_DATE" ) );
                        } else {
                        	b_config_date = false;
                        }


                        /* SCENEとPRODUCTの意味は違う気もする */
                        if ( econf_search( &ef, "PRODUCT_UL_CORNER_LAT" ) != NULL ) {
                            b_config = true;

                            conf_lt_lat = econf_as_double( econf_search( &ef, "PRODUCT_UL_CORNER_LAT" ) );
                            conf_lt_lon = econf_as_double( econf_search( &ef, "PRODUCT_UL_CORNER_LON" ) );
                            conf_rt_lat = econf_as_double( econf_search( &ef, "PRODUCT_UR_CORNER_LAT" ) );
                            conf_rt_lon = econf_as_double( econf_search( &ef, "PRODUCT_UR_CORNER_LON" ) );
                            conf_lb_lat = econf_as_double( econf_search( &ef, "PRODUCT_LL_CORNER_LAT" ) );
                            conf_lb_lon = econf_as_double( econf_search( &ef, "PRODUCT_LL_CORNER_LON" ) );
                            conf_rb_lat = econf_as_double( econf_search( &ef, "PRODUCT_LR_CORNER_LAT" ) );
                            conf_rb_lon = econf_as_double( econf_search( &ef, "PRODUCT_LR_CORNER_LON" ) );
                        } else {
                            b_config = false;
                        }
                    }

                    econf_close( &ef );
                }
            }

            FindClose( sr_conf );
            FindClose( sr_conf_2 );
        }

        // だいちでなければ画像データーから座標を読み込む ( うまくいけばこちらに移行 )
        if ( ( b_config == true && b_config_land == true ) || b_config == false ) {
        	// TIFFの認識を試みる
            GeotiffRec gt;
            int i;

            // 一つでも読めれるまで試す
            for ( i = 0; i < OpenDialog->Files->Count; i++ ) {
            	if ( gt.open( OpenDialog->Files->Strings[i].c_str() ) ) {
                	break;
                }
            }

            // 初期化出来れば処理開始
            if ( gt.initialize() ) {
            	// 初期化できたので閉じる
                gt.close();

                // 四辺座標設定
                conf_lt_lat = gt.getLatLon( Point2D( 0, 0 ) ).y;
                conf_lt_lon = gt.getLatLon( Point2D( 0, 0 ) ).x;
                conf_rt_lat = gt.getLatLon( Point2D( img_w, 0 ) ).y;
                conf_rt_lon = gt.getLatLon( Point2D( img_w, 0 ) ).x;
                conf_lb_lat = gt.getLatLon( Point2D( 0, img_h ) ).y;
                conf_lb_lon = gt.getLatLon( Point2D( 0, img_h ) ).x;
                conf_rb_lat = gt.getLatLon( Point2D( img_w, img_h ) ).y;
                conf_rb_lon = gt.getLatLon( Point2D( img_w, img_h ) ).x;

                // 設定有効
                b_config = true;

                // UTMモードならUTM設定有効
                b_config_utm = gt.isUTM();

                config_utm = gt;

                // 解像度設定 ( LatLonで座標設定をしているもので対応できない UTMに変換してからという手がある : TODO )
                if ( gt.isUTM() ) {
                	b_config_resolution = true;

                	conf_resolution_x = gt.getScale().x;
                	conf_resolution_y = gt.getScale().y;
				}
            }
        }

        // 設定文字列更新
        setSettingStr();

        // ウィンドウタイトル設定
        setWindowTitle( OpenDialog->Files->Strings[OpenDialog->Files->Count - 1] );
    }

    /* 描画 */
    DrawImg();
}
//---------------------------------------------------------------------------
void TSatViewMainForm::setWindowTitle( AnsiString filename )
{
	// ウィンドウタイトル設定
    AnsiString path;

    // 何かある場合
    if ( filename != "" ) {
        path = ExtractFileDir( filename );

        while ( path.Pos( "\\" ) ) {
            path.Delete( 1, path.Pos( "\\" ) );
        }

        SatViewMainForm->Caption = path + " - " + AnsiString( "SatelliteEye" );
        Application->Title = path + " - " + AnsiString( "SatelliteEye" );
    } else {
        SatViewMainForm->Caption = AnsiString( "SatelliteEye" );
        Application->Title = AnsiString( "SatelliteEye" );
    }
}
//---------------------------------------------------------------------------
void TSatViewMainForm::setSettingStr( void )
{
	// 設定文字列を設定
    if ( b_config || ( b_config == false && b_config_land == true ) ) {
    	// 衛星種別
    	if ( b_config_land ) {
        	SettingStrPanel->Caption = "Landsat";

            if ( b_config_ls7 ) {
            	SettingStrPanel->Caption = SettingStrPanel->Caption + "7";
            }
        } else if ( b_config_alos ) {
        	SettingStrPanel->Caption = "だいち";
        } else {
            SettingStrPanel->Caption = "不明(座標設定のみ有効)";
        }

        // UTMならutmを追加
        if ( b_config_utm ) {
        	// SettingStrPanel->Caption = SettingStrPanel->Caption + "(UTM)";
        }

        // 日付
        if ( b_config_date ) {
        	SettingStrPanel->Caption = SettingStrPanel->Caption + " : " + config_date_str;
        }
    } else {
    	SettingStrPanel->Caption = "設定ファイル無効";
    }

    /*
    if ( b_config ) {
        StatusBar->Panels->Items[3]->Text = "座標設定有効";

        if ( b_config_land ) {
            StatusBar->Panels->Items[3]->Text = "座標設定有効 LANDSAT温度設定有効";
        }
    } else if ( b_config_land ) {
        StatusBar->Panels->Items[3]->Text = "LANDSAT温度設定有効";
    } else {
    	StatusBar->Panels->Items[3]->Text = "設定ファイル無効";
    }
    */
}
//---------------------------------------------------------------------------
void TSatViewMainForm::clearSetting( void )
{
	// 設定をクリアする
    b_config = false;
    b_config_land = false;
    b_config_resolution = false;
    b_config_utm = false;
    b_config_ls7 = false;
    b_config_date = false;
    b_config_alos = false;
    b_config_latlon_utm_proj = false;

    // 設定文字列設定
    setSettingStr();

    // ウィンドウタイトルクリア
    setWindowTitle( "" );
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::Button1Click(TObject *Sender)
{
	ColorBarForm->Show();
    ColorBarForm->draw();

    ColorMapConfigForm->setColorMap( color_map );
    ColorMapConfigForm->updateView();
    ColorMapConfigForm->Show();
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::setColorBarExp( AnsiString exp )
{
	// 式文字設定
    color_bar_exp = exp;
}
//---------------------------------------------------------------------------
void __fastcall TSatViewMainForm::TBMonosasiClick(TObject *Sender)
{
	if ( b_mode_length ) {
    	b_mode_length = false;
        TBMonosasi->Down = false;
    } else {
        // 設定が有効であれば定規モードに移る
        if ( b_config_resolution ) {
            b_mode_length = true;
            TBMonosasi->Down = true;
        } else {
            Application->MessageBoxA( "解像度の設定を読み込むことができませんでした。\n距離を測定できません。", "二点間の距離を測定", MB_OK | MB_ICONINFORMATION );
            TBMonosasi->Down = false;

            return;
        }

        // スタンプモード有効ならオフに
        if ( b_mode_stamp_cb ) {
        	TBStamp->Click();
        }
    }
}
//---------------------------------------------------------------------------


void __fastcall TSatViewMainForm::TBStampClick(TObject *Sender)
{
	// カラーバースタンプモード切り替え
	if ( b_mode_stamp_cb ) {
    	b_mode_stamp_cb = false;
        TBStamp->Down = false;

        // バッファからコピー
        SatImage->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), img_back->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );

    } else {
    	// 式モードでは実行できない
        if ( b_draw_mode_exp ) {
        	Application->MessageBoxA( "「式で描く」モードでは色の見本を貼り付けられません。", "情報", MB_ICONINFORMATION | MB_OK );
            TBStamp->Down = false;

            return;
        }

    	b_mode_stamp_cb = true;
        TBStamp->Down = true;

		// バッファへ転送
    	img_back->Canvas->CopyRect( Rect( 0, 0, SatImage->Width, SatImage->Height ), SatImage->Canvas, Rect( 0, 0, SatImage->Width, SatImage->Height ) );


        // 定規モード有効ならオフに
        if ( b_mode_length ) {
        	TBMonosasi->Click();
        }
    }
}
//---------------------------------------------------------------------------



void __fastcall TSatViewMainForm::ExpDrawRadioButtonClick(TObject *Sender)
{
	// 式描画モード
    b_draw_mode_exp = true;
    ExpEditR->Enabled = true;
	ExpEditG->Enabled = true;
    ExpEditB->Enabled = true;

    ShowLevelColorBarButton->Enabled = false;
    ConfigLevelButton->Enabled = false;

    // カラーバーを消す
    ColorBarForm->Close();

    // カラーバー設定 ( 一応 )
    ColorBarForm->setExpMode( true );

    // 再描画
    DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::LevelDrawRadioButtonClick(
      TObject *Sender)
{
	// レベルスライス描画モード
    b_draw_mode_exp = false;
    ExpEditR->Enabled = false;
	ExpEditG->Enabled = false;
    ExpEditB->Enabled = false;

    ShowLevelColorBarButton->Enabled = true;
    ConfigLevelButton->Enabled = true;

    // カラーバー設定
    ColorBarForm->setExpMode( false );
    ColorBarForm->setColorMap( color_map );

    // 再描画
    DrawImg();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ConfigLevelButtonClick(TObject *Sender)
{
	// レベルスライスの設定を起動
    ColorMapConfigForm->Show();
    ColorMapConfigForm->setColorMap( color_map );
    ColorMapConfigForm->updateView();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::ShowLevelColorBarButtonClick(
      TObject *Sender)
{
    // カラーバー表示
    ColorBarForm->viewNowValue( true );
	ColorBarForm->setColorMap( color_map );
    ColorBarForm->Show();
    ColorBarForm->draw();
}
//---------------------------------------------------------------------------

void __fastcall TSatViewMainForm::MI_C_GDEMClick(TObject *Sender)
{
	// ASTERGDEMのプリセット
    int i;
    struct REMOS_FRONT_BAND *box;

    /* ファイルが開かれて無ければ開かない */
    if ( !b_open ) {
    	return;
    }

    /* 初期設定 */
    PresetFormAGDEM->BandComboBox->Clear();

    /* バンド登録 */
    for ( i = 0; i < list_band->Count; i++ ) {
    	box = (struct REMOS_FRONT_BAND *)list_band->Items[i];

        PresetFormAGDEM->BandComboBox->Items->Add( box->label_name->Caption );
    }

    PresetFormAGDEM->BandComboBox->ItemIndex = 0;

	PresetFormAGDEM->Show();
}
//---------------------------------------------------------------------------





void __fastcall TSatViewMainForm::TBCurposClick(TObject *Sender)
{
	/* ピクセル情報表示 */
    if ( !PixForm->Visible ) {
		PixForm->Show();
        PixForm->Caption = "座標値 ( 最新 )";
    }
}
//---------------------------------------------------------------------------


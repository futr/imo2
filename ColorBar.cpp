//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ColorBar.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TColorBarForm *ColorBarForm;
//---------------------------------------------------------------------------
__fastcall TColorBarForm::TColorBarForm(TComponent* Owner)
	: TForm(Owner)
{
	// 初期設定
    tok_r = NULL;
    tok_g = NULL;
    tok_b = NULL;
    tok_unit = NULL;
    tok_cm_unit = NULL;

	viewNowValue( true );
    setNowValue( 100 );
    setValueRange( 0, 255 );
    setColorString( "a", "a", "a" );
    setUnitString( "a", "[UNIT]" );
    setExpMode();
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setExpMode( bool mode )
{
	// 式モードかカラーバーモードか
    b_exp_mode = mode;
}
//---------------------------------------------------------------------------
bool __fastcall TColorBarForm::getExpMode( void )
{
	// 式モードかを返す
    return b_exp_mode;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::viewNowValue( bool view )
{
	// 現在の値を表示するか
    write_now_value = view;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setValueRange( double bottom, double top )
{
	// 値の範囲
    bottom_value = bottom;
    top_value = top;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setNowValue( double now )
{
	// 現在の値
    now_value = now;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setColorMap( ColorMap *map )
{
	// カラーマップ設定
    color_map = map;

    // 単位式設定
    ecalc_free_token( tok_cm_unit );
    tok_cm_unit = ecalc_make_token( AnsiString( "a" ).c_str() );
	tok_cm_unit = ecalc_make_tree( tok_cm_unit );
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setColorString( AnsiString r, AnsiString g, AnsiString b )
{
	// 式文字設定
    r_str = r;
    g_str = g;
    b_str = b;

    ecalc_free_token( tok_r );
    tok_r = ecalc_make_token( r.c_str() );
	tok_r = ecalc_make_tree( tok_r );
    ecalc_free_token( tok_g );
    tok_g = ecalc_make_token( g.c_str() );
	tok_g = ecalc_make_tree( tok_g );
    ecalc_free_token( tok_b );
    tok_b = ecalc_make_token( b.c_str() );
	tok_b = ecalc_make_tree( tok_b );
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::setUnitString( AnsiString exp, AnsiString unit )
{
	// 単位文字列設定
    ecalc_free_token( tok_unit );
    tok_unit = ecalc_make_token( exp.c_str() );
    tok_unit = ecalc_make_tree( tok_unit );

    this->unit = unit;
    unit_str = exp;
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::draw( void )
{
	// 描画
    TCanvas *canvas;
    int width;
    int height;
    int pad;
    int i;
    int r;
    int g;
    int b;
    int pos;
    AnsiString str;
    double val;
    TColor color;
    double var[ECALC_VAR_COUNT];
    double *vars[ECALC_VAR_COUNT];

    double unit_var[ECALC_VAR_COUNT];
    double *unit_vars[ECALC_VAR_COUNT];

    double bottom;
    double top;
    double now;

    // 式用ポインター初期化
    for ( i = 0; i < ECALC_VAR_COUNT; i++ ) {
    	vars[i] = &var[i];
        unit_vars[i] = &unit_var[i];
    }

    // 大きさ
    ColorBarImage->Picture->Bitmap->Width  = ColorBarImage->Width;
    ColorBarImage->Picture->Bitmap->Height = ColorBarImage->Height;

    width  = ColorBarImage->Width;
    height = ColorBarImage->Height;

    // キャンバス取得
    canvas = ColorBarImage->Canvas;

    // 設定
    pad = canvas->TextHeight( "text" ) + 5;

    // 背景
    canvas->Brush->Color = clWhite;
    canvas->FillRect( Rect( 0, 0, width, height ) );

    // カラーバーモードでレベルが登録されてなければ何もしない
    if ( b_exp_mode == false ) {
    	if ( color_map == NULL || color_map->getColorLevelCount() < 2 ) {


        	return;
        }
    }

    // カラーバーを書く
    for ( i = 0; i < width - ( pad * 2 ); i++ ) {
        // モードで動作切り替え
        if ( b_exp_mode ) {
        	// 式モード

            // 今書くべき位置の値
        	val = ( (double)i / ( width - ( pad * 2 ) - 1 ) ) * ( fabs( top_value - bottom_value ) ) + bottom_value;

            // 値設定
            var[0] = val;

            // 色を取得する
            r = ecalc_get_tree_value( tok_r, vars, 0 );
            g = ecalc_get_tree_value( tok_g, vars, 0 );
            b = ecalc_get_tree_value( tok_b, vars, 0 );

            color = (TColor)RGB( r, g, b );
        } else {
        	// カラーバーモード

            // 今書くべき位置の値
        	val = ( (double)i / ( width - ( pad * 2 ) - 1 ) ) * ( fabs( color_map->getColorLevel( color_map->getColorLevelCount() - 1 )->getLevel() - color_map->getColorLevel( 0 )->getLevel() ) ) + color_map->getColorLevel( 0 )->getLevel();

            color_map->setValue( val );
            color_map->makeColor();

            color = (TColor)RGB( color_map->getR(), color_map->getG(), color_map->getB() );
        }

        // 書く
        canvas->Pen->Color = color;

        canvas->MoveTo( pad + i, pad );
        canvas->LineTo( pad + i, height - canvas->TextHeight( "text" ) );
    }

    // 文字を書く
    canvas->Pen->Color = clBlack;

    // 上下の値を書く
    if ( b_exp_mode ) {
        // 下
        unit_var[0] = bottom_value;
        canvas->TextOutA( pad, height - canvas->TextHeight( "text" ), ecalc_get_tree_value( tok_unit, unit_vars, 0 ) );

        // 上
        unit_var[0] = top_value;
        str = ecalc_get_tree_value( tok_unit, unit_vars, 0 );
        str = str + " " + unit;

        canvas->TextOutA( width - pad - canvas->TextWidth( str ), height - canvas->TextHeight( "text" ), str );
    } else {
    	// 下
        unit_var[0] = color_map->getColorLevel( 0 )->getLevel();
        canvas->TextOutA( pad, height - canvas->TextHeight( "text" ), ecalc_get_tree_value( tok_cm_unit, unit_vars, 0 ) );

        // 上
        unit_var[0] = color_map->getColorLevel( color_map->getColorLevelCount() - 1 )->getLevel();
        str = ecalc_get_tree_value( tok_cm_unit, unit_vars, 0 );
        str = str + " " + color_map->getUnitString();

        canvas->TextOutA( width - pad - canvas->TextWidth( str ), height - canvas->TextHeight( "text" ), str );
    }

	// 現在の値が必要なら書く
    if ( write_now_value ) {
    	// 描画モードで分ける
        if ( b_exp_mode ) {
        	// 式モード

            // 文字作成
            unit_var[0] = now_value;
            str = str.sprintf( "%.1f", ecalc_get_tree_value( tok_unit, unit_vars, 0 ) );
            str = str + " " + unit;

            // 位置決定
            if ( now_value < bottom_value ) {
                pos = pad;
            } else if ( now_value > top_value ) {
                pos = width - pad * 2 + pad;
            } else {
                pos = ( now_value - bottom_value ) / ( top_value - bottom_value ) * ( width - pad * 2 ) + pad;
            }

            // 位置に線を書く
            canvas->Pen->Color = clRed;
            canvas->MoveTo( pos, 0 );
            canvas->LineTo( pos, pad );

            // 飛び出さないように字を書く
            if ( pos + canvas->TextWidth( str ) > width ) {
                canvas->TextOutA( width - canvas->TextWidth( str ), 0, str );
            } else {
                canvas->TextOutA( pos, 0, str );
            }
        } else {
        	// カラーバーモード

            // 文字作成
            unit_var[0] = now_value;
            str = str.sprintf( "%.1f", ecalc_get_tree_value( tok_cm_unit, unit_vars, 0 ) );
            str = str + " " + color_map->getUnitString();

            // 位置決定
            if ( now_value < color_map->getColorLevel( 0 )->getLevel() ) {
                pos = pad;
            } else if ( now_value > color_map->getColorLevel( color_map->getColorLevelCount() - 1 )->getLevel() ) {
                pos = width - pad * 2 + pad;
            } else {
                pos = ( now_value - color_map->getColorLevel( 0 )->getLevel() ) / ( color_map->getColorLevel( color_map->getColorLevelCount() - 1 )->getLevel() - color_map->getColorLevel( 0 )->getLevel() ) * ( width - pad * 2 ) + pad;
            }

            // 位置に線を書く
            canvas->Pen->Color = clRed;
            canvas->MoveTo( pos, 0 );
            canvas->LineTo( pos, pad );

            // 飛び出さないように字を書く
            if ( pos + canvas->TextWidth( str ) > width ) {
                canvas->TextOutA( width - canvas->TextWidth( str ), 0, str );
            } else {
                canvas->TextOutA( pos, 0, str );
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TColorBarForm::FormResize(TObject *Sender)
{
	draw();
}
//---------------------------------------------------------------------------


void __fastcall TColorBarForm::FormDestroy(TObject *Sender)
{
	// 解放
    ecalc_free_token( tok_r );
    ecalc_free_token( tok_g );
    ecalc_free_token( tok_b );
    ecalc_free_token( tok_unit );
    ecalc_free_token( tok_cm_unit );	
}
//---------------------------------------------------------------------------


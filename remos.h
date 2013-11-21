/*
 * remos.c GDEM用改造版
 * 本来GDEMはsignedなのでうまく処理できていない
 * signedフラグとエンディアンフラグを作り処理で使うべき
 *
 *
 */

#ifndef REMOS_H_INCLUDED
#define REMOS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "tifrec.h"

#define REMOS_MAX_FILENAME			1024

#define REMOS_SIZE_INT				4						/* intの大きさ */
#define REMOS_SIZE_CHAR				1						/* charの大きさ */
#define REMOS_SIZE_SHORT			2						/* short intの大きさ */

#define REMOS_FILE_TYPE_NOT_RECOG		-3					/* 識別させない */
#define REMOS_FILE_TYPE_UNKNOWN			-2					/* 不明 */
#define REMOS_FILE_TYPE_AUTO			-1					/* 自動判別 */
#define REMOS_FILE_TYPE_BIL				0					/* 標準的なBIL */
#define REMOS_FILE_TYPE_BSQ 			1					/* 標準的なBSQ */
#define REMOS_FILE_TYPE_BIL_ALOS		2					/* ALOSのBIL */
#define REMOS_FILE_TYPE_BSQ_ALOS		3					/* ALOSのBSQ */
#define REMOS_FILE_TYPE_TIFF			4					/* TIFF */
#define REMOS_FILE_TYPE_BIL_R10			6					/* いも天形式のBIL */
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL	7					/* ALOSのPAL */
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL_11 8					/* ALOSのPAL1.1 */

#define REMOS_RET_FAILED			0						/* 失敗 */
#define REMOS_RET_SUCCEED			1						/* 成功 */
#define REMOS_RET_UNKNOWN			2						/* 開けはしたけど識別失敗 */
#define REMOS_RET_READ_FAILED		3						/* なぜか読み込み失敗 */

enum remos_endian {
	REMOS_ENDIAN_BIG,
	REMOS_ENDIAN_LITTLE
};

enum remos_band_color {
	REMOS_BAND_COLOR_BW,
	REMOS_BAND_COLOR_RGB,
	REMOS_BAND_COLOR_PACKED_USINT_2							/* PAL1.1用 ( 無理やり ) */
};

enum remos_band_sample_format {								/* サンプル方法 ( tiffのタグの値を参考 ) */
	REMOS_BAND_SAMPLE_FORMAT_INT,
	REMOS_BAND_SAMPLE_FORMAT_UINT,
	REMOS_BAND_SAMPLE_FORMAT_IEEEFP,
	REMOS_BAND_SAMPLE_FORMAT_COMPLEX_INT,
	REMOS_BAND_SAMPLE_FORMAT_COMPLEX_IEEEFP,
};

struct REMOS_PIXELS {
	unsigned int *pixels;									/* データ配列 */
	int count;												/* データ数 */
};

struct REMOS_BAND {											/* バンド情報構造体 */
	int band_num;											/* バンド番号 0から  */
	int band_count;											/* 所属するファイルのバンド数 */
	
	int endian;
	
	int bits;												/* ビット幅 ( 今のとこ8オンリー ) */
	int byte_per_sample;									/* 1pix何バイトか ( 未実装 ) */
	int sample_per_pix;										/* 1pixあたりのサンプル数 */

	int color;												/* バンドの色情報 */
	int sample_format;										/* ピクセルのサンプル方法 */

	int line_width;											/* ライン幅 */
	int line_count;											/* ライン数 */
	int line_header;										/* ラインヘッダ */
	int line_footer;										/* ラインフッター */
	int line_img_width;										/* ライン内の画像幅 */

	unsigned int hist[256];									/* ヒストグラム ( 8bit専用 ) */
	int hist_max;											/* ヒストグラム最大値 */
	int hist_max_reduce_topbottom;							/* 0と255を除いたヒストグラム最大値 */

	float range_top;										/* レンジ上 */
	float range_bottom;										/* レンジ下 */
	float range_max;										/* レンジ最大値 */
	float range_min;										/* レンジ最小値 */

	int header;												/* 全ヘッダサイズ */

	FILE *fp;
};

struct REMOS_FILE_CONTAINER {								/* ファイルコンテナ? */
	FILE *fp;												/* ファイルポインタ */
	char file_name[REMOS_MAX_FILENAME];						/* ファイル名 */
	
	int type;												/* 読み込まれたファイルタイプ */
	int band_count;											/* バンド数 */
	
	int img_width;											/* 画像幅 */
	int img_height;											/* 画像高さ */
	
	struct REMOS_BAND *bands;								/* バンド配列 */
};

/* 各衛星用ヘッダ ( 詰め物が存在するとまずい、けどcharなら大丈夫かな？ ) */

struct REMOS_HEADER_LINE_ALOS {								/* イメージファイルディスクリプタレコード */
	unsigned char record_count_I6[6];
	unsigned char record_length_I6[6];
	unsigned char blank_A24[24];
	unsigned char bit_per_pix_I4[4];
	unsigned char pix_per_data_I4[4];
	unsigned char byte_per_data_I4[4];
	unsigned char bitlist_A4[4];
	unsigned char band_per_file_I4[4];
	unsigned char line_per_band_I8[8];
	unsigned char left_border_pix_per_line_I4[4];
	unsigned char pix_per_line_I8[8];
	unsigned char right_border_pix_per_line_I4[4];
	unsigned char top_border_lines_I4[4];
	unsigned char under_border_lines_I4[4];
	unsigned char image_format_A4[4];
	unsigned char record_per_line_single_I4[4];
	unsigned char record_per_line_I4[4];
	unsigned char header_bytes_I4[4];						/* よくわからないけど、おそらくヘッダサイズ */
	unsigned char data_bytes_per_record_I8[8];
	unsigned char footer_bytes_I4[4];						/* よくわからないけど、恐らくフッター */

	/* 残りは恐らくデータの一行目の長さ - ここまで */
};

struct REMOS_HEADER_LINE_ALOS_PAL {								/* イメージファイルディスクリプタレコード */
	unsigned char record_count_I6[6];
	unsigned char record_length_I6[6];
	unsigned char blank_A24[24];
	unsigned char bit_per_pix_I4[4];
	unsigned char pix_per_data_I4[4];
	unsigned char byte_per_data_I4[4];
	unsigned char bitlist_A4[4];
	unsigned char band_per_file_I4[4];
	unsigned char line_per_band_I8[8];
	unsigned char left_border_pix_per_line_I4[4];
	unsigned char pix_per_line_I8[8];
	unsigned char right_border_pix_per_line_I4[4];
	unsigned char top_border_lines_I4[4];
	unsigned char under_border_lines_I4[4];
	unsigned char image_format_A4[4];
	unsigned char record_per_line_single_I2[2];
	unsigned char record_per_line_I2[2];
	unsigned char header_bytes_I4[4];						/* よくわからないけど、おそらくヘッダサイズ */
	unsigned char data_bytes_per_record_I8[8];
	unsigned char footer_bytes_I4[4];						/* よくわからないけど、恐らくフッター */

	/* 残りは恐らくデータの一行目の長さ - ここまで */
};

#ifdef __cplusplus
extern "C" {
#endif

int remos_open( struct REMOS_FILE_CONTAINER *cont, char *filename, int type );								/* ファイルを開く */
int remos_close( struct REMOS_FILE_CONTAINER *cont );														/* ファイルを閉じる */

/* タイプを手動設定 */
int remos_set_type_BIL( struct REMOS_FILE_CONTAINER *cont, int file_header, int header, int footer, int lines, int width, int bands ,int bits );
int remos_set_type_BSQ( struct REMOS_FILE_CONTAINER *cont, int file_header, int header, int footer, int lines, int width, int bits );

float remos_get_pixel( struct REMOS_BAND *band, int pos );													/* 指定バンドの指定位置から1ピクセルもらう */
int remos_get_line_pixels( struct REMOS_BAND *band, unsigned char *buf, int line, int from, int count );	/* 指定バンドの指定位置からcountピクセルをつめこむ  */
void remos_get_pixels( struct REMOS_FILE_CONTAINER *cont, int pos, struct REMOS_PIXELS *pixs );				/* ファイルコンテナ内の全バンドの指定一のデータを一括でもらう */
void remos_get_ranged_pixels( struct REMOS_BAND *band, unsigned char *buf, int count );						/* 指定バンドのダイナミックレンジに応じてレンジングする */
int remos_get_ranged_pixel( struct REMOS_BAND *band, int val );												/* 指定バンドのレンジに応じて1バイト処理 */

void remos_make_pixels( struct REMOS_FILE_CONTAINER *cont, struct REMOS_PIXELS *pixs );						/* PIXELS内の配列を自動確保 */
void remos_free_pixels( struct REMOS_PIXELS *pixs );														/* 確保されたPIXELSを自動解放 */

int remos_check_file_type( struct REMOS_FILE_CONTAINER *cont );												/* ファイルタイプを自動設定 */
int remos_read_file( struct REMOS_FILE_CONTAINER *cont );													/* 指定されたタイプでファイル識別 */
int remos_make_hist( struct REMOS_BAND *band );																/* ヒストグラム生成 */
void remos_calc_auto_range( struct REMOS_BAND *band, double per, int topbottom );							/* 自動でダイナミックレンジ設定 */
void remos_set_range( struct REMOS_BAND *band, int bottom, int top );										/* ダイナミックレンジ(?)設定 */

int remos_I2int( char *data, int length );																	/* I形式のテキストをintに alos等用 */
unsigned int remos_BE2usint( char *data, int length );														/* ビッグエンディアンのデータを読む */

float remos_data_to_value_band( struct REMOS_BAND *band, unsigned char *data );								/* バンドのルールに対応した方法でデーターを取り出す */
float remos_data_to_value_format( unsigned char *data, int len, int endian, int format );					/* エンディアンと書式にしたがってデータを読み込む ( floatで返す ) */
unsigned int remos_data_to_value( unsigned char *data, int len, int endian );								/* エンディアンに従ってデータを読む */
float remos_data_to_float( unsigned char *data, int len, int endian );										/* エンディアンに従ってデータを読む ( float ) */

struct REMOS_BAND *remos_get_band( struct REMOS_FILE_CONTAINER *cont, int num );							/* 指定したバンド番号のバンドを得る */

/* ユーティリティ */
void remos_latlon_to_xy( int *i, int *j, int w, int h, float lat, float lon, float ox, float oy, float ax, float ay, float bx, float by, float cx, float cy );

#ifdef __cplusplus
}
#endif

#endif

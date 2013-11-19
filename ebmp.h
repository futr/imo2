#ifndef EBMP_H_INCLUDED
#define EBMP_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EBMP_FILENAME_LEN 1024

#define EBMP_FILEHEADER_SIZE 14
#define EBMP_INFOHEADER_SIZE 40

enum EBMP_COMPRESSION {
	EBMP_COMP_RGB  = 0,
	EBMP_COMP_RLE8 = 1,
	EBMP_COMP_RLE4 = 2,
	EBMP_COMP_BITFIELDS = 3
};

typedef struct EBMP_FILEHEADER_tag {
	unsigned short bfType;			/* BM */
	unsigned int   bfSize;			/* ファイルのバイト数 ( 無意味 ) */
	unsigned short bfRes1;			/* Reserved */
	unsigned short bfRes2;			/* Reserved */
	unsigned int   bfOffBits;		/* BMPファイル先頭からビットマップデータへのバイト単位のオフセット */
} EBMP_FILEHEADER;

typedef struct EBMP_INFOHEADER_tag {
	unsigned int biSize;
	int biWidth;
	int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} EBMP_INFOHEADER;

typedef struct EBMP_FILE_tag {
	FILE *fp;							/* ファイルポインター */
	char filename[EBMP_FILENAME_LEN];	/* ファイル名 */
	
	int width;							/* 画像の幅 */
	int height;							/* 画像の高さ */
	
	int bits;							/* ビット数 */
	int line_size;						/* scanlineの大きさ ( 4バイトアライン ) */
	int header_size;					/* ヘッダーサイズ */
	
	EBMP_FILEHEADER file_header;
	EBMP_INFOHEADER info_header;
} EBMP_FILE;

#ifdef __cplusplus
extern "C" {
#endif

int ebmp_create_file_open( EBMP_FILE *bmp, char *filename, int width, int height );
int ebmp_create_file_close( EBMP_FILE *bmp );

void ebmp_write_file_header( EBMP_FILE *bmp );
void ebmp_write_info_header( EBMP_FILE *bmp );

void ebmp_write_point_rgb_24( EBMP_FILE *bmp, int x, int y, int r, int g, int b );
void ebmp_write_line( EBMP_FILE *bmp, int y, void *data, size_t size );

void ebmp_write_line_seq_start( EBMP_FILE *bmp );
void ebmp_write_line_seq( EBMP_FILE *bmp, void *data, size_t size );

#ifdef __cplusplus
}
#endif

#endif

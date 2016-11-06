/*
 * remos.c GDEM�p������
 * �{��GDEM��signed�Ȃ̂ł��܂������ł��Ă��Ȃ�
 * signed�t���O�ƃG���f�B�A���t���O����菈���Ŏg���ׂ�
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
#include "hsdinfo.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define REMOS_MAX_FILENAME			1024

#define REMOS_SIZE_INT				4						/* int�̑傫�� */
#define REMOS_SIZE_CHAR				1						/* char�̑傫�� */
#define REMOS_SIZE_SHORT			2						/* short int�̑傫�� */

#define REMOS_FILE_TYPE_NOT_RECOG		    -3					/* ���ʂ����Ȃ� */
#define REMOS_FILE_TYPE_UNKNOWN			    -2					/* �s�� */
#define REMOS_FILE_TYPE_AUTO			    -1					/* �������� */
#define REMOS_FILE_TYPE_BIL				    0					/* �W���I��BIL */
#define REMOS_FILE_TYPE_BSQ 			    1					/* �W���I��BSQ */
#define REMOS_FILE_TYPE_BIL_ALOS		    2					/* ALOS��BIL */
#define REMOS_FILE_TYPE_BSQ_ALOS		    3					/* ALOS��BSQ */
#define REMOS_FILE_TYPE_TIFF			    4					/* TIFF */
#define REMOS_FILE_TYPE_BIL_R10		   	    6					/* �����V�`����BIL */
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL   	    7					/* ALOS��PAL */
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL_11     8					/* ALOS��PAL1.1 */
#define REMOS_FILE_TYPE_HSD                 9                   // �Ђ܂��HSD
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL2_11    10					/* ALOS2��PAL1.1 */
#define REMOS_FILE_TYPE_BSQ_ALOS_PAL2_21    11					/* ALOS2��PAL2.1 */

#define REMOS_RET_FAILED			0						/* ���s */
#define REMOS_RET_SUCCEED			1						/* ���� */
#define REMOS_RET_UNKNOWN			2						/* �J���͂������ǎ��ʎ��s */
#define REMOS_RET_READ_FAILED		3						/* �Ȃ����ǂݍ��ݎ��s */

#define REMOS_MAX_32BIT 4294967296ULL

enum remos_band_mode{
    REMOS_BAND_MODE_BIL,
    REMOS_BAND_MODE_PACK,
    REMOS_BAND_MODE_BSQ,
};

enum remos_endian {
	REMOS_ENDIAN_BIG,
	REMOS_ENDIAN_LITTLE
};

enum remos_band_color {
	REMOS_BAND_COLOR_BW,
	REMOS_BAND_COLOR_RGB,
	REMOS_BAND_COLOR_PACKED_IEEEFP							/* PAL1.1�p */
};

enum remos_band_sample_format {								/* �T���v�����@ ( tiff�̃^�O�̒l���Q�l ) */
	REMOS_BAND_SAMPLE_FORMAT_INT,
	REMOS_BAND_SAMPLE_FORMAT_UINT,
	REMOS_BAND_SAMPLE_FORMAT_IEEEFP,
	REMOS_BAND_SAMPLE_FORMAT_COMPLEX_INT,
	REMOS_BAND_SAMPLE_FORMAT_COMPLEX_IEEEFP,
};

enum remos_file_move_method {
    REMOS_FILE_MOVE_CURRENT,
    REMOS_FILE_MOVE_BEGIN,
    REMOS_FILE_MOVE_END,
};

struct REMOS_FILE_CONTAINER;

struct REMOS_BAND {											/* �o���h���\���� */
	int band_num;											/* �o���h�ԍ� 0����  */
	int band_count;											/* ��������t�@�C���̃o���h�� */
	
	int endian;
	
	int bits;												/* �r�b�g�� ( ���̂Ƃ�8�I�����[ ) */
	int byte_per_sample;									/* 1pix���o�C�g�� ( ������ ) */
	int sample_per_pix;										/* 1pix������̃T���v���� */

	int color;												/* �o���h�̐F��� */
	int sample_format;										/* �s�N�Z���̃T���v�����@ */

	int line_width;											/* ���C���� ( �w�b�_�E�t�b�^�܂ރo�C�g ) */
	int line_count;											/* ���C���� */
	int line_header;										/* ���C���w�b�_ */
	int line_footer;										/* ���C���t�b�^�[ */
	int line_img_width;										/* ���C�����̉摜�� ( �o�C�g���ł͂Ȃ� ) */

	unsigned int hist[256];									/* �q�X�g�O���� */
	int hist_max;											/* �q�X�g�O�����ő�l ( �q�X�g�O�������ʒu0-255 ) */
	int hist_max_reduce_topbottom;							/* 0��255���������q�X�g�O�����ő�l ( �q�X�g�O�������ʒu0-255 ) */
	int hist_pixels;										/* �q�X�g�O�����v�Z���ɐ������s�N�Z���� */

	float range_top;										/* �����W�� ( ���ۂ̒l ) */
	float range_bottom;										/* �����W�� ( ���ۂ̒l ) */
	float range_max;										/* �����W�ő�l ( ���ۂ̒l ) */
	float range_min;										/* �����W�ŏ��l ( ���ۂ̒l ) */

	int header;												/* �S�w�b�_�T�C�Y */

    enum remos_band_mode band_mode;                         /* �o���h�f�[�^�̊i�[���@ */

	FILE *fp;

    struct REMOS_FILE_CONTAINER *cont;                      /* �e�R���e�i */
};

struct REMOS_FILE_CONTAINER {								/* �t�@�C���R���e�i? */
#ifdef _WIN32
    HANDLE hf;
#endif
	FILE *fp;												/* �t�@�C���|�C���^ */
    
	char file_name[REMOS_MAX_FILENAME];						/* �t�@�C���� */
	
	int type;												/* �ǂݍ��܂ꂽ�t�@�C���^�C�v */
	int band_count;											/* �o���h�� */
	
	int img_width;											/* �摜�� */
	int img_height;											/* �摜���� */
	
	struct REMOS_BAND *bands;								/* �o���h�z�� */
};

/* �e�q���p�w�b�_ ( �l�ߕ������݂���Ƃ܂����A����char�Ȃ���v���ȁH ) */

struct REMOS_HEADER_LINE_ALOS {								/* �C���[�W�t�@�C���f�B�X�N���v�^���R�[�h */
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
	unsigned char header_bytes_I4[4];						/* �悭�킩��Ȃ����ǁA�����炭�w�b�_�T�C�Y */
	unsigned char data_bytes_per_record_I8[8];
	unsigned char footer_bytes_I4[4];						/* �悭�킩��Ȃ����ǁA���炭�t�b�^�[ */

	/* �c��͋��炭�f�[�^�̈�s�ڂ̒��� - �����܂� */
};

struct REMOS_HEADER_LINE_ALOS_PAL {								/* �C���[�W�t�@�C���f�B�X�N���v�^���R�[�h */
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
	unsigned char header_bytes_I4[4];						/* �悭�킩��Ȃ����ǁA�����炭�w�b�_�T�C�Y */
	unsigned char data_bytes_per_record_I8[8];
	unsigned char footer_bytes_I4[4];						/* �悭�킩��Ȃ����ǁA���炭�t�b�^�[ */

	/* �c��͋��炭�f�[�^�̈�s�ڂ̒��� - �����܂� */
};

#ifdef __cplusplus
extern "C" {
#endif

int remos_open( struct REMOS_FILE_CONTAINER *cont, char *filename, int type );								/* �t�@�C�����J�� */
int remos_close( struct REMOS_FILE_CONTAINER *cont );														/* �t�@�C������� */

/* private �t�@�C������ */
int remos_fopen( struct REMOS_FILE_CONTAINER *cont, char *filename );
int remos_fclose( struct REMOS_FILE_CONTAINER *cont );
int remos_fseek( struct REMOS_FILE_CONTAINER *cont, long long move, enum remos_file_move_method method );
int remos_fread( struct REMOS_FILE_CONTAINER *cont, unsigned char *dest, unsigned int num );

/* �^�C�v���蓮�ݒ� */
int remos_set_type_BIL( struct REMOS_FILE_CONTAINER *cont, int file_header, int header, int footer, int lines, int width, int bands ,int bits );
int remos_set_type_BSQ( struct REMOS_FILE_CONTAINER *cont, int file_header, int header, int footer, int lines, int width, int bits );

float remos_get_pixel_value( struct REMOS_BAND *band, int pos );													/* �w��o���h�̎w��ʒu����1�s�N�Z�����炤 */
int remos_get_line_pixels( struct REMOS_BAND *band, unsigned char *buf, int line, int from, int count );	/* �w��o���h�̎w��ʒu����count�s�N�Z�����߂���  */
float remos_get_ranged_pixel( struct REMOS_BAND *band, float val );											/* �w��o���h�̃����W�ɉ�����1�o�C�g���� */

void remos_make_pixels( struct REMOS_FILE_CONTAINER *cont, struct REMOS_PIXELS *pixs );						/* PIXELS���̔z��������m�� */
void remos_free_pixels( struct REMOS_PIXELS *pixs );														/* �m�ۂ��ꂽPIXELS��������� */

int remos_check_file_type( struct REMOS_FILE_CONTAINER *cont );												/* �t�@�C���^�C�v�������ݒ� */
int remos_read_file( struct REMOS_FILE_CONTAINER *cont );													/* �w�肳�ꂽ�^�C�v�Ńt�@�C������ */
int remos_make_hist( struct REMOS_BAND *band );																/* �q�X�g�O�������� */
void remos_calc_auto_range( struct REMOS_BAND *band, double per, int topbottom );							/* �����Ń_�C�i�~�b�N�����W�ݒ� */
void remos_set_range( struct REMOS_BAND *band, int bottom, int top );										/* �_�C�i�~�b�N�����W(?)�ݒ� */

float remos_data_to_value_band( struct REMOS_BAND *band, unsigned char *data, int index );  				/* �o���h�̃��[���ɑΉ��������@�Ńf�[�^�[�����o�� */
float remos_data_to_value_format( unsigned char *data, int len, int endian, int format );					/* �G���f�B�A���Ə����ɂ��������ăf�[�^��ǂݍ��� ( float�ŕԂ� ) */

struct REMOS_BAND *remos_get_band( struct REMOS_FILE_CONTAINER *cont, int num );							/* �w�肵���o���h�ԍ��̃o���h�𓾂� */

/* ���[�e�B���e�B */
void remos_latlon_to_xy( int *i, int *j, int w, int h, float lat, float lon, float ox, float oy, float ax, float ay, float bx, float by, float cx, float cy );

#ifdef __cplusplus
}
#endif

#endif

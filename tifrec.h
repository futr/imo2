/* tifrec�w�b�_�[�t�@�C�� */
/* 2013-04-04 */

/* TAG */
/* COMPRESS = 1�Ŕ񈳏k  */
/* BIT_PER_SAMPLE���r�b�g�� */
/* PHOTOMETRIC = 0�̓O���[�X�P�[�� ( 0 = white ) */
/* PHOTOMETRIC = 1�̓O���[�X�P�[�� ( 0 = black ) */
/* PHOTOMETRIC = 2�̓J���[ ( PlanarConfiguration = 1��RGB ) */
/* STRIP_OFFSET : �����炭�摜�̃I�t�Z�b�g*/
/* SAMPLE_PER_PIX : �s�N�Z��������̃T���v���� */
/* ROW_PER_STRIP : �X�g���b�v������̍s�� ( StripsPerImage = (ImageLength + RowsPerStrip - 1) / RowsPerStrip ) */
/* STRIP_BYTE_COUNT : �X�g���b�v���̃o�C�g�� */
/* X_RESOLUTION : �𑜒P�ʂ�����̃s�N�Z���� */
/* Y_RESOLUTION : �𑜒P�ʂ�����̃s�N�Z���� */
/* PLANAR_CONFIG = 1�Ńf�[�^�͗אڂ��đ��� ( RGBRGBRGB ) */
/* PLANAR_CONFIG = 2�ł悭�킩��Ȃ� */
/* RESOLUTION_UNIT = 1�ŒP�ʂȂ� */
/* RESOLUTION_UNIT = 2�ŃC���` */
/* RESOLUTION_UNIT = 3�ŃZ���` */


#ifndef TIFREC_H_INCLUDED
#define TIFREC_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TIFREC_MAX_FILENAME 1024

enum tifrec_tag{
	TIFREC_TAG_IMG_WIDTH        = 256,
	TIFREC_TAG_IMG_LENGTH       = 257,
	TIFREC_TAG_BIT_PER_SAMPLE   = 258,
	TIFREC_TAG_COMPRESS         = 259,
	TIFREC_TAG_PHOTOMETRIC      = 262,
	TIFREC_TAG_STRIP_OFFSET     = 273,
	TIFREC_TAG_SAMPLE_PER_PIX   = 277,
	TIFREC_TAG_ROW_PER_STRIP    = 278,
	TIFREC_TAG_STRIP_BYTE_COUNT = 279,
	TIFREC_TAG_X_RESOLUTION     = 282,
	TIFREC_TAG_Y_RESOLUTION     = 283,
	TIFREC_TAG_PLANAR_CONFIG    = 284,
	TIFREC_TAG_RESOLUTION_UNIT  = 296,
	TIFREC_TAG_SAMPLE_FORMAT    = 339,
};

enum tifrec_result {
	TIFREC_RES_FAILED,
	TIFREC_RES_READ_FAILED,
	TIFREC_RES_RECOG_FAILED,
	TIFREC_RES_SUCCEED
};

enum tifrec_endian {
	ENDIAN_BIG,
	ENDIAN_LITTLE,
	ENDIAN_UNKNOWN
};

struct TIFREC_IFD_ENTRY_USINT_PACK {
	unsigned int num;
	unsigned int denom;
};

struct TIFREC_IFD_ENTRY_INT_PACK {
	int num;
	int denom;
};

struct TIFREC_IFD {
	unsigned int count;
	
	struct TIFREC_IFD_ENTRY *entry;
	
	struct TIFREC_IFD *next;
};

struct TIFREC_IFD_ENTRY {
	unsigned int tag;
	unsigned int type;
	unsigned int count;
	double value;

	unsigned int pointer;

	double *value_double;
	float *value_float;
	unsigned int *value_usint;
	int *value_int;
	
	unsigned short *value_usshort;
	short *value_short;
	
	unsigned char *value_uschar;
	char *value_char;
	unsigned char *value_undef;
	
	unsigned int *value_usint_num;
	unsigned int *value_usint_denom;
	
	int *value_int_num;
	int *value_int_denom;
	
	char *value_ascii;
};

struct TIFREC_FILE_CONTAINER {
	FILE *fp;
	char filename[TIFREC_MAX_FILENAME];
	
	int endian;
	
	int ifds;
	struct TIFREC_IFD *ifd;
};

#ifdef __cplusplus
extern "C" {
#endif

int tifrec_open( struct TIFREC_FILE_CONTAINER *tr, char *filename );												/* �t�@�C�����J�� */
int tifrec_close( struct TIFREC_FILE_CONTAINER *tr );																/* �t�@�C������� */

int tifrec_check_tiff( struct TIFREC_FILE_CONTAINER *tr );															/* TIFF�����ׂ� */

void tifrec_get_value( struct TIFREC_FILE_CONTAINER *tr, struct TIFREC_IFD_ENTRY *entry, unsigned int offset );		/* entry���̃^�O�ɉ�����value��ǂ� */
void tifrec_get_data( struct TIFREC_FILE_CONTAINER *tr, void *data, int pos, int count );							/* �w��ꏊ�Ɏw��ꏊ����w��ǂ� */
double tifrec_get_usdata( struct TIFREC_FILE_CONTAINER *tr, int pos, int bytes );									/* ���������Ƃ��ăf�[�^�ǂݍ��� */

#ifdef __cplusplus
}
#endif

#endif

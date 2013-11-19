#include "ebmp.h"

int ebmp_create_file_open( EBMP_FILE *bmp, char *filename, int width, int height )
{
	/* �������ݗp�t�@�C�����J�� */

	/* �J�� */
	if ( ( bmp->fp = fopen( filename, "wb" ) ) == NULL ) {
		/* �G���[ */
		return 0;
	}

	/* �l�ݒ� */
	strcpy( bmp->filename, filename );

	bmp->width  = width;
	bmp->height = height;
	
	/* �o�C�g���A���C���T�C�Y���� */
	bmp->bits = 24;
	bmp->line_size = bmp->bits / 8.0 * bmp->width;
	
	/* �A���C�������g */
	if ( bmp->line_size % 4 != 0 ) {
		bmp->line_size = ( bmp->line_size / 4 + 1 ) * 4;
	}
	
	/* �w�b�_�[��ݒ� ( ����24bit��p ) */
	bmp->file_header.bfType    = 0x4D42;
	bmp->file_header.bfRes1    = 0;
	bmp->file_header.bfRes2    = 0;
	bmp->file_header.bfOffBits = EBMP_FILEHEADER_SIZE + EBMP_INFOHEADER_SIZE;
	bmp->file_header.bfSize    = bmp->file_header.bfOffBits + bmp->line_size * height;
	
	/* INFO�w�b�_�[��ݒ� ( ����INFO�w�b�_�[�݂̂��g�p ) */
	bmp->info_header.biSize          = 40;
	bmp->info_header.biWidth         = width;
	bmp->info_header.biHeight        = height;
	bmp->info_header.biPlanes        = 1;
	bmp->info_header.biBitCount      = bmp->bits;
	bmp->info_header.biCompression   = EBMP_COMP_RGB;
	bmp->info_header.biSizeImage     = 0;
	bmp->info_header.biXPelsPerMeter = 3000;		/* �K�� */
	bmp->info_header.biYPelsPerMeter = 3000;
	bmp->info_header.biClrUsed       = 0;
	bmp->info_header.biClrImportant  = 0;

	/* �w�b�_�[�T�C�Y */
	bmp->header_size = EBMP_FILEHEADER_SIZE + EBMP_INFOHEADER_SIZE;

	/* �w�b�_�[���������� */
	ebmp_write_file_header( bmp );
	ebmp_write_info_header( bmp );
	
	return 1;
}

void ebmp_write_file_header( EBMP_FILE *bmp )
{
	/* �t�@�C���w�b�_�[�������� */
	unsigned char buf[2];
	
	/* �擪�� */
	fseek( bmp->fp, 0, SEEK_SET );
	
	/* BM */
	buf[0] = 'B';
	buf[1] = 'M';
	fwrite( buf, 1, 2, bmp->fp );
	
	/* ���̑� */
	fwrite( &bmp->file_header.bfSize,    1, 4, bmp->fp );
	fwrite( &bmp->file_header.bfRes1,    1, 2, bmp->fp );
	fwrite( &bmp->file_header.bfRes2,    1, 2, bmp->fp );
	fwrite( &bmp->file_header.bfOffBits, 1, 4, bmp->fp );
}

void ebmp_write_info_header( EBMP_FILE *bmp )
{
	/* INFO�w�b�_�[���������� */	
	
	/* �������݈ʒu�� */
	fseek( bmp->fp, EBMP_FILEHEADER_SIZE, SEEK_SET );
	
	/* �������� */
	fwrite( &bmp->info_header.biSize,          1, 4, bmp->fp );
	fwrite( &bmp->info_header.biWidth,         1, 4, bmp->fp );
	fwrite( &bmp->info_header.biHeight,        1, 4, bmp->fp );
	fwrite( &bmp->info_header.biPlanes,        1, 2, bmp->fp );
	fwrite( &bmp->info_header.biBitCount,      1, 2, bmp->fp );
	fwrite( &bmp->info_header.biCompression,   1, 4, bmp->fp );
	fwrite( &bmp->info_header.biSizeImage,     1, 4, bmp->fp );
	fwrite( &bmp->info_header.biXPelsPerMeter, 1, 4, bmp->fp );
	fwrite( &bmp->info_header.biYPelsPerMeter, 1, 4, bmp->fp );
	fwrite( &bmp->info_header.biClrUsed,       1, 4, bmp->fp );
	fwrite( &bmp->info_header.biClrImportant,  1, 4, bmp->fp );
}

int ebmp_create_file_close( EBMP_FILE *bmp )
{
	/* �t�@�C������� */
	fclose( bmp->fp );
	bmp->fp = NULL;
	
	return 1;
}

void ebmp_write_point_rgb_24( EBMP_FILE *bmp, int x, int y, int r, int g, int b )
{
	/* bmp��̓���̈ʒu��rgb�Œl���������� */
	unsigned char pix[3];
	
	pix[0] = b;
	pix[1] = g;
	pix[2] = r;
	
	/* �������ݏꏊ�ֈړ� */
	fseek( bmp->fp, bmp->header_size + ( ( bmp->height - 1 - y ) * bmp->line_size + x * 3 ), SEEK_SET );
	
	/* �������� */
	fwrite( pix, 1, 3, bmp->fp );
}

void ebmp_write_line( EBMP_FILE *bmp, int y, void *data, size_t size )
{
	/* ���C���P�ʂŏ������� */

	/* �w��ʒu�� */
	fseek( bmp->fp, bmp->header_size + ( bmp->height - 1 - y ) * bmp->line_size, SEEK_SET );

	/* �������� */
	fwrite( data, size, 1, bmp->fp );
}

void ebmp_write_line_seq_start( EBMP_FILE *bmp )
{
	/* ���ԏ������݊J�n */

	/* �w��ʒu�� */
	fseek( bmp->fp, bmp->header_size, SEEK_SET );
}

void ebmp_write_line_seq( EBMP_FILE *bmp, void *data, size_t size )
{
	/* ���ԏ������� */
	fwrite( data, 1, size, bmp->fp );
}

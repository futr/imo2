/* TIFF�ȈՉ�̓v���O���� 1.0 modified for imo2 */
/* �I���W�i����tifrec�ł͂Ȃ������������Ă��� */
/* 2013-04-04 */
/* (c) 2013 Masato Takahashi */
/* ilce.ma@gmail.com */
/* sizeof( int )	 : 4 */
/* sizeof( short )	 : 2 */
/* sizeof( char )	 : 1 */
/* sizeof( float )	 : 4 */
/* sizeof( double )	 : 8 */
/* ���O�� */

#include "tifrec.h"

int tifrec_open( struct TIFREC_FILE_CONTAINER *tr, char *filename )
{
	/* TIFF�t�@�C�����J���Ď��ʂ܂Ŏ��݂� */
	char buf[64];
	unsigned int offset;
	struct TIFREC_IFD *next;
	unsigned int count;
	unsigned int i;
	unsigned int ifds;
	
	/* ������ */
	tr->ifd = NULL;
	strcpy( tr->filename, filename );
	
	/* �t�@�C�����J�� */
	if ( ( tr->fp = fopen( filename, "rb" ) ) == NULL ) {
		/* �I�[�v�����s */
		return TIFREC_RES_READ_FAILED;
	}
	
	/* �w�b�_�`�F�b�N */
	if ( !tifrec_check_tiff( tr ) ) {
		fclose( tr->fp );
		tr->fp = NULL;
		
		return TIFREC_RES_RECOG_FAILED;
	}
	
	/* IFD�ւ̃I�t�Z�b�g�擾 */
	offset = tifrec_get_usdata( tr, 4, 4 );
	
	/* �擪��IFD�쐬 */
	tr->ifd = (struct TIFREC_IFD *)malloc( sizeof(struct TIFREC_IFD) );
	next = tr->ifd;
	
	/* IFD����J��Ȃ������Ă��� */
	for ( count = 1; ; count++ ) {
		/* �G���g�����擾 */
		next->count = tifrec_get_usdata( tr, offset, 2 );
		offset += 2;
		
		/* �G���g���̈�m�� */
		next->entry = (struct TIFREC_IFD_ENTRY *)malloc( sizeof(struct TIFREC_IFD_ENTRY) * next->count );
		
		/* �G���g���ǂݍ��� */
		for ( i = 0; i < next->count; i++ ) {
			next->entry[i].tag   = tifrec_get_usdata( tr, offset, 2 );
			next->entry[i].type  = tifrec_get_usdata( tr, offset + 2, 2 );
			next->entry[i].count = tifrec_get_usdata( tr, offset + 4, 4 );
			offset += 8;
			
			/* value�ǂݍ��� */
			tifrec_get_value( tr, &next->entry[i], offset );
			
			/* offset���� */
			offset += 4;
		}
		
		/* ����IFD��OFFSET�擾 */
		offset = tifrec_get_usdata( tr, offset, 4 );
		
		/* �ǂݏI�������`�F�b�N */
		if ( offset == 0 ) {
			/* �ǂݏI���� */
			next->next = NULL;
			
			break;
		}
		
		/* �ǂݏI���ĂȂ��̂Ŏ��� */
		next->next = (struct TIFREC_IFD *)malloc( sizeof(struct TIFREC_IFD) );
		next = next->next;
	}
	
	/* ���ۑ� */
	tr->ifds = count;
	
	/* ���� */
	return TIFREC_RES_SUCCEED;
}

int tifrec_close( struct TIFREC_FILE_CONTAINER *tr )
{
	/* ���� */
	int i;
	struct TIFREC_IFD *next, *next_next;
	
	if ( tr->fp != NULL ) {
		fclose( tr->fp );
		tr->fp = NULL;
	}
	
	/* IFD���^�O���ĊJ�� */
	next = tr->ifd;
	
	/* next��NULL�ɂȂ�܂ł����� */
	while ( next != NULL ) {
		next_next = next->next;
		
		free( next->entry->value_ascii );
		free( next->entry->value_double );
		free( next->entry->value_float );
		free( next->entry->value_usint );
		free( next->entry->value_int );
		free( next->entry->value_usshort );
		free( next->entry->value_short );
		free( next->entry->value_uschar );
		free( next->entry->value_char );
		free( next->entry->value_undef );
		free( next->entry->value_usint_num );
		free( next->entry->value_usint_denom );
		free( next->entry->value_int_num );
		free( next->entry->value_int_denom );
		
		free( next->entry );
		free( next );
		
		next = next_next;
	}
	
	tr->ifd = NULL;
	
	/* �Ƃ肠�������� */
	return TIFREC_RES_SUCCEED;
}

int tifrec_check_tiff( struct TIFREC_FILE_CONTAINER *tr )
{
	/* TIFF�����ׂ� ( �ȒP�� ) */
	char type[3];
	int ret;
	
	/* �擪�� */
	fseek( tr->fp, 0, SEEK_SET );
	
	/* MM��II��ǂݍ��� */
	fread( type, 1, 2, tr->fp );
	type[2] = '\0';
	
	/* MM��II���`�F�b�N */
	if ( strcmp( "MM", type ) == 0 ) {
		/* ���g���[���`�� */
		tr->endian = ENDIAN_BIG;
		
		ret = 1;
	} else if ( strcmp( "II", type ) == 0 ) {
		/* �C���e���`�� */
		tr->endian = ENDIAN_LITTLE;
		
		ret = 1;
	} else {
		/* �s���`�� */
		tr->endian = ENDIAN_UNKNOWN;
		
		ret = 0;
	}
	
	return ret;
}

double tifrec_get_usdata( struct TIFREC_FILE_CONTAINER *tr, int pos, int bytes )
{
	/* ���������Ƃ��ăf�[�^�ǂݍ��� */
	int i;
	unsigned char *buf;
	unsigned int ret;
	
	ret = 0;
	
	/* �o�b�t�@�m�� */
	buf = (unsigned char *)malloc( bytes );
	
	/* �w��ꏊ�� */
	fseek( tr->fp, pos, SEEK_SET );
	
	/* �o�b�t�@�֓ǂݍ��� */
	fread( buf, 1, bytes, tr->fp );

	/* �ǂݍ��� */
	for ( i = 0; i < bytes; i++ ) {
		if ( tr->endian == ENDIAN_BIG ) {
			ret += buf[i] * pow( 256, bytes - 1 - i );
		} else {
			ret += buf[i] * pow( 256, i );
		}
	}
	
	return ret;
}

void tifrec_get_value( struct TIFREC_FILE_CONTAINER *tr, struct TIFREC_IFD_ENTRY *entry, unsigned int offset )
{
	/* entry��type�ɉ����ăf�[�^�ǂݍ��� */
	/* �ȈՃA�N�Z�X�p��value�ƁA�����ƃA�N�Z�X�p��value_type������ */
	unsigned int pointer;
	double value_1, value_2;
	unsigned int value_1_usint, value_2_usint;
	unsigned int i;
	
	/* �|�C���^������ */
	entry->value_double  = NULL;
	entry->value_float   = NULL;
	entry->value_usint   = NULL;
	entry->value_int     = NULL;
	entry->value_usshort = NULL;
	entry->value_short   = NULL;
	entry->value_uschar  = NULL;
	entry->value_char    = NULL;
	entry->value_undef   = NULL;
	entry->value_usint_num   = NULL;
	entry->value_usint_denom = NULL;
	entry->value_int_num     = NULL;
	entry->value_int_denom   = NULL;
	entry->value_ascii       = NULL;
	
	/* type�ɂ���ēǂݕ���ς��� */
	switch ( entry->type ) {
		case 1: {
			/* unsigned char */
			
			/* �̈�m�� */
			entry->value_uschar = malloc( sizeof( unsigned char ) * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_uschar[i], entry->pointer + ( i * 1 ), 1 );
			}
			
			entry->value = tifrec_get_usdata( tr, offset, 1 );
			break;
		}
		
		case 2: {
			/* string �K���ɓǂݍ��� */
			
			/* �̈�m�� */
			entry->value_ascii = malloc( entry->count );
			
			if ( entry->count > 4 ) {
				/* 4���傫�����̓|�C���^�o�R�ŃA�N�Z�X */
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}
			
			/* �ǂݍ��� */
			fseek( tr->fp, entry->pointer, SEEK_SET );
			fread( entry->value_ascii, 1, entry->count, tr->fp );
			
			break;
		}

		case 3: {
			/* unsigned short */

			/* �̈�m�� */
			entry->value_usshort = malloc( 2 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 2 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_usshort[i], entry->pointer + ( i * 2 ), 2 );
			}

			entry->value = (unsigned short)tifrec_get_usdata( tr, offset, 2 );
			
			break;
		}
		
		case 4: {
			/* unsigned int */

			/* �̈�m�� */
			entry->value_usint = malloc( 4 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_usint[i], entry->pointer + ( i * 4 ), 4 );
			}
			
			entry->value = (unsigned int)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}
		
		case 5: {
			/* unsigned int / unsigned int */
			
			/* �̈�m�� */
			entry->value_usint_num   = malloc( 4 * entry->count );
			entry->value_usint_denom = malloc( 4 * entry->count );
			
			/* �|�C���^�擾 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_usint_num[i],   entry->pointer + ( i * 4 ), 4 );
				tifrec_get_data( tr, &entry->value_usint_denom[i], entry->pointer + ( ( i + 1 ) * 4 ), 4 );
			}
			
			value_1 = tifrec_get_usdata( tr, entry->pointer, 4 );
			value_2 = tifrec_get_usdata( tr, entry->pointer + 4, 4 );
			
			entry->value = value_1 / value_2;
			
			break;
		}
		
		case 6: {
			/* signed char 8bit */
			
			/* �̈�m�� */
			entry->value_char = malloc( 1 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_char[i], entry->pointer + i, 1 );
			}
			
			entry->value = (char)tifrec_get_usdata( tr, offset, 1 );
			
			break;
		}

		case 7: {
			/* undefine */
			
			/* �̈�m�� */
			entry->value_undef = malloc( 1 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_undef[i], entry->pointer + ( i * 1 ), 1 );
			}
			
			entry->value = tifrec_get_usdata( tr, offset, 1 );
			
			break;
		}

		case 8: {
			/* signed short 16bit */
			
			/* �̈�m�� */
			entry->value_short = malloc( 2 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 2 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_short[i], entry->pointer + ( i * 2 ), 2 );
			}

			entry->value = (short)tifrec_get_usdata( tr, offset, 2 );
			
			break;
		}
		
		case 9: {
			/* signed int 32bit */
			
			/* �̈�m�� */
			entry->value_int = malloc( 4 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_int[i], entry->pointer + ( i * 4 ), 4 );
			}

			entry->value = (int)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}
		
		case 10: {
			/* signed int / signed int */
			
			/* �̈�m�� */
			entry->value_int_num   = malloc( 4 * entry->count );
			entry->value_int_denom = malloc( 4 * entry->count );
			
			/* �|�C���^�擾 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_int_num[i],   entry->pointer + ( i * 4 ), 4 );
				tifrec_get_data( tr, &entry->value_int_denom[i], entry->pointer + ( ( i + 1 ) * 4 ), 4 );
			}
			
			value_1 = (int)tifrec_get_usdata( tr, entry->pointer, 4 );
			value_2 = (int)tifrec_get_usdata( tr, entry->pointer + 4, 4 );
			
			entry->value = value_1 / value_2;
			
			break;
		}

		case 11: {
			/* float */
			
			/* �̈�m�� */
			entry->value_float = malloc( 4 * entry->count );
			
			/* �|�C���^�擾 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_float[i], entry->pointer + ( i * 4 ), 4 );
			}

			entry->value = (float)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}

		case 12: {
			/* double */
			
			/* �̈�m�� */
			entry->value_double = malloc( 8 * entry->count );
			
			/* �|�C���^�擾 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* �l�擾 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_double[i], entry->pointer + ( i * 8 ), 8 );
			}

			tifrec_get_data( tr, &entry->value, entry->pointer, 8 );
			
			break;
		}
		
		default: {
			/* �킩��Ȃ��͓̂K�� */
			
			entry->value = tifrec_get_usdata( tr, offset, 4 );
			break;
		}
	}
}

void tifrec_get_data( struct TIFREC_FILE_CONTAINER *tr, void *data, int pos, int count )
{
	/* �w��ꏊ�Ɏw��ꏊ����w��ǂݍ��� */
	int i;
	unsigned char *buf;
	
	/* �o�b�t�@�m�� */
	buf = (unsigned char *)malloc( count );
	
	/* �w��ꏊ�� */
	fseek( tr->fp, pos, SEEK_SET );
	
	/* �o�b�t�@�֓ǂݍ��� */
	fread( buf, 1, count, tr->fp );

	/* �ǂݍ��� */
	for ( i = 0; i < count; i++ ) {
		if ( tr->endian == ENDIAN_BIG ) {
			( (unsigned char *)data )[i] = buf[count - 1 - i];
		} else {
			( (unsigned char *)data )[i] = buf[i];
		}
	}

    free( buf );
}


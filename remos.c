#include "remos.h"

/* !!!!PALSAT�Ή��̂��߂����蒆 2013 0607 */
/* ����float remos_data_to_value_band( struct REMOS_BAND *band, unsigned char *data )���������Ă邯�ǂ܂����������m�F���Ă��Ȃ� */

/* �ėp�q���f�[�^���[�_�[���C�u���� ver1.2 */
/* ����8bit�ɂ����Ή����Ă��Ȃ� ( �������PALSAR��16bit�����͑Ή����Ă��� ) */
/* �Ȉ�TIFF�A�N�Z�X */
/* 8bit�񈳏k�I�����[ */
/* GeoTiff�̃^�O�͂킩��Ȃ� */
/* bands 0, 1, 2�̏���RGB */

int remos_open( struct REMOS_FILE_CONTAINER *cont, char *filename, int type )
{
	/* �t�@�C�����J�� �J���Ȃ�������RET_FAILED, �ǂݍ��݂Ɏ��s������RET_READ_FAILED,  */

	/* ������ */
	cont->type       = type;
	cont->band_count = 0;
	cont->bands      = NULL;
	cont->fp         = NULL;

    /* �t�@�C�����R�s�[ */
    memcpy( cont->file_name, filename, strlen( filename ) );
    cont->file_name[strlen( filename )] = '\0';

	/* �t�@�C�����J�� */
	if ( ( cont->fp = fopen( filename, "rb" ) ) == NULL ) {
		/* ���s */
		return REMOS_RET_FAILED;
	}
	
	/* �蓮���ʃ��[�h�Ȃ牽�����Ȃ� */
	if ( type == REMOS_FILE_TYPE_NOT_RECOG ) {
		return REMOS_RET_SUCCEED;
	}
	
	/* �����ݒ胂�[�h���H */
	if ( type == REMOS_FILE_TYPE_AUTO ) {
		/* �^�C�v���� */
		cont->type = remos_check_file_type( cont );
		
		/* ���ʂł����H */
		if ( cont->type == REMOS_FILE_TYPE_UNKNOWN ) {
			/* ���ʎ��s */
			return REMOS_RET_READ_FAILED;
		}
	}
	
	/* ���ۂɓǂݍ��� */
	if ( remos_read_file( cont ) == REMOS_RET_FAILED ) {
		/* ���s���� */
		return REMOS_RET_READ_FAILED;
	}
	
	/* ���� */
	return REMOS_RET_SUCCEED;
}

int remos_close( struct REMOS_FILE_CONTAINER *cont )
{
	/* �t�@�C������� */
	
	fclose( cont->fp );
	cont->fp = NULL;
	
	/* �m�ۂ��ꂽ�o���h����� */
	free( cont->bands );
	cont->bands = NULL;
	
	return REMOS_RET_SUCCEED;
}

int remos_check_file_type( struct REMOS_FILE_CONTAINER *cont )
{
	/* �t�@�C���^�C�v�𒲂ׂ� */
	char buf[64];
	char str_buf[32];
	
	/* �擪��63�o�C�g��ǂ�ł݂� */
	fread( buf, 1, 63, cont->fp );
	buf[63] = '\0';
	
	/* REMO10�`���`�F�b�N */
	memcpy( str_buf, buf, 8 );
	str_buf[8] = '\0';
	
	if ( !strcmp( str_buf, "SUGIMURA" ) ) {
		/* ��w�b�_�[�t��remo10�`�������� */
		return REMOS_FILE_TYPE_BIL_R10;
	}
	
	/* ALOS��BSQ�`�F�b�N ( ���ɊÂ� ) */
	memcpy( str_buf, buf + 48, 2 );
	str_buf[2] = '\0';
	
	if ( !strcmp( str_buf, "AL" ) ) {
		/* ALOS�pBSQ������ */
		
		/* PAL�� ( �G ) */
		memcpy( str_buf, buf + 52, 4 );
		str_buf[4] = '\0';
		
		if ( !strcmp( str_buf, "PSRC" ) ) {
			/* PALSAR1.5������ */
			return REMOS_FILE_TYPE_BSQ_ALOS_PAL;
		} else if ( !strcmp( str_buf, "PSRB" ) ) { 
			/* PALSAR1.1������ */
			return REMOS_FILE_TYPE_BSQ_ALOS_PAL_11;
		} else {
			/* AV2��PRISM������ */
			return REMOS_FILE_TYPE_BSQ_ALOS;
		}
	}
	
	/* TIFF�`�� ( ���ɊÂ� ) */
	memcpy( str_buf, buf, 2 );
	str_buf[2] = '\0';
	
	if ( strcmp( str_buf, "II" ) == 0 || strcmp( str_buf, "MM" ) == 0 ) {
		return REMOS_FILE_TYPE_TIFF;
	}
	
	/* ������Ȃ����� */
	return REMOS_FILE_TYPE_UNKNOWN;
}

int remos_read_file( struct REMOS_FILE_CONTAINER *cont )
{
	/* �w�肳�ꂽcont->type�ɏ]���āA�t�@�C���ǂݍ��� */
	struct TIFREC_FILE_CONTAINER tr;
	struct TIFREC_IFD *next;
	unsigned char *buf;
	unsigned char buf_short[2];
	unsigned char buf_int[4];
	unsigned int i;
	
	int err;
	int width;
	int height;
	int header;
	int color;
	int sample;
    int bits;
    int set_sample;
    int sample_format;
	
	switch ( cont->type ) {
		case REMOS_FILE_TYPE_BSQ_ALOS_PAL_11:								/* ALOS��BSQ������PAL��1.1������ */
			/* �R���e�i��ݒ� */
			cont->band_count = 2;
			cont->bands      = malloc( sizeof(struct REMOS_BAND) * 2 );

            /* �t�@�C���w�b�_�̑傫�����擾���� */
            buf = malloc( 4 );

			/* �w�b�_�ǂݍ��� */
			fseek( cont->fp, 8, SEEK_SET );
			fread( buf, 1, 4, cont->fp );

            /* �w�b�_���� */
            header = remos_data_to_value( buf, 4, REMOS_ENDIAN_BIG );

            /* �J�� */
            free( buf );

			/* ���擾�̂��߂Ƀo�b�t�@�쐬 */
			buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS_PAL) );

			/* �w�b�_�ǂݍ��� */
			fseek( cont->fp, 180, SEEK_SET );
			fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS_PAL), cont->fp );
			
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].band_count  = 2;

				cont->bands[i].color         = REMOS_BAND_COLOR_PACKED_USINT_2;
				cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands[i].endian        = REMOS_ENDIAN_BIG;
				
				cont->bands[i].fp          = cont->fp;
				cont->bands[i].band_num    = i;
				cont->bands[i].bits        = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->bit_per_pix_I4,   4 );
				cont->bands[i].line_width  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->record_length_I6, 6 );
				cont->bands[i].line_count  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->line_per_band_I8,  8 );
				cont->bands[i].line_header = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->header_bytes_I4,  4 );
				cont->bands[i].line_footer = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->footer_bytes_I4,  4 );

				cont->bands[i].sample_per_pix  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->pix_per_data_I4, 4 );
	            cont->bands[i].byte_per_sample = cont->bands->bits / 8;

				cont->bands[i].line_img_width = ( cont->bands[i].line_width - cont->bands[i].line_header - cont->bands[i].line_footer ) / ( cont->bands[i].byte_per_sample * cont->bands[i].sample_per_pix );
				
				cont->bands[i].header       = header;				/* �{����+180�̋C���������A+180�͖��������ǂ�, cont->bands->line_width��header�ɒu�������Ă݂� */

				cont->bands[i].range_top    = pow( 2, cont->bands->bits ) - 1;
				cont->bands[i].range_bottom = 0;
				cont->bands[i].range_max    = pow( 2, cont->bands->bits ) - 1;
				cont->bands[i].range_min    = 0;
			}

			/* �R���e�i�ɉ摜�T�C�Y��ݒ� */
			cont->img_height = cont->bands[0].line_count;
			cont->img_width  = cont->bands[0].line_img_width;

			/* �o�b�t�@����� */
			free( buf );

			return REMOS_RET_SUCCEED;
		
		case REMOS_FILE_TYPE_BSQ_ALOS:										/* ALOS��BSQ������ */
		case REMOS_FILE_TYPE_BSQ_ALOS_PAL:
			/* �R���e�i��ݒ� */
			cont->band_count = 1;
			cont->bands      = malloc( sizeof(struct REMOS_BAND) );

            /* �t�@�C���w�b�_�̑傫�����擾���� */
            buf = malloc( 4 );

			/* �w�b�_�ǂݍ��� */
			fseek( cont->fp, 8, SEEK_SET );
			fread( buf, 1, 4, cont->fp );

            /* �w�b�_���� */
            header = remos_data_to_value( buf, 4, REMOS_ENDIAN_BIG );

            /* �J�� */
            free( buf );

			/* PALSAR���ۂ��ŏ����Ⴄ */
			if ( cont->type == REMOS_FILE_TYPE_BSQ_ALOS_PAL ) {
				/* ���擾�̂��߂Ƀo�b�t�@�쐬 */
				buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS_PAL) );

				/* �w�b�_�ǂݍ��� */
				fseek( cont->fp, 180, SEEK_SET );
				fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS_PAL), cont->fp );
				
				/* �o���h�ɐݒ�K�p PALSAR�ł�line_header�����̈Ӗ����قȂ邽�ߐ��m�łȂ� */
				cont->bands->band_count = 1;

				cont->bands->color         = REMOS_BAND_COLOR_BW;
				cont->bands->sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands->endian        = REMOS_ENDIAN_BIG;					/* bigendian�ő��v���낤�� */
				
				cont->bands->fp          = cont->fp;
				cont->bands->band_num    = 0;
				cont->bands->bits        = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->bit_per_pix_I4,   4 );
				cont->bands->line_width  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->record_length_I6, 6 );
				cont->bands->line_count  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->record_count_I6,  6 );
				cont->bands->line_header = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->header_bytes_I4,  4 );
				cont->bands->line_footer = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->footer_bytes_I4,  4 );
			} else {
				/* ���擾�̂��߂Ƀo�b�t�@�쐬 */
				buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS) );

				/* �w�b�_�ǂݍ��� */
				fseek( cont->fp, 180, SEEK_SET );
				fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS), cont->fp );
				
				/* �o���h�ɐݒ�K�p PALSAR�ł�line_header�����̈Ӗ����قȂ邽�ߐ��m�łȂ� */
				cont->bands->band_count = 1;

				cont->bands->color         = REMOS_BAND_COLOR_BW;
				cont->bands->sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands->endian        = REMOS_ENDIAN_BIG;					/* bigendian�ő��v���낤�� */
				
				cont->bands->fp          = cont->fp;
				cont->bands->band_num    = 0;
				cont->bands->bits        = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->bit_per_pix_I4,   4 );
				cont->bands->line_width  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->record_length_I6, 6 );
				cont->bands->line_count  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->record_count_I6,  6 );
				cont->bands->line_header = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->header_bytes_I4,  4 );
				cont->bands->line_footer = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->footer_bytes_I4,  4 );
			}

            cont->bands->byte_per_sample = cont->bands->bits / 8;
            cont->bands->sample_per_pix  = 1;				/* �Ƃ肠����1 */

			cont->bands->line_img_width = ( cont->bands->line_width - cont->bands->line_header - cont->bands->line_footer ) / cont->bands->byte_per_sample;
			
			cont->bands->header       = header;				/* �{����+180�̋C���������A+180�͖��������ǂ�, cont->bands->line_width��header�ɒu�������Ă݂� */

			cont->bands->range_top    = pow( 2, cont->bands->bits ) - 1;
			cont->bands->range_bottom = 0;
			cont->bands->range_max    = pow( 2, cont->bands->bits ) - 1;
			cont->bands->range_min    = 0;

			/* �R���e�i�ɉ摜�T�C�Y��ݒ� */
			cont->img_height = cont->bands->line_count;
			cont->img_width  = cont->bands->line_img_width;

			/* �o�b�t�@����� */
			free( buf );

			return REMOS_RET_SUCCEED;

		case REMOS_FILE_TYPE_BIL_R10:										/* remo10��BIL������ �Ȃ���big�G���f�B�A��  */
			/* ���擾�̂��߂Ƀo�b�t�@�쐬 */
			buf = malloc( 512 );

			/* �o�b�t�@�ɓǂݍ��� */
			fseek( cont->fp, 0, SEEK_SET );
			fread( buf, 1, 512, cont->fp );

			/* �R���e�i�ɏ��ݒ� */
			cont->band_count = remos_BE2usint( buf + 0x56, 2 );				/* �o���h���擾 */
			cont->img_width  = remos_BE2usint( buf + 0xC,  2 );				/* �摜���擾 */
			cont->img_height = remos_BE2usint( buf + 0xE,  2 );				/* �摜�����擾 */

			/* �o���h���� */
			cont->bands = malloc( sizeof(struct REMOS_BAND) * cont->band_count );

			/* �o���h��ݒ� */
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].fp   = cont->fp;
				cont->bands[i].bits = 8;									/* ���̂Ƃ���8�����l���ĂȂ� */
				cont->bands[i].byte_per_sample = cont->bands[i].bits / 8;
				cont->bands[i].sample_per_pix  = 1;							/* �Ƃ肠����1 */

				cont->bands[i].color = REMOS_BAND_COLOR_BW;
				cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands[i].endian        = REMOS_ENDIAN_LITTLE;			/* �Ƃ肠����little */

				cont->bands[i].line_width  = cont->img_width;
				cont->bands[i].line_count  = cont->img_height;
				cont->bands[i].line_header = 0;
				cont->bands[i].line_footer = 0;

				cont->bands[i].line_img_width = cont->img_width;

				cont->bands[i].header     = 512;
				cont->bands[i].band_num   = i;
				cont->bands[i].band_count = cont->band_count;
				
				cont->bands[i].range_top    = pow( 2, cont->bands[i].bits ) - 1;
				cont->bands[i].range_bottom = 0;
				cont->bands[i].range_max    = pow( 2, cont->bands[i].bits ) - 1;
				cont->bands[i].range_min    = 0;
			}
			
			/* �o�b�t�@����� */
			free( buf );
			
			return REMOS_RET_SUCCEED;
		
		case REMOS_FILE_TYPE_TIFF:											/* �Ȉ�TIFF������ */
			/* ������ */
			err = 0;
            set_sample = 0;
            sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;

			/* TIFREC�ɂЂ炢�Ă��炤 ( �G���[���� ) */
			tifrec_open( &tr, cont->file_name );
			
			/* ifd��������A�K�v�ȏ���T�� ( DEBUG : ����1�ڂ�IFD�����ǂނ悤�ɂ��Ă��� : next = NULL ) */
			for ( next = tr.ifd; next != NULL; next = NULL ) {
				for ( i = 0; i < next->count; i++ ) {
					if ( next->entry[i].tag == TIFREC_TAG_IMG_WIDTH ) {
						/* �摜�� */
                        width = next->entry[i].value;
					} else if ( next->entry[i].tag == TIFREC_TAG_IMG_LENGTH ) {
						/* �摜�� */
						height = next->entry[i].value;
					} else if ( next->entry[i].tag == TIFREC_TAG_STRIP_OFFSET ) {
						/* �t�@�C���w�b�_ */
						header = next->entry[i].value_usint[0];
					} else if ( next->entry[i].tag == TIFREC_TAG_COMPRESS ) {
						/* ���k */
						if ( next->entry[i].value != 1 ) {
							/* �񈳏k�����Ή����Ȃ� */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_PHOTOMETRIC ) {
						/* �摜�`�� */
						if ( next->entry[i].value == 1 ) {
							/* ���� ( black = 0 ) */
							color = REMOS_BAND_COLOR_BW;
						} else if ( next->entry[i].value == 2 ) {
							/* RGB */
							color = REMOS_BAND_COLOR_RGB;
						} else {
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_PLANAR_CONFIG ) {
						/* �f�[�^���� */
						if ( next->entry[i].value != 1 ) {
							/* �אڂ����Ή����Ȃ� */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_BIT_PER_SAMPLE ) {
						/* �r�b�g�� */
                        bits = next->entry[i].value_usshort[0];

						if ( next->entry[i].value_usshort[0] != 8 ) {
							/* 8bit�����Ή����Ȃ� */
                            /* 8bit�̂ݑΉ����������� : DEBUG */
							// err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_SAMPLE_PER_PIX ) {
						/* �T���v���� */
						sample = next->entry[i].value;

                        /* �T���v�������w�肵�Ă��Ȃ��摜�̂��߂ɐݒ肳�ꂽ�����L�� */
                        set_sample = 1;
					} else if ( next->entry[i].tag == TIFREC_TAG_ROW_PER_STRIP ) {
						/* �X�g���b�v������̍s�� */
						if ( next->entry[i].value != 1 ) {
							/* 1�X�g���b�v1�s�����Ή����Ȃ� */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_SAMPLE_FORMAT ) {
						/* �T���v���t�H�[�}�b�g�̎w�肾���� */
						switch ( (int)next->entry[i].value ) {
							case 2:
								sample_format = REMOS_BAND_SAMPLE_FORMAT_INT;
								break;
							case 3:
								sample_format = REMOS_BAND_SAMPLE_FORMAT_IEEEFP;
								break;
							default:
								break;
						}
					}
				}
			}
			
			/* ���� */
			tifrec_close( &tr );
			
			if ( err != 0 ) {
				/* ��������Ή����ĂȂ����� */
				return REMOS_RET_FAILED;
			}
			
			/* �R���e�i��ݒ� */
			if ( color == REMOS_BAND_COLOR_RGB ) {
				/* �J���[������ */
				if ( sample != 3 ) {
					/* 3�T���v���ȊO�͎��s */
					return REMOS_RET_FAILED;
				}
				
				cont->band_count = 3;
			} else {
				/* ���������� */
				if ( set_sample == 1 && sample != 1 ) {
					/* 1�T���v���ȊO�͎��s */
					return REMOS_RET_FAILED;
				}
				
				cont->band_count = 1;
			}

			/* �J���[�^�C�v�ɉ����ăo���h�쐬 */
			cont->bands = malloc( sizeof(struct REMOS_BAND) * cont->band_count );
			
			/* �o���h�ɐݒ�K�p */
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].bits = bits;
				cont->bands[i].byte_per_sample = cont->bands[i].bits / 8;
				
				/* �J���[�̏ꍇ�ł��W�J���Ă���̂Ńs�N�Z��������̃T���v������1�ŌŒ� */
				cont->bands[i].sample_per_pix  = 1;

				/* ���̃o���h����肤��l�͈̔͂��m�� */
				if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_UINT ) {
					/* �����Ȃ����� */
					cont->bands[i].range_max    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_top    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_min    = 0;
					cont->bands[i].range_bottom = 0;
				} else if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_INT ) {
					/* �����t������ */
					cont->bands[i].range_max    = pow( 2, cont->bands[i].bits - 1 ) - 1;
					cont->bands[i].range_top    = pow( 2, cont->bands[i].bits - 1 ) - 1;
					cont->bands[i].range_min    = -pow( 2, cont->bands[i].bits - 1 );
					cont->bands[i].range_bottom = -pow( 2, cont->bands[i].bits - 1 );
				} else if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_IEEEFP ) {
					/* IEEE���������_ ( �قƂ�ǖ��Ӗ� ) */
					cont->bands[i].range_max    = FLT_MAX;
					cont->bands[i].range_top    = FLT_MAX;
					cont->bands[i].range_min    = -FLT_MAX;
					cont->bands[i].range_bottom = -FLT_MAX;
				} else {
					/* �s�� ( �����Ȃ��Ɠ��� ) */
					cont->bands[i].range_max    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_top    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_min    = 0;
					cont->bands[i].range_bottom = 0;
				}

				cont->bands[i].band_count   = cont->band_count;
				cont->bands[i].band_num     = i;
				
				cont->bands[i].fp = cont->fp;
				
				cont->bands[i].color = color;
				cont->bands[i].sample_format = sample_format;
				
				/* endian�̐ݒ� */
				if ( tr.endian == ENDIAN_LITTLE ) {
					cont->bands[i].endian = REMOS_ENDIAN_LITTLE;
				} else {
					cont->bands[i].endian = REMOS_ENDIAN_BIG;
				}
				
				cont->bands[i].line_header = 0;
				cont->bands[i].line_footer = 0;
				
				cont->bands[i].header = header;
				
				cont->bands[i].line_img_width = width;
				cont->bands[i].line_width = width;
				
				cont->bands[i].line_count = height;
			}
			
			/* �R���e�i�ɉ摜�T�C�Y��ݒ� */
			cont->img_height = cont->bands[0].line_count;
			cont->img_width  = cont->bands[0].line_width - cont->bands[0].line_header - cont->bands[0].line_footer;
			
			return REMOS_RET_SUCCEED;
		
		default:															/* ���������ł��Ȃ����� */
			/* ���������ł��Ȃ����� */
			
			return REMOS_RET_FAILED;
	}
}

int remos_set_type_BIL( struct REMOS_FILE_CONTAINER *cont,
	int file_header, int header, int footer, int lines, int width, int bands, int bits )
{
	/* �t�@�C���^�C�v���蓮�ݒ� BIL */
	int i;
	
	/* �R���e�i�ݒ� */
	cont->type       = REMOS_FILE_TYPE_BIL;
	cont->img_height = lines;
	cont->img_width  = width - header - footer;
	cont->band_count = bands;
	
	/* ���łɊm�ۂ���Ă���΁A�J�����Ă���ݒ肵�Ȃ��� */
	if ( cont->bands != NULL ) {
		free( cont->bands );
	}
	
	/* �m�� */
	cont->bands = malloc( sizeof(struct REMOS_BAND) * bands );
	
	for ( i = 0; i < bands; i++ ) {
		cont->bands[i].bits = bits;											/* ���̂Ƃ���8�����l���ĂȂ� */
		cont->bands[i].byte_per_sample = bits / 8;
		cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;		/* unsignedinteger������ */
		cont->bands[i].endian        = REMOS_ENDIAN_LITTLE;					/* endian��little�ɉ��� */
		
		cont->bands[i].color = REMOS_BAND_COLOR_BW;
		
		cont->bands[i].line_width  = cont->img_width;
		cont->bands[i].line_count  = cont->img_height;
		cont->bands[i].line_header = header;
		cont->bands[i].line_footer = footer;
		
		cont->bands[i].line_img_width = cont->img_width;
		
		cont->bands[i].header     = file_header;
		cont->bands[i].band_num   = i;
		cont->bands[i].band_count = cont->band_count;
		
		cont->bands[i].range_max    = pow( 2, bits ) - 1;
		cont->bands[i].range_top    = pow( 2, bits ) - 1;
		cont->bands[i].range_bottom = 0;
		cont->bands[i].range_min    = 0;
	}
	
	return REMOS_RET_SUCCEED;
}

int remos_set_type_BSQ( struct REMOS_FILE_CONTAINER *cont,
	int file_header, int header, int footer, int lines, int width, int bits )
{
	/* �t�@�C���^�C�v���蓮�ݒ� BSQ */

	/* �R���e�i�ݒ� */
	cont->type       = REMOS_FILE_TYPE_BSQ;
	cont->img_height = lines;
	cont->img_width  = width - header - footer;
	cont->band_count = 1;
	
	/* ���łɊm�ۂ���Ă���΁A�J�����Ă���ݒ肵�Ȃ��� */
	if ( cont->bands != NULL ) {
		free( cont->bands );
	}
	
	/* �m�� */
	cont->bands = malloc( sizeof(struct REMOS_BAND) );

	/* �o���h��ݒ� */
	cont->bands->bits = bits;													/* ���̂Ƃ���8�����l���ĂȂ� */
	cont->bands->byte_per_sample = bits / 8;
	cont->bands->sample_format   = REMOS_BAND_SAMPLE_FORMAT_UINT;				/* unsignedinteger������ */
	cont->bands->endian          = REMOS_ENDIAN_LITTLE;							/* little endian ������ */
	
	cont->bands->color = REMOS_BAND_COLOR_BW;
	
	cont->bands->line_width  = cont->img_width;
	cont->bands->line_count  = cont->img_height;
	cont->bands->line_header = header;
	cont->bands->line_footer = footer;
	
	cont->bands->line_img_width = cont->img_width;
	
	cont->bands->header     = file_header;
	cont->bands->band_num   = 0;
	cont->bands->band_count = 1;
	
	cont->bands->range_top    = pow( 2, bits ) - 1;
	cont->bands->range_max    = pow( 2, bits ) - 1;
	cont->bands->range_bottom = 0;
	
	return REMOS_RET_SUCCEED;
}

int remos_I2int( char *data, int length )
{
	/* ������strto�Alength�͐��m����Ȃ��Ɩ\���H */
	char *buf;
	int ret;
	
	buf = malloc( length + 1 );						/* �o�b�t�@�m�� */
	
	memcpy( buf, data, length );					/* �o�b�t�@�փR�s�[ */
	buf[length] = '\0';								/* �I�[�R�[�h�ǉ� */
	
	ret = strtol( buf, NULL, 10 );

    free( buf );

	return ret;										/* �ϊ����ďI�� */
}

unsigned int remos_BE2usint( char *data, int length )
{
	/* �r�b�O�G���f�B�A���̃f�[�^��ǂݍ����USINT�� */
	unsigned char *buf;
	int i;
	unsigned int ret;
	
	buf = malloc( length );
	
	/* �t���ɓǂݎ�� */
	for ( i = 0; i < length; i++ ) {
		buf[i] = *( ( data + ( length - 1 ) ) - i );
	}
	
	/* �T�C�Y�ɂ���ĕԂ�����ύX ( ���ˑ� ) */
	switch ( length ) {
		case REMOS_SIZE_INT:
			ret = *(unsigned int *)buf;
			break;
		
		case REMOS_SIZE_CHAR:
			ret = *(unsigned char *)buf;
			break;
		
		case REMOS_SIZE_SHORT:
			ret = *(unsigned short *)buf;
			break;
		
		default:
			ret = 0;
			break;
	}

	free( buf );

	return ret;
}

void remos_make_pixels( struct REMOS_FILE_CONTAINER *cont, struct REMOS_PIXELS *pixs )
{
	/* PIXELS���̔z��������m�� */

	pixs->pixels = malloc( sizeof(unsigned int) * cont->band_count );
	pixs->count  = cont->band_count;
}

void remos_free_pixels( struct REMOS_PIXELS *pixs )
{
	/* �m�ۂ��ꂽPIXELS��������� */
	
	free( pixs->pixels );
	pixs->pixels = NULL;
}

int remos_make_hist( struct REMOS_BAND *band )
{
	/* �q�X�g�O�������� */
	int i, j;
	int lines;
	int pos;
	float val;
	unsigned int max;
	unsigned char *buf;

	max = 0;

	/* ���C���� */
	lines = band->line_count;
	
	/* �������N���A */
	for ( i = 0; i < 256; i++ ) {
		band->hist[i] = 0;
	}
	
	/* �o�b�t�@�m�� */
	buf = malloc( band->line_img_width * band->byte_per_sample * band->sample_per_pix );
	
	/* �v�� */
	for ( i = 0; i < lines; i++ ) {
		/* 1���C���擾 */
		remos_get_line_pixels( band, buf, i, 0, band->line_img_width );
		
		for ( j = 0; j < band->line_img_width; j++ ) {
			/* �l�ɕϊ� */
			val = remos_data_to_value_band( band, buf + band->bits / 8 * j );
			
			/* �q�X�g�O������ł̈ʒu���m�� */
			pos = ( val - band->range_min ) / ( band->range_max - band->range_min ) * 255;
			
			/* �q�X�g�O�����ǉ� */
			band->hist[pos]++;
		}
	}
	
	/* �ő�l�v�� */
	for ( i = 0; i < 256; i++ ) {
		if ( max < band->hist[i] ) {
			max = band->hist[i];
		}
	}

	/* �ő�l�ۑ� */
	band->hist_max = max;
	
	/* �ő�l�v�� */
	for ( i = 1, max = 0; i < 255; i++ ) {
		if ( max < band->hist[i] ) {
			max = band->hist[i];
		}
	}
	
	/* �ő�l�ۑ� */
	band->hist_max_reduce_topbottom = max;
	
	/* �o�b�t�@��� */
	free( buf );
	
	return REMOS_RET_SUCCEED;
}

void remos_calc_auto_range( struct REMOS_BAND *band, double per, int topbottom )
{
	/* �����Ń_�C�i�~�b�N�����W�ݒ� */
	int count;
	int skip;
	int i, j;
	int start, end;
	
	/* ������ */
	skip  = band->line_count * band->line_img_width * per;
	count = 0;
	
	/* topbottom��0�ȊO�ŏ㉺���w�蕪�̂��� */
	
	/* �v�� */
	for ( i = topbottom; i < 256; i++ ) {
		count += band->hist[i];
		
		/* �������H */
		if ( count > skip ) {
			band->range_bottom = i / 255.0 * ( band->range_max - band->range_min ) + band->range_min;
			
			break;
		}
	}
	
	count = 0;
	
	for ( i = topbottom; i < 256; i++ ) {
		count += band->hist[255 - i];
		
		/* �������H */
		if ( count > skip ) {
			band->range_top = ( 255 - i ) / 255.0 * ( band->range_max - band->range_min ) + band->range_min;;
			
			break;
		}
	}
}

float remos_get_pixel( struct REMOS_BAND *band, int pos )
{
	/* �w��o���h�̎w��ʒu����1�s�N�Z�����炤 */
	int line;
	int file_pos;
	// unsigned int ret;
    float ret;
	unsigned char *buf;

	/* �o�b�t�@���m�� */
	buf = malloc( band->byte_per_sample );

	/* �ʒu����� */
	line = pos / band->line_img_width;

    /* �ʒu������������ΒE�o */
    if ( pos < 0 || line >= band->line_count ) {
    	ret = 0;
        goto L_EXIT_GET_PIXEL;
    }

	/* �J���[�^�C�v�ɂ���Ĉʒu���Ⴄ */
	if ( band->color == REMOS_BAND_COLOR_BW ) {
    	// DEBUG �C��
		file_pos = ( ( band->band_count * line + band->band_num ) * band->line_width * band->byte_per_sample ) + band->line_header + ( pos % band->line_img_width ) * band->byte_per_sample + band->header;
	} else if ( band->color == REMOS_BAND_COLOR_RGB ) {
		file_pos = ( ( line + band->band_num ) * band->line_img_width * 3 * band->byte_per_sample ) + band->line_header + ( ( pos % band->line_img_width ) * 3 * band->byte_per_sample ) + band->header;
	} else if ( band->color == REMOS_BAND_COLOR_PACKED_USINT_2 ) {
		/* ���ALOSPAL1.1�p */
		file_pos = ( line * band->line_width ) + band->line_header + ( pos % band->line_img_width ) * band->byte_per_sample * band->sample_per_pix + band->band_num * band->byte_per_sample + band->header;
	}

	/* �ǂݍ��� */
	fseek( band->fp, file_pos, SEEK_SET );
	fread( buf, 1, band->byte_per_sample, band->fp );

	/* �f�[�^�𐔒l�� */
	if ( band->color == REMOS_BAND_COLOR_PACKED_USINT_2 ) {
		ret = remos_data_to_float( buf, band->byte_per_sample, REMOS_ENDIAN_BIG );
	} else {
    	// DEBUG : BIG����LITTLE�ɕύX
		// DEBUG : �g���֐���ύX
		ret = remos_data_to_value_band( band, buf );
		// ret = remos_data_to_value( buf, band->byte_per_sample, REMOS_ENDIAN_LITTLE );
	}

L_EXIT_GET_PIXEL:

	/* �J�� */
	free( buf );

	return ret;
}

float remos_data_to_float( unsigned char *data, int len, int endian )
{
	/* �G���f�B�A���ɏ]���ăf�[�^��float�� */
	int i;
	float ret;
	
	ret = 0;

	if ( endian == REMOS_ENDIAN_BIG ) {
		for ( i = 0; i < len; i++ ) {
			( (unsigned char *)&ret )[i] = data[len - 1 - i];
		}
	} else {
		for ( i = 0; i < len; i++ ) {
			( (unsigned char *)&ret )[i] = data[i];
		}
	}

	return ret;
}

float remos_data_to_value_format( unsigned char *data, int len, int endian, int format )
{
	/* �G���f�B�A���Ə����ɏ]���ăf�[�^�𐔒l�ɂ��� ( float �ŕԂ� ) */
	/* �G���[�`�F�b�N���Â���32bit�ȏ�͏����ł��Ȃ��̂Ŗ�肠�� */
	float ret;
	char *ret_ptr;
	float ret_f;
	char ret_c;
	unsigned char ret_uc;
	short ret_s;
	unsigned short ret_us;
	int ret_i;
	unsigned int ret_ui;
	int i;
	
	/* �������ݐ�|�C���^�[����� */
	switch ( format ) {
		case REMOS_BAND_SAMPLE_FORMAT_INT:
			/* signed integer */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_i;
					break;
				case 2:
					ret_ptr = &ret_s;
					break;
				case 1:
					ret_ptr = &ret_c;
					break;
			}
			
			break;

		case REMOS_BAND_SAMPLE_FORMAT_UINT:
			/* unsigned integer */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_ui;
					break;
				case 2:
					ret_ptr = &ret_us;
					break;
				case 1:
					ret_ptr = &ret_uc;
					break;
			}
			
			break;

		case REMOS_BAND_SAMPLE_FORMAT_IEEEFP:
			/* float */
			switch ( len ) {
				case 4:
					ret_ptr = &ret_f;
					break;
			}
			
			break;
		
		default:
			ret_ptr = &ret_i;
			break;
	}
	
	/* �G���f�B�A���ɂ��������ăf�[�^�[���R�s�[ */
	if ( endian == REMOS_ENDIAN_BIG ) {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[len - 1 - i];
		}
	} else {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[i];
		}
	}
	
	/* �l��float�ɕϊ����� */
	switch ( format ) {
		case REMOS_BAND_SAMPLE_FORMAT_INT:
			/* signed integer */
			switch ( len ) {
				case 4:
					ret = ret_i;
					break;
				case 2:
					ret = ret_s;
					break;
				case 1:
					ret = ret_c;
					break;
			}
			
			break;

		case REMOS_BAND_SAMPLE_FORMAT_UINT:
			/* unsigned integer */
			switch ( len ) {
				case 4:
					ret = ret_ui;
					break;
				case 2:
					ret = ret_us;
					break;
				case 1:
					ret = ret_uc;
					break;
			}
			
			break;

		case REMOS_BAND_SAMPLE_FORMAT_IEEEFP:
			/* float */
			switch ( len ) {
				case 4:
					ret = ret_f;
					break;
			}
			
			break;
		
		default:
			ret = ret_i;
			break;
	}
	
	/* �Ԃ� */
	return ret;
}

unsigned int remos_data_to_value( unsigned char *data, int len, int endian )
{
	/* �G���f�B�A���ɏ]���ăf�[�^�𐔒l�� */
	int i;
	unsigned int ret;
	
	ret = 0;
	
	if ( endian == REMOS_ENDIAN_BIG ) {
		for ( i = 0; i < len; i++ ) {
        	ret += data[i] * pow( 256, len - i - 1 );
		}
	} else {
		for ( i = 0; i < len; i++ ) {
			ret += data[i] * pow( 256, i );
		}
	}
	
	return ret;
}

float remos_data_to_value_band( struct REMOS_BAND *band, unsigned char *data )
{
	/* �o���h�̃��[���Ńf�[�^�[�����o����float�ɕϊ� */

    /* ALOS��PALSAR1.1�̓p�b�N�h������get_line_pixels�Ŗ������l�߂Ȃ����Ă�̂�BytePerSample�Ȃǂ̒l���㏑������ ( ���͕����Ȃǂ�����悤�ɂ��Ă���̂ŉ����̗]�n�����邩���H ) */
    if ( band->color == REMOS_BAND_COLOR_PACKED_USINT_2 ) {
        return remos_data_to_value_format( data, 1, REMOS_ENDIAN_LITTLE, REMOS_BAND_SAMPLE_FORMAT_UINT );
    } else {
    	return remos_data_to_value_format( data, band->byte_per_sample, band->endian, band->sample_format );
}   }

void remos_get_pixels( struct REMOS_FILE_CONTAINER *cont, int pos, struct REMOS_PIXELS *pixs )
{
	/* �t�@�C���R���e�i���̑S�o���h�̎w���̃f�[�^���ꊇ�ł��炤 */
	int i;
	
	/* �e�o���h���ꂼ��ǂݍ��� */
	for ( i = 0; i < pixs->count; i++ ) {
		pixs->pixels[i] = remos_get_pixel( &cont->bands[i], pos );
	}
}

void remos_set_range( struct REMOS_BAND *band, int bottom, int top )
{
	/* �_�C�i�~�b�N�����W(?)�ݒ� */

	band->range_top    = top;
	band->range_bottom = bottom;
}

struct REMOS_BAND *remos_get_band( struct REMOS_FILE_CONTAINER *cont, int num )
{
	/* �w�肵���o���h�ԍ��̃o���h�𓾂� */

	return &cont->bands[num];
}

int remos_get_line_pixels( struct REMOS_BAND *band, unsigned char *buf, int line, int from, int count )
{
	/* �w��o���h�̂���s( 0�X�^�[�g )��from( 0�X�^�[�g )����Acount�f�[�^��ǂ�  */
	int i;
	int file_pos;
	unsigned int read_data_usint;
	float read_data_float;
	char *read_buf;

    /* �͈͊O�ł���΁A�����I�ɔ͈͓��ɂ��� */
    if ( line >= band->line_count ) {
    	line = band->line_count - 1;
    }

    if ( band->line_img_width <= from ) {
    	from = band->line_img_width - 1;
    }

    if ( count + from >= band->line_img_width ) {
    	count = band->line_img_width - from;
    }

	/* �ʒu����� */


	/* �J���[�^�C�v�ɂ��ʒu����� */
	if ( band->color == REMOS_BAND_COLOR_BW ) {
    	// DEBUG �C��
		file_pos = ( ( band->band_count * line + band->band_num ) * band->line_width * band->byte_per_sample ) + band->line_header + from * band->byte_per_sample + band->header;
	} else if ( band->color == REMOS_BAND_COLOR_RGB ) {
		file_pos = ( ( line + band->band_num ) * band->line_img_width * 3 * band->byte_per_sample ) + band->line_header + from * 3 * band->byte_per_sample + band->header;
	} else if ( band->color == REMOS_BAND_COLOR_PACKED_USINT_2 ) {
		/* ���ALOSPAL1.1�p */
		file_pos = ( line * band->line_width ) + band->line_header + from * band->byte_per_sample * band->sample_per_pix + band->header;
	}

	/* �����ʒu�� */
	fseek( band->fp, file_pos, SEEK_SET );

	/* �J���[�^�C�v�ɂ��ǂݍ��� */
	if ( band->color == REMOS_BAND_COLOR_BW ) {
		/* DEBUG : �b��I��16bit�Ή� */
        /* DEBUG : 16bit�Ή����@��ύX */
		switch ( band->bits ) {
			case 16: {
				/* �o�b�t�@�m�� */
				// read_buf = malloc( band->byte_per_sample * count );

				/* �ǂݍ��� */
				// fread( read_buf, 1, band->byte_per_sample * count, band->fp );

				/* �l�ߍ��� */
				for ( i = 0; i < count; i++ ) {
					// buf[i] = remos_data_to_value( read_buf + i * band->byte_per_sample, band->byte_per_sample, REMOS_ENDIAN_BIG ) / 65535.0 * 255.0;
				}

				// free( read_buf );

                fread( buf, 1, count * 2, band->fp );

				break;
			}

			case 8: {
				fread( buf, 1, count, band->fp );

				break;
			}
			
			default: {
				break;
			}
		}
		
	} else if ( band->color == REMOS_BAND_COLOR_RGB ) {
		/* �o�b�t�@�m�� */
		read_buf = malloc( band->line_img_width * 3 * band->byte_per_sample );
		
		/* �ǂݍ��� */
		fread( read_buf, 1, count * 3 * band->byte_per_sample, band->fp );
		
		/* �l�ߍ��� */
		for ( i = 0; i < count; i++ ) {
			buf[i] = read_buf[i * 3 + band->band_num];
		}
		
		/* �J�� */
		free( read_buf );
	} else if ( band->color == REMOS_BAND_COLOR_PACKED_USINT_2 ) {
		/* ���ALOS-PAL1.1�p */

		/* �o�b�t�@�m�� */
		read_buf = malloc( band->line_img_width * band->sample_per_pix * band->byte_per_sample );
		
		/* �ǂݍ��� */
		fread( read_buf, 1, count * band->sample_per_pix * band->byte_per_sample, band->fp );
		
		/* �߂Ȃ��� */
		for ( i = 0; i < count; i++ ) {
			read_data_float = remos_data_to_float( read_buf + i * band->byte_per_sample * band->sample_per_pix + band->band_num * band->byte_per_sample, band->byte_per_sample, REMOS_ENDIAN_BIG );
			buf[i] = fabs( read_data_float ) / 1000000 * 255.0;
		}
		
		/* �J�� */
		free( read_buf );
	}

	return REMOS_RET_SUCCEED;
}

void remos_get_ranged_pixels( struct REMOS_BAND *band, unsigned char *buf, int count )
{
	/* �w��o���h�̃_�C�i�~�b�N�����W�ɉ����Ēl���ۂ߂� */
	int i;
	int ret;

	/* ���̂Ƃ���8bit�����Ή����Ȃ��̂�8bit�ȊO�ł͂Ȃɂ����Ȃ� */
	if ( band->bits != 8 ) {
		return;
	}

	/* unsigned int �ł����g���Ȃ� */
	if ( band->sample_format != REMOS_BAND_SAMPLE_FORMAT_UINT ) {
		return;
	}
	
	/* �w������I�[�o�[���Ă�΋����I�Ɏ��߂� */
	if ( count > band->line_img_width ) {
		count = band->line_img_width;
	}
	
	/* �w�肳��ĂȂ���Όv�Z���Ȃ� */
	if ( band->range_bottom == 0 && band->range_top == 255 ) {
		return;
	}

    /* ����0�Ȃ�Ȃɂ����Ȃ� */
    if ( ( band->range_top - band->range_bottom ) < 1 ) {
    	return;
    }

	/* �v�Z */
	for ( i = 0; i < count; i++ ) {
		ret = ( buf[i] - band->range_bottom ) * ( 255.999999 / ( band->range_top - band->range_bottom ) );

		if ( ret < 0 ) {
			buf[i] = 0;
		} else if ( ret > 255 ) {
			buf[i] = 255;
		} else {
			buf[i] = ret;
		}
	}
}

float remos_get_ranged_pixel( struct REMOS_BAND *band, float val )
{
	/* �w��o���h�̃_�C�i�~�b�N�����W�ɉ����Ēl���ۂ߂� */
	int i;
	int ret;

	/* �w�肳��ĂȂ���Όv�Z���Ȃ� */
	if ( band->range_bottom == band->range_min && band->range_top == band->range_max ) {
		return val;
	}

    /* ����0�Ȃ�Ȃɂ����Ȃ� */
    if ( ( band->range_top - band->range_bottom ) == 0 ) {
    	return val;
    }

	/* �v�Z */
	ret = ( val - band->range_bottom ) / ( band->range_top - band->range_bottom ) * ( band->range_max - band->range_min ) + band->range_min;

	if ( ret < band->range_min ) {
		ret = band->range_min;
	} else if ( ret > band->range_max ) {
		ret = band->range_max;
	}

	return ret;
}

void remos_latlon_to_xy( int *i, int *j, int w, int h, float lat, float lon, float ox, float oy, float ax, float ay, float bx, float by, float cx, float cy )
{
	/* �l���̈ܓx�o�x��^����ƁA�ܓx�o�x����xy���W���t�Z���Ă���� */
    float x, y;
    float a_x, a_y;
    float b_x, b_y;
    float g_x, g_y;

    float a, b, c;
    float i_buf;
    float j_buf;

	x   = lon - ox;
    a_x = ax - ox;
    b_x = bx - ox;
    g_x = cx - ax - bx + ox;

    y   = lat - oy;
    a_y = ay - oy;
    b_y = by - oy;
    g_y = cy - ay - by + oy;

    a = a_x * g_y - a_y * g_x;
    b = a_x * b_y - a_y * b_x - x * g_y + y * g_x;
    c = -x * b_y + y * b_x;

    // a��0�̏ꍇ�̕���
    if ( a ) {
        j_buf = ( -b + sqrt( b * b - 4 * a * c ) ) / ( 2 * a );

        if ( j_buf < 0 || j_buf * ( h - 1 ) >= h ) {
            j_buf = ( -b - sqrt( b * b - 4 * a * c ) ) / ( 2 * a );
        }
    } else {
    	j_buf = -c / b;
    }

    a = b_x * g_y - b_y * g_x;
    b = b_x * a_y - b_y * a_x - x * g_y + y * g_x;
    c = -x * a_y + y * a_x;

    // a��0�̏ꍇ�̕���
    if ( a ) {
        i_buf = ( -b + sqrt( b * b - 4 * a * c ) ) / ( 2 * a );

        if ( i_buf < 0 || i_buf * ( w - 1 ) >= w ) {
            i_buf = ( -b - sqrt( b * b - 4 * a * c ) ) / ( 2 * a );
        }
    } else {
    	i_buf = -c / b;
    }

    /* ���ʕԂ� */
    *i = i_buf * ( w - 1 );
    *j = j_buf * ( h - 1 );
}


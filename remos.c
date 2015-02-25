#include "remos.h"

/* 汎用衛星データリーダーライブラリ ver1.3 */
/* 簡易TIFFアクセス */
/* 8bit非圧縮オンリー */
/* GeoTiffのタグはわからない */
/* bands 0, 1, 2の順でRGB */

static int remos_I2int( char *data, int length );															/* I形式のテキストをintに alos等用 */
static unsigned int remos_BE2usint( char *data, int length );												/* ビッグエンディアンのデータを読む */
static unsigned int remos_data_to_value( unsigned char *data, int len, int endian );						/* エンディアンに従ってデータを読む */
static float remos_data_to_float( unsigned char *data, int len, int endian );										/* エンディアンに従ってデータを読む ( float ) */

int remos_open( struct REMOS_FILE_CONTAINER *cont, char *filename, int type )
{
	/* ファイルを開く 開けなかったらRET_FAILED, 読み込みに失敗したらRET_READ_FAILED,  */

	/* 初期化 */
	cont->type       = type;
	cont->band_count = 0;
	cont->bands      = NULL;
	cont->fp         = NULL;

    /* ファイル名コピー */
    memcpy( cont->file_name, filename, strlen( filename ) );
    cont->file_name[strlen( filename )] = '\0';

	/* ファイルを開く */
	if ( ( cont->fp = fopen( filename, "rb" ) ) == NULL ) {
		/* 失敗 */
		return REMOS_RET_FAILED;
	}
	
	/* 手動識別モードなら何もしない */
	if ( type == REMOS_FILE_TYPE_NOT_RECOG ) {
		return REMOS_RET_SUCCEED;
	}
	
	/* 自動設定モードか？ */
	if ( type == REMOS_FILE_TYPE_AUTO ) {
		/* タイプ識別 */
		cont->type = remos_check_file_type( cont );
		
		/* 識別できた？ */
		if ( cont->type == REMOS_FILE_TYPE_UNKNOWN ) {
			/* 識別失敗 */
			return REMOS_RET_READ_FAILED;
		}
	}
	
	/* 実際に読み込む */
	if ( remos_read_file( cont ) == REMOS_RET_FAILED ) {
		/* 失敗した */
		return REMOS_RET_READ_FAILED;
	}
	
	/* 成功 */
	return REMOS_RET_SUCCEED;
}

int remos_close( struct REMOS_FILE_CONTAINER *cont )
{
	/* ファイルを閉じる */
	
	fclose( cont->fp );
	cont->fp = NULL;
	
	/* 確保されたバンドを解放 */
	free( cont->bands );
	cont->bands = NULL;
	
	return REMOS_RET_SUCCEED;
}

int remos_check_file_type( struct REMOS_FILE_CONTAINER *cont )
{
	/* ファイルタイプを調べる */
	char buf[64];
	char str_buf[32];
	
	/* 先頭の63バイトを読んでみる */
	fread( buf, 1, 63, cont->fp );
	buf[63] = '\0';
	
	/* REMO10形式チェック */
	memcpy( str_buf, buf, 8 );
	str_buf[8] = '\0';
	
	if ( !strcmp( str_buf, "SUGIMURA" ) ) {
		/* 謎ヘッダー付きremo10形式だった */
		return REMOS_FILE_TYPE_BIL_R10;
	}
	
	/* ALOSのBSQチェック ( 非常に甘い ) */
	memcpy( str_buf, buf + 48, 2 );
	str_buf[2] = '\0';
	
	if ( !strcmp( str_buf, "AL" ) ) {
		/* ALOS用BSQだった */
		
		/* PALか ( 雑 ) */
		memcpy( str_buf, buf + 52, 4 );
		str_buf[4] = '\0';
		
		if ( !strcmp( str_buf, "PSRC" ) || !strncmp( str_buf, "SAR", 3 ) ) {
			/* PALSAR1.5だった */
			return REMOS_FILE_TYPE_BSQ_ALOS_PAL;
		} else if ( !strcmp( str_buf, "PSRB" ) ) {
			/* PALSAR1.1だった */
			return REMOS_FILE_TYPE_BSQ_ALOS_PAL_11;
		} else {
			/* AV2かPRISMだった */
			return REMOS_FILE_TYPE_BSQ_ALOS;
		}
	}
	
	/* TIFF形式 ( 非常に甘い ) */
	memcpy( str_buf, buf, 2 );
	str_buf[2] = '\0';
	
	if ( strcmp( str_buf, "II" ) == 0 || strcmp( str_buf, "MM" ) == 0 ) {
		return REMOS_FILE_TYPE_TIFF;
	}
	
	/* 分からなかった */
	return REMOS_FILE_TYPE_UNKNOWN;
}

int remos_read_file( struct REMOS_FILE_CONTAINER *cont )
{
	/* 指定されたcont->typeに従って、ファイル読み込み */
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
		case REMOS_FILE_TYPE_BSQ_ALOS_PAL_11:								/* ALOSのBSQだけどPALの1.1だった */
			/* コンテナを設定 */
			cont->band_count = 2;
			cont->bands      = malloc( sizeof(struct REMOS_BAND) * 2 );

            /* ファイルヘッダの大きさを取得する */
            buf = malloc( 4 );

			/* ヘッダ読み込み */
			fseek( cont->fp, 8, SEEK_SET );
			fread( buf, 1, 4, cont->fp );

            /* ヘッダ決定 */
            header = remos_data_to_value( buf, 4, REMOS_ENDIAN_BIG );

            /* 開放 */
            free( buf );

			/* 情報取得のためにバッファ作成 */
			buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS_PAL) );

			/* ヘッダ読み込み */
			fseek( cont->fp, 180, SEEK_SET );
			fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS_PAL), cont->fp );
			
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].band_count  = 2;

				cont->bands[i].color         = REMOS_BAND_COLOR_PACKED_IEEEFP;
				cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_IEEEFP;
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
				
				cont->bands[i].header       = header;				/* 本当は+180の気がしたが、+180は無い方が良い, cont->bands->line_widthをheaderに置き換えてみた */

				cont->bands[i].range_top    = pow( 2, cont->bands->bits ) - 1;
				cont->bands[i].range_bottom = 0;
				cont->bands[i].range_max    = pow( 2, cont->bands->bits ) - 1;
				cont->bands[i].range_min    = 0;

                cont->bands[i].band_mode    = REMOS_BAND_MODE_PACK;
			}

			/* コンテナに画像サイズを設定 */
			cont->img_height = cont->bands[0].line_count;
			cont->img_width  = cont->bands[0].line_img_width;

			/* バッファを解放 */
			free( buf );

			return REMOS_RET_SUCCEED;
		
		case REMOS_FILE_TYPE_BSQ_ALOS:										/* ALOSのBSQだった */
		case REMOS_FILE_TYPE_BSQ_ALOS_PAL:
			/* コンテナを設定 */
			cont->band_count = 1;
			cont->bands      = malloc( sizeof(struct REMOS_BAND) );

            /* ファイルヘッダの大きさを取得する */
            buf = malloc( 4 );

			/* ヘッダ読み込み */
			fseek( cont->fp, 8, SEEK_SET );
			fread( buf, 1, 4, cont->fp );

            /* ヘッダ決定 */
            header = remos_data_to_value( buf, 4, REMOS_ENDIAN_BIG );

            /* 開放 */
            free( buf );

			/* PALSARか否かで少し違う */
			if ( cont->type == REMOS_FILE_TYPE_BSQ_ALOS_PAL ) {
				/* 情報取得のためにバッファ作成 */
				buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS_PAL) );

				/* ヘッダ読み込み */
				fseek( cont->fp, 180, SEEK_SET );
				fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS_PAL), cont->fp );
				
				/* バンドに設定適用 PALSARではline_header部等の意味が異なるため正確でない */
				cont->bands->band_count = 1;

				cont->bands->color         = REMOS_BAND_COLOR_BW;
				cont->bands->sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands->endian        = REMOS_ENDIAN_BIG;					/* bigendianで大丈夫だろうか */
				
				cont->bands->fp          = cont->fp;
				cont->bands->band_num    = 0;
				cont->bands->bits        = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->bit_per_pix_I4,   4 );
				cont->bands->line_width  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->record_length_I6, 6 );
				cont->bands->line_count  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->record_count_I6,  6 );
				cont->bands->line_header = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->header_bytes_I4,  4 );
				cont->bands->line_footer = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS_PAL *)buf )->footer_bytes_I4,  4 );
			} else {
				/* 情報取得のためにバッファ作成 */
				buf = malloc( sizeof(struct REMOS_HEADER_LINE_ALOS) );

				/* ヘッダ読み込み */
				fseek( cont->fp, 180, SEEK_SET );
				fread( buf, 1, sizeof(struct REMOS_HEADER_LINE_ALOS), cont->fp );
				
				/* バンドに設定適用 PALSARではline_header部等の意味が異なるため正確でない */
				cont->bands->band_count = 1;

				cont->bands->color         = REMOS_BAND_COLOR_BW;
				cont->bands->sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands->endian        = REMOS_ENDIAN_BIG;					/* bigendianで大丈夫だろうか */
				
				cont->bands->fp          = cont->fp;
				cont->bands->band_num    = 0;
				cont->bands->bits        = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->bit_per_pix_I4,   4 );
				cont->bands->line_width  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->record_length_I6, 6 );
				cont->bands->line_count  = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->record_count_I6,  6 );
				cont->bands->line_header = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->header_bytes_I4,  4 );
				cont->bands->line_footer = remos_I2int( ( (struct REMOS_HEADER_LINE_ALOS *)buf )->footer_bytes_I4,  4 );
			}

            cont->bands->byte_per_sample = cont->bands->bits / 8;
            cont->bands->sample_per_pix  = 1;				/* とりあえず1 */

			cont->bands->line_img_width = ( cont->bands->line_width - cont->bands->line_header - cont->bands->line_footer ) / cont->bands->byte_per_sample;
			
			cont->bands->header       = header;				/* 本当は+180の気がしたが、+180は無い方が良い, cont->bands->line_widthをheaderに置き換えてみた */

			cont->bands->range_top    = pow( 2, cont->bands->bits ) - 1;
			cont->bands->range_bottom = 0;
			cont->bands->range_max    = pow( 2, cont->bands->bits ) - 1;
			cont->bands->range_min    = 0;

            cont->bands->band_mode    = REMOS_BAND_MODE_BSQ;

			/* コンテナに画像サイズを設定 */
			cont->img_height = cont->bands->line_count;
			cont->img_width  = cont->bands->line_img_width;

			/* バッファを解放 */
			free( buf );

			return REMOS_RET_SUCCEED;

		case REMOS_FILE_TYPE_BIL_R10:										/* remo10のBILだった なぜかbigエンディアン  */
			/* 情報取得のためにバッファ作成 */
			buf = malloc( 512 );

			/* バッファに読み込み */
			fseek( cont->fp, 0, SEEK_SET );
			fread( buf, 1, 512, cont->fp );

			/* コンテナに情報設定 */
			cont->band_count = remos_BE2usint( buf + 0x56, 2 );				/* バンド数取得 */
			cont->img_width  = remos_BE2usint( buf + 0xC,  2 );				/* 画像幅取得 */
			cont->img_height = remos_BE2usint( buf + 0xE,  2 );				/* 画像高さ取得 */

			/* バンド生成 */
			cont->bands = malloc( sizeof(struct REMOS_BAND) * cont->band_count );

			/* バンドを設定 */
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].fp   = cont->fp;
				cont->bands[i].bits = 8;									/* 今のところ8しか考えてない */
				cont->bands[i].byte_per_sample = cont->bands[i].bits / 8;
				cont->bands[i].sample_per_pix  = 1;							/* とりあえず1 */

				cont->bands[i].color = REMOS_BAND_COLOR_BW;
				cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;
				cont->bands[i].endian        = REMOS_ENDIAN_LITTLE;			/* とりあえずlittle */

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

                cont->bands[i].band_mode    = REMOS_BAND_MODE_BIL;
			}
			
			/* バッファを解放 */
			free( buf );
			
			return REMOS_RET_SUCCEED;
		
		case REMOS_FILE_TYPE_TIFF:											/* 簡易TIFFだった */
			/* 初期化 */
			err = 0;
            set_sample = 0;
            sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;

			/* TIFRECにひらいてもらう ( エラー無視 ) */
			tifrec_open( &tr, cont->file_name );
			
			/* ifdをたぐり、必要な情報を探す ( DEBUG : 今は1つ目のIFDだけ読むようにしている : next = NULL ) */
			for ( next = tr.ifd; next != NULL; next = NULL ) {
				for ( i = 0; i < next->count; i++ ) {
					if ( next->entry[i].tag == TIFREC_TAG_IMG_WIDTH ) {
						/* 画像幅 */
                        width = next->entry[i].value;
					} else if ( next->entry[i].tag == TIFREC_TAG_IMG_LENGTH ) {
						/* 画像幅 */
						height = next->entry[i].value;
					} else if ( next->entry[i].tag == TIFREC_TAG_STRIP_OFFSET ) {
						/* ファイルヘッダ */
						header = next->entry[i].value_usint[0];
					} else if ( next->entry[i].tag == TIFREC_TAG_COMPRESS ) {
						/* 圧縮 */
						if ( next->entry[i].value != 1 ) {
							/* 非圧縮しか対応しない */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_PHOTOMETRIC ) {
						/* 画像形式 */
						if ( next->entry[i].value == 1 ) {
							/* 白黒 ( black = 0 ) */
							color = REMOS_BAND_COLOR_BW;
						} else if ( next->entry[i].value == 2 ) {
							/* RGB */
							color = REMOS_BAND_COLOR_RGB;
						} else {
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_PLANAR_CONFIG ) {
						/* データ並び */
						if ( next->entry[i].value != 1 ) {
							/* 隣接しか対応しない */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_BIT_PER_SAMPLE ) {
						/* ビット数 */
                        bits = next->entry[i].value_usshort[0];

						if ( next->entry[i].value_usshort[0] != 8 ) {
							/* 8bitしか対応しない */
                            /* 8bitのみ対応を強制解除 : DEBUG */
							// err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_SAMPLE_PER_PIX ) {
						/* サンプル数 */
						sample = next->entry[i].value;

                        /* サンプル数が指定していない画像のために設定されたかを記憶 */
                        set_sample = 1;
					} else if ( next->entry[i].tag == TIFREC_TAG_ROW_PER_STRIP ) {
						/* ストリップあたりの行数 */
						if ( next->entry[i].value != 1 ) {
							/* 1ストリップ1行しか対応しない */
							err++;
						}
					} else if ( next->entry[i].tag == TIFREC_TAG_SAMPLE_FORMAT ) {
						/* サンプルフォーマットの指定だった */
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
			
			/* 閉じる */
			tifrec_close( &tr );
			
			if ( err != 0 ) {
				/* 何かしら対応してなかった */
				return REMOS_RET_FAILED;
			}
			
			/* コンテナを設定 */
			if ( color == REMOS_BAND_COLOR_RGB ) {
				/* カラーだった */
				if ( sample != 3 ) {
					/* 3サンプル以外は失敗 */
					return REMOS_RET_FAILED;
				}
				
				cont->band_count = 3;
			} else {
				/* 白黒だった */
				if ( set_sample == 1 && sample != 1 ) {
					/* 1サンプル以外は失敗 */
					return REMOS_RET_FAILED;
				}
				
				cont->band_count = 1;
			}

			/* カラータイプに応じてバンド作成 */
			cont->bands = malloc( sizeof(struct REMOS_BAND) * cont->band_count );
			
			/* バンドに設定適用 */
			for ( i = 0; i < cont->band_count; i++ ) {
				cont->bands[i].bits = bits;
				cont->bands[i].byte_per_sample = cont->bands[i].bits / 8;

				/* サンプル数指定 */
				cont->bands[i].sample_per_pix = sample;

				/* このバンドが取りうる値の範囲を確定 */
				if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_UINT ) {
					/* 符号なし整数 */
					cont->bands[i].range_max    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_top    = pow( 2, cont->bands[i].bits ) - 1;
					cont->bands[i].range_min    = 0;
					cont->bands[i].range_bottom = 0;
				} else if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_INT ) {
					/* 符号付き整数 */
					cont->bands[i].range_max    = pow( 2, cont->bands[i].bits - 1 ) - 1;
					cont->bands[i].range_top    = pow( 2, cont->bands[i].bits - 1 ) - 1;
					cont->bands[i].range_min    = -pow( 2, cont->bands[i].bits - 1 );
					cont->bands[i].range_bottom = -pow( 2, cont->bands[i].bits - 1 );
				} else if ( sample_format == REMOS_BAND_SAMPLE_FORMAT_IEEEFP ) {
					/* IEEE浮動小数点 ( ほとんど無意味 ) */
					cont->bands[i].range_max    = FLT_MAX;
					cont->bands[i].range_top    = FLT_MAX;
					cont->bands[i].range_min    = -FLT_MAX;
					cont->bands[i].range_bottom = -FLT_MAX;
				} else {
					/* 不明 ( 符号なしと同じ ) */
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
				
				/* endianの設定 */
				if ( tr.endian == ENDIAN_LITTLE ) {
					cont->bands[i].endian = REMOS_ENDIAN_LITTLE;
				} else {
					cont->bands[i].endian = REMOS_ENDIAN_BIG;
				}
				
				cont->bands[i].line_header = 0;
				cont->bands[i].line_footer = 0;
				
				cont->bands[i].header = header;
				
				cont->bands[i].line_img_width = width;
				cont->bands[i].line_width = width * cont->bands[i].byte_per_sample * cont->bands[i].sample_per_pix;
				
				cont->bands[i].line_count = height;

                // バンドモードは一応PACKを仮定
                cont->bands[i].band_mode = REMOS_BAND_MODE_PACK;
			}
			
			/* コンテナに画像サイズを設定 */
			cont->img_height = cont->bands[0].line_count;
			cont->img_width  = cont->bands[0].line_img_width;

			return REMOS_RET_SUCCEED;
		
		default:															/* 何も処理できなかった */
			/* 何も処理できなかった */
			
			return REMOS_RET_FAILED;
	}
}

int remos_set_type_BIL( struct REMOS_FILE_CONTAINER *cont,
	int file_header, int header, int footer, int lines, int width, int bands, int bits )
{
	/* ファイルタイプを手動設定 BIL */
	int i;
	
	/* コンテナ設定 */
	cont->type       = REMOS_FILE_TYPE_BIL;
	cont->img_height = lines;
	cont->img_width  = width - header - footer;
	cont->band_count = bands;
	
	/* すでに確保されていれば、開放してから設定しなおす */
	if ( cont->bands != NULL ) {
		free( cont->bands );
	}
	
	/* 確保 */
	cont->bands = malloc( sizeof(struct REMOS_BAND) * bands );
	
	for ( i = 0; i < bands; i++ ) {
		cont->bands[i].bits = bits;											/* 今のところ8しか考えてない */
		cont->bands[i].byte_per_sample = bits / 8;
		cont->bands[i].sample_format = REMOS_BAND_SAMPLE_FORMAT_UINT;		/* unsignedintegerを仮定 */
		cont->bands[i].endian        = REMOS_ENDIAN_LITTLE;					/* endianをlittleに仮定 */
		
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

        cont->bands[i].band_mode    = REMOS_BAND_MODE_BIL;
	}
	
	return REMOS_RET_SUCCEED;
}

int remos_set_type_BSQ( struct REMOS_FILE_CONTAINER *cont,
	int file_header, int header, int footer, int lines, int width, int bits )
{
	/* ファイルタイプを手動設定 BSQ */

	/* コンテナ設定 */
	cont->type       = REMOS_FILE_TYPE_BSQ;
	cont->img_height = lines;
	cont->img_width  = width - header - footer;
	cont->band_count = 1;
	
	/* すでに確保されていれば、開放してから設定しなおす */
	if ( cont->bands != NULL ) {
		free( cont->bands );
	}
	
	/* 確保 */
	cont->bands = malloc( sizeof(struct REMOS_BAND) );

	/* バンドを設定 */
	cont->bands->bits = bits;													/* 今のところ8しか考えてない */
	cont->bands->byte_per_sample = bits / 8;
	cont->bands->sample_format   = REMOS_BAND_SAMPLE_FORMAT_UINT;				/* unsignedintegerを仮定 */
	cont->bands->endian          = REMOS_ENDIAN_LITTLE;							/* little endian を仮定 */
	
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

    cont->bands->band_mode    = REMOS_BAND_MODE_BSQ;
	
	return REMOS_RET_SUCCEED;
}

static int remos_I2int( char *data, int length )
{
	/* ただのstrto、lengthは正確じゃないと暴走？ */
	char *buf;
	int ret;
	
	buf = malloc( length + 1 );						/* バッファ確保 */
	
	memcpy( buf, data, length );					/* バッファへコピー */
	buf[length] = '\0';								/* 終端コード追加 */
	
	ret = strtol( buf, NULL, 10 );

    free( buf );

	return ret;										/* 変換して終了 */
}

static unsigned int remos_BE2usint( char *data, int length )
{
	/* ビッグエンディアンのデータを読み込んでUSINTに */
	unsigned char *buf;
	int i;
	unsigned int ret;
	
	buf = malloc( length );
	
	/* 逆順に読み取り */
	for ( i = 0; i < length; i++ ) {
		buf[i] = *( ( data + ( length - 1 ) ) - i );
	}
	
	/* サイズによって返し方を変更 ( 環境依存 ) */
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

int remos_make_hist( struct REMOS_BAND *band )
{
	/* ヒストグラム生成 */
	int i, j;
	int lines;
	int pos;
	int pixels;
	int skip;
	float val;
	unsigned int max;
	unsigned char *buf;

	max = 0;

	/* ライン数 */
	lines = band->line_count;
	
	/* メモリクリア */
	for ( i = 0; i < 256; i++ ) {
		band->hist[i] = 0;
	}
	
	/* バッファ確保 */
	buf = malloc( band->line_img_width * band->byte_per_sample * band->sample_per_pix );
	
	/* 4個ごと、4ラインごとに計測 */
	skip   = 4;
	pixels = 0;
	
	for ( i = 0; i < lines; i += skip ) {
		/* 1ライン取得 */
		remos_get_line_pixels( band, buf, i, 0, band->line_img_width );
		
		for ( j = 0; j < band->line_img_width; j += skip ) {
			/* 値に変換 */
			val = remos_data_to_value_band( band, buf, j );
			
			/* ヒストグラム上での位置を確定 */
			pos = ( val - band->range_min ) / ( band->range_max - band->range_min ) * 255;
			
			/* ヒストグラム追加 */
			band->hist[pos]++;
			
			/* 画素数 */
			pixels++;
		}
	}
	
	/* ヒストグラム計算に使った画素数保存 */
	band->hist_pixels = pixels;
	
	/* 最大値計測 */
	for ( i = 0; i < 256; i++ ) {
		if ( max < band->hist[i] ) {
			max = band->hist[i];
		}
	}

	/* 最大値保存 */
	band->hist_max = max;
	
	/* 最大値計測 */
	for ( i = 1, max = 0; i < 255; i++ ) {
		if ( max < band->hist[i] ) {
			max = band->hist[i];
		}
	}
	
	/* 最大値保存 */
	band->hist_max_reduce_topbottom = max;
	
	/* バッファ解放 */
	free( buf );
	
	return REMOS_RET_SUCCEED;
}

void remos_calc_auto_range( struct REMOS_BAND *band, double per, int topbottom )
{
	/* 自動でダイナミックレンジ設定 */
	int count;
	int skip;
	int i, j;
	int start, end;
	
	/* 個数決定 */
	skip  = band->hist_pixels * per;
	count = 0;
	
	/* topbottomが0以外で上下を指定分のぞく */

	/* 計測 */
	for ( i = topbottom; i < 256; i++ ) {
		count += band->hist[i];

		/* 超えた？ */
		if ( count > skip ) {
			band->range_bottom = i / 255.0 * ( band->range_max - band->range_min ) + band->range_min;

			break;
		}
	}

	count = 0;

	for ( i = topbottom; i < 256; i++ ) {
		count += band->hist[255 - i];

		/* 超えた？ */
		if ( count > skip ) {
			band->range_top = ( 255 - i ) / 255.0 * ( band->range_max - band->range_min ) + band->range_min;;

			break;
		}
	}
}

float remos_get_pixel_value( struct REMOS_BAND *band, int pos )
{
	/* 指定バンドの指定位置から1ピクセルもらう */
	int line;
	int file_pos;
    float ret;
	unsigned char *buf;

	/* バッファを確保 */
	buf = malloc( band->byte_per_sample );

	/* 位置を特定 */
	line = pos / band->line_img_width;

    // ライン読み込み
    remos_get_line_pixels( band, buf, line, pos % band->line_img_width, 1 );

    ret = remos_data_to_value_band( band, buf, 0 );

	/* 開放 */
	free( buf );

	return ret;
}

static float remos_data_to_float( unsigned char *data, int len, int endian )
{
	/* エンディアンに従ってデータをfloatに */
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
	/* エンディアンと書式に従ってデータを数値にする ( float で返す ) */
	/* エラーチェックも甘いし32bit以上は処理できないので問題あり */
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
	
	/* 書き込み先ポインターを特定 */
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
	
	/* エンディアンにしたがってデーターをコピー */
	if ( endian == REMOS_ENDIAN_BIG ) {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[len - 1 - i];
		}
	} else {
		for ( i = 0; i < len; i++ ) {
			ret_ptr[i] = data[i];
		}
	}

	/* 値をfloatに変換する */
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

	/* 返す */
	return ret;
}

static unsigned int remos_data_to_value( unsigned char *data, int len, int endian )
{
	/* エンディアンに従ってデータを数値に */
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

float remos_data_to_value_band( struct REMOS_BAND *band, unsigned char *data, int index )
{
    /* 位置計算 バンドの値が連続して並んでることを想定している */
    unsigned char *start = data + index * band->byte_per_sample;

	/* バンドのルールでデーターを取り出してfloatに変換 */
    return remos_data_to_value_format( start, band->byte_per_sample, band->endian, band->sample_format );
}

void remos_set_range( struct REMOS_BAND *band, int bottom, int top )
{
	/* ダイナミックレンジ(?)設定 */

	band->range_top    = top;
	band->range_bottom = bottom;
}

struct REMOS_BAND *remos_get_band( struct REMOS_FILE_CONTAINER *cont, int num )
{
	/* 指定したバンド番号のバンドを得る */

	return &cont->bands[num];
}

int remos_get_line_pixels( struct REMOS_BAND *band, unsigned char *buf, int line, int from, int count )
{
	/* 指定バンドのある行( 0スタート )のfrom( 0スタート )から、count個データを読む  */
	int i;
	int file_pos;
	unsigned int read_data_usint;
	float read_data_float;
	char *read_buf;

    /* 範囲外であれば、強制的に範囲内にする */
    if ( line >= band->line_count ) {
    	line = band->line_count - 1;
    }

    if ( band->line_img_width <= from ) {
    	from = band->line_img_width - 1;
    }

    if ( count + from >= band->line_img_width ) {
    	count = band->line_img_width - from;
    }

	/* 位置を特定 */

    // DEBUG BILかどうかで動作を切り分けなければならないはず
    // 最近BILを扱っていないので正しく動作しなくなっている可能性がある
    // BILかつPACKのがうまく扱えないという問題があるかも

    // バンドモードで処理を分岐
    if ( band->band_mode == REMOS_BAND_MODE_BIL ) {
        // BIL
        file_pos = band->header + ( ( band->band_count * line + band->band_num ) * band->line_width ) + band->line_header + from * band->byte_per_sample * band->sample_per_pix;
    } else {
        // BSQ, PACK
        file_pos = band->header + ( line * band->line_width ) + band->line_header + from * band->sample_per_pix * band->byte_per_sample;
    }

	/* 初期位置へ */
	fseek( band->fp, file_pos, SEEK_SET );

    // バンドモードによって詰め込み
    if ( band->band_mode == REMOS_BAND_MODE_BIL || band->band_mode == REMOS_BAND_MODE_BSQ || band->band_count == 1 ) {
        // パックされていないBIL、BSQ、もしくはバンド数が1のもの
        fread( buf, 1, count * band->byte_per_sample * band->sample_per_pix, band->fp );
    } else {
        // バンド数が1より多く、PACK

		/* バッファ確保 */
		read_buf = malloc( band->line_img_width * band->sample_per_pix * band->byte_per_sample );

		/* 読み込み */
		fread( read_buf, 1, count * band->sample_per_pix * band->byte_per_sample, band->fp );

		/* 詰め込み */
		for ( i = 0; i < count; i++ ) {
            memcpy( buf + i * band->byte_per_sample, read_buf + i * band->byte_per_sample * band->sample_per_pix + band->band_num * band->byte_per_sample, band->byte_per_sample );
		}

		/* 開放 */
		free( read_buf );
    }

	return REMOS_RET_SUCCEED;
}

float remos_get_ranged_pixel( struct REMOS_BAND *band, float val )
{
	/* 指定バンドのダイナミックレンジに応じて値を丸める */
	int i;
	int ret;

	/* 指定されてなければ計算しない */
	if ( band->range_bottom == band->range_min && band->range_top == band->range_max ) {
		return val;
	}

    /* 幅が0ならなにもしない */
    if ( ( band->range_top - band->range_bottom ) == 0 ) {
    	return val;
    }

	/* 計算 */
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
	/* 四隅の緯度経度を与えると、緯度経度からxy座標を逆算してくれる */
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

    // aが0の場合の分岐
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

    // aが0の場合の分岐
    if ( a ) {
        i_buf = ( -b + sqrt( b * b - 4 * a * c ) ) / ( 2 * a );

        if ( i_buf < 0 || i_buf * ( w - 1 ) >= w ) {
            i_buf = ( -b - sqrt( b * b - 4 * a * c ) ) / ( 2 * a );
        }
    } else {
    	i_buf = -c / b;
    }

    /* 結果返す */
    *i = i_buf * ( w - 1 );
    *j = j_buf * ( h - 1 );
}


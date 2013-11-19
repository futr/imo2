/* TIFF簡易解析プログラム 1.0 modified for imo2 */
/* オリジナルのtifrecではなく少しいじっている */
/* 2013-04-04 */
/* (c) 2013 Masato Takahashi */
/* ilce.ma@gmail.com */
/* sizeof( int )	 : 4 */
/* sizeof( short )	 : 2 */
/* sizeof( char )	 : 1 */
/* sizeof( float )	 : 4 */
/* sizeof( double )	 : 8 */
/* が前提 */

#include "tifrec.h"

int tifrec_open( struct TIFREC_FILE_CONTAINER *tr, char *filename )
{
	/* TIFFファイルを開いて識別まで試みる */
	char buf[64];
	unsigned int offset;
	struct TIFREC_IFD *next;
	unsigned int count;
	unsigned int i;
	unsigned int ifds;
	
	/* 初期化 */
	tr->ifd = NULL;
	strcpy( tr->filename, filename );
	
	/* ファイルを開く */
	if ( ( tr->fp = fopen( filename, "rb" ) ) == NULL ) {
		/* オープン失敗 */
		return TIFREC_RES_READ_FAILED;
	}
	
	/* ヘッダチェック */
	if ( !tifrec_check_tiff( tr ) ) {
		fclose( tr->fp );
		tr->fp = NULL;
		
		return TIFREC_RES_RECOG_FAILED;
	}
	
	/* IFDへのオフセット取得 */
	offset = tifrec_get_usdata( tr, 4, 4 );
	
	/* 先頭のIFD作成 */
	tr->ifd = (struct TIFREC_IFD *)malloc( sizeof(struct TIFREC_IFD) );
	next = tr->ifd;
	
	/* IFDを手繰りながら作っていく */
	for ( count = 1; ; count++ ) {
		/* エントリ数取得 */
		next->count = tifrec_get_usdata( tr, offset, 2 );
		offset += 2;
		
		/* エントリ領域確保 */
		next->entry = (struct TIFREC_IFD_ENTRY *)malloc( sizeof(struct TIFREC_IFD_ENTRY) * next->count );
		
		/* エントリ読み込み */
		for ( i = 0; i < next->count; i++ ) {
			next->entry[i].tag   = tifrec_get_usdata( tr, offset, 2 );
			next->entry[i].type  = tifrec_get_usdata( tr, offset + 2, 2 );
			next->entry[i].count = tifrec_get_usdata( tr, offset + 4, 4 );
			offset += 8;
			
			/* value読み込み */
			tifrec_get_value( tr, &next->entry[i], offset );
			
			/* offset調整 */
			offset += 4;
		}
		
		/* 次のIFDのOFFSET取得 */
		offset = tifrec_get_usdata( tr, offset, 4 );
		
		/* 読み終えたかチェック */
		if ( offset == 0 ) {
			/* 読み終えた */
			next->next = NULL;
			
			break;
		}
		
		/* 読み終えてないので次へ */
		next->next = (struct TIFREC_IFD *)malloc( sizeof(struct TIFREC_IFD) );
		next = next->next;
	}
	
	/* 個数保存 */
	tr->ifds = count;
	
	/* 完了 */
	return TIFREC_RES_SUCCEED;
}

int tifrec_close( struct TIFREC_FILE_CONTAINER *tr )
{
	/* 閉じる */
	int i;
	struct TIFREC_IFD *next, *next_next;
	
	if ( tr->fp != NULL ) {
		fclose( tr->fp );
		tr->fp = NULL;
	}
	
	/* IFDをタグって開放 */
	next = tr->ifd;
	
	/* nextがNULLになるまでたぐる */
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
	
	/* とりあえず成功 */
	return TIFREC_RES_SUCCEED;
}

int tifrec_check_tiff( struct TIFREC_FILE_CONTAINER *tr )
{
	/* TIFFか調べる ( 簡単に ) */
	char type[3];
	int ret;
	
	/* 先頭へ */
	fseek( tr->fp, 0, SEEK_SET );
	
	/* MMかIIを読み込み */
	fread( type, 1, 2, tr->fp );
	type[2] = '\0';
	
	/* MMかIIかチェック */
	if ( strcmp( "MM", type ) == 0 ) {
		/* モトローラ形式 */
		tr->endian = ENDIAN_BIG;
		
		ret = 1;
	} else if ( strcmp( "II", type ) == 0 ) {
		/* インテル形式 */
		tr->endian = ENDIAN_LITTLE;
		
		ret = 1;
	} else {
		/* 不明形式 */
		tr->endian = ENDIAN_UNKNOWN;
		
		ret = 0;
	}
	
	return ret;
}

double tifrec_get_usdata( struct TIFREC_FILE_CONTAINER *tr, int pos, int bytes )
{
	/* 符号無しとしてデータ読み込み */
	int i;
	unsigned char *buf;
	unsigned int ret;
	
	ret = 0;
	
	/* バッファ確保 */
	buf = (unsigned char *)malloc( bytes );
	
	/* 指定場所へ */
	fseek( tr->fp, pos, SEEK_SET );
	
	/* バッファへ読み込み */
	fread( buf, 1, bytes, tr->fp );

	/* 読み込み */
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
	/* entry内typeに応じてデータ読み込み */
	/* 簡易アクセス用のvalueと、ちゃんとアクセス用のvalue_typeがある */
	unsigned int pointer;
	double value_1, value_2;
	unsigned int value_1_usint, value_2_usint;
	unsigned int i;
	
	/* ポインタ初期化 */
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
	
	/* typeによって読み方を変える */
	switch ( entry->type ) {
		case 1: {
			/* unsigned char */
			
			/* 領域確保 */
			entry->value_uschar = malloc( sizeof( unsigned char ) * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_uschar[i], entry->pointer + ( i * 1 ), 1 );
			}
			
			entry->value = tifrec_get_usdata( tr, offset, 1 );
			break;
		}
		
		case 2: {
			/* string 適当に読み込み */
			
			/* 領域確保 */
			entry->value_ascii = malloc( entry->count );
			
			if ( entry->count > 4 ) {
				/* 4より大きい時はポインタ経由でアクセス */
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}
			
			/* 読み込み */
			fseek( tr->fp, entry->pointer, SEEK_SET );
			fread( entry->value_ascii, 1, entry->count, tr->fp );
			
			break;
		}

		case 3: {
			/* unsigned short */

			/* 領域確保 */
			entry->value_usshort = malloc( 2 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 2 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_usshort[i], entry->pointer + ( i * 2 ), 2 );
			}

			entry->value = (unsigned short)tifrec_get_usdata( tr, offset, 2 );
			
			break;
		}
		
		case 4: {
			/* unsigned int */

			/* 領域確保 */
			entry->value_usint = malloc( 4 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_usint[i], entry->pointer + ( i * 4 ), 4 );
			}
			
			entry->value = (unsigned int)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}
		
		case 5: {
			/* unsigned int / unsigned int */
			
			/* 領域確保 */
			entry->value_usint_num   = malloc( 4 * entry->count );
			entry->value_usint_denom = malloc( 4 * entry->count );
			
			/* ポインタ取得 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* 値取得 */
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
			
			/* 領域確保 */
			entry->value_char = malloc( 1 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_char[i], entry->pointer + i, 1 );
			}
			
			entry->value = (char)tifrec_get_usdata( tr, offset, 1 );
			
			break;
		}

		case 7: {
			/* undefine */
			
			/* 領域確保 */
			entry->value_undef = malloc( 1 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 4 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_undef[i], entry->pointer + ( i * 1 ), 1 );
			}
			
			entry->value = tifrec_get_usdata( tr, offset, 1 );
			
			break;
		}

		case 8: {
			/* signed short 16bit */
			
			/* 領域確保 */
			entry->value_short = malloc( 2 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 2 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_short[i], entry->pointer + ( i * 2 ), 2 );
			}

			entry->value = (short)tifrec_get_usdata( tr, offset, 2 );
			
			break;
		}
		
		case 9: {
			/* signed int 32bit */
			
			/* 領域確保 */
			entry->value_int = malloc( 4 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_int[i], entry->pointer + ( i * 4 ), 4 );
			}

			entry->value = (int)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}
		
		case 10: {
			/* signed int / signed int */
			
			/* 領域確保 */
			entry->value_int_num   = malloc( 4 * entry->count );
			entry->value_int_denom = malloc( 4 * entry->count );
			
			/* ポインタ取得 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* 値取得 */
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
			
			/* 領域確保 */
			entry->value_float = malloc( 4 * entry->count );
			
			/* ポインタ取得 */
			if ( entry->count > 1 ) {
				tifrec_get_data( tr, &entry->pointer, offset, 4 );
			} else {
				entry->pointer = offset;
			}

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_float[i], entry->pointer + ( i * 4 ), 4 );
			}

			entry->value = (float)tifrec_get_usdata( tr, offset, 4 );
			
			break;
		}

		case 12: {
			/* double */
			
			/* 領域確保 */
			entry->value_double = malloc( 8 * entry->count );
			
			/* ポインタ取得 */
			tifrec_get_data( tr, &entry->pointer, offset, 4 );

			/* 値取得 */
			for ( i = 0; i < entry->count; i++ ) {
				tifrec_get_data( tr, &entry->value_double[i], entry->pointer + ( i * 8 ), 8 );
			}

			tifrec_get_data( tr, &entry->value, entry->pointer, 8 );
			
			break;
		}
		
		default: {
			/* わからないのは適当 */
			
			entry->value = tifrec_get_usdata( tr, offset, 4 );
			break;
		}
	}
}

void tifrec_get_data( struct TIFREC_FILE_CONTAINER *tr, void *data, int pos, int count )
{
	/* 指定場所に指定場所から指定個読み込む */
	int i;
	unsigned char *buf;
	
	/* バッファ確保 */
	buf = (unsigned char *)malloc( count );
	
	/* 指定場所へ */
	fseek( tr->fp, pos, SEEK_SET );
	
	/* バッファへ読み込み */
	fread( buf, 1, count, tr->fp );

	/* 読み込み */
	for ( i = 0; i < count; i++ ) {
		if ( tr->endian == ENDIAN_BIG ) {
			( (unsigned char *)data )[i] = buf[count - 1 - i];
		} else {
			( (unsigned char *)data )[i] = buf[i];
		}
	}

    free( buf );
}


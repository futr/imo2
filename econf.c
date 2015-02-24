#include "econf.h"

/* 簡易設定リーダー ver1.1 */
/* abc = xyz のようなもののみ読める */
/* 空白文字は飛ばし、エスケープシーケンスは存在しない */
/* 行ベースで出来ていて入れ子とかは解釈しないため、keyは一意でなければならない */

int econf_open( struct ECONF_FILE_CONTAINER *ef, char *filename )
{
	/* ファイルを開く */
	
	/* 初期化 */
	ef->fp = NULL;
	ef->lines = NULL;
	ef->count = 0;
	
	if ( ( ef->fp = fopen( filename, "r" ) ) == NULL ) {
		/* 失敗 */
		return 0;
	}
	
	/* 成功 */
	strcpy( ef->filename, filename );
	
	return 1;
}

struct ECONF_TOKEN *econf_recog_line( char *line )
{
	/* 1行解釈レキシカルアナライザ */
	struct ECONF_TOKEN *top;
	struct ECONF_TOKEN *next;
	char *text;
	char *start;

	text = line;
	top = malloc( sizeof(struct ECONF_TOKEN) );

	for ( next = top; next != NULL; next = next->next ) {
		for ( ;; ) {
			if ( *text == '=' ) {
				/* 等号 */
				next->type = ECONF_TOKEN_EQU;
				next->str = malloc( 2 );

				next->str[0] = '=';
				next->str[1] = '\0';

				text++;

				break;
			} else if ( isspace( *text ) ) {
				/* 空白は飛ばす */
				do {
					text++;
				} while ( isspace( *text ) );
			} else if ( *text != '\0' ) {
				/* 後は文字列とみなす */
				start = text;

				next->type = ECONF_TOKEN_STR;

				/* 文字列切り出し */

                /* もし"で始まるなら"まで */
                if ( *start == '\"' ) {
                	do {
						text++;
					} while ( ( *text != '\0' ) && ( *text != '\"' ) && ( *text != '=' ) );

                    if ( *text == '\"' ) {
                    	text++;
                    }
                } else {
                	do {
						text++;
					} while ( ( *text != '\0' ) && ( !isspace( *text ) ) && ( *text != '=' ) );
                }

				next->str = malloc( text - start + 1 );

				memcpy( next->str, start, text - start );
				next->str[text - start] = '\0';

				break;
			} else {
				/* \0等 ( エラー発生 ) */
				next->type = ECONF_TOKEN_ERR;
				next->str = NULL;

				break;
			}
		}

		/* エラー出なければ続く */
		if ( next->type != ECONF_TOKEN_ERR ) {
			next->next = malloc( sizeof(struct ECONF_TOKEN) );
		} else {
			next->next = NULL;
		}
	}

	/* 先頭を返す */
	return top;
}

int econf_recog( struct ECONF_FILE_CONTAINER *ef )
{
	/* 認識する ( パーサー？ ) */
	char buf[ECONF_MAX_STR_LEN];
	char *text;
	struct ECONF_LINE *next;
	struct ECONF_LINE *top;
	struct ECONF_TOKEN *left;
	struct ECONF_TOKEN *right;
	struct ECONF_TOKEN *equ;
	struct ECONF_TOKEN *next_tok;
	int count;
	int i;
	
	/* ひらけてる？ */
	if ( ef->fp == NULL ) {
		return 0;
	}
	
	/* ファイルの頭へ */
	fseek( ef->fp, 0, SEEK_SET );
	
	/* 先頭確保 */
	top = malloc( sizeof(struct ECONF_LINE) );
	top->type = ECONF_LINE_TOP;
	top->left = NULL;
	top->right = NULL;
	
	count = 0;
	next = top;
	
	/* 1行ずつ読み込み */
	while ( fgets( buf, ECONF_MAX_STR_LEN, ef->fp ) ) {
		/* ポインタコピー */
		
		/* 1行解釈 */
		left = econf_recog_line( buf );
		
		/* 有効か？ */
		if ( left->type == ECONF_TOKEN_STR && left->next != NULL ) {
			if ( left->next->type == ECONF_TOKEN_EQU && left->next->next != NULL ) {
				if ( left->next->next->type == ECONF_TOKEN_STR ) {
					/* ちゃんと式だった */
					next->next = malloc( sizeof(struct ECONF_LINE) );
					next->next->type = ECONF_LINE_LINE;
					
					/* 文字コピー */
					next->next->left = malloc( strlen( left->str ) + 1 );
					strcpy( next->next->left, left->str );
					
					next->next->right = malloc( strlen( left->next->next->str ) + 1 );
					strcpy( next->next->right, left->next->next->str );
					
					/* next更新 */
					next = next->next;
					
					count++;
				}
			}
		}
		
		/* 開放 */
		econf_free_token( left );
	}
	
	/* 終了フラグ */
	next->next = NULL;

	/* うまくいったか？ */
	if ( !count ) {
		/* 失敗 */
		econf_free_line( top );
		
		return 0;
	}
	
	/* 配列作成 */
	ef->lines = malloc( sizeof(struct ECONF_LINE *) * count );
	ef->count = count;
	
	/* 登録 */
	for ( i = 0, next = top->next; i < count; i++, next = next->next ) {
		ef->lines[i] = next;
	}
	
	/* topはいらない */
	econf_free_line( top );
	
	/* 成功 */
	return 1;
}

void econf_free_line( struct ECONF_LINE *line )
{
	/* 線開放 */
	free( line->left );
	free( line->right );
	free( line );
}

void econf_free_token( struct ECONF_TOKEN *tok )
{
	/* token開放 */
	struct ECONF_TOKEN *next;
	struct ECONF_TOKEN *buf;
	
	next = tok;
	
	while ( next != NULL ) {
		free( next->str );
		
		buf = next;
		next = next->next;
		
		free( buf );
	}
}

struct ECONF_LINE *econf_search( struct ECONF_FILE_CONTAINER *ef, char *key )
{
	/* 検索 */
	int i;
	struct ECONF_LINE *ret;
	
	ret = NULL;
	
	for ( i = 0; i < ef->count; i++ ) {
		if ( !strcmp( key, ef->lines[i]->left ) ) {
			/* 発見 */
			ret = ef->lines[i];
			
			break;
		}
	}
	
	return ret;
}

double econf_as_double( struct ECONF_LINE *line )
{
	/* 実数として読む */
	char *buf;
	char *text;
	double ret;

	/* NULLならなにもしない */
	if ( line == NULL ) {
		return 0;
	}

	/* バッファ */
	buf = malloc( strlen( line->right ) + 1 );
	strcpy( buf, line->right );

	/* いらない文字を消す */
	for ( text = buf; *text != '\0'; text++ ) {
		if ( !isdigit( *text ) && *text != '-' && *text != '.' && *text != 'E' && *text != 'e' ) {
			*text = ' ';
		}
	}

	/* 数字に */
	ret = atof( buf );

	/* 開放 */
	free( buf );

	return ret;
}

char *econf_as_str( struct ECONF_LINE *line )
{
	/* 文字列としてそのまま読む */

	/* NULLなら空文字 */
	if ( line == NULL ) {
		return "";
	}

	return line->right;
}

int econf_close( struct ECONF_FILE_CONTAINER *ef )
{
	/* 閉じる */
	int i;
	
	if ( ef->fp == NULL ) {
		return 0;
	}
	
	fclose( ef->fp );
	
	for ( i = 0; i < ef->count; i++ ) {
		free( ef->lines[i]->left );
		free( ef->lines[i]->right );
		free( ef->lines[i] );
	}
	
	free( ef->lines );
	
	return 1;
}

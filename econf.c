#include "econf.h"

/* �ȈՐݒ胊�[�_�[ ver1.1 */
/* abc = xyz �̂悤�Ȃ��̂̂ݓǂ߂� */
/* �󔒕����͔�΂��A�G�X�P�[�v�V�[�P���X�͑��݂��Ȃ� */
/* �s�x�[�X�ŏo���Ă��ē���q�Ƃ��͉��߂��Ȃ����߁Akey�͈�ӂłȂ���΂Ȃ�Ȃ� */

int econf_open( struct ECONF_FILE_CONTAINER *ef, char *filename )
{
	/* �t�@�C�����J�� */
	
	/* ������ */
	ef->fp = NULL;
	ef->lines = NULL;
	ef->count = 0;
	
	if ( ( ef->fp = fopen( filename, "r" ) ) == NULL ) {
		/* ���s */
		return 0;
	}
	
	/* ���� */
	strcpy( ef->filename, filename );
	
	return 1;
}

struct ECONF_TOKEN *econf_recog_line( char *line )
{
	/* 1�s���߃��L�V�J���A�i���C�U */
	struct ECONF_TOKEN *top;
	struct ECONF_TOKEN *next;
	char *text;
	char *start;

	text = line;
	top = malloc( sizeof(struct ECONF_TOKEN) );

	for ( next = top; next != NULL; next = next->next ) {
		for ( ;; ) {
			if ( *text == '=' ) {
				/* ���� */
				next->type = ECONF_TOKEN_EQU;
				next->str = malloc( 2 );

				next->str[0] = '=';
				next->str[1] = '\0';

				text++;

				break;
			} else if ( isspace( *text ) ) {
				/* �󔒂͔�΂� */
				do {
					text++;
				} while ( isspace( *text ) );
			} else if ( *text != '\0' ) {
				/* ��͕�����Ƃ݂Ȃ� */
				start = text;

				next->type = ECONF_TOKEN_STR;

				/* ������؂�o�� */

                /* ����"�Ŏn�܂�Ȃ�"�܂� */
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
				/* \0�� ( �G���[���� ) */
				next->type = ECONF_TOKEN_ERR;
				next->str = NULL;

				break;
			}
		}

		/* �G���[�o�Ȃ���Α��� */
		if ( next->type != ECONF_TOKEN_ERR ) {
			next->next = malloc( sizeof(struct ECONF_TOKEN) );
		} else {
			next->next = NULL;
		}
	}

	/* �擪��Ԃ� */
	return top;
}

int econf_recog( struct ECONF_FILE_CONTAINER *ef )
{
	/* �F������ ( �p�[�T�[�H ) */
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
	
	/* �Ђ炯�Ă�H */
	if ( ef->fp == NULL ) {
		return 0;
	}
	
	/* �t�@�C���̓��� */
	fseek( ef->fp, 0, SEEK_SET );
	
	/* �擪�m�� */
	top = malloc( sizeof(struct ECONF_LINE) );
	top->type = ECONF_LINE_TOP;
	top->left = NULL;
	top->right = NULL;
	
	count = 0;
	next = top;
	
	/* 1�s���ǂݍ��� */
	while ( fgets( buf, ECONF_MAX_STR_LEN, ef->fp ) ) {
		/* �|�C���^�R�s�[ */
		
		/* 1�s���� */
		left = econf_recog_line( buf );
		
		/* �L�����H */
		if ( left->type == ECONF_TOKEN_STR && left->next != NULL ) {
			if ( left->next->type == ECONF_TOKEN_EQU && left->next->next != NULL ) {
				if ( left->next->next->type == ECONF_TOKEN_STR ) {
					/* �����Ǝ������� */
					next->next = malloc( sizeof(struct ECONF_LINE) );
					next->next->type = ECONF_LINE_LINE;
					
					/* �����R�s�[ */
					next->next->left = malloc( strlen( left->str ) + 1 );
					strcpy( next->next->left, left->str );
					
					next->next->right = malloc( strlen( left->next->next->str ) + 1 );
					strcpy( next->next->right, left->next->next->str );
					
					/* next�X�V */
					next = next->next;
					
					count++;
				}
			}
		}
		
		/* �J�� */
		econf_free_token( left );
	}
	
	/* �I���t���O */
	next->next = NULL;

	/* ���܂����������H */
	if ( !count ) {
		/* ���s */
		econf_free_line( top );
		
		return 0;
	}
	
	/* �z��쐬 */
	ef->lines = malloc( sizeof(struct ECONF_LINE *) * count );
	ef->count = count;
	
	/* �o�^ */
	for ( i = 0, next = top->next; i < count; i++, next = next->next ) {
		ef->lines[i] = next;
	}
	
	/* top�͂���Ȃ� */
	econf_free_line( top );
	
	/* ���� */
	return 1;
}

void econf_free_line( struct ECONF_LINE *line )
{
	/* ���J�� */
	free( line->left );
	free( line->right );
	free( line );
}

void econf_free_token( struct ECONF_TOKEN *tok )
{
	/* token�J�� */
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
	/* ���� */
	int i;
	struct ECONF_LINE *ret;
	
	ret = NULL;
	
	for ( i = 0; i < ef->count; i++ ) {
		if ( !strcmp( key, ef->lines[i]->left ) ) {
			/* ���� */
			ret = ef->lines[i];
			
			break;
		}
	}
	
	return ret;
}

double econf_as_double( struct ECONF_LINE *line )
{
	/* �����Ƃ��ēǂ� */
	char *buf;
	char *text;
	double ret;

	/* NULL�Ȃ�Ȃɂ����Ȃ� */
	if ( line == NULL ) {
		return 0;
	}

	/* �o�b�t�@ */
	buf = malloc( strlen( line->right ) + 1 );
	strcpy( buf, line->right );

	/* ����Ȃ����������� */
	for ( text = buf; *text != '\0'; text++ ) {
		if ( !isdigit( *text ) && *text != '-' && *text != '.' && *text != 'E' && *text != 'e' ) {
			*text = ' ';
		}
	}

	/* ������ */
	ret = atof( buf );

	/* �J�� */
	free( buf );

	return ret;
}

char *econf_as_str( struct ECONF_LINE *line )
{
	/* ������Ƃ��Ă��̂܂ܓǂ� */

	/* NULL�Ȃ�󕶎� */
	if ( line == NULL ) {
		return "";
	}

	return line->right;
}

int econf_close( struct ECONF_FILE_CONTAINER *ef )
{
	/* ���� */
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

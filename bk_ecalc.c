#include "ecalc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

/* �ȒP�d��C�t�@�C�� ecalc ver1.3 */
/* malloc�L��o�[�W���� */
/* 2011�N�X�V�� */

/* IF���́A���Ӓl���^�̏ꍇ�͉E�ӂ̎���]�����A�����̂̒l�͍��Ӓl�ƂȂ� */
/* �p�x���[�h��C�̃��C�u�����Ɉˑ� ( �������W�A�� ) */
/* �v�Z�͑S��double�ŁA�~�����̒l��M_PI */

/* ���Z�q�D��x��
 1 | ()		| �J�b�R	| ->
 2 | �֐�	| �֐�		| ->
 3 | +,-	| �P��-.+	| ->
 4 | *,/	| �揜�Z	| ->
 5 | +,-	| �����Z	| ->
 6 | <,>	| ��r		| ->
 7 | @		| ���[�v	| <-
 8 | =		| ���		| <-
 9 | ,		| ��؂�	| ->
*/

/* �������}�l�[�W���[�p�g�[�N�� */
static struct ECALC_TOKEN ecalc_tokens[ECALC_MEMMAN_TOKENS];

/* �\��ꃊ�X�g */
static struct ECALC_RESERVED_CONTAINER ecalc_reserved[] = {
	{ "PI",		ECALC_TOKEN_FUNC_NON,	ECALC_FUNC_PI },
	{ "EPS0",	ECALC_TOKEN_FUNC_NON,	ECALC_FUNC_EPS0 },
	{ "ANS",	ECALC_TOKEN_FUNC_NON,	ECALC_FUNC_ANS },
	{ "SIN",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_SIN },
	{ "COS",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_COS },
	{ "TAN",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_TAN },
	{ "ASIN",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_ASIN },
	{ "ACOS",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_ACOS },
	{ "ATAN",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_ATAN },
	{ "LN",		ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_LOGN },
	{ "LOG",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_LOG10 },
	{ "ROOT",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_SQRT },
	{ "RAD",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_RAD },
	{ "DEG",	ECALC_TOKEN_FUNC_ONE,	ECALC_FUNC_DEG },
	{ "POW",	ECALC_TOKEN_FUNC_TWO,	ECALC_FUNC_POW },
	{ "POWER",	ECALC_TOKEN_FUNC_TWO,	ECALC_FUNC_POW },
	{ "IF",		ECALC_TOKEN_FUNC_TWO,	ECALC_FUNC_IF }
};

/* ------------- �������}�l�[�W�� ------------- */

void ecalc_memman_init( void )
{
	/* �������}�l�[�W�������� */

	/* malloc�łł͂Ȃɂ����Ȃ� */
	return;
}

struct ECALC_TOKEN *ecalc_memman_malloc( void )
{
	/* ��g�[�N�������炤 */
	struct ECALC_TOKEN *ret;
	
	ret = malloc( sizeof(struct ECALC_TOKEN) );
	
	return ret;
}

void ecalc_memman_free( struct ECALC_TOKEN *tok )
{
	/* �g�[�N����J�� */
	free( tok );
}

/* ------------- ���[�U�[�p�֐� ------------- */

double ecalc_container_get_value( struct ECALC_CONTAINER *cont )
{
	/* �l�𓾂� */
	return ecalc_get_tree_value( cont->token, cont->vars, cont->ans );
}

int ecalc_container_recog( struct ECALC_CONTAINER *cont, char *text )
{
	/* �������؂� */
	ecalc_free_token( cont->token );
	
	cont->token = ecalc_make_token( text );
	cont->token = ecalc_make_tree( cont->token );
	
	if ( cont->token == NULL ) {
		/* ���s */
		return 0;
	} else {
		/* ���� */
		return 1;
	}
}

void ecalc_container_init( struct ECALC_CONTAINER *cont )
{
	/* �R���e�i�������� */
	cont->token = NULL;
}

void ecalc_container_free( struct ECALC_CONTAINER *cont )
{
	/* �R���e�i���J�� */
	ecalc_free_token( cont->token );
	cont->token = NULL;
}

/* ------------- �v�Z�@�֐� ------------- */

struct ECALC_TOKEN *ecalc_make_token( char *text )
{
	/* �e�L�X�g���g�[�N���ɕ��� ( ���L�V�J���A�i���C�U�[���ǂ� ) */
	struct ECALC_TOKEN *ret;
	char *point;
	char buf[ECALC_MAX_TOKEN_LENGTH];
	int i;
	int resvs;
	
	/* �֐��̐� */
	resvs = sizeof(ecalc_reserved) / sizeof(ecalc_reserved[0]);
	
	/* �g�[�N���ЂƂm�� */
	ret = ecalc_malloc_token();

L_ECALC_ANSYS:
	
	/* �l���ꎞ�ۑ� */
	point = text;
	
	/* �擪�́H */
	if ( isdigit( *text ) ) {
		/* ���������� */
		
		/* 0x�̕\�L������ */
		if ( ( *text == '0' ) && ( *( text + 1 ) == 'x' || *( text + 1 ) == 'X' ) ) {
			/* 0x��΂� */
			text += 2;
			
			/* �����̊Ԃ͌J��Ԃ� */
			do {
				text++;
			} while ( isxdigit( *text ) || *text == '.' );
		} else {
			/* 0x�ł͂Ȃ����� */

			/* �����̊Ԃ͌J��Ԃ� */
			do {
				text++;
			} while ( isdigit( *text ) || *text == '.' );
		}

		/* �����񂪍ő�g�[�N�����������I�[�o�[���Ă��Ȃ����H */
		if ( ( text - point ) > ( ECALC_MAX_TOKEN_LENGTH - 1 ) ) {
			/* �G���[�g�[�N���ɕύX */
			ret->type   = ECALC_TOKEN_ERROR;
			ret->value  = ECALC_ERROR_TOO_LONG;
			
			strncpy( ret->str, point, ECALC_MAX_TOKEN_LENGTH - 1 );
			ret->str[ECALC_MAX_TOKEN_LENGTH - 1] = '\0';
		} else {		
			/* ���Ȃ��̂Ńg�[�N�����o�b�t�@�� */
			memcpy( buf, point, text - point );
			buf[text - point] = '\0';
			
			/* �i�[ */
			ret->type  = ECALC_TOKEN_LITE;
			ret->value = strtod( buf, NULL );
			strcpy( ret->str, buf );
		}
	} else if ( isalpha( *text ) ) {
		/* �A���t�@�x�b�g������ */
		
		/* �\��ꌟ�� */
		for ( i = 0; i < resvs; i++ ) {
			if ( ecalc_strncmp( text, ecalc_reserved[i].text, strlen( ecalc_reserved[i].text ) ) == 0 ) {
				/* �\��ꔭ�� */
				ret->type  = ecalc_reserved[i].type;
				ret->value = ecalc_reserved[i].value;
				strcpy( ret->str, ecalc_reserved[i].text );
				
				/* �����񕪂����߂� */
				text += strlen( ecalc_reserved[i].text );
				
				break;
			}
		}
		
		/* �֐��Ɉ�v���Ȃ������̂ŕϐ��Ƃ��ď��� */
		if ( i >= resvs ) {
			ret->type  = ECALC_TOKEN_VAR;
			ret->value = toupper( *text ) - 0x41;
			
			*(ret->str)     = *text;
			*(ret->str + 1) = '\0';
			
			/* ��i�� */
			text++;
		}
	} else if ( *text == '(' ) {
		/* ������ */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_LBRA;
		ret->value = 0;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == ')' ) {
		/* �E���� */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_RBRA;
		ret->value = 0;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '*' ) {
		/* ��Z */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_MUL;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '/' ) {
		/* ���Z */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_DIV;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
		
	} else if ( *text == '+' ) {
		/* + */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_ADD;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '-' ) {
		/* - */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_SUB;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '>' ) {
		/* ���傫�� */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_LBIG;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '<' ) {
		/* �E�傫�� */
		
		/* �i�[ */
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_RBIG;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '=' ) {
		/* ��� */
		
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_STI;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == ',' ) {
		/* ��؂� */
		
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_SEPA;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( *text == '@' ) {
		/* ���[�v */
		
		ret->type  = ECALC_TOKEN_OPE;
		ret->value = ECALC_OPE_LOOP;
		
		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* ��i�� */
		text++;
	} else if ( isspace( *text ) ) {
		/* �󔒕��� */
		
		/* �󔒂̊Ԃ͌J��Ԃ� */
		do {
			text++;
		} while ( isspace( *text ) );
		
		goto L_ECALC_ANSYS;
		
	} else if ( *text == '\0' ) {
		/* �I�[�R�[�h */
		
		ret->type  = ECALC_TOKEN_END;
		ret->value = 0;
		ret->next  = NULL;

		*(ret->str) = '\0';
		
		/* �I��� */
		return ret;
	} else {
		/* ���̑� ( �G���[ ) */
		
		ret->type  = ECALC_TOKEN_ERROR;
		ret->value = ECALC_ERROR_BAD_CHAR;
		/* ret->next  = NULL; */ 					/* �G���[�g�[�N���ŏI���Ȃ�R�����g������ */

		*(ret->str)     = *text;
		*(ret->str + 1) = '\0';
		
		/* �I��� �G���[�g�[�N���ŏI���Ȃ�R�����g������ */
		/* return ret; */
		
		text++;										/* �G���[�g�[�N���ŏI���Ȃ�R�����g�A�E�g */
	}
	
	/* �g�[�N����������āA�����Ɍq�� */
	ret->next = ecalc_make_token( text );
	
	return ret;
}

struct ECALC_TOKEN *ecalc_make_tree( struct ECALC_TOKEN *token )
{
	/* �g�[�N���A�����X�g����c���[����� ( �p�[�T�[���ǂ� ) */
	int i;
	int count;
	int err_count;
	int c_lbra;

	struct ECALC_TOKEN b_left;
	struct ECALC_TOKEN *top;
	struct ECALC_TOKEN *next;
	struct ECALC_TOKEN *forw;
	struct ECALC_TOKEN *b_tok;
	struct ECALC_TOKEN *b_rbra;
	struct ECALC_TOKEN *b_rbra_next;
	struct ECALC_TOKEN *b_ret;
	struct ECALC_TOKEN *ret;

	/* �ŏ�ʂ̃g�[�N������� ( ��ɒP���}�C�i�X���Z�p ) */
	top = ecalc_malloc_token();
	top->next  = token;
	top->type  = ECALC_TOKEN_TOP;
	top->value = 0;
	
	/* �l������ */
	err_count = 0;
	
	/* �J�b�R�̏��� �D��x1 */
	for ( next = top; next != NULL; next = next->next ) {
		/* �����ʔ��� */
		if ( next->type == ECALC_TOKEN_LBRA ) {
			/* �E���ʂ�T�� ( �Ō�܂� ) */
			b_rbra = NULL;
			c_lbra = 1;
			
			/* �J�b�R�̊J�����𐔂��� ( ( 4 - 34 * ( 200 ) ) -34 ) �݂����Ȃ̂ɑΉ����邽�� */
			for ( forw = next->next; ( forw != NULL ) && ( c_lbra > 0 ); forw = forw->next ) {
				/* ���������� */
				if ( forw->type == ECALC_TOKEN_LBRA ) {
					c_lbra++;										/* �����ʂ�����ɊJ���� */
				} else if ( forw->type == ECALC_TOKEN_RBRA ) {
					b_rbra = forw;
					c_lbra--;										/* �E���ʂ����� */
				}
			}
			
			/* �E���ʂ������� */
			if ( b_rbra != NULL ) {
				/* ���ʓ��̃|�C���^��ێ� */
				b_tok = next->next;
				
				/* )�̎���ۑ� */
				b_rbra_next = b_rbra->next;
				
				/* �����ʂ����� */
				next->type = ECALC_TOKEN_EXP;
				
				/* �E���ʂ��I�[�L���ɕϊ� */
				b_rbra->type = ECALC_TOKEN_END;
				b_rbra->next = NULL;
				
				/* ���ʓ����������Ă��炤 */
				b_ret = ecalc_make_tree( b_tok );
				
				/* �G���[���N���� ( ������return���Ă������͂������A�G���[�̐��𐔂��Ă݂� ) */
				if ( b_ret == NULL ) {
					next->type = ECALC_TOKEN_ERROR;
					err_count++;
				} else {
					/* ���������̂�(��EXP�ɕύX */
					*next = *b_ret;

					/* ()���̖؂̃g�b�v�̓��ꕨ�͕K�v�Ȃ��̂ŉ�� */
					ecalc_memman_free( b_ret );
				}
				
				/* �Đڑ� */
				next->next = b_rbra_next;
			}
		}
	}
	
L_ECALC_RECHECK_FUNC:

	/* �֐��Ăяo�� �D��x2 */
	for ( next = top; next != NULL; next = next->next ) {
		if ( next->type == ECALC_TOKEN_FUNC_NON ) {				/* �萔�֐� */
			/* �����i�[����exp�ɕύX */
			next-> type = ECALC_TOKEN_EXP;
			next->left  = NULL;
			next->right = NULL;
			
			/* �ă`�F�b�N */
			goto L_ECALC_RECHECK_FUNC;
		} else if ( next->type == ECALC_TOKEN_FUNC_ONE ) {		/* �P�����Z�֐� */
			if ( next->next != NULL ) {
				if ( ecalc_token_is_value( next->next ) ) {
					/* �����\�Ȃ̂ŁA���� */
					next->right = next->next;					/* �����͉E�Ӓl�ɗ^���� */
					next->left  = NULL;							/* �P���Ȃ̂ō��ӂ͂���Ȃ� */
					
					/* �l�ݒ� */
					next->type = ECALC_TOKEN_EXP;
					
					/* �Đڑ� */
					next->next = next->next->next;
					
					/* �_�u��Ȃ��悤�ɐݒ� */
					next->right->next = NULL;
					
					/* �ă`�F�b�N */
					goto L_ECALC_RECHECK_FUNC;
				}
			}
		} else if ( ecalc_token_is_value( next ) ) {			/* �Q�����Z�֐� */
			if ( next->next != NULL ) {
				if ( next->next->type == ECALC_TOKEN_FUNC_TWO ) {
					if ( next->next->next != NULL ) {
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* �E�ӂƍ��ӌ����\�Ȃ̂Ō��� */
							ecalc_make_exp( next );
							
							/* �ă`�F�b�N */
							goto L_ECALC_RECHECK_FUNC;
						}
					}
				}
			}
		}
	}
	
L_ECALC_RECHECK_SINGLE_MP:
	
	/* �P�����Z�q(-,+)������ �D��x3 ��������<- */
	for ( next = top, b_tok = NULL; next != NULL; next = next->next ) {
		if ( !ecalc_token_is_value( next ) ) {					/* �l�������Ȃ��A���Z�q */
			if ( next->next != NULL ) {
				if ( ( next->next->type == ECALC_TOKEN_OPE ) &&
					 ( ( next->next->value == ECALC_OPE_SUB ) || ( next->next->value == ECALC_OPE_ADD ) ) ) {
					if ( next->next->next != NULL ) {
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* �E�ӂɒl�����P�����Z�q�𔭌� */
							
							/* �ꏊ���L�� */
							b_tok = next->next;
						}
					}
				}
			}
		}
	}
	
	if ( b_tok != NULL ) {
		/* �P�����Z���� */

		/* ���ɕϊ� */
		b_tok->type = ECALC_TOKEN_EXP;
		
		/* ���Ӓl�A�E�Ӓl�ݒ� */
		b_tok->left  = ecalc_malloc_token();
		b_tok->right = b_tok->next;
		
		/* 0����� */
		b_tok->left->value = 0;
		b_tok->left->type  = ECALC_TOKEN_LITE;
		
		/* �Đڑ� */
		b_tok->next = b_tok->next->next;
		
		/* �_�u��Ȃ��悤�� */
		b_tok->right->next = NULL;
		
		/* �ă`�F�b�N */
		goto L_ECALC_RECHECK_SINGLE_MP;
	}
	
L_ECALC_RECHECK_MUL:
	
	/* ��Z�A���Z�̏��� �D��x4 */
	for ( next = top; next != NULL; next = next->next ) {
		/* ���Ӓl�ɐ��肤����̂𔭌� */
		if ( ecalc_token_is_value( next ) ) {
			/* ��������H */
			if ( next->next != NULL ) {
				if ( ( next->next->type == ECALC_TOKEN_OPE && next->next->value == ECALC_OPE_MUL ) ||
				     ( next->next->type == ECALC_TOKEN_OPE && next->next->value == ECALC_OPE_DIV ) ) {
					/* ���Z�q����揜�Z */
					if ( next->next->next != NULL ) {
						/* ���ɗ���̂��E�Ӓl�ɂȂ蓾��H */
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* �v�Z�\�I */
							
							/* ���ӂƉE�ӂ����Z�q�Ō��� */
							ecalc_make_exp( next );
							
							/* �����������A�������g��������x�]�����ė~�����̂ŁAfor�ɓ��蒼�� */
							goto L_ECALC_RECHECK_MUL;
						}
					}
				} else if ( ( next->next->type == ECALC_TOKEN_VAR ) || ( next->next->type == ECALC_TOKEN_EXP ) ) {
					/* ���Z�q�Ȃ��揜�Z */
					
					/* ��m�� */
					b_tok = ecalc_malloc_token();
					
					/* ���Ӓl�R�s�[ */
					*b_tok = *next;
					
					/* ���ɕϊ� */
					next->type  = ECALC_TOKEN_EXP;
					next->value = ECALC_OPE_MUL;
					
					/* ���Ӓl�A�E�Ӓl�ݒ� */
					next->left  = b_tok;
					next->right = next->next;
					
					/* �Đڑ� */
					next->next = next->next->next;
					
					/* �_�u��Ȃ��悤�� */
					next->right->next = NULL;
					next->left->next  = NULL;
					
					/* �������g���ĕ]���������̂�for�ɓ���Ȃ��� */
					goto L_ECALC_RECHECK_MUL;
				}
			}
		}
	}

L_ECALC_RECHECK_ADD:

	/* +-�̏��� �D��x5 */
	for ( next = top; next != NULL; next = next->next ) {
		if ( ecalc_token_is_value( next ) ) {
			/* �������� */
			if ( next->next != NULL ) {
				/* ����+-������ */
				if ( ( next->next->type == ECALC_TOKEN_OPE && next->next->value == ECALC_OPE_SUB ) ||
				     ( next->next->type == ECALC_TOKEN_OPE && next->next->value == ECALC_OPE_ADD ) ) {
					/* �E�Ӓl�����݂��� */
					if ( next->next->next != NULL ) {
						/* �����\ */
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* +-�ł������A�����Ȃ����Ƃ͂Ȃ��̂ŁA�����ڑ� */

							/* ���Z */
							ecalc_make_exp( next );
							
							/* �����������A�������g��������x�]�����ė~�����̂ŁAfor�ɓ��蒼�� */
							goto L_ECALC_RECHECK_ADD;
						}
					}
				}
			}
		}
	}
	
L_ECALC_RECHECK_BIG:

	/* ��r���Z�q �D��x6 */
	for ( next = top; next != NULL; next = next->next ) {
		if ( ecalc_token_is_value( next ) ) {
			/* �������� */
			if ( next->next != NULL ) {
				/* ����+-������ */
				if ( ( next->next->type == ECALC_TOKEN_OPE ) &&
				     ( ( next->next->value == ECALC_OPE_RBIG ) || ( next->next->value == ECALC_OPE_LBIG ) ) ) {
					/* �E�Ӓl�����݂��� */
					if ( next->next->next != NULL ) {
						/* �����\ */
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* ���Z */
							ecalc_make_exp( next );
							
							/* �����������A�������g��������x�]�����ė~�����̂ŁAfor�ɓ��蒼�� */
							goto L_ECALC_RECHECK_BIG;
						}
					}
				}
			}
		}
	}
	
L_ECALC_RECHECK_LOOP:
	
	/* ���[�v �������� <- �D��x7 */
	for ( next = top, b_tok = NULL; next != NULL; next = next->next ) {
		/* ���[�v�L������ԉE�܂Œ��� */
		if ( ecalc_token_is_value( next ) ) {
			/* �������� */
			if ( next->next != NULL ) {
				/* ��������L�������� */
				if ( ( next->next->type == ECALC_TOKEN_OPE ) && ( next->next->value == ECALC_OPE_LOOP ) ) {
					/* �E�Ӓl�����݂��� */
					if ( next->next->next != NULL ) {
						/* �E�Ӓl�͎��ȊO������Ȃ� */
						if ( next->next->next->type == ECALC_TOKEN_EXP ) {
							/* �ʒu���o���Ă��� */
							b_tok = next;
						}
					}
				}
			}
		}
	}
	
	/* ���[�v�L���������������H */
	if ( b_tok != NULL ) {
		/* ���Z */
		ecalc_make_exp( b_tok );
		
		/* �ă`�F�b�N���Ă��炤 */
		goto L_ECALC_RECHECK_LOOP;
	}
	
L_ECALC_RECHECK_STI:
	
	/* ��� �������� <- �D��x8 */
	for ( next = top, b_tok = NULL; next != NULL; next = next->next ) {
		/* =����ԉE�܂Œ��� */
		if ( next->type == ECALC_TOKEN_VAR ) {
			/* �������� */
			if ( next->next != NULL ) {
				/* ��������L�������� */
				if ( ( next->next->type == ECALC_TOKEN_OPE ) && ( next->next->value == ECALC_OPE_STI ) ) {
					/* �E�Ӓl�����݂��� */
					if ( next->next->next != NULL ) {
						/* �����\ */
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* �ʒu���o���Ă��� */
							b_tok = next;
						}
					}
				}
			}
		}
	}
	
	/* ����L���������������H */
	if ( b_tok != NULL ) {
		/* ���Z */
		ecalc_make_exp( b_tok );
		
		/* �ă`�F�b�N���Ă��炤 */
		goto L_ECALC_RECHECK_STI;
	}
	
L_ECALC_RECHECK_SEPA:
	
	/* ����؂� �D��x9 */
	for ( next = top; next != NULL; next = next->next ) {
		if ( ecalc_token_is_value( next ) ) {
			/* �������� */
			if ( next->next != NULL ) {
				/* ������؂蕶�������� */
				if ( ( next->next->type == ECALC_TOKEN_OPE ) && ( next->next->value == ECALC_OPE_SEPA ) ) {
					/* �E�Ӓl�����݂��� */
					if ( next->next->next != NULL ) {
						/* �����\ */
						if ( ecalc_token_is_value( next->next->next ) ) {
							/* �ڑ����� */

							/* ���Z */
							ecalc_make_exp( next );
							
							/* �������g���ă`�F�b�N */
							goto L_ECALC_RECHECK_SEPA;
						}
					}
				}
			}
		}
	}
	
	/* �c���[�쐬�I���A���ʂ𒲂ׂ� */
	
	/* �c�����g�[�N���𐔂��� */
	for ( count = 0, next = top; next != NULL; count++ ) {
		next = next->next;
	}
	
	/* ��͌��ʂ��`�F�b�N ( token��NULL���^�����Ă����ꍇ��count = 1�Ȃ̂Ŗ��Ȃ� ) */
	if ( err_count ) {
		/* �G���[���� */
		ret = NULL;
		ecalc_free_token( top );
	} else if ( count == 3 ) {
		/* ���� */
		
		/* EXP�I�����[? */
		if ( top->next->type == ECALC_TOKEN_EXP ) {
			/* ���̂܂܋A�� */
			ret = top->next;
			
			/* ����Ȃ����̂��J�� */
			top->next = top->next->next;
			ecalc_free_token( top );
			ret->next = NULL;
		} else if ( ( top->next->type == ECALC_TOKEN_VAR ) || ( top->next->type == ECALC_TOKEN_LITE ) ) {
			/* ���l */
			
			/* �g�[�N���m�� */
			b_tok = ecalc_malloc_token();
			ret   = ecalc_malloc_token();
			
			/* �g�b�v��EXP���쐬 */
			ret->value = ECALC_OPE_ADD;
			ret->left  = b_tok;
			ret->right = top->next;
			ret->type  = ECALC_TOKEN_EXP;
			
			/* 0����� */
			b_tok->value = 0;
			b_tok->type  = ECALC_TOKEN_LITE;

			/* token���J������ */
			top->next = top->next->next;
			ret->right->next = NULL;
			ecalc_free_token( top );
		} else {
			/* ���s */
			ret = NULL;
			ecalc_free_token( top );
		}
	} else {
		/* ���S�Ɏ��s */
		ret = NULL;
		ecalc_free_token( top );
	}
	
	return ret;
}

int ecalc_token_is_value( struct ECALC_TOKEN *token )
{
	/* �^����ꂽ�g�[�N�����l�������`�F�b�N */
	if ( ( token->type == ECALC_TOKEN_LITE ) ||
	     ( token->type == ECALC_TOKEN_VAR )   ||
	     ( token->type == ECALC_TOKEN_EXP ) ) {
		return 1;
	} else {
		return 0;
	}
}

void ecalc_make_exp( struct ECALC_TOKEN *left )
{
	/* ���ӂƉE�ӂ��������Ď������ */
	struct ECALC_TOKEN b_left;
	struct ECALC_TOKEN *b_tok;
	
	/* ���ӂ����Ӓl�ɁA�E�ӂ��E�Ӓl�ɐڑ� ( ���Ӓl�����Z�q�ƌ��� ) */
	left->next->left  = left->next;
	left->next->right = left->next->next;

	/* ���Z�q�����֕ϊ� */
	left->next->type = ECALC_TOKEN_EXP;
	
	/* ���Z�q������TOKEN�����Ӓl�Ɠ���ւ� */
	b_left   = *left;							/* ���Ӓl�o�b�t�@�����O */
	b_tok    = left->next;						/* ���Z�q�̈ʒu���L�� */
	*left    = *(left->next);					/* ���Z�q�����Ӓl�ɂ����čs�� */	
	*(b_tok) = b_left;							/* ���Ӓl�����Z�q�̂����Ƃ���� */
	
	/* �Đڑ� */
	left->next = left->right->next;
	
	/* �l���_�u��Ȃ��悤�ɂ��Ă��� */
	left->left->next  = NULL;
	left->right->next = NULL;
}

double ecalc_get_tree_value( struct ECALC_TOKEN *token, double **vars, double ans )
{
	/* �c���[����l�𓾂� */
	int i;
	double left  = 0;			/* �����l��0�Ȃ̂ŗႦ��0@(3)��0�ƂȂ� */
	double right = 0;
	double *left_var;

	/* NULL�Ȃ牽�����Ȃ� */
	if ( token == NULL ) {
		return 0;
	}

	/* EXP�ȊO��^������Ƌ����I��0 */
	if ( token->type != ECALC_TOKEN_EXP ) {
		return 0;
	}

	/* ���ӎ擾 */
	if ( token->left != NULL ) {
		if ( token->left->type == ECALC_TOKEN_LITE ) {
			left = token->left->value;
		} else if ( token->left->type == ECALC_TOKEN_VAR ) {
			left     = *vars[(int)token->left->value];
			left_var = vars[(int)token->left->value];
		} else if ( token->left->type == ECALC_TOKEN_EXP ) {
			left = ecalc_get_tree_value( token->left, vars, ans );
		} else {
			left = 0;
		}
	}
	
	/* �E�ӎ擾 */
	if ( token->right != NULL ) {
		if ( token->right->type == ECALC_TOKEN_LITE ) {
			right = token->right->value;
		} else if ( token->right->type == ECALC_TOKEN_VAR ) {
			right = *vars[(int)token->right->value];
		} else if ( token->right->type == ECALC_TOKEN_EXP ) {
			/* �E�ӂ����Ƃ��Ĉ����ꍇ�A�����ŏ��� */
			if ( token->value == ECALC_OPE_LOOP ) {						/* ���[�v */
				for ( i = 0; i < (int)left; i++ ) {
					right = ecalc_get_tree_value( token->right, vars, ans );
				}
			} else if ( token->value == ECALC_FUNC_IF ) {				/* IF */
				if ( left ) {
					ecalc_get_tree_value( token->right, vars, ans );
				}
				
				right = left;
			} else {													/* ���� */
				right = ecalc_get_tree_value( token->right, vars, ans );
			}
		} else {
			right = 0;
		}
	}
	
	/* �v�Z */
	switch ( (int)token->value ) {
		case ECALC_OPE_ADD:					/* �����Z */
			return left + right;
		case ECALC_OPE_SUB:					/* �����Z */
			return left - right;
		case ECALC_OPE_MUL:					/* �|���Z */
			return left * right;
		case ECALC_OPE_DIV:					/* ����Z */
			/* / 0�̓G���[�𔭐������Ȃ� */
			if ( right == 0 ) {
				return 0;
			} else {
				return left / right;
			}
		case ECALC_OPE_STI:					/* ��� */
			return ( *left_var = right );
		case ECALC_OPE_SEPA:				/* ��؂� */
			return right;
		case ECALC_OPE_LOOP:				/* �J��Ԃ� */
			return right;
		case ECALC_OPE_LBIG:				/* �����傫�� */
			return ( left > right );
		case ECALC_OPE_RBIG:				/* �E���傫�� */
			return ( left < right );
		case ECALC_FUNC_SIN:				/* sin */
			return sin( right );
		case ECALC_FUNC_COS:				/* cos */
			return cos( right );
		case ECALC_FUNC_TAN:				/* tan */
			return tan( right );
		case ECALC_FUNC_ASIN:				/* asin */
			return asin( right );
		case ECALC_FUNC_ACOS:				/* acos */
			return acos( right );
		case ECALC_FUNC_ATAN:				/* atan */
			return atan( right );
		case ECALC_FUNC_LOG10:				/* log10 */
			return log10( right );
		case ECALC_FUNC_LOGN:				/* logn */
			return log( right );
		case ECALC_FUNC_SQRT:				/* root */
			return sqrt( right );
		case ECALC_FUNC_POW:				/* power */
			return pow( left, right );
		case ECALC_FUNC_RAD:				/* rad */
			return ( ( right / 180 ) * M_PI );
		case ECALC_FUNC_DEG:				/* deg */
			return ( ( right / M_PI ) * 180.0 );
		case ECALC_FUNC_PI:					/* �� */
			return M_PI;
		case ECALC_FUNC_EPS0:				/* ��0 */
			return 8.85418782e-12;
		case ECALC_FUNC_ANS:				/* ans */
			return ans;
		case ECALC_FUNC_IF:					/* if�� */
			return right;
		default:
			return 0;
	}
}

void ecalc_free_token( struct ECALC_TOKEN *token )
{
	/* �g�[�N�����J�� */

	/* NULL����Ȃ����� */
	if ( token != NULL ) {
		/* left������ΊJ�� */
		if ( token->left != NULL ) {
			ecalc_free_token( token->left );
		}

		/* right������ΊJ�� */
		if ( token->right != NULL ) {
			ecalc_free_token( token->right );
		}

		/* next������Ή�� */
		if ( token->next != NULL ) {
			ecalc_free_token( token->next );
		}
	}

	/* �ŏ��̂��J�� */
	ecalc_memman_free( token );
}

struct ECALC_TOKEN *ecalc_malloc_token( void )
{
	/* �g�[�N�������� */
	struct ECALC_TOKEN *ret;
	
	ret = ecalc_memman_malloc();
	
	ret->value  = 0;
	ret->next   = NULL;
	ret->left   = NULL;
	ret->right  = NULL;
	ret->str[0] = '\0';
	ret->type   = ECALC_TOKEN_ALLOC;
	
	return ret;
}

int ecalc_strncmp( char *str1, char *str2, int n )
{
	/* �啶������������ʂ��Ȃ�strncmp */
	int i, ret;
	
	ret = 0;
	
	for ( i = 0; i < n; i++ ) {
		/* \0�ɏo���킷�Ǝ��s���� ( �{���ł͂����ŏ������~�߂� = break ) */
		if ( str1[i] == '\0' || str2[i] == '\0' ) {
			return 1;
		}
		
		/* ����Ă���0�ȊO��Ԃ� */
		if ( toupper( str1[i] ) != toupper( str2[i] ) ) {
			ret = 1;
		}
	}
	
	return ret;
}

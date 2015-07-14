#ifndef ECALC_H_INCLUDED
#define ECALC_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define ECALC_MEMMAN_TOKENS     512                         /* �������}�l�[�W���[�Ŏg���g�[�N���� */

#define ECALC_FUNC_COUNT        10                          /* �֐��̐� */
#define ECALC_MAX_TOKEN_LENGTH  64                          /* �ő�g�[�N�������� */
#define ECALC_VAR_COUNT         26                          /* �ő�ϐ��� */

enum ecalc_error {
	ECALC_ERROR_TOO_LONG,                                   /* ������ */
	ECALC_ERROR_BAD_CHAR                                    /* ���肦�Ȃ����� */
};

enum ecalc_connector {                                      /* �ڑ��q ( ���Z�q ) */
	ECALC_OPE_ADD,                                          /* �����Z */
	ECALC_OPE_SUB,                                          /* �����Z */
	ECALC_OPE_MUL,                                          /* �|���Z */
	ECALC_OPE_DIV,                                          /* ����Z */
	ECALC_OPE_MOD,
	ECALC_OPE_STI,                                          /* ��� */
	ECALC_OPE_SEPA,                                         /* ��؂� */
	ECALC_OPE_LOOP,                                         /* �J��Ԃ� */
	ECALC_OPE_LBIG,                                         /* > �����傫�� */
	ECALC_OPE_RBIG,                                         /* < �E���傫�� */
	ECALC_OPE_EQU,											/* == ���� */
	ECALC_OPE_ERR,                                          /* ���@�G���[ */

	ECALC_FUNC_PI,                                          /* �� */
	ECALC_FUNC_EPS0,                                        /* ��0 */
	ECALC_FUNC_ANS,                                         /* �O��̓��� */
	ECALC_FUNC_POW,                                         /* �w�� */
	ECALC_FUNC_SQRT,                                        /* ���[�g */
	ECALC_FUNC_LOG10,                                       /* �ΐ�10 */
	ECALC_FUNC_LOGN,                                        /* �ΐ����R */
	ECALC_FUNC_RAD,                                         /* �x�����W�A���� */
	ECALC_FUNC_DEG,                                         /* ���W�A����x�� */
	ECALC_FUNC_SIN,                                         /* sin */
	ECALC_FUNC_COS,                                         /* cos */
	ECALC_FUNC_TAN,                                         /* tan */
	ECALC_FUNC_ASIN,                                        /* asin */
	ECALC_FUNC_ACOS,                                        /* acos */
	ECALC_FUNC_ATAN,                                        /* atan */
	ECALC_FUNC_ATAN2,                                       /* atan */
	ECALC_FUNC_IF                                           /* if�� */
};

enum ecalc_token_type {                                     /* �g�[�N���̎�� */
	ECALC_TOKEN_LITE,                                       /* ���l */
	ECALC_TOKEN_OPE,                                        /* ���Z�q */
	ECALC_TOKEN_LBRA,                                       /* �������� */
	ECALC_TOKEN_RBRA,                                       /* �E������ */
	ECALC_TOKEN_VAR,                                        /* �ϐ� */
	ECALC_TOKEN_END,                                        /* �I�� */
	ECALC_TOKEN_TOP,                                        /* �擪 */
	ECALC_TOKEN_EXP,                                        /* �� */
	ECALC_TOKEN_ERROR,                                      /* �G���[ */
	ECALC_TOKEN_FUNC_NON,                                   /* �萔�֐� */
	ECALC_TOKEN_FUNC_ONE,                                   /* �P�����Z�֐� */
	ECALC_TOKEN_FUNC_TWO,                                   /* 2�����Z�֐� */
	ECALC_TOKEN_ALLOC,                                      /* �m�ۂ��ꂽ */
	ECALC_TOKEN_FREE                                        /* ���ł��Ȃ� */
};

struct ECALC_TOKEN {                                        /* �g�[�N���\���� */
	int type;                                               /* �g�[�N���̎�� */
	double value;                                           /* �g�[�N���̒l */
	char str[ECALC_MAX_TOKEN_LENGTH];                       /* �g�[�N�������� */

	struct ECALC_TOKEN *left;                               /* EXP�p���Ӓl */
	struct ECALC_TOKEN *right;                              /* �E�Ӓl */
	struct ECALC_TOKEN *next;                               /* ���̃g�[�N�� */
};

struct ECALC_CONTAINER {                                    /* �R���e�i */
	double *vars[ECALC_VAR_COUNT];                          /* �ϐ��̃|�C���^�z�� */
	double ans;                                             /* �O��̓��� */

	struct ECALC_TOKEN *token;                              /* �g�[�N�� */
};

struct ECALC_RESERVED_CONTAINER {
	char *text;                                             /* ������ */
	int type;                                               /* ��� */
	int value;                                              /* �@�\ */
	void (*func)( void );									/* �֐� */
};

#ifdef __cplusplus
extern "C" {
#endif

// �������[�}�l�[�W��
void ecalc_memman_init( void );                                                         /* �������}�l�[�W�������� */
struct ECALC_TOKEN *ecalc_memman_malloc( void );                                        /* malloc */
void ecalc_memman_free( struct ECALC_TOKEN *tok );                                      /* free */

// public
struct ECALC_TOKEN *ecalc_make_token( char *text );                                     /* �g�[�N���A�����X�g����� */
struct ECALC_TOKEN *ecalc_make_tree( struct ECALC_TOKEN *token );                       /* �g�[�N�����c���[�ɕϊ� */
double ecalc_get_tree_value( struct ECALC_TOKEN *token, double **vars, double ans );    /* �c���[����l�𓾂� */
void ecalc_free_token( struct ECALC_TOKEN *token );                                     /* �g�[�N���A�����X�g��� */

// static
struct ECALC_TOKEN *ecalc_malloc_token( void );                                         /* �g�[�N�������� */
void ecalc_make_exp( struct ECALC_TOKEN *left );                                        /* ���ӂƉE�ӂ����� */
int ecalc_token_is_value( struct ECALC_TOKEN *token );                                  /* �^����ꂽ�g�[�N�����l�������̂��`�F�b�N */
int ecalc_strncmp( char *str1, char *str2, int n );                                     /* �啶������������ʂ��Ȃ�strncmp */
void ( *ecalc_get_func_addr( enum ecalc_connector func ) )( void );						/* �֐��̃|�C���^���擾���� */
// ��̐錾��
// ecalc... is function( enum... ) returning pointer to function( void ) retruning void
// �ł�

#ifdef __cplusplus
}
#endif

#endif


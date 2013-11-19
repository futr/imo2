#ifndef __ECONF_H__
#define __ECONF_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define ECONF_MAX_FILENAME	1024
#define ECONF_MAX_STR_LEN	1024

enum econf_line_type {
	ECONF_LINE_TOP,
	ECONF_LINE_LINE
};

enum econf_token_type {
	ECONF_TOKEN_EQU,
	ECONF_TOKEN_STR,
	ECONF_TOKEN_ERR
};

struct ECONF_TOKEN {
	int type;
	char *str;
	
	struct ECONF_TOKEN *next;
};

struct ECONF_LINE {
	char *left;
	char *right;
	
	int type;
	
	struct ECONF_LINE *next;
};

struct ECONF_FILE_CONTAINER {
	char filename[ECONF_MAX_FILENAME];
	FILE *fp;
	
	int count;
	struct ECONF_LINE **lines;
};

#ifdef __cplusplus
extern "C" {
#endif

int econf_open( struct ECONF_FILE_CONTAINER *ef, char *filename );						/* �t�@�C���J�� */
int econf_recog( struct ECONF_FILE_CONTAINER *ef );										/* �F�� */
void econf_free_line( struct ECONF_LINE *line );										/* ���J�� */
void econf_free_token( struct ECONF_TOKEN *tok );
struct ECONF_TOKEN *econf_recog_line( char *line );
struct ECONF_LINE *econf_search( struct ECONF_FILE_CONTAINER *ef, char *key );			/* ���� */
int econf_close( struct ECONF_FILE_CONTAINER *ef );										/* ���� */

double econf_as_double( struct ECONF_LINE *line );
char *econf_as_str( struct ECONF_LINE *line );

#ifdef __cplusplus
}
#endif

#endif

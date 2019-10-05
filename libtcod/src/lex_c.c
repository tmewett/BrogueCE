/*
* libtcod 1.5.2
* Copyright (c) 2008,2009,2010,2012 Jice & Mingos
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Jice or Mingos may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY JICE AND MINGOS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL JICE OR MINGOS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "libtcod.h"

#define MAX_JAVADOC_COMMENT_SIZE 16384

/* damn ANSI C does not know strdup, strcasecmp, strncasecmp */
char *TCOD_strdup(const char *s) {
	uint32 l=strlen(s)+1;
	char *ret=malloc(sizeof(char)*l);
	memcpy(ret,s,sizeof(char)*l);
	return ret;
}

int TCOD_strcasecmp(const char *s1, const char *s2) {
	unsigned char c1,c2;
	do {
		c1 = *s1++;
		c2 = *s2++;
		c1 = (unsigned char) tolower( (unsigned char) c1);
		c2 = (unsigned char) tolower( (unsigned char) c2);
	} while((c1 == c2) && (c1 != '\0'));
	return (int) c1-c2;
} 

int TCOD_strncasecmp(const char *s1, const char *s2, size_t n) {
	unsigned char c1,c2;
	do {
		c1 = *s1++;
		c2 = *s2++;
		c1 = (unsigned char) tolower( (unsigned char) c1);
		c2 = (unsigned char) tolower( (unsigned char) c2);
		n--;
	} while((c1 == c2) && (c1 != '\0') && n > 0);
	return (int) c1-c2;
} 

static const char * TCOD_LEX_names[] = {
  "unknown token",
  "symbol",
  "keyword",
  "identifier",
  "string",
  "integer",
  "float",
  "char",
  "eof"
};

static char *TCOD_last_error=NULL;

const char *TCOD_lex_get_token_name(int token_type) { return TCOD_LEX_names[token_type]; }

static void allocate_tok(TCOD_lex_t *lex, int len) {
	if ( lex->toklen > len ) return;
	while ( lex->toklen <= len ) lex->toklen *= 2;
	lex->tok = (char *)realloc(lex->tok,lex->toklen);
}


char *TCOD_lex_get_last_error() {
	return TCOD_last_error;
}

TCOD_lex_t *TCOD_lex_new_intern() {
	return (TCOD_lex_t *)calloc(1,sizeof(TCOD_lex_t));
}

TCOD_lex_t * TCOD_lex_new( const char **_symbols, const char **_keywords, const char *simpleComment,
		const char *commentStart, const char *commentStop, const char *javadocCommentStart, 
		const char *_stringDelim, int _flags)
{
	TCOD_lex_t *lex=(TCOD_lex_t *)TCOD_lex_new_intern();
    lex->flags = _flags;
	lex->last_javadoc_comment = (char *)calloc(sizeof(char),MAX_JAVADOC_COMMENT_SIZE );
    if ( _symbols )
    {
		while ( _symbols[ lex->nb_symbols ] )
		{
			if ( strlen( _symbols[ lex->nb_symbols ] ) >= TCOD_LEX_SYMBOL_SIZE )
			{
				static char msg[255];
				sprintf (msg, "symbol '%s' too long (max size %d)",
				       _symbols[ lex->nb_symbols ], TCOD_LEX_SYMBOL_SIZE );
				TCOD_last_error=TCOD_strdup(msg);
				return NULL;
			}
			strcpy(lex->symbols[ lex->nb_symbols ], _symbols[ lex->nb_symbols ] );
			lex->nb_symbols++;
		}
    }
    if ( _keywords )
    {
		while ( _keywords[ lex->nb_keywords ] )
		{
			if ( strlen( _keywords[ lex->nb_keywords ] ) >= TCOD_LEX_KEYWORD_SIZE )
			{
				static char msg[255];
				sprintf(msg,"keyword '%s' too long (max size %d)",
						   _keywords[ lex->nb_keywords ], TCOD_LEX_KEYWORD_SIZE);
				TCOD_last_error=TCOD_strdup(msg);
				return NULL;
			}
			if ( lex->flags & TCOD_LEX_FLAG_NOCASE )
			{
				char *ptr = (char *)_keywords[ lex->nb_keywords ];
				while ( *ptr )
				{
					*ptr = (char)toupper( *ptr);
					ptr++;
				}
			}
			strcpy(lex->keywords[ lex->nb_keywords ], _keywords[ lex->nb_keywords ] );
			lex->nb_keywords++;
		}
	}
	lex->simpleCmt = simpleComment;
	lex->cmtStart = commentStart;
	lex->cmtStop = commentStop;
	lex->javadocCmtStart = javadocCommentStart;
	lex->stringDelim = _stringDelim;
	lex->lastStringDelim='\0';
	lex->tok = (char *)calloc(sizeof(char),256);
	lex->toklen=256;
	return (TCOD_lex_t *)lex;
}

char *TCOD_lex_get_last_javadoc(TCOD_lex_t *lex)
{
	if ( ! lex->javadoc_read && lex->last_javadoc_comment[0] != '\0' )
	{
		lex->javadoc_read=true;
		return lex->last_javadoc_comment;
	}
	lex->javadoc_read=false;
	lex->last_javadoc_comment[0]='\0';
	return NULL;
}

void TCOD_lex_delete(TCOD_lex_t *lex)
{
	if ( ! lex->savept )
	{
		if ( lex->filename ) free( lex->filename );
		if ( lex->buf && lex->allocBuf ) free(lex->buf);
		if ( lex->last_javadoc_comment ) free(lex->last_javadoc_comment);
	}
	lex->filename=NULL;
	lex->buf = NULL;
	lex->allocBuf=false;
	if ( lex->tok ) free(lex->tok);
	free(lex);
}

void TCOD_lex_set_data_buffer_internal(TCOD_lex_t *lex)
{
    lex->file_line = 1;
    lex->pos = lex->buf;
    lex->token_type = TCOD_LEX_UNKNOWN;
    lex->token_int_val = 0;
    lex->token_float_val = 0.0;
    lex->token_idx = -1;
    lex->tok[0] = '\0';
}


void TCOD_lex_set_data_buffer(TCOD_lex_t *lex,char *dat)
{
	lex->buf = dat;
	lex->allocBuf = false;
	TCOD_lex_set_data_buffer_internal(lex);
}

bool TCOD_lex_set_data_file(TCOD_lex_t *lex, const char *_filename)
{
    FILE *f;
    char *ptr;
    long size;
    if ( ! _filename ) {
    	TCOD_last_error = (char *)"Lex.setDatafile(NULL) called";
    	return false;
    }
    f = fopen( _filename, "rb" );
    if ( f == NULL )
    {
		static char msg[255];
		sprintf(msg, "Cannot open '%s'", _filename);
		TCOD_last_error=TCOD_strdup(msg);
		return false;
    }
	fseek(f, 0, SEEK_END);
   	size = ftell(f);
   	fclose(f);
    f = fopen( _filename, "r" );
    
    lex->buf = (char*)calloc(sizeof(char),(size + 1));
    lex->filename = TCOD_strdup( _filename );
    if ( lex->buf == NULL || lex->filename == NULL )
    {
		fclose(f);
		if ( lex->buf ) free(lex->buf);
		if ( lex->filename ) {
			free( lex->filename );
		}
		TCOD_last_error=(char *)"Out of memory";
		return false;
    }
	ptr=lex->buf;
	/* can't rely on size to read because of MS/DOS dumb CR/LF handling */
	while ( fgets(ptr, size,f ) )
	{
		ptr += strlen(ptr);
	}
    fclose(f);
	TCOD_lex_set_data_buffer_internal(lex);
	lex->allocBuf=true;
	return true;
}

void TCOD_lex_get_new_line(TCOD_lex_t *lex)
{
	if ( *(lex->pos) == '\n' )
    {
		lex->file_line ++;
		lex->pos++;
    }
}

#ifdef TCOD_VISUAL_STUDIO
#pragma warning(disable:4127) /* conditional expression is constant */
#endif

int TCOD_lex_get_space(TCOD_lex_t *lex)
{
	char c;
	char *startPos=NULL;
	while ( 1 )
	{
		while ( (c = *lex->pos) <= ' ')
		{
			if (c=='\n')
				TCOD_lex_get_new_line(lex);
			else if (c == 0)
				return TCOD_LEX_EOF;		/* end of file */
			else lex->pos++;
		}
		if ( lex->simpleCmt && strncmp(lex->pos, lex->simpleCmt, strlen(lex->simpleCmt)) == 0 )
		{
			if ( ! startPos ) startPos = lex->pos;
			while ( *lex->pos != '\0' && *lex->pos != '\n' )
			lex->pos++;
			TCOD_lex_get_new_line(lex);
			continue;
		}
		if ( lex->cmtStart && lex->cmtStop && strncmp(lex->pos, lex->cmtStart, strlen(lex->cmtStart)) == 0 )
		{
			int isJavadoc=( lex->javadocCmtStart && strncmp(lex->pos, lex->javadocCmtStart, strlen(lex->javadocCmtStart)) == 0 );
			int cmtLevel=1;
			char *javadocStart = NULL;
			if ( ! startPos ) startPos = lex->pos;
			if ( isJavadoc )
			{
				javadocStart=lex->pos+strlen(lex->javadocCmtStart);
				while ( isspace(*javadocStart) ) javadocStart++;
			}
			lex->pos++;
			do
			{
				if ( *lex->pos == '\n' )
				{
					TCOD_lex_get_new_line(lex);
				}
				else
					lex->pos++;
				if ( *lex->pos == '\0' )
					return TCOD_LEX_EOF;
				if ( (lex->flags & TCOD_LEX_FLAG_NESTING_COMMENT) && strncmp(lex->pos-1, lex->cmtStart, strlen(lex->cmtStart)) == 0) cmtLevel++;
				if ( strncmp(lex->pos-1, lex->cmtStop, strlen(lex->cmtStop)) == 0) cmtLevel--;
			} while ( cmtLevel > 0 );
			lex->pos++;
			if ( isJavadoc )
			{
                char *src, *dst;
				char *end = lex->pos - strlen(lex->cmtStop);
				while ( isspace(*end) && end > javadocStart ) end --;
				src = javadocStart;
				dst = lex->last_javadoc_comment;
				while ( src < end )
				{
					/* skip heading spaces */
					while ( src < end && isspace(*src) && *src != '\n') src ++;
					/* copy comment line */
					while ( src < end && *src != '\n' ) *dst++ = *src++;
					if ( *src == '\n' ) *dst++ = *src++;
				}
				/* remove trailing spaces */
				while ( dst > lex->last_javadoc_comment && isspace (*(dst-1)) ) dst --;
				*dst = '\0';
				lex->javadoc_read=false;
			}
			continue;
		}
		break;
	}
	if ( (lex->flags & TCOD_LEX_FLAG_TOKENIZE_COMMENTS) && startPos && lex->pos > startPos ) {
		int len = lex->pos - startPos;
		allocate_tok(lex, len+1);
		strncpy(lex->tok,startPos,len);
		lex->tok[len]=0;
		lex->token_type = TCOD_LEX_COMMENT;
		lex->token_idx = -1;
		return TCOD_LEX_COMMENT;
	}
	return TCOD_LEX_UNKNOWN;
}



int TCOD_lex_hextoint(char c) {
	int v=toupper(c);
	if ( v >= '0' && v <= '9' ) return v-'0';
	return 10 + (v-'A');
}

static bool TCOD_lex_get_special_char(TCOD_lex_t *lex, char *c) {
	*c = *(++(lex->pos) );

	switch ( *c )
	{
		case 'n' : *c='\n'; break;
		case 't' : *c='\t'; break;
		case 'r' : *c='\r'; break;
		case '\\' : 
		case '\"' :
		case '\'' :
		break;
		case 'x' :
		{
			/* hexadecimal value "\x80" */
			int value=0;
			bool hasHex=false;
			*c = *(++(lex->pos) );
			while (( *c >= '0' && *c <= '9' ) || (*c >= 'a' && *c <= 'f') || (*c >= 'A' && *c <= 'F') ) {
				hasHex=true;
				value <<= 4;
				value += TCOD_lex_hextoint(*c);
				*c = *(++(lex->pos) );
			}
			if (! hasHex ) {
				TCOD_last_error=(char *)"\\x must be followed by an hexadecimal value";
				return false;
			}
			*c = value;
			lex->pos--;
		}
		break;
		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		{
			/* octal value "\200" */
			int value=0;
			while ( *c >= '0' && *c <= '7' ) {
				value <<= 3;
				value += (*c - '0');
				*c = *(++(lex->pos) );
			}
			*c = value;
			lex->pos--;
		}
		break;
		default :
			TCOD_last_error=(char *)"bad escape sequence inside quote";
			return false;
	}
	return true;
}

int TCOD_lex_get_string(TCOD_lex_t *lex)
{
    char c;
    int len = 0;
    do
    {
		c= *(++(lex->pos));
		if ( c == '\0' ) {
			TCOD_last_error=(char *)"EOF inside quote";
			return TCOD_LEX_ERROR;
		}
		if ( c == '\n' ) {
			TCOD_last_error=(char *)"newline inside quote";
			return TCOD_LEX_ERROR;
		}
		if ( c== '\\' )
		{
		    if ( ! TCOD_lex_get_special_char(lex,&c) ) return TCOD_LEX_ERROR;
		}
		else if ( c == lex->lastStringDelim )
		{
			allocate_tok(lex, len);
			lex->tok[ len ] = '\0';
			lex->token_type = TCOD_LEX_STRING;
			lex->token_idx = -1;
			lex->pos++;
			return TCOD_LEX_STRING;
		}
		allocate_tok(lex, len);
		lex->tok[ len++ ] = c;
    } while ( 1 );
}

#ifdef TCOD_VISUAL_STUDIO
#pragma warning(default:4127) /* conditional expression is constant */
#endif

int TCOD_lex_get_number(TCOD_lex_t *lex)
{
	int c;
    int len;
    char *ptr;
    int bhex = 0, bfloat = 0;

    len = 0;
    if ( *lex->pos == '-' )
    {
		allocate_tok(lex, len);
		lex->tok[ len ++ ] = '-';
		lex->pos++;
    }

    c = toupper(*lex->pos);

    if ( c == '0' && ( lex->pos[1] == 'x' || lex->pos[1]=='X') )
    {
		bhex = 1;
		allocate_tok(lex, len);
		lex->tok[ len ++ ] = '0';
		lex->pos++;
		c = toupper( * (lex->pos));
    }
    do
    {
		allocate_tok(lex, len);
		lex->tok[ len++ ] = (char)c;
		lex->pos++;
		if ( c == '.' )
		{
			if ( bhex ) {
				TCOD_last_error=(char *)"bad constant format";
				return TCOD_LEX_ERROR;
			}
			bfloat = 1;
		}
		c = toupper(*lex->pos);
    } while ((c >= '0' && c<= '9')
	     || ( bhex && c >= 'A' && c <= 'F' )
	     || c == '.' );
	allocate_tok(lex, len);
    lex->tok[len] = 0;

    if ( !bfloat )
    {
		lex->token_int_val = strtol( lex->tok, &ptr, 0 );
		lex->token_float_val = (float)lex->token_int_val;
		lex->token_type = TCOD_LEX_INTEGER;
		lex->token_idx = -1;
		return TCOD_LEX_INTEGER;
    }
    else
    {
		lex->token_float_val = (float)atof( lex->tok );
		lex->token_type = TCOD_LEX_FLOAT;
		lex->token_idx = -1;
		return TCOD_LEX_FLOAT;
    }
}

int TCOD_lex_get_char(TCOD_lex_t *lex)
{
    char c;
    c= *(++(lex->pos));

    if ( c == '\0' ) {
		TCOD_last_error=(char *)"EOF inside simple quote";
		return TCOD_LEX_ERROR;
    }
    if ( c == '\n' ) {
		TCOD_last_error=(char *)"newline inside simple quote";
		return TCOD_LEX_ERROR;
    }
    if ( c== '\\' )
    {
	if ( ! TCOD_lex_get_special_char(lex,&c) ) return TCOD_LEX_ERROR;
	lex->pos++;
    }
    else
		lex->pos++;

    if ( *lex->pos != '\'' ) {
		TCOD_last_error= (char *)"bad character inside simple quote" ;
		return TCOD_LEX_ERROR;
    }
    lex->pos ++;
    lex->tok[ 0 ] = c;
    lex->tok[ 1 ] = '\0';
    lex->token_type = TCOD_LEX_CHAR;
    lex->token_int_val = (int)c;
    lex->token_idx = -1;
    return TCOD_LEX_CHAR;
}

int TCOD_lex_get_symbol(TCOD_lex_t *lex)
{
    int symb = 0;
	static char msg[255];

    while ( symb < lex->nb_symbols )
    {
		if ( ( ( lex->flags & TCOD_LEX_FLAG_NOCASE )
			&& TCOD_strncasecmp( lex->symbols[ symb ], lex->pos, strlen( lex->symbols[ symb ] ) ) == 0 )
			|| ( strncmp( lex->symbols[ symb ], lex->pos, strlen( lex->symbols[ symb ] ) ) == 0 ) )
		{
			strcpy( lex->tok, lex->symbols[ symb ] );
			lex->pos += strlen( lex->symbols[ symb ] );
			lex->token_idx = symb;
			lex->token_type = TCOD_LEX_SYMBOL;
			return TCOD_LEX_SYMBOL;
		}
		symb ++;
    }

    lex->pos++;
	sprintf(msg, "unknown symbol %.10s", lex->pos-1 );
	TCOD_last_error=TCOD_strdup(msg);
    return TCOD_LEX_ERROR;
}

int TCOD_lex_get_iden(TCOD_lex_t *lex)
{
    char c = *lex->pos;
    int len =  0, key = 0;

    do
    {
		allocate_tok(lex, len);
		lex->tok[ len++ ] = c;
		c = *( ++ (lex->pos)  );
    } while ( ( c >= 'a' && c <= 'z' )
	      || ( c >= 'A' && c <= 'Z' )
	      || ( c >= '0' && c <= '9' )
	      || c == '_' );
	allocate_tok(lex, len);
    lex->tok[len ] = 0;

    while ( key < lex->nb_keywords )
    {
		if ( strcmp( lex->tok, lex->keywords[ key ] ) == 0
		|| ( lex->flags & TCOD_LEX_FLAG_NOCASE && TCOD_strcasecmp( lex->tok, lex->keywords[ key ] ) == 0 ))
		{
			lex->token_type = TCOD_LEX_KEYWORD;
			lex->token_idx = key;
			return TCOD_LEX_KEYWORD;
		}
		key ++;
    }

    lex->token_type = TCOD_LEX_IDEN;
    lex->token_idx = -1;
    return TCOD_LEX_IDEN;
}

int TCOD_lex_parse(TCOD_lex_t *lex)
{
	char *ptr;
	int token;

	token = TCOD_lex_get_space(lex);
	if ( token == TCOD_LEX_ERROR ) return token;
	ptr = lex->pos;
	if ( token !=  TCOD_LEX_UNKNOWN )
	{
		lex->token_type = token;
		return token;
	}
	if ( strchr(lex->stringDelim, *ptr) )
	{
		lex->lastStringDelim=*ptr;
		return TCOD_lex_get_string(lex);
	}
	if ( *ptr == '\'' )
	{
		return TCOD_lex_get_char(lex);
	}
	if ( isdigit( (int)(*ptr) )
		|| ( *ptr == '-' && isdigit( (int)(ptr[1]) ) )
	)
	{
		return TCOD_lex_get_number(lex);
	}
	if ( ( *ptr >= 'a' && *ptr <= 'z' ) || ( *ptr >= 'A' && *ptr <= 'Z' )
		|| *ptr == '_' )
	{
		return TCOD_lex_get_iden(lex);
	}
	return TCOD_lex_get_symbol(lex);
}


int TCOD_lex_parse_until_token_type(TCOD_lex_t *lex,int tokenType)
{
	int token;
    token = TCOD_lex_parse(lex);
    if ( token == TCOD_LEX_ERROR ) return token;
    while ( token != TCOD_LEX_EOF )
    {
        if ( token == tokenType )
            return token;
	    token = TCOD_lex_parse(lex);
	    if ( token == TCOD_LEX_ERROR ) return token;
    }
    return token;
}

int TCOD_lex_parse_until_token_value(TCOD_lex_t *lex, const char *tokenValue)
{
	int token;
    token = TCOD_lex_parse(lex);
    if ( token == TCOD_LEX_ERROR ) return token;
    {
    while ( token != TCOD_LEX_EOF )
        if ( strcmp( lex->tok, tokenValue ) == 0
			|| ( ( lex->flags & TCOD_LEX_FLAG_NOCASE ) && TCOD_strcasecmp(lex->tok, tokenValue ) == 0 ) )
            return token;
	    token = TCOD_lex_parse(lex);
	    if ( token == TCOD_LEX_ERROR ) return token;
    }
    return token;
}

void TCOD_lex_savepoint(TCOD_lex_t *lex,TCOD_lex_t *_savept)
{
	TCOD_lex_t *savept=(TCOD_lex_t *)_savept;
	*savept = *lex;
	savept->tok = (char *)calloc(sizeof(char),lex->toklen);
	strcpy(savept->tok,lex->tok);
	savept->savept=true;
}

void TCOD_lex_restore(TCOD_lex_t *lex,TCOD_lex_t *_savept)
{
	TCOD_lex_t *savept=(TCOD_lex_t *)_savept;
	*lex = *savept;
	lex->savept=false;
}

bool TCOD_lex_expect_token_type(TCOD_lex_t *lex,int token_type)
{
	return (TCOD_lex_parse(lex) == token_type);
}

bool TCOD_lex_expect_token_value(TCOD_lex_t *lex,int token_type, const char *token_value)
{
	TCOD_lex_parse(lex);
	return (token_type == lex->token_type && strcmp(lex->tok, token_value) == 0 );
}


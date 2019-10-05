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

#ifndef _TCOD_LEX_H
#define _TCOD_LEX_H
/*
 * This is a libtcod internal module.
 * Use at your own risks...
 */

#define TCOD_LEX_FLAG_NOCASE 1
#define TCOD_LEX_FLAG_NESTING_COMMENT 2
#define TCOD_LEX_FLAG_TOKENIZE_COMMENTS 4

#define TCOD_LEX_ERROR -1
#define TCOD_LEX_UNKNOWN 0
#define TCOD_LEX_SYMBOL 1
#define TCOD_LEX_KEYWORD 2
#define TCOD_LEX_IDEN 3
#define TCOD_LEX_STRING 4
#define TCOD_LEX_INTEGER 5
#define TCOD_LEX_FLOAT 6
#define TCOD_LEX_CHAR 7
#define TCOD_LEX_EOF 8
#define TCOD_LEX_COMMENT 9

#define TCOD_LEX_MAX_SYMBOLS 100
#define TCOD_LEX_SYMBOL_SIZE 5
#define TCOD_LEX_MAX_KEYWORDS 100
#define TCOD_LEX_KEYWORD_SIZE 20

typedef struct {
	int file_line, token_type, token_int_val, token_idx;
	float token_float_val;
	char *tok;
    int toklen;
	char lastStringDelim;
	char *pos;
	char *buf;
	char *filename;
	char *last_javadoc_comment;
	/* private stuff */
	int nb_symbols, nb_keywords, flags;
	char symbols[ TCOD_LEX_MAX_SYMBOLS][ TCOD_LEX_SYMBOL_SIZE ], 
	keywords[ TCOD_LEX_MAX_KEYWORDS ][ TCOD_LEX_KEYWORD_SIZE ];
	const char *simpleCmt;
	const char *cmtStart, *cmtStop, *javadocCmtStart;
	const char *stringDelim;
	bool javadoc_read;
	bool allocBuf;
	bool savept; /* is this object a savepoint (no free in destructor) */	
} TCOD_lex_t;

TCODLIB_API TCOD_lex_t *TCOD_lex_new_intern();
TCODLIB_API TCOD_lex_t *TCOD_lex_new(const char **symbols, const char **keywords, const char *simpleComment, 
		const char *commentStart, const char *commentStop, const char *javadocCommentStart, const char *stringDelim, int flags);
TCODLIB_API void TCOD_lex_delete(TCOD_lex_t *lex);

TCODLIB_API void TCOD_lex_set_data_buffer(TCOD_lex_t *lex,char *dat);
TCODLIB_API bool TCOD_lex_set_data_file(TCOD_lex_t *lex,const char *filename);

TCODLIB_API int TCOD_lex_parse(TCOD_lex_t *lex);
TCODLIB_API int TCOD_lex_parse_until_token_type(TCOD_lex_t *lex,int token_type);
TCODLIB_API int TCOD_lex_parse_until_token_value(TCOD_lex_t *lex,const char *token_value);

TCODLIB_API bool TCOD_lex_expect_token_type(TCOD_lex_t *lex,int token_type);
TCODLIB_API bool TCOD_lex_expect_token_value(TCOD_lex_t *lex,int token_type,const char *token_value);

TCODLIB_API void TCOD_lex_savepoint(TCOD_lex_t *lex,TCOD_lex_t *savept);
TCODLIB_API void TCOD_lex_restore(TCOD_lex_t *lex,TCOD_lex_t *savept);
TCODLIB_API char *TCOD_lex_get_last_javadoc(TCOD_lex_t *lex);
TCODLIB_API const char *TCOD_lex_get_token_name(int token_type);
TCODLIB_API char *TCOD_lex_get_last_error();

TCODLIB_API int TCOD_lex_hextoint(char c);

#endif

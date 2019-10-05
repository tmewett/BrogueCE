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

#include <stdlib.h>
#include <string.h>
#include "libtcod.h"
#include "libtcod_int.h"

#define MAX_INT 0x7FFFFFFF

typedef struct {
    int x, y; /* coordinates on parent console */
    int w, h; /* textfield display size */
    int max; /* maximum nb of characters */
    int interval; /* cursor blinking interval */
    int halfinterval; /* half of the above */
    int ascii_cursor; /* cursor char. 0 if none */
	int cursor_pos, sel_start,sel_end; /* cursor position in text, selection range */
	int tab_size; /* tab size, if 0, no tab */
    char * prompt; /* prompt to be displayed before the string */
	int textx,texty; /* coordinate of start of text (after prompt) */
    TCOD_console_t con; /* offscreen console that will contain the textfield */
    bool input_continue; /* controls whether ENTER has been pressed */
    int len; /* allocated size of the text */
    int curlen; /* current length of the text */
    TCOD_color_t back; /* background colour */
    TCOD_color_t fore; /* foreground colour */
    float transparency; /* background transparency */
	bool multiline; /* multiline support ? */
    char * text; /* the text itself */
} text_t;

/* ctor */
TCOD_text_t TCOD_text_init (int x, int y, int w, int h, int max_chars) {
    text_t * data = (text_t*)calloc(sizeof(text_t),1);
	TCOD_IFNOT(w> 0 && h > 0) return data;
    data->x = x;
    data->y = y;
    data->w = w;
    data->h = h;
    data->multiline = (h > 1);
    data->max = (max_chars > 0 ? max_chars + 1 : MAX_INT);
    data->interval = 800;
    data->halfinterval = 400;
    data->ascii_cursor = 0;
    data->prompt = NULL;
    data->textx = data->texty = 0;
    data->con = TCOD_console_new(w,h);
	data->sel_start = MAX_INT;
	data->sel_end = -1;
	/*
	if (! data->multiline ) {
		data->max = MIN(w - data->textx,data->max);
	} else {
		data->max = MIN(w*(h-data->texty) - data->textx,data->max);
	}
	*/
    if (max_chars && max_chars > 0) data->max = max_chars;
    else data->max = data->w * data->h;
    data->input_continue = true;
    data->len = MIN(64,data->max);
    data->text = (char*)calloc(data->len,sizeof(char));
    data->back.r = data->back.g = data->back.b = 0;
    data->fore.r = data->fore.g = data->fore.b = 255;
    data->transparency = 1.0f;
    return (TCOD_text_t)data;
}

TCOD_text_t TCOD_text_init2(int w, int h, int max_chars) {
    text_t * data = (text_t*)calloc(sizeof(text_t),1);
	TCOD_IFNOT(w> 0 && h > 0) return data;
    data->x = 0;
    data->y = 0;
    data->w = w;
    data->h = h;
    data->multiline = (h > 1);
    data->max = (max_chars > 0 ? max_chars + 1 : MAX_INT);
    data->interval = 800;
    data->halfinterval = 400;
    data->ascii_cursor = 0;
    data->prompt = NULL;
    data->textx = data->texty = 0;
    data->con = TCOD_console_new(w,h);
	data->sel_start = MAX_INT;
	data->sel_end = -1;
	/*
	if (! data->multiline ) {
		data->max = MIN(w - data->textx,data->max);
	} else {
		data->max = MIN(w*(h-data->texty) - data->textx,data->max);
	}
	*/
    if (max_chars && max_chars > 0) data->max = max_chars;
    else data->max = data->w * data->h;
    data->input_continue = true;
    data->len = MIN(64,data->max);
    data->text = (char*)calloc(data->len,sizeof(char));
    data->back.r = data->back.g = data->back.b = 0;
    data->fore.r = data->fore.g = data->fore.b = 255;
    data->transparency = 1.0f;
    return (TCOD_text_t)data;
}

void TCOD_text_set_pos(TCOD_text_t txt, int x, int y) {
    text_t * data = (text_t*)txt;
	data->x=x;
	data->y=y;	
}

/* set cursor and prompt */
void TCOD_text_set_properties (TCOD_text_t txt, int cursor_char, int blink_interval, const char * prompt, int tab_size) {
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return;
    data->interval = blink_interval;
    data->halfinterval = (blink_interval > 0 ? blink_interval / 2 : 0);
    data->ascii_cursor = cursor_char;
    if (data->prompt) free(data->prompt);
    data->prompt = prompt ? TCOD_strdup(prompt) : NULL;
    data->textx = data->texty = 0;
	data->tab_size=tab_size;
    if ( prompt ) {
		const char *ptr=prompt;
		while (*ptr) {
			data->textx++;
			if ( *ptr == '\n' || data->textx == data->w) {
				data->textx=0;data->texty++;
			}
			ptr++;
		}
	}
}

/* set colours */
void TCOD_text_set_colors (TCOD_text_t txt, TCOD_color_t fore, TCOD_color_t back, float back_transparency) {
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return;
    data->back = back;
    data->fore = fore;
    data->transparency = back_transparency;
}

/* increase the buffer size. internal function */
static void allocate(text_t *data) {
    char *tmp;
    data->len *= 2;
    tmp = (char*)calloc(data->len,sizeof(char));
    memcpy(tmp,data->text,data->len/2);
    free(data->text);
    data->text = tmp;
}

/* insert a character at cursor position. internal function */
static void insertChar(text_t *data, char c) {
	char *ptr, *end;
	if (data->cursor_pos + 1 == data->max) {
		/* max size reached. replace the last char. don't increase text size */
		*(data->text + data->cursor_pos -1) = c;
		return;
	}
	if (data->curlen + 1 == data->len ) allocate(data);
	ptr=data->text + data->cursor_pos;
	end=data->text + data->curlen;
	do {
		*(end+1) = *end;
		end--;
	} while ( end >= ptr );
	*ptr = c;
	data->curlen++;
	data->cursor_pos++;
}

/* delete character at cursor position */
static void deleteChar(text_t *data) {
	char *ptr;
	if ( data->cursor_pos == 0 ) return;
	ptr=data->text + data->cursor_pos-1;
	do {
		*ptr = *(ptr+1);
		ptr++;
	} while (*ptr);
	if ( data->cursor_pos > 0 ) {
		data->cursor_pos--;
		data->curlen--;
	}
}

/* convert current cursor_pos into console coordinates. internal function */
static void get_cursor_coords(text_t *data, int *cx, int *cy) {
	char *ptr;
	if (data->multiline) {
		int curcount=data->cursor_pos;
		ptr=data->text;
		*cx = data->textx;
		*cy = data->texty;
		while (curcount > 0 && *ptr) {
			if ( *ptr == '\n') {
				*cx=0;
				(*cy)++;
			} else {
				(*cx)++;
				if ( *cx == data->w ) {
					*cx=0;
					(*cy)++;
				}
			}
			ptr++;
			curcount--;
		}
	} else {
		*cx = data->textx + data->cursor_pos;
		*cy = data->texty;
	}
}

/* check if the text does not overflow the textfield */
/*
not working...
static bool check_last_pos(text_t *data) {
	int count = strlen(data->text);
	int cx=data->textx;
	int cy=data->texty;
	char *ptr=data->text;
	while ( count > 0 ) {
		if ( *ptr == '\n') {
			cx=0;
			cy++;
		} else {
			cx++;
			if ( cx == data->w ) {
				cx=0;
				cy++;
			}
		}
		ptr++;
		count--;
	}
	return ( cy < data->h );
}
*/

/* set cursor_pos from coordinates. internal function */
static void set_cursor_pos(text_t *data, int cx, int cy, bool clamp) {
	if ( data->multiline ) {
		int curx=data->textx,cury=data->texty;
		char *ptr=data->text;
		int newpos=0;
		if ( clamp ) {
			cy=MAX(data->texty,cy);
			if ( cy == data->texty) cx = MAX(data->textx,cx);
		}
		/* find the right line */
		while ( *ptr && cury < cy && cury < data->h ) {
			if (*ptr == '\n' || curx == data->w-1) {
				curx=0;cury++;
			} else curx++;
			ptr++;
			newpos++;
		}
		if ( cury >= data->h ) return;
		if ( cury == cy ) {
			/* check if cx can be reached */
			while ( *ptr && curx < cx && *ptr != '\n') {
				ptr++;
				curx++;
				newpos++;
			}
		}
		data->cursor_pos = newpos;
	} else {
		int newpos = cx - data->textx + (cy - data->texty)*data->w;
		if ( clamp ) newpos = CLAMP(0,data->curlen,newpos);
		if ( newpos >= 0 && newpos <= data->curlen ) data->cursor_pos = newpos;
	}
}


/* decreases the selection range start */
static void selectStart(text_t *data, int oldpos, TCOD_key_t key) {
	if ( data->multiline && data->cursor_pos != oldpos ) {
		if ( key.shift ) {
			if ( data->sel_start > data->cursor_pos ) data->sel_start = data->cursor_pos;
			else data->sel_end = data->cursor_pos;
		} else {
			data->sel_start=MAX_INT;
			data->sel_end=-1;
		}
	}
}

/* increases the selection range end */
static void selectEnd(text_t *data, int oldpos, TCOD_key_t key) {
	if ( data->multiline && data->cursor_pos != oldpos ) {
		if ( key.shift ) {
			if ( data->sel_end < data->cursor_pos ) data->sel_end = data->cursor_pos;
			else data->sel_start = data->cursor_pos;
		} else {
			data->sel_start=MAX_INT;
			data->sel_end=-1;
		}
	}
}

enum { TYPE_SYMBOL, TYPE_ALPHANUM, TYPE_SPACE };
static const char symbols[]="&~\"#'{([-|`_\\^@)]=+}*/!:;.,?<>";

/* check whether a character is a space */
/* this is needed because cctype isspace() returns rubbish for many diacritics */
static bool is_space (int ch) {
    bool ret;
    switch (ch) {
        case ' ': case '\n': case '\r': case '\t': ret = true; break;
        default: ret = false; break;
    }
    return ret;
}

static void typecheck (int * type, int ch) {
    if (strchr(symbols,ch)) *type = TYPE_SYMBOL;
    else if (is_space(ch)) *type = TYPE_SPACE;
    else *type = TYPE_ALPHANUM;
}

/* go one word left */
static void previous_word(text_t *data) {
	/* current character type */
	if ( data->cursor_pos > 0 ) {
		/* detect current char type (alphanum/space or symbol) */
		char *ptr=data->text + data->cursor_pos - 1;
		int curtype, prevtype;
		typecheck(&curtype,*ptr);
		/* go back until char type changes from alphanumeric to something else */
		do {
			data->cursor_pos--;
			ptr--;
			prevtype = curtype;
			typecheck(&curtype,*ptr);
		} while ( data->cursor_pos > 0 && !(curtype != TYPE_ALPHANUM && prevtype == TYPE_ALPHANUM));
	}
}

/* go one word right */
static void next_word(text_t *data) {
   /* current character type */
	if ( data->text[data->cursor_pos] ) {
		/* detect current char type (alphanum/space or symbol) */
		char *ptr=data->text + data->cursor_pos;
		int curtype, prevtype;
		typecheck(&curtype,*ptr);
		/* go forth until char type changes from non alphanumeric to alphanumeric */
		do {
			data->cursor_pos++;
			ptr++;
			prevtype = curtype;
			typecheck(&curtype,*ptr);
		} while ( *ptr && !(curtype == TYPE_ALPHANUM  && prevtype != TYPE_ALPHANUM));
	}
}

/* erase the selected text */
static void deleteSelection(text_t *data) {
	int count = data->sel_end-data->sel_start;
	data->cursor_pos = data->sel_start+1;
	while ( count > 0 ) {
		deleteChar(data);
		count--;
		data->cursor_pos++;
	}
	data->cursor_pos--;
	data->sel_start=MAX_INT;
	data->sel_end=-1;
}

/* copy selected text to clipboard */
static void copy(text_t *data) {
	if ( data->sel_end - data->sel_start > 0 ) {
		char *clipbuf = (char*)calloc(data->sel_end - data->sel_start+1,1);
		char *ptr=clipbuf;
		int i;
		for (i=data->sel_start; i != data->sel_end; i++) {
			*ptr++ = data->text[i];
		}
		TCOD_sys_clipboard_set(clipbuf);
		free(clipbuf);
	}
}

/* cut selected text to clipboard */
static void cut(text_t *data) {
	if ( data->sel_end - data->sel_start > 0 ) {
		char *clipbuf = (char*)calloc(data->sel_end - data->sel_start+1,1);
		char *ptr=clipbuf;
		int i;
		for (i=data->sel_start; i != data->sel_end; i++) {
			*ptr++ = data->text[i];
		}
		TCOD_sys_clipboard_set(clipbuf);
		free(clipbuf);
		deleteSelection(data);
	}
}

/* paste from clipboard */
static void paste(text_t *data) {
	char *clipbuf=TCOD_sys_clipboard_get();
	if ( clipbuf ) {
		if ( data->sel_start != MAX_INT ) {
			deleteSelection(data);
		}
		while (*clipbuf) {
			insertChar(data,*clipbuf++);
		}
	}
}

/* update returns false if enter has been pressed, true otherwise */
bool TCOD_text_update (TCOD_text_t txt, TCOD_key_t key) {
	int cx,cy,oldpos;
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return false;
	oldpos = data->cursor_pos;
	/* for real-time keyboard : only on key release */
    if ( key.pressed ) {
	    /* process keyboard input */
	    switch (key.vk) {
	        case TCODK_BACKSPACE: /* get rid of the last character */
				if ( data->sel_start != MAX_INT ) {
					deleteSelection(data);
				} else {
					deleteChar(data);
				}
	            break;
			case TCODK_DELETE:
				if ( key.shift ) {
					/* SHIFT-DELETE : cut to clipboard */
					cut(data);
				} else {
					if ( data->sel_start != MAX_INT ) {
						deleteSelection(data);
					} else if ( data->text[data->cursor_pos] ) {
						data->cursor_pos++;
						deleteChar(data);
					}
				}
				break;
			/* shift + arrow / home / end = selection */
			/* ctrl + arrow = word skipping. ctrl + shift + arrow = word selection */
			case TCODK_LEFT:
				if ( data->multiline && key.shift && data->sel_end == -1) {
					data->sel_end = data->cursor_pos;
				}
				if ( data->cursor_pos > 0 ) {
					if ( key.lctrl || key.rctrl ) {
						previous_word(data);
					} else data->cursor_pos--;
					selectStart(data,oldpos,key);
				}
				break;
			case TCODK_RIGHT:
				if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
					data->sel_start = data->cursor_pos;
				}
				if ( data->text[data->cursor_pos] ) {
					if ( key.lctrl || key.rctrl ) {
						next_word(data);
					} else data->cursor_pos++;
					selectEnd(data,oldpos,key);
				}
				break;
			case TCODK_UP :
				get_cursor_coords(data,&cx,&cy);
				if ( data->multiline && key.shift && data->sel_end == -1) {
					data->sel_end = data->cursor_pos;
				}
				set_cursor_pos(data,cx,cy-1,false);
				selectStart(data,oldpos,key);
				break;
			case TCODK_DOWN :
				get_cursor_coords(data,&cx,&cy);
				if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
					data->sel_start = data->cursor_pos;
				}
				set_cursor_pos(data,cx,cy+1,false);
				selectEnd(data,oldpos,key);
				break;
			case TCODK_HOME:
				get_cursor_coords(data,&cx,&cy);
				if ( data->multiline && key.shift && data->sel_end == -1) {
					data->sel_end = data->cursor_pos;
				}
				if ( key.lctrl || key.rctrl ) {
					set_cursor_pos(data,0,0,true);
				} else {
					set_cursor_pos(data,0,cy,true);
				}
				selectStart(data,oldpos,key);
				break;
			case TCODK_END:
				get_cursor_coords(data,&cx,&cy);
				if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
					data->sel_start = data->cursor_pos;
				}
				if ( key.lctrl || key.rctrl ) {
					set_cursor_pos(data,data->w,data->h,true);
				} else {
					set_cursor_pos(data,data->w-1,cy,true);
				}
				selectEnd(data,oldpos,key);
				break;
	        case TCODK_ENTER: /* validate input */
	        case TCODK_KPENTER:
				if ( data->sel_start != MAX_INT ) {
					deleteSelection(data);
				}
				if ( data->multiline ) {
					get_cursor_coords(data,&cx,&cy);
					if ( cy < data->h-1 ) insertChar(data,'\n');
				} else {
		            data->input_continue = false;
				}
	            break;
			case TCODK_TAB :
				if (data->tab_size ) {
					int count=data->tab_size;
					if ( data->sel_start != MAX_INT ) {
						deleteSelection(data);
					}
					while ( count > 0 ) {
						insertChar(data,' ');
						count--;
					}
				}
				break;
	        default: { /* append a new character */
				if ( (key.c == 'c' || key.c=='C' || key.vk == TCODK_INSERT) && (key.lctrl || key.rctrl) ) {
					/* CTRL-C or CTRL-INSERT : copy to clipboard */
					copy(data);
				} else if ( (key.c == 'x' || key.c=='X') && (key.lctrl || key.rctrl) ) {
					/* CTRL-X : cut to clipboard */
					cut(data);
				} else if ( ((key.c == 'v' || key.c=='V') && (key.lctrl || key.rctrl))
					|| ( key.vk == TCODK_INSERT && key.shift )
					 ) {
					/* CTRL-V or SHIFT-INSERT : paste from clipboard */
					paste(data);
				} else if (key.c > 31) {
					if ( data->sel_start != MAX_INT ) {
						deleteSelection(data);
					}
					insertChar(data,(char)(key.c));
	            }
	            break;
	        }
	    }
    }
    return data->input_continue;
}

/* renders the textfield */
void TCOD_text_render (TCOD_text_t txt, TCOD_console_t con) {
    text_t * data = (text_t*)txt;
    uint32 time;
	bool cursor_on;
	char back=0;
	int curx,cury,cursorx,cursory, curpos;
	char *ptr;
    TCOD_IFNOT(data && data->con ) return;
    time = TCOD_sys_elapsed_milli();
	cursor_on = (int)( time % data->interval ) > data->halfinterval;
    TCOD_console_set_default_background(data->con, data->back);
    TCOD_console_set_default_foreground(data->con, data->fore);
    TCOD_console_clear(data->con);

	/* compute cursor position */
	get_cursor_coords(data,&cursorx,&cursory);

	if ( cursor_on && data->ascii_cursor) {
		/* save the character under cursor position */
		back = data->text[data->cursor_pos];
		data->text[data->cursor_pos] = data->ascii_cursor;
	}
	/* render prompt */
    if (data->prompt) TCOD_console_print_rect_ex(data->con,0,0,data->w,data->h,TCOD_BKGND_SET,TCOD_LEFT,"%s",data->prompt);
	/* render text */
	curx=data->textx;
	cury=data->texty;
	ptr=data->text;
	curpos=0;
	while (*ptr) {
		if ( *ptr == '\n') {
			if ( (curx == 0 || curpos == 0 ) && curpos >= data->sel_start && curpos < data->sel_end ) {
				/* inverted colors for selected empty lines */
				TCOD_console_set_char_background(data->con, curx, cury, data->fore, TCOD_BKGND_SET);
				TCOD_console_set_char_foreground(data->con, curx, cury, data->back);
			}
			curx=0;
			cury++;
		} else {
			if ( curpos >= data->sel_start && curpos < data->sel_end ) {
				/* inverted colors for selection */
				TCOD_console_set_char_background(data->con, curx, cury, data->fore, TCOD_BKGND_SET);
				TCOD_console_set_char_foreground(data->con, curx, cury, data->back);
			}
			TCOD_console_set_char(data->con,curx,cury,*ptr);
			curx++;
			if ( curx == data->w ) {
				curx=0;
				cury++;
			}
		}
		ptr++;
		curpos++;
	}
	if ( cursor_on ) {
		if ( data->ascii_cursor) {
			/* restore the character under cursor */
			data->text[data->cursor_pos] = back;
		} else {
			/* invert colors at cursor position */
			TCOD_console_set_char_background(data->con,cursorx,cursory,data->fore,TCOD_BKGND_SET);
			TCOD_console_set_char_foreground(data->con,cursorx,cursory,data->back);
		}
	} else if (! cursor_on && ! data->ascii_cursor && data->multiline ) {
		/* normal colors for cursor ( might be inside selection ) */
		TCOD_console_set_char_background(data->con,cursorx,cursory,data->back,TCOD_BKGND_SET);
		TCOD_console_set_char_foreground(data->con,cursorx,cursory,data->fore);
	}
    TCOD_console_blit(data->con,0,0,data->w,data->h,con,data->x,data->y,1.0f,data->transparency);
}

/* returns the text currently stored in the textfield object */
const char * TCOD_text_get (TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return "";
    return data->text;
}

/* resets the text initial state */
void TCOD_text_reset (TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return;
    memset(data->text,'\0',data->len);
    data->curlen = 0;
    data->input_continue = true;
}

/* destructor */
void TCOD_text_delete (TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    TCOD_IFNOT(data && data->con ) return;
    if ( data->text ) free(data->text);
    if ( data->prompt ) free(data->prompt);
    TCOD_console_delete(data->con);
    free(data);
}

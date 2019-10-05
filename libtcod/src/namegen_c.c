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
/*
* Mingos' NameGen
* This file was written by Dominik "Mingos" Marczuk.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "libtcod.h"

/* ------------ *
 * the typedefs *
 * ------------ */

/* the struct containing a definition of an unprocessed syllable set */
typedef struct {
    char * name;
    char * vocals;
    char * consonants;
    char * pre;
    char * start;
    char * middle;
    char * end;
    char * post;
    char * illegal;
    char * rules;
} namegen_syllables_t;

/* and the generator struct */
typedef struct {
    /* the name that will be called */
    char * name;
    /* needs to use a random number generator */
    TCOD_random_t random;
    /* the lists with all the data */
    TCOD_list_t vocals;
    TCOD_list_t consonants;
    TCOD_list_t syllables_pre;
    TCOD_list_t syllables_start;
    TCOD_list_t syllables_middle;
    TCOD_list_t syllables_end;
    TCOD_list_t syllables_post;
    TCOD_list_t illegal_strings;
    TCOD_list_t rules;
} namegen_t;

/* ------------------- *
 * variables and stuff *
 * ------------------- */

/* the list containing the generators */
TCOD_list_t namegen_generators_list = NULL;

/* the file parser */
TCOD_parser_t namegen_parser;
/* parsed files list */
TCOD_list_t parsed_files = NULL;
/* the data that will be filled out */
namegen_syllables_t * parser_data = NULL;
namegen_t * parser_output = NULL;
/* this one's needed to correctly update the generators with RNG pointer */
TCOD_random_t namegen_random;

/* the string that will be pointed to upon generating a name */
char * namegen_name = NULL;
/* for keeping track of the size of output names */
size_t namegen_name_size;

/* ------------------------------------ *
 * stuff to operate on the syllable set *
 * ------------------------------------ */

/* initialise a syllable set */
namegen_syllables_t * namegen_syllables_new (void) {
    namegen_syllables_t * data = calloc(sizeof(namegen_syllables_t),1);
    return data;
}

/* free a syllables set */
void namegen_syllables_delete (namegen_syllables_t * data) {
    if (data->vocals) free(data->vocals);
    if (data->consonants) free(data->consonants);
    if (data->pre) free(data->pre);
    if (data->start) free(data->start);
    if (data->middle) free(data->middle);
    if (data->end) free(data->end);
    if (data->post) free(data->post);
    if (data->illegal) free(data->illegal);
    if (data->rules) free(data->rules);
    free(data->name);
    free(data);
}

/* ---------------------------------- *
 * stuff to operate on the generators *
 * ---------------------------------- */

/* create a new generator */
namegen_t * namegen_generator_new (void) {
    namegen_t * data = malloc(sizeof(namegen_t));
    data->name = NULL;
    /* assign the rng */
	data->random = TCOD_random_get_instance();
	/* create the lists */
    data->vocals = TCOD_list_new();
    data->consonants = TCOD_list_new();
    data->syllables_pre = TCOD_list_new();
    data->syllables_start = TCOD_list_new();
    data->syllables_middle = TCOD_list_new();
    data->syllables_end = TCOD_list_new();
    data->syllables_post = TCOD_list_new();
    data->illegal_strings = TCOD_list_new();
    data->rules = TCOD_list_new();
    return (TCOD_namegen_t)data;
}

/* check whether a given generator already exists */
bool namegen_generator_check (const char * name) {
    /* if the list is not created yet, create it */
    if (namegen_generators_list == NULL) {
        namegen_generators_list = TCOD_list_new();
        return false;
    }
    /* otherwise, scan it for the name */
    else {
        namegen_t ** it;
        for (it = (namegen_t**)TCOD_list_begin(namegen_generators_list); it < (namegen_t**)TCOD_list_end(namegen_generators_list); it++) {
            if (strcmp((*it)->name,name) == 0) return true;
        }
        return false;
    }
}

/* retrieve available generator names */
void namegen_get_sets_on_error (void) {
    namegen_t ** it;
    fprintf (stderr,"Registered syllable sets are:\n");
    for (it = (namegen_t**)TCOD_list_begin(namegen_generators_list); it < (namegen_t**)TCOD_list_end(namegen_generators_list); it++) {
        fprintf (stderr," * \"%s\"\n",(*it)->name);
    }
}

/* get the appropriate syllables set */
namegen_t * namegen_generator_get (const char * name) {
    if (namegen_generator_check(name) == true) {
        namegen_t ** it;
        for (it = (namegen_t**)TCOD_list_begin(namegen_generators_list); it != (namegen_t**)TCOD_list_end(namegen_generators_list); it++) {
            if (strcmp((*it)->name,name) == 0) return (*it);
        }
    }
    /* and if there's no such set... */
    else
        fprintf(stderr,"Generator \"%s\" could not be retrieved.\n",name);
    return NULL;
}



/* destroy a generator */
void namegen_generator_delete (namegen_t * generator) {
    namegen_t * data = generator;
    free(data->name);
    data->random = NULL;
    TCOD_list_clear_and_delete(data->vocals);
    TCOD_list_clear_and_delete(data->consonants);
    TCOD_list_clear_and_delete(data->syllables_pre);
    TCOD_list_clear_and_delete(data->syllables_start);
    TCOD_list_clear_and_delete(data->syllables_middle);
    TCOD_list_clear_and_delete(data->syllables_end);
    TCOD_list_clear_and_delete(data->syllables_post);
    TCOD_list_clear_and_delete(data->illegal_strings);
    TCOD_list_clear_and_delete(data->rules);
    free(data);
}

/* ------------------------------ *
 * Populating namegen_t with data *
 * ------------------------------ */

/* fill the pointed list with syllable data by extracting tokens */
void namegen_populate_list (char * source, TCOD_list_t list, bool wildcards) {
    size_t len = strlen(source);
    size_t i = 0;
    char * token = malloc(strlen(source)+1); /* tokens will typically be many and very short, but let's be cautious. What if the entire string is a single token?*/
    memset(token,'\0',strlen(source)+1);
    do {
        /* do the tokenising using an iterator immitation :) */
        char * it = source + i;
        /* append a normal character */
        if ((*it >= 'a' && *it <= 'z') ||  (*it >= 'A' && *it <= 'Z') || *it == '\'' || *it == '-')
            strncat(token,it,1);
        /* special character */
        else if (*it == '/') {
            if (wildcards == true) strncat(token,it++,2);
            else strncat(token,++it,1);
            i++;
        }
        /* underscore is converted to space */
        else if (*it == '_') {
            if (wildcards == true) strncat(token,it,1);
            else strcat(token," ");
        }
        /* add wildcards if they are allowed */
        else if (wildcards == true && (*it == '$' || *it == '%' || (*it >= '0' && *it <= '9')))
            strncat(token,it,1);
        /* all other characters are treated as separators and cause adding the current token to the list */
        else if (strlen(token) > 0) {
            TCOD_list_push(list,TCOD_strdup(token));
            memset(token,'\0',strlen(source)+1);
        }
    } while (++i <= len);
    free(token);
}

/* populate all lists of a namegen_t struct */
void namegen_populate (namegen_t * dst, namegen_syllables_t * src) {
    if (dst == NULL || src == NULL) {
        fprintf(stderr,"Couldn't populate the name generator with data.\n");
        exit(1);
    }
    if (src->vocals != NULL)        namegen_populate_list (src->vocals,dst->vocals,false);
    if (src->consonants != NULL)    namegen_populate_list (src->consonants,dst->consonants,false);
    if (src->pre != NULL)           namegen_populate_list (src->pre,dst->syllables_pre,false);
    if (src->start != NULL)         namegen_populate_list (src->start,dst->syllables_start,false);
    if (src->middle != NULL)        namegen_populate_list (src->middle,dst->syllables_middle,false);
    if (src->end != NULL)           namegen_populate_list (src->end,dst->syllables_end,false);
    if (src->post != NULL)          namegen_populate_list (src->post,dst->syllables_post,false);
    if (src->illegal != NULL)       namegen_populate_list (src->illegal,dst->illegal_strings,false);
    if (src->rules != NULL)         namegen_populate_list (src->rules,dst->rules,true);
    dst->name = TCOD_strdup(src->name);
}

/* -------------------- *
 * parser-related stuff *
 * -------------------- */

/* preparing the parser */
void namegen_parser_prepare (void) {
    static bool namegen_parser_ready = false;
    if (namegen_parser_ready == true) return;
    else {
        TCOD_parser_struct_t parser_name ;
        namegen_parser = TCOD_parser_new();
        parser_name = TCOD_parser_new_struct(namegen_parser, "name");
        TCOD_struct_add_property(parser_name, "phonemesVocals", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "phonemesConsonants", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "syllablesPre", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "syllablesStart", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "syllablesMiddle", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "syllablesEnd", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "syllablesPost", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "illegal", TCOD_TYPE_STRING, false);
        TCOD_struct_add_property(parser_name, "rules", TCOD_TYPE_STRING, true);
        namegen_parser_ready = true;
    }
}

/* parser listener */
bool namegen_parser_new_struct (TCOD_parser_struct_t str, const char *name) {
    parser_data = namegen_syllables_new();
    return true;
}

bool namegen_parser_flag (const char *name) {
    return true;
}

bool namegen_parser_property(const char *name, TCOD_value_type_t type, TCOD_value_t value) {
    if (strcmp(name,"syllablesStart") == 0)             parser_data->start = TCOD_strdup(value.s);
    else if (strcmp(name,"syllablesMiddle") == 0)       parser_data->middle = TCOD_strdup(value.s);
    else if (strcmp(name,"syllablesEnd") == 0)          parser_data->end = TCOD_strdup(value.s);
    else if (strcmp(name,"syllablesPre") == 0)          parser_data->pre = TCOD_strdup(value.s);
    else if (strcmp(name,"syllablesPost") == 0)         parser_data->post = TCOD_strdup(value.s);
    else if (strcmp(name,"phonemesVocals") == 0)        parser_data->vocals = TCOD_strdup(value.s);
    else if (strcmp(name,"phonemesConsonants") == 0)    parser_data->consonants = TCOD_strdup(value.s);
    else if (strcmp(name,"rules") == 0)                 parser_data->rules = TCOD_strdup(value.s);
    else if (strcmp(name,"illegal") == 0) { /* illegal strings are converted to lowercase */
        char * str ;
        int i;
        parser_data->illegal = TCOD_strdup(value.s);
        str = parser_data->illegal;
        for(i = 0; i < (int)strlen(str); i++) str[i] = (char)(tolower(str[i]));
    }
    else return false;
    return true;
}

bool namegen_parser_end_struct(TCOD_parser_struct_t str, const char *name) {
    /* if there's no syllable set by this name, add it to the list */
    if (namegen_generator_check(name) == false) {
        parser_data->name = TCOD_strdup(name);
        parser_output = namegen_generator_new();
        namegen_populate(parser_output,parser_data);
        parser_output->random = namegen_random;
        if (namegen_generators_list == NULL) namegen_generators_list = TCOD_list_new();
        TCOD_list_push(namegen_generators_list, (const void*)parser_output);
    }
    /* free the allocated memory to prevent a memory leak */
    namegen_syllables_delete(parser_data);
    return true;
}

void namegen_parser_error(const char *msg) {
    fprintf(stderr,"%s\n",msg);
    exit(1);
}

TCOD_parser_listener_t namegen_listener = {
    namegen_parser_new_struct,
    namegen_parser_flag,
    namegen_parser_property,
    namegen_parser_end_struct,
    namegen_parser_error
};

/* run the parser */
void namegen_parser_run (const char * filename) {
    char ** it;
    /* prepare the parser --- this will be executed only once */
    namegen_parser_prepare();
    if (parsed_files == NULL) parsed_files = TCOD_list_new();
    if (TCOD_list_size(parsed_files) > 0) {
        for (it = (char **)TCOD_list_begin(parsed_files); it != (char **)TCOD_list_end(parsed_files); it++)
            if (strcmp(*it,filename) == 0) return;
    }
    /* if the file hasn't been parsed yet, add its name to the list so that it's never parsed twice */
    TCOD_list_push(parsed_files,(const void *)TCOD_strdup(filename));
    /* run the parser */
    TCOD_parser_run(namegen_parser,filename,&namegen_listener);
}

/* --------------- *
 * rubbish pruning *
 * --------------- */

/* search for occurrences of triple characters (case-insensitive) */
bool namegen_word_has_triples (char * str) {
    char * it = str;
    char c = (char)(tolower(*it));
    int cnt = 1;
    bool has_triples = false;
    it++;
    while (*it != '\0') {
        if ((char)(tolower(*it)) == c) cnt++;
        else {
            cnt = 1;
            c = (char)(tolower(*it));
        }
        if (cnt >= 3) has_triples = true;
        it++;
    }
    return has_triples;
}

/* search for occurrences of illegal strings */
bool namegen_word_has_illegal (namegen_t * data, char * str) {
    /* convert word to lowercase */
    char * haystack = TCOD_strdup(str);
    int i;
    for(i = 0; i < (int)strlen(haystack); i++) haystack[i] = (char)(tolower(haystack[i]));
    /* look for illegal strings */
    if (TCOD_list_size(data->illegal_strings) > 0) {
        char ** it;
        for (it = (char**)TCOD_list_begin(data->illegal_strings); it != (char**)TCOD_list_end(data->illegal_strings); it++) {
            if (strstr(haystack,*it) != NULL) {
                free(haystack);
                return true;
            }
        }
    }
    free(haystack);
    return false;
}

/* removes double spaces, as well as leading and ending spaces */
void namegen_word_prune_spaces (char * str) {
    char * s;
    char * data = str;
    /* remove leading spaces */
    while (data[0] == ' ') memmove (data, data+1, strlen(data));
    /* reduce double spaces to single spaces */
    while ((s = strstr(data,"  ")) != NULL) memmove (s, s+1, strlen(s));
    /* remove the final space */
    while (data[strlen(data)-1] == ' ') data[strlen(data)-1] = '\0';
}

/* prune repeated "syllables", such as Arnarn */
bool namegen_word_prune_syllables (char *str) {
    char * data = TCOD_strdup(str);
    int len = strlen(data); /* length of the string */
    char check[8];
    int i; /* iteration in for loops */
    /* change to lowercase */
    for (i = 0; i < len; i++) data[i] = (char)(tolower(data[i]));
    /* start pruning */
    /* 2-character direct repetitions */
    for (i = 0; i < len - 4; i++) {
        memset(check,'\0',8);
        strncpy(check,data+i,2);
        strncat(check,data+i,2);
        if (strstr(data,check) != NULL) {
                free(data);
                return true;
            }
    }
    /* 3-character repetitions (anywhere in the word) - prunes everything, even 10-char repetitions */
    for (i = 0; i < len - 6; i++) {
        memset(check,'\0',8);
        strncpy(check,data+i,3);
        if (strstr(data+i+3,check) != NULL) {
            free(data);
            return true;
        }
    }
    free(data);
    return false;
}

/* everything stacked together */
bool namegen_word_is_ok (namegen_t * data, char * str) {
    namegen_word_prune_spaces(str);
    return
        (strlen(str)>0) &
        (!namegen_word_has_triples(str)) &
        (!namegen_word_has_illegal(data,str)) &
        (!namegen_word_prune_syllables(str));
}

/* ---------------------------- *
 * publicly available functions *
 * ---------------------------- */

/* parse a new syllable sets file - allocates a new data structure and fills it with necessary content */
void TCOD_namegen_parse (const char * filename, TCOD_random_t random) {
    /* check for file existence */
    FILE * in = fopen(filename,"r");
    if (in == NULL) {
        fprintf(stderr,"File \"%s\" not found!\n",filename);
        return;
    }
    fclose(in);
    /* set namegen RNG */
    namegen_random = random;
    /* run the proper parser - add the file's contents to the data structures */
    namegen_parser_run(filename);
}

/* generate a name using a given generation rule */
char * TCOD_namegen_generate_custom (char * name, char * rule, bool allocate) {
    namegen_t * data;
    size_t buflen = 1024;
    char * buf ;
    size_t rule_len ;
    if (namegen_generator_check(name)) data = namegen_generator_get(name);
    else {
        fprintf(stderr,"The name \"%s\" has not been found.\n",name);
        namegen_get_sets_on_error();
        return NULL;
    }
    buf = malloc(buflen);
    rule_len = strlen(rule);
    /* let the show begin! */
    do {
        char * it = rule;
        memset(buf,'\0',buflen);
        while (it <= rule + rule_len) {
            /* make sure the buffer is large enough */
            if (strlen(buf) >= buflen) {
                char * tmp ;
                while (strlen(buf) >= buflen) buflen *= 2;
                tmp = malloc(buflen);
                strcpy(tmp,buf);
                free(buf);
                buf = tmp;
            }
            /* append a normal character */
            if ((*it >= 'a' && *it <= 'z') ||  (*it >= 'A' && *it <= 'Z') || *it == '\'' || *it == '-')
                strncat(buf,it,1);
            /* special character */
            else if (*it == '/') {
                it++;
                strncat(buf,it,1);
            }
            /* underscore is converted to space */
            else if (*it == '_') strcat(buf," ");
            /* interpret a wildcard */
            else if (*it == '$') {
                int chance = 100;
                it++;
                /* food for the randomiser */
                if (*it >= '0' && *it <= '9') {
                    chance = 0;
                    while (*it >= '0' && *it <= '9') {
                        chance *= 10;
                        chance += (int)(*it) - (int)('0');
                        it++;
                    }
                }
                /* ok, so the chance of wildcard occurrence is calculated, now evaluate it */
                if (chance >= TCOD_random_get_int(data->random,0,100)) {
                    TCOD_list_t lst;
                    switch (*it) {
                        case 'P': lst = data->syllables_pre; break;
                        case 's': lst = data->syllables_start; break;
                        case 'm': lst = data->syllables_middle; break;
                        case 'e': lst = data->syllables_end; break;
                        case 'p': lst = data->syllables_post; break;
                        case 'v': lst = data->vocals; break;
                        case 'c': lst = data->consonants; break;
                        case '?': lst = (TCOD_random_get_int(data->random,0,1) == 0 ? data->vocals : data->consonants); break;
                        default:
                            fprintf(stderr,"Wrong rules syntax encountered!\n");
                            exit(1);
                            break;
                    }
                    if (TCOD_list_size(lst) == 0)
                        fprintf(stderr,"No data found in the requested string (wildcard *%c). Check your name generation rule %s.\n",*it,rule);
                    else
                        strcat(buf,(char*)TCOD_list_get(lst,TCOD_random_get_int(data->random,0,TCOD_list_size(lst)-1)));
                }
            }
            it++;
        }
    } while (!namegen_word_is_ok(data,buf));
    /* prune the spare spaces out */
    namegen_word_prune_spaces(buf);
    /* return the name accordingly */
    if (allocate == true) return buf;
    else {
        /* take care of ensuring the recipient is sized properly */
        if (namegen_name == NULL) {
            namegen_name_size = 64;
            namegen_name = malloc (namegen_name_size);
        }
        while (strlen(buf) > namegen_name_size - 1) {
            namegen_name_size *= 2;
            free(namegen_name);
            namegen_name = malloc(namegen_name_size);
        }
        strcpy(namegen_name,buf);
        free(buf);
        return namegen_name;
    }
}

/* generate a name with one of the rules from the file */
char * TCOD_namegen_generate (char * name, bool allocate) {
    namegen_t * data;
    int rule_number;
    int chance;
    char * rule_rolled;
    int truncation;
    char * rule_parsed ;
    char * ret ;
    if (namegen_generator_check(name)) data = namegen_generator_get(name);
    else {
        fprintf(stderr,"The name \"%s\" has not been found.\n",name);
        namegen_get_sets_on_error();
        return NULL;
    }
    /* check if the rules list is present */
    if (TCOD_list_size(data->rules) == 0) {
        fprintf(stderr,"The rules list is empty!\n");
        exit(1);
    }
    /* choose the rule */
    do {
        rule_number = TCOD_random_get_int(data->random,0,TCOD_list_size(data->rules)-1);
        rule_rolled = (char*)TCOD_list_get(data->rules,rule_number);
        chance = 100;
        truncation = 0;
        if (rule_rolled[0] == '%') {
            truncation = 1;
            chance = 0;
            while (rule_rolled[truncation] >= '0' && rule_rolled[truncation] <= '9') {
                chance *= 10;
                chance += (int)(rule_rolled[truncation]) - (int)('0');
                truncation++;
            }
        }
    } while (TCOD_random_get_int(data->random,0,100) > chance);
    /* OK, we've got ourselves a new rule! */
    rule_parsed = TCOD_strdup(rule_rolled+truncation);
    ret = TCOD_namegen_generate_custom(name,rule_parsed,allocate);
    free(rule_parsed);
    return ret;
}

/* retrieve the list of all available syllable set names */
TCOD_list_t TCOD_namegen_get_sets (void) {
    TCOD_list_t l = TCOD_list_new();
    if (namegen_generators_list != NULL) {
        namegen_t ** it;
        for (it = (namegen_t**)TCOD_list_begin(namegen_generators_list); it < (namegen_t**)TCOD_list_end(namegen_generators_list); it++) {
            TCOD_list_push(l,(const void*)((*it)->name));
        }
    }
    return l;
}

/* delete all the generators */
void TCOD_namegen_destroy (void) {
    /* delete all generators */
    namegen_t ** it;
    for (it = (namegen_t**)TCOD_list_begin(namegen_generators_list); it < (namegen_t**)TCOD_list_end(namegen_generators_list); it++)
        namegen_generator_delete(*it);
    /* clear the generators list */
    TCOD_list_clear(namegen_generators_list);
    /* get rid of the parsed files list */
    TCOD_list_clear_and_delete(parsed_files);
}


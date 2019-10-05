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

#ifndef _LIBTCOD_H
#define _LIBTCOD_H

/* uncomment to disable unicode support */
/*#define NO_UNICODE */

/* uncomment to disable opengl support */
/*#define NO_OPENGL */

/* os identification
   TCOD_WINDOWS : OS is windows
   TCOD_LINUX : OS is Linux
   TCOD_MACOSX : OS is Mac OS X
   TCOD_HAIKU : OS is Haiku */

/* compiler identification
   TCOD_VISUAL_STUDIO : compiler is Microsoft Visual Studio
   TCOD_MINGW32 : compiler is Mingw32
   TCOD_GCC : compiler is gcc/g++ */

/* word size
   TCOD_64BITS : 64 bits OS
   TCOD_WIN64 : 64 bits Windows
   TCOD_WIN32 : 32 bits Windows
   TCOD_LINUX64 : 64 bits Linux
   TCOD_LINUX32 : 32 bits Linux
   TCOD_FREEBSD64 : 64 bits FreeBSD
   TCOD_FREEBSD32 : 32 bits FreeBSD */

#if defined( _MSC_VER )
#  define TCOD_VISUAL_STUDIO
#  define TCOD_WINDOWS
#  ifdef _WIN64
#    define TCOD_WIN64
#    define TCOD_64BITS
#  else
#    define TCOD_WIN32
#  endif
#elif defined( __MINGW32__ )
#  define TCOD_WINDOWS
#  define TCOD_MINGW32
#  define TCOD_WIN32
#elif defined( __HAIKU__ )
#  define TCOD_HAIKU
#  define TCOD_GCC
#  if __WORDSIZE == 64
#    define TCOD_64BITS
#  endif
#elif defined( __linux )
#  define TCOD_LINUX
#  define TCOD_GCC
#  if __WORDSIZE == 64
#    define TCOD_LINUX64
#    define TCOD_64BITS
#  else
#    define TCOD_LINUX32
#  endif
#elif defined( __FreeBSD__ )
#  define TCOD_FREEBSD
#  define TCOD_GCC
#  if __WORDSIZE == 64
#    define TCOD_FREEBSD64
#    define TCOD_64BITS
#  else
#    define TCOD_FREEBSD32
#  endif
#elif defined (__APPLE__) && defined (__MACH__)
#  define TCOD_MACOSX
#  define TCOD_GCC
#endif

/* unicode rendering functions support */
#ifndef NO_UNICODE
#include <wchar.h>
#endif

/* This is a hack. SDL by default want you to rename your main statement, and insert it's own first
   It does that to handle some init code. However, libtcod handles that for you. If we did this
   wrappers like libtcod-net would be hosed, since there is no main statement there. */
#ifdef TCOD_MACOSX
#define _SDL_main_h
#include "SDL/SDL.h"
#endif

/* base types */
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;
/* int with the same size as a pointer (32 or 64 depending on OS) */
typedef long intptr;
typedef unsigned long uintptr;

#define TCOD_HEXVERSION 0x010502
#define TCOD_STRVERSION "1.5.2"
#define TCOD_TECHVERSION 0x01050200

/* bool support for C */
#ifndef __cplusplus
#ifndef bool
typedef uint8 bool;
#define false ((bool)0)
#define true ((bool)1)
#endif
#else
/* in C++ all C functions prototypes should use uint8 instead of bool */
#define bool uint8
#endif

/* DLL export */
#ifdef TCOD_WINDOWS
#ifdef LIBTCOD_EXPORTS
#define TCODLIB_API __declspec(dllexport)
#else
#define TCODLIB_API __declspec(dllimport)
#endif
#else
#define TCODLIB_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ansi C lacks support for those functions */
TCODLIB_API char *TCOD_strdup(const char *s);
TCODLIB_API int TCOD_strcasecmp(const char *s1, const char *s2);
TCODLIB_API int TCOD_strncasecmp(const char *s1, const char *s2, size_t n);

#if defined(TCOD_WINDOWS)
char *strcasestr (const char *haystack, const char *needle);
#endif
#if defined(TCOD_LINUX) || defined(TCOD_HAIKU) || defined(TCOD_FREEBSD) || defined(TCOD_MACOSX)
#define vsnwprintf vswprintf
#endif
#ifdef TCOD_WINDOWS
#define vsnwprintf _vsnwprintf
#endif

/******************************************
 utility macros
 ******************************************/
#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))
#define ABS(a) ((a)<0?-(a):(a))
#define CLAMP(a, b, x)		((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define LERP(a, b, x) ( (a) + (x) * ((b) - (a)) )

#include "list.h"
#include "color.h"
#include "console.h"
#include "image.h"
#include "mouse.h"
#include "sys.h"
#include "mersenne.h"
#include "bresenham.h"
#include "noise.h"
#include "fov.h"
#include "path.h"
#include "lex.h"
#include "parser.h"
#include "tree.h"
#include "bsp.h"
#include "heightmap.h"
// ~ #include "zip.h"
#include "namegen.h"
#include "txtfield.h"
#ifdef __cplusplus
#undef bool
}
#endif

#endif

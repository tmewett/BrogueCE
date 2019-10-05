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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <string.h>

#if defined (__linux) && ! defined (__ANDROID__) || defined (__FreeBSD__)
/* X11 stuff for clipboard support */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#elif defined (__APPLE__) && defined (__MACH__)
#include <ApplicationServices/ApplicationServices.h>
#endif
#include "libtcod.h"
#include "libtcod_int.h"
#ifdef TCOD_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>
#endif

#if defined(TCOD_WINDOWS)
char *strcasestr (const char *haystack, const char *needle) {
	const char *p, *startn = 0, *np = 0;

	for (p = haystack; *p; p++) {
		if (np) {
			if (toupper(*p) == toupper(*np)) {
				if (!*++np)
					return (char *)startn;
			} else
				np = 0;
		} else if (toupper(*p) == toupper(*needle)) {
			np = needle + 1;
			startn = p;
		}
	}

	return 0;
}
#endif

void TCOD_sys_get_fullscreen_offsets(int *offx, int *offy) {
	if ( offx ) *offx = TCOD_ctx.fullscreen_offsetx;
	if ( offy ) *offy = TCOD_ctx.fullscreen_offsety;
}

bool TCOD_sys_create_directory(const char *path) {
#ifdef TCOD_WINDOWS
	return (CreateDirectory(path,NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS);
#else
	return mkdir(path,0755) == 0 || errno == EEXIST;
#endif
}

bool TCOD_sys_delete_file(const char *path) {
#ifdef TCOD_WINDOWS
	return DeleteFile(path) != 0;
#else
	return unlink(path) == 0 || errno == ENOENT;
#endif
}

bool TCOD_sys_delete_directory(const char *path) {
#ifdef TCOD_WINDOWS
	return RemoveDirectory(path) != 0;
#else
	return rmdir(path) == 0 || errno == ENOENT;
#endif
}


bool TCOD_sys_is_directory(const char *path) {
#ifdef TCOD_WINDOWS
	DWORD type=GetFileAttributes(path);
	return ( type & FILE_ATTRIBUTE_DIRECTORY) != 0 ;
#else
	DIR *d=opendir(path);
	if ( d ) { closedir(d); return true; }
	return false;
#endif
}


static bool filename_match(const char *name, const char *pattern) {
	char *ptr;
	if ( pattern == NULL || pattern[0] == 0 ) return true;
	ptr=strchr(pattern,'*');
	if ( ! ptr ) return strcmp(name,pattern) == 0;
	if ( ptr != name && strncmp(name,pattern, ptr - pattern) != 0 ) return false;
	return  strcmp( name + strlen(name) - strlen(ptr+1), ptr+1) == 0;
}

TCOD_list_t TCOD_sys_get_directory_content(const char *path, const char *pattern) {
    TCOD_list_t list=TCOD_list_new();
#ifdef TCOD_WINDOWS
    WIN32_FIND_DATA FileData;
    HANDLE          hList;
	char dname[ 512 ];
	sprintf(dname, "%s\\*",path);
    hList = FindFirstFile(dname, &FileData);
    if (hList == INVALID_HANDLE_VALUE)
    {
        return list;
    }
	do
	{
		if ( ! (strcmp(FileData.cFileName,".") == 0 || strcmp(FileData.cFileName,"..") == 0 ) )
		{
			if ( filename_match(FileData.cFileName,pattern) )
				TCOD_list_push(list,TCOD_strdup(FileData.cFileName));
		}

	} while ( FindNextFile(hList, &FileData) );
    FindClose(hList);
#else
    DIR *dir = opendir(path);
    struct dirent *dirent = NULL;
    if ( ! dir ) return list;
    while ( ( dirent = readdir(dir) ) )
    {
		if ( ! (strcmp(dirent->d_name,".") == 0 || strcmp(dirent->d_name,"..") == 0 ) )
		{
			if ( filename_match(dirent->d_name,pattern) )
				TCOD_list_push(list,strdup(dirent->d_name));
		}
	}
	closedir(dir);
#endif
	return list;
}

/* thread stuff */
#ifdef TCOD_WINDOWS
/* Helper function to count set bits in the processor mask. */
static DWORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}
#endif

int TCOD_sys_get_num_cores() {
#ifdef TCOD_WINDOWS
	/* what a crap !!! works only on xp sp3 & vista */
	typedef enum _PROCESSOR_CACHE_TYPE {
	  CacheUnified,
	  CacheInstruction,
	  CacheData,
	  CacheTrace
	} PROCESSOR_CACHE_TYPE;

	typedef struct _CACHE_DESCRIPTOR {
	  BYTE                   Level;
	  BYTE                   Associativity;
	  WORD                   LineSize;
	  DWORD                  Size;
	  PROCESSOR_CACHE_TYPE   Type;
	} CACHE_DESCRIPTOR,
	 *PCACHE_DESCRIPTOR;
	typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP {
	  RelationProcessorCore,
	  RelationNumaNode,
	  RelationCache,
	  RelationProcessorPackage
	} LOGICAL_PROCESSOR_RELATIONSHIP;

	typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
	  ULONG_PTR                        ProcessorMask;
	  LOGICAL_PROCESSOR_RELATIONSHIP   Relationship;
	  union {
	    struct {
	      BYTE Flags;
	    } ProcessorCore;
	    struct {
	      DWORD NodeNumber;
	    } NumaNode;
	    CACHE_DESCRIPTOR Cache;
	    ULONGLONG Reserved[2];
	  };
	} SYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	 *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;
	typedef BOOL (WINAPI *LPFN_GLPI)(
	    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
	    PDWORD);

    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD byteOffset = 0;

    glpi = (LPFN_GLPI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32")),
                            "GetLogicalProcessorInformation");
    if (! glpi) {
        return 1;
    }

    while (!done) {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                        returnLength);

                if (NULL == buffer) {
                    return 1;
                }
            } else {
                return 1;
            }
        } else {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
        switch (ptr->Relationship) {
        case RelationProcessorCore:
            /* A hyperthreaded core supplies more than one logical processor. */
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;
        default: break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(buffer);

    return logicalProcessorCount;
#else
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

TCOD_thread_t TCOD_thread_new(int (*func)(void *), void *data)
{
#ifdef TCOD_WINDOWS
	HANDLE ret = CreateThread(NULL,0,(DWORD (WINAPI *)( LPVOID ))func,data,0,NULL);
	return (TCOD_thread_t)ret;
#else
	pthread_t id;
	int iret;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	iret =pthread_create(&id,&attr,(void *(*)(void *))func,data);
	if ( iret != 0 ) id=0;
	return (TCOD_thread_t)id;
#endif
}

void TCOD_thread_delete(TCOD_thread_t th)
{
#ifdef TCOD_WINDOWS
	CloseHandle((HANDLE)th);
#endif
}

void TCOD_thread_wait(TCOD_thread_t th) {
#ifdef TCOD_WINDOWS
	WaitForSingleObject((HANDLE)th,INFINITE);
#else
	pthread_t id=(pthread_t)th;
	pthread_join(id,NULL);
#endif
}

TCOD_mutex_t TCOD_mutex_new()
{
#ifdef TCOD_WINDOWS
	CRITICAL_SECTION *cs = (CRITICAL_SECTION *)calloc(sizeof(CRITICAL_SECTION),1);
	InitializeCriticalSection(cs);
	return (TCOD_mutex_t)cs;
#else
	static pthread_mutex_t tmp=PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t *mut = (pthread_mutex_t *)calloc(sizeof(pthread_mutex_t),1);
	*mut = tmp;
	return (TCOD_mutex_t)mut;
#endif
}

void TCOD_mutex_in(TCOD_mutex_t mut)
{
#ifdef TCOD_WINDOWS
	EnterCriticalSection((CRITICAL_SECTION *)mut);
#else
	pthread_mutex_lock((pthread_mutex_t *)mut);
#endif
}

void TCOD_mutex_out(TCOD_mutex_t mut)
{
#ifdef TCOD_WINDOWS
	LeaveCriticalSection((CRITICAL_SECTION *)mut);
#else
	pthread_mutex_unlock((pthread_mutex_t *)mut);
#endif
}

void TCOD_mutex_delete(TCOD_mutex_t mut)
{
#ifdef TCOD_WINDOWS
	DeleteCriticalSection((CRITICAL_SECTION *)mut);
	free(mut);
#else
	pthread_mutex_destroy((pthread_mutex_t *)mut);
	free(mut);
#endif
}

TCOD_semaphore_t TCOD_semaphore_new(int initVal)
{
#ifdef TCOD_WINDOWS
	HANDLE ret = CreateSemaphore(NULL,initVal,255,NULL);
	return (TCOD_semaphore_t)ret;
#else
	sem_t *ret = (sem_t *)calloc(sizeof(sem_t),1);
	if ( ret ) sem_init(ret,0,initVal);
	return (TCOD_semaphore_t) ret;
#endif
}

void TCOD_semaphore_lock(TCOD_semaphore_t sem)
{
#ifdef TCOD_WINDOWS
	WaitForSingleObject((HANDLE)sem,INFINITE);
#else
	if ( sem ) sem_wait((sem_t *)sem);
#endif
}

void TCOD_semaphore_unlock(TCOD_semaphore_t sem)
{
#ifdef TCOD_WINDOWS
	ReleaseSemaphore((HANDLE)sem,1,NULL);
#else
	if ( sem ) sem_post((sem_t *)sem);
#endif
}

void TCOD_semaphore_delete( TCOD_semaphore_t sem)
{
#ifdef TCOD_WINDOWS
	CloseHandle((HANDLE)sem);
#else
	if ( sem )
	{
		sem_destroy((sem_t *)sem);
		free (sem);
	}
#endif
}

#ifdef TCOD_WINDOWS
/* poor win32 api has no thread conditions */
typedef struct {
	int nbSignals;
	int nbWaiting;
	TCOD_mutex_t mutex;
	TCOD_semaphore_t waiting;
	TCOD_semaphore_t waitDone;
} cond_t;
#endif

TCOD_cond_t TCOD_condition_new() {
#ifdef TCOD_WINDOWS
	cond_t *ret = (cond_t *)calloc(sizeof(cond_t),1);
	ret->mutex = TCOD_mutex_new();
	ret->waiting = TCOD_semaphore_new(0);
	ret->waitDone = TCOD_semaphore_new(0);
	return (TCOD_cond_t)ret;
#else
	pthread_cond_t *ret = (pthread_cond_t *)calloc(sizeof(pthread_cond_t),1);
	if ( ret ) pthread_cond_init(ret,NULL);
	return (TCOD_cond_t) ret;
#endif
}

void TCOD_condition_signal(TCOD_cond_t pcond) {
#ifdef TCOD_WINDOWS
	cond_t *cond=(cond_t *)pcond;
	if ( cond ) {
		TCOD_mutex_in(cond->mutex);
		if ( cond->nbWaiting > cond->nbSignals ) {
			cond->nbSignals++;
			TCOD_semaphore_unlock(cond->waiting);
			TCOD_mutex_out(cond->mutex);
			TCOD_semaphore_lock(cond->waitDone);
		} else {
			TCOD_mutex_out(cond->mutex);
		}
	}
#else
	if ( pcond ) {
		pthread_cond_signal((pthread_cond_t *)pcond);
	}
#endif
}

void TCOD_condition_broadcast(TCOD_cond_t pcond) {
#ifdef TCOD_WINDOWS
	cond_t *cond=(cond_t *)pcond;
	if ( cond ) {
		TCOD_mutex_in(cond->mutex);
		if ( cond->nbWaiting > cond->nbSignals ) {
			int nbUnlock=cond->nbWaiting-cond->nbSignals;
			int i;
			cond->nbSignals=cond->nbWaiting;
			for (i=nbUnlock; i > 0; i--) {
				TCOD_semaphore_unlock(cond->waiting);
			}
			TCOD_mutex_out(cond->mutex);
			for (i=nbUnlock; i > 0; i--) {
				TCOD_semaphore_lock(cond->waitDone);
			}
		} else {
			TCOD_mutex_out(cond->mutex);
		}
	}
#else
	if ( pcond ) {
		pthread_cond_broadcast((pthread_cond_t *)pcond);
	}
#endif
}

void TCOD_condition_wait(TCOD_cond_t pcond, TCOD_mutex_t mut) {
#ifdef TCOD_WINDOWS
	cond_t *cond=(cond_t *)pcond;
	if ( cond ) {
		TCOD_mutex_in(cond->mutex);
		cond->nbWaiting++;
		TCOD_mutex_out(cond->mutex);
		TCOD_mutex_out(mut);
		TCOD_semaphore_lock(cond->waiting);
		TCOD_mutex_in(cond->mutex);
		if ( cond->nbSignals > 0 ) {
			TCOD_semaphore_unlock(cond->waitDone);
			cond->nbSignals--;
		}
		cond->nbWaiting--;
		TCOD_mutex_out(cond->mutex);
	}
#else
	if ( pcond && mut ) {
		pthread_cond_wait((pthread_cond_t *)pcond, (pthread_mutex_t *)mut);
	}
#endif
}

void TCOD_condition_delete( TCOD_cond_t pcond) {
#ifdef TCOD_WINDOWS
	cond_t *cond=(cond_t *)pcond;
	if ( cond ) {
		TCOD_mutex_delete(cond->mutex);
		TCOD_semaphore_delete(cond->waiting);
		TCOD_semaphore_delete(cond->waitDone);
		free(cond);
	}
#else
	if ( pcond ) {
		pthread_cond_destroy((pthread_cond_t *)pcond);
		free (pcond);
	}
#endif
}

/*clipboard stuff */
#ifdef TCOD_WINDOWS
void TCOD_sys_clipboard_set(const char *value)
{
    HGLOBAL clipbuffer;
    char * buffer;
	if (!OpenClipboard(0) || ! value) return;
    EmptyClipboard();
    clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(value)+1);
    buffer = (char*)GlobalLock(clipbuffer);
	strcpy(buffer, value);
    GlobalUnlock(clipbuffer);
    SetClipboardData(CF_TEXT, clipbuffer);
    CloseClipboard();
}

char *TCOD_sys_clipboard_get()
{
    char * buffer = NULL;
    HANDLE hData;
	if (!OpenClipboard(NULL)) return 0;
    hData = GetClipboardData( CF_TEXT );
    buffer = (char*)GlobalLock( hData );
    GlobalUnlock( hData );
    CloseClipboard();
    return buffer;
}
#elif defined(TCOD_MACOSX)
void TCOD_sys_clipboard_set(const char *value)
{
	PasteboardRef clipboard;
  if (PasteboardCreate(kPasteboardClipboard, &clipboard) != noErr) return;
  if (PasteboardClear(clipboard) != noErr) {
      CFRelease(clipboard);
      return;
  }
	size_t len = strlen(value);
  CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
																					(const UInt8 *)value,
                                          len, kCFAllocatorNull);
  if (data == NULL) {
      CFRelease(clipboard);
      return;
  }
  OSStatus err;
  err = PasteboardPutItemFlavor(clipboard, NULL, kUTTypePlainText, data, 0);
  CFRelease(clipboard);
  CFRelease(data);
}

char clipboardText[256];

char *TCOD_sys_clipboard_get()
{
	PasteboardSyncFlags syncFlags;
	ItemCount itemCount;
	PasteboardRef clipboard;
	UInt32 itemIndex;
	if (PasteboardCreate(kPasteboardClipboard, &clipboard) != noErr) return NULL;
	syncFlags = PasteboardSynchronize(clipboard);
	if (PasteboardGetItemCount(clipboard, &itemCount) != noErr) return NULL;
	if (itemCount == 0) return NULL;
	for(itemIndex = 1; itemIndex <= itemCount; itemIndex++) {
		PasteboardItemID itemID;
		CFArrayRef flavorTypeArray;
		CFIndex flavorCount;
		CFIndex flavorIndex;
		if (PasteboardGetItemIdentifier(clipboard, itemIndex, &itemID ) != noErr) return NULL;
		if (PasteboardCopyItemFlavors(clipboard, itemID, &flavorTypeArray) != noErr) return NULL;
		flavorCount = CFArrayGetCount(flavorTypeArray);
		for(flavorIndex = 0; flavorIndex < flavorCount; flavorIndex++) {
        CFStringRef flavorType;
        CFDataRef flavorData;
        CFIndex flavorDataSize;
				flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);
				if (UTTypeConformsTo(flavorType, CFSTR("public.plain-text"))) {
						if (PasteboardCopyItemFlavorData(clipboard, itemID, flavorType, &flavorData) != noErr) {
							CFRelease(flavorData);
							return NULL;
						}
						flavorDataSize = CFDataGetLength( flavorData );
						flavorDataSize = (flavorDataSize<254) ? flavorDataSize : 254;
						short dataIndex;
	          for(dataIndex = 0; dataIndex <= flavorDataSize; dataIndex++) {
	             clipboardText[dataIndex] = *(CFDataGetBytePtr(flavorData) + dataIndex);
	          }
	          clipboardText[flavorDataSize] = '\0';
	          clipboardText[flavorDataSize+1] = '\n';
						CFRelease (flavorData);
	     }
		}
	}
	return clipboardText;
}
#elif defined(TCOD_HAIKU) || defined(__ANDROID__)
/* TODO */
void TCOD_sys_clipboard_set(const char *value)
{
}
char *TCOD_sys_clipboard_get()
{
	return "";
}
#else
static Display *dpy=NULL;
void TCOD_sys_clipboard_set(const char *value)
{
	if ( ! value ) return;
	if (!dpy ) dpy = XOpenDisplay(NULL);
	XStoreBytes(dpy,value,strlen(value)+1);
	/* doesn't seem to work without this... */
	int len;
	char *xbuf = XFetchBytes(dpy,&len);
	XFree(xbuf);
}

char *TCOD_sys_clipboard_get()
{
	int len;
	if (!dpy ) dpy = XOpenDisplay(NULL);
	char *xbuf = XFetchBytes(dpy,&len);
	if (! xbuf ) return NULL;
	char *ret=strdup(xbuf);
	XFree(xbuf);
	return ret;
}
#endif


/* library initialization function */
#ifdef TCOD_WINDOWS
BOOL APIENTRY DllMain( HANDLE hModule, DWORD reason, LPVOID reserved) {
	switch (reason ) {
		/* case DLL_PROCESS_ATTACH : TCOD_sys_startup(); break;  -- not safe, locks up in SDL2/RegisterClass call */
		default : break;
	}
	return TRUE;
}
#else
/* JBR03202012 Presumably there was a reason for this being if !MACOSOX, but it works fine for me
	#ifndef TCOD_MACOSX */
	void __attribute__ ((constructor)) DllMain() {
		/* TCOD_sys_startup(); */
	}
/*	#endif */
#endif

/* dynamic library support */
#ifdef TCOD_WINDOWS
TCOD_library_t TCOD_load_library(const char *path) {
	return (TCOD_library_t)LoadLibrary(path);
}
void * TCOD_get_function_address(TCOD_library_t library, const char *function_name) {
	return (void *)GetProcAddress((HMODULE)library,function_name);	
}
void TCOD_close_library(TCOD_library_t library) {
	FreeLibrary((HMODULE)library);
}
#else
TCOD_library_t TCOD_load_library(const char *path) {
	void *l=dlopen(path,RTLD_LAZY);
	return (TCOD_library_t)l;
}
void * TCOD_get_function_address(TCOD_library_t library, const char *function_name) {
	return dlsym(library,(char *)function_name);
}
void TCOD_close_library(TCOD_library_t library) {
	dlclose(library);
}
#endif

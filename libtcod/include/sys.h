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

#ifndef _TCOD_SYS_H
#define _TCOD_SYS_H

TCODLIB_API uint32 TCOD_sys_elapsed_milli();
TCODLIB_API float TCOD_sys_elapsed_seconds();
TCODLIB_API void TCOD_sys_sleep_milli(uint32 val);
TCODLIB_API void TCOD_sys_save_screenshot(const char *filename);
TCODLIB_API void TCOD_sys_force_fullscreen_resolution(int width, int height);
TCODLIB_API void TCOD_sys_set_renderer(TCOD_renderer_t renderer);
TCODLIB_API TCOD_renderer_t TCOD_sys_get_renderer();
TCODLIB_API void TCOD_sys_set_fps(int val);
TCODLIB_API int TCOD_sys_get_fps();
TCODLIB_API float TCOD_sys_get_last_frame_length();
TCODLIB_API void TCOD_sys_get_current_resolution(int *w, int *h);
TCODLIB_API void TCOD_sys_get_fullscreen_offsets(int *offx, int *offy);
TCODLIB_API void TCOD_sys_update_char(int asciiCode, int fontx, int fonty, TCOD_image_t img, int x, int y);
TCODLIB_API void TCOD_sys_get_char_size(int *w, int *h);
TCODLIB_API void *TCOD_sys_get_SDL_window();

typedef enum {
  TCOD_EVENT_NONE=0,
  TCOD_EVENT_KEY_PRESS=1,
  TCOD_EVENT_KEY_RELEASE=2,
  TCOD_EVENT_KEY=TCOD_EVENT_KEY_PRESS|TCOD_EVENT_KEY_RELEASE,
  TCOD_EVENT_MOUSE_MOVE=4,
  TCOD_EVENT_MOUSE_PRESS=8,
  TCOD_EVENT_MOUSE_RELEASE=16,
  TCOD_EVENT_MOUSE=TCOD_EVENT_MOUSE_MOVE|TCOD_EVENT_MOUSE_PRESS|TCOD_EVENT_MOUSE_RELEASE,
/* #ifdef TCOD_TOUCH_INPUT */
  TCOD_EVENT_FINGER_MOVE=32,
  TCOD_EVENT_FINGER_PRESS=64,
  TCOD_EVENT_FINGER_RELEASE=128,
  TCOD_EVENT_FINGER=TCOD_EVENT_FINGER_MOVE|TCOD_EVENT_FINGER_PRESS|TCOD_EVENT_FINGER_RELEASE,
/* #endif */
  TCOD_EVENT_ANY=TCOD_EVENT_KEY|TCOD_EVENT_MOUSE|TCOD_EVENT_FINGER,
} TCOD_event_t;
TCODLIB_API TCOD_event_t TCOD_sys_wait_for_event(int eventMask, TCOD_key_t *key, TCOD_mouse_t *mouse, bool flush);
TCODLIB_API TCOD_event_t TCOD_sys_check_for_event(int eventMask, TCOD_key_t *key, TCOD_mouse_t *mouse);

/* filesystem stuff */
TCODLIB_API bool TCOD_sys_create_directory(const char *path);
TCODLIB_API bool TCOD_sys_delete_file(const char *path);
TCODLIB_API bool TCOD_sys_delete_directory(const char *path);
TCODLIB_API bool TCOD_sys_is_directory(const char *path);
TCODLIB_API TCOD_list_t TCOD_sys_get_directory_content(const char *path, const char *pattern);
TCODLIB_API bool TCOD_sys_file_exists(const char * filename, ...);
TCODLIB_API bool TCOD_sys_read_file(const char *filename, unsigned char **buf, size_t *size);
TCODLIB_API bool TCOD_sys_write_file(const char *filename, unsigned char *buf, uint32 size);

/* clipboard */
TCODLIB_API void TCOD_sys_clipboard_set(const char *value);
TCODLIB_API char *TCOD_sys_clipboard_get();

/* thread stuff */
typedef void *TCOD_thread_t;
typedef void *TCOD_semaphore_t;
typedef void *TCOD_mutex_t;
typedef void *TCOD_cond_t;
/* threads */
TCODLIB_API TCOD_thread_t TCOD_thread_new(int (*func)(void *), void *data);
TCODLIB_API void TCOD_thread_delete(TCOD_thread_t th);
TCODLIB_API int TCOD_sys_get_num_cores();
TCODLIB_API void TCOD_thread_wait(TCOD_thread_t th);
/* mutex */
TCODLIB_API TCOD_mutex_t TCOD_mutex_new();
TCODLIB_API void TCOD_mutex_in(TCOD_mutex_t mut);
TCODLIB_API void TCOD_mutex_out(TCOD_mutex_t mut);
TCODLIB_API void TCOD_mutex_delete(TCOD_mutex_t mut);
/* semaphore */
TCODLIB_API TCOD_semaphore_t TCOD_semaphore_new(int initVal);
TCODLIB_API void TCOD_semaphore_lock(TCOD_semaphore_t sem);
TCODLIB_API void TCOD_semaphore_unlock(TCOD_semaphore_t sem);
TCODLIB_API void TCOD_semaphore_delete( TCOD_semaphore_t sem);
/* condition */
TCODLIB_API TCOD_cond_t TCOD_condition_new();
TCODLIB_API void TCOD_condition_signal(TCOD_cond_t sem);
TCODLIB_API void TCOD_condition_broadcast(TCOD_cond_t sem);
TCODLIB_API void TCOD_condition_wait(TCOD_cond_t sem, TCOD_mutex_t mut);
TCODLIB_API void TCOD_condition_delete( TCOD_cond_t sem);
/* dynamic library */
typedef void *TCOD_library_t;
TCODLIB_API TCOD_library_t TCOD_load_library(const char *path);
TCODLIB_API void * TCOD_get_function_address(TCOD_library_t library, const char *function_name);
TCODLIB_API void TCOD_close_library(TCOD_library_t);
/* SDL renderer callback */
#ifdef TCOD_SDL2
typedef void (*SDL_renderer_t) (void *sdl_renderer);
#else
typedef void (*SDL_renderer_t) (void *sdl_surface);
#endif
TCODLIB_API void TCOD_sys_register_SDL_renderer(SDL_renderer_t renderer);
#endif

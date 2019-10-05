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

#ifndef _TCOD_PATH_H
#define _TCOD_PATH_H

typedef float (*TCOD_path_func_t)( int xFrom, int yFrom, int xTo, int yTo, void *user_data );
typedef void *TCOD_path_t;

TCODLIB_API TCOD_path_t TCOD_path_new_using_map(TCOD_map_t map, float diagonalCost);
TCODLIB_API TCOD_path_t TCOD_path_new_using_function(int map_width, int map_height, TCOD_path_func_t func, void *user_data, float diagonalCost);

TCODLIB_API bool TCOD_path_compute(TCOD_path_t path, int ox,int oy, int dx, int dy);
TCODLIB_API bool TCOD_path_walk(TCOD_path_t path, int *x, int *y, bool recalculate_when_needed);
TCODLIB_API bool TCOD_path_is_empty(TCOD_path_t path);
TCODLIB_API int TCOD_path_size(TCOD_path_t path);
TCODLIB_API void TCOD_path_reverse(TCOD_path_t path);
TCODLIB_API void TCOD_path_get(TCOD_path_t path, int index, int *x, int *y);
TCODLIB_API void TCOD_path_get_origin(TCOD_path_t path, int *x, int *y);
TCODLIB_API void TCOD_path_get_destination(TCOD_path_t path, int *x, int *y);
TCODLIB_API void TCOD_path_delete(TCOD_path_t path);

/* Dijkstra stuff - by Mingos*/

typedef void *TCOD_dijkstra_t;

TCODLIB_API TCOD_dijkstra_t TCOD_dijkstra_new (TCOD_map_t map, float diagonalCost);
TCODLIB_API TCOD_dijkstra_t TCOD_dijkstra_new_using_function(int map_width, int map_height, TCOD_path_func_t func, void *user_data, float diagonalCost);
TCODLIB_API void TCOD_dijkstra_compute (TCOD_dijkstra_t dijkstra, int root_x, int root_y);
TCODLIB_API float TCOD_dijkstra_get_distance (TCOD_dijkstra_t dijkstra, int x, int y);
TCODLIB_API bool TCOD_dijkstra_path_set (TCOD_dijkstra_t dijkstra, int x, int y);
TCODLIB_API bool TCOD_dijkstra_is_empty(TCOD_dijkstra_t path);
TCODLIB_API int TCOD_dijkstra_size(TCOD_dijkstra_t path);
TCODLIB_API void TCOD_dijkstra_reverse(TCOD_dijkstra_t path);
TCODLIB_API void TCOD_dijkstra_get(TCOD_dijkstra_t path, int index, int *x, int *y);
TCODLIB_API bool TCOD_dijkstra_path_walk (TCOD_dijkstra_t dijkstra, int *x, int *y);
TCODLIB_API void TCOD_dijkstra_delete (TCOD_dijkstra_t dijkstra);

#endif

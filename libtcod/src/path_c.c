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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "libtcod.h"
#include "libtcod_int.h"
enum { NORTH_WEST,NORTH, NORTH_EAST, WEST,NONE,EAST, SOUTH_WEST,SOUTH,SOUTH_EAST };
typedef unsigned char dir_t;

/* convert dir_t to dx,dy */
static int dirx[]={-1,0,1,-1,0,1,-1,0,1};
static int diry[]={-1,-1,-1,0,0,0,1,1,1};
static int invdir[] = {SOUTH_EAST,SOUTH,SOUTH_WEST,EAST,NONE,WEST,NORTH_EAST,NORTH,NORTH_WEST};

typedef struct {
	int ox,oy; /* coordinates of the creature position */
	int dx,dy; /* coordinates of the creature's destination */
	TCOD_list_t path; /* list of dir_t to follow the path */
	int w,h; /* map size */
	float *grid; /* wxh djikstra distance grid (covered distance) */
	float *heur; /* wxh A* score grid (covered distance + estimated remaining distance) */
	dir_t *prev; /* wxh 'previous' grid : direction to the previous cell */
	float diagonalCost;
	TCOD_list_t heap; /* min_heap used in the algorithm. stores the offset in grid/heur (offset=x+y*w) */
	TCOD_map_t map;
	TCOD_path_func_t func;
	void *user_data;
} TCOD_path_data_t;

/* small layer on top of TCOD_list_t to implement a binary heap (min_heap) */
static void heap_sift_down(TCOD_path_data_t *path, TCOD_list_t heap) {
	/* sift-down : move the first element of the heap down to its right place */
	int cur=0;
	int end = TCOD_list_size(heap)-1;
	int child=1;
	uintptr *array=(uintptr *)TCOD_list_begin(heap);
	while ( child <= end ) {
		int toSwap=cur;
		uint32 off_cur=array[cur];
		float cur_dist=path->heur[off_cur];
		float swapValue=cur_dist;
		uint32 off_child=array[child];
		float child_dist=path->heur[off_child];
		if ( child_dist < cur_dist ) {
			toSwap=child;
			swapValue=child_dist;
		}
		if ( child < end ) {
			/* get the min between child and child+1 */
			uintptr off_child2=array[child+1];
			float child2_dist=path->heur[off_child2];
			if ( swapValue > child2_dist ) {
				toSwap=child+1;
				swapValue=child2_dist;
			}
		}
		if ( toSwap != cur ) {
			/* get down one level */
			uintptr tmp = array[toSwap];
			array[toSwap]=array[cur];
			array[cur]=tmp;
			cur=toSwap;
		} else return;
		child=cur*2+1;
	}
}

static void heap_sift_up(TCOD_path_data_t *path, TCOD_list_t heap) {
	/* sift-up : move the last element of the heap up to its right place */
	int end = TCOD_list_size(heap)-1;
	int child=end;
	uintptr *array=(uintptr *)TCOD_list_begin(heap);
	while ( child > 0 ) {
		uintptr off_child=array[child];
		float child_dist=path->heur[off_child];
		int parent = (child-1)/2;
		uintptr off_parent=array[parent];
		float parent_dist=path->heur[off_parent];
		if ( parent_dist > child_dist ) {
			/* get up one level */
			uintptr tmp = array[child];
			array[child]=array[parent];
			array[parent]=tmp;
			child=parent;
		} else return;
	}
}

/* add a coordinate pair in the heap so that the heap root always contains the minimum A* score */
static void heap_add(TCOD_path_data_t *path, TCOD_list_t heap, int x, int y) {
	/* append the new value to the end of the heap */
	uintptr off=x+y*path->w;
	TCOD_list_push(heap,(void *)off);
	/* bubble the value up to its real position */
	heap_sift_up(path,heap);
}

/* get the coordinate pair with the minimum A* score from the heap */
static uint32 heap_get(TCOD_path_data_t *path,TCOD_list_t heap) {
	/* return the first value of the heap (minimum score) */
	uintptr *array=(uintptr *)TCOD_list_begin(heap);
	int end=TCOD_list_size(heap)-1;
	uint32 off=(uint32)(array[0]);
	/* take the last element and put it at first position (heap root) */
	array[0] = array[end];
	TCOD_list_pop(heap);
	/* and bubble it down to its real position */
	heap_sift_down(path,heap);
	return off;
}

/* this is the slow part, when we change the heuristic of a cell already in the heap */
static void heap_reorder(TCOD_path_data_t *path, uint32 offset) {
	uintptr *array=(uintptr *)TCOD_list_begin(path->heap);
	uintptr *end=(uintptr *)TCOD_list_end(path->heap);
	uintptr *cur=array;
	uintptr off_idx=0;
	float value;
	int idx=0;
	int heap_size=TCOD_list_size(path->heap);
	/* find the node corresponding to offset ... SLOW !! */
	while (cur != end) {
		if (*cur == offset ) break;
		cur++;idx++;
	}
	if ( cur == end ) return;
	off_idx=array[idx];
	value=path->heur[off_idx];
	if ( idx > 0 ) {
		int parent=(idx-1)/2;
		/* compare to its parent */
		uintptr off_parent=array[parent];
		float parent_value=path->heur[off_parent];
		if (value < parent_value) {
			/* smaller. bubble it up */
			while ( idx > 0 && value < parent_value ) {
				/* swap with parent */
				array[parent]=off_idx;
				array[idx] = off_parent;
				idx=parent;
				if ( idx > 0 ) {
					parent=(idx-1)/2;
					off_parent=array[parent];
					parent_value=path->heur[off_parent];
				}
			}
			return;
		}
	}
	/* compare to its sons */
	while ( idx*2+1 < heap_size ) {
		int child=idx*2+1;
		uintptr off_child=array[child];
		int toSwap=idx;
		int child2;
		float swapValue=value;
		if ( path->heur[off_child] < value ) {
			/* swap with son1 ? */
			toSwap=child;
			swapValue=path->heur[off_child];
		}
		child2 = child+1;
		if ( child2 < heap_size ) {
			uintptr off_child2=array[child2];
			if ( path->heur[off_child2] < swapValue) {
				/* swap with son2 */
				toSwap=child2;
			}
		}
		if ( toSwap != idx ) {
			/* bigger. bubble it down */
			uintptr tmp = array[toSwap];
			array[toSwap]=array[idx];
			array[idx] = tmp;
			idx=toSwap;
		} else return;
	}
}


/* private functions */
static void TCOD_path_push_cell(TCOD_path_data_t *path, int x, int y);
static void TCOD_path_get_cell(TCOD_path_data_t *path, int *x, int *y, float *distance);
static void TCOD_path_set_cells(TCOD_path_data_t *path);
static float TCOD_path_walk_cost(TCOD_path_data_t *path, int xFrom, int yFrom, int xTo, int yTo);

static TCOD_path_data_t *TCOD_path_new_intern(int w, int h) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)calloc(sizeof(TCOD_path_data_t),1);
	path->w=w;
	path->h=h;
	path->grid=(float *)calloc(sizeof(float),w*h);
	path->heur=(float *)calloc(sizeof(float),w*h);
	path->prev=(dir_t *)calloc(sizeof(dir_t),w*h);
	if (! path->grid || ! path->heur || ! path->prev ) {
		TCOD_fatal("Fatal error : path finding module cannot allocate djikstra grids (size %dx%d)\n",w,h);
		exit(1);
	}
	path->path=TCOD_list_new();
	path->heap=TCOD_list_new();
	return path;
}

TCOD_path_t TCOD_path_new_using_map(TCOD_map_t map, float diagonalCost) {
	TCOD_path_data_t *path;
	TCOD_IFNOT(map != NULL) return NULL;
	path=TCOD_path_new_intern(TCOD_map_get_width(map),TCOD_map_get_height(map));
	path->map=map;
	path->diagonalCost=diagonalCost;
	return (TCOD_path_t)path;
}

TCOD_path_t TCOD_path_new_using_function(int map_width, int map_height, TCOD_path_func_t func, void *user_data, float diagonalCost) {
	TCOD_path_data_t *path;
	TCOD_IFNOT(func != NULL && map_width > 0 && map_height > 0) return NULL;
	path=TCOD_path_new_intern(map_width,map_height);
	path->func=func;
	path->user_data=user_data;
	path->diagonalCost=diagonalCost;
	return (TCOD_path_t)path;
}

bool TCOD_path_compute(TCOD_path_t p, int ox,int oy, int dx, int dy) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return false;
	path->ox=ox;
	path->oy=oy;
	path->dx=dx;
	path->dy=dy;
	TCOD_list_clear(path->path);
	TCOD_list_clear(path->heap);
	if ( ox == dx && oy == dy ) return true; /* trivial case */
	/* check that origin and destination are inside the map */
	TCOD_IFNOT((unsigned)ox < (unsigned)path->w && (unsigned)oy < (unsigned)path->h) return false;
	TCOD_IFNOT((unsigned)dx < (unsigned)path->w && (unsigned)dy < (unsigned)path->h) return false;
	/* initialize djikstra grids */
	memset(path->grid,0,sizeof(float)*path->w*path->h);
	memset(path->prev,NONE,sizeof(dir_t)*path->w*path->h);
	path->heur[ ox + oy * path->w ] = 1.0f; /* anything != 0 */
	TCOD_path_push_cell(path,ox,oy); /* put the origin cell as a bootstrap */
	/* fill the djikstra grid until we reach dx,dy */
	TCOD_path_set_cells(path);
	if ( path->grid[dx + dy * path->w] == 0 ) return false; /* no path found */
	/* there is a path. retrieve it */
	do {
		/* walk from destination to origin, using the 'prev' array */
		int step=path->prev[ dx + dy * path->w ];
		TCOD_list_push(path->path,(void *)(uintptr)step);
		dx -= dirx[step];
		dy -= diry[step];
	} while ( dx != ox || dy != oy );
	return true;
}

void TCOD_path_reverse(TCOD_path_t p) {
	int tmp,i;
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return ;
	tmp=path->ox;
	path->ox=path->dx;
	path->dx=tmp;
	tmp=path->oy;
	path->oy=path->dy;
	path->dy=tmp;
	for (i=0; i < TCOD_list_size(path->path); i++) {
		int d=(int)(uintptr)TCOD_list_get(path->path,i);
		d = invdir[d];
		TCOD_list_set(path->path,(void *)(uintptr)d,i);
	}
}

bool TCOD_path_walk(TCOD_path_t p, int *x, int *y, bool recalculate_when_needed) {
	int newx,newy;
	float can_walk;
	int d;
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return false;
	if ( TCOD_path_is_empty(path) ) return false;
	d=(int)(uintptr)TCOD_list_pop(path->path);
	newx=path->ox + dirx[d];
	newy=path->oy + diry[d];
	/* check if the path is still valid */
	can_walk = TCOD_path_walk_cost(path,path->ox,path->oy,newx,newy);
	if ( can_walk == 0.0f ) {
		if (! recalculate_when_needed ) return false; /* don't walk */
		/* calculate a new path */
		if (! TCOD_path_compute(path, path->ox,path->oy, path->dx,path->dy) ) return false ; /* cannot find a new path */
		return TCOD_path_walk(p,x,y,true); /* walk along the new path */
	}
	if ( x ) *x=newx;
	if ( y ) *y=newy;
	path->ox=newx;
	path->oy=newy;
	return true;
}

bool TCOD_path_is_empty(TCOD_path_t p) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return true;
	return TCOD_list_is_empty(path->path);
}

int TCOD_path_size(TCOD_path_t p) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return 0;
	return TCOD_list_size(path->path);
}

void TCOD_path_get(TCOD_path_t p, int index, int *x, int *y) {
	int pos;
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return;
	if ( x ) *x=path->ox;
	if ( y ) *y=path->oy;
	pos = TCOD_list_size(path->path)-1;
	do {
		int step=(int)(uintptr)TCOD_list_get(path->path,pos);
		if ( x ) *x += dirx[step];
		if ( y ) *y += diry[step];
		pos--;index--;
	} while (index >= 0);
}

void TCOD_path_delete(TCOD_path_t p) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return;
	if ( path->grid ) free(path->grid);
	if ( path->heur ) free(path->heur);
	if ( path->prev ) free(path->prev);
	if ( path->path ) TCOD_list_delete(path->path);
	if ( path->heap ) TCOD_list_delete(path->heap);
	free(path);
}

/* private stuff */
/* add a new unvisited cells to the cells-to-treat list
 * the list is in fact a min_heap. Cell at index i has its sons at 2*i+1 and 2*i+2
 */
static void TCOD_path_push_cell(TCOD_path_data_t *path, int x, int y) {
	heap_add(path,path->heap,x,y);
}

/* get the best cell from the heap */
static void TCOD_path_get_cell(TCOD_path_data_t *path, int *x, int *y, float *distance) {
	uint32 offset = heap_get(path,path->heap);
	*x=(offset % path->w);
	*y=(offset / path->w);
	*distance=path->grid[offset];
}
/* fill the grid, starting from the origin until we reach the destination */
static void TCOD_path_set_cells(TCOD_path_data_t *path) {
	while ( path->grid[path->dx + path->dy * path->w ] == 0 && ! TCOD_list_is_empty(path->heap) ) {

		int x,y,i,imax;
		float distance;
		TCOD_path_get_cell(path,&x,&y,&distance);
		imax= ( path->diagonalCost == 0.0f ? 4 : 8) ;
		for (i=0; i < imax; i++ ) {
			/* convert i to dx,dy */
			static int idirx[]={0,-1,1,0,-1,1,-1,1};
			static int idiry[]={-1,0,0,1,-1,-1,1,1};
			/* convert i to direction */
			static dir_t prevdirs[] = {
				NORTH, WEST, EAST, SOUTH, NORTH_WEST, NORTH_EAST,SOUTH_WEST,SOUTH_EAST
			};
			/* coordinate of the adjacent cell */
			int cx=x+idirx[i];
			int cy=y+idiry[i];
			if ( cx >= 0 && cy >= 0 && cx < path->w && cy < path->h ) {
				float walk_cost = TCOD_path_walk_cost(path,x,y,cx,cy);
				if ( walk_cost > 0.0f ) {
					/* in of the map and walkable */
					float covered=distance + walk_cost * (i>=4 ? path->diagonalCost : 1.0f);
					float previousCovered = path->grid[cx + cy * path->w ];
					if ( previousCovered == 0 ) {
						/* put a new cell in the heap */
						int offset=cx + cy * path->w;
						/* A* heuristic : remaining distance */
						float remaining=(float)sqrt((cx-path->dx)*(cx-path->dx)+(cy-path->dy)*(cy-path->dy));
						path->grid[ offset ] = covered;
						path->heur[ offset ] = covered + remaining;
						path->prev[ offset ] =  prevdirs[i];
						TCOD_path_push_cell(path,cx,cy);
					} else if ( previousCovered > covered ) {
						/* we found a better path to a cell already in the heap */
						int offset=cx + cy * path->w;
						path->grid[ offset ] = covered;
						path->heur[ offset ] -= (previousCovered - covered); /* fix the A* score */
						path->prev[ offset ] =  prevdirs[i];
						/* reorder the heap */
						heap_reorder(path,offset);
					}
				}
			}
		}
	}
}

/* check if a cell is walkable (from the pathfinder point of view) */
static float TCOD_path_walk_cost(TCOD_path_data_t *path, int xFrom, int yFrom, int xTo, int yTo) {
	if ( path->map ) return TCOD_map_is_walkable(path->map,xTo,yTo) ? 1.0f : 0.0f;
	return path->func(xFrom,yFrom,xTo,yTo,path->user_data);
}

void TCOD_path_get_origin(TCOD_path_t p, int *x, int *y) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return;
	if ( x ) *x=path->ox;
	if ( y ) *y=path->oy;
}

void TCOD_path_get_destination(TCOD_path_t p, int *x, int *y) {
	TCOD_path_data_t *path=(TCOD_path_data_t *)p;
	TCOD_IFNOT(p != NULL) return;
	if ( x ) *x=path->dx;
	if ( y ) *y=path->dy;
}

/* ------------------------------------------------------- *
 * Dijkstra                                                *
 * written by Mingos                                       *
 * -----------------                                       *
 * A floodfill-like algo that will calculate all distances *
 * to all accessible cells (nodes) from a given root node. *
 * ------------------------------------------------------- */

/* Dijkstra data structure */
typedef struct {
	int diagonal_cost;
	int width, height, nodes_max;
	TCOD_map_t map; /* a TCODMap with walkability data */
	TCOD_path_func_t func;
	void *user_data;
	unsigned int * distances; /* distances grid */
	unsigned int * nodes; /* the processed nodes */
	TCOD_list_t path;
} dijkstra_t;

/* create a Dijkstra object */
TCOD_dijkstra_t TCOD_dijkstra_new (TCOD_map_t map, float diagonalCost) {
	dijkstra_t * data ;
	TCOD_IFNOT(map != NULL) return NULL;
	data = malloc(sizeof(dijkstra_t));
	data->map = map;
	data->func = NULL;
	data->user_data=NULL;
	data->distances = malloc(TCOD_map_get_nb_cells(data->map)*sizeof(int));
	data->nodes = malloc(TCOD_map_get_nb_cells(data->map)*sizeof(int));
	data->diagonal_cost = (int)((diagonalCost * 100.0f)+0.1f); /* because (int)(1.41f*100.0f) == 140!!! */
	data->width = TCOD_map_get_width(data->map);
	data->height = TCOD_map_get_height(data->map);
	data->nodes_max = TCOD_map_get_nb_cells(data->map);
	data->path = TCOD_list_new();
	return (TCOD_dijkstra_t)data;
}

TCOD_dijkstra_t TCOD_dijkstra_new_using_function(int map_width, int map_height, TCOD_path_func_t func, void *user_data, float diagonalCost) {
	dijkstra_t * data;
	TCOD_IFNOT(func != NULL && map_width > 0 && map_height > 0) return NULL;
	data = malloc(sizeof(dijkstra_t));
	data->map = NULL;
	data->func = func;
	data->user_data=user_data;
	data->distances = malloc(map_width*map_height*sizeof(int)*4);
	data->nodes = malloc(map_width*map_height*sizeof(int)*4);
	data->diagonal_cost = (int)((diagonalCost * 100.0f)+0.1f); /* because (int)(1.41f*100.0f) == 140!!! */
	data->width = map_width;
	data->height = map_height;
	data->nodes_max = map_width*map_height;
	data->path = TCOD_list_new();
	return (TCOD_dijkstra_t)data;
}


/* compute a Dijkstra grid */
void TCOD_dijkstra_compute (TCOD_dijkstra_t dijkstra, int root_x, int root_y) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	/* map size data */
	unsigned int mx = data->width;
	unsigned int my = data->height;
	unsigned int mmax = data->nodes_max;
	/* encode the root coords in one integer */
	unsigned int root = (root_y * mx) + root_x;
	/* some stuff to walk through the nodes table */
	unsigned int index = 0; /* the index of the first node in queue */
	unsigned int last_index = 1; /* total nb of registered queue indices */
	unsigned int * nodes = data->nodes; /* table of nodes to which the indices above apply */
	/* ok, here's the order of node processing: W, S, E, N, NW, NE, SE, SW */
	static int dx[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
	static int dy[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
	/* and distances for each index */
	int dd[8] = { 100, 100, 100, 100, data->diagonal_cost, data->diagonal_cost, data->diagonal_cost, data->diagonal_cost };
	/* if diagonal_cost is 0, disallow diagonal moves */
	int imax = (data->diagonal_cost == 0 ? 4 : 8);
	/* aight, now set the distances table and set everything to infinity */
	unsigned int * distances = data->distances;
	TCOD_IFNOT(data != NULL) return;
	TCOD_IFNOT((unsigned)root_x < (unsigned)mx && (unsigned)root_y < (unsigned)my) return;
	memset(distances,0xFFFFFFFF,mmax*sizeof(int));
	/* data for root node is known... */
	distances[root] = 0;
	nodes[index] = root; /*set starting note to root */
	/* and the loop */
	do {
		/* coordinates of currently processed node */
		unsigned int x = nodes[index] % mx;
		unsigned int y = nodes[index] / mx;

		/* check adjacent nodes */
		int i;
		for(i=0;i<imax;i++) {
			/* checked node's coordinates */
			unsigned int tx = x + dx[i];
			unsigned int ty = y + dy[i];
			if ((unsigned)tx < (unsigned)mx && (unsigned)ty < (unsigned)my  ) {
				/* otherwise, calculate distance, ... */
				unsigned int dt = distances[nodes[index]], new_node;
				float userDist = 0.0f;
				if ( data->map ) dt += dd[i];
				else {
					/* distance given by the user callback */
					userDist=data->func(x,y,tx,ty,data->user_data);
					dt += (unsigned int)(userDist*dd[i]);
				}
				/* ..., encode coordinates, ... */
				new_node = (ty * mx) + tx;
				/* and check if the node's eligible for queuing */
				 if (distances[new_node] > dt) {
					unsigned int j;
					/* if not walkable, don't process it */
					if (data->map && !TCOD_map_is_walkable(data->map,tx,ty)) continue;
					else if ( data->func && userDist <= 0.0f ) continue;
					distances[new_node] = dt; /* set processed node's distance */
					/* place the processed node in the queue before the last queued node with greater distance */
					j = last_index - 1;
					while (distances[nodes[j]] >= distances[new_node]) {
						/* this ensures that if the node has been queued previously, but with a higher distance, it's removed */
						if (nodes[j] == new_node) {
							int k = j;
							while ((unsigned)k <= last_index) {
								nodes[k] = nodes[k+1];
								k++;
							}
							last_index--;
						}
						else nodes[j+1] = nodes[j];
						j--;
					}
					last_index++; /* increase total indices count */
					nodes[j+1] = new_node; /* and finally put the node where it belongs in the queue */
				}
			}
		}
	} while (mmax > ++index);
}

/* get distance from source */
float TCOD_dijkstra_get_distance (TCOD_dijkstra_t dijkstra, int x, int y) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	unsigned int * distances;
	TCOD_IFNOT(data != NULL) return -1.0f;
	TCOD_IFNOT ((unsigned)x < (unsigned)data->width && (unsigned)y < (unsigned)data->height) return -1.0f;
	if (data->distances[(y*data->width)+x] == 0xFFFFFFFF) return -1.0f;
	distances = data->distances;
	return ((float)distances[(y * data->width) + x] * 0.01f);
}

unsigned int dijkstra_get_int_distance (dijkstra_t * data, int x, int y) {
	unsigned int * distances = data->distances;
	return distances[(y * data->width) + x];
}

/* create a path */
bool TCOD_dijkstra_path_set (TCOD_dijkstra_t dijkstra, int x, int y) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	int px = x, py = y;
	static int dx[9] = { -1, 0, 1, 0, -1, 1, 1, -1, 0 };
	static int dy[9] = { 0, -1, 0, 1, -1, -1, 1, 1, 0 };
	unsigned int distances[8];
	int lowest_index;
	int imax = (data->diagonal_cost == 0 ? 4 : 8);
	TCOD_IFNOT(data != NULL) return false;
	TCOD_IFNOT((unsigned)x < (unsigned)data->width && (unsigned)y < (unsigned)data->height) return false;
	/* check that destination is reachable */
	if ( dijkstra_get_int_distance(data,x,y) == 0xFFFFFFFF ) return false;
	TCOD_list_clear(data->path);
	do {
		unsigned int lowest;
		int i;
		TCOD_list_push(data->path,(const void*)(uintptr)((py * data->width) + px));
		for(i=0;i<imax;i++) {
			int cx = px + dx[i];
			int cy = py + dy[i];
			if ((unsigned)cx < (unsigned)data->width && (unsigned)cy < (unsigned)data->height) distances[i] = dijkstra_get_int_distance(data,cx,cy);
			else distances[i] = 0xFFFFFFFF;
		}
		lowest = dijkstra_get_int_distance(data,px,py);
		lowest_index = 8;
		for(i=0;i<imax;i++) if (distances[i] < lowest) {
			lowest = distances[i];
			lowest_index = i;
		}
		px += dx[lowest_index];
		py += dy[lowest_index];
	} while (lowest_index != 8);
	/* remove the last step */
	TCOD_list_pop(data->path);
	return true;
}

void TCOD_dijkstra_reverse(TCOD_dijkstra_t dijkstra) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	TCOD_IFNOT(data != NULL) return;
	TCOD_list_reverse(data->path);
}

/* walk the path */
bool TCOD_dijkstra_path_walk (TCOD_dijkstra_t dijkstra, int *x, int *y) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	TCOD_IFNOT(data != NULL) return false;
	if (TCOD_list_is_empty(data->path)) return false;
	else {
		unsigned int node = (unsigned int)(uintptr)TCOD_list_pop(data->path);
		if ( x ) *x = (int)(node % data->width);
		if ( y ) *y = (int)(node / data->width);
	}
	return true;
}

/* delete a Dijkstra object */
void TCOD_dijkstra_delete (TCOD_dijkstra_t dijkstra) {
	dijkstra_t * data = (dijkstra_t*)dijkstra;
	TCOD_IFNOT(data != NULL) return;
	if ( data->distances ) free(data->distances);
	if ( data->nodes ) free(data->nodes);
	if ( data->path ) TCOD_list_clear(data->path);
	free(data);
}

bool TCOD_dijkstra_is_empty(TCOD_dijkstra_t p) {
	dijkstra_t * data = (dijkstra_t*)p;
	TCOD_IFNOT(data != NULL) return true;
	return TCOD_list_is_empty(data->path);
}

int TCOD_dijkstra_size(TCOD_dijkstra_t p) {
	dijkstra_t * data = (dijkstra_t*)p;
	TCOD_IFNOT(data != NULL) return 0;
	return TCOD_list_size(data->path);
}

void TCOD_dijkstra_get(TCOD_dijkstra_t p, int index, int *x, int *y) {
	dijkstra_t * data = (dijkstra_t*)p;
	unsigned int node ;
	TCOD_IFNOT(data != NULL) return;
	node = (unsigned int)(uintptr)TCOD_list_get(data->path,TCOD_list_size(data->path)-index-1);
	if ( x ) *x = (int)(node % data->width);
	if ( y ) *y = (int)(node / data->width);
}

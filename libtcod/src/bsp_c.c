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
#include "libtcod.h"

TCOD_bsp_t *TCOD_bsp_new() {
	return (TCOD_bsp_t *)calloc(sizeof(TCOD_bsp_t),1);
}

TCOD_bsp_t *TCOD_bsp_new_with_size(int x,int y,int w, int h) {
	TCOD_bsp_t *bsp=(TCOD_bsp_t *)calloc(sizeof(TCOD_bsp_t),1);
	bsp->x=x;
	bsp->y=y;
	bsp->w=w;
	bsp->h=h;
	return bsp;
}

TCOD_bsp_t * TCOD_bsp_left(TCOD_bsp_t *node) {
	return (TCOD_bsp_t *)node->tree.sons;
}

TCOD_bsp_t * TCOD_bsp_right(TCOD_bsp_t *node) {
	return node->tree.sons ? (TCOD_bsp_t *)node->tree.sons->next : NULL;
}

TCOD_bsp_t * TCOD_bsp_father(TCOD_bsp_t *node) {
	return (TCOD_bsp_t *)node->tree.father;
}

bool TCOD_bsp_is_leaf(TCOD_bsp_t *node) {
	return node->tree.sons==NULL;
}

void TCOD_bsp_delete(TCOD_bsp_t *node) {
	TCOD_bsp_remove_sons(node);
	free(node);
}

static TCOD_bsp_t *TCOD_bsp_new_intern(TCOD_bsp_t *father, bool left) {
	TCOD_bsp_t *bsp=(TCOD_bsp_t *)calloc(sizeof(TCOD_bsp_t),1);
	if ( father->horizontal ) {
		bsp->x=father->x;
		bsp->w=father->w;
		bsp->y = left ? father->y : father->position;
		bsp->h = left ? father->position - bsp->y: father->y + father->h - father->position;
	} else {
		bsp->y=father->y;
		bsp->h=father->h;
		bsp->x = left ? father->x : father->position;
		bsp->w = left ? father->position - bsp->x: father->x + father->w - father->position;
	}
	bsp->level=father->level+1;
	return bsp;
}

bool TCOD_bsp_traverse_pre_order(TCOD_bsp_t *node, TCOD_bsp_callback_t listener, void *userData) {
	if (!listener(node,userData)) return false;
	if ( TCOD_bsp_left(node) && !TCOD_bsp_traverse_pre_order(TCOD_bsp_left(node),listener,userData)) return false;
	if ( TCOD_bsp_right(node) && !TCOD_bsp_traverse_pre_order(TCOD_bsp_right(node),listener,userData)) return false;
	return true;
}

bool TCOD_bsp_traverse_in_order(TCOD_bsp_t *node, TCOD_bsp_callback_t listener, void *userData) {
	if ( TCOD_bsp_left(node) && !TCOD_bsp_traverse_in_order(TCOD_bsp_left(node),listener,userData)) return false;
	if (!listener(node,userData)) return false;
	if ( TCOD_bsp_right(node) && !TCOD_bsp_traverse_in_order(TCOD_bsp_right(node),listener,userData)) return false;
	return true;
}

bool TCOD_bsp_traverse_post_order(TCOD_bsp_t *node, TCOD_bsp_callback_t listener, void *userData) {
	if ( TCOD_bsp_left(node) && !TCOD_bsp_traverse_post_order(TCOD_bsp_left(node),listener,userData)) return false;
	if ( TCOD_bsp_right(node) && !TCOD_bsp_traverse_post_order(TCOD_bsp_right(node),listener,userData)) return false;
	if (!listener(node,userData)) return false;
	return true;
}

bool TCOD_bsp_traverse_level_order(TCOD_bsp_t *node, TCOD_bsp_callback_t listener, void *userData) {
	TCOD_list_t stack=TCOD_list_new();
	TCOD_list_push(stack,node);
	while ( ! TCOD_list_is_empty(stack) ) {
		TCOD_bsp_t *node=(TCOD_bsp_t *)TCOD_list_get(stack,0);
		TCOD_list_remove(stack,node);
		if ( TCOD_bsp_left(node) ) TCOD_list_push(stack,TCOD_bsp_left(node));
		if ( TCOD_bsp_right(node) ) TCOD_list_push(stack,TCOD_bsp_right(node));
		if (!listener(node,userData)) {
			TCOD_list_delete(stack);
			return false;
		}
	}
	TCOD_list_delete(stack);
	return true;
}

bool TCOD_bsp_traverse_inverted_level_order(TCOD_bsp_t *node, TCOD_bsp_callback_t listener, void *userData) {
	TCOD_list_t stack1=TCOD_list_new();
	TCOD_list_t stack2=TCOD_list_new();
	TCOD_list_push(stack1,node);
	while ( ! TCOD_list_is_empty(stack1) ) {
		TCOD_bsp_t *node=(TCOD_bsp_t *)TCOD_list_get(stack1,0);
		TCOD_list_push(stack2,node);
		TCOD_list_remove(stack1,node);
		if ( TCOD_bsp_left(node) ) TCOD_list_push(stack1,TCOD_bsp_left(node));
		if ( TCOD_bsp_right(node) ) TCOD_list_push(stack1,TCOD_bsp_right(node));
	}
	while ( ! TCOD_list_is_empty(stack2) ) {
		TCOD_bsp_t *node=(TCOD_bsp_t *)TCOD_list_pop(stack2);
		if (!listener(node,userData)) {
			TCOD_list_delete(stack1);
			TCOD_list_delete(stack2);
			return false;
		}
	}
	TCOD_list_delete(stack1);
	TCOD_list_delete(stack2);
	return true;
}

void TCOD_bsp_remove_sons(TCOD_bsp_t *root) {
	TCOD_bsp_t *node=(TCOD_bsp_t *)root->tree.sons;
	while ( node ) {
		TCOD_bsp_t *nextNode=(TCOD_bsp_t *)node->tree.next;
		TCOD_bsp_remove_sons(node);
		free( node );
		node=nextNode;
	}
	root->tree.sons=NULL;
}

void TCOD_bsp_split_once(TCOD_bsp_t *node, bool horizontal, int position) {
	node->horizontal = horizontal;
	node->position=position;
	TCOD_tree_add_son(&node->tree,&TCOD_bsp_new_intern(node,true)->tree);
	TCOD_tree_add_son(&node->tree,&TCOD_bsp_new_intern(node,false)->tree);
}

void TCOD_bsp_split_recursive(TCOD_bsp_t *node, TCOD_random_t randomizer, int nb, 
	int minHSize, int minVSize, float maxHRatio, float maxVRatio) {
	bool horiz;
	int position;
	if ( nb == 0 || (node->w < 2*minHSize && node->h < 2*minVSize ) ) return;
	if (! randomizer ) randomizer=TCOD_random_get_instance();
	/* promote square rooms */
	if ( node->h < 2*minVSize || node->w > node->h * maxHRatio ) horiz = false;
	else if ( node->w < 2*minHSize || node->h > node->w * maxVRatio) horiz = true;
	else horiz = (TCOD_random_get_int(randomizer,0,1) == 0);
	if ( horiz ) {
		position = TCOD_random_get_int(randomizer,node->y+minVSize,node->y+node->h-minVSize);
	} else {
		position = TCOD_random_get_int(randomizer,node->x+minHSize,node->x+node->w-minHSize);
	}
	TCOD_bsp_split_once(node,horiz,position);
	TCOD_bsp_split_recursive(TCOD_bsp_left(node),randomizer,nb-1,minHSize,minVSize,maxHRatio,maxVRatio);
	TCOD_bsp_split_recursive(TCOD_bsp_right(node),randomizer,nb-1,minHSize,minVSize,maxHRatio,maxVRatio);
}

void TCOD_bsp_resize(TCOD_bsp_t *node, int x,int y, int w, int h) {
	node->x=x;
	node->y=y;
	node->w=w;
	node->h=h;
	if ( TCOD_bsp_left(node) ) {
		if ( node->horizontal ) {
			TCOD_bsp_resize(TCOD_bsp_left(node),x,y,w,node->position-y);
			TCOD_bsp_resize(TCOD_bsp_right(node),x,node->position,w,y+h-node->position);
		} else {
			TCOD_bsp_resize(TCOD_bsp_left(node),x,y,node->position-x,h);
			TCOD_bsp_resize(TCOD_bsp_right(node),node->position,y,x+w-node->position,h);
		}
	}
}

bool TCOD_bsp_contains(TCOD_bsp_t *node, int x, int y) {
	return (x >= node->x && y >= node->y && x < node->x+node->w && y < node->y+node->h);
}

TCOD_bsp_t * TCOD_bsp_find_node(TCOD_bsp_t *node, int x, int y) {
	if ( ! TCOD_bsp_contains(node,x,y) ) return NULL;
	if ( ! TCOD_bsp_is_leaf(node) ) {
		TCOD_bsp_t *left,*right;
		left=TCOD_bsp_left(node);
		if ( TCOD_bsp_contains(left,x,y) ) return TCOD_bsp_find_node(left,x,y);
		right=TCOD_bsp_right(node);
		if ( TCOD_bsp_contains(right,x,y) ) return TCOD_bsp_find_node(right,x,y);
	}
	return node;
}



#ifndef __TILES_H__
#define __TILES_H__

#include <SDL.h>
#include <SDL_image.h>
#include "platform.h"

SDL_Surface *getTiles(int tileWidth, int tileHeight);
double prepareTile(SDL_Surface *tiles, int row, int column, boolean optimizing);
void resizeWindow(int width, int height);

#endif

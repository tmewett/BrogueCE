#ifndef __TILES_H__
#define __TILES_H__

#include <SDL.h>

void initTiles(void);
void resizeWindow(int width, int height);
void updateTile(int row, int column, short charIndex,
    short foreRed, short foreGreen, short foreBlue,
    short backRed, short backGreen, short backBlue);
void updateScreen(void);
SDL_Surface *captureScreen(void);

#endif

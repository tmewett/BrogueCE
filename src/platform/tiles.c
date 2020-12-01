#include <math.h>
#include "tiles.h"

#define PI  3.14159265358979323846

#define PNG_WIDTH    2048   // width (px) of the source PNG
#define PNG_HEIGHT   5568   // height (px) of the source PNG
#define TILE_WIDTH    128   // width (px) of a single tile in the source PNG
#define TILE_HEIGHT   232   // height (px) of a single tile in the source PNG
#define TILE_ROWS      24   // number of rows in the source PNG
#define TILE_COLS      16   // number of columns in the source PNG
#define TEXT_X_HEIGHT  96   // height (px) of the 'x' outline
#define TEXT_BASELINE  47   // height (px) of the blank space below the 'x' outline
#define MAX_TILE_SIZE  64   // maximum width or height (px) of screen tiles

// How each tile should be processed:
//  -  's' = stretch: tile stretches to fill the space
//  -  'f' = fit: preserve aspect ratio (but tile can stretch up to 20%)
//  -  't' = text: characters must line up vertically (max. stretch 40%)
static const char TileProcessing[TILE_ROWS][TILE_COLS+1] = {
    "ffffffffffffffff", "ffffffffffffffff", "tttttttttttttttt", "tttttttttttttttt",
    "tttttttttttttttt", "tttttttttttttttt", "tttttttttttttttt", "tttttttttttttttt",
    "ffffffffffffffff", "ffffffffffffffff", "tttttttttttttttt", "tttttttttttttttt",
    "tttttttttttttttt", "tttttttttttttttt", "tttttttttttttttt", "tttttttttttttttt",
    "ffsfsfsfffssssss", "ssfsfsffffffffff", "fffffffffffffsff", "ffffffffffffffff",
    "fsssfffffffffffs", "fsffffffffffffff", "ffffssssffssffff", "ffffsfffffssssff"
};


SDL_Window *Win = NULL;
SDL_Surface *WinSurf = NULL;

static SDL_Surface *Tiles[5];
static int8_t tilePadding[TILE_ROWS][TILE_COLS];
static int8_t tileShifts[TILE_ROWS][TILE_COLS][2][MAX_TILE_SIZE][3];
static boolean tileReady[TILE_ROWS][TILE_COLS][4];
static boolean tileEmpty[TILE_ROWS][TILE_COLS];

int windowWidth = 0;
int windowHeight = 0;


static void sdlfatal() {
    fprintf(stderr, "Fatal SDL error: %s\n", SDL_GetError());
    exit(1);
}


static void imgfatal() {
    fprintf(stderr, "Fatal SDL_image error: %s\n", IMG_GetError());
    exit(1);
}


static void refreshWindow() {
    WinSurf = SDL_GetWindowSurface(Win);
    if (WinSurf == NULL) sdlfatal();
    SDL_FillRect(WinSurf, NULL, SDL_MapRGB(WinSurf->format, 0, 0, 0));
    refreshScreen();
}


static int getPadding(int row, int column) {
    int padding;
    unsigned char *pixels = Tiles[0]->pixels;
    for (padding = 0; padding < TILE_HEIGHT / 4; padding++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            int y1 = padding;
            int y2 = TILE_HEIGHT - padding - 1;
            if (pixels[((x + column * TILE_WIDTH) + (y1 + row * TILE_HEIGHT) * PNG_WIDTH) * 4] > 0 ||
                pixels[((x + column * TILE_WIDTH) + (y2 + row * TILE_HEIGHT) * PNG_WIDTH) * 4] > 0)
            {
                return padding;
            }
        }
    }
    return padding;
}


static boolean isTileEmpty(int row, int column) {
    unsigned char *pixels = Tiles[0]->pixels;
    for (int y = 0; y < TILE_HEIGHT; y++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            if (pixels[((x + column * TILE_WIDTH) + (y + row * TILE_HEIGHT) * PNG_WIDTH) * 4] > 0) {
                return false;
            }
        }
    }
    return true;
}


static uint64_t xorshift64s(uint64_t *state) {
    uint64_t x = *state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	*state = x;
	return x * 0x2545f4914f6cdd1d;
}


static uint64_t noise(uint64_t *state) {
    return (xorshift64s(state) >> 54) + 0x100000280;
}


double prepareTile(SDL_Surface *tiles, int row, int column, boolean optimizing) {
    int index;
    for (index = 1; index < 5; index++) {
        if (tiles == Tiles[index]) break;
    }

    if (!optimizing) {
        if (tileReady[row][column][index - 1]) return 0;
        tileReady[row][column][index - 1] = true;
    }

    int padding = tilePadding[row][column];         // how much blank spaces there is at the top and bottom of the source tile
    char processing = TileProcessing[row][column];  // how should this tile be processed?

    // Size of the downscaled image
    const int imageWidth = Tiles[index]->w;
    const int imageHeight = Tiles[index]->h;

    // Size of downscaled tiles
    const int tileWidth = imageWidth / TILE_COLS;
    const int tileHeight = imageHeight / TILE_ROWS;

    // Size of the area the glyph must fit into
    int fitWidth = (index == 1 || index == 3 || tileWidth < 2 || optimizing ? tileWidth : tileWidth - 1);
    int fitHeight = (index == 1 || index == 2 || tileHeight < 2 || optimizing ? tileHeight : tileHeight - 1);

    // Number of sine waves that can fit in the tile (for wall tops)
    const int numHorizWaves = max(2, min(6, round(fitWidth * .25)));
    const int numVertWaves = max(2, min(11, round(fitHeight * .25)));

    // Size of the downscaled glyph
    int glyphWidth, glyphHeight;

    // accumulator for pixel values (linear color space), encoded as
    // 0xCCCCCCCCSSSSSSSS where C is a counter and S is a sum of squares
    uint64_t values[MAX_TILE_SIZE][MAX_TILE_SIZE];
    memset(values, 0, sizeof(values));
    double blur = 0;

    // if the tile is empty, we can skip the downscaling
    if (isTileEmpty(row, column)) goto downscaled;

    // decide how large we can draw the glyph
    if (processing == 's' || optimizing) {
        // stretch
        glyphWidth = fitWidth = tileWidth;
        glyphHeight = fitHeight = tileHeight;
    } else if (processing == 't') {
        // text
        glyphWidth = max(1, min(fitWidth, round(1.4 * fitHeight * TILE_WIDTH / TILE_HEIGHT)));
        glyphHeight = max(1, min(fitHeight, round(1.4 * fitWidth * TILE_HEIGHT / TILE_WIDTH)));
    } else {
        // fit
        double stretch = max(1.0, min(1.2, (double)(fitWidth * TILE_HEIGHT) / (fitHeight * TILE_WIDTH)));
        glyphWidth = max(1, min(fitWidth, round(1.2 * fitHeight * TILE_WIDTH / (TILE_HEIGHT - 2 * padding))));
        glyphHeight = max(1, min(fitHeight, round(stretch * fitWidth * (TILE_HEIGHT - 2 * padding) / TILE_WIDTH)));
    }

    // map source pixels to target pixels...
    int scaledX[TILE_WIDTH], scaledY[TILE_HEIGHT];
    int stop0, stop1, stop2, stop3, stop4;
    double map0, map1, map2, map3, map4;
    int8_t *shifts;

    // ... horizontally:

    stop0 = 0;
    stop1 = TILE_WIDTH / 5; // 20%
    stop2 = TILE_WIDTH / 2; // 50%
    stop3 = TILE_WIDTH * 4/5; // 80%
    stop4 = TILE_WIDTH;

    shifts = tileShifts[row][column][0][glyphWidth - 1];
    map0 = (fitWidth - glyphWidth) / 2;
    map1 = map0 + glyphWidth * (double)(stop1 - stop0) / (stop4 - stop0) + shifts[0] * 0.1;
    map2 = map0 + glyphWidth * (double)(stop2 - stop0) / (stop4 - stop0) + shifts[2] * 0.1;
    map3 = map0 + glyphWidth * (double)(stop3 - stop0) / (stop4 - stop0) + shifts[1] * 0.1;
    map4 = map0 + glyphWidth;

    for (int x = stop0; x < stop1; x++) scaledX[x] = map0 + (map1 - map0) * (x - stop0) / (stop1 - stop0);
    for (int x = stop1; x < stop2; x++) scaledX[x] = map1 + (map2 - map1) * (x - stop1) / (stop2 - stop1);
    for (int x = stop2; x < stop3; x++) scaledX[x] = map2 + (map3 - map2) * (x - stop2) / (stop3 - stop2);
    for (int x = stop3; x < stop4; x++) scaledX[x] = map3 + (map4 - map3) * (x - stop3) / (stop4 - stop3);

    // ... vertically:

    if (processing == 't') {
        stop4 = TILE_HEIGHT;
        stop3 = stop4 - TEXT_BASELINE;
        stop2 = stop3 - TEXT_X_HEIGHT;
        stop1 = stop2 / 3;
        stop0 = 0;
    } else {
        stop0 = padding;
        stop4 = TILE_HEIGHT - padding;
        stop1 = stop0 + (stop4 - stop0) / 5;    // 20%
        stop2 = stop0 + (stop4 - stop0) / 2;    // 50%
        stop3 = stop0 + (stop4 - stop0) * 4/5;  // 80%
    }

    map0 = (fitHeight - glyphHeight) / 2;
    map1 = map0 + glyphHeight * (double)(stop1 - stop0) / (stop4 - stop0);
    map2 = map0 + glyphHeight * (double)(stop2 - stop0) / (stop4 - stop0);
    map3 = map0 + glyphHeight * (double)(stop3 - stop0) / (stop4 - stop0);
    map4 = map0 + glyphHeight;

    if (processing == 't') {
        // align stops #2 and #3 with output pixels
        map3 += round(map2) - map2;
        map2 = round(map2);
        map3 = max(map2 + 1, round(map3));
        map1 = map0 + (map2 - map0) / 3;
    }

    shifts = tileShifts[row][column][1][glyphHeight - 1];
    map1 += shifts[0] * 0.1;
    map2 += shifts[2] * 0.1;
    map3 += shifts[1] * 0.1;

    for (int y = 0; y < stop0; y++) scaledY[y] = -1; // not mapped
    for (int y = stop0; y < stop1; y++) scaledY[y] = map0 + (map1 - map0) * (y - stop0) / (stop1 - stop0);
    for (int y = stop1; y < stop2; y++) scaledY[y] = map1 + (map2 - map1) * (y - stop1) / (stop2 - stop1);
    for (int y = stop2; y < stop3; y++) scaledY[y] = map2 + (map3 - map2) * (y - stop2) / (stop3 - stop2);
    for (int y = stop3; y < stop4; y++) scaledY[y] = map3 + (map4 - map3) * (y - stop3) / (stop4 - stop3);
    for (int y = stop4; y < TILE_HEIGHT; y++) scaledY[y] = -1; // not mapped

    // downscale source tile to accumulator
    for (int y = 0; y < TILE_HEIGHT; y++) {
        int targetY = scaledY[y];
        if (targetY < 0 || targetY >= MAX_TILE_SIZE) continue;
        uint64_t *target = values[targetY];
        unsigned char *pixel = Tiles[0]->pixels;
        pixel += ((column * TILE_WIDTH) + (row * TILE_HEIGHT + y) * PNG_WIDTH) * 4;
        for (int x = 0; x < TILE_WIDTH; x++) {
            uint64_t value = pixel[x*4];
            target[scaledX[x]] += value * value  // (gamma 2.0)
                                  | 0x100000000; // (count = 1)
        }
    }
    downscaled:

    // add floor dust (if the floor tile is blank)
    if (row == 20 && column == 2 && isTileEmpty(row, column) && tileWidth > 2 && tileHeight > 2 && !optimizing) {
        int w = tileWidth - 2;
        int h = tileHeight - 2;
        uint64_t state = 1234567;
        uint16_t idx[MAX_TILE_SIZE * MAX_TILE_SIZE];

        // stitch edges together
        for (int x = 0; x < w; x += 4) values[0][x] = noise(&state);
        for (int y = 0; y < h; y += 4) values[y][0] = noise(&state);
        for (int x = 2; x < w; x += 4) values[h+1][x] = noise(&state);
        for (int y = 2; y < h; y += 4) values[y][w+1] = noise(&state);

        // fill center with isolated dots, randomly placed
        for (int i = 0; i < w * h; i++) idx[i] = i;             // array of indexes
        for (int i = 0; i < w * h - 1; i++) {                   // shuffle the array
            int j = xorshift64s(&state) % (w * h - i) + i;      // Fisher–Yates shuffle
            uint64_t t = idx[i]; idx[i] = idx[j]; idx[j] = t;   // swap indexes
        }
        for (int i = 0; i < w * h; i++) {
            int x = 1 + (idx[i] % w);
            int y = 1 + (idx[i] / w);
            if (!values[y][x+1] && !values[y][x-1] && !values[y+1][x+1] && !values[y-1][x-1] &&
                !values[y+1][x] && !values[y-1][x] & !values[y+1][x-1] && !values[y-1][x+1]) {
                values[y][x] = noise(&state);
            }
        }
    }

    // add wall tops: diagonal sine waves
    if ((row == 16 && column == 2 || row == 21 && column == 1 || row == 22 && column == 4) && !optimizing) {
        for (int y = 0; y < tileHeight; y++) {
            if (row != 21 && (y > tileHeight / 2 || (values[y][0] & 0xFFFFFFFF))) break;
            for (int x = 0; x < tileWidth; x++) {
                double value = sin(2. * PI * ((double)x / tileWidth * numHorizWaves
                                            + (double)y / tileHeight * numVertWaves)) / 2. + 0.5;
                values[y][x] = (uint64_t)(255 * 255 * value * value) | 0x100000000;
            }
        }
    }

    // convert accumulator to image transparency
    for (int y = 0; y < tileHeight; y++) {
        unsigned char *pixel = Tiles[index]->pixels;
        pixel += ((column * tileWidth) + (row * tileHeight + y) * imageWidth) * 4;
        for (int x = 0; x < tileWidth; x++) {
            uint64_t value = values[y][x];

            // average light intensity (linear scale, 0 .. 255*255)
            value = (value ? (value & 0xFFFFFFFF) / (value >> 32) : 0);

            // metric for "blurriness": black (0) and white (255*255) pixels count for 0, gray pixels for 1
            if (optimizing) blur += sin(PI/(255*255) * value);

            // pixel value (gamma-compressed, 0 .. 255)
            value = (value == 0 ? 0 : value == 255*255 ? 255 : round(sqrt(value)));

            *pixel++ = 255;
            *pixel++ = 255;
            *pixel++ = 255;
            *pixel++ = value;
        }
    }

    return blur; // (used by the optimizer)
}


static void optimizeTiles() {
    for (int row = 0; row < TILE_ROWS; row++) {
        for (int column = 0; column < TILE_COLS; column++) {
            if (tileEmpty[row][column]) continue;
            fprintf(stderr, "Optimizing tile %d / %d ...\n", row * TILE_COLS + column + 1, TILE_ROWS * TILE_COLS);

            // horizontal shifts
            for (int i = 0; i < 3; i++) {
                for (int width = 5; width <= MAX_TILE_SIZE; width++) {
                    int8_t *shifts = tileShifts[row][column][0][width - 1];
                    int height = 64;
                    Tiles[1] = SDL_CreateRGBSurfaceWithFormat(0, width * TILE_COLS, height * TILE_ROWS, 32, SDL_PIXELFORMAT_RGBA32);

                    for (int idx = 0; idx < (TileProcessing[row][column] == 't' ? 2 : 3); idx++) {
                        double bestResult = 1e20;
                        int8_t bestShift = 0;
                        int8_t minShift = (idx < 2 ? -5 : (shifts[0] + shifts[1]) / 2 - 5);
                        for (int8_t shift = minShift; shift < minShift + 11; shift++) {
                            shifts[idx] = shift;
                            double blur = prepareTile(Tiles[1], row, column, true);
                            if (blur < bestResult) {
                                bestResult = blur;
                                bestShift = shift;
                            }
                        }
                        shifts[idx] = bestShift;
                    }

                    SDL_FreeSurface(Tiles[1]);
                    Tiles[1] = NULL;
                }
            }

            // vertical shifts
            for (int i = 0; i < 3; i++) {
                for (int height = 7; height <= MAX_TILE_SIZE; height++) {
                    int8_t *shifts = tileShifts[row][column][1][height - 1];
                    int width = 64;
                    Tiles[1] = SDL_CreateRGBSurfaceWithFormat(0, width * TILE_COLS, height * TILE_ROWS, 32, SDL_PIXELFORMAT_RGBA32);

                    for (int idx = 0; idx < (TileProcessing[row][column] == 't' ? 1 : 3); idx++) {
                        double bestResult = 1e20;
                        int8_t bestShift = 0;
                        int8_t minShift = (idx < 2 ? -5 : (shifts[0] + shifts[1]) / 2 - 5);
                        for (int8_t shift = minShift; shift < minShift + 11; shift++) {
                            shifts[idx] = shift;
                            double blur = prepareTile(Tiles[1], row, column, true);
                            if (blur < bestResult) {
                                bestResult = blur;
                                bestShift = shift;
                            }
                        }
                        shifts[idx] = bestShift;
                    }

                    SDL_FreeSurface(Tiles[1]);
                    Tiles[1] = NULL;
                }
            }
        }
    }
}


static void loadTiles() {

    if (Tiles[0] == NULL) {

        // load the large PNG
        char filename[BROGUE_FILENAME_MAX];
        sprintf(filename, "%s/assets/tiles.png", dataDirectory);
        SDL_Surface *image = IMG_Load(filename);
        if (!image) imgfatal();
        Tiles[0] = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(image);

        // measure padding
        for (int row = 0; row < TILE_ROWS; row++) {
            for (int column = 0; column < TILE_COLS; column++) {
                tileEmpty[row][column] = isTileEmpty(row, column);
                tilePadding[row][column] = (TileProcessing[row][column] == 'f' ? getPadding(row, column) : 0);
            }
        }

        // load shifts
        sprintf(filename, "%s/assets/tiles.bin", dataDirectory);
        FILE *file = fopen(filename, "rb");
        if (file) {
            fread(tileShifts, 1, sizeof(tileShifts), file);
            fclose(file);
        } else {
            optimizeTiles();
            file = fopen(filename, "wb");
            fwrite(tileShifts, 1, sizeof(tileShifts), file);
            fclose(file);
        }
    }

    // The original image will be resized to 4 possible sizes:
    //  1.  N x M pixels
    //  2.  (N+1) x M pixels
    //  3.  N x (M+1) pixels
    //  4.  (N+1) x (M+1) pixels
    // The first three may be set to NULL if either dimension is 0.
    for (int i = 1; i < 5; i++) {
        if (Tiles[i]) SDL_FreeSurface(Tiles[i]);
        Tiles[i] = NULL;
        int imageWidth = (windowWidth / COLS + (i == 2 || i == 4 ? 1 : 0)) * TILE_COLS;
        int imageHeight = (windowHeight / ROWS + (i == 3 || i == 4 ? 1 : 0)) * TILE_ROWS;
        if (imageWidth == 0 || imageHeight == 0) continue;
        Tiles[i] = SDL_CreateRGBSurfaceWithFormat(0, imageWidth, imageHeight, 32, SDL_PIXELFORMAT_RGBA32);
    }

    memset(tileReady, 0, sizeof(tileReady));
}


/*
Creates or resizes the game window with the currently loaded font.
*/
void resizeWindow(int width, int height) {

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);
    boolean fullScreen = false;

    if (!width) width = mode.w * 7/10;  // 70% of monitor by default
    if (!height) height = mode.h * 7/10;
    if (width >= MAX_TILE_SIZE * COLS) width = MAX_TILE_SIZE * COLS;  // this is large enough for 4K resolution
    if (height >= MAX_TILE_SIZE * ROWS) height = MAX_TILE_SIZE * ROWS;
    if (width >= mode.w && height >= mode.h) {  // go to fullscreen mode if window matches monitor
        width = mode.w;
        height = mode.h;
        fullScreen = true;
    }
    if (Win != NULL && width == windowWidth && height == windowHeight) {
        // the window has the requested size already
        refreshWindow();
        return;
    }

    // create the window
    if (Win == NULL) {
        Win = SDL_CreateWindow("Brogue",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
        if (Win == NULL) sdlfatal();

        char filename[BROGUE_FILENAME_MAX];
        sprintf(filename, "%s/assets/icon.png", dataDirectory);
        SDL_Surface *icon = IMG_Load(filename);
        if (icon == NULL) imgfatal();
        SDL_SetWindowIcon(Win, icon);
        SDL_FreeSurface(icon);
    }

    if (fullScreen) {
        if (!(SDL_GetWindowFlags(Win) & SDL_WINDOW_FULLSCREEN_DESKTOP)) {
            // switch to fullscreen mode
            SDL_SetWindowFullscreen(Win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
    } else {
        if (SDL_GetWindowFlags(Win) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            // switch to windowed mode
            SDL_SetWindowFullscreen(Win, 0);
        } else {
            // what is the current size?
            SDL_GetWindowSize(Win, &windowWidth, &windowHeight);
            if (windowWidth != width || windowHeight != height) {
                // resize the window
                SDL_SetWindowSize(Win, width, height);
                SDL_RestoreWindow(Win);
            }
        }
    }

    // prepare tiles for window's new resolution
    SDL_GetWindowSize(Win, &windowWidth, &windowHeight);
    loadTiles();
    refreshWindow();
}


SDL_Surface *getTiles(int tileWidth, int tileHeight) {
    for (int i = 1; i < 5; i ++) {
        if (Tiles[i] != NULL && Tiles[i]->w == tileWidth * TILE_COLS && Tiles[i]->h == tileHeight * TILE_ROWS) {
            return Tiles[i];
        }
    }
    return NULL;
}

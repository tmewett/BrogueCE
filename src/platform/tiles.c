#include <math.h>
#include <stdlib.h>
#include <SDL_image.h>
#include "platform.h"
#include "tiles.h"

#define PI  3.14159265358979323846

#define PNG_WIDTH    2048   // width (px) of the source PNG
#define PNG_HEIGHT   5568   // height (px) of the source PNG
#define TILE_WIDTH    128   // width (px) of a single tile in the source PNG
#define TILE_HEIGHT   232   // height (px) of a single tile in the source PNG
#define TILE_ROWS      24   // number of rows in the source PNG
#define TILE_COLS      16   // number of columns in the source PNG
#define TEXT_X_HEIGHT 100   // height (px) of the 'x' outline
#define TEXT_BASELINE  46   // height (px) of the blank space below the 'x' outline
#define MAX_TILE_SIZE  64   // maximum width or height (px) of screen tiles before we switch to linear interpolation


// How each tile should be processed:
//  -  's' = stretch: tile stretches to fill the space
//  -  'f' = fit: preserve aspect ratio (but tile can stretch up to 20%)
//  -  't' = text: characters must line up vertically (max. stretch 40%)
//  -  '#' = symbols: other Unicode characters (max. stretch 40%)
static const char TileProcessing[TILE_ROWS][TILE_COLS+1] = {
    "ffffffffffffffff", "ssssssssssssssss", "#t##########t#t#", "tttttttttttt###t",
    "#ttttttttttttttt", "ttttttttttt#####", "#ttttttttttttttt", "ttttttttttt#####",
    "################", "################", "################", "################",
    "tttttttttttttttt", "ttttttt#tttttttt", "tttttttttttttttt", "ttttttt#tttttttt",
    "ffsfsfsffsssssss", "ssfsfsffffffffff", "fffffffffffffsff", "ffffffffffffffff",
    "fsssfffffffffffs", "fsffffffffffffff", "ffffssssffssffff", "ffffsfffffssssff"
};

typedef struct ScreenTile {
    short foreRed, foreGreen, foreBlue; // foreground color (0..100)
    short backRed, backGreen, backBlue; // background color (0..100)
    short charIndex;    // index of the glyph to draw
    short needsRefresh; // true if the tile has changed since the last screen refresh, else false
} ScreenTile;

static SDL_Window *Win = NULL;      // the SDL window
static SDL_Surface *TilesPNG;       // source PNG
static SDL_Texture *Textures[4];    // textures used by the renderer to draw tiles
static int numTextures = 0;         // how many textures are available in `Textures`
static int8_t tilePadding[TILE_ROWS][TILE_COLS];  // how many black lines are at the top/bottom of each tile in the source PNG
static boolean tileEmpty[TILE_ROWS][TILE_COLS];   // true if a tile is completely black in the source PNG, else false

// How much should a corner of a tile be shifted by before its downscaling, to improve its sharpness.
// The first two dimensions are the tile's coordinates (row and column, both zero-based).
// The third dimension is either 0 for shifts along the horizontal axis, or 1 for the vertical axis.
// The fourth dimention is the downscaling operation's target size (width for horizontal axis, height for vertical).
// The last dimension is 0 for top/left, 1 for bottom/right, 2 for center.
// The values stored in tileShifts are signed integers. Unit is 1/10th of a pixel.
static int8_t tileShifts[TILE_ROWS][TILE_COLS][2][MAX_TILE_SIZE][3];

static ScreenTile screenTiles[ROWS][COLS];  // buffer for the expected contents of the screen
static int baseTileWidth = -1;      // width (px) of tiles in the smallest texture (`Textures[0]`)
static int baseTileHeight = -1;     // height (px) of tiles in the smallest texture (`Textures[0]`)


int windowWidth = -1;               // the SDL window's width (in "screen units", not pixels)
int windowHeight = -1;              // the SDL window's height (in "screen units", not pixels)
boolean fullScreen = false;         // true if the window should be full-screen, else false
boolean softwareRendering = false;  // true if hardware acceleration is disabled (by choice or by force)


/// Prints the fatal error message provided by SDL then closes the app.
static void sdlfatal(char *file, int line) {
    fprintf(stderr, "Fatal SDL error (%s:%d): %s\n", file, line, SDL_GetError());
    exit(EXIT_STATUS_FAILURE_PLATFORM_ERROR);
}


/// Prints the fatal error message provided by SDL_image then closes the app.
static void imgfatal(char *file, int line) {
    fprintf(stderr, "Fatal SDL_image error (%s:%d): %s\n", file, line, IMG_GetError());
    exit(EXIT_STATUS_FAILURE_PLATFORM_ERROR);
}


#if !SDL_VERSION_ATLEAST(2, 0, 5)

SDL_Surface *SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format) {
    Uint32 r, g, b, a;
    if (!SDL_PixelFormatEnumToMasks(format, &depth, &r, &g, &b, &a)) sdlfatal(__FILE__, __LINE__);
    return SDL_CreateRGBSurface(flags, width, height, depth, r, g, b, a);
}

#endif


/// Returns the numbers of black lines at the top and bottom of a given glyph in the source PNG.
///
/// For example, if the glyph has 30 black lines at the top and 40 at the bottom, the function
/// returns 30 (the least of the two).
///
/// In case the glyph is very small, the function never returns more than TILE_HEIGHT / 4.
/// This is to avoid drawing the glyph more than twice its size relative to other glyphs
/// when the window's aspect ratio is very large (super wide).
///
static int getPadding(int row, int column) {
    int padding;
    Uint32 *pixels = TilesPNG->pixels; // each pixel is encoded as 0xffRRGGBB
    for (padding = 0; padding < TILE_HEIGHT / 4; padding++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            int y1 = padding;
            int y2 = TILE_HEIGHT - padding - 1;
            if (pixels[(x + column * TILE_WIDTH) + (y1 + row * TILE_HEIGHT) * PNG_WIDTH] & 0xffffffU ||
                pixels[(x + column * TILE_WIDTH) + (y2 + row * TILE_HEIGHT) * PNG_WIDTH] & 0xffffffU)
            {
                return padding;
            }
        }
    }
    return padding;
}


/// Tells if a tile is completely empty (black) in the source PNG.
static boolean isTileEmpty(int row, int column) {
    Uint32 *pixels = TilesPNG->pixels; // each pixel is encoded as 0xffRRGGBB
    for (int y = 0; y < TILE_HEIGHT; y++) {
        for (int x = 0; x < TILE_WIDTH; x++) {
            if (pixels[(x + column * TILE_WIDTH) + (y + row * TILE_HEIGHT) * PNG_WIDTH] & 0xffffffU) {
                return false;
            }
        }
    }
    return true;
}


/// Downscales a tile to the specified size.
///
/// The downscaling is performed in linear color space, rather than in gamma-compressed space which would cause
/// dimming (average of a black pixel and a white pixel should be 50% grey = 0.5 luminance = 187 sRGB, not 128).
/// To simplify the computation, we assume a gamma of 2.0.
///
/// The tile is logically split into 16 parts (sliced by 3 vertical lines and 3 horizontal ones), so that:
///
///  -  the four corners can be positioned to preserve the tile's aspect ratio,
///     possibly filling the blank spaces at the top and bottom;
///  -  the top, bottom, left, right, and center areas can be independently aligned
///     with output pixels to improve sharpness. This is achieved by adding pre-computed
///     sub-pixel shifts to the target coordinates as we map source pixels to target pixels.
///
/// Some tiles, like walls and doors, always stretch to fill the space regardless of aspect ratio.
/// Other tiles must preserve aspect ratio, but we allow up to 20% stretch for graphic tiles and
/// 40% for text tiles.
///
/// For text tiles (letters, digits, punctuation signs), the characters' x-height and baseline
/// (corresponding to the top and bottom of "x") are pixel-aligned regardless of outlines.
/// This ensures that all letters neatly line up without jumping up and down.
/// We also reduce perceived boldness by applying a custom brightness curve;
/// it makes the text more legible at the smaller sizes.
///
/// Wall tops are diagonal sine waves, approximately 4 pixels apart.
///
/// \param surface the target surface
/// \param tileWidth width (px) of tiles in the target surface
/// \param tileHeight height (px) of tiles in the target surface
/// \param row row (zero-based) on which the tile is located in both the source PNG and the target surface
/// \param column column (zero-based) in which the tile is located in both the source PNG and the target surface
/// \param optimizing pass true when optimizing tiles, else false
/// \return estimated amount of blur in the resulting tile (when optimizing)
///
static double downscaleTile(SDL_Surface *surface, int tileWidth, int tileHeight, int row, int column, boolean optimizing) {
    int8_t noShifts[3] = {0, 0, 0};
    int padding = tilePadding[row][column];         // how much blank spaces there is at the top and bottom of the source tile
    char processing = TileProcessing[row][column];  // how should this tile be processed?

    // Size of the area the glyph must fit into
    int fitWidth = max(1, baseTileWidth);
    int fitHeight = max(1, baseTileHeight);

    // Number of sine waves that can fit in the tile (for wall tops)
    const int numHorizWaves = max(2, min(6, round(fitWidth * .25)));
    const int numVertWaves = max(2, min(11, round(fitHeight * .25)));

    // Size of the downscaled glyph
    int glyphWidth, glyphHeight;

    // accumulator for pixel values (linear color space), encoded as
    // 0xCCCCCCCCSSSSSSSS where C is a counter and S is a sum of squares
    uint64_t *values = malloc(tileWidth * tileHeight * sizeof(uint64_t));
    memset(values, 0, tileWidth * tileHeight * sizeof(uint64_t));
    double blur = 0;

    // if the tile is empty, we can skip the downscaling
    if (tileEmpty[row][column]) goto downscaled;

    // decide how large we can draw the glyph
    if (processing == 's' || optimizing) {
        // stretch
        glyphWidth = fitWidth = tileWidth;
        glyphHeight = fitHeight = tileHeight;
    } else if (processing == 'f') {
        // fit
        int hi = fitHeight * TILE_HEIGHT / (TILE_HEIGHT - 2 * padding);
        int lo = max(1, min(fitHeight, round(1.2 * fitWidth * TILE_HEIGHT / TILE_WIDTH)));
        glyphHeight = max(lo, min(hi, round((double)fitWidth * TILE_HEIGHT / TILE_WIDTH)));
        glyphWidth = max(1, min(fitWidth, round(1.2 * glyphHeight * TILE_WIDTH / TILE_HEIGHT)));
    } else {
        // text
        glyphWidth = max(1, min(fitWidth, round(1.4 * fitHeight * TILE_WIDTH / TILE_HEIGHT)));
        glyphHeight = max(1, min(fitHeight, round(1.4 * fitWidth * TILE_HEIGHT / TILE_WIDTH)));
    }

    // map source pixels to target pixels...
    int scaledX[TILE_WIDTH], scaledY[TILE_HEIGHT];
    int stop0, stop1, stop2, stop3, stop4;
    double map0, map1, map2, map3, map4;
    int8_t *shifts;

    // ... horizontally:

    // horizontal coordinates on the source tile for the left border (stop0), 3 taps (stop1, 2, 3), and right border (stop4)
    stop0 = 0;
    stop1 = TILE_WIDTH / 5;   // 20%
    stop2 = TILE_WIDTH / 2;   // 50%
    stop3 = TILE_WIDTH * 4/5; // 80%
    stop4 = TILE_WIDTH;

    // corresponding coordinates on the target tile, taking into account centering and sub-pixel positioning
    shifts = (glyphWidth > MAX_TILE_SIZE ? noShifts : tileShifts[row][column][0][glyphWidth - 1]);
    map0 = (fitWidth - glyphWidth + (shifts[0] + shifts[1] < 0 ? 1 : 0)) / 2;
    map1 = map0 + glyphWidth * (double)(stop1 - stop0) / (stop4 - stop0) + shifts[0] * 0.1;
    map2 = map0 + glyphWidth * (double)(stop2 - stop0) / (stop4 - stop0) + shifts[2] * 0.1;
    map3 = map0 + glyphWidth * (double)(stop3 - stop0) / (stop4 - stop0) + shifts[1] * 0.1;
    map4 = map0 + glyphWidth;

    // now we can interpolate the horizontal coordinates for all pixels
    for (int x = stop0; x < stop1; x++) scaledX[x] = map0 + (map1 - map0) * (x - stop0) / (stop1 - stop0);
    for (int x = stop1; x < stop2; x++) scaledX[x] = map1 + (map2 - map1) * (x - stop1) / (stop2 - stop1);
    for (int x = stop2; x < stop3; x++) scaledX[x] = map2 + (map3 - map2) * (x - stop2) / (stop3 - stop2);
    for (int x = stop3; x < stop4; x++) scaledX[x] = map3 + (map4 - map3) * (x - stop3) / (stop4 - stop3);

    // ... vertically:

    if (processing == 't') {
        // vertical coordinates on the source tile for the top edge (stop0), stem (stop1), "x" (stop2, stop3), and bottom edge (stop4)
        stop4 = TILE_HEIGHT;
        stop3 = stop4 - TEXT_BASELINE;
        stop2 = stop3 - TEXT_X_HEIGHT;
        stop1 = stop2 / 3;
        stop0 = 0;
    } else {
        // vertical coordinates on the source tile for the top edge (stop0), 3 taps (stop1, 2, 3), and bottom edge (stop4)
        stop0 = 0;
        stop1 = TILE_HEIGHT / 5;   // 20%
        stop2 = TILE_HEIGHT / 2;   // 50%
        stop3 = TILE_HEIGHT * 4/5; // 80%
        stop4 = TILE_HEIGHT;
    }

    // corresponding coordinates on the target tile, taking into account centering
    map0 = (fitHeight - glyphHeight) / 2;
    map1 = map0 + glyphHeight * (double)(stop1 - stop0) / (stop4 - stop0);
    map2 = map0 + glyphHeight * (double)(stop2 - stop0) / (stop4 - stop0);
    map3 = map0 + glyphHeight * (double)(stop3 - stop0) / (stop4 - stop0);
    map4 = map0 + glyphHeight;

    // for text tiles, we must exactly align stops #2 and #3 with output pixels
    if (processing == 't') {
        map3 += round(map2) - map2;
        map2 = round(map2);
        map3 = max(map2 + 1, round(map3));
        map1 = map0 + (map2 - map0) / 3;
    }

    // now add sub-pixel positioning (for text tiles, we have shifts[1] == shifts[2] == 0)
    shifts = (glyphHeight > MAX_TILE_SIZE ? noShifts : tileShifts[row][column][1][glyphHeight - 1]);
    map1 += shifts[0] * 0.1;
    map2 += shifts[2] * 0.1;
    map3 += shifts[1] * 0.1;

    // finally we can interpolate the vertical coordinates for all pixels
    for (int y = 0; y < stop0; y++) scaledY[y] = -1; // not mapped (can happen with fitted tiles)
    for (int y = stop0; y < stop1; y++) scaledY[y] = map0 + (map1 - map0) * (y - stop0) / (stop1 - stop0);
    for (int y = stop1; y < stop2; y++) scaledY[y] = map1 + (map2 - map1) * (y - stop1) / (stop2 - stop1);
    for (int y = stop2; y < stop3; y++) scaledY[y] = map2 + (map3 - map2) * (y - stop2) / (stop3 - stop2);
    for (int y = stop3; y < stop4; y++) scaledY[y] = map3 + (map4 - map3) * (y - stop3) / (stop4 - stop3);
    for (int y = stop4; y < TILE_HEIGHT; y++) scaledY[y] = -1; // not mapped (can happen with fitted tiles)

    // downscale source tile to accumulator
    for (int y0 = 0; y0 < TILE_HEIGHT; y0++) {
        int y1 = scaledY[y0];
        if (y1 < 0 || y1 >= tileHeight) continue;
        uint64_t *dst = &values[y1 * tileWidth];
        Uint32 *src = TilesPNG->pixels; // each pixel is encoded as 0xffRRGGBB
        src += (column * TILE_WIDTH) + (row * TILE_HEIGHT + y0) * PNG_WIDTH;
        for (int x0 = 0; x0 < TILE_WIDTH; x0++) {
            uint64_t value = src[x0] & 0xffU;
            dst[scaledX[x0]] += (value * value) | 0x100000000U; // (gamma = 2.0, count = 1)
        }
        // interpolate skipped lines, if any
        if (y1 >= 2 && y0 >= 1 && scaledY[y0 - 1] == y1 - 2) {
            for (int x1 = 0; x1 < tileWidth; x1++) {
                dst[x1 - tileWidth] = dst[x1 - 2*tileWidth] + dst[x1];
            }
        }
    }
    downscaled:

    // procedural wall tops: diagonal sine waves
    if ((row == 16 && column == 2 || row == 21 && column == 1 || row == 22 && column == 4) && !optimizing) {
        for (int y = 0; y < tileHeight; y++) {
            if (row != 21 && (y > tileHeight / 2 || (values[y * tileWidth] & 0xffffffffU))) break;
            for (int x = 0; x < tileWidth; x++) {
                double value = sin(2. * PI * ((double)x / tileWidth * numHorizWaves
                                            + (double)y / tileHeight * numVertWaves)) / 2. + 0.5;
                values[y * tileWidth + x] = (uint64_t)round(255 * 255 * value * value) | 0x100000000U;
            }
        }
    }

    // convert accumulator to image transparency
    for (int y = 0; y < tileHeight; y++) {
        Uint32 *pixel = surface->pixels; // each pixel is encoded as 0xAARRGGBB
        pixel += (column * tileWidth) + (row * tileHeight + y) * surface->w;
        for (int x = 0; x < tileWidth; x++) {
            uint64_t value = values[y * tileWidth + x];

            // average light intensity (linear scale, 0 .. 255*255)
            value = ((value >> 32) ? (value & 0xffffffffU) / (value >> 32) : 0);

            // metric for "blurriness": black (0) and white (255*255) pixels count for 0, gray pixels for 1
            if (optimizing) blur += sin(PI/(255*255) * value);

            // make text look less bold, at the cost of accuracy
            if (processing == 't' || processing == '#') {
                value = (value < 255*255/2 ? value / 2 : value * 3/2 - 255*255/2);
            }

            // opacity (gamma-compressed, 0 .. 255)
            uint32_t alpha = (value == 0 ? 0 : value > 64770 ? 255 : round(sqrt(value)));

            *pixel++ = (alpha << 24) | 0xffffffU;
        }
    }

    free(values);
    return blur; // (used by the optimizer)
}


/// Finds the best possible sub-pixel alignments of tiles for their downscaling at every possible size.
/// Results are recorded into `tileShifts`.
///
/// This is a slow function (takes ~2 minutes) so the results are saved to disk and reloaded when Brogue starts.
/// After you modify the PNG, you should also delete "tiles.bin" and run Brogue so that the new tiles get optimized.
static void optimizeTiles() {
    SDL_Window *window = SDL_CreateWindow("Brogue", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 300, 0);

    for (int row = 0; row < TILE_ROWS; row++) {
        for (int column = 0; column < TILE_COLS; column++) {
            if (tileEmpty[row][column]) continue;
            char processing = TileProcessing[row][column];

            // show what we are doing
            char title[100];
            sprintf(title, "Brogue - Optimizing tile %d / %d ...\n", row * TILE_COLS + column + 1, TILE_ROWS * TILE_COLS);
            SDL_SetWindowTitle(window, title);
            SDL_Surface *winSurface = SDL_GetWindowSurface(window);
            if (!winSurface) sdlfatal(__FILE__, __LINE__);
            if (SDL_BlitSurface(TilesPNG, &(SDL_Rect){.x=column*TILE_WIDTH, .y=row*TILE_HEIGHT, .w=TILE_WIDTH, .h=TILE_HEIGHT},
                    winSurface, &(SDL_Rect){.x=0, .y=0, .w=TILE_WIDTH, .h=TILE_HEIGHT}) < 0) sdlfatal(__FILE__, __LINE__);
            if (SDL_UpdateWindowSurface(window) < 0) sdlfatal(__FILE__, __LINE__);

            // detect closing the window
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    SDL_Quit();
                    fprintf(stderr, "Aborted.\n");
                    exit(EXIT_STATUS_FAILURE_PLATFORM_ERROR);
                }
            }

            // horizontal shifts
            baseTileHeight = MAX_TILE_SIZE;
            for (baseTileWidth = 5; baseTileWidth <= MAX_TILE_SIZE; baseTileWidth++) {
                int8_t *shifts = tileShifts[row][column][0][baseTileWidth - 1];
                SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, baseTileWidth * TILE_COLS, baseTileHeight * TILE_ROWS, 32, SDL_PIXELFORMAT_ARGB8888);
                if (!surface) sdlfatal(__FILE__, __LINE__);

                for (int i = 0; i < 3; i++) {
                    for (int idx = 0; idx < (processing == 't' || processing == '#' ? 2 : 3); idx++) {
                        double bestResult = 1e20;
                        int8_t bestShift = 0;
                        int8_t midShift = (idx == 2 ? (shifts[0] + shifts[1]) / 2 : 0);
                        for (int8_t shift = midShift - 5; shift <= midShift + 5; shift++) {
                            shifts[idx] = shift;
                            if (processing == 't' || processing == '#') {
                                shifts[2] = (shifts[0] + shifts[1]) / 2;
                            }
                            double blur = downscaleTile(surface, baseTileWidth, baseTileHeight, row, column, true);
                            if (blur < bestResult) {
                                bestResult = blur;
                                bestShift = shift;
                            }
                        }
                        shifts[idx] = bestShift;
                        if (processing == 't' || processing == '#') {
                            shifts[2] = (shifts[0] + shifts[1]) / 2;
                        }
                    }
                }

                SDL_FreeSurface(surface);
            }

            // vertical shifts
            baseTileWidth = MAX_TILE_SIZE;
            for (baseTileHeight = 7; baseTileHeight <= MAX_TILE_SIZE; baseTileHeight++) {
                int8_t *shifts = tileShifts[row][column][1][baseTileHeight - 1];
                SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, baseTileWidth * TILE_COLS, baseTileHeight * TILE_ROWS, 32, SDL_PIXELFORMAT_ARGB8888);
                if (!surface) sdlfatal(__FILE__, __LINE__);

                for (int i = 0; i < 3; i++) {
                    for (int idx = 0; idx < (processing == 't' ? 1 : 3); idx++) {
                        double bestResult = 1e20;
                        int8_t bestShift = 0;
                        int8_t midShift = (idx == 2 ? (shifts[0] + shifts[1]) / 2 : 0);
                        for (int8_t shift = midShift - 5; shift <= midShift + 5; shift++) {
                            shifts[idx] = shift;
                            double blur = downscaleTile(surface, baseTileWidth, baseTileHeight, row, column, true);
                            if (blur < bestResult) {
                                bestResult = blur;
                                bestShift = shift;
                            }
                        }
                        shifts[idx] = bestShift;
                    }
                }

                SDL_FreeSurface(surface);
            }
        }
    }
    SDL_DestroyWindow(window);
}


/// Loads the PNG and analyses it.
void initTiles() {
    char filename[BROGUE_FILENAME_MAX];
    sprintf(filename, "%s/assets/tiles.png", dataDirectory);

    // are we running Brogue from the correct folder to begin with?
    if (!fileExists(filename)) {
        fprintf(stderr, "Error: \"%s\" not found!\n", filename);
        exit(EXIT_STATUS_FAILURE_PLATFORM_ERROR);
    }

    // load the large PNG
    SDL_Surface *image = IMG_Load(filename);
    if (!image) imgfatal(__FILE__, __LINE__);
    TilesPNG = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_ARGB8888, 0);
    if (!TilesPNG) sdlfatal(__FILE__, __LINE__);
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
        fprintf(stderr, "\"%s\" not found. Re-generating it...\n", filename);
        optimizeTiles();
        file = fopen(filename, "wb");
        if (!file) {
            fprintf(stderr, "Error: could not write to \"%s\"\n", filename);
            exit(EXIT_STATUS_FAILURE_PLATFORM_ERROR);
        }
        fwrite(tileShifts, 1, sizeof(tileShifts), file);
        fclose(file);
    }
}


/// Creates the textures to fit a specific output size
/// (which is equal to the window size on standard DPI displays, but can be larger on HiDPI).
///
/// We build up to 4 textures, with different tile sizes. Each texture has all the tiles on it.
/// The first texture (`Textures[0]`) is always present, and is always the smallest of the bunch.
///
/// The reason for having tiles of different sizes ready to be drawn on screen is that the window
/// width is usually not a multiple of 100 (the `COLS` constant), and height not a multiple of 34
/// (`ROWS`). Since tiles must have integer dimensions, that means some tiles must be larger by
/// 1 pixel than others, so that we can cover the window without black padding on the sides nor
/// columns/rows of blank pixels between tiles.
///
/// If the window is so large that tiles would have to be over 64x64 pixels, we generate a single, large
/// texture instead of four and use it for all tiles, allowing the renderer to do some linear interpolation.
///
/// To ensure compatibility with older OpenGL drivers, texture dimensions are always powers of 2.
///
/// \param outputWidth renderer's output width
/// \param outputHeight renderer's output height
///
static void createTextures(SDL_Renderer *renderer, int outputWidth, int outputHeight) {

    // choose tile size
    double tileAspectRatio = (double)(outputWidth * ROWS) / (outputHeight * COLS);
    int newBaseTileWidth = max(1, outputWidth / COLS);
    int newBaseTileHeight = max(1, outputHeight / ROWS);
    if (newBaseTileWidth >= MAX_TILE_SIZE || newBaseTileHeight >= MAX_TILE_SIZE) {
        newBaseTileWidth = max(1, min(TILE_WIDTH, round(TILE_HEIGHT * tileAspectRatio)));
        newBaseTileHeight = max(1, min(TILE_HEIGHT, round(TILE_WIDTH / tileAspectRatio)));
    }

    // if tile size has not changed, we don't need to rebuild the tiles
    if (baseTileWidth == newBaseTileWidth && baseTileHeight == newBaseTileHeight) {
        return;
    }

    baseTileWidth = newBaseTileWidth;
    baseTileHeight = newBaseTileHeight;

    // destroy the old textures
    for (int i = 0; i < 4; i++) {
        if (Textures[i]) SDL_DestroyTexture(Textures[i]);
        Textures[i] = NULL;
    }

    // choose the number of textures
    if (baseTileWidth >= MAX_TILE_SIZE || baseTileHeight >= MAX_TILE_SIZE) {
        numTextures = 1;
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    } else {
        numTextures = 4;
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    }

    // The original image will be resized to 4 possible sizes:
    //  -  Textures[0]: tiles are   W   x   H   pixels
    //  -  Textures[1]: tiles are (W+1) x   H   pixels
    //  -  Textures[2]: tiles are   W   x (H+1) pixels
    //  -  Textures[3]: tiles are (W+1) x (H+1) pixels

    for (int i = 0; i < numTextures; i++) {

        // choose dimensions
        int tileWidth = baseTileWidth + (i == 1 || i == 3 ? 1 : 0);
        int tileHeight = baseTileHeight + (i == 2 || i == 3 ? 1 : 0);
        int surfaceWidth = 1, surfaceHeight = 1;
        while (surfaceWidth < tileWidth * TILE_COLS) surfaceWidth *= 2;
        while (surfaceHeight < tileHeight * TILE_ROWS) surfaceHeight *= 2;

        // downscale the tiles
        SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, surfaceWidth, surfaceHeight, 32, SDL_PIXELFORMAT_ARGB8888);
        if (!surface) sdlfatal(__FILE__, __LINE__);
        for (int row = 0; row < TILE_ROWS; row++) {
            for (int column = 0; column < TILE_COLS; column++) {
                downscaleTile(surface, tileWidth, tileHeight, row, column, false);
            }
        }

        // convert to texture
        Textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        if (!Textures[i]) sdlfatal(__FILE__, __LINE__);
        if (SDL_SetTextureBlendMode(Textures[i], SDL_BLENDMODE_BLEND) < 0) sdlfatal(__FILE__, __LINE__);
        SDL_FreeSurface(surface);
    }
}


/// Updates the screen buffer.
///
/// \param row row on screen (between 0 and ROWS-1)
/// \param column column on screen (between 0 and COLS-1)
/// \param charIndex glyph number (e.g 48 for "@", 281 for Dragon)
/// \param foreRed red component of the foreground color (0..100)
/// \param foreGreen green component of the foreground color (0..100)
/// \param foreBlue blue component of the foreground color (0..100)
/// \param backRed red component of the background color (0..100)
/// \param backGreen green component of the background color (0..100)
/// \param backBlue blue component of the background color (0..100)
///
void updateTile(int row, int column, short charIndex,
    short foreRed, short foreGreen, short foreBlue,
    short backRed, short backGreen, short backBlue)
{
    screenTiles[row][column] = (ScreenTile){
        .foreRed   = foreRed,
        .foreGreen = foreGreen,
        .foreBlue  = foreBlue,
        .backRed   = backRed,
        .backGreen = backGreen,
        .backBlue  = backBlue,
        .charIndex = charIndex,
        .needsRefresh = 1
    };
}


/// Draws everything on screen.
///
/// OpenGL drivers don't like alternating between different textures too much, so we
/// first draw the background colors then do 4 passes over the tiles, one pass per texture.
///
/// Some video drivers are quite inefficient, notably in virtual machines, so
/// there is a new `--no-gpu` command-line parameter to disable hardware acceleration.
/// The software renderer does not support HiDPI, though.
///
/// To improve performance of the software renderer, we don't redraw the whole screen but
/// only the tiles that have changed recently (which is tracked with ScreenTile::needsRefresh).
/// This works because, unlike the accelerated renderers, the software renderer draws on a
/// single surface and doesn't do double-buffering.
///
void updateScreen() {
    if (!Win) return;

    SDL_Renderer *renderer = SDL_GetRenderer(Win);
    if (!renderer) {
        renderer = SDL_CreateRenderer(Win, -1, (softwareRendering ? SDL_RENDERER_SOFTWARE : 0));
        if (!renderer) sdlfatal(__FILE__, __LINE__);

        if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE) < 0) sdlfatal(__FILE__, __LINE__);

        // see if we ended up using the software renderer or not
        SDL_RendererInfo info;
        if (SDL_GetRendererInfo(renderer, &info) < 0) sdlfatal(__FILE__, __LINE__);
        softwareRendering = (strcmp(info.name, "software") == 0);
    }

    int outputWidth, outputHeight;
    if (SDL_GetRendererOutputSize(renderer, &outputWidth, &outputHeight) < 0) sdlfatal(__FILE__, __LINE__);
    if (outputWidth == 0 || outputHeight == 0) return;

    createTextures(renderer, outputWidth, outputHeight);

    if (!softwareRendering) {
        // black out the frame (double-buffering invalidated it)
        if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) < 0) sdlfatal(__FILE__, __LINE__);
        if (SDL_RenderClear(renderer) < 0) sdlfatal(__FILE__, __LINE__);
    }

    // To please the OpenGL renderer, we'll proceed in 5 steps:
    //  -1. background colors
    //  0.  Textures[0]
    //  1.  Textures[1]
    //  2.  Textures[2]
    //  3.  Textures[3]

    for (int step = -1; step < numTextures; step++) {

        for (int x = 0; x < COLS; x++) {
            int tileWidth = ((x+1) * outputWidth / COLS) - (x * outputWidth / COLS);
            if (tileWidth == 0) continue;

            for (int y = 0; y < ROWS; y++) {
                int tileHeight = ((y+1) * outputHeight / ROWS) - (y * outputHeight / ROWS);
                if (tileHeight == 0) continue;

                ScreenTile *tile = &screenTiles[y][x];
                if (softwareRendering && !tile->needsRefresh) {
                    continue; // software rendering does not use double-buffering, so the tile is still on screen
                }

                if (step < 0) {
                    if (!softwareRendering && tile->backRed == 0 && tile->backGreen == 0 && tile->backBlue == 0) {
                        continue; // SDL_RenderClear already painted everything black
                    }

                    SDL_Rect dest;
                    dest.w = tileWidth;
                    dest.h = tileHeight;
                    dest.x = x * outputWidth / COLS;
                    dest.y = y * outputHeight / ROWS;

                    // paint the background
                    if (SDL_SetRenderDrawColor(renderer,
                        round(2.55 * tile->backRed),
                        round(2.55 * tile->backGreen),
                        round(2.55 * tile->backBlue), 255) < 0) sdlfatal(__FILE__, __LINE__);
                    if (SDL_RenderFillRect(renderer, &dest) < 0) sdlfatal(__FILE__, __LINE__);

                } else {
                    int textureIndex = (numTextures < 4 ? 0 : (tileWidth > baseTileWidth ? 1 : 0) + (tileHeight > baseTileHeight ? 2 : 0));
                    if (step != textureIndex) {
                        continue; // this tile uses another texture and gets painted at another step
                    }

                    int tileRow    = tile->charIndex / 16;
                    int tileColumn = tile->charIndex % 16;

                    if (tileEmpty[tileRow][tileColumn]
                            && !(tileRow == 21 && tileColumn == 1)) {  // wall top (procedural)
                        continue; // there is nothing to draw
                    }

                    SDL_Rect src;
                    src.w = baseTileWidth  + (step == 1 || step == 3 ? 1 : 0);
                    src.h = baseTileHeight + (step == 2 || step == 3 ? 1 : 0);
                    src.x = src.w * tileColumn;
                    src.y = src.h * tileRow;

                    SDL_Rect dest;
                    dest.w = tileWidth;
                    dest.h = tileHeight;
                    dest.x = x * outputWidth / COLS;
                    dest.y = y * outputHeight / ROWS;

                    // blend the foreground
                    if (SDL_SetTextureColorMod(Textures[step],
                        round(2.55 * tile->foreRed),
                        round(2.55 * tile->foreGreen),
                        round(2.55 * tile->foreBlue)) < 0) sdlfatal(__FILE__, __LINE__);
                    if (SDL_RenderCopy(renderer, Textures[step], &src, &dest) < 0) sdlfatal(__FILE__, __LINE__);
                }
            }
        }
    }

    SDL_RenderPresent(renderer);

    // the screen is now up to date
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            screenTiles[y][x].needsRefresh = 0;
        }
    }
}


/*
Creates or resizes the game window with the currently loaded font.
*/
void resizeWindow(int width, int height) {

    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) < 0) sdlfatal(__FILE__, __LINE__);

    // By default the window will have an aspect ratio of 16:10
    // and either 80% of monitor width or 80% of monitor height, whichever is smaller
    if (width < 0) width = min(mode.w * 8/10, mode.h * 8*16/(10*10));
    if (height < 0) height = width * 10/16;

    // go to fullscreen mode if the window is as big as the screen
    if (width >= mode.w && height >= mode.h) fullScreen = true;

    if (Win == NULL) {
        // create the window
        Win = SDL_CreateWindow("Brogue",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
        if (!Win) sdlfatal(__FILE__, __LINE__);

        // set its icon
        char filename[BROGUE_FILENAME_MAX];
        sprintf(filename, "%s/assets/icon.png", dataDirectory);
        SDL_Surface *icon = IMG_Load(filename);
        if (!icon) imgfatal(__FILE__, __LINE__);
        SDL_SetWindowIcon(Win, icon);
        SDL_FreeSurface(icon);
    }

    if (fullScreen) {
        if (!(SDL_GetWindowFlags(Win) & SDL_WINDOW_FULLSCREEN_DESKTOP)) {
            // switch to fullscreen mode
            if (SDL_SetWindowFullscreen(Win, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0) sdlfatal(__FILE__, __LINE__);
        }
    } else {
        if (SDL_GetWindowFlags(Win) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
            // switch to windowed mode
            if (SDL_SetWindowFullscreen(Win, 0) < 0) sdlfatal(__FILE__, __LINE__);
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

    SDL_GetWindowSize(Win, &windowWidth, &windowHeight);
    refreshScreen();
    updateScreen();
}


SDL_Surface *captureScreen() {
    if (!Win) return NULL;

    // get the renderer
    SDL_Renderer *renderer = SDL_GetRenderer(Win);
    if (!renderer) return NULL;

    // get its size
    int outputWidth, outputHeight;
    if (SDL_GetRendererOutputSize(renderer, &outputWidth, &outputHeight) < 0) sdlfatal(__FILE__, __LINE__);
    if (outputWidth == 0 || outputHeight == 0) return NULL;

    // take a screenshot
    SDL_Surface *screenshot = SDL_CreateRGBSurfaceWithFormat(0, outputWidth, outputHeight, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!screenshot) sdlfatal(__FILE__, __LINE__);
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels, outputWidth * 4) < 0) sdlfatal(__FILE__, __LINE__);
    return screenshot;
}

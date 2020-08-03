#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef SDL_PATHS
#include <unistd.h>
#endif

#include <SDL.h>
#include <SDL_image.h>

#include "platform.h"

#define PAUSE_BETWEEN_EVENT_POLLING     36L//17
#define MAX_REMAPS  128

// Dimensions of the font characters
#define N_FONTS  15
static const int fontWidths[] = {7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 24, 27};
static const int fontHeights[] = {11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 38, 44, 49};

struct keypair {
    char from;
    char to;
};

static SDL_Window *Win = NULL;
static SDL_Surface *WinSurf = NULL;
static SDL_Surface *Font = NULL;
static SDL_Surface *Tiles = NULL;

static struct keypair remapping[MAX_REMAPS];
static size_t nremaps = 0;
static boolean showGraphics = false;

static rogueEvent lastEvent;


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


static void loadFont(int fontsize) {
    char filename[BROGUE_FILENAME_MAX];

    static int lastsize = 0;
    if (lastsize != fontsize) {
        sprintf(filename, "%s/assets/font-%i.png", dataDirectory, fontsize);
        if (Font != NULL) SDL_FreeSurface(Font);
        Font = IMG_Load(filename);
        if (Font == NULL) imgfatal();

        sprintf(filename, "%s/assets/tiles-%i.png", dataDirectory, fontsize);
        if (Tiles != NULL) SDL_FreeSurface(Tiles);
        Tiles = IMG_Load(filename);
        if (Tiles == NULL) imgfatal();
    }
    lastsize = fontsize;
}


static int fitFontSize(int width, int height) {
    int size;
    for (
        size = N_FONTS - 1;
        size > 0
            && (fontWidths[size] * COLS > width
                || fontHeights[size] * ROWS > height);
        size--
    );
    return size + 1;
}


/*
Creates or resizes the game window with the currently loaded font.
*/
static void ensureWindow() {
    if (Font == NULL) return;
    int cellw = fontWidths[brogueFontSize - 1], cellh = fontHeights[brogueFontSize - 1];

    if (Win != NULL) {
        SDL_SetWindowSize(Win, cellw*COLS, cellh*ROWS);
    } else {
        Win = SDL_CreateWindow("Brogue",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, cellw*COLS, cellh*ROWS, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        if (Win == NULL) sdlfatal();

        char filename[BROGUE_FILENAME_MAX];
        sprintf(filename, "%s/assets/icon.png", dataDirectory);
        SDL_Surface *icon = IMG_Load(filename);
        if (icon == NULL) imgfatal();
        SDL_SetWindowIcon(Win, icon);
        SDL_FreeSurface(icon);
    }

    refreshWindow();
}


static void getWindowPadding(int *x, int *y) {
    if (Font == NULL) return;
    int cellw = fontWidths[brogueFontSize - 1], cellh = fontHeights[brogueFontSize - 1];

    int winw, winh;
    SDL_GetWindowSize(Win, &winw, &winh);

    *x = (winw - cellw*COLS) / 2;
    *y = (winh - cellh*ROWS) / 2;
}


/*
If the key is to be processed, returns true and updates event. False
otherwise. This function only listens for keypresses which do not produce
corresponding TextInputEvents.
*/
static boolean eventFromKey(rogueEvent *event, SDL_Keycode key) {
    event->param1 = -1;

    switch (key) {
        case SDLK_ESCAPE:
            event->param1 = ESCAPE_KEY;
            return true;
        case SDLK_UP:
            event->param1 = UP_ARROW;
            return true;
        case SDLK_DOWN:
            event->param1 = DOWN_ARROW;
            return true;
        case SDLK_RIGHT:
            event->param1 = RIGHT_ARROW;
            return true;
        case SDLK_LEFT:
            event->param1 = LEFT_ARROW;
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            event->param1 = RETURN_KEY;
            return true;
        case SDLK_BACKSPACE:
            event->param1 = DELETE_KEY;
            return true;
        case SDLK_TAB:
            event->param1 = TAB_KEY;
            return true;
        case SDLK_PRINTSCREEN:
            event->param1 = PRINTSCREEN_KEY;
            return true;
    }

    /*
    Only process keypad events when we're holding a modifier, as there is no
    TextInputEvent then.
    */
    if (event->shiftKey || event->controlKey) {
        switch (key) {
            case SDLK_KP_0:
                event->param1 = NUMPAD_0;
                return true;
            case SDLK_KP_1:
                event->param1 = NUMPAD_1;
                return true;
            case SDLK_KP_2:
                event->param1 = NUMPAD_2;
                return true;
            case SDLK_KP_3:
                event->param1 = NUMPAD_3;
                return true;
            case SDLK_KP_4:
                event->param1 = NUMPAD_4;
                return true;
            case SDLK_KP_5:
                event->param1 = NUMPAD_5;
                return true;
            case SDLK_KP_6:
                event->param1 = NUMPAD_6;
                return true;
            case SDLK_KP_7:
                event->param1 = NUMPAD_7;
                return true;
            case SDLK_KP_8:
                event->param1 = NUMPAD_8;
                return true;
            case SDLK_KP_9:
                event->param1 = NUMPAD_9;
                return true;
        }
    }

    // Ctrl+letter doesn't give a TextInputEvent
    if (event->controlKey && key >= SDLK_a && key <= SDLK_z) {
        event->param1 = 'a' + (key - SDLK_a);
        if (event->shiftKey) event->param1 -= 'a' - 'A';
        return true;
    }

    return false;
}


static boolean _modifierHeld(int mod) {
    SDL_Keymod km = SDL_GetModState();
    return mod == 0 && (km & (KMOD_LSHIFT | KMOD_RSHIFT))
        || mod == 1 && (km & (KMOD_LCTRL | KMOD_RCTRL));
}


static char applyRemaps(char c) {
    for (size_t i=0; i < nremaps; i++) {
        if (remapping[i].from == c) return remapping[i].to;
    }
    return c;
}


/*
If an event is available, returns true and updates returnEvent. Otherwise
it returns false and an error event. This function also processes
platform-specific inputs/behaviours.
*/
static boolean pollBrogueEvent(rogueEvent *returnEvent, boolean textInput) {
    static int mx = 0, my = 0;
    int cellw = fontWidths[brogueFontSize - 1], cellh = fontHeights[brogueFontSize - 1];

    int padx, pady;
    getWindowPadding(&padx, &pady);

    returnEvent->eventType = EVENT_ERROR;
    returnEvent->shiftKey = _modifierHeld(0);
    returnEvent->controlKey = _modifierHeld(1);

    SDL_Event event;
    boolean ret = false;


    // ~ for (int i=0; i < 100 && SDL_PollEvent(&event); i++) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            rogue.gameHasEnded = true; // causes the game loop to terminate quickly
            rogue.nextGame = NG_QUIT; // causes the menu to drop out immediately
            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = ESCAPE_KEY;
            return true;
        } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            brogueFontSize = fitFontSize(event.window.data1, event.window.data2);
            loadFont(brogueFontSize);
            refreshWindow();
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_PAGEUP && brogueFontSize < N_FONTS) {
                loadFont(++brogueFontSize);
                ensureWindow();
            } else if (key == SDLK_PAGEDOWN && brogueFontSize > 1) {
                loadFont(--brogueFontSize);
                ensureWindow();
            } else if (key == SDLK_F11 || key == SDLK_F12
                    || key == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT)) {
                // Toggle fullscreen
                SDL_SetWindowFullscreen(Win,
                    (SDL_GetWindowFlags(Win) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                refreshWindow();
            }

            if (eventFromKey(returnEvent, key)) {
                returnEvent->eventType = KEYSTROKE;
                return true;
            }
        } else if (event.type == SDL_TEXTINPUT && event.text.text[0] < 0x80) {
            /*
            It's difficult/impossible to check what characters are on the
            shifts of keys. So to detect '&', '>' etc. reliably we need to
            listen for text input events as well as keydowns. This results
            in hybrid keyboard code, where Brogue KEYSTROKEs can come from
            different SDL events.
            */
            char c = event.text.text[0];

            if (!textInput) {
                c = applyRemaps(c);
                if ((c == '=' || c == '+') && brogueFontSize < N_FONTS) {
                    loadFont(++brogueFontSize);
                    ensureWindow();
                } else if (c == '-' && brogueFontSize > 1) {
                    loadFont(--brogueFontSize);
                    ensureWindow();
                }
            }

            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = c;
            // ~ printf("textinput %s\n", event.text.text);
            return true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    returnEvent->eventType = MOUSE_DOWN;
                } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                    returnEvent->eventType = MOUSE_UP;
                } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                    returnEvent->eventType = RIGHT_MOUSE_DOWN;
                } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
                    returnEvent->eventType = RIGHT_MOUSE_UP;
                }
                returnEvent->param1 = (event.button.x - padx) / cellw;
                returnEvent->param2 = (event.button.y - pady) / cellh;
                return true;
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            // We don't want to return on a mouse motion event, because only the last
            // in the queue is important. That's why we just set ret=true
            int xcell = (event.motion.x - padx) / cellw,
                ycell = (event.motion.y - pady) / cellh;
            if (xcell != mx || ycell != my) {
                returnEvent->eventType = MOUSE_ENTERED_CELL;
                returnEvent->param1 = xcell;
                returnEvent->param2 = ycell;
                mx = xcell;
                my = ycell;
                ret = true;
            }
        }
    }

    return ret;
}


static void _gameLoop() {
#ifdef SDL_PATHS
    char *path = SDL_GetBasePath();
    if (path) {
        path[strlen(path) - 1] = '\0';  // remove trailing separator
        strcpy(dataDirectory, path);
    } else {
        fprintf(stderr, "Failed to find the path to the application\n");
        exit(1);
    }
    free(path);

    path = SDL_GetPrefPath("Brogue", "Brogue CE");
    if (!path || chdir(path) != 0) {
        fprintf(stderr, "Failed to find or change to the save directory\n");
        exit(1);
    }
    free(path);
#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0) sdlfatal();

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) imgfatal();

    lastEvent.eventType = EVENT_ERROR;

    if (brogueFontSize == 0) {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(0, &mode);
        brogueFontSize = fitFontSize(mode.w - 20, mode.h - 100);
    }

    loadFont(brogueFontSize);
    ensureWindow();

    rogueMain();

    SDL_DestroyWindow(Win);
    SDL_Quit();
}


static boolean _pauseForMilliseconds(short ms) {
    SDL_UpdateWindowSurface(Win);
    SDL_Delay(ms);
    return (pollBrogueEvent(&lastEvent, false) || lastEvent.eventType != EVENT_ERROR)
        && lastEvent.eventType != MOUSE_ENTERED_CELL;
}


static void _nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    long tstart, dt;

    SDL_UpdateWindowSurface(Win);

    if (lastEvent.eventType != EVENT_ERROR) {
        *returnEvent = lastEvent;
        lastEvent.eventType = EVENT_ERROR;
        return;
    }

    while (true) {
        tstart = SDL_GetTicks();

        if (colorsDance) {
            shuffleTerrainColors(3, true);
            commitDraws();
        }

        SDL_UpdateWindowSurface(Win);

        if (pollBrogueEvent(returnEvent, textInput)) break;

        dt = PAUSE_BETWEEN_EVENT_POLLING - (SDL_GetTicks() - tstart);
        if (dt > 0) {
            SDL_Delay(dt);
        }
    }
}


/*
Returns the index of the sprite representing the given glyph. Sprites <256 are
from the text font sheet, 256+ are from the tiles sheet.
*/
static int fontIndex(enum displayGlyph glyph) {
    // These are the only non-ASCII glyphs which always come from the font sheet
    if (glyph == G_UP_ARROW) return 0x90;
    if (glyph == G_DOWN_ARROW) return 0x91;

    if (glyph < 128) {
        // ASCII characters map directly
        return glyph;
    } else if (showGraphics && glyph >= 128) {
        // Tile glyphs have sprite indices starting at 256
        // -2 to disregard the up and down arrow glyphs
        return glyph + 128 - 2;
    } else {
        unsigned int code = glyphToUnicode(glyph);
        switch (code) {
            case U_MIDDLE_DOT: return 0x80;
            case U_FOUR_DOTS: return 0x81;
            case U_DIAMOND: return 0x82;
            case U_FLIPPED_V: return 0x83;
            case U_ARIES: return 0x84;
            case U_ESZETT: return 0xdf;
            case U_ANKH: return 0x85;
            case U_MUSIC_NOTE: return 0x86;
            case U_CIRCLE: return 0x87;
            case U_LIGHTNING_BOLT: return 0x99;
            case U_FILLED_CIRCLE: return 0x89;
            case U_NEUTER: return 0x8a;
            case U_U_ACUTE: return 0xda;
            case U_CURRENCY: return 0xa4;
            case U_UP_ARROW: return 0x90;
            case U_DOWN_ARROW: return 0x91;
            case U_LEFT_ARROW: return 0x92;
            case U_RIGHT_ARROW: return 0x93;
            case U_OMEGA: return 0x96;
            case U_CIRCLE_BARS: return 0x8c;
            case U_FILLED_CIRCLE_BARS: return 0x8d;

            default:
                brogueAssert(code < 256);
                return code;
        }
    }
}


static void _plotChar(
    enum displayGlyph inputChar,
    short x, short y,
    short foreRed, short foreGreen, short foreBlue,
    short backRed, short backGreen, short backBlue
) {
    int padx, pady;
    getWindowPadding(&padx, &pady);

    SDL_Surface *sheet;
    inputChar = fontIndex(inputChar);
    if (inputChar >= 256) {
        sheet = Tiles;
        inputChar -= 256;
    } else {
        sheet = Font;
    }

    SDL_Rect src, dest;
    int cellw = fontWidths[brogueFontSize - 1], cellh = fontHeights[brogueFontSize - 1];
    src.x = (inputChar % 16) * cellw;
    src.y = (inputChar / 16) * cellh;
    src.w = cellw;
    src.h = cellh;

    dest.x = cellw * x + padx;
    dest.y = cellh * y + pady;
    dest.w = cellw;
    dest.h = cellh;

    SDL_FillRect(WinSurf, &dest, SDL_MapRGB(
        WinSurf->format, backRed * 255 / 100, backGreen * 255 / 100, backBlue * 255 / 100
    ));
    SDL_SetSurfaceColorMod(sheet, foreRed * 255 / 100, foreGreen * 255 / 100, foreBlue * 255 / 100);
    SDL_BlitSurface(sheet, &src, WinSurf, &dest);
}


static void _remap(const char *from, const char *to) {
    if (nremaps < MAX_REMAPS) {
        remapping[nremaps].from = from[0];
        remapping[nremaps].to = to[0];
        nremaps++;
    }
}


/*
 * Take screenshot in current working directory (ScreenshotN.png)
 */
static boolean _takeScreenshot() {
    char screenshotFilepath[BROGUE_FILENAME_MAX];

    getAvailableFilePath(screenshotFilepath, "Screenshot", SCREENSHOT_SUFFIX);
    strcat(screenshotFilepath, SCREENSHOT_SUFFIX);

    if (WinSurf) {
        IMG_SavePNG(WinSurf, screenshotFilepath);
        return true;
    }
    return false;
}


static boolean _setGraphicsEnabled(boolean state) {
    showGraphics = state;
    if (WinSurf) refreshScreen();
    return state;
}


struct brogueConsole sdlConsole = {
    _gameLoop,
    _pauseForMilliseconds,
    _nextKeyOrMouseEvent,
    _plotChar,
    _remap,
    _modifierHeld,
    NULL,
    _takeScreenshot,
    _setGraphicsEnabled
};

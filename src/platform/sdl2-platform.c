#ifdef SDL_PATHS
#include <unistd.h>
#endif

#include <limits.h>
#include "tiles.h"

#define PAUSE_BETWEEN_EVENT_POLLING     36L//17
#define MAX_REMAPS  128

struct keypair {
    char from;
    char to;
};

extern SDL_Window *Win;
extern SDL_Surface *WinSurf;

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
            resizeWindow(event.window.data1, event.window.data2);
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_PAGEUP) {
                resizeWindow(windowWidth * 11/10, windowHeight * 11/10);
                continue;
            } else if (key == SDLK_PAGEDOWN) {
                resizeWindow(windowWidth * 10/11, windowHeight * 10/11);
                continue;
            } else if (key == SDLK_F11 || key == SDLK_F12
                    || key == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT)) {
                SDL_DisplayMode mode;
                SDL_GetCurrentDisplayMode(0, &mode);
                int targetSize = (windowWidth == mode.w && windowHeight == mode.h ? 0 : INT_MAX);
                resizeWindow(targetSize, targetSize);
                continue;
            }

            if (eventFromKey(returnEvent, key)) {
                returnEvent->eventType = KEYSTROKE;
                return true;
            }
        } else if (event.type == SDL_TEXTINPUT && (unsigned char)(event.text.text[0]) < 0x80) {
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
                if (c == '=' || c == '+') {
                    resizeWindow(windowWidth * 11/10, windowHeight * 11/10);
                } else if (c == '-') {
                    resizeWindow(windowWidth * 10/11, windowHeight * 10/11);
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
                returnEvent->param1 = event.button.x * COLS / windowWidth;
                returnEvent->param2 = event.button.y * ROWS / windowHeight;
                return true;
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            // We don't want to return on a mouse motion event, because only the last
            // in the queue is important. That's why we just set ret=true
            int xcell = event.motion.x * COLS / windowWidth,
                ycell = event.motion.y * ROWS / windowHeight;
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

    resizeWindow(windowWidth, windowHeight);

    rogueMain();

    SDL_DestroyWindow(Win);
    SDL_Quit();
}


static boolean _pauseForMilliseconds(short ms) {
    SDL_UpdateWindowSurface(Win);
    SDL_Delay(ms);

    if (lastEvent.eventType != EVENT_ERROR
        && lastEvent.eventType != MOUSE_ENTERED_CELL) {
        return true; // SDL already gave us an interrupting event to process
    }

    return pollBrogueEvent(&lastEvent, false) // ask SDL for a new event if one is available
        && lastEvent.eventType != EVENT_ERROR // and check if it is interrupting
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
    SDL_Surface *tiles;
    SDL_Rect src, dest;

    inputChar = fontIndex(inputChar);

    dest.x = x * windowWidth / COLS;
    dest.y = y * windowHeight / ROWS;
    dest.w = (x+1) * windowWidth / COLS - dest.x;
    dest.h = (y+1) * windowHeight / ROWS - dest.y;

    tiles = getTiles(dest.w, dest.h);
    if (!tiles) return;

    src.x = (inputChar % 16) * dest.w;
    src.y = (inputChar / 16) * dest.h;
    src.w = dest.w;
    src.h = dest.h;

    prepareTile(tiles, inputChar / 16, inputChar % 16, false);
    SDL_FillRect(WinSurf, &dest, SDL_MapRGB(
        WinSurf->format, backRed * 255 / 100, backGreen * 255 / 100, backBlue * 255 / 100
    ));
    SDL_SetSurfaceColorMod(tiles, foreRed * 255 / 100, foreGreen * 255 / 100, foreBlue * 255 / 100);
    SDL_BlitSurface(tiles, &src, WinSurf, &dest);
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

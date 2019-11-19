#ifdef BROGUE_SDL

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL.h>
#include <SDL_image.h>

#include "platform.h"

#define PAUSE_BETWEEN_EVENT_POLLING     36L//17

extern playerCharacter rogue;

static SDL_Window *Win = NULL;
static SDL_Surface *WinSurf = NULL;
static SDL_Surface *Font = NULL;
static int FontSize = 8;

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
Creates or resizes the game window with the specified font size.
*/
static void ensureWindow(int fontsize) {
    char fontname[] = "fonts/font-000.png";
    sprintf(fontname, "fonts/font-%i.png", fontsize);

    if (Font != NULL) SDL_FreeSurface(Font);
    Font = IMG_Load(fontname);
    if (Font == NULL) imgfatal();

    int cellw = Font->w / 16, cellh = Font->h / 16;

    if (Win != NULL) {
        SDL_SetWindowSize(Win, cellw*COLS, cellh*ROWS);
    } else {
        Win = SDL_CreateWindow("Brogue " BROGUE_VERSION_STRING,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cellw*COLS, cellh*ROWS, 0);
        if (Win == NULL) sdlfatal();
    }
    WinSurf = SDL_GetWindowSurface(Win);
    if (WinSurf == NULL) sdlfatal();
    refreshScreen();
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
            event->param1 = RETURN_KEY;
            return true;
        case SDLK_KP_ENTER:
            event->param1 = ENTER_KEY;
            return true;
        case SDLK_BACKSPACE:
            event->param1 = DELETE_KEY;
            return true;
        case SDLK_TAB:
            event->param1 = TAB_KEY;
            return true;
    }

    /*
    Only process keypad events when we're holding shift, as there is no
    TextInputEvent then.
    */
    if (event->shiftKey) {
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

    // Ctrl+N (custom new game) doesn't give a TextInputEvent
    if (event->controlKey && key == SDLK_n) {
        event->param1 = 'n';
        return true;
    }

    return false;
}


static boolean _modifierHeld(int mod) {
    SDL_Keymod km = SDL_GetModState();
    return mod == 0 && (km & (KMOD_LSHIFT | KMOD_RSHIFT))
        || mod == 1 && (km & (KMOD_LCTRL | KMOD_RCTRL));
}


/*
If an event is available, returns true and updates returnEvent. Otherwise
it returns false and an error event. This function also processes
platform-specific inputs/behaviours.
*/
static boolean pollBrogueEvent(rogueEvent *returnEvent, boolean textInput) {
    static int mx = 0, my = 0;
    int cellw = Font->w / 16, cellh = Font->h / 16;

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
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_PAGEUP && FontSize < 13) {
                ensureWindow(++FontSize);
            } else if (key == SDLK_PAGEDOWN && FontSize > 1) {
                ensureWindow(--FontSize);
            }

            if (eventFromKey(returnEvent, key)) {
                returnEvent->eventType = KEYSTROKE;
                return true;
            }
        } else if (event.type == SDL_TEXTINPUT && strlen(event.text.text) == 1) {
            /*
            It's difficult/impossible to check what characters are on the
            shifts of keys. So to detect '&', '>' etc. reliably we need to
            listen for text input events as well as keydowns. This results
            in hybrid keyboard code, where Brogue KEYSTROKEs can come from
            different SDL events.
            */
            char c = event.text.text[0];
            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = c;

            if (!textInput) {
                if ((c == '=' || c == '+') && FontSize < 13) {
                    ensureWindow(++FontSize);
                } else if (c == '-' && FontSize > 1) {
                    ensureWindow(--FontSize);
                }
            }

            // ~ printf("textinput %s\n", event.text.text);
            return true;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            returnEvent->eventType = MOUSE_DOWN;
            returnEvent->param1 = event.button.x / cellw;
            returnEvent->param2 = event.button.y / cellh;
            return true;
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            returnEvent->eventType = MOUSE_UP;
            returnEvent->param1 = event.button.x / cellw;
            returnEvent->param2 = event.button.y / cellh;
            return true;
        } else if (event.type == SDL_MOUSEMOTION) {
            // We don't want to return on a mouse motion event, because only the last
            // in the queue is important. That's why we just set ret=true
            int xcell = event.motion.x / cellw, ycell = event.motion.y / cellh;
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

    if (SDL_Init(SDL_INIT_VIDEO) < 0) sdlfatal();

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) imgfatal();

    lastEvent.eventType = EVENT_ERROR;

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);

    // Dimensions of the font graphics. Divide by 16 to get individual character dimensions.
    int fontWidths[13] = {112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304};
    int fontHeights[13] = {176, 208, 240, 272, 304, 336, 368, 400, 432, 464, 496, 528, 528};

    int size;
    for (
        size = 12;
        size >= 0
            && (fontWidths[size] / 16 * COLS > mode.w - 20
                || fontHeights[size] / 16 * ROWS > mode.h - 50);
        size--
    );
    // If no sizes are small enough, choose smallest
    FontSize = size >= 0 ? size + 1 : 1;

    ensureWindow(FontSize);

    rogueMain();

    SDL_DestroyWindow(Win);
    SDL_Quit();
}


static boolean _pauseForMilliseconds(short ms) {
    SDL_UpdateWindowSurface(Win);
    SDL_Delay(ms);
    return lastEvent.eventType != EVENT_ERROR || pollBrogueEvent(&lastEvent, false);
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


static void _plotChar(
    uchar inputChar,
    short x, short y,
    short foreRed, short foreGreen, short foreBlue,
    short backRed, short backGreen, short backBlue
) {
    if (inputChar == STATUE_CHAR) {
        inputChar = 223;
    } else if (inputChar > 255) {
        switch (inputChar) {
#ifdef USE_UNICODE
            case FLOOR_CHAR: inputChar = 128 + 0; break;
            case CHASM_CHAR: inputChar = 128 + 1; break;
            case TRAP_CHAR: inputChar = 128 + 2; break;
            case FIRE_CHAR: inputChar = 128 + 3; break;
            case FOLIAGE_CHAR: inputChar = 128 + 4; break;
            case AMULET_CHAR: inputChar = 128 + 5; break;
            case SCROLL_CHAR: inputChar = 128 + 6; break;
            case RING_CHAR: inputChar = 128 + 7; break;
            case WEAPON_CHAR: inputChar = 128 + 8; break;
            case GEM_CHAR: inputChar = 128 + 9; break;
            case TOTEM_CHAR: inputChar = 128 + 10; break;
            case BAD_MAGIC_CHAR: inputChar = 128 + 12; break;
            case GOOD_MAGIC_CHAR: inputChar = 128 + 13; break;

            case DOWN_ARROW_CHAR: inputChar = 144 + 1; break;
            case LEFT_ARROW_CHAR: inputChar = 144 + 2; break;
            case RIGHT_ARROW_CHAR: inputChar = 144 + 3; break;
            case UP_TRIANGLE_CHAR: inputChar = 144 + 4; break;
            case DOWN_TRIANGLE_CHAR: inputChar = 144 + 5; break;
            case OMEGA_CHAR: inputChar = 144 + 6; break;
            case THETA_CHAR: inputChar = 144 + 7; break;
            case LAMDA_CHAR: inputChar = 144 + 8; break;
            case KOPPA_CHAR: inputChar = 144 + 9; break; // is this right?
            case CHARM_CHAR: inputChar = 144 + 9; break;
            case LOZENGE_CHAR: inputChar = 144 + 10; break;
            case CROSS_PRODUCT_CHAR: inputChar = 144 + 11; break;
#endif
            default: inputChar = '?'; break;
        }
    }

    SDL_Rect src, dest;
    int cellw = Font->w / 16, cellh = Font->h / 16;
    src.x = (inputChar % 16) * cellw;
    src.y = (inputChar / 16) * cellh;
    src.w = cellw;
    src.h = cellh;

    dest.x = cellw * x;
    dest.y = cellh * y;
    dest.w = cellw;
    dest.h = cellh;

    SDL_FillRect(WinSurf, &dest, SDL_MapRGB(
        WinSurf->format, backRed * 255 / 100, backGreen * 255 / 100, backBlue * 255 / 100
    ));
    SDL_SetSurfaceColorMod(Font, foreRed * 255 / 100, foreGreen * 255 / 100, foreBlue * 255 / 100);
    SDL_BlitSurface(Font, &src, WinSurf, &dest);
}


static void _remap(const char *a, const char *b) {}


struct brogueConsole sdlConsole = {
    _gameLoop,
    _pauseForMilliseconds,
    _nextKeyOrMouseEvent,
    _plotChar,
    _remap,
    _modifierHeld
};

#endif

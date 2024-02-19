#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include "platform.h"
#include "term.h"

static void gameLoop() {
    signal(SIGINT, SIG_DFL); // keep SDL from overriding the default ^C handler when it's linked

    if (!Term.start()) {
        return;
    }
    Term.title("Brogue");
    Term.resize(COLS, ROWS);

    rogueMain();

    Term.end();
}

static char glyphToAscii(enum displayGlyph glyph) {
    unsigned int ch;

    switch (glyph) {
        case G_UP_ARROW: return '^';
        case G_DOWN_ARROW: return 'v';
        case G_FLOOR: return '.';
        case G_CHASM: return ':';
        case G_TRAP: return '%';
        case G_FIRE: return '^';
        case G_FOLIAGE: return '&';
        case G_AMULET: return ',';
        case G_SCROLL: return '?';
        case G_RING: return '=';
        case G_WEAPON: return '(';
        case G_GEM: return '+';
        case G_TOTEM: return '0'; // zero
        case G_GOOD_MAGIC: return '$';
        case G_BAD_MAGIC: return '+';
        case G_DOORWAY: return '<';
        case G_CHARM: return '7';
        case G_GUARDIAN: return '5';
        case G_WINGED_GUARDIAN: return '5';
        case G_EGG: return 'o';
        case G_BLOODWORT_STALK: return '&';
        case G_FLOOR_ALT: return '.';
        case G_UNICORN: return 'U';
        case G_TURRET: return '*';
        case G_CARPET: return '.';
        case G_STATUE: return '5';
        case G_CRACKED_STATUE: return '5';
        case G_MAGIC_GLYPH: return ':';
        case G_ELECTRIC_CRYSTAL: return '$';

        default:
            ch = glyphToUnicode(glyph);
            brogueAssert(ch < 0x80); // assert ascii
            return ch;
    }
}

static void curses_plotChar(enum displayGlyph ch,
              short xLoc, short yLoc,
              short foreRed, short foreGreen, short foreBlue,
              short backRed, short backGreen, short backBlue) {

    fcolor fore;
    fcolor back;

    fore.r = (float) foreRed / 100;
    fore.g = (float) foreGreen / 100;
    fore.b = (float) foreBlue / 100;
    back.r = (float) backRed / 100;
    back.g = (float) backGreen / 100;
    back.b = (float) backBlue / 100;

    ch = glyphToAscii(ch);

    if (ch < ' ' || ch > 127) ch = ' ';
    Term.put(xLoc, yLoc, ch, &fore, &back);
}


struct mapsymbol {
    int in_c, out_c;
    struct mapsymbol *next;
};

static struct mapsymbol *keymap = NULL;

static int rewriteKey(int key, boolean text) {
    if (text) return key;

    struct mapsymbol *s = keymap;
    while (s != NULL) {
        if (s->in_c == key) {
            return s->out_c;
        }

        s = s->next;
    }
    return key;
}


#define PAUSE_BETWEEN_EVENT_POLLING     34//17

static uint64_t getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static long lastDelayTime = 0;

// Like SDL_Delay, but reduces the delay if time has passed since the last delay
static void _delayUpTo(short ms) {
    long curTime = getTime();
    long timeDiff = curTime - lastDelayTime;
    ms -= timeDiff;

    if (ms > 0) {
        Term.wait(ms);
    } // else delaying further would go past the time we want to delay until

    lastDelayTime = getTime();
}

static boolean curses_pauseForMilliseconds(short milliseconds) {
    Term.refresh();
    _delayUpTo(milliseconds);

    // hasKey returns true if we have a mouse event, too.
    return Term.hasKey();
}

static void curses_nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    int key;
    // TCOD_mouse_t mouse;
    // short x, y;

    Term.refresh();

    for (;;) {
        /*if (TCOD_console_is_window_closed()) {
            rogue.gameHasEnded = true; // causes the game loop to terminate quickly
            returnEvent->eventType = KEYSTROKE;
            returnEvent->param1 = ACKNOWLEDGE_KEY;
            return;
        }*/

        if (colorsDance) {
            shuffleTerrainColors(3, true);
            commitDraws();
        }


        key = Term.getkey();
        if (key == TERM_MOUSE) {
            if (Term.mouse.x > 0 && Term.mouse.y > 0 && Term.mouse.x < COLS && Term.mouse.y < ROWS) {
                returnEvent->param1 = Term.mouse.x;
                returnEvent->param2 = Term.mouse.y;
                returnEvent->eventType = KEYSTROKE;
                if (Term.mouse.justReleased) returnEvent->eventType = MOUSE_UP;
                if (Term.mouse.justPressed) returnEvent->eventType = MOUSE_DOWN;
                if (Term.mouse.justMoved) returnEvent->eventType = MOUSE_ENTERED_CELL;
                returnEvent->controlKey = Term.mouse.control;
                returnEvent->shiftKey = Term.mouse.shift;
                if (returnEvent->eventType != KEYSTROKE) return;
            }
        } else if (key != TERM_NONE) {
            key = rewriteKey(key, textInput);

            returnEvent->eventType = KEYSTROKE;
            returnEvent->controlKey = Term.ctrlPressed(&key); //(key.rctrl || key.lctrl);
            returnEvent->shiftKey = 0; //key.shift;
            returnEvent->param1 = key;

            if (key == Term.keys.backspace || key == Term.keys.del) returnEvent->param1 = DELETE_KEY;
            else if (key == Term.keys.up) returnEvent->param1 = UP_ARROW;
            else if (key == Term.keys.down) returnEvent->param1 = DOWN_ARROW;
            else if (key == Term.keys.left) returnEvent->param1 = LEFT_ARROW;
            else if (key == Term.keys.right) returnEvent->param1 = RIGHT_ARROW;
            else if (key == Term.keys.quit) {
                rogue.gameHasEnded = true;
                rogue.gameExitStatusCode = EXIT_STATUS_SUCCESS;
                rogue.nextGame = NG_QUIT; // causes the menu to drop out immediately
            }
            else if ((key >= 'A' && key <= 'Z')) {
                returnEvent->shiftKey = 1;
                // returnEvent->param1 += 'a' - 'A';
            }
            // we could try to catch control keys, where possible, but we'll catch keys we mustn't
            /* else if ((key >= 'A'-'@' && key <= 'Z'-'@')) {
                returnEvent->controlKey = 1;
                returnEvent->param1 += 'a' - ('A'-'@');
            } */

            return;
        }

        _delayUpTo(PAUSE_BETWEEN_EVENT_POLLING);
    }
}

static void curses_remap(const char *input_name, const char *output_name) {
    struct mapsymbol *sym = malloc(sizeof(*sym));

    if (sym == NULL) return; // out of memory?  seriously?

    sym->in_c = Term.keycodeByName(input_name);
    sym->out_c = Term.keycodeByName(output_name);

    sym->next = keymap;
    keymap = sym;
}

static boolean modifier_held(int modifier) {
    return 0;
}

struct brogueConsole cursesConsole = {
    gameLoop,
    curses_pauseForMilliseconds,
    curses_nextKeyOrMouseEvent,
    curses_plotChar,
    curses_remap,
    modifier_held,
    NULL,
    NULL,
    NULL
};

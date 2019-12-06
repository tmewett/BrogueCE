#ifdef BROGUE_CURSES
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "term.h"
#include <stdint.h>
#include <signal.h>
#include "platform.h"

extern playerCharacter rogue;

static void gameLoop() {
    signal(SIGINT, SIG_DFL); // keep SDL from overriding the default ^C handler when it's linked

    if (!Term.start()) {
        return;
    }
    Term.title("Brogue " BROGUE_VERSION_STRING);
    Term.resize(COLS, ROWS);

    rogueMain();

    Term.end();
}

static void curses_plotChar(uchar ch,
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

    #ifdef USE_UNICODE
    // because we can't look at unicode and ascii without messing with Rogue.h, reinterpret until some later version comes along:
    switch (ch) {
    case FLOOR_CHAR: ch = '.'; break;
    case CHASM_CHAR: ch = ':'; break;
    case TRAP_CHAR: ch = '%'; break;
    case FIRE_CHAR: ch = '^'; break;
    case FOLIAGE_CHAR: ch = '&'; break;
    case AMULET_CHAR: ch = ','; break;
    case SCROLL_CHAR: ch = '?'; break;
    case RING_CHAR: ch = '='; break;
    case WEAPON_CHAR: ch = '('; break;
    case GEM_CHAR: ch = '+'; break;
    case TOTEM_CHAR: ch = '0'; break;
    case BAD_MAGIC_CHAR: ch = '+'; break;
    case GOOD_MAGIC_CHAR: ch = '$'; break;

    // case UP_ARROW_CHAR: ch = '^'; break; // same as WEAPON_CHAR
    case DOWN_ARROW_CHAR: ch = 'v'; break;
    case LEFT_ARROW_CHAR: ch = '<'; break;
    case RIGHT_ARROW_CHAR: ch = '>'; break;

    case UP_TRIANGLE_CHAR: ch = '^'; break;
    case DOWN_TRIANGLE_CHAR: ch = 'v'; break;

    case CHARM_CHAR: ch = '7'; break;

    case OMEGA_CHAR: ch = '<'; break;
    case THETA_CHAR: ch = '0'; break;
    case LAMDA_CHAR: ch = '^'; break;
    case KOPPA_CHAR: ch = '0'; break;

    case LOZENGE_CHAR: ch = 'o'; break;
    case CROSS_PRODUCT_CHAR: ch = 'x'; break;

    case STATUE_CHAR: ch = '5'; break;
    case UNICORN_CHAR: ch = 'U'; break;
    }
    #endif

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

static uint32_t getTime() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return 1000 * tp.tv_sec + tp.tv_nsec / 1000000;
}

static boolean curses_pauseForMilliseconds(short milliseconds) {
    Term.refresh();
    Term.wait(milliseconds);

    // hasKey returns true if we have a mouse event, too.
    return Term.hasKey();
}

static void curses_nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    int key;
    // TCOD_mouse_t mouse;
    uint32_t theTime, waitTime;
    // short x, y;

    Term.refresh();

    if (noMenu && rogue.nextGame == NG_NOTHING) rogue.nextGame = NG_NEW_GAME;

    for (;;) {
        theTime = getTime(); //TCOD_sys_elapsed_milli();

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
            returnEvent->controlKey = 0; //(key.rctrl || key.lctrl);
            returnEvent->shiftKey = 0; //key.shift;
            returnEvent->param1 = key;

            if (key == Term.keys.backspace || key == Term.keys.del) returnEvent->param1 = DELETE_KEY;
            else if (key == Term.keys.up) returnEvent->param1 = UP_ARROW;
            else if (key == Term.keys.down) returnEvent->param1 = DOWN_ARROW;
            else if (key == Term.keys.left) returnEvent->param1 = LEFT_ARROW;
            else if (key == Term.keys.right) returnEvent->param1 = RIGHT_ARROW;
            else if (key == Term.keys.quit) {
                rogue.gameHasEnded = true;
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

        waitTime = PAUSE_BETWEEN_EVENT_POLLING + theTime - getTime();

        if (waitTime > 0 && waitTime <= PAUSE_BETWEEN_EVENT_POLLING) {
            curses_pauseForMilliseconds(waitTime);
        }
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
    modifier_held
};
#endif


#include "Rogue.h"

struct brogueConsole {
    /*
    The platform entrypoint, called by the main function. Should initialize
    and then call rogueMain.
    */
    void (*gameLoop)();

    /*
    Pause the game, returning a boolean specifying whether an input event
    is available for receiving with nextKeyOrMouseEvent.
    */
    boolean (*pauseForMilliseconds)(short milliseconds);

    /*
    Block until an event is available and then update returnEvent with
    its details. textInput is true iff a text-entry box is active. See
    sdl2-platform.c for the boilerplate for colorsDance.
    */
    void (*nextKeyOrMouseEvent)(rogueEvent *returnEvent, boolean textInput, boolean colorsDance);

    /*
    Draw a character at a location with a specific color.
    */
    void (*plotChar)(
        uchar inputChar,
        short x, short y,
        short foreRed, short foreGreen, short foreBlue,
        short backRed, short backGreen, short backBlue
    );

    void (*remap)(const char *, const char *);

    /*
    Returns whether a keyboard modifier is active -- 0 for Shift, 1 for Ctrl.
    */
    boolean (*modifierHeld)(int modifier);
};

void loadKeymap();

#ifdef BROGUE_SDL
extern struct brogueConsole sdlConsole;
#endif

#ifdef BROGUE_CURSES
extern struct brogueConsole cursesConsole;
#endif

extern struct brogueConsole currentConsole;
extern boolean noMenu;


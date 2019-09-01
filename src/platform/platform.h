#include "Rogue.h"

struct brogueConsole {
    void (*gameLoop)();
    boolean (*pauseForMilliseconds)(short milliseconds);
    void (*nextKeyOrMouseEvent)(rogueEvent *returnEvent, boolean textInput, boolean colorsDance);
    void (*plotChar)(uchar, short, short, short, short, short, short, short, short);
    void (*remap)(const char *, const char *);
    boolean (*modifierHeld)(int modifier);
};

void loadKeymap();

#ifdef BROGUE_TCOD
extern struct brogueConsole tcodConsole;
#endif

#ifdef BROGUE_CURSES
extern struct brogueConsole cursesConsole;
#endif

extern struct brogueConsole currentConsole;
extern boolean serverMode;
extern boolean noMenu;


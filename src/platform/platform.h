#include "Rogue.h"

#define MIDDLE_DOT  0x00b7
#define FOUR_DOTS  0x2237
#define DIAMOND  0x25c7
#define FLIPPED_V  0x22CF
#define ARIES  0x2648
#define ESZETT  0x00df
#define ANKH  0x2640
#define MUSIC_NOTE  0x266A
#define CIRCLE  0x26AA
#define LIGHTNING_BOLT  0x03DF
#define UP_ARROW  0x2191
#define FILLED_CIRCLE  0x25cf
#define NEUTER  0x26b2
#define U_ACUTE  0x00da
#define CURRENCY 0x00A4
#define UP_ARROW  0x2191
#define DOWN_ARROW  0x2193
#define LEFT_ARROW  0x2190
#define RIGHT_ARROW  0x2192
#define UP_TRIANGLE  0x2206
#define DOWN_TRIANGLE  0x2207
#define OMEGA  0x03A9
#define THETA  0x03B8
#define LAMDA  0x03BB
#define KOPPA  0x03DE
#define LOZENGE  0x29EB
#define CROSS_PRODUCT  0x2A2F
#define CIRCLE_BARS  0x29F2
#define FILLED_CIRCLE_BARS  0x29F3

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

    /*
    Notifies the platform code of an event during the game - e.g. victory
    */
    void (*notifyEvent)(short eventId, int data1, int data2, const char *str1, const char *str2);
};

// defined in platform
void loadKeymap();
void dumpScores();

#ifdef BROGUE_SDL
extern struct brogueConsole sdlConsole;
#endif

#ifdef BROGUE_CURSES
extern struct brogueConsole cursesConsole;
#endif

#ifdef BROGUE_WEB
extern struct brogueConsole webConsole;
#endif

extern struct brogueConsole currentConsole;
extern boolean noMenu;
extern int brogueFontSize;
extern char dataDirectory[];

// defined in brogue
extern playerCharacter rogue;


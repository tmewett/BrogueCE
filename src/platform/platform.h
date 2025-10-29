#include "Rogue.h"

#define U_MIDDLE_DOT  0x00b7
#define U_FOUR_DOTS  0x2237
#define U_DIAMOND  0x25c7
#define U_FLIPPED_V  0x22CF
#define U_ARIES  0x2648
#define U_ESZETT  0x00df
#define U_ANKH  0x2640
#define U_MUSIC_NOTE  0x266A
#define U_CIRCLE  0x26AA
#define U_LIGHTNING_BOLT  0x03DF
#define U_FILLED_CIRCLE  0x25cf
#define U_NEUTER  0x26b2
#define U_U_ACUTE  0x00da
#define U_CURRENCY 0x00A4
#define U_UP_ARROW  0x2191
#define U_DOWN_ARROW  0x2193
#define U_LEFT_ARROW  0x2190
#define U_RIGHT_ARROW  0x2192
#define U_OMEGA  0x03A9
#define U_CIRCLE_BARS  0x29F2
#define U_FILLED_CIRCLE_BARS  0x29F3
#define U_LEFT_TRIANGLE 0x1F780

// #define U_UP_TRIANGLE  0x2206
// #define U_DOWN_TRIANGLE  0x2207
// #define U_THETA  0x03B8
// #define U_LAMDA  0x03BB
// #define U_KOPPA  0x03DE
// #define U_LOZENGE  0x29EB
// #define U_CROSS_PRODUCT  0x2A2F

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
    boolean (*pauseForMilliseconds)(short milliseconds, PauseBehavior behavior);

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
        enum displayGlyph inputChar,
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
    Optional. Notifies the platform code of an event during the game - e.g. victory
    */
    void (*notifyEvent)(short eventId, int data1, int data2, const char *str1, const char *str2);

    /*
    Optional. Take a screenshot in current working directory
    */
    boolean (*takeScreenshot)();

    /*
    Optional. Enables or disables graphical tiles, returning the new state. This
    is called when the user changes the option in-game. It is also called at the
    very start of the program, even before .gameLoop, to set the initial value.
    */
    enum graphicsModes (*setGraphicsMode)(enum graphicsModes mode);
};

// defined in platform
void loadKeymap(void);
void dumpScores(void);
unsigned int glyphToUnicode(enum displayGlyph glyph);
boolean isEnvironmentGlyph(enum displayGlyph glyph);
void setHighScoresFilename(char *buffer, int bufferMaxLength);

#ifdef BROGUE_SDL
extern struct brogueConsole sdlConsole;
extern int windowWidth;
extern int windowHeight;
extern boolean fullScreen;
extern boolean softwareRendering;
#endif

#ifdef BROGUE_CURSES
extern struct brogueConsole cursesConsole;
#endif

#ifdef BROGUE_WEB
extern struct brogueConsole webConsole;
#endif

extern struct brogueConsole nullConsole;

extern struct brogueConsole currentConsole;
extern boolean noMenu;
extern char dataDirectory[];

// defined in brogue
extern playerCharacter rogue;


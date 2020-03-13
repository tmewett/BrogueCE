#include "platform.h"

// Expanding a macro as a string constant requires two levels of macros
#define _str(x)  #x
#define STRINGIFY(x)  _str(x)

struct brogueConsole currentConsole;

int brogueFontSize = 0;
char dataDirectory[BROGUE_FILENAME_MAX] = STRINGIFY(DATADIR);
boolean serverMode = false;

static boolean endswith(const char *str, const char *ending)
{
    int str_len = strlen(str), ending_len = strlen(ending);
    if (str_len < ending_len) return false;
    return strcmp(str + str_len - ending_len, ending) == 0 ? true : false;
}

static void append(char *str, char *ending, int bufsize) {
    int str_len = strlen(str), ending_len = strlen(ending);
    if (str_len + ending_len + 1 > bufsize) return;
    strcpy(str + str_len, ending);
}

static void printCommandlineHelp() {
    printf("%s",
    "--help         -h          print this help message\n"
    "--version      -V          print the version (i.e., " BROGUE_VERSION_STRING ")\n"
    "--scores                   dump scores to output and exit immediately\n"
    "-n                         start a new game, skipping the menu\n"
    "-s seed                    start a new game with the specified numerical seed\n"
    "-o filename[.broguesave]   open a save file (extension optional)\n"
    "-v recording[.broguerec]   view a recording (extension optional)\n"
#ifdef BROGUE_WEB
    "--server-mode              run the game in web-brogue server mode\n"
#endif
#ifdef BROGUE_SDL
    "--size N                   starts the game at font size N (1 to 13)\n"
#endif
#ifdef BROGUE_CURSES
    "--term         -t          run in ncurses-based terminal mode\n"
#endif
    "--print-seed-catalog       prints a catalog of the first five levels of seeds 1-1000\n"
    );
    return;
}

static void badArgument(const char *arg) {
    printf("Bad argument: %s\n\n", arg);
    printCommandlineHelp();
}

int main(int argc, char *argv[])
{

#if 0
#define TOD(x)  ((double) (x) / FP_FACTOR)
    fixpt n = FP_FACTOR * 1/10, nint = FP_FACTOR * 1/50;
    for (int i=0; i < 10; i++) {
        printf("sqrt(%.5f) = %.5f\n", TOD(n), TOD(fp_sqrt(n)));
        n += nint;
    }
    exit(0);
#endif

#if 0
#define TOD(x)  ((double) (x) / FP_FACTOR)
    fixpt base = FP_FACTOR * 2, expn = FP_FACTOR * -5, eint = FP_FACTOR * 1;
    for (int i=0; i < 10; i++) {
        printf("%.5f ^ %.5f = %.5f\n", TOD(base), TOD(expn), TOD(fp_pow(base, expn)));
        expn += eint;
    }
    exit(0);
#endif

#ifdef BROGUE_SDL
    currentConsole = sdlConsole;
#elif BROGUE_WEB
    currentConsole = webConsole;
#else
    currentConsole = cursesConsole;
#endif

    rogue.nextGame = NG_NOTHING;
    rogue.nextGamePath[0] = '\0';
    rogue.nextGameSeed = 0;

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--scores") == 0) {
            // just dump the scores and quit!
            dumpScores();
            return 0;
        }

        if (strcmp(argv[i], "--seed") == 0 || strcmp(argv[i], "-s") == 0) {
            // pick a seed!
            if (i + 1 < argc) {
                unsigned int seed = atof(argv[i + 1]); // plenty of precision in a double, and simpler than any other option
                if (seed != 0) {
                    i++;
                    rogue.nextGameSeed = seed;
                    rogue.nextGame = NG_NEW_GAME_WITH_SEED;
                    continue;
                }
            }
        }

        if (strcmp(argv[i], "-n") == 0) {
            if (rogue.nextGameSeed == 0) {
                rogue.nextGame = NG_NEW_GAME;
            } else {
                rogue.nextGame = NG_NEW_GAME_WITH_SEED;
            }
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--open") == 0) {
            if (i + 1 < argc) {
                strncpy(rogue.nextGamePath, argv[i + 1], BROGUE_FILENAME_MAX);
                rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
                rogue.nextGame = NG_OPEN_GAME;

                if (!endswith(rogue.nextGamePath, GAME_SUFFIX)) {
                    append(rogue.nextGamePath, GAME_SUFFIX, BROGUE_FILENAME_MAX);
                }

                i++;
                continue;
            }
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--view") == 0) {
            if (i + 1 < argc) {
                strncpy(rogue.nextGamePath, argv[i + 1], BROGUE_FILENAME_MAX);
                rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
                rogue.nextGame = NG_VIEW_RECORDING;

                if (!endswith(rogue.nextGamePath, RECORDING_SUFFIX)) {
                    append(rogue.nextGamePath, RECORDING_SUFFIX, BROGUE_FILENAME_MAX);
                }

                i++;
                continue;
            }
        }

        if (strcmp(argv[i], "--print-seed-catalog") == 0) {
            rogue.nextGame = NG_SCUM;
            continue;
        }

        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s\n", BROGUE_VERSION_STRING);
            return 0;
        }

        if (!(strcmp(argv[i], "-?") && strcmp(argv[i], "-h") && strcmp(argv[i], "--help"))) {
            printCommandlineHelp();
            return 0;
        }

#ifdef BROGUE_SDL
        if (strcmp(argv[i], "--size") == 0) {
            // pick a font size
            int size = atoi(argv[i + 1]);
            if (size != 0) {
                i++;
                brogueFontSize = size;
                continue;
            };
        }
#endif

#ifdef BROGUE_CURSES
        if (strcmp(argv[i], "--term") == 0 || strcmp(argv[i], "-t") == 0) {
            currentConsole = cursesConsole;
            continue;
        }
#endif

#ifdef BROGUE_WEB
        if(strcmp(argv[i], "--server-mode") == 0) {
            currentConsole = webConsole;
            rogue.nextGame = NG_NEW_GAME;
            serverMode = true;
            continue;
        }
#endif

        // maybe it ends with .broguesave or .broguerec, then?
        if (endswith(argv[i], GAME_SUFFIX)) {
            strncpy(rogue.nextGamePath, argv[i], BROGUE_FILENAME_MAX);
            rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
            rogue.nextGame = NG_OPEN_GAME;
            continue;
        }

        if (endswith(argv[i], RECORDING_SUFFIX)) {
            strncpy(rogue.nextGamePath, argv[i], BROGUE_FILENAME_MAX);
            rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
            rogue.nextGame = NG_VIEW_RECORDING;
            continue;
        }

        badArgument(argv[i]);
        return 1;
    }

    loadKeymap();
    currentConsole.gameLoop();

    return 0;
}


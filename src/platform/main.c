#include <math.h>
#include <limits.h>
#include "platform.h"

#ifndef DATADIR
#error "The DATADIR macro is undefined."
#endif

struct brogueConsole currentConsole;

char dataDirectory[BROGUE_FILENAME_MAX] = STRINGIFY(DATADIR);
boolean serverMode = false;
boolean nonInteractivePlayback = false;
boolean hasGraphics = false;
enum graphicsModes graphicsMode = TEXT_GRAPHICS;
boolean isCsvFormat = false;

static void printCommandlineHelp() {
    printf("%s",
    "--help         -h          print this help message\n"
    "--version      -V          print the version (i.e., " BROGUE_VERSION_STRING ")\n"
    "--scores                   dump scores to output and exit immediately\n"
    "-n                         start a new game, skipping the menu\n"
    "-s seed                    start a new game with the specified numerical seed\n"
    "-o filename[.broguesave]   open a save file (extension optional)\n"
    "-v recording[.broguerec]   view a recording (extension optional)\n"
    "-vn recording[.broguerec]  view a recording non-interactively (extension optional)\n"
#ifdef BROGUE_WEB
    "--server-mode              run the game in web-brogue server mode\n"
#endif
#ifdef BROGUE_SDL
    "--size N                   starts the game at font size N (1 to 20)\n"
    "--graphics     -G          enable graphical tiles\n"
    "--hybrid       -H          enable hybrid graphics\n"
    "--full-screen  -F          enable full screen\n"
    "--no-gpu                   disable hardware-accelerated graphics and HiDPI\n"
#endif
#ifdef BROGUE_CURSES
    "--term         -t          run in ncurses-based terminal mode\n"
#endif
    "--stealth      -S          display stealth range\n"
    "--no-effects   -E          disable color effects\n"
    "--wizard       -W          run in wizard mode, invincible with powerful items\n"
    "[--csv] --print-seed-catalog [START NUM LEVELS]\n"
    "                           (optional csv format)\n"
    "                           prints a catalog of the first LEVELS levels of NUM\n"
    "                           seeds from seed START (defaults: 1 1000 5)\n"
    "--data-dir DIRECTORY       specify directory containing game resources (experimental)\n"
    );
    return;
}

static void badArgument(const char *arg) {
    printf("Bad argument: %s\n\n", arg);
    printCommandlineHelp();
}

boolean tryParseUint64(char *str, uint64_t *num) {
    unsigned long long n;
    char buf[100];
    if (strlen(str)                 // we need some input
        && sscanf(str, "%llu", &n)  // try to convert to number
        && sprintf(buf, "%llu", n)  // convert back to string
        && !strcmp(buf, str)) {     // compare (we need them equal)
        *num = (uint64_t)n;
        return true; // success
    } else {
        return false; // input was too large or not a decimal number
    }
}

int main(int argc, char *argv[])
{

#if 0
#define TOD(x)  ((double) (x) / FP_FACTOR)
    fixpt y, x1 = 1, x2 = FP_FACTOR * 70 / 100;
    for (int i=0; i < 10; i++) {
        y = fp_pow(x2, x1); printf("%.5f ^ %i = %.5f  (%lli)\n", TOD(x2), x1, TOD(y), y);
        // y = fp_sqrt(x1); printf("sqrt(%.5f) = %.5f  (%lli)\n", TOD(x1), TOD(y), y);
        x1 += 1;
    }
    exit(0);
#endif

#ifdef BROGUE_SDL
    currentConsole = sdlConsole;
#elif BROGUE_WEB
    currentConsole = webConsole;
#elif BROGUE_CURSES
    currentConsole = cursesConsole;
#else
    currentConsole = nullConsole;
#endif

    rogue.nextGame = NG_NOTHING;
    rogue.nextGamePath[0] = '\0';
    rogue.nextGameSeed = 0;
    rogue.wizard = false;
    rogue.displayStealthRangeMode = false;
    rogue.trueColorMode = false;

    enum graphicsModes initialGraphics = TEXT_GRAPHICS;

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--scores") == 0) {
            // just dump the scores and quit!
            dumpScores();
            return 0;
        }

        if (strcmp(argv[i], "--seed") == 0 || strcmp(argv[i], "-s") == 0) {
            // pick a seed!
            uint64_t seed;
            if (i + 1 == argc || !tryParseUint64(argv[i + 1], &seed)) {
                printf("Invalid seed, please specify a number between 1 and 18446744073709551615\n");
                return 1;
            }
            if (seed != 0) {
                rogue.nextGameSeed = seed;
                rogue.nextGame = NG_NEW_GAME_WITH_SEED;
            }
            i++;
            continue;
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

        if (strcmp(argv[i], "-vn") == 0) {
            if (i + 1 < argc) {
                strncpy(rogue.nextGamePath, argv[i + 1], BROGUE_FILENAME_MAX);
                rogue.nextGamePath[BROGUE_FILENAME_MAX - 1] = '\0';
                rogue.nextGame = NG_VIEW_RECORDING;

                if (!endswith(rogue.nextGamePath, RECORDING_SUFFIX)) {
                    append(rogue.nextGamePath, RECORDING_SUFFIX, BROGUE_FILENAME_MAX);
                }

                currentConsole = nullConsole;
                nonInteractivePlayback = true;

                i++;
                continue;
            }
        }

        if (strcmp(argv[i], "--print-seed-catalog") == 0) {
            if (i + 3 < argc) {
                uint64_t startingSeed, numberOfSeeds;
                // Use converter for the type the next size up, because it returns signed
                unsigned int numberOfLevels = atol(argv[i + 3]);

                if (tryParseUint64(argv[i+1], &startingSeed) && tryParseUint64(argv[i+2], &numberOfSeeds)
                        && startingSeed > 0 && numberOfLevels <= 40) {
                    printSeedCatalog(startingSeed, numberOfSeeds, numberOfLevels, isCsvFormat);
                    return 0;
                }
            } else {
                printSeedCatalog(1, 1000, 5, isCsvFormat);
                return 0;
            }
        }

        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s\n", BROGUE_VERSION_STRING);
            return 0;
        }

        if (!(strcmp(argv[i], "-?") && strcmp(argv[i], "-h") && strcmp(argv[i], "--help"))) {
            printCommandlineHelp();
            return 0;
        }

        if (strcmp(argv[i], "-G") == 0 || strcmp(argv[i], "--graphics") == 0) {
            initialGraphics = TILES_GRAPHICS;  // we call setGraphicsMode later
            continue;
        }

        if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--hybrid") == 0) {
            initialGraphics = HYBRID_GRAPHICS;  // we call setGraphicsMode later
            continue;
        }

        if (strcmp(argv[i], "--csv") == 0 ) {
            isCsvFormat = true;  // we call printSeedCatalog later
            continue;
        }

#ifdef BROGUE_SDL
        if (strcmp(argv[i], "--size") == 0) {
            if (i + 1 < argc) {
                int size = atoi(argv[i + 1]);
                if (size > 0 && size <= 20) {
                    windowWidth = round(pow(1.1, size) * 620.);
                    // Height set automatically
                };

                i++;
                continue;
            }
        }

        if (strcmp(argv[i], "-F") == 0 || strcmp(argv[i], "--full-screen") == 0) {
            fullScreen = true;
            continue;
        }

        if (strcmp(argv[i], "--no-gpu") == 0) {
            softwareRendering = true;
            continue;
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

        if (strcmp(argv[i], "--stealth") == 0 || strcmp(argv[i], "-S") == 0) {
            rogue.displayStealthRangeMode = true;
            continue;
        }

        if (strcmp(argv[i], "--no-effects") == 0 || strcmp(argv[i], "-E") == 0) {
            rogue.trueColorMode = true;
            continue;
        }

        if (strcmp(argv[i], "--wizard") == 0 || strcmp(argv[i], "-W") == 0) {
            rogue.wizard = true;
            continue;
        }

        if (strcmp(argv[i], "--data-dir") == 0) {
            if (i + 1 < argc) {
                strcpy(dataDirectory, argv[++i]);
                continue;
            }
        }

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

    hasGraphics = (currentConsole.setGraphicsMode != NULL);
    // Now actually set graphics. We do this to ensure there is exactly one
    // call, whether true or false
    graphicsMode = setGraphicsMode(initialGraphics);

    loadKeymap();
    currentConsole.gameLoop();

    return 0;
}


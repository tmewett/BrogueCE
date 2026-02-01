/*
 *  platformdependent.c
 *  Brogue
 *
 *  Created by Brian Walker on 4/13/10.
 *  Copyright 2010. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  Brogue is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Brogue is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Brogue.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "platform.h"
#include "GlobalsBase.h"

typedef struct brogueScoreEntry {
    long int score;
    long int dateNumber; // in seconds
    char dateText[COLS]; // in the form mm/dd/yy
    char description[COLS];
} brogueScoreEntry;

brogueScoreEntry scoreBuffer[HIGH_SCORES_COUNT];

unsigned int glyphToUnicode(enum displayGlyph glyph) {
    if (glyph < 128) return glyph;

    switch (glyph) {
        case G_UP_ARROW: return U_UP_ARROW;
        case G_DOWN_ARROW: return U_DOWN_ARROW;
        case G_POTION: return '!';
        case G_GRASS: return '"';
        case G_WALL: return '#';
        case G_DEMON: return '&';
        case G_OPEN_DOOR: return '\'';
        case G_GOLD: return '*';
        case G_CLOSED_DOOR: return '+';
        case G_RUBBLE: return ',';
        case G_KEY: return '-';
        case G_BOG: return '~';
        case G_CHAIN_TOP_LEFT:
        case G_CHAIN_BOTTOM_RIGHT:
            return '\\';
        case G_CHAIN_TOP_RIGHT:
        case G_CHAIN_BOTTOM_LEFT:
            return '/';
        case G_CHAIN_TOP:
        case G_CHAIN_BOTTOM:
            return '|';
        case G_CHAIN_LEFT:
        case G_CHAIN_RIGHT:
            return '-';
        case G_FOOD: return ';';
        case G_UP_STAIRS: return '<';
        case G_VENT: return '=';
        case G_DOWN_STAIRS: return '>';
        case G_PLAYER: return '@';
        case G_BOG_MONSTER: return 'B';
        case G_CENTAUR: return 'C';
        case G_DRAGON: return 'D';
        case G_FLAMEDANCER: return 'F';
        case G_GOLEM: return 'G';
        case G_TENTACLE_HORROR: return 'H';
        case G_IFRIT: return 'I';
        case G_JELLY: return 'J';
        case G_KRAKEN: return 'K';
        case G_LICH: return 'L';
        case G_NAGA: return 'N';
        case G_OGRE: return 'O';
        case G_PHANTOM: return 'P';
        case G_REVENANT: return 'R';
        case G_SALAMANDER: return 'S';
        case G_TROLL: return 'T';
        case G_UNDERWORM: return 'U';
        case G_VAMPIRE: return 'V';
        case G_WRAITH: return 'W';
        case G_ZOMBIE: return 'Z';
        case G_ARMOR: return '[';
        case G_STAFF: return '/';
        case G_WEB: return ':';
        case G_MOUND: return 'a';
        case G_BLOAT: return 'b';
        case G_CENTIPEDE: return 'c';
        case G_DAR_BLADEMASTER: return 'd';
        case G_EEL: return 'e';
        case G_FURY: return 'f';
        case G_GOBLIN: return 'g';
        case G_IMP: return 'i';
        case G_JACKAL: return 'j';
        case G_KOBOLD: return 'k';
        case G_MONKEY: return 'm';
        case G_PIXIE: return 'p';
        case G_RAT: return 'r';
        case G_SPIDER: return 's';
        case G_TOAD: return 't';
        case G_BAT: return 'v';
        case G_WISP: return 'w';
        case G_PHOENIX: return 'P';
        case G_ALTAR: return '|';
        case G_LIQUID: return '~';
        case G_FLOOR: return U_MIDDLE_DOT;
        case G_CHASM: return U_FOUR_DOTS;
        case G_TRAP: return U_DIAMOND;
        case G_FIRE: return U_FLIPPED_V;
        case G_FOLIAGE: return U_ARIES;
        case G_AMULET: return U_ANKH;
        case G_SCROLL: return U_MUSIC_NOTE;
        case G_RING: return U_CIRCLE;
        case G_WEAPON: return U_UP_ARROW;
        case G_GEM: return U_FILLED_CIRCLE;
        case G_TOTEM: return U_NEUTER;
        case G_GOOD_MAGIC: return U_FILLED_CIRCLE_BARS;
        case G_BAD_MAGIC: return U_CIRCLE_BARS;
        case G_DOORWAY: return U_OMEGA;
        case G_CHARM: return U_LIGHTNING_BOLT;
        case G_WALL_TOP: return '#';
        case G_DAR_PRIESTESS: return 'd';
        case G_DAR_BATTLEMAGE: return 'd';
        case G_GOBLIN_MAGIC: return 'g';
        case G_GOBLIN_CHIEFTAN: return 'g';
        case G_OGRE_MAGIC: return 'O';
        case G_GUARDIAN: return U_ESZETT;
        case G_WINGED_GUARDIAN: return U_ESZETT;
        case G_EGG: return U_FILLED_CIRCLE;
        case G_WARDEN: return 'Y';
        case G_DEWAR: return '&';
        case G_ANCIENT_SPIRIT: return 'M';
        case G_LEVER: return '/';
        case G_LEVER_PULLED: return '\\';
        case G_BLOODWORT_STALK: return U_ARIES;
        case G_FLOOR_ALT: return U_MIDDLE_DOT;
        case G_UNICORN: return U_U_ACUTE;
        case G_TURRET: return U_FILLED_CIRCLE;
        case G_WAND: return '~';
        case G_GRANITE: return '#';
        case G_CARPET: return U_MIDDLE_DOT;
        case G_CLOSED_IRON_DOOR: return '+';
        case G_OPEN_IRON_DOOR: return '\'';
        case G_TORCH: return '#';
        case G_CRYSTAL: return '#';
        case G_PORTCULLIS: return '#';
        case G_BARRICADE: return '#';
        case G_STATUE: return U_ESZETT;
        case G_CRACKED_STATUE: return U_ESZETT;
        case G_CLOSED_CAGE: return '#';
        case G_OPEN_CAGE: return '|';
        case G_PEDESTAL: return '|';
        case G_CLOSED_COFFIN: return '-';
        case G_OPEN_COFFIN: return '-';
        case G_MAGIC_GLYPH: return U_FOUR_DOTS;
        case G_BRIDGE: return '=';
        case G_BONES: return ',';
        case G_ELECTRIC_CRYSTAL: return U_CURRENCY;
        case G_ASHES: return '\'';
        case G_BEDROLL: return '=';
        case G_BLOODWORT_POD: return '*';
        case G_VINE: return ':';
        case G_NET: return ':';
        case G_LICHEN: return '"';
        case G_PIPES: return '+';
        case G_SAC_ALTAR: return '|';
        case G_ORB_ALTAR: return '|';
        case G_LEFT_TRIANGLE: return U_LEFT_TRIANGLE;

        default:
            brogueAssert(false);
            return '?';
    }
}

/*
Tells if a glyph represents part of the environment (true) or an item or creature (false).
*/
boolean isEnvironmentGlyph(enum displayGlyph glyph) {
    switch (glyph) {
        // items
        case G_AMULET: case G_ARMOR: case G_BEDROLL: case G_CHARM:
        case G_DEWAR: case G_EGG: case G_FOOD: case G_GEM: case G_BLOODWORT_POD:
        case G_GOLD: case G_KEY: case G_POTION: case G_RING:
        case G_SCROLL: case G_STAFF: case G_WAND: case G_WEAPON: case G_LEFT_TRIANGLE:
            return false;

        // creatures
        case G_ANCIENT_SPIRIT: case G_BAT: case G_BLOAT: case G_BOG_MONSTER:
        case G_CENTAUR: case G_CENTIPEDE: case G_DAR_BATTLEMAGE: case G_DAR_BLADEMASTER:
        case G_DAR_PRIESTESS: case G_DEMON: case G_DRAGON: case G_EEL:
        case G_FLAMEDANCER: case G_FURY: case G_GOBLIN: case G_GOBLIN_CHIEFTAN:
        case G_GOBLIN_MAGIC: case G_GOLEM: case G_GUARDIAN: case G_IFRIT:
        case G_IMP: case G_JACKAL: case G_JELLY: case G_KOBOLD:
        case G_KRAKEN: case G_LICH: case G_MONKEY: case G_MOUND:
        case G_NAGA: case G_OGRE: case G_OGRE_MAGIC: case G_PHANTOM:
        case G_PHOENIX: case G_PIXIE: case G_PLAYER: case G_RAT:
        case G_REVENANT: case G_SALAMANDER: case G_SPIDER: case G_TENTACLE_HORROR:
        case G_TOAD: case G_TROLL: case G_UNDERWORM: case G_UNICORN:
        case G_VAMPIRE: case G_WARDEN: case G_WINGED_GUARDIAN: case G_WISP:
        case G_WRAITH: case G_ZOMBIE:
            return false;

        // everything else is considered part of the environment
        default:
            return true;
    }
}

void plotChar(enum displayGlyph inputChar,
              short xLoc, short yLoc,
              short foreRed, short foreGreen, short foreBlue,
              short backRed, short backGreen, short backBlue) {
    currentConsole.plotChar(inputChar, xLoc, yLoc, foreRed, foreGreen, foreBlue, backRed, backGreen, backBlue);
}

boolean shiftKeyIsDown(void) {
    return currentConsole.modifierHeld(0);
}
boolean controlKeyIsDown(void) {
    return currentConsole.modifierHeld(1);
}

void nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    currentConsole.nextKeyOrMouseEvent(returnEvent, textInput, colorsDance);
}

boolean pauseForMilliseconds(short milliseconds, PauseBehavior behavior) {
    return currentConsole.pauseForMilliseconds(milliseconds, behavior);
}

void notifyEvent(short eventId, int data1, int data2, const char *str1, const char *str2) {
    if (currentConsole.notifyEvent) {
        currentConsole.notifyEvent(eventId, data1, data2, str1, str2);
    }
}

boolean takeScreenshot(void) {
    if (currentConsole.takeScreenshot) {
        return currentConsole.takeScreenshot();
    } else {
        return false;
    }
}

enum graphicsModes setGraphicsMode(enum graphicsModes mode) {
    if (currentConsole.setGraphicsMode) {
        return currentConsole.setGraphicsMode(mode);
    } else {
        return TEXT_GRAPHICS;
    }
}

// creates an empty high scores file
static void initScores(void) {
    short i;
    FILE *scoresFile;
    char highScoresFilename[BROGUE_FILENAME_MAX];

    setHighScoresFilename(highScoresFilename, BROGUE_FILENAME_MAX);

    scoresFile = fopen(highScoresFilename, "w");
    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        fprintf(scoresFile, "%li\t%li\t%s", (long) 0, (long) 0, "(empty entry)\n");
    }
    fclose(scoresFile);
}

// sorts the entries of the scoreBuffer global variable by score in descending order;
// returns the sorted line number of the most recent entry
static short sortScoreBuffer(void) {
    short i, j, highestUnsortedLine, mostRecentSortedLine = 0;
    long highestUnsortedScore, mostRecentDate;
    brogueScoreEntry sortedScoreBuffer[HIGH_SCORES_COUNT];
    boolean lineSorted[HIGH_SCORES_COUNT];

    mostRecentDate = 0;

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        lineSorted[i] = false;
    }

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        highestUnsortedLine = 0;
        highestUnsortedScore = 0;
        for (j=0; j<HIGH_SCORES_COUNT; j++) {
            if (!lineSorted[j] && scoreBuffer[j].score >= highestUnsortedScore) {
                highestUnsortedLine = j;
                highestUnsortedScore = scoreBuffer[j].score;
            }
        }
        sortedScoreBuffer[i] = scoreBuffer[highestUnsortedLine];
        lineSorted[highestUnsortedLine] = true;
    }

    // copy the sorted list back into scoreBuffer, remember the most recent entry
    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        scoreBuffer[i] = sortedScoreBuffer[i];
        if (scoreBuffer[i].dateNumber > mostRecentDate) {
            mostRecentDate = scoreBuffer[i].dateNumber;
            mostRecentSortedLine = i;
        }
    }
    return mostRecentSortedLine;
}

void setHighScoresFilename(char *buffer, int bufferMaxLength) {
    strncpy(buffer, gameConst->variantName, bufferMaxLength);
    strncat(buffer, "HighScores.txt", bufferMaxLength);
    buffer[0] = toupper(buffer[0]);
}

// loads the ([V]ariantName)HighScores.txt file into the scoreBuffer global variable
// score file format is: score, tab, date in seconds, tab, description, newline.
static short loadScoreBuffer(void) {
    short i;
    FILE *scoresFile;
    time_t rawtime;
    struct tm * timeinfo;

    char highScoresFilename[BROGUE_FILENAME_MAX];
    setHighScoresFilename(highScoresFilename, BROGUE_FILENAME_MAX);

    scoresFile = fopen(highScoresFilename, "r");

    if (scoresFile == NULL) {
        initScores();
        scoresFile = fopen(highScoresFilename, "r");
    }

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        // load score and also the date in seconds
        fscanf(scoresFile, "%li\t%li\t", &(scoreBuffer[i].score), &(scoreBuffer[i].dateNumber));

        // load description
        fgets(scoreBuffer[i].description, COLS, scoresFile);
        // strip the newline off the end
        scoreBuffer[i].description[strlen(scoreBuffer[i].description) - 1] = '\0';

        // convert date to DATE_FORMAT
        rawtime = (time_t) scoreBuffer[i].dateNumber;
        timeinfo = localtime(&rawtime);
        strftime(scoreBuffer[i].dateText, DCOLS, DATE_FORMAT, timeinfo);
    }
    fclose(scoresFile);
    return sortScoreBuffer();
}

void loadKeymap(void) {
    int i;
    FILE *f;
    char buffer[512];

    f = fopen("keymap.txt", "r");

    if (f != NULL) {
        while (fgets(buffer, 512, f) != NULL) {
            // split it in two (destructively)
            int mode = 1;
            char *input_name = NULL, *output_name = NULL;
            for (i = 0; buffer[i]; i++) {
                if (isspace(buffer[i])) {
                    buffer[i] = '\0';
                    mode = 1;
                } else {
                    if (mode) {
                        if (input_name == NULL) input_name = buffer + i;
                        else if (output_name == NULL) output_name = buffer + i;
                    }
                    mode = 0;
                }
            }
            if (input_name != NULL && output_name != NULL) {
                if (input_name[0] == '#') continue; // must be a comment

                if (currentConsole.remap) {
                    currentConsole.remap(input_name, output_name);
                }
            }
        }
        fclose(f);
    }
}


// saves the scoreBuffer global variable into the BrogueHighScores.txt file,
// thus overwriting whatever is already there.
// The numerical version of the date is what gets saved; the "mm/dd/yy" version is ignored.
// Does NOT do any sorting.
static void saveScoreBuffer(void) {
    short i;
    FILE *scoresFile;
    char highScoresFilename[BROGUE_FILENAME_MAX];

    setHighScoresFilename(highScoresFilename, BROGUE_FILENAME_MAX);

    scoresFile = fopen(highScoresFilename, "w");

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        // save the entry
        fprintf(scoresFile, "%li\t%li\t%s\n", scoreBuffer[i].score, scoreBuffer[i].dateNumber, scoreBuffer[i].description);
    }

    fclose(scoresFile);
}

void dumpScores(void) {
    int i;

    rogueHighScoresEntry list[HIGH_SCORES_COUNT];
    getHighScoresList(list);

    for (i = 0; i < HIGH_SCORES_COUNT; i++) {
        if (list[i].score > 0) {
            printf("%d\t%s\t%s\n", (int) list[i].score, list[i].date, list[i].description);
        }
    }
}

short getHighScoresList(rogueHighScoresEntry returnList[HIGH_SCORES_COUNT]) {
    short i, mostRecentLineNumber;

    mostRecentLineNumber = loadScoreBuffer();

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        returnList[i].score =               scoreBuffer[i].score;
        strcpy(returnList[i].date,          scoreBuffer[i].dateText);
        strcpy(returnList[i].description,   scoreBuffer[i].description);
    }

    return mostRecentLineNumber;
}

boolean saveHighScore(rogueHighScoresEntry theEntry) {
    short i, lowestScoreIndex = -1;
    long lowestScore = -1;

    loadScoreBuffer();

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        if (scoreBuffer[i].score < lowestScore || i == 0) {
            lowestScore = scoreBuffer[i].score;
            lowestScoreIndex = i;
        }
    }

    if (lowestScore > theEntry.score) {
        return false;
    }

    scoreBuffer[lowestScoreIndex].score =               theEntry.score;
    scoreBuffer[lowestScoreIndex].dateNumber =          (long) time(NULL);
    strcpy(scoreBuffer[lowestScoreIndex].description,   theEntry.description);

    saveScoreBuffer();

    return true;
}

/// @brief Sets the name of the run history file based on the variant
/// @param buffer The filename
/// @param bufferMaxLength The maximum filename length
static void setRunHistoryFilename(char *buffer, int bufferMaxLength) {
    strncpy(buffer, gameConst->variantName, bufferMaxLength);
    strncat(buffer, "RunHistory.txt", bufferMaxLength);
    buffer[0] = toupper(buffer[0]);
}

/// @brief Saves the run to the history file at the end of a game
/// @param result The game result (Escaped, Mastered, Died, Quit)
/// @param killedBy How the player died (monster name, etc.) 
/// @param score The total score
/// @param lumenstones The number of lumenstones collected
void saveRunHistory(char *result, char *killedBy, int score, int lumenstones) {
    FILE *runHistoryFile;
    char runHistoryFilename[BROGUE_FILENAME_MAX];

    setRunHistoryFilename(runHistoryFilename, BROGUE_FILENAME_MAX);
    runHistoryFile = fopen(runHistoryFilename, "a"); // append. create if not found.

    fprintf(runHistoryFile, "%llu\t%li\t%s\t%s\t%i\t%i\t%i\t%i\t%i\n", rogue.seed, (long) time(NULL), result, killedBy, 
            score, (int) rogue.gold, lumenstones, (int) rogue.deepestLevel, (int) rogue.playerTurnNumber);
    fclose(runHistoryFile);
}
/// @brief Saves a "reset" run to the history file. This serves to reset the player's recent stats to zero.
void saveResetRun(void) {
    FILE *runHistoryFile;
    char runHistoryFilename[BROGUE_FILENAME_MAX];

    setRunHistoryFilename(runHistoryFilename, BROGUE_FILENAME_MAX);
    runHistoryFile = fopen(runHistoryFilename, "a"); // append. create if not found.

    fprintf(runHistoryFile, "%i\t%li\t%s\t%s\t%i\t%i\t%i\t%i\t%i\n", 0, (long) time(NULL), "Reset", "-", 0, 0, 0, 0, 0);
    fclose(runHistoryFile);
}

/// @brief Loads the run history file
/// @return Linked list of runs
rogueRun* loadRunHistory(void) {
    FILE *runHistoryFile;
    char runHistoryFilename[BROGUE_FILENAME_MAX];

    setRunHistoryFilename(runHistoryFilename, BROGUE_FILENAME_MAX);
    runHistoryFile = fopen(runHistoryFilename, "r"); // read

    if (runHistoryFile == NULL) {
        runHistoryFile = fopen(runHistoryFilename, "w"); // create if not found
        fclose(runHistoryFile);
        runHistoryFile = fopen(runHistoryFilename, "r");
    }

    rogueRun *runHistory = NULL;
    rogueRun *current = NULL;
    char line[1024]; // maximum line length
    while (fgets(line, sizeof(line), runHistoryFile) != NULL) {
        rogueRun *run = (rogueRun *)malloc(sizeof(rogueRun));
        memset(run, '\0', sizeof(rogueRun));
        run->nextRun = NULL;

        int vals = sscanf(line, "%llu\t%li\t%s\t%[^\t]\t%i\t%i\t%i\t%i\t%i\n", &run->seed, &run->dateNumber,
                   run->result, run->killedBy, &run->score, &run->gold, &run->lumenstones,
                   &run->deepestLevel, &run->turns);

        if ( vals == 9) {
            if (runHistory == NULL) {
                runHistory = run;
                current = run;
            } else {
                current->nextRun = run;
                current = run;
            }
        } else {
            fprintf(stderr, "Error parsing line: %s\n", line);
            free(run);
        }
    }
    fclose(runHistoryFile);

    return runHistory;
}

// start of file listing

struct filelist {
    fileEntry *files;
    char *names;

    int nfiles, maxfiles;
    int nextname, maxname;
};

static struct filelist *newFilelist(void) {
    struct filelist *list = malloc(sizeof(*list));

    list->nfiles = 0;
    list->nextname = 0;
    list->maxfiles = 64;
    list->maxname = list->maxfiles * 64;

    list->files = malloc(sizeof(fileEntry) * list->maxfiles);
    list->names = malloc(list->maxname);

    return list;
}

static fileEntry *addfile(struct filelist *list, const char *name) {
    int len = strlen(name);
    if (len + list->nextname >= list->maxname) {
        int newmax = (list->maxname + len) * 2;
        char *newnames = realloc(list->names, newmax);
        if (newnames != NULL) {
            list->names = newnames;
            list->maxname = newmax;
        } else {
            // fail silently
            return NULL;
        }
    }

    if (list->nfiles >= list->maxfiles) {
        int newmax = list->maxfiles * 2;
        fileEntry *newfiles = realloc(list->files, sizeof(fileEntry) * newmax);
        if (newfiles != NULL) {
            list->files = newfiles;
            list->maxfiles = newmax;
        } else {
            // fail silently
            return NULL;
        }
    }

    // add the new file and copy the name into the buffer
    list->files[list->nfiles].path = ((char *) NULL) + list->nextname; // don't look at them until they are transferred out
    list->files[list->nfiles].date = (struct tm) {0}; // associate a dummy date (1899-12-31) to avoid random data, it will be correctly populated when using listFiles()

    strncpy(list->names + list->nextname, name, len + 1);

    list->nextname += len + 1;
    list->nfiles += 1;

    return list->files + (list->nfiles - 1);
}

static void freeFilelist(struct filelist *list) {
    //if (list->names != NULL) free(list->names);
    //if (list->files != NULL) free(list->files);
    free(list);
}

static fileEntry *commitFilelist(struct filelist *list, char **namebuffer) {
    int i;
    /*fileEntry *files = malloc(list->nfiles * sizeof(fileEntry) + list->nextname); // enough space for all the names and all the files

    if (files != NULL) {
        char *names = (char *) (files + list->nfiles);

        for (i=0; i < list->nfiles; i++) {
            files[i] = list->files[i];
            files[i].path = names + (files[i].path - (char *) NULL);
        }

        memcpy(names, list->names, list->nextname);
    }
    */
    for (i=0; i < list->nfiles; i++) {
        list->files[i].path = list->names + (list->files[i].path - (char *) NULL);
    }
    *namebuffer = list->names;

    return list->files;
}

fileEntry *listFiles(short *fileCount, char **namebuffer) {
    struct filelist *list = newFilelist();

    // windows: FindFirstFile/FindNextFile
    DIR *dp= opendir ("./");

    if (dp != NULL) {
        struct dirent *ep;
        struct stat statbuf;
        struct tm *timeinfo;

        while ((ep = readdir(dp))) {
            // get statistics about the file (0 on success)
            if (!stat(ep->d_name, &statbuf)) {
                fileEntry *file = addfile(list, ep->d_name);
                if (file != NULL) {
                    // add the modification date to the file entry
                    timeinfo = localtime(&statbuf.st_mtime);
                    file->date = *timeinfo;
                }
            }
        }

        closedir (dp);
    }
    else {
        *fileCount = 0;
        return NULL;
    }

    fileEntry *files = commitFilelist(list, namebuffer);

    if (files != NULL) {
        *fileCount = (short) list->nfiles;
    } else {
        *fileCount = 0;
    }

    freeFilelist(list);

    return files;
}

// end of file listing

void initializeLaunchArguments(enum NGCommands *command, char *path, uint64_t *seed) {
    // we've actually already done this at this point, except for the seed.
}

boolean isApplicationActive(void) {
    // FIXME: finish
    return true;
}


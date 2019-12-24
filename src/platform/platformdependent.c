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

typedef struct brogueScoreEntry {
    long int score;
    long int dateNumber; // in seconds
    char dateText[COLS]; // in the form mm/dd/yy
    char description[COLS];
} brogueScoreEntry;

brogueScoreEntry scoreBuffer[HIGH_SCORES_COUNT];

void plotChar(uchar inputChar,
              short xLoc, short yLoc,
              short foreRed, short foreGreen, short foreBlue,
              short backRed, short backGreen, short backBlue) {
    currentConsole.plotChar(inputChar, xLoc, yLoc, foreRed, foreGreen, foreBlue, backRed, backGreen, backBlue);
}

void pausingTimerStartsNow() {

}

boolean shiftKeyIsDown() {
    return currentConsole.modifierHeld(0);
}
boolean controlKeyIsDown() {
    return currentConsole.modifierHeld(1);
}

void nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance) {
    currentConsole.nextKeyOrMouseEvent(returnEvent, textInput, colorsDance);
}

boolean pauseForMilliseconds(short milliseconds) {
    return currentConsole.pauseForMilliseconds(milliseconds);
}

void notifyEvent(short eventId, int data1, int data2, const char *str1, const char *str2) {
    currentConsole.notifyEvent(eventId, data1, data2, str1, str2);
}

// creates an empty high scores file
void initScores() {
    short i;
    FILE *scoresFile;

    scoresFile = fopen("BrogueHighScores.txt", "w");
    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        fprintf(scoresFile, "%li\t%li\t%s", (long) 0, (long) 0, "(empty entry)\n");
    }
    fclose(scoresFile);
}

// sorts the entries of the scoreBuffer global variable by score in descending order;
// returns the sorted line number of the most recent entry
short sortScoreBuffer() {
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

// loads the BrogueHighScores.txt file into the scoreBuffer global variable
// score file format is: score, tab, date in seconds, tab, description, newline.
short loadScoreBuffer() {
    short i;
    FILE *scoresFile;
    time_t rawtime;
    struct tm * timeinfo;

    scoresFile = fopen("BrogueHighScores.txt", "r");

    if (scoresFile == NULL) {
        initScores();
        scoresFile = fopen("BrogueHighScores.txt", "r");
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

void loadKeymap() {
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

                currentConsole.remap(input_name, output_name);
            }
        }
        fclose(f);
    }
}


// saves the scoreBuffer global variable into the BrogueHighScores.txt file,
// thus overwriting whatever is already there.
// The numerical version of the date is what gets saved; the "mm/dd/yy" version is ignored.
// Does NOT do any sorting.
void saveScoreBuffer() {
    short i;
    FILE *scoresFile;

    scoresFile = fopen("BrogueHighScores.txt", "w");

    for (i=0; i<HIGH_SCORES_COUNT; i++) {
        // save the entry
        fprintf(scoresFile, "%li\t%li\t%s\n", scoreBuffer[i].score, scoreBuffer[i].dateNumber, scoreBuffer[i].description);
    }

    fclose(scoresFile);
}

void dumpScores() {
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

// start of file listing

struct filelist {
    fileEntry *files;
    char *names;

    int nfiles, maxfiles;
    int nextname, maxname;
};

struct filelist *newFilelist() {
    struct filelist *list = malloc(sizeof(*list));

    list->nfiles = 0;
    list->nextname = 0;
    list->maxfiles = 64;
    list->maxname = list->maxfiles * 64;

    list->files = malloc(sizeof(fileEntry) * list->maxfiles);
    list->names = malloc(list->maxname);

    return list;
}

fileEntry *addfile(struct filelist *list, const char *name) {
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

void freeFilelist(struct filelist *list) {
    //if (list->names != NULL) free(list->names);
    //if (list->files != NULL) free(list->files);
    free(list);
}

fileEntry *commitFilelist(struct filelist *list, char **namebuffer) {
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

void initializeLaunchArguments(enum NGCommands *command, char *path, unsigned long *seed) {
    // we've actually already done this at this point, except for the seed.
}

boolean isApplicationActive(void) {
    // FIXME: finish
    return true;
}


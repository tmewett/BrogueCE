/*
 *  Recordings.c
 *  Brogue
 *
 *  Created by Brian Walker on 8/8/10.
 *  Copyright 2012. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <time.h>
#include <math.h>
#include <limits.h>
#include "Rogue.h"
#include "IncludeGlobals.h"

#define RECORDING_HEADER_LENGTH     32  // bytes at the start of the recording file to store global data

static const int keystrokeTable[] = {UP_ARROW, LEFT_ARROW, DOWN_ARROW, RIGHT_ARROW,
    ESCAPE_KEY, RETURN_KEY, DELETE_KEY, TAB_KEY, NUMPAD_0, NUMPAD_1,
    NUMPAD_2, NUMPAD_3, NUMPAD_4, NUMPAD_5, NUMPAD_6, NUMPAD_7, NUMPAD_8, NUMPAD_9};


void recordChar(unsigned char c) {
    inputRecordBuffer[locationInRecordingBuffer++] = c;
    recordingLocation++;
}

void considerFlushingBufferToFile() {
    if (locationInRecordingBuffer >= INPUT_RECORD_BUFFER) {
        flushBufferToFile();
    }
}

// compresses a int into a char, discarding stuff we don't need
unsigned char compressKeystroke(int c) {
    short i;

    for (i=0; i<18; i++) {
        if (keystrokeTable[i] == c) {
            return (unsigned char) (128 + i);
        }
    }
    if (c < 256) {
        return (unsigned char) c;
    }
    return UNKNOWN_KEY;
}

void numberToString(unsigned long number, short numberOfBytes, unsigned char *recordTo) {
    short i;
    unsigned long n;

    n = number;
    for (i=numberOfBytes - 1; i >= 0; i--) {
        recordTo[i] = n % 256;
        n /= 256;
    }
    if (n > 0) {
        printf("\nError: the number %li does not fit in %i bytes.", number, numberOfBytes);
        brogueAssert(false);
    }
}

// numberOfBytes can't be greater than 10
void recordNumber(unsigned long number, short numberOfBytes) {
    short i;
    unsigned char c[10];

    numberToString(number, numberOfBytes, c);
    for (i=0; i<numberOfBytes; i++) {
        recordChar(c[i]);
    }
}

// Events are recorded as follows:
// Keystrokes: Event type, keystroke value, modifier flags. (3 bytes.)
// All other events: Event type, x-coordinate of the event, y-coordinate of the event, modifier flags. (4 bytes.)
// Note: these must be sanitized, because user input may contain more than one byte per parameter.
void recordEvent(rogueEvent *event) {
    unsigned char c;

    if (rogue.playbackMode) {
        return;
    }

    recordChar((unsigned char) event->eventType);

    if (event->eventType == KEYSTROKE) {
        // record which key
        c = compressKeystroke(event->param1);
        if (c == UNKNOWN_KEY) {
            return;
        }
        recordChar(c);
    } else {
        recordChar((unsigned char) event->param1);
        recordChar((unsigned char) event->param2);
    }

    // record the modifier keys
    c = 0;
    if (event->controlKey) {
        c += Fl(1);
    }
    if (event->shiftKey) {
        c += Fl(2);
    }
    recordChar(c);
}

// For convenience.
void recordKeystroke(int keystroke, boolean controlKey, boolean shiftKey) {
    rogueEvent theEvent;

    if (rogue.playbackMode) {
        return;
    }

    theEvent.eventType = KEYSTROKE;
    theEvent.param1 = keystroke;
    theEvent.controlKey = controlKey;
    theEvent.shiftKey = shiftKey;
    recordEvent(&theEvent);
}

// record a series of keystrokes; string must end with a null terminator
void recordKeystrokeSequence(unsigned char *keystrokeSequence) {
    short i;
    for (i=0; keystrokeSequence[i] != '\0'; i++) {
        recordKeystroke(keystrokeSequence[i], false, false);
    }
}

// For convenience.
void recordMouseClick(short x, short y, boolean controlKey, boolean shiftKey) {
    rogueEvent theEvent;

    if (rogue.playbackMode) {
        return;
    }

    theEvent.eventType = MOUSE_UP;
    theEvent.param1 = x;
    theEvent.param2 = y;
    theEvent.controlKey = controlKey;
    theEvent.shiftKey = shiftKey;
    recordEvent(&theEvent);
}

void writeHeaderInfo(char *path) {
    unsigned char c[RECORDING_HEADER_LENGTH];
    short i;
    FILE *recordFile;

    // Zero out the entire header to start.
    for (i=0; i<RECORDING_HEADER_LENGTH; i++) {
        c[i] = 0;
    }

    // Note the version string to gracefully deny compatibility when necessary.
    for (i = 0; BROGUE_RECORDING_VERSION_STRING[i] != '\0'; i++) {
        c[i] = BROGUE_RECORDING_VERSION_STRING[i];
    }
    i = 16;
    numberToString(rogue.seed, 4, &c[i]);
    i += 4;
    numberToString(rogue.playerTurnNumber, 4, &c[i]);
    i += 4;
    numberToString(rogue.deepestLevel, 4, &c[i]);
    i += 4;
    numberToString(lengthOfPlaybackFile, 4, &c[i]);
    i += 4;

    if (!fileExists(path)) {
        recordFile = fopen(path, "wb");
        if (recordFile) {
            fclose(recordFile);
        }
    }

    recordFile = fopen(path, "r+b");
    rewind(recordFile);
    for (i=0; i<RECORDING_HEADER_LENGTH; i++) {
        putc(c[i], recordFile);
    }
    if (recordFile) {
        fclose(recordFile);
    }

    if (lengthOfPlaybackFile < RECORDING_HEADER_LENGTH) {
        lengthOfPlaybackFile = RECORDING_HEADER_LENGTH;
    }
}

void flushBufferToFile() {
    short i;
    FILE *recordFile;

    if (rogue.playbackMode) {
        return;
    }

    lengthOfPlaybackFile += locationInRecordingBuffer;
    writeHeaderInfo(currentFilePath);

    if (locationInRecordingBuffer != 0) {

        recordFile = fopen(currentFilePath, "ab");

        for (i=0; i<locationInRecordingBuffer; i++) {
            putc(inputRecordBuffer[i], recordFile);
        }

        if (recordFile) {
            fclose(recordFile);
        }

        locationInRecordingBuffer = 0;
    }
}

void fillBufferFromFile() {
//  short i;
    FILE *recordFile;

    recordFile = fopen(currentFilePath, "rb");
    fseek(recordFile, positionInPlaybackFile, SEEK_SET);

    fread((void *) inputRecordBuffer, 1, INPUT_RECORD_BUFFER, recordFile);

    positionInPlaybackFile = ftell(recordFile);
    fclose(recordFile);

    locationInRecordingBuffer = 0;
}

unsigned char recallChar() {
    unsigned char c;
    if (recordingLocation > lengthOfPlaybackFile) {
        return END_OF_RECORDING;
    }
    c = inputRecordBuffer[locationInRecordingBuffer++];
    recordingLocation++;
    if (locationInRecordingBuffer >= INPUT_RECORD_BUFFER) {
        fillBufferFromFile();
    }
    return c;
}

int uncompressKeystroke(int c) {
    if (c >= 128 && c <= UNKNOWN_KEY) {
        return keystrokeTable[c - 128];
    }
    return (int) c;
}

unsigned long recallNumber(short numberOfBytes) {
    short i;
    unsigned long n;

    n = 0;

    for (i=0; i<numberOfBytes; i++) {
        n *= 256;
        n += (unsigned long) recallChar();
    }
    return n;
}

#define OOS_APOLOGY "Playback of the recording has diverged from the originally recorded game.\n\n\
This could be caused by recording or playing the file on a modified version of Brogue, or it could \
simply be the result of a bug.  (The recording feature is still in beta for this reason.)\n\n\
If this is a different computer from the one on which the recording was saved, the recording \
might succeed on the original computer."

void playbackPanic() {
    cellDisplayBuffer rbuf[COLS][ROWS];

    if (!rogue.playbackOOS) {
        rogue.playbackFastForward = false;
        rogue.playbackPaused = true;
        rogue.playbackOOS = true;
        blackOutScreen();
        displayLevel();
        refreshSideBar(-1, -1, false);

        confirmMessages();
        message("Playback is out of sync.", false);

        printTextBox(OOS_APOLOGY, 0, 0, 0, &white, &black, rbuf, NULL, 0);

        rogue.playbackMode = false;
        displayMoreSign();
        rogue.playbackMode = true;

        overlayDisplayBuffer(rbuf, 0);

        printf("\n\nPlayback panic at location %li!", recordingLocation - 1);

        overlayDisplayBuffer(rbuf, 0);

        mainInputLoop();
    }
}

void recallEvent(rogueEvent *event) {
    unsigned char c;
    boolean tryAgain;

    do {
        tryAgain = false;
        c = recallChar();
        event->eventType = c;

        switch (c) {
            case KEYSTROKE:
                // record which key
                event->param1 = uncompressKeystroke(recallChar());
                event->param2 = 0;
                break;
            case SAVED_GAME_LOADED:
                tryAgain = true;
                flashTemporaryAlert(" Saved game loaded ", 1000);
                break;
            case MOUSE_UP:
            case MOUSE_DOWN:
            case MOUSE_ENTERED_CELL:
            case RIGHT_MOUSE_UP:
            case RIGHT_MOUSE_DOWN:
                event->param1 = recallChar();
                event->param2 = recallChar();
                break;
            case RNG_CHECK:
            case END_OF_RECORDING:
            case EVENT_ERROR:
            default:
                message("Unrecognized event type in playback.", true);
                printf("Unrecognized event type in playback: event ID %i", c);
                tryAgain = true;
                playbackPanic();
                break;
        }
    } while (tryAgain && !rogue.gameHasEnded);

    // record the modifier keys
    c = recallChar();
    event->controlKey = (c & Fl(1)) ? true : false;
    event->shiftKey =   (c & Fl(2)) ? true : false;
}

void loadNextAnnotation() {
    unsigned long currentReadTurn;
    short i;
    FILE *annotationFile;

    if (rogue.nextAnnotationTurn == -1) {
        return;
    }

    annotationFile =  fopen(annotationPathname, "r");
    fseek(annotationFile, rogue.locationInAnnotationFile, SEEK_SET);

    for (;;) {

        // load turn number
        if (fscanf(annotationFile, "%lu\t", &(currentReadTurn)) != 1) {
            if (feof(annotationFile)) {
                rogue.nextAnnotation[0] = '\0';
                rogue.nextAnnotationTurn = -1;
                break;
            } else {
                // advance to the end of the line
                fgets(rogue.nextAnnotation, 5000, annotationFile);
                continue;
            }
        }

        // load description
        fgets(rogue.nextAnnotation, 5000, annotationFile);

        if (currentReadTurn > rogue.playerTurnNumber ||
            (currentReadTurn <= 1 && rogue.playerTurnNumber <= 1 && currentReadTurn >= rogue.playerTurnNumber)) {
            rogue.nextAnnotationTurn = currentReadTurn;

            // strip the newline off the end
            rogue.nextAnnotation[strlen(rogue.nextAnnotation) - 1] = '\0';
            // strip out any gremlins in the annotation
            for (i=0; i<5000 && rogue.nextAnnotation[i]; i++) {
                if (rogue.nextAnnotation[i] < ' '
                    || rogue.nextAnnotation[i] > '~') {
                    rogue.nextAnnotation[i] = ' ';
                }
            }
            break;
        }
    }
    rogue.locationInAnnotationFile = ftell(annotationFile);
    fclose(annotationFile);
}

void displayAnnotation() {
    cellDisplayBuffer rbuf[COLS][ROWS];

    if (rogue.playbackMode
        && rogue.playerTurnNumber == rogue.nextAnnotationTurn) {

        if (!rogue.playbackFastForward) {
            refreshSideBar(-1, -1, false);

            printTextBox(rogue.nextAnnotation, player.xLoc, 0, 0, &black, &white, rbuf, NULL, 0);

            rogue.playbackMode = false;
            displayMoreSign();
            rogue.playbackMode = true;

            overlayDisplayBuffer(rbuf, 0);
        }

        loadNextAnnotation();
    }
}

// creates a game recording file, or if in playback mode,
// initializes based on and starts reading from the recording file
void initRecording() {
    short i;
    char versionString[16], buf[100];
    FILE *recordFile;

#ifdef AUDIT_RNG
    if (fileExists(RNG_LOG)) {
        remove(RNG_LOG);
    }
    RNGLogFile = fopen(RNG_LOG, "a");
#endif

    locationInRecordingBuffer   = 0;
    positionInPlaybackFile      = 0;
    recordingLocation           = 0;
    maxLevelChanges             = 0;
    rogue.playbackOOS           = false;
    rogue.playbackOmniscience   = false;
    rogue.nextAnnotationTurn    = 0;
    rogue.nextAnnotation[0]     = '\0';
    rogue.locationInAnnotationFile  = 0;

    if (rogue.playbackMode) {
        lengthOfPlaybackFile        = 100000; // so recall functions don't freak out
        rogue.playbackDelayPerTurn  = DEFAULT_PLAYBACK_DELAY;
        rogue.playbackDelayThisTurn = rogue.playbackDelayPerTurn;
        rogue.playbackPaused        = false;

        fillBufferFromFile();

        for (i=0; i<16; i++) {
            versionString[i] = recallChar();
        }

        if (strcmp(versionString, BROGUE_RECORDING_VERSION_STRING)) {
            rogue.playbackMode = false;
            rogue.playbackFastForward = false;
            sprintf(buf, "This file is from version %s and cannot be opened in version %s.", versionString, BROGUE_RECORDING_VERSION_STRING);
            dialogAlert(buf);
            rogue.playbackMode = true;
            rogue.playbackPaused = true;
            rogue.playbackFastForward = false;
            rogue.playbackOOS = false;
            rogue.gameHasEnded = true;
        }
        rogue.seed              = recallNumber(4);          // master random seed
        rogue.howManyTurns      = recallNumber(4);          // how many turns are in this recording
        maxLevelChanges         = recallNumber(4);          // how many times the player changes depths
        lengthOfPlaybackFile    = recallNumber(4);
        seedRandomGenerator(rogue.seed);
        previousGameSeed = rogue.seed;

        if (fileExists(annotationPathname)) {
            loadNextAnnotation();
        } else {
            rogue.nextAnnotationTurn = -1;
        }
    } else {
        lengthOfPlaybackFile = 1;
        remove(currentFilePath);
        recordFile = fopen(currentFilePath, "wb"); // create the file
        fclose(recordFile);

        flushBufferToFile(); // header info never makes it into inputRecordBuffer when recording
    }
    rogue.currentTurnNumber = 0;
}

void OOSCheck(unsigned long x, short numberOfBytes) {
    unsigned char eventType;
    unsigned long recordedNumber;

    if (rogue.playbackMode) {
        eventType = recallChar();
        recordedNumber = recallNumber(numberOfBytes);
        if (eventType != RNG_CHECK || recordedNumber != x) {
            if (eventType != RNG_CHECK) {
                printf("\nEvent type mismatch in RNG check.");
                playbackPanic();
            } else if (recordedNumber != x) {
                printf("\nExpected RNG output of %li; got %i.", recordedNumber, (int) x);
                playbackPanic();
            }
        }
    } else {
        recordChar(RNG_CHECK);
        recordNumber(x, numberOfBytes);
        considerFlushingBufferToFile();
    }
}

// compare a random number once per player turn so we instantly know if we are out of sync during playback
void RNGCheck() {
    short oldRNG;
    unsigned long randomNumber;

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_SUBSTANTIVE;

//#ifdef AUDIT_RNG
//reportRNGState();
//#endif

    randomNumber = (unsigned long) rand_range(0, 255);
    OOSCheck(randomNumber, 1);

    rogue.RNG = oldRNG;
}

boolean unpause() {
    if (rogue.playbackOOS) {
        flashTemporaryAlert(" Out of sync ", 2000);
    } else if (rogue.playbackPaused) {
        rogue.playbackPaused = false;
        return true;
    }
    return false;
}

#define PLAYBACK_HELP_LINE_COUNT    19

void printPlaybackHelpScreen() {
    short i, j;
    cellDisplayBuffer dbuf[COLS][ROWS], rbuf[COLS][ROWS];
    char helpText[PLAYBACK_HELP_LINE_COUNT][80] = {
        "Commands:",
        "",
        "         <space>: ****pause or unpause playback",
        "   k or up arrow: ****play back faster",
        " j or down arrow: ****play back slower",
        "               >: ****skip to next level",
        "             0-9: ****skip to specified turn number",
        "l or right arrow: ****advance one turn (shift for 5 turns; control for 20)",
        "",
        "           <tab>: ****enable or disable omniscience",
        "          return: ****examine surroundings",
        "               i: ****display inventory",
        "               D: ****display discovered items",
        "               V: ****view saved recording",
        "               O: ****open and resume saved game",
        "               N: ****begin a new game",
        "               Q: ****quit to title screen",
        "",
        "        -- press any key to continue --"
    };

    // Replace the "****"s with color escapes.
    for (i=0; i<PLAYBACK_HELP_LINE_COUNT; i++) {
        for (j=0; helpText[i][j]; j++) {
            if (helpText[i][j] == '*') {
                j = encodeMessageColor(helpText[i], j, &white);
            }
        }
    }

    clearDisplayBuffer(dbuf);

    for (i=0; i<PLAYBACK_HELP_LINE_COUNT; i++) {
        printString(helpText[i], mapToWindowX(5), mapToWindowY(i), &itemMessageColor, &black, dbuf);
    }

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            dbuf[i][j].opacity = (i < STAT_BAR_WIDTH ? 0 : INTERFACE_OPACITY);
        }
    }
    overlayDisplayBuffer(dbuf, rbuf);

    rogue.playbackMode = false;
    waitForAcknowledgment();
    rogue.playbackMode = true;
    overlayDisplayBuffer(rbuf, NULL);
}

void advanceToLocation(unsigned long destinationFrame) {
    unsigned long progressBarInterval, initialFrameNumber;
    rogueEvent theEvent;
    boolean useProgressBar, omniscient, stealth, trueColors;

    omniscient = rogue.playbackOmniscience;
    stealth = rogue.displayAggroRangeMode;
    trueColors = rogue.trueColorMode;
    rogue.playbackOmniscience = false;
    rogue.displayAggroRangeMode = false;
    rogue.trueColorMode = false;

    cellDisplayBuffer dbuf[COLS][ROWS];

    if (destinationFrame < rogue.playerTurnNumber) {
        useProgressBar = (destinationFrame > 100 ? true : false);

        // Start the recording over, and fast-forward to chosen frame.
        freeEverything();
        randomNumbersGenerated = 0;
        rogue.playbackMode = true;
        initializeRogue(0); // Seed argument is ignored because we're in playback.
        startLevel(rogue.depthLevel, 1);
        if (useProgressBar) {
            blackOutScreen();
        }
    } else {
        useProgressBar = (destinationFrame - rogue.playerTurnNumber > 100 ? true : false);
    }

    clearDisplayBuffer(dbuf);
    rectangularShading((COLS - 20) / 2, ROWS / 2, 20, 1, &black, INTERFACE_OPACITY, dbuf);
    overlayDisplayBuffer(dbuf, 0);
    commitDraws();
    displayMoreSign();

    rogue.playbackFastForward = true;
    progressBarInterval = max(1, (destinationFrame - rogue.playerTurnNumber) / 500);
    initialFrameNumber = rogue.playerTurnNumber;

    while (rogue.playerTurnNumber < destinationFrame && !rogue.gameHasEnded && !rogue.playbackOOS) {
        if (useProgressBar && !(rogue.playerTurnNumber % progressBarInterval)) {
            rogue.playbackFastForward = false;
            printProgressBar((COLS - 20) / 2, ROWS / 2, "[     Loading...   ]",
                             rogue.playerTurnNumber - initialFrameNumber,
                             destinationFrame - initialFrameNumber, &darkPurple, false);
            rogue.playbackFastForward = true;
            commitDraws();
        }

        rogue.RNG = RNG_COSMETIC; // dancing terrain colors can't influence recordings
        rogue.playbackDelayThisTurn = 0;
        nextBrogueEvent(&theEvent, false, true, false);
        rogue.RNG = RNG_SUBSTANTIVE;
        executeEvent(&theEvent);
    }

    rogue.playbackOmniscience = omniscient;
    rogue.displayAggroRangeMode = stealth;
    rogue.trueColorMode = trueColors;

    rogue.playbackPaused = true;
    rogue.playbackFastForward = false;
    confirmMessages();
    updateMessageDisplay();
    refreshSideBar(-1, -1, false);
    displayLevel();
}

void promptToAdvanceToLocation(short keystroke) {
    char entryText[30], buf[max(30, DCOLS)];
    unsigned long destinationFrame;
    boolean enteredText;

    if (!rogue.playbackPaused || unpause()) {
        buf[0] = (keystroke == '0' ? '\0' : keystroke);
        buf[1] = '\0';

        rogue.playbackMode = false;
        enteredText = getInputTextString(entryText, "Go to turn number: ", log10(ULONG_MAX) - 1, buf, "", TEXT_INPUT_NUMBERS, false);
        confirmMessages();
        rogue.playbackMode = true;

        if (enteredText && entryText[0] != '\0') {
            sscanf(entryText, "%lu", &destinationFrame);

            if (destinationFrame >= rogue.howManyTurns) {
                flashTemporaryAlert(" Past end of recording ", 3000);
            } else if (destinationFrame == rogue.playerTurnNumber) {
                sprintf(buf, " Already at turn %li ", destinationFrame);
                flashTemporaryAlert(buf, 1000);
            } else {
                advanceToLocation(destinationFrame);
            }
            rogue.playbackPaused = true;
        }
    }
}

void pausePlayback() {
    //short oldRNG;
    if (!rogue.playbackPaused) {
        rogue.playbackPaused = true;
        messageWithColor(KEYBOARD_LABELS ? "recording paused. Press space to play." : "recording paused.",
                         &teal, false);
        refreshSideBar(-1, -1, false);
        //oldRNG = rogue.RNG;
        //rogue.RNG = RNG_SUBSTANTIVE;
        mainInputLoop();
        //rogue.RNG = oldRNG;
        messageWithColor("recording unpaused.", &teal, false);
        rogue.playbackPaused = false;
        refreshSideBar(-1, -1, false);
        rogue.playbackDelayThisTurn = DEFAULT_PLAYBACK_DELAY;
    }
}

// Used to interact with playback -- e.g. changing speed, pausing.
boolean executePlaybackInput(rogueEvent *recordingInput) {
    signed long key;
    short newDelay, frameCount, x, y, previousDeepestLevel;
    unsigned long destinationFrame;
    boolean pauseState, proceed;
    rogueEvent theEvent;
    char path[BROGUE_FILENAME_MAX];

    if (!rogue.playbackMode) {
        return false;
    }

    if (recordingInput->eventType == KEYSTROKE) {
        key = recordingInput->param1;
        stripShiftFromMovementKeystroke(&key);

        switch (key) {
            case UP_ARROW:
            case UP_KEY:
                newDelay = max(1, min(rogue.playbackDelayPerTurn / 1.5, rogue.playbackDelayPerTurn - 1));
                if (newDelay != rogue.playbackDelayPerTurn) {
                    flashTemporaryAlert(" Faster ", 300);
                }
                rogue.playbackDelayPerTurn = newDelay;
                rogue.playbackDelayThisTurn = rogue.playbackDelayPerTurn;
                return true;
            case DOWN_ARROW:
            case DOWN_KEY:
                newDelay = min(3000, max(rogue.playbackDelayPerTurn * 1.5, rogue.playbackDelayPerTurn + 1));
                if (newDelay != rogue.playbackDelayPerTurn) {
                    flashTemporaryAlert(" Slower ", 300);
                }
                rogue.playbackDelayPerTurn = newDelay;
                rogue.playbackDelayThisTurn = rogue.playbackDelayPerTurn;
                return true;
            case ACKNOWLEDGE_KEY:
                if (rogue.playbackOOS && rogue.playbackPaused) {
                    flashTemporaryAlert(" Out of sync ", 2000);
                } else {
                    rogue.playbackPaused = !rogue.playbackPaused;
                }
                return true;
            case TAB_KEY:
                rogue.playbackOmniscience = !rogue.playbackOmniscience;
                displayLevel();
                refreshSideBar(-1, -1, false);
                if (rogue.playbackOmniscience) {
                    messageWithColor("Omniscience enabled.", &teal, false);
                } else {
                    messageWithColor("Omniscience disabled.", &teal, false);
                }
                return true;
            case DESCEND_KEY:
                pauseState = rogue.playbackPaused;
                previousDeepestLevel = rogue.deepestLevel;
                if (!rogue.playbackPaused || unpause()) {
                    if ((unsigned long) rogue.deepestLevel < maxLevelChanges) {
                        displayCenteredAlert(" Loading... ");
                        commitDraws();
                        rogue.playbackFastForward = true;
                        while ((rogue.deepestLevel <= previousDeepestLevel || !rogue.playbackBetweenTurns)
                               && !rogue.gameHasEnded) {
                            rogue.RNG = RNG_COSMETIC; // dancing terrain colors can't influence recordings
                            nextBrogueEvent(&theEvent, false, true, false);
                            rogue.RNG = RNG_SUBSTANTIVE;
                            executeEvent(&theEvent);
                        }
                        rogue.playbackFastForward = false;
                        rogue.playbackPaused = pauseState;
                        displayLevel();
                        refreshSideBar(-1, -1, false);
                        updateMessageDisplay();
                    } else {
                        flashTemporaryAlert(" Already reached deepest depth explored ", 1000);
                    }
                }
                rogue.playbackPaused = pauseState;
                return true;
            case INVENTORY_KEY:
                rogue.playbackMode = false;
                displayInventory(ALL_ITEMS, 0, 0, true, false);
                rogue.playbackMode = true;
                return true;
            case RIGHT_KEY:
            case RIGHT_ARROW:
            case LEFT_KEY:
            case LEFT_ARROW:
                if (key == RIGHT_KEY || key == RIGHT_ARROW) {
                    frameCount = 1;
                } else {
                    frameCount = -1;
                }
                if (recordingInput->shiftKey) {
                    frameCount *= 5;
                    rogue.playbackFastForward = true;
                }
                if (recordingInput->controlKey) {
                    frameCount *= 20;
                    rogue.playbackFastForward = true;
                }

                if (frameCount < 0) {
                    if ((unsigned long) (frameCount * -1) > rogue.playerTurnNumber) {
                        destinationFrame = 0;
                    } else {
                        destinationFrame = rogue.playerTurnNumber + frameCount;
                    }
                } else {
                    destinationFrame = min(rogue.playerTurnNumber + frameCount, rogue.howManyTurns - 1);
                }

                if (destinationFrame == rogue.playerTurnNumber) {
                    flashTemporaryAlert(" Already at end of recording ", 1000);
                } else if (frameCount < 0) {
                    rogue.playbackMode = false;
                    proceed = (rogue.playerTurnNumber < 100 || confirm("Rewind?", true));
                    rogue.playbackMode = true;
                    if (proceed) {
                        advanceToLocation(destinationFrame);
                    }
                } else {
                    // advance by the right number of turns
                    if (!rogue.playbackPaused || unpause()) {
                        while (rogue.playerTurnNumber < destinationFrame && !rogue.gameHasEnded && !rogue.playbackOOS) {
                            rogue.RNG = RNG_COSMETIC; // dancing terrain colors can't influence recordings
                            rogue.playbackDelayThisTurn = 0;
                            nextBrogueEvent(&theEvent, false, true, false);
                            rogue.RNG = RNG_SUBSTANTIVE;
                            executeEvent(&theEvent);
                        }
                        rogue.playbackPaused = true;
                        if (rogue.playbackFastForward) {
                            rogue.playbackFastForward = false;
                            displayLevel();
                            updateMessageDisplay();
                        }
                        refreshSideBar(-1, -1, false);
                    }
                }
                return true;
            case HELP_KEY:
                printPlaybackHelpScreen();
                return true;
            case DISCOVERIES_KEY:
                rogue.playbackMode = false;
                printDiscoveriesScreen();
                rogue.playbackMode = true;
                return true;
            case MESSAGE_ARCHIVE_KEY:
                rogue.playbackMode = false;
                displayMessageArchive();
                rogue.playbackMode = true;
                return true;
            case VIEW_RECORDING_KEY:
                confirmMessages();
                rogue.playbackMode = false;
                if (dialogChooseFile(path, RECORDING_SUFFIX, "View recording: ")) {
                    if (fileExists(path)) {
                        strcpy(rogue.nextGamePath, path);
                        rogue.nextGame = NG_VIEW_RECORDING;
                        rogue.gameHasEnded = true;
                    } else {
                        message("File not found.", false);
                    }
                }
                rogue.playbackMode = true;
                return true;
            case LOAD_SAVED_GAME_KEY:
                confirmMessages();
                rogue.playbackMode = false;
                if (dialogChooseFile(path, GAME_SUFFIX, "Open saved game: ")) {
                    if (fileExists(path)) {
                        strcpy(rogue.nextGamePath, path);
                        rogue.nextGame = NG_OPEN_GAME;
                        rogue.gameHasEnded = true;
                    } else {
                        message("File not found.", false);
                    }
                }
                rogue.playbackMode = true;
                return true;
            case NEW_GAME_KEY:
                rogue.playbackMode = false;
                if (confirm("Close recording and begin a new game?", true)) {
                    rogue.nextGame = NG_NEW_GAME;
                    rogue.gameHasEnded = true;
                }
                rogue.playbackMode = true;
                return true;
            case QUIT_KEY:
                //freeEverything();
                rogue.gameHasEnded = true;
                rogue.playbackOOS = false;
                rogue.creaturesWillFlashThisTurn = false;
                notifyEvent(GAMEOVER_RECORDING, 0, 0, "recording ended", "none");
                return true;
            case TRUE_COLORS_KEY:
                rogue.trueColorMode = !rogue.trueColorMode;
                displayLevel();
                refreshSideBar(-1, -1, false);
                if (rogue.trueColorMode) {
                    messageWithColor(KEYBOARD_LABELS ? "Color effects disabled. Press '\\' again to enable." : "Color effects disabled.",
                                     &teal, false);
                } else {
                    messageWithColor(KEYBOARD_LABELS ? "Color effects enabled. Press '\\' again to disable." : "Color effects enabled.",
                                     &teal, false);
                }
                return true;
            case AGGRO_DISPLAY_KEY:
                rogue.displayAggroRangeMode = !rogue.displayAggroRangeMode;
                displayLevel();
                refreshSideBar(-1, -1, false);
                if (rogue.displayAggroRangeMode) {
                    messageWithColor(KEYBOARD_LABELS ? "Stealth range displayed. Press ']' again to hide." : "Stealth range displayed.",
                                     &teal, false);
                } else {
                    messageWithColor(KEYBOARD_LABELS ? "Stealth range hidden. Press ']' again to display." : "Stealth range hidden.",
                                     &teal, false);
                }
                return true;
            case SEED_KEY:
                //rogue.playbackMode = false;
                //DEBUG {displayGrid(safetyMap); displayMoreSign(); displayLevel();}
                //rogue.playbackMode = true;
                printSeed();
                return true;
            default:
                if (key >= '0' && key <= '9'
                    || key >= NUMPAD_0 && key <= NUMPAD_9) {

                    promptToAdvanceToLocation(key);
                    return true;
                }
                return false;
        }
    } else if (recordingInput->eventType == MOUSE_UP) {
        x = recordingInput->param1;
        y = recordingInput->param2;
        if (windowToMapX(x) >= 0 && windowToMapX(x) < DCOLS && y >= 0 && y < MESSAGE_LINES) {
            // If the click location is in the message block, display the message archive.
            rogue.playbackMode = false;
            displayMessageArchive();
            rogue.playbackMode = true;
            return true;
        }
    } else if (recordingInput->eventType == RIGHT_MOUSE_UP) {
        rogue.playbackMode = false;
        displayInventory(ALL_ITEMS, 0, 0, true, false);
        rogue.playbackMode = true;
        return true;
    }
    return false;
}

// Pass in defaultPath (the file name WITHOUT suffix), and the suffix.
// Get back either defaultPath, or "defaultPath N",
// where N is the lowest counting number that doesn't collide with an existing file.
void getAvailableFilePath(char *returnPath, const char *defaultPath, const char *suffix) {
    char fullPath[BROGUE_FILENAME_MAX];
    short fileNameIterator = 2;

    strcpy(returnPath, defaultPath);
    sprintf(fullPath, "%s%s", returnPath, suffix);
    while (fileExists(fullPath)) {
        sprintf(returnPath, "%s %i", defaultPath, fileNameIterator);
        sprintf(fullPath, "%s%s", returnPath, suffix);
        fileNameIterator++;
    }
}

boolean characterForbiddenInFilename(const char theChar) {
    if (theChar == '/' || theChar == '\\' || theChar == ':') {
        return true;
    } else {
        return false;
    }
}

void saveGame() {
    char filePath[BROGUE_FILENAME_MAX], defaultPath[BROGUE_FILENAME_MAX];
    boolean askAgain;

    if (rogue.playbackMode) {
        return; // Call me paranoid, but I'd rather it be impossible to embed malware in a recording.
    }

    getAvailableFilePath(defaultPath, "Saved game", GAME_SUFFIX);

    deleteMessages();
    do {
        askAgain = false;
        if (getInputTextString(filePath, "Save game as (<esc> to cancel): ",
                               BROGUE_FILENAME_MAX - strlen(GAME_SUFFIX), defaultPath, GAME_SUFFIX, TEXT_INPUT_FILENAME, false)) {
            strcat(filePath, GAME_SUFFIX);
            if (!fileExists(filePath) || confirm("File of that name already exists. Overwrite?", true)) {
                remove(filePath);
                flushBufferToFile();
                rename(currentFilePath, filePath);
                strcpy(currentFilePath, filePath);
                message("Saved.", true);
                rogue.gameHasEnded = true;
            } else {
                askAgain = true;
            }
        }
    } while (askAgain);
    deleteMessages();
}

void saveRecordingNoPrompt(char *filePath)
{
    if (rogue.playbackMode) {
        return;
    }
    getAvailableFilePath(filePath, "Recording", RECORDING_SUFFIX);
    strcat(filePath, RECORDING_SUFFIX);
    remove(filePath);
    rename(currentFilePath, filePath);
}

void saveRecording(char *filePath) {
    char defaultPath[BROGUE_FILENAME_MAX];
    boolean askAgain;

    if (rogue.playbackMode) {
        return;
    }

    getAvailableFilePath(defaultPath, "Recording", RECORDING_SUFFIX);

    deleteMessages();
    do {
        askAgain = false;
        if (getInputTextString(filePath, "Save recording as (<esc> to cancel): ",
                               BROGUE_FILENAME_MAX - strlen(RECORDING_SUFFIX), defaultPath, RECORDING_SUFFIX, TEXT_INPUT_FILENAME, false)) {

            strcat(filePath, RECORDING_SUFFIX);
            if (!fileExists(filePath) || confirm("File of that name already exists. Overwrite?", true)) {
                remove(filePath);
                rename(currentFilePath, filePath);
            } else {
                askAgain = true;
            }
        } else { // Declined to save recording; save it anyway as LastRecording, and delete LastRecording if it already exists.
            strcpy(filePath, LAST_RECORDING_NAME);
            strcat(filePath, RECORDING_SUFFIX);
            if (fileExists(filePath)) {
                remove(filePath);
            }
            rename(currentFilePath, filePath);
        }
    } while (askAgain);
    deleteMessages();
}

void copyFile(char *fromFilePath, char *toFilePath, unsigned long fromFileLength) {
    unsigned long m, n;
    unsigned char fileBuffer[INPUT_RECORD_BUFFER];
    FILE *fromFile, *toFile;

    remove(toFilePath);

    fromFile    = fopen(fromFilePath, "rb");
    toFile      = fopen(toFilePath, "wb");

    for (n = 0; n < fromFileLength; n += m) {
        m = min(INPUT_RECORD_BUFFER, fromFileLength - n);
        fread((void *) fileBuffer, 1, m, fromFile);
        fwrite((void *) fileBuffer, 1, m, toFile);
    }

    fclose(fromFile);
    fclose(toFile);
}

// at the end of loading a saved game, this function transitions into active play mode.
void switchToPlaying() {
    char lastGamePath[BROGUE_FILENAME_MAX];

    getAvailableFilePath(lastGamePath, LAST_GAME_NAME, GAME_SUFFIX);
    strcat(lastGamePath, GAME_SUFFIX);

    rogue.playbackMode          = false;
    rogue.playbackFastForward   = false;
    rogue.playbackOmniscience   = false;
    locationInRecordingBuffer   = 0;
    copyFile(currentFilePath, lastGamePath, recordingLocation);

#ifdef DELETE_SAVE_FILE_AFTER_LOADING
    remove(currentFilePath);
#endif

    strcpy(currentFilePath, lastGamePath);

    blackOutScreen();
    refreshSideBar(-1, -1, false);
    updateMessageDisplay();
    displayLevel();
}

void loadSavedGame() {
    unsigned long progressBarInterval;
    rogueEvent theEvent;

    cellDisplayBuffer dbuf[COLS][ROWS];

    randomNumbersGenerated = 0;
    rogue.playbackMode = true;
    rogue.playbackFastForward = true;
    initializeRogue(0); // Calls initRecording(). Seed argument is ignored because we're initially in playback mode.
    if (!rogue.gameHasEnded) {
        blackOutScreen();
        startLevel(rogue.depthLevel, 1);
    }

    if (rogue.howManyTurns > 0) {

        progressBarInterval = max(1, lengthOfPlaybackFile / 100);

        clearDisplayBuffer(dbuf);
        rectangularShading((COLS - 20) / 2, ROWS / 2, 20, 1, &black, INTERFACE_OPACITY, dbuf);
        rogue.playbackFastForward = false;
        overlayDisplayBuffer(dbuf, 0);
        rogue.playbackFastForward = true;

        while (recordingLocation < lengthOfPlaybackFile
               && rogue.playerTurnNumber < rogue.howManyTurns
               && !rogue.gameHasEnded
               && !rogue.playbackOOS) {

            rogue.RNG = RNG_COSMETIC;
            nextBrogueEvent(&theEvent, false, true, false);
            rogue.RNG = RNG_SUBSTANTIVE;

            executeEvent(&theEvent);

            if (!(recordingLocation % progressBarInterval) && !rogue.playbackOOS) {
                rogue.playbackFastForward = false; // so the progress bar redraws make it to the screen
                printProgressBar((COLS - 20) / 2, ROWS / 2, "[     Loading...   ]", recordingLocation, lengthOfPlaybackFile, &darkPurple, false);
                commitDraws();
                rogue.playbackFastForward = true;
            }
        }
    }

    if (!rogue.gameHasEnded && !rogue.playbackOOS) {
        switchToPlaying();
        recordChar(SAVED_GAME_LOADED);
    }
}

// the following functions are used to create human-readable descriptions of playback files for debugging purposes

void describeKeystroke(unsigned char key, char *description) {
    short i;
    int c;
    const int keyList[51] = {UP_KEY, DOWN_KEY, LEFT_KEY, RIGHT_KEY, UP_ARROW, LEFT_ARROW,
        DOWN_ARROW, RIGHT_ARROW, UPLEFT_KEY, UPRIGHT_KEY, DOWNLEFT_KEY, DOWNRIGHT_KEY,
        DESCEND_KEY, ASCEND_KEY, REST_KEY, AUTO_REST_KEY, SEARCH_KEY, INVENTORY_KEY,
        ACKNOWLEDGE_KEY, EQUIP_KEY, UNEQUIP_KEY, APPLY_KEY, THROW_KEY, RELABEL_KEY, DROP_KEY, CALL_KEY,
        //FIGHT_KEY, FIGHT_TO_DEATH_KEY,
        HELP_KEY, DISCOVERIES_KEY, RETURN_KEY,
        EXPLORE_KEY, AUTOPLAY_KEY, SEED_KEY, EASY_MODE_KEY, ESCAPE_KEY,
        RETURN_KEY, DELETE_KEY, TAB_KEY, PERIOD_KEY, VIEW_RECORDING_KEY, NUMPAD_0,
        NUMPAD_1, NUMPAD_2, NUMPAD_3, NUMPAD_4, NUMPAD_5, NUMPAD_6, NUMPAD_7, NUMPAD_8,
        NUMPAD_9, UNKNOWN_KEY};
    const char descList[52][30] = {"up", "down", "left", "right", "up arrow", "left arrow",
        "down arrow", "right arrow", "upleft", "upright", "downleft", "downright",
        "descend", "ascend", "rest", "auto rest", "search", "inventory", "acknowledge",
        "equip", "unequip", "apply", "throw", "relabel", "drop", "call",
        //"fight", "fight to death",
        "help", "discoveries", "repeat travel", "explore", "autoplay", "seed",
        "easy mode", "escape", "return", "delete", "tab", "period", "open file",
        "numpad 0", "numpad 1", "numpad 2", "numpad 3", "numpad 4", "numpad 5", "numpad 6",
        "numpad 7", "numpad 8", "numpad 9", "unknown", "ERROR"};

    c = uncompressKeystroke(key);
    for (i=0; keyList[i] != c && i < 53; i++);
    if (key >= 32 && key <= 126) {
        sprintf(description, "Key: %c\t(%s)", key, descList[i]);
    } else {
        sprintf(description, "Key: %i\t(%s)", key, descList[i]);
    }
}

void appendModifierKeyDescription(char *description) {
    unsigned char c = recallChar();

    if (c & Fl(1)) {
        strcat(description, " + CRTL");
    }
    if (c & Fl(2)) {
        strcat(description, " + SHIFT");
    }
}

// Deprecated! Only used to parse recordings, a debugging feature.
boolean selectFile(char *prompt, char *defaultName, char *suffix) {
    boolean retval;
    char newFilePath[BROGUE_FILENAME_MAX];

    retval = false;

    if (chooseFile(newFilePath, prompt, defaultName, suffix)) {
        if (openFile(newFilePath)) {
            retval = true;
        } else {
            confirmMessages();
            message("File not found.", false);
            retval = false;
        }
    }
    return retval;
}

void parseFile() {
    FILE *descriptionFile;
    unsigned long oldFileLoc, oldRecLoc, oldLength, oldBufLoc, i, seed, numTurns, numDepths, fileLength, startLoc;
    unsigned char c;
    char description[1000], versionString[500];
    short x, y;

    if (selectFile("Parse recording: ", "Recording.broguerec", "")) {

        oldFileLoc = positionInPlaybackFile;
        oldRecLoc = recordingLocation;
        oldBufLoc = locationInRecordingBuffer;
        oldLength = lengthOfPlaybackFile;

        positionInPlaybackFile = 0;
        locationInRecordingBuffer = 0;
        recordingLocation = 0;
        lengthOfPlaybackFile = 10000000; // hack so that the recalls don't freak out
        fillBufferFromFile();

        descriptionFile = fopen("Recording Description.txt", "w");

        for (i=0; i<16; i++) {
            versionString[i] = recallChar();
        }

        seed        = recallNumber(4);
        numTurns    = recallNumber(4);
        numDepths   = recallNumber(4);
        fileLength  = recallNumber(4);

        fprintf(descriptionFile, "Parsed file \"%s\":\n\tVersion: %s\n\tSeed: %li\n\tNumber of turns: %li\n\tNumber of depth changes: %li\n\tFile length: %li\n",
                currentFilePath,
                versionString,
                seed,
                numTurns,
                numDepths,
                fileLength);
        for (i=0; recordingLocation < fileLength; i++) {
            startLoc = recordingLocation;
            c = recallChar();
            switch (c) {
                case KEYSTROKE:
                    describeKeystroke(recallChar(), description);
                    appendModifierKeyDescription(description);
                    break;
                case MOUSE_UP:
                case MOUSE_DOWN:
                case MOUSE_ENTERED_CELL:
                    x = (short) recallChar();
                    y = (short) recallChar();
                    sprintf(description, "Mouse click: (%i, %i)", x, y);
                    appendModifierKeyDescription(description);
                    break;
                case RNG_CHECK:
                    sprintf(description, "\tRNG check: %i", (short) recallChar());
                    break;
                case SAVED_GAME_LOADED:
                    strcpy(description, "Saved game loaded");
                    break;
                default:
                    sprintf(description, "UNKNOWN EVENT TYPE: %i", (short) c);
                    break;
            }
            fprintf(descriptionFile, "\nEvent %li, loc %li, length %li:%s\t%s", i, startLoc, recordingLocation - startLoc, (i < 10 ? " " : ""), description);
        }

        fclose(descriptionFile);

        positionInPlaybackFile = oldFileLoc;
        recordingLocation = oldRecLoc;
        lengthOfPlaybackFile = oldLength;
        locationInRecordingBuffer = oldBufLoc;
        message("File parsed.", false);
    } else {
        confirmMessages();
    }
}

void RNGLog(char *message) {
#ifdef AUDIT_RNG
    fputs(message, RNGLogFile);
#endif
}

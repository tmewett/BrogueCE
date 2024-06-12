/*
 *  IO.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/10/09.
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
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
 */

#include <math.h>
#include <time.h>

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

// Populates path[][] with a list of coordinates starting at origin and traversing down the map. Returns the number of steps in the path.
short getPlayerPathOnMap(pos path[1000], short **map, pos origin) {
    pos at = origin;

    int steps;
    for (steps = 0; true; steps++) {
        const int dir = nextStep(map, at.x, at.y, &player, false);
        if (dir == -1) {
            break;
        }
        at = posNeighborInDirection(at, dir);
        path[steps] = at;
        brogueAssert(coordinatesAreInMap(x, y));
    }
    return steps;
}

void reversePath(pos path[1000], short steps) {
    for (int i=0; i<steps / 2; i++) {
        pos temp = path[steps - i - 1];
        path[steps - i - 1] = path[i];
        path[i] = temp;
    }
}

void hilitePath(pos path[1000], short steps, boolean unhilite) {
    if (unhilite) {
        for (int i=0; i<steps; i++) {
            brogueAssert(isPosInMap(path[i]));
            pmapAt(path[i])->flags &= ~IS_IN_PATH;
            refreshDungeonCell(path[i]);
        }
    } else {
        for (int i=0; i<steps; i++) {
            brogueAssert(isPosInMap(path[i]));
            pmapAt(path[i])->flags |= IS_IN_PATH;
            refreshDungeonCell(path[i]);
        }
    }
}

// More expensive than hilitePath(__, __, true), but you don't need access to the path itself.
void clearCursorPath() {
    short i, j;

    if (!rogue.playbackMode) { // There are no cursor paths during playback.
        for (i=1; i<DCOLS; i++) {
            for (j=1; j<DROWS; j++) {
                if (pmap[i][j].flags & IS_IN_PATH) {
                    pmap[i][j].flags &= ~IS_IN_PATH;
                    refreshDungeonCell((pos){ i, j });
                }
            }
        }
    }
}

void hideCursor() {
    // Drop out of cursor mode if we're in it, and hide the path either way.
    rogue.cursorMode = false;
    rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);
    rogue.cursorLoc = INVALID_POS;
}

void showCursor() {
    // Return or enter turns on cursor mode. When the path is hidden, move the cursor to the player.
    if (!isPosInMap(rogue.cursorLoc)) {
        rogue.cursorLoc = player.loc;
        rogue.cursorMode = true;
        rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);
    } else {
        rogue.cursorMode = true;
        rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);
    }
}

static pos getClosestValidLocationOnMap(short **map, short x, short y) {
    pos answer = INVALID_POS;

    int closestDistance = 10000;
    int lowestMapScore = 10000;
    for (int i=1; i<DCOLS-1; i++) {
        for (int j=1; j<DROWS-1; j++) {
            if (map[i][j] >= 0 && map[i][j] < 30000) {

                const int dist = (i - x)*(i - x) + (j - y)*(j - y);
                //hiliteCell(i, j, &purple, min(dist / 2, 100), false);
                if (dist < closestDistance
                    || dist == closestDistance && map[i][j] < lowestMapScore) {

                    answer = (pos){ i, j };
                    closestDistance = dist;
                    lowestMapScore = map[i][j];
                }
            }
        }
    }

    return answer;
}

static void processSnapMap(short **map) {
    short **costMap;
    enum directions dir;
    short i, j, newX, newY;

    costMap = allocGrid();

    populateCreatureCostMap(costMap, &player);
    fillGrid(map, 30000);
    map[player.loc.x][player.loc.y] = 0;
    dijkstraScan(map, costMap, true);
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            if (cellHasTMFlag((pos){ i, j }, TM_INVERT_WHEN_HIGHLIGHTED)) {
                for (dir = 0; dir < 4; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && map[newX][newY] >= 0
                        && map[newX][newY] < map[i][j]) {

                        map[i][j] = map[newX][newY];
                    }
                }
            }
        }
    }

    freeGrid(costMap);
}

// Displays a menu of buttons for various commands.
// Buttons will be disabled if not permitted based on the playback state.
// Returns the keystroke to effect the button's command, or -1 if canceled.
// Some buttons take effect in this function instead of returning a value,
// i.e. true colors mode and display stealth mode.
static short actionMenu(short x, boolean playingBack) {
    short buttonCount;
    short y;
    boolean takeActionOurselves[ROWS] = {false};
    rogueEvent theEvent;

    brogueButton buttons[ROWS] = {{{0}}};
    char yellowColorEscape[5] = "", whiteColorEscape[5] = "", darkGrayColorEscape[5] = "";
    short i, j, longestName = 0, buttonChosen;

    encodeMessageColor(yellowColorEscape, 0, &itemMessageColor);
    encodeMessageColor(whiteColorEscape, 0, &white);
    encodeMessageColor(darkGrayColorEscape, 0, &black);

    do {
        for (i=0; i<ROWS; i++) {
            initializeButton(&(buttons[i]));
            buttons[i].buttonColor = interfaceBoxColor;
            buttons[i].opacity = INTERFACE_OPACITY;
        }

        buttonCount = 0;

        if (playingBack) {
#ifdef ENABLE_PLAYBACK_SWITCH
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "  %sP: %sPlay from here  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Play from here  ");
            }
            buttons[buttonCount].hotkey[0] = SWITCH_TO_PLAYING_KEY;
            buttonCount++;

            sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
            buttons[buttonCount].flags &= ~B_ENABLED;
            buttonCount++;
#endif
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "  %sk: %sFaster playback  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Faster playback  ");
            }
            buttons[buttonCount].hotkey[0] = UP_KEY;
            buttons[buttonCount].hotkey[1] = UP_ARROW;
            buttons[buttonCount].hotkey[2] = NUMPAD_8;
            buttonCount++;
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "  %sj: %sSlower playback  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Slower playback  ");
            }
            buttons[buttonCount].hotkey[0] = DOWN_KEY;
            buttons[buttonCount].hotkey[1] = DOWN_ARROW;
            buttons[buttonCount].hotkey[2] = NUMPAD_2;
            buttonCount++;
            sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
            buttons[buttonCount].flags &= ~B_ENABLED;
            buttonCount++;

            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "%s0-9: %sFast forward to turn  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Fast forward to turn  ");
            }
            buttons[buttonCount].hotkey[0] = '0';
            buttonCount++;
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "  %s<:%s Previous Level  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Previous Level  ");
            }
            buttons[buttonCount].hotkey[0] = ASCEND_KEY;
            buttonCount++;
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text,  "  %s>:%s Next Level  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Next Level  ");
            }
            buttons[buttonCount].hotkey[0] = DESCEND_KEY;
            buttonCount++;
            sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
            buttons[buttonCount].flags &= ~B_ENABLED;
            buttonCount++;
        } else {
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text, "  %sZ: %sRest until better  ",      yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Rest until better  ");
            }
            buttons[buttonCount].hotkey[0] = AUTO_REST_KEY;
            buttonCount++;

            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text, "  %sA: %sAutopilot  ",              yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Autopilot  ");
            }
            buttons[buttonCount].hotkey[0] = AUTOPLAY_KEY;
            buttonCount++;

            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text, "  %sT: %sRe-throw at last monster  ",              yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Re-throw at last monster  ");
            }
            buttons[buttonCount].hotkey[0] = RETHROW_KEY;
            buttonCount++;

            if (!rogue.easyMode) {
                if (KEYBOARD_LABELS) {
                    sprintf(buttons[buttonCount].text, "  %s&: %sEasy mode  ",              yellowColorEscape, whiteColorEscape);
                } else {
                    strcpy(buttons[buttonCount].text, "  Easy mode  ");
                }
                buttons[buttonCount].hotkey[0] = EASY_MODE_KEY;
                buttonCount++;
            }

            sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
            buttons[buttonCount].flags &= ~B_ENABLED;
            buttonCount++;
        }

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %s\\: %s[%s] Hide color effects  ",   yellowColorEscape, whiteColorEscape, rogue.trueColorMode ? "X" : " ");
        } else {
            sprintf(buttons[buttonCount].text, "  [%s] Hide color effects  ",   rogue.trueColorMode ? " " : "X");
        }
        buttons[buttonCount].hotkey[0] = TRUE_COLORS_KEY;
        takeActionOurselves[buttonCount] = true;
        buttonCount++;
        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %s]: %s[%s] Display stealth range  ", yellowColorEscape, whiteColorEscape, rogue.displayStealthRangeMode ? "X" : " ");
        } else {
            sprintf(buttons[buttonCount].text, "  [%s] Show stealth range  ",   rogue.displayStealthRangeMode ? "X" : " ");
        }
        buttons[buttonCount].hotkey[0] = STEALTH_RANGE_KEY;
        takeActionOurselves[buttonCount] = true;
        buttonCount++;

        if (hasGraphics) {
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text, "  %sG: %s[%c] Enable graphics  ", yellowColorEscape, whiteColorEscape, " X~"[graphicsMode]);
            } else {
                sprintf(buttons[buttonCount].text, "  [%c] Enable graphics  ",   " X~"[graphicsMode]);
            }
            buttons[buttonCount].hotkey[0] = GRAPHICS_KEY;
            takeActionOurselves[buttonCount] = true;
            buttonCount++;
        }

        sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
        buttons[buttonCount].flags &= ~B_ENABLED;
        buttonCount++;

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %sF: %sFeats             ",   yellowColorEscape, whiteColorEscape);
        } else {
            strcpy(buttons[buttonCount].text, "  Feats             ");
        }
        buttons[buttonCount].hotkey[0] = FEATS_KEY;
        buttonCount++;
        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %sD: %sDiscovered items  ",   yellowColorEscape, whiteColorEscape);
        } else {
            strcpy(buttons[buttonCount].text, "  Discovered items  ");
        }
        buttons[buttonCount].hotkey[0] = DISCOVERIES_KEY;
        DEBUG {
            buttonCount++;
            if (KEYBOARD_LABELS) {
                sprintf(buttons[buttonCount].text, "  %sC: %sCreate item or monster  ", yellowColorEscape, whiteColorEscape);
            } else {
                strcpy(buttons[buttonCount].text, "  Create item or monster  ");
            }
            buttons[buttonCount].hotkey[0] = CREATE_ITEM_MONSTER_KEY;
        }
        buttonCount++;
        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %s~: %sView dungeon seed  ",  yellowColorEscape, whiteColorEscape);
        } else {
            strcpy(buttons[buttonCount].text, "  View dungeon seed  ");
        }
        buttons[buttonCount].hotkey[0] = SEED_KEY;
        buttonCount++;
        if (KEYBOARD_LABELS) { // No help button if we're not in keyboard mode.
            sprintf(buttons[buttonCount].text, "  %s?: %sHelp  ", yellowColorEscape, whiteColorEscape);
            buttons[buttonCount].hotkey[0] = BROGUE_HELP_KEY;
            buttonCount++;
        }
        sprintf(buttons[buttonCount].text, "    %s---", darkGrayColorEscape);
        buttons[buttonCount].flags &= ~B_ENABLED;
        buttonCount++;

        if (!serverMode) {
            if (playingBack) {
                 if (KEYBOARD_LABELS) {
                    sprintf(buttons[buttonCount].text, "  %sO: %sOpen saved game  ",        yellowColorEscape, whiteColorEscape);
                } else {
                    strcpy(buttons[buttonCount].text, "  Open saved game  ");
                }
                buttons[buttonCount].hotkey[0] = LOAD_SAVED_GAME_KEY;
                buttonCount++;
                if (KEYBOARD_LABELS) {
                    sprintf(buttons[buttonCount].text, "  %sV: %sView saved recording  ",       yellowColorEscape, whiteColorEscape);
                } else {
                    strcpy(buttons[buttonCount].text, "  View saved recording  ");
                }
                buttons[buttonCount].hotkey[0] = VIEW_RECORDING_KEY;
                buttonCount++;
            } else {
                if (KEYBOARD_LABELS) {
                    sprintf(buttons[buttonCount].text, "  %sS: %sSave and exit  ",  yellowColorEscape, whiteColorEscape);
                } else {
                    strcpy(buttons[buttonCount].text, "  Save and exit  ");
                }
                buttons[buttonCount].hotkey[0] = SAVE_GAME_KEY;
                buttonCount++;
            }
        }
        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text, "  %sQ: %sQuit %s  ",    yellowColorEscape, whiteColorEscape, (playingBack ? "to title screen" : "and abandon game"));
        } else {
            sprintf(buttons[buttonCount].text, "  Quit %s  ",   (playingBack ? "to title screen" : "and abandon game"));
        }
        buttons[buttonCount].hotkey[0] = QUIT_KEY;
        buttonCount++;

        strcpy(buttons[buttonCount].text, " ");
        buttons[buttonCount].flags &= ~B_ENABLED;
        buttonCount++;

        for (i=0; i<buttonCount; i++) {
            longestName = max(longestName, strLenWithoutEscapes(buttons[i].text));
        }
        if (x + longestName >= COLS) {
            x = COLS - longestName - 1;
        }
        y = ROWS - buttonCount;
        for (i=0; i<buttonCount; i++) {
            buttons[i].x = x;
            buttons[i].y = y + i;
            for (j = strLenWithoutEscapes(buttons[i].text); j < longestName; j++) {
                strcat(buttons[i].text, " "); // Schlemiel the Painter, but who cares.
            }
        }

        const SavedDisplayBuffer rbuf = saveDisplayBuffer();
        screenDisplayBuffer dbuf;
        clearDisplayBuffer(&dbuf);
        rectangularShading(x - 1, y, longestName + 2, buttonCount, &black, INTERFACE_OPACITY / 2, &dbuf);
        overlayDisplayBuffer(&dbuf);
        buttonChosen = buttonInputLoop(buttons, buttonCount, x - 1, y, longestName + 2, buttonCount, NULL);
        restoreDisplayBuffer(&rbuf);
        if (buttonChosen == -1) {
            return -1;
        } else if (takeActionOurselves[buttonChosen]) {

            theEvent.eventType = KEYSTROKE;
            theEvent.param1 = buttons[buttonChosen].hotkey[0];
            theEvent.param2 = 0;
            theEvent.shiftKey = theEvent.controlKey = false;
            executeEvent(&theEvent);
        } else {
            return buttons[buttonChosen].hotkey[0];
        }
    } while (takeActionOurselves[buttonChosen]);
    brogueAssert(false);
    return -1;
}

#define MAX_MENU_BUTTON_COUNT 5

static void initializeMenuButtons(buttonState *state, brogueButton buttons[5]) {
    short i, x, buttonCount;
    char goldTextEscape[MAX_MENU_BUTTON_COUNT] = "";
    char whiteTextEscape[MAX_MENU_BUTTON_COUNT] = "";

    encodeMessageColor(goldTextEscape, 0, KEYBOARD_LABELS ? &yellow : &white);
    encodeMessageColor(whiteTextEscape, 0, &white);

    for (i=0; i<MAX_MENU_BUTTON_COUNT; i++) {
        initializeButton(&(buttons[i]));
        buttons[i].opacity = 75;
        buttons[i].buttonColor = interfaceButtonColor;
        buttons[i].y = ROWS - 1;
        buttons[i].flags |= B_WIDE_CLICK_AREA;
        buttons[i].flags &= ~B_KEYPRESS_HIGHLIGHT;
    }

    buttonCount = 0;

    if (rogue.playbackMode) {
        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text,  " Unpause (%sspace%s) ", goldTextEscape, whiteTextEscape);
        } else {
            strcpy(buttons[buttonCount].text,   "     Unpause     ");
        }
        buttons[buttonCount].hotkey[0] = ACKNOWLEDGE_KEY;
        buttonCount++;

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text,  "Omniscience (%stab%s)", goldTextEscape, whiteTextEscape);
        } else {
            strcpy(buttons[buttonCount].text,   "   Omniscience   ");
        }
        buttons[buttonCount].hotkey[0] = TAB_KEY;
        buttonCount++;

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text,  " Next Turn (%sl%s) ", goldTextEscape, whiteTextEscape);
        } else {
            strcpy(buttons[buttonCount].text,   "   Next Turn   ");
        }
        buttons[buttonCount].hotkey[0] = RIGHT_KEY;
        buttons[buttonCount].hotkey[1] = RIGHT_ARROW;
        buttonCount++;

        strcpy(buttons[buttonCount].text,       "  Menu  ");
        buttonCount++;
    } else {
        sprintf(buttons[buttonCount].text,  "   E%sx%splore   ", goldTextEscape, whiteTextEscape);
        buttons[buttonCount].hotkey[0] = EXPLORE_KEY;
        buttons[buttonCount].hotkey[1] = 'X';
        buttonCount++;

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text,  "   Rest (%sz%s)   ", goldTextEscape, whiteTextEscape);
        } else {
            strcpy(buttons[buttonCount].text,   "     Rest     ");
        }
        buttons[buttonCount].hotkey[0] = REST_KEY;
        buttonCount++;

        if (KEYBOARD_LABELS) {
            sprintf(buttons[buttonCount].text,  "  Search (%ss%s)  ", goldTextEscape, whiteTextEscape);
        } else {
            strcpy(buttons[buttonCount].text,   "    Search    ");
        }
        buttons[buttonCount].hotkey[0] = SEARCH_KEY;
        buttonCount++;

        strcpy(buttons[buttonCount].text,       "    Menu    ");
        buttonCount++;
    }

    sprintf(buttons[4].text,    "   %sI%snventory   ", goldTextEscape, whiteTextEscape);
    buttons[4].hotkey[0] = INVENTORY_KEY;
    buttons[4].hotkey[1] = 'I';

    x = mapToWindowX(0);
    for (i=0; i<5; i++) {
        buttons[i].x = x;
        x += strLenWithoutEscapes(buttons[i].text) + 2; // Gap between buttons.
    }

    initializeButtonState(state,
                          buttons,
                          5,
                          mapToWindowX(0),
                          ROWS - 1,
                          COLS - mapToWindowX(0),
                          1);
}


// This is basically the main loop for the game.
void mainInputLoop() {
    pos oldTargetLoc = { 0, 0 };
    short steps, oldRNG, dir;
    pos path[1000];
    creature *monst;
    item *theItem;
    SavedDisplayBuffer rbuf;

    boolean canceled, targetConfirmed, tabKey, focusedOnMonster, focusedOnItem, focusedOnTerrain,
    playingBack, doEvent, textDisplayed;

    rogueEvent theEvent;
    short **costMap, **playerPathingMap, **cursorSnapMap;
    brogueButton buttons[5] = {{{0}}};
    buttonState state;
    short buttonInput;
    short backupCost;

    canceled = false;
    rogue.cursorMode = false; // Controls whether the keyboard moves the cursor or the character.
    steps = 0;

    rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);

    // Initialize buttons.
    initializeMenuButtons(&state, buttons);

    playingBack = rogue.playbackMode;
    rogue.playbackMode = false;
    costMap = allocGrid();
    playerPathingMap = allocGrid();
    cursorSnapMap = allocGrid();

    rogue.cursorLoc = INVALID_POS;

    while (!rogue.gameHasEnded && (!playingBack || !canceled)) { // repeats until the game ends

        oldRNG = rogue.RNG;
        rogue.RNG = RNG_COSMETIC;

        focusedOnMonster = focusedOnItem = focusedOnTerrain = false;
        steps = 0;
        clearCursorPath();

        const pos originLoc = player.loc;

        if (playingBack && rogue.cursorMode) {
            temporaryMessage("Examine what? (<hjklyubn>, mouse, or <tab>)", 0);
        }

        if (!playingBack
            && posEq(player.loc, rogue.cursorLoc)
            && posEq(oldTargetLoc, rogue.cursorLoc)) {

            // Path hides when you reach your destination.
            rogue.cursorMode = false;
            rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);
            rogue.cursorLoc = INVALID_POS;
        }

        oldTargetLoc = rogue.cursorLoc;

        populateCreatureCostMap(costMap, &player);

        fillGrid(playerPathingMap, 30000);
        playerPathingMap[player.loc.x][player.loc.y] = 0;
        dijkstraScan(playerPathingMap, costMap, true);
        processSnapMap(cursorSnapMap);

        do {
            textDisplayed = false;

            // Draw the cursor and path
            if (isPosInMap(oldTargetLoc)) {
                refreshDungeonCell(oldTargetLoc);               // Remove old cursor.
            }
            if (!playingBack) {
                if (isPosInMap(oldTargetLoc)) {
                    hilitePath(path, steps, true);                                  // Unhilite old path.
                }
                if (isPosInMap(rogue.cursorLoc)) {
                    pos pathDestination;
                    if (cursorSnapMap[rogue.cursorLoc.x][rogue.cursorLoc.y] >= 0
                        && cursorSnapMap[rogue.cursorLoc.x][rogue.cursorLoc.y] < 30000) {
                        pathDestination = rogue.cursorLoc;
                    } else {
                        // If the cursor is aimed at an inaccessible area, find the nearest accessible area to path toward.
                        pathDestination = getClosestValidLocationOnMap(cursorSnapMap, rogue.cursorLoc.x, rogue.cursorLoc.y);
                    }

                    fillGrid(playerPathingMap, 30000);
                    playerPathingMap[pathDestination.x][pathDestination.y] = 0;
                    backupCost = costMap[pathDestination.x][pathDestination.y];
                    costMap[pathDestination.x][pathDestination.y] = 1;
                    dijkstraScan(playerPathingMap, costMap, true);
                    costMap[pathDestination.x][pathDestination.y] = backupCost;
                    steps = getPlayerPathOnMap(path, playerPathingMap, player.loc);

//                  steps = getPlayerPathOnMap(path, playerPathingMap, pathDestination[0], pathDestination[1]) - 1; // Get new path.
//                  reversePath(path, steps);   // Flip it around, back-to-front.

                    if (steps >= 0) {
                        path[steps] = pathDestination;
                    }
                    steps++;
//                  if (playerPathingMap[cursor[0]][cursor[1]] != 1
                    if (playerPathingMap[player.loc.x][player.loc.y] != 1
                        || !posEq(pathDestination, rogue.cursorLoc)) {

                        hilitePath(path, steps, false);     // Hilite new path.
                    }
                }
            }

            if (isPosInMap(rogue.cursorLoc)) {
                hiliteCell(rogue.cursorLoc.x,
                           rogue.cursorLoc.y,
                           &white,
                           (steps <= 0
                            || posEq(path[steps-1], rogue.cursorLoc)
                            || (!playingBack && distanceBetween(player.loc, rogue.cursorLoc) <= 1) ? 100 : 25),
                           true);

                oldTargetLoc = rogue.cursorLoc;

                monst = monsterAtLoc(rogue.cursorLoc);
                theItem = itemAtLoc(rogue.cursorLoc);
                if (monst != NULL && (canSeeMonster(monst) || rogue.playbackOmniscience)) {
                    rogue.playbackMode = playingBack;
                    refreshSideBar(rogue.cursorLoc.x, rogue.cursorLoc.y, false);
                    rogue.playbackMode = false;

                    focusedOnMonster = true;
                    if (monst != &player && (!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience || player.status[STATUS_TELEPATHIC])) {
                        rbuf = saveDisplayBuffer();
                        printMonsterDetails(monst);
                        textDisplayed = true;
                    }
                } else if (theItem != NULL && playerCanSeeOrSense(rogue.cursorLoc.x, rogue.cursorLoc.y)) {
                    rogue.playbackMode = playingBack;
                    refreshSideBar(rogue.cursorLoc.x, rogue.cursorLoc.y, false);
                    rogue.playbackMode = false;

                    focusedOnItem = true;
                    if (!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience) {
                        rbuf = saveDisplayBuffer();
                        printFloorItemDetails(theItem);
                        textDisplayed = true;
                    }
                } else if (cellHasTMFlag(rogue.cursorLoc, TM_LIST_IN_SIDEBAR) && playerCanSeeOrSense(rogue.cursorLoc.x, rogue.cursorLoc.y)) {
                    rogue.playbackMode = playingBack;
                    refreshSideBar(rogue.cursorLoc.x, rogue.cursorLoc.y, false);
                    rogue.playbackMode = false;
                    focusedOnTerrain = true;
                }

                printLocationDescription(rogue.cursorLoc.x, rogue.cursorLoc.y);
            }

            // Get the input!
            rogue.playbackMode = playingBack;
            doEvent = moveCursor(&targetConfirmed, &canceled, &tabKey, &rogue.cursorLoc, &theEvent, &state, !textDisplayed, rogue.cursorMode, true);
            rogue.playbackMode = false;

            if (state.buttonChosen == 3) { // Actions menu button.
                buttonInput = actionMenu(buttons[3].x - 4, playingBack); // Returns the corresponding keystroke.
                if (buttonInput == -1) { // Canceled.
                    doEvent = false;
                } else {
                    theEvent.eventType = KEYSTROKE;
                    theEvent.param1 = buttonInput;
                    theEvent.param2 = 0;
                    theEvent.shiftKey = theEvent.controlKey = false;
                    doEvent = true;
                }
            } else if (state.buttonChosen > -1) {
                theEvent.eventType = KEYSTROKE;
                theEvent.param1 = buttons[state.buttonChosen].hotkey[0];
                theEvent.param2 = 0;
            }
            state.buttonChosen = -1;

            if (playingBack) {
                if (canceled) {
                    rogue.cursorMode = false;
                    rogue.cursorPathIntensity = (rogue.cursorMode ? 50 : 20);
                }

                if (theEvent.eventType == KEYSTROKE
                    && theEvent.param1 == ACKNOWLEDGE_KEY) { // To unpause by button during playback.
                    canceled = true;
                } else {
                    canceled = false;
                }
            }

            if (focusedOnMonster || focusedOnItem || focusedOnTerrain) {
                focusedOnMonster = false;
                focusedOnItem = false;
                focusedOnTerrain = false;
                if (textDisplayed) {
                    restoreDisplayBuffer(&rbuf); // Erase the monster info window.
                }
                rogue.playbackMode = playingBack;
                refreshSideBar(-1, -1, false);
                rogue.playbackMode = false;
            }

            if (tabKey && !playingBack) { // The tab key cycles the cursor through monsters, items and terrain features.
                pos newLoc;
                if (nextTargetAfter(NULL, &newLoc, rogue.cursorLoc, AUTOTARGET_MODE_EXPLORE, theEvent.shiftKey)) {
                    rogue.cursorLoc = newLoc;
                }
            }

            if (theEvent.eventType == KEYSTROKE
                && (theEvent.param1 == ASCEND_KEY && rogue.cursorLoc.x == rogue.upLoc.x && rogue.cursorLoc.y == rogue.upLoc.y
                    || theEvent.param1 == DESCEND_KEY && rogue.cursorLoc.x == rogue.downLoc.x && rogue.cursorLoc.y == rogue.downLoc.y)) {

                    targetConfirmed = true;
                    doEvent = false;
                }
        } while (!targetConfirmed && !canceled && !doEvent && !rogue.gameHasEnded);

        if (isPosInMap(oldTargetLoc)) {
            refreshDungeonCell(oldTargetLoc);                       // Remove old rogue.cursorLoc.
        }

        restoreRNG;

        if (canceled && !playingBack) {
            hideCursor();
            confirmMessages();
        } else if (targetConfirmed && !playingBack && isPosInMap(rogue.cursorLoc)) {
            if (theEvent.eventType == MOUSE_UP
                && theEvent.controlKey
                && steps > 1) {
                // Control-clicking moves the player one step along the path.
                for (dir=0;
                     dir < DIRECTION_COUNT && !posEq(posNeighborInDirection(player.loc, dir) , path[0]);
                     dir++);
                playerMoves(dir);
            } else if (D_WORMHOLING) {
                travel(rogue.cursorLoc.x, rogue.cursorLoc.y, true);
            } else {
                confirmMessages();
                if (posEq(originLoc, rogue.cursorLoc)) {
                    confirmMessages();
                } else if (abs(player.loc.x - rogue.cursorLoc.x) + abs(player.loc.y - rogue.cursorLoc.y) == 1 // horizontal or vertical
                           || (distanceBetween(player.loc, rogue.cursorLoc) == 1 // includes diagonals
                               && (!diagonalBlocked(player.loc.x, player.loc.y, rogue.cursorLoc.x, rogue.cursorLoc.y, !rogue.playbackOmniscience)
                                   || ((pmapAt(rogue.cursorLoc)->flags & HAS_MONSTER) && (monsterAtLoc(rogue.cursorLoc)->info.flags & MONST_ATTACKABLE_THRU_WALLS)) // there's a turret there
                                   || ((terrainFlags(rogue.cursorLoc) & T_OBSTRUCTS_PASSABILITY) && (terrainMechFlags(rogue.cursorLoc) & TM_PROMOTES_ON_PLAYER_ENTRY))))) { // there's a lever there
                                                                                                                                                                                      // Clicking one space away will cause the player to try to move there directly irrespective of path.
                                   for (dir=0;
                                        dir < DIRECTION_COUNT && (player.loc.x + nbDirs[dir][0] != rogue.cursorLoc.x || player.loc.y + nbDirs[dir][1] != rogue.cursorLoc.y);
                                        dir++);
                                   playerMoves(dir);
                               } else if (steps) {
                                   travelRoute(path, steps);
                               }
            }
        } else if (doEvent) {
            // If the player entered input during moveCursor() that wasn't a cursor movement command.
            // Mainly, we want to filter out directional keystrokes when we're in cursor mode, since
            // those should move the cursor but not the player.
            brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);
            if (playingBack) {
                rogue.playbackMode = true;
                executePlaybackInput(&theEvent);
#ifdef ENABLE_PLAYBACK_SWITCH
                if (!rogue.playbackMode) {
                    // Playback mode is off, user must have taken control
                    // Redraw buttons to reflect that
                    initializeMenuButtons(&state, buttons);
                }
#endif
                playingBack = rogue.playbackMode;
                rogue.playbackMode = false;
            } else {
                executeEvent(&theEvent);
                if (rogue.playbackMode) {
                    playingBack = true;
                    rogue.playbackMode = false;
                    confirmMessages();
                    break;
                }
            }
        }
    }

    rogue.playbackMode = playingBack;
    refreshSideBar(-1, -1, false);
    freeGrid(costMap);
    freeGrid(playerPathingMap);
    freeGrid(cursorSnapMap);
}

// accuracy depends on how many clock cycles occur per second
#define MILLISECONDS    (clock() * 1000 / CLOCKS_PER_SEC)

#define MILLISECONDS_FOR_CAUTION    100

void considerCautiousMode() {
    /*
    signed long oldMilliseconds = rogue.milliseconds;
    rogue.milliseconds = MILLISECONDS;
    clock_t i = clock();
    printf("\n%li", i);
    if (rogue.milliseconds - oldMilliseconds < MILLISECONDS_FOR_CAUTION) {
        rogue.cautiousMode = true;
    }*/
}

// previouslyPlottedCells is only accessed by commitDraws and refreshScreen,
// as below.
static screenDisplayBuffer previouslyPlottedCells;

// Only cells which have changed since the previous commitDraws are actually
// drawn.
void commitDraws() {
    for (int j = 0; j < ROWS; j++) {
        for (int i = 0; i < COLS; i++) {
            cellDisplayBuffer *lastPlotted = &previouslyPlottedCells.cells[i][j];
            cellDisplayBuffer *curr = &displayBuffer.cells[i][j];
            boolean needsUpdate =
                lastPlotted->character != curr->character
                || lastPlotted->foreColorComponents[0] != curr->foreColorComponents[0]
                || lastPlotted->foreColorComponents[1] != curr->foreColorComponents[1]
                || lastPlotted->foreColorComponents[2] != curr->foreColorComponents[2]
                || lastPlotted->backColorComponents[0] != curr->backColorComponents[0]
                || lastPlotted->backColorComponents[1] != curr->backColorComponents[1]
                || lastPlotted->backColorComponents[2] != curr->backColorComponents[2];

            if (!needsUpdate) {
                continue;
            }

            plotChar(curr->character, i, j,
                     curr->foreColorComponents[0],
                     curr->foreColorComponents[1],
                     curr->foreColorComponents[2],
                     curr->backColorComponents[0],
                     curr->backColorComponents[1],
                     curr->backColorComponents[2]
            );
            *lastPlotted = *curr;
        }
    }
}

// flags the entire window as needing to be redrawn at next flush.
// very low level -- does not interface with the guts of the game.
void refreshScreen() {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            cellDisplayBuffer *curr = &displayBuffer.cells[i][j];
            plotChar(curr->character, i, j,
                     curr->foreColorComponents[0],
                     curr->foreColorComponents[1],
                     curr->foreColorComponents[2],
                     curr->backColorComponents[0],
                     curr->backColorComponents[1],
                     curr->backColorComponents[2]
            );
            // Remember that it was previously plotted, so that
            // commitDraws still knows when it needs updates.
            previouslyPlottedCells.cells[i][j] = *curr;
        }
    }
}

// higher-level redraw
void displayLevel() {
    short i, j;

    for( i=0; i<DCOLS; i++ ) {
        for (j = DROWS-1; j >= 0; j--) {
            refreshDungeonCell((pos){ i, j });
        }
    }
}

// converts colors into components
void storeColorComponents(char components[3], const color *theColor) {
    short rand = rand_range(0, theColor->rand);
    components[0] = max(0, min(100, theColor->red + rand_range(0, theColor->redRand) + rand));
    components[1] = max(0, min(100, theColor->green + rand_range(0, theColor->greenRand) + rand));
    components[2] = max(0, min(100, theColor->blue + rand_range(0, theColor->blueRand) + rand));
}

static void bakeTerrainColors(color *foreColor, color *backColor, short x, short y) {
    const short *vals;
    const short neutralColors[8] = {1000, 1000, 1000, 1000, 0, 0, 0, 0};
    if (rogue.trueColorMode) {
        vals = neutralColors;
    } else {
        vals = &(terrainRandomValues[x][y][0]);
    }

    const short foreRand = foreColor->rand * vals[6] / 1000;
    const short backRand = backColor->rand * vals[7] / 1000;

    foreColor->red += foreColor->redRand * vals[0] / 1000 + foreRand;
    foreColor->green += foreColor->greenRand * vals[1] / 1000 + foreRand;
    foreColor->blue += foreColor->blueRand * vals[2] / 1000 + foreRand;
    foreColor->redRand = foreColor->greenRand = foreColor->blueRand = foreColor->rand = 0;

    backColor->red += backColor->redRand * vals[3] / 1000 + backRand;
    backColor->green += backColor->greenRand * vals[4] / 1000 + backRand;
    backColor->blue += backColor->blueRand * vals[5] / 1000 + backRand;
    backColor->redRand = backColor->greenRand = backColor->blueRand = backColor->rand = 0;

    if (foreColor->colorDances || backColor->colorDances) {
        pmap[x][y].flags |= TERRAIN_COLORS_DANCING;
    } else {
        pmap[x][y].flags &= ~TERRAIN_COLORS_DANCING;
    }
}

void bakeColor(color *theColor) {
    short rand;
    rand = rand_range(0, theColor->rand);
    theColor->red += rand_range(0, theColor->redRand) + rand;
    theColor->green += rand_range(0, theColor->greenRand) + rand;
    theColor->blue += rand_range(0, theColor->blueRand) + rand;
    theColor->redRand = theColor->greenRand = theColor->blueRand = theColor->rand = 0;
}

void shuffleTerrainColors(short percentOfCells, boolean refreshCells) {
    enum directions dir;
    short i, j;

    assureCosmeticRNG;

    for (i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (playerCanSeeOrSense(i, j)
                && (!rogue.automationActive || !(rogue.playerTurnNumber % 5))
                && ((pmap[i][j].flags & TERRAIN_COLORS_DANCING)
                    || (player.status[STATUS_HALLUCINATING] && playerCanDirectlySee(i, j)))
                && (i != rogue.cursorLoc.x || j != rogue.cursorLoc.y)
                && (percentOfCells >= 100 || rand_range(1, 100) <= percentOfCells)) {

                    for (dir=0; dir<DIRECTION_COUNT; dir++) {
                        terrainRandomValues[i][j][dir] += rand_range(-600, 600);
                        terrainRandomValues[i][j][dir] = clamp(terrainRandomValues[i][j][dir], 0, 1000);
                    }

                    if (refreshCells) {
                        refreshDungeonCell((pos){ i, j });
                    }
                }
        }
    }
    restoreRNG;
}

// if forecolor is too similar to back, darken or lighten it and return true.
// Assumes colors have already been baked (no random components).
boolean separateColors(color *fore, const color *back) {
    color f, b;
    const color *modifier;
    short failsafe;
    boolean madeChange;

    f = *fore;
    b = *back;
    f.red       = clamp(f.red, 0, 100);
    f.green     = clamp(f.green, 0, 100);
    f.blue      = clamp(f.blue, 0, 100);
    b.red       = clamp(b.red, 0, 100);
    b.green     = clamp(b.green, 0, 100);
    b.blue      = clamp(b.blue, 0, 100);

    if (f.red + f.blue + f.green > 50 * 3) {
        modifier = &black;
    } else {
        modifier = &white;
    }

    madeChange = false;
    failsafe = 10;

    while(COLOR_DIFF(f, b) < MIN_COLOR_DIFF && --failsafe) {
        applyColorAverage(&f, modifier, 20);
        madeChange = true;
    }

    if (madeChange) {
        *fore = f;
        return true;
    } else {
        return false;
    }
}

void normColor(color *baseColor, const short aggregateMultiplier, const short colorTranslation) {

    baseColor->red += colorTranslation;
    baseColor->green += colorTranslation;
    baseColor->blue += colorTranslation;
    const short vectorLength =  baseColor->red + baseColor->green + baseColor->blue;

    if (vectorLength != 0) {
        baseColor->red =    baseColor->red * 300    / vectorLength * aggregateMultiplier / 100;
        baseColor->green =  baseColor->green * 300  / vectorLength * aggregateMultiplier / 100;
        baseColor->blue =   baseColor->blue * 300   / vectorLength * aggregateMultiplier / 100;
    }
    baseColor->redRand = 0;
    baseColor->greenRand = 0;
    baseColor->blueRand = 0;
    baseColor->rand = 0;
}

// Used to determine whether to draw a wall top glyph above
static boolean glyphIsWallish(enum displayGlyph glyph) {
    switch (glyph) {
        case G_WALL:
        case G_OPEN_DOOR:
        case G_CLOSED_DOOR:
        case G_UP_STAIRS:
        case G_DOORWAY:
        case G_WALL_TOP:
        case G_LEVER:
        case G_LEVER_PULLED:
        case G_CLOSED_IRON_DOOR:
        case G_OPEN_IRON_DOOR:
        case G_TURRET:
        case G_GRANITE:
        case G_TORCH:
        case G_PORTCULLIS:
            return true;

        default:
            return false;
    }
}

static enum monsterTypes randomAnimateMonster() {
    /* Randomly pick an animate and vulnerable monster type. Used by
    getCellAppearance for hallucination effects. */
    static int listLength = 0;
    static enum monsterTypes animate[NUMBER_MONSTER_KINDS];

    if (listLength == 0) {
        for (int i=0; i < NUMBER_MONSTER_KINDS; i++) {
            if (!(monsterCatalog[i].flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
                animate[listLength++] = i;
            }
        }
    }

    return animate[rand_range(0, listLength - 1)];
}

// okay, this is kind of a beast...
void getCellAppearance(pos loc, enum displayGlyph *returnChar, color *returnForeColor, color *returnBackColor) {
    short bestBCPriority, bestFCPriority, bestCharPriority;
    short distance;
    enum displayGlyph cellChar = 0;
    color cellForeColor, cellBackColor, lightMultiplierColor = black, gasAugmentColor;
    boolean monsterWithDetectedItem = false, needDistinctness = false;
    short gasAugmentWeight = 0;
    creature *monst = NULL;
    item *theItem = NULL;
    enum tileType tile = NOTHING;
    enum dungeonLayers layer, maxLayer;

    assureCosmeticRNG;

    brogueAssert(isPosInMap(loc));

    if (pmapAt(loc)->flags & HAS_MONSTER) {
        monst = monsterAtLoc(loc);
    } else if (pmapAt(loc)->flags & HAS_DORMANT_MONSTER) {
        monst = dormantMonsterAtLoc(loc);
    }
    if (monst) {
        monsterWithDetectedItem = (monst->carriedItem && (monst->carriedItem->flags & ITEM_MAGIC_DETECTED)
                                   && itemMagicPolarity(monst->carriedItem) && !canSeeMonster(monst));
    }

    if (monsterWithDetectedItem) {
        theItem = monst->carriedItem;
    } else {
        theItem = itemAtLoc(loc);
    }

    if (!playerCanSeeOrSense(loc.x, loc.y)
        && !(pmapAt(loc)->flags & (ITEM_DETECTED | HAS_PLAYER))
        && (!monst || !monsterRevealed(monst))
        && !monsterWithDetectedItem
        && (pmapAt(loc)->flags & (DISCOVERED | MAGIC_MAPPED))
        && (pmapAt(loc)->flags & STABLE_MEMORY)) {

        // restore memory
        cellChar = pmapAt(loc)->rememberedAppearance.character;
        cellForeColor = colorFromComponents(pmapAt(loc)->rememberedAppearance.foreColorComponents);
        cellBackColor = colorFromComponents(pmapAt(loc)->rememberedAppearance.backColorComponents);
    } else {
        // Find the highest-priority fore color, back color and character.
        bestFCPriority = bestBCPriority = bestCharPriority = 10000;

        // Default to the appearance of floor.
        cellForeColor = *(tileCatalog[FLOOR].foreColor);
        cellBackColor = *(tileCatalog[FLOOR].backColor);
        cellChar = tileCatalog[FLOOR].displayChar;

        if (!(pmapAt(loc)->flags & DISCOVERED) && !rogue.playbackOmniscience) {
            if (pmapAt(loc)->flags & MAGIC_MAPPED) {
                maxLayer = LIQUID + 1; // Can see only dungeon and liquid layers with magic mapping.
            } else {
                maxLayer = 0; // Terrain shouldn't influence the tile appearance at all if it hasn't been discovered.
            }
        } else {
            maxLayer = NUMBER_TERRAIN_LAYERS;
        }

        for (layer = 0; layer < maxLayer; layer++) {
            // Gas shows up as a color average, not directly.
            if (pmapAt(loc)->layers[layer] && layer != GAS) {
                tile = pmapAt(loc)->layers[layer];
                if (rogue.playbackOmniscience && (tileCatalog[tile].mechFlags & TM_IS_SECRET)) {
                    tile = dungeonFeatureCatalog[tileCatalog[tile].discoverType].tile;
                }

                if (tileCatalog[tile].drawPriority < bestFCPriority
                    && tileCatalog[tile].foreColor) {

                    cellForeColor = *(tileCatalog[tile].foreColor);
                    bestFCPriority = tileCatalog[tile].drawPriority;
                }
                if (tileCatalog[tile].drawPriority < bestBCPriority
                    && tileCatalog[tile].backColor) {

                    cellBackColor = *(tileCatalog[tile].backColor);
                    bestBCPriority = tileCatalog[tile].drawPriority;
                }
                if (tileCatalog[tile].drawPriority < bestCharPriority
                    && tileCatalog[tile].displayChar) {

                    cellChar = tileCatalog[tile].displayChar;
                    bestCharPriority = tileCatalog[tile].drawPriority;
                    needDistinctness = (tileCatalog[tile].mechFlags & TM_VISUALLY_DISTINCT) ? true : false;
                }
            }
        }

        if (rogue.trueColorMode) {
            lightMultiplierColor = colorMultiplier100;
        } else {
            colorMultiplierFromDungeonLight(loc.x, loc.y, &lightMultiplierColor);
        }

        if (pmapAt(loc)->layers[GAS]
            && tileCatalog[pmapAt(loc)->layers[GAS]].backColor) {

            gasAugmentColor = *(tileCatalog[pmapAt(loc)->layers[GAS]].backColor);
            if (rogue.trueColorMode) {
                gasAugmentWeight = 30;
            } else {
                gasAugmentWeight = min(90, 30 + pmapAt(loc)->volume);
            }
        }

        if (D_DISABLE_BACKGROUND_COLORS) {
            if (COLOR_DIFF(cellBackColor, black) > COLOR_DIFF(cellForeColor, black)) {
                cellForeColor = cellBackColor;
            }
            cellBackColor = black;
            needDistinctness = true;
        }

        if (pmapAt(loc)->flags & HAS_PLAYER) {
            cellChar = player.info.displayChar;
            cellForeColor = *(player.info.foreColor);
            needDistinctness = true;
        } else if (((pmapAt(loc)->flags & HAS_ITEM) && (pmapAt(loc)->flags & ITEM_DETECTED)
                    && itemMagicPolarity(theItem)
                    && !playerCanSeeOrSense(loc.x, loc.y))
                   || monsterWithDetectedItem){

            int polarity = itemMagicPolarity(theItem);
            if (theItem->category == AMULET) {
                cellChar = G_AMULET;
                cellForeColor = white;
            } else if (polarity == -1) {
                cellChar = G_BAD_MAGIC;
                cellForeColor = badMessageColor;
            } else if (polarity == 1) {
                cellChar = G_GOOD_MAGIC;
                cellForeColor = goodMessageColor;
            } else {
                cellChar = 0;
                cellForeColor = white;
            }

            needDistinctness = true;
        } else if ((pmapAt(loc)->flags & HAS_MONSTER)
                   && (playerCanSeeOrSense(loc.x, loc.y) || ((monst->info.flags & MONST_IMMOBILE) && (pmapAt(loc)->flags & DISCOVERED)))
                   && (!monsterIsHidden(monst, &player) || rogue.playbackOmniscience)) {
            needDistinctness = true;
            if (player.status[STATUS_HALLUCINATING] > 0
                    && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
                    && !rogue.playbackOmniscience
                    && !player.status[STATUS_TELEPATHIC]) {
                cellChar = monsterCatalog[randomAnimateMonster()].displayChar;
                cellForeColor = *(monsterCatalog[randomAnimateMonster()].foreColor);
            } else {
                cellChar = monst->info.displayChar;
                cellForeColor = *(monst->info.foreColor);
                if (monst->status[STATUS_INVISIBLE] || (monst->bookkeepingFlags & MB_SUBMERGED)) {
                    // Invisible allies show up on the screen with a transparency effect.
                    //cellForeColor = cellBackColor;
                    applyColorAverage(&cellForeColor, &cellBackColor, 75);
                } else {
                    if (monst->creatureState == MONSTER_ALLY && !(monst->info.flags & MONST_INANIMATE)) {
                        if (rogue.trueColorMode) {
                            cellForeColor = white;
                        } else {
                            applyColorAverage(&cellForeColor, &pink, 50);
                        }
                    }
                }
                //DEBUG if (monst->bookkeepingFlags & MB_LEADER) applyColorAverage(&cellBackColor, &purple, 50);
            }
        } else if (monst
                   && monsterRevealed(monst)
                   && !canSeeMonster(monst)) {
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience && !player.status[STATUS_TELEPATHIC]) {
                cellChar = (rand_range(0, 1) ? 'X' : 'x');
            } else {
                cellChar = (monst->info.isLarge ? 'X' : 'x');
            }
            cellForeColor = white;
            lightMultiplierColor = white;
            if (!(pmapAt(loc)->flags & DISCOVERED)) {
                cellBackColor = black;
                gasAugmentColor = black;
            }
        } else if ((pmapAt(loc)->flags & HAS_ITEM) && !cellHasTerrainFlag(loc, T_OBSTRUCTS_ITEMS)
                   && (playerCanSeeOrSense(loc.x, loc.y) || ((pmapAt(loc)->flags & DISCOVERED) && !cellHasTerrainFlag(loc, T_MOVES_ITEMS)))) {
            needDistinctness = true;
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                cellChar = getItemCategoryGlyph(getHallucinatedItemCategory());
                cellForeColor = itemColor;
            } else {
                theItem = itemAtLoc(loc);
                cellChar = theItem->displayChar;
                cellForeColor = *(theItem->foreColor);
                // Remember the item was here
                pmapAt(loc)->rememberedItemCategory = theItem->category;
                pmapAt(loc)->rememberedItemKind = theItem->kind;
                pmapAt(loc)->rememberedItemQuantity = theItem->quantity;
                pmapAt(loc)->rememberedItemOriginDepth = theItem->originDepth;
            }
        } else if (playerCanSeeOrSense(loc.x, loc.y) || (pmapAt(loc)->flags & (DISCOVERED | MAGIC_MAPPED))) {
            // just don't want these to be plotted as black
            // Also, ensure we remember there are no items here
            pmapAt(loc)->rememberedItemCategory = 0;
            pmapAt(loc)->rememberedItemKind = 0;
            pmapAt(loc)->rememberedItemQuantity = 0;
            pmapAt(loc)->rememberedItemOriginDepth = 0;
        } else {
            *returnChar = ' ';
            *returnForeColor = black;
            *returnBackColor = undiscoveredColor;

            if (D_DISABLE_BACKGROUND_COLORS) *returnBackColor = black;

            restoreRNG;
            return;
        }

        if (gasAugmentWeight && ((pmapAt(loc)->flags & DISCOVERED) || rogue.playbackOmniscience)) {
            if (!rogue.trueColorMode || !needDistinctness) {
                applyColorAverage(&cellForeColor, &gasAugmentColor, gasAugmentWeight);
            }
            // phantoms create sillhouettes in gas clouds
            if ((pmapAt(loc)->flags & HAS_MONSTER)
                && monst->status[STATUS_INVISIBLE]
                && playerCanSeeOrSense(loc.x, loc.y)
                && !monsterRevealed(monst)
                && !monsterHiddenBySubmersion(monst, &player)) {

                if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience && !player.status[STATUS_TELEPATHIC]) {
                    cellChar = monsterCatalog[randomAnimateMonster()].displayChar;
                } else {
                    cellChar = monst->info.displayChar;
                }
                cellForeColor = cellBackColor;
            }
            applyColorAverage(&cellBackColor, &gasAugmentColor, gasAugmentWeight);
        }

        if (!(pmapAt(loc)->flags & (ANY_KIND_OF_VISIBLE | ITEM_DETECTED | HAS_PLAYER))
            && !playerCanSeeOrSense(loc.x, loc.y)
            && (!monst || !monsterRevealed(monst)) && !monsterWithDetectedItem) {

            pmapAt(loc)->flags |= STABLE_MEMORY;
            pmapAt(loc)->rememberedAppearance.character = cellChar;

            if (rogue.trueColorMode) {
                bakeTerrainColors(&cellForeColor, &cellBackColor, loc.x, loc.y);
            }

            // store memory
            storeColorComponents(pmapAt(loc)->rememberedAppearance.foreColorComponents, &cellForeColor);
            storeColorComponents(pmapAt(loc)->rememberedAppearance.backColorComponents, &cellBackColor);

            applyColorAugment(&lightMultiplierColor, &basicLightColor, 100);
            if (!rogue.trueColorMode || !needDistinctness) {
                applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
            }
            applyColorMultiplier(&cellBackColor, &lightMultiplierColor);
            bakeTerrainColors(&cellForeColor, &cellBackColor, loc.x, loc.y);

            // Then restore, so that it looks the same on this pass as it will when later refreshed.
            cellForeColor = colorFromComponents(pmapAt(loc)->rememberedAppearance.foreColorComponents);
            cellBackColor = colorFromComponents(pmapAt(loc)->rememberedAppearance.backColorComponents);
        }
    }

    // Smooth out walls: if there's a "wall-ish" tile drawn below us, just draw the wall top
    if ((cellChar == G_WALL || cellChar == G_GRANITE) && coordinatesAreInMap(loc.x, loc.y+1)
        && glyphIsWallish(displayBuffer.cells[mapToWindowX(loc.x)][mapToWindowY(loc.y+1)].character)) {
        cellChar = G_WALL_TOP;
    }

    if (((pmapAt(loc)->flags & ITEM_DETECTED) || monsterWithDetectedItem
         || (monst && monsterRevealed(monst)))
        && !playerCanSeeOrSense(loc.x, loc.y)) {
        // do nothing
    } else if (!(pmapAt(loc)->flags & VISIBLE) && (pmapAt(loc)->flags & CLAIRVOYANT_VISIBLE)) {
        // can clairvoyantly see it
        if (rogue.trueColorMode) {
            lightMultiplierColor = basicLightColor;
        } else {
            applyColorAugment(&lightMultiplierColor, &basicLightColor, 100);
        }
        if (!rogue.trueColorMode || !needDistinctness) {
            applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
            applyColorMultiplier(&cellForeColor, &clairvoyanceColor);
        }
        applyColorMultiplier(&cellBackColor, &lightMultiplierColor);
        applyColorMultiplier(&cellBackColor, &clairvoyanceColor);
    } else if (!(pmapAt(loc)->flags & VISIBLE) && (pmapAt(loc)->flags & TELEPATHIC_VISIBLE)) {
        // Can telepathically see it through another creature's eyes.

        applyColorAugment(&lightMultiplierColor, &basicLightColor, 100);

        if (!rogue.trueColorMode || !needDistinctness) {
            applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
            applyColorMultiplier(&cellForeColor, &telepathyMultiplier);
        }
        applyColorMultiplier(&cellBackColor, &lightMultiplierColor);
        applyColorMultiplier(&cellBackColor, &telepathyMultiplier);
    } else if (!(pmapAt(loc)->flags & DISCOVERED) && (pmapAt(loc)->flags & MAGIC_MAPPED)) {
        // magic mapped only
        if (!rogue.playbackOmniscience) {
            needDistinctness = false;
            if (!rogue.trueColorMode || !needDistinctness) {
                applyColorMultiplier(&cellForeColor, &magicMapColor);
            }
            applyColorMultiplier(&cellBackColor, &magicMapColor);
        }
    } else if (!(pmapAt(loc)->flags & VISIBLE) && !rogue.playbackOmniscience) {
        // if it's not visible

        needDistinctness = false;
        if (rogue.inWater) {
            applyColorAverage(&cellForeColor, &black, 80);
            applyColorAverage(&cellBackColor, &black, 80);
        } else {
            if (!cellHasTMFlag(loc, TM_BRIGHT_MEMORY)
                && (!rogue.trueColorMode || !needDistinctness)) {

                applyColorMultiplier(&cellForeColor, &memoryColor);
                applyColorAverage(&cellForeColor, &memoryOverlay, 25);
            }
            applyColorMultiplier(&cellBackColor, &memoryColor);
            applyColorAverage(&cellBackColor, &memoryOverlay, 25);
        }
    } else if (playerCanSeeOrSense(loc.x, loc.y) && rogue.playbackOmniscience && !(pmapAt(loc)->flags & ANY_KIND_OF_VISIBLE)) {
        // omniscience
        applyColorAugment(&lightMultiplierColor, &basicLightColor, 100);
        if (!rogue.trueColorMode || !needDistinctness) {
            applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
            applyColorMultiplier(&cellForeColor, &omniscienceColor);
        }
        applyColorMultiplier(&cellBackColor, &lightMultiplierColor);
        applyColorMultiplier(&cellBackColor, &omniscienceColor);
    } else {
        if (!rogue.trueColorMode || !needDistinctness) {
            applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
        }
        applyColorMultiplier(&cellBackColor, &lightMultiplierColor);

        if (player.status[STATUS_HALLUCINATING] && !rogue.trueColorMode) {
            randomizeColor(&cellForeColor, 40 * player.status[STATUS_HALLUCINATING] / 300 + 20);
            randomizeColor(&cellBackColor, 40 * player.status[STATUS_HALLUCINATING] / 300 + 20);
        }
        if (rogue.inWater) {
            applyColorMultiplier(&cellForeColor, &deepWaterLightColor);
            applyColorMultiplier(&cellBackColor, &deepWaterLightColor);
        }
    }
//   DEBUG cellBackColor.red = max(0,((scentMap[loc.x][loc.y] - rogue.scentTurnNumber) * 2) + 100);
//   DEBUG if (pmapAt(loc)->flags & KNOWN_TO_BE_TRAP_FREE) cellBackColor.red += 20;
//   DEBUG if (cellHasTerrainFlag(loc, T_IS_FLAMMABLE)) cellBackColor.red += 50;

    if (pmapAt(loc)->flags & IS_IN_PATH) {
        if (cellHasTMFlag(loc, TM_INVERT_WHEN_HIGHLIGHTED)) {
            swapColors(&cellForeColor, &cellBackColor);
        } else {
            if (!rogue.trueColorMode || !needDistinctness) {
                applyColorAverage(&cellForeColor, &yellow, rogue.cursorPathIntensity);
            }
            applyColorAverage(&cellBackColor, &yellow, rogue.cursorPathIntensity);
        }
        needDistinctness = true;
    }

    bakeTerrainColors(&cellForeColor, &cellBackColor, loc.x, loc.y);

    if (rogue.displayStealthRangeMode && (pmapAt(loc)->flags & IN_FIELD_OF_VIEW)) {
        distance = min(rogue.scentTurnNumber - scentMap[loc.x][loc.y], scentDistance(loc.x, loc.y, player.loc.x, player.loc.y));
        if (distance > rogue.stealthRange * 2) {
            applyColorAverage(&cellForeColor, &orange, 12);
            applyColorAverage(&cellBackColor, &orange, 12);
            applyColorAugment(&cellForeColor, &orange, 12);
            applyColorAugment(&cellBackColor, &orange, 12);
        }
    }

    if ((rogue.trueColorMode || rogue.displayStealthRangeMode)
        && playerCanSeeOrSense(loc.x, loc.y)) {

        if (displayDetail[loc.x][loc.y] == DV_DARK) {
            applyColorMultiplier(&cellForeColor, &inDarknessMultiplierColor);
            applyColorMultiplier(&cellBackColor, &inDarknessMultiplierColor);

            applyColorAugment(&cellForeColor, &purple, 10);
            applyColorAugment(&cellBackColor, &white, -10);
            applyColorAverage(&cellBackColor, &purple, 20);
        } else if (displayDetail[loc.x][loc.y] == DV_LIT) {

            colorMultiplierFromDungeonLight(loc.x, loc.y, &lightMultiplierColor);
            normColor(&lightMultiplierColor, 175, 50);
            //applyColorMultiplier(&cellForeColor, &lightMultiplierColor);
            //applyColorMultiplier(&cellBackColor, &lightMultiplierColor);
            applyColorAugment(&cellForeColor, &lightMultiplierColor, 5);
            applyColorAugment(&cellBackColor, &lightMultiplierColor, 5);
        }
    }

    if (needDistinctness) {
        separateColors(&cellForeColor, &cellBackColor);
    }

    if (D_SCENT_VISION) {
        if (rogue.scentTurnNumber > (unsigned short) scentMap[loc.x][loc.y]) {
            cellBackColor.red = rogue.scentTurnNumber - (unsigned short) scentMap[loc.x][loc.y];
            cellBackColor.red = clamp(cellBackColor.red, 0, 100);
        } else {
            cellBackColor.green = abs(rogue.scentTurnNumber - (unsigned short) scentMap[loc.x][loc.y]);
            cellBackColor.green = clamp(cellBackColor.green, 0, 100);
        }
    }

    *returnChar = cellChar;
    *returnForeColor = cellForeColor;
    *returnBackColor = cellBackColor;

    if (D_DISABLE_BACKGROUND_COLORS) *returnBackColor = black;
    restoreRNG;
}

void refreshDungeonCell(pos loc) {
    enum displayGlyph cellChar;
    color foreColor, backColor;
    brogueAssert(isPosInMap(loc));

    getCellAppearance(loc, &cellChar, &foreColor, &backColor);
    plotCharWithColor(cellChar, mapToWindow(loc), &foreColor, &backColor);
}

void applyColorMultiplier(color *baseColor, const color *multiplierColor) {
    baseColor->red = baseColor->red * multiplierColor->red / 100;
    baseColor->redRand = baseColor->redRand * multiplierColor->redRand / 100;
    baseColor->green = baseColor->green * multiplierColor->green / 100;
    baseColor->greenRand = baseColor->greenRand * multiplierColor->greenRand / 100;
    baseColor->blue = baseColor->blue * multiplierColor->blue / 100;
    baseColor->blueRand = baseColor->blueRand * multiplierColor->blueRand / 100;
    baseColor->rand = baseColor->rand * multiplierColor->rand / 100;
    //baseColor->colorDances *= multiplierColor->colorDances;
    return;
}

void applyColorAverage(color *baseColor, const color *newColor, short averageWeight) {
    short weightComplement = 100 - averageWeight;
    baseColor->red = (baseColor->red * weightComplement + newColor->red * averageWeight) / 100;
    baseColor->redRand = (baseColor->redRand * weightComplement + newColor->redRand * averageWeight) / 100;
    baseColor->green = (baseColor->green * weightComplement + newColor->green * averageWeight) / 100;
    baseColor->greenRand = (baseColor->greenRand * weightComplement + newColor->greenRand * averageWeight) / 100;
    baseColor->blue = (baseColor->blue * weightComplement + newColor->blue * averageWeight) / 100;
    baseColor->blueRand = (baseColor->blueRand * weightComplement + newColor->blueRand * averageWeight) / 100;
    baseColor->rand = (baseColor->rand * weightComplement + newColor->rand * averageWeight) / 100;
    baseColor->colorDances = (baseColor->colorDances || newColor->colorDances);
    return;
}

void applyColorAugment(color *baseColor, const color *augmentingColor, short augmentWeight) {
    baseColor->red += (augmentingColor->red * augmentWeight) / 100;
    baseColor->redRand += (augmentingColor->redRand * augmentWeight) / 100;
    baseColor->green += (augmentingColor->green * augmentWeight) / 100;
    baseColor->greenRand += (augmentingColor->greenRand * augmentWeight) / 100;
    baseColor->blue += (augmentingColor->blue * augmentWeight) / 100;
    baseColor->blueRand += (augmentingColor->blueRand * augmentWeight) / 100;
    baseColor->rand += (augmentingColor->rand * augmentWeight) / 100;
    return;
}

void applyColorScalar(color *baseColor, short scalar) {
    baseColor->red          = baseColor->red        * scalar / 100;
    baseColor->redRand      = baseColor->redRand    * scalar / 100;
    baseColor->green        = baseColor->green      * scalar / 100;
    baseColor->greenRand    = baseColor->greenRand  * scalar / 100;
    baseColor->blue         = baseColor->blue       * scalar / 100;
    baseColor->blueRand     = baseColor->blueRand   * scalar / 100;
    baseColor->rand         = baseColor->rand       * scalar / 100;
}

void applyColorBounds(color *baseColor, short lowerBound, short upperBound) {
    baseColor->red          = clamp(baseColor->red, lowerBound, upperBound);
    baseColor->redRand      = clamp(baseColor->redRand, lowerBound, upperBound);
    baseColor->green        = clamp(baseColor->green, lowerBound, upperBound);
    baseColor->greenRand    = clamp(baseColor->greenRand, lowerBound, upperBound);
    baseColor->blue         = clamp(baseColor->blue, lowerBound, upperBound);
    baseColor->blueRand     = clamp(baseColor->blueRand, lowerBound, upperBound);
    baseColor->rand         = clamp(baseColor->rand, lowerBound, upperBound);
}

void desaturate(color *baseColor, short weight) {
    short avg;
    avg = (baseColor->red + baseColor->green + baseColor->blue) / 3 + 1;
    baseColor->red = baseColor->red * (100 - weight) / 100 + (avg * weight / 100);
    baseColor->green = baseColor->green * (100 - weight) / 100 + (avg * weight / 100);
    baseColor->blue = baseColor->blue * (100 - weight) / 100 + (avg * weight / 100);

    avg = (baseColor->redRand + baseColor->greenRand + baseColor->blueRand);
    baseColor->redRand = baseColor->redRand * (100 - weight) / 100;
    baseColor->greenRand = baseColor->greenRand * (100 - weight) / 100;
    baseColor->blueRand = baseColor->blueRand * (100 - weight) / 100;

    baseColor->rand += avg * weight / 3 / 100;
}

static short randomizeByPercent(short input, short percent) {
    return (rand_range(input * (100 - percent) / 100, input * (100 + percent) / 100));
}

void randomizeColor(color *baseColor, short randomizePercent) {
    baseColor->red = randomizeByPercent(baseColor->red, randomizePercent);
    baseColor->green = randomizeByPercent(baseColor->green, randomizePercent);
    baseColor->blue = randomizeByPercent(baseColor->blue, randomizePercent);
}

void swapColors(color *color1, color *color2) {
    color tempColor = *color1;
    *color1 = *color2;
    *color2 = tempColor;
}

// Assumes colors are pre-baked.
static void blendAppearances(const color *fromForeColor, const color *fromBackColor, const enum displayGlyph fromChar,
                      const color *toForeColor, const color *toBackColor, const enum displayGlyph toChar,
                      color *retForeColor, color *retBackColor, enum displayGlyph *retChar,
                      const short percent) {
    // Straight average of the back color:
    *retBackColor = *fromBackColor;
    applyColorAverage(retBackColor, toBackColor, percent);

    // Pick the character:
    if (percent >= 50) {
        *retChar = toChar;
    } else {
        *retChar = fromChar;
    }

    // Pick the method for blending the fore color.
    if (fromChar == toChar) {
        // If the character isn't changing, do a straight average.
        *retForeColor = *fromForeColor;
        applyColorAverage(retForeColor, toForeColor, percent);
    } else {
        // If it is changing, the first half blends to the current back color, and the second half blends to the final back color.
        if (percent >= 50) {
            *retForeColor = *retBackColor;
            applyColorAverage(retForeColor, toForeColor, (percent - 50) * 2);
        } else {
            *retForeColor = *fromForeColor;
            applyColorAverage(retForeColor, retBackColor, percent * 2);
        }
    }
}

void irisFadeBetweenBuffers(screenDisplayBuffer* fromBuf,
                            screenDisplayBuffer* toBuf,
                            short x, short y,
                            short frameCount,
                            boolean outsideIn) {
    short i, j, frame, percentBasis, thisCellPercent;
    boolean fastForward;
    color fromBackColor, toBackColor, fromForeColor, toForeColor, currentForeColor, currentBackColor;
    enum displayGlyph fromChar, toChar, currentChar;
    short completionMap[COLS][ROWS], maxDistance;

    fastForward = false;
    frame = 1;

    // Calculate the square of the maximum distance from (x, y) that the iris will have to spread.
    if (x < COLS / 2) {
        i = COLS - x;
    } else {
        i = x;
    }
    if (y < ROWS / 2) {
        j = ROWS - y;
    } else {
        j = y;
    }
    maxDistance = i*i + j*j;

    // Generate the initial completion map as a percent of maximum distance.
    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            completionMap[i][j] = (i - x)*(i - x) + (j - y)*(j - y); // square of distance
            completionMap[i][j] = 100 * completionMap[i][j] / maxDistance; // percent of max distance
            if (outsideIn) {
                completionMap[i][j] -= 100; // translate to [-100, 0], with the origin at -100 and the farthest point at 0.
            } else {
                completionMap[i][j] *= -1; // translate to [-100, 0], with the origin at 0 and the farthest point at -100.
            }
        }
    }

    do {
        percentBasis = 10000 * frame / frameCount;

        for (i=0; i<COLS; i++) {
            for (j=0; j<ROWS; j++) {
                thisCellPercent = percentBasis * 3 / 100 + completionMap[i][j];

                fromBackColor = colorFromComponents(fromBuf->cells[i][j].backColorComponents);
                fromForeColor = colorFromComponents(fromBuf->cells[i][j].foreColorComponents);
                fromChar = fromBuf->cells[i][j].character;

                toBackColor = colorFromComponents(toBuf->cells[i][j].backColorComponents);
                toForeColor = colorFromComponents(toBuf->cells[i][j].foreColorComponents);
                toChar = toBuf->cells[i][j].character;

                blendAppearances(&fromForeColor, &fromBackColor, fromChar, &toForeColor, &toBackColor, toChar, &currentForeColor, &currentBackColor, &currentChar, clamp(thisCellPercent, 0, 100));
                plotCharWithColor(currentChar, (windowpos){ i, j }, &currentForeColor, &currentBackColor);
            }
        }

        fastForward = pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT);
        frame++;
    } while (frame <= frameCount && !fastForward);
    overlayDisplayBuffer(toBuf);
}

// takes dungeon coordinates
void colorBlendCell(short x, short y, const color *hiliteColor, short hiliteStrength) {
    enum displayGlyph displayChar;
    color foreColor, backColor;

    getCellAppearance((pos){ x, y }, &displayChar, &foreColor, &backColor);
    applyColorAverage(&foreColor, hiliteColor, hiliteStrength);
    applyColorAverage(&backColor, hiliteColor, hiliteStrength);
    plotCharWithColor(displayChar, mapToWindow((pos){ x, y }), &foreColor, &backColor);
}

// takes dungeon coordinates
void hiliteCell(short x, short y, const color *hiliteColor, short hiliteStrength, boolean distinctColors) {
    enum displayGlyph displayChar;
    color foreColor, backColor;

    assureCosmeticRNG;

    getCellAppearance((pos){ x, y }, &displayChar, &foreColor, &backColor);
    applyColorAugment(&foreColor, hiliteColor, hiliteStrength);
    applyColorAugment(&backColor, hiliteColor, hiliteStrength);
    if (distinctColors) {
        separateColors(&foreColor, &backColor);
    }
    plotCharWithColor(displayChar, mapToWindow((pos){ x, y }), &foreColor, &backColor);

    restoreRNG;
}

static short adjustedLightValue(short x) {
    if (x <= LIGHT_SMOOTHING_THRESHOLD) {
        return x;
    } else {
        return fp_sqrt(x * FP_FACTOR / LIGHT_SMOOTHING_THRESHOLD) * LIGHT_SMOOTHING_THRESHOLD / FP_FACTOR;
    }
}

void colorMultiplierFromDungeonLight(short x, short y, color *editColor) {

    editColor->red      = editColor->redRand    = adjustedLightValue(max(0, tmap[x][y].light[0]));
    editColor->green    = editColor->greenRand  = adjustedLightValue(max(0, tmap[x][y].light[1]));
    editColor->blue     = editColor->blueRand   = adjustedLightValue(max(0, tmap[x][y].light[2]));

    editColor->rand = adjustedLightValue(max(0, tmap[x][y].light[0] + tmap[x][y].light[1] + tmap[x][y].light[2]) / 3);
    editColor->colorDances = false;
}

void plotCharWithColor(enum displayGlyph inputChar, windowpos loc, const color *cellForeColor, const color *cellBackColor) {
    short oldRNG;

    short foreRed = cellForeColor->red,
    foreGreen = cellForeColor->green,
    foreBlue = cellForeColor->blue,

    backRed = cellBackColor->red,
    backGreen = cellBackColor->green,
    backBlue = cellBackColor->blue,

    foreRand, backRand;

    brogueAssert(locIsInWindow(loc));
    if (!locIsInWindow(loc)) {
        return;
    }

    if (rogue.gameHasEnded || rogue.playbackFastForward) {
        return;
    }

    //assureCosmeticRNG;
    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;

    foreRand = rand_range(0, cellForeColor->rand);
    backRand = rand_range(0, cellBackColor->rand);
    foreRed += rand_range(0, cellForeColor->redRand) + foreRand;
    foreGreen += rand_range(0, cellForeColor->greenRand) + foreRand;
    foreBlue += rand_range(0, cellForeColor->blueRand) + foreRand;
    backRed += rand_range(0, cellBackColor->redRand) + backRand;
    backGreen += rand_range(0, cellBackColor->greenRand) + backRand;
    backBlue += rand_range(0, cellBackColor->blueRand) + backRand;

    foreRed =       min(100, max(0, foreRed));
    foreGreen =     min(100, max(0, foreGreen));
    foreBlue =      min(100, max(0, foreBlue));
    backRed =       min(100, max(0, backRed));
    backGreen =     min(100, max(0, backGreen));
    backBlue =      min(100, max(0, backBlue));

    if (inputChar != ' '
        && foreRed      == backRed
        && foreGreen    == backGreen
        && foreBlue     == backBlue) {

        inputChar = ' ';
    }

    cellDisplayBuffer *target = &displayBuffer.cells[loc.window_x][loc.window_y];
    target->character = inputChar;
    target->foreColorComponents[0] = foreRed;
    target->foreColorComponents[1] = foreGreen;
    target->foreColorComponents[2] = foreBlue;
    target->backColorComponents[0] = backRed;
    target->backColorComponents[1] = backGreen;
    target->backColorComponents[2] = backBlue;

    restoreRNG;
}

void plotCharToBuffer(enum displayGlyph inputChar, windowpos loc, const color *foreColor, const color *backColor, screenDisplayBuffer *dbuf) {
    short oldRNG;

    if (!dbuf) {
        plotCharWithColor(inputChar, loc, foreColor, backColor);
        return;
    }

    brogueAssert(locIsInWindow(loc));
    if (!locIsInWindow(loc)) {
        return;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    cellDisplayBuffer* cell = &dbuf->cells[loc.window_x][loc.window_y];
    cell->foreColorComponents[0] = foreColor->red + rand_range(0, foreColor->redRand) + rand_range(0, foreColor->rand);
    cell->foreColorComponents[1] = foreColor->green + rand_range(0, foreColor->greenRand) + rand_range(0, foreColor->rand);
    cell->foreColorComponents[2] = foreColor->blue + rand_range(0, foreColor->blueRand) + rand_range(0, foreColor->rand);
    cell->backColorComponents[0] = backColor->red + rand_range(0, backColor->redRand) + rand_range(0, backColor->rand);
    cell->backColorComponents[1] = backColor->green + rand_range(0, backColor->greenRand) + rand_range(0, backColor->rand);
    cell->backColorComponents[2] = backColor->blue + rand_range(0, backColor->blueRand) + rand_range(0, backColor->rand);
    cell->character = inputChar;
    restoreRNG;
}

void plotForegroundChar(enum displayGlyph inputChar, short x, short y, const color *foreColor, boolean affectedByLighting) {
    color multColor, myColor, backColor, ignoredColor;
    enum displayGlyph ignoredChar;

    myColor = *foreColor;
    getCellAppearance((pos){ x, y }, &ignoredChar, &ignoredColor, &backColor);
    if (affectedByLighting) {
        colorMultiplierFromDungeonLight(x, y, &multColor);
        applyColorMultiplier(&myColor, &multColor);
    }
    plotCharWithColor(inputChar, mapToWindow((pos){ x, y }), &myColor, &backColor);
}

// Debug feature: display the level to the screen without regard to lighting, field of view, etc.
void dumpLevelToScreen() {
    short i, j;
    pcell backup;

    assureCosmeticRNG;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].layers[DUNGEON] != GRANITE
                || (pmap[i][j].flags & DISCOVERED)) {

                backup = pmap[i][j];
                pmap[i][j].flags |= (VISIBLE | DISCOVERED);
                tmap[i][j].light[0] = 100;
                tmap[i][j].light[1] = 100;
                tmap[i][j].light[2] = 100;
                refreshDungeonCell((pos){ i, j });
                pmap[i][j] = backup;
            } else {
                plotCharWithColor(' ', mapToWindow((pos){ i, j }), &white, &black);
            }

        }
    }
    restoreRNG;
}

// To be used immediately after dumpLevelToScreen() above.
// Highlight the portion indicated by hiliteCharGrid with the hiliteColor at the hiliteStrength -- both latter arguments are optional.
void hiliteCharGrid(char hiliteCharGrid[DCOLS][DROWS], const color *hiliteColor, short hiliteStrength) {
    short i, j, x, y;
    color hCol;

    assureCosmeticRNG;

    if (hiliteColor) {
        hCol = *hiliteColor;
    } else {
        hCol = yellow;
    }

    bakeColor(&hCol);

    if (!hiliteStrength) {
        hiliteStrength = 75;
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (hiliteCharGrid[i][j]) {
                x = mapToWindowX(i);
                y = mapToWindowY(j);

                cellDisplayBuffer *cell = &displayBuffer.cells[x][y];
                cell->backColorComponents[0] = clamp(cell->backColorComponents[0] + hCol.red * hiliteStrength / 100, 0, 100);
                cell->backColorComponents[1] = clamp(cell->backColorComponents[1] + hCol.green * hiliteStrength / 100, 0, 100);
                cell->backColorComponents[2] = clamp(cell->backColorComponents[2] + hCol.blue * hiliteStrength / 100, 0, 100);
                cell->foreColorComponents[0] = clamp(cell->foreColorComponents[0] + hCol.red * hiliteStrength / 100, 0, 100);
                cell->foreColorComponents[1] = clamp(cell->foreColorComponents[1] + hCol.green * hiliteStrength / 100, 0, 100);
                cell->foreColorComponents[2] = clamp(cell->foreColorComponents[2] + hCol.blue * hiliteStrength / 100, 0, 100);
            }
        }
    }
    restoreRNG;
}

void blackOutScreen() {
    short i, j;

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            plotCharWithColor(' ', (windowpos){ i, j }, &black, &black);
        }
    }
}

void colorOverDungeon(const color *color) {
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            plotCharWithColor(' ', mapToWindow((pos){ i, j }), color, color);
        }
    }
}

void copyDisplayBuffer(screenDisplayBuffer *toBuf, screenDisplayBuffer *fromBuf) {
    *toBuf = *fromBuf;
}

void clearDisplayBuffer(screenDisplayBuffer *dbuf) {
    short i, j, k;

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            dbuf->cells[i][j].character = ' ';
            for (k=0; k<3; k++) {
                dbuf->cells[i][j].foreColorComponents[k] = 0;
                dbuf->cells[i][j].backColorComponents[k] = 0;
            }
            dbuf->cells[i][j].opacity = 0;
        }
    }
}

color colorFromComponents(const char rgb[3]) {
    color theColor = black;
    theColor.red    = rgb[0];
    theColor.green  = rgb[1];
    theColor.blue   = rgb[2];
    return theColor;
}

SavedDisplayBuffer saveDisplayBuffer(void) {
    return (SavedDisplayBuffer) { .savedScreen = displayBuffer };
}
void restoreDisplayBuffer(const SavedDisplayBuffer *savedBuf) {
    displayBuffer = savedBuf->savedScreen;
}

// draws overBuf over the current display with per-cell pseudotransparency as specified in overBuf.
void overlayDisplayBuffer(const screenDisplayBuffer *overBuf) {
    for (int i=0; i<COLS; i++) {
        for (int j=0; j<ROWS; j++) {
            if (overBuf->cells[i][j].opacity != 0) {
                color foreColor, backColor, tempColor;
                enum displayGlyph character;
                backColor = colorFromComponents(overBuf->cells[i][j].backColorComponents);

                // character and fore color:
                if (overBuf->cells[i][j].character == ' ') { // Blank cells in the overbuf take the character from the screen.
                    character = displayBuffer.cells[i][j].character;
                    foreColor = colorFromComponents(displayBuffer.cells[i][j].foreColorComponents);
                    applyColorAverage(&foreColor, &backColor, overBuf->cells[i][j].opacity);
                } else {
                    character = overBuf->cells[i][j].character;
                    foreColor = colorFromComponents(overBuf->cells[i][j].foreColorComponents);
                }

                // back color:
                tempColor = colorFromComponents(displayBuffer.cells[i][j].backColorComponents);
                applyColorAverage(&backColor, &tempColor, 100 - overBuf->cells[i][j].opacity);

                plotCharWithColor(character, (windowpos){ i, j }, &foreColor, &backColor);
            }
        }
    }
}

// Takes a list of locations, a color and a list of strengths and flashes the foregrounds of those locations.
// Strengths are percentages measuring how hard the color flashes at its peak.
void flashForeground(short *x, short *y, const color **flashColor, short *flashStrength, short count, short frames) {
    short i, j, percent;
    enum displayGlyph *displayChar;
    color *bColor, *fColor, newColor;
    short oldRNG;

    if (count <= 0) {
        return;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    displayChar = (enum displayGlyph *) malloc(count * sizeof(enum displayGlyph));
    fColor = (color *) malloc(count * sizeof(color));
    bColor = (color *) malloc(count * sizeof(color));

    for (i=0; i<count; i++) {
        getCellAppearance((pos){ x[i], y[i] }, &displayChar[i], &fColor[i], &bColor[i]);
        bakeColor(&fColor[i]);
        bakeColor(&bColor[i]);
    }

    for (j=frames; j>= 0; j--) {
        for (i=0; i<count; i++) {
            percent = flashStrength[i] * j / frames;
            newColor = fColor[i];
            applyColorAverage(&newColor, flashColor[i], percent);
            plotCharWithColor(displayChar[i], mapToWindow((pos){ x[i], y[i] }), &newColor, &(bColor[i]));
        }
        if (j) {
            if (pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT)) {
                j = 1;
            }
        }
    }

    free(displayChar);
    free(fColor);
    free(bColor);

    restoreRNG;
}

void flashCell(const color *theColor, short frames, short x, short y) {
    short i;
    boolean interrupted = false;

    for (i=0; i<frames && !interrupted; i++) {
        colorBlendCell(x, y, theColor, 100 - 100 * i / frames);
        interrupted = pauseAnimation(50, PAUSE_BEHAVIOR_DEFAULT);
    }

    refreshDungeonCell((pos){ x, y });
}

// special effect expanding flash of light at dungeon coordinates (x, y) restricted to tiles with matching flags
void colorFlash(const color *theColor, unsigned long reqTerrainFlags,
                unsigned long reqTileFlags, short frames, short maxRadius, short x, short y) {
    short i, j, k, intensity, currentRadius, fadeOut;
    short localRadius[DCOLS][DROWS];
    boolean tileQualifies[DCOLS][DROWS], aTileQualified, fastForward;

    aTileQualified = false;
    fastForward = false;

    for (i = max(x - maxRadius, 0); i <= min(x + maxRadius, DCOLS - 1); i++) {
        for (j = max(y - maxRadius, 0); j <= min(y + maxRadius, DROWS - 1); j++) {
            if ((!reqTerrainFlags || cellHasTerrainFlag((pos){ reqTerrainFlags, i }, j))
                && (!reqTileFlags || (pmap[i][j].flags & reqTileFlags))
                && (i-x) * (i-x) + (j-y) * (j-y) <= maxRadius * maxRadius) {

                tileQualifies[i][j] = true;
                localRadius[i][j] = fp_sqrt(((i-x) * (i-x) + (j-y) * (j-y)) * FP_FACTOR) / FP_FACTOR;
                aTileQualified = true;
            } else {
                tileQualifies[i][j] = false;
            }
        }
    }

    if (!aTileQualified) {
        return;
    }

    for (k = 1; k <= frames; k++) {
        currentRadius = max(1, maxRadius * k / frames);
        fadeOut = min(100, (frames - k) * 100 * 5 / frames);
        for (i = max(x - maxRadius, 0); i <= min(x + maxRadius, DCOLS - 1); i++) {
            for (j = max(y - maxRadius, 0); j <= min(y + maxRadius, DROWS - 1); j++) {
                if (tileQualifies[i][j] && (localRadius[i][j] <= currentRadius)) {

                    intensity = 100 - 100 * (currentRadius - localRadius[i][j] - 2) / currentRadius;
                    intensity = fadeOut * intensity / 100;

                    hiliteCell(i, j, theColor, intensity, false);
                }
            }
        }
        if (!fastForward && (rogue.playbackFastForward || pauseAnimation(50, PAUSE_BEHAVIOR_DEFAULT))) {
            k = frames - 1;
            fastForward = true;
        }
    }
}

#define bCurve(x)   (((x) * (x) + 11) / (10 * ((x) * (x) + 1)) - 0.1)

// x and y are global coordinates, not within the playing square
void funkyFade(screenDisplayBuffer *displayBuf, const color *colorStart,
               const color *colorEnd, short stepCount, short x, short y, boolean invert) {
    short i, j, n, weight;
    double x2, y2, weightGrid[COLS][ROWS][3], percentComplete;
    color tempColor, colorMid, foreColor, backColor;
    enum displayGlyph tempChar;
    short **distanceMap;
    boolean fastForward;

    assureCosmeticRNG;

    fastForward = false;
    distanceMap = allocGrid();
    fillGrid(distanceMap, 0);
    calculateDistances(distanceMap, player.loc.x, player.loc.y, T_OBSTRUCTS_PASSABILITY, 0, true, true);

    for (i=0; i<COLS; i++) {
        x2 = (double) ((i - x) * 5.0 / COLS);
        for (j=0; j<ROWS; j++) {
            y2 = (double) ((j - y) * 2.5 / ROWS);

            weightGrid[i][j][0] = bCurve(x2*x2+y2*y2) * (.7 + .3 * cos(5*x2*x2) * cos(5*y2*y2));
            weightGrid[i][j][1] = bCurve(x2*x2+y2*y2) * (.7 + .3 * sin(5*x2*x2) * cos(5*y2*y2));
            weightGrid[i][j][2] = bCurve(x2*x2+y2*y2);
        }
    }

    for (n=(invert ? stepCount - 1 : 0); (invert ? n >= 0 : n <= stepCount); n += (invert ? -1 : 1)) {
        for (i=0; i<COLS; i++) {
            for (j=0; j<ROWS; j++) {

                percentComplete = (double) (n) * 100 / stepCount;

                colorMid = *colorStart;
                if (colorEnd) {
                    applyColorAverage(&colorMid, colorEnd, n * 100 / stepCount);
                }

                // the fade color floods the reachable dungeon tiles faster
                if (!invert && coordinatesAreInMap(windowToMapX(i), windowToMapY(j))
                    && distanceMap[windowToMapX(i)][windowToMapY(j)] >= 0 && distanceMap[windowToMapX(i)][windowToMapY(j)] < 30000) {
                    percentComplete *= 1.0 + (100.0 - min(100, distanceMap[windowToMapX(i)][windowToMapY(j)])) / 100.;
                }

                weight = (short)(percentComplete + weightGrid[i][j][2] * percentComplete * 10);
                weight = min(100, weight);
                tempColor = black;

                tempColor.red = (short)(percentComplete + weightGrid[i][j][0] * percentComplete * 10) * colorMid.red / 100;
                tempColor.red = min(colorMid.red, tempColor.red);

                tempColor.green = (short)(percentComplete + weightGrid[i][j][1] * percentComplete * 10) * colorMid.green / 100;
                tempColor.green = min(colorMid.green, tempColor.green);

                tempColor.blue = (short)(percentComplete + weightGrid[i][j][2] * percentComplete * 10) * colorMid.blue / 100;
                tempColor.blue = min(colorMid.blue, tempColor.blue);

                backColor = black;

                backColor.red = displayBuf->cells[i][j].backColorComponents[0];
                backColor.green = displayBuf->cells[i][j].backColorComponents[1];
                backColor.blue = displayBuf->cells[i][j].backColorComponents[2];

                foreColor = (invert ? white : black);

                if (j == (MESSAGE_LINES - 1)
                    && i >= mapToWindowX(0)
                    && i < mapToWindowX(strLenWithoutEscapes(displayedMessage[MESSAGE_LINES - j - 1]))) {
                    tempChar = displayedMessage[MESSAGE_LINES - j - 1][windowToMapX(i)];
                } else {
                    tempChar = displayBuf->cells[i][j].character;

                    foreColor.red = displayBuf->cells[i][j].foreColorComponents[0];
                    foreColor.green = displayBuf->cells[i][j].foreColorComponents[1];
                    foreColor.blue = displayBuf->cells[i][j].foreColorComponents[2];

                    applyColorAverage(&foreColor, &tempColor, weight);
                }
                applyColorAverage(&backColor, &tempColor, weight);
                plotCharWithColor(tempChar, (windowpos){ i, j }, &foreColor, &backColor);
            }
        }
        if (!fastForward && pauseAnimation(16, PAUSE_BEHAVIOR_DEFAULT)) {
            // drop the event - skipping the transition should only skip the transition
            rogueEvent event;
            nextKeyOrMouseEvent(&event, false, false);
            fastForward = true;
            n = (invert ? 1 : stepCount - 2);
        }
    }

    freeGrid(distanceMap);

    restoreRNG;
}

static void displayWaypoints() {
    short i, j, w, lowestDistance;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            lowestDistance = 30000;
            for (w=0; w<rogue.wpCount; w++) {
                if (rogue.wpDistance[w][i][j] < lowestDistance) {
                    lowestDistance = rogue.wpDistance[w][i][j];
                }
            }
            if (lowestDistance < 10) {
                hiliteCell(i, j, &white, clamp(100 - lowestDistance*15, 0, 100), true);
            }
        }
    }
    temporaryMessage("Waypoints:", REQUIRE_ACKNOWLEDGMENT);
    displayLevel();
}

static void displayMachines() {
    short i, j;
    color foreColor, backColor, machineColors[50];
    enum displayGlyph dchar;

    assureCosmeticRNG;

    for (i=0; i<50; i++) {
        machineColors[i] = black;
        machineColors[i].red = rand_range(0, 100);
        machineColors[i].green = rand_range(0, 100);
        machineColors[i].blue = rand_range(0, 100);
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].machineNumber) {
                getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
                applyColorAugment(&backColor, &(machineColors[pmap[i][j].machineNumber]), 50);
                //plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
                if (pmap[i][j].machineNumber < 10) {
                    dchar ='0' + pmap[i][j].machineNumber;
                } else if (pmap[i][j].machineNumber < 10 + 26) {
                    dchar = 'a' + pmap[i][j].machineNumber - 10;
                } else {
                    dchar = 'A' + pmap[i][j].machineNumber - 10 - 26;
                }
                plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
            }
        }
    }
    temporaryMessage("Machines:", REQUIRE_ACKNOWLEDGMENT);
    displayLevel();

    restoreRNG;
}

#define CHOKEMAP_DISPLAY_CUTOFF 160
static void displayChokeMap() {
    short i, j;
    color foreColor, backColor;
    enum displayGlyph dchar;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (chokeMap[i][j] < CHOKEMAP_DISPLAY_CUTOFF) {
                if (pmap[i][j].flags & IS_GATE_SITE) {
                    getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
                    applyColorAugment(&backColor, &teal, 50);
                    plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
                } else
                    if (chokeMap[i][j] < CHOKEMAP_DISPLAY_CUTOFF) {
                    getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
                    applyColorAugment(&backColor, &red, 100 - chokeMap[i][j] * 100 / CHOKEMAP_DISPLAY_CUTOFF);
                    plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
                }
            }
        }
    }
    temporaryMessage("Choke map:", REQUIRE_ACKNOWLEDGMENT);
    displayLevel();
}

static void displayLoops() {
    short i, j;
    color foreColor, backColor;
    enum displayGlyph dchar;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].flags & IN_LOOP) {
                getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
                applyColorAugment(&backColor, &yellow, 50);
                plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
                //colorBlendCell(i, j, &tempColor, 100);//hiliteCell(i, j, &tempColor, 100, true);
            }
            if (pmap[i][j].flags & IS_CHOKEPOINT) {
                getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
                applyColorAugment(&backColor, &teal, 50);
                plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &backColor);
            }
        }
    }
    temporaryMessage("Loops:", REQUIRE_ACKNOWLEDGMENT);
    displayLevel();
}

static void exploreKey(const boolean controlKey) {
    short x, y, finalX = 0, finalY = 0;
    short **exploreMap;
    enum directions dir;
    boolean tooDark = false;

    // fight any adjacent enemies first
    dir = adjacentFightingDir();
    if (dir == NO_DIRECTION) {
        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            x = player.loc.x + nbDirs[dir][0];
            y = player.loc.y + nbDirs[dir][1];
            if (coordinatesAreInMap(x, y)
                && !(pmap[x][y].flags & DISCOVERED)) {

                tooDark = true;
                break;
            }
        }
        if (!tooDark) {
            x = finalX = player.loc.x;
            y = finalY = player.loc.y;

            exploreMap = allocGrid();
            getExploreMap(exploreMap, false);
            do {
                dir = nextStep(exploreMap, x, y, NULL, false);
                if (dir != NO_DIRECTION) {
                    x += nbDirs[dir][0];
                    y += nbDirs[dir][1];
                    if (pmap[x][y].flags & (DISCOVERED | MAGIC_MAPPED)) {
                        finalX = x;
                        finalY = y;
                    }
                }
            } while (dir != NO_DIRECTION);
            freeGrid(exploreMap);
        }
    } else {
        x = finalX = player.loc.x + nbDirs[dir][0];
        y = finalY = player.loc.y + nbDirs[dir][1];
    }

    if (tooDark) {
        message("It's too dark to explore!", 0);
    } else if (x == player.loc.x && y == player.loc.y) {
        message("I see no path for further exploration.", 0);
    } else if (proposeOrConfirmLocation(finalX, finalY, "I see no path for further exploration.")) {
        explore(controlKey ? 1 : 20); // Do the exploring until interrupted.
        hideCursor();
        exploreKey(controlKey);
    }
}

boolean pauseBrogue(short milliseconds, PauseBehavior behavior) {
    commitDraws();
    if (rogue.playbackMode && rogue.playbackFastForward) {
        return true;
    }
    // For long delays, let's pause in small increments so that we can immediately react to user interruptions.
    while (milliseconds > 100) {
        if (pauseForMilliseconds(50, behavior)) return true;
        milliseconds -= 50;
    }
    return pauseForMilliseconds(milliseconds, behavior);
}

// Same as pauseBrogue, but during playback the delay scales according to playback speed.
boolean pauseAnimation(short milliseconds, PauseBehavior behavior) {
    if (rogue.playbackMode && !rogue.playbackPaused && milliseconds > 0) {
        double factor = rogue.playbackDelayPerTurn / (double)DEFAULT_PLAYBACK_DELAY;
        if (factor > 1.) factor = sqrt(factor); // so that animations don't slow down too much
        milliseconds = max(1, lround(milliseconds * factor));
    }
    return pauseBrogue(milliseconds, behavior);
}

void nextBrogueEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance, boolean realInputEvenInPlayback) {
    rogueEvent recordingInput;
    boolean repeatAgain, interaction;
    short pauseDuration;

    returnEvent->eventType = EVENT_ERROR;

    if (rogue.playbackMode && !realInputEvenInPlayback) {
        do {
            repeatAgain = false;
            if ((!rogue.playbackFastForward && rogue.playbackBetweenTurns)
                || rogue.playbackOOS) {

                pauseDuration = (rogue.playbackPaused ? DEFAULT_PLAYBACK_DELAY : rogue.playbackDelayThisTurn);
                if (pauseDuration && pauseBrogue(pauseDuration, PAUSE_BEHAVIOR_DEFAULT)) {
                    // if the player did something during playback
                    nextBrogueEvent(&recordingInput, false, false, true);
                    interaction = executePlaybackInput(&recordingInput);
                    repeatAgain = !rogue.playbackPaused && interaction;
                }
            }
        } while ((repeatAgain || rogue.playbackOOS) && !rogue.gameHasEnded);
        rogue.playbackDelayThisTurn = rogue.playbackDelayPerTurn;
        recallEvent(returnEvent);
    } else {
        commitDraws();
        if (rogue.creaturesWillFlashThisTurn) {
            displayMonsterFlashes(true);
        }
        do {
            nextKeyOrMouseEvent(returnEvent, textInput, colorsDance); // No mouse clicks outside of the window will register.
        } while (returnEvent->eventType == MOUSE_UP && !locIsInWindow((windowpos){ returnEvent->param1, returnEvent->param2 }));
        // recording done elsewhere
    }

    if (returnEvent->eventType == EVENT_ERROR) {
        rogue.playbackPaused = rogue.playbackMode; // pause if replaying
        message("Event error!", REQUIRE_ACKNOWLEDGMENT);
    }
}

void executeMouseClick(rogueEvent *theEvent) {
    short x, y;
    boolean autoConfirm;
    x = theEvent->param1;
    y = theEvent->param2;
    autoConfirm = theEvent->controlKey;

    if (theEvent->eventType == RIGHT_MOUSE_UP) {
        displayInventory(ALL_ITEMS, 0, 0, true, true);
    } else if (coordinatesAreInMap(windowToMapX(x), windowToMapY(y))) {
        if (autoConfirm) {
            travel(windowToMapX(x), windowToMapY(y), autoConfirm);
        } else {
            rogue.cursorLoc.x = windowToMapX(x);
            rogue.cursorLoc.y = windowToMapY(y);
            mainInputLoop();
        }

    } else if (windowToMapX(x) >= 0 && windowToMapX(x) < DCOLS && y >= 0 && y < MESSAGE_LINES) {
        // If the click location is in the message block, display the message archive.
        displayMessageArchive();
    }
}

void executeKeystroke(signed long keystroke, boolean controlKey, boolean shiftKey) {
    short direction = -1;

    confirmMessages();
    stripShiftFromMovementKeystroke(&keystroke);

    switch (keystroke) {
        case UP_KEY:
        case UP_ARROW:
        case NUMPAD_8:
            direction = UP;
            break;
        case DOWN_KEY:
        case DOWN_ARROW:
        case NUMPAD_2:
            direction = DOWN;
            break;
        case LEFT_KEY:
        case LEFT_ARROW:
        case NUMPAD_4:
            direction = LEFT;
            break;
        case RIGHT_KEY:
        case RIGHT_ARROW:
        case NUMPAD_6:
            direction = RIGHT;
            break;
        case NUMPAD_7:
        case UPLEFT_KEY:
            direction = UPLEFT;
            break;
        case UPRIGHT_KEY:
        case NUMPAD_9:
            direction = UPRIGHT;
            break;
        case DOWNLEFT_KEY:
        case NUMPAD_1:
            direction = DOWNLEFT;
            break;
        case DOWNRIGHT_KEY:
        case NUMPAD_3:
            direction = DOWNRIGHT;
            break;
        case DESCEND_KEY:
            considerCautiousMode();
            if (D_WORMHOLING) {
                recordKeystroke(DESCEND_KEY, false, false);
                useStairs(1);
            } else if (proposeOrConfirmLocation(rogue.downLoc.x, rogue.downLoc.y, "I see no way down.")) {
                travel(rogue.downLoc.x, rogue.downLoc.y, true);
            }
            break;
        case ASCEND_KEY:
            considerCautiousMode();
            if (D_WORMHOLING) {
                recordKeystroke(ASCEND_KEY, false, false);
                useStairs(-1);
            } else if (proposeOrConfirmLocation(rogue.upLoc.x, rogue.upLoc.y, "I see no way up.")) {
                travel(rogue.upLoc.x, rogue.upLoc.y, true);
            }
            break;
        case RETURN_KEY:
            showCursor();
            break;
        case REST_KEY:
        case PERIOD_KEY:
        case NUMPAD_5:
            considerCautiousMode();
            rogue.justRested = true;
            recordKeystroke(REST_KEY, false, false);
            playerTurnEnded();
            break;
        case AUTO_REST_KEY:
            rogue.justRested = true;
            autoRest();
            break;
        case SEARCH_KEY:
            if (controlKey) {
                rogue.disturbed = false;
                rogue.automationActive = true;
                do {
                    manualSearch();
                    if (pauseAnimation(80, PAUSE_BEHAVIOR_DEFAULT)) {
                        rogue.disturbed = true;
                    }
                } while (player.status[STATUS_SEARCHING] < 5 && !rogue.disturbed);
                rogue.automationActive = false;
            } else {
                manualSearch();
            }
            break;
        case INVENTORY_KEY:
            displayInventory(ALL_ITEMS, 0, 0, true, true);
            break;
        case EQUIP_KEY:
            equip(NULL);
            break;
        case UNEQUIP_KEY:
            unequip(NULL);
            break;
        case DROP_KEY:
            drop(NULL);
            break;
        case APPLY_KEY:
            apply(NULL);
            break;
        case THROW_KEY:
            throwCommand(NULL, false);
            break;
        case RETHROW_KEY:
            if (rogue.lastItemThrown != NULL && itemIsCarried(rogue.lastItemThrown)) {
                throwCommand(rogue.lastItemThrown, true);
            }
            break;
        case RELABEL_KEY:
            relabel(NULL);
            break;
        case SWAP_KEY:
            swapLastEquipment();
            break;
        case TRUE_COLORS_KEY:
            rogue.trueColorMode = !rogue.trueColorMode;
            displayLevel();
            refreshSideBar(-1, -1, false);
            if (rogue.trueColorMode) {
                messageWithColor(KEYBOARD_LABELS ? "Color effects disabled. Press '\\' again to enable." : "Color effects disabled.",
                                 &teal, 0);
            } else {
                messageWithColor(KEYBOARD_LABELS ? "Color effects enabled. Press '\\' again to disable." : "Color effects enabled.",
                                 &teal, 0);
            }
            break;
        case STEALTH_RANGE_KEY:
            rogue.displayStealthRangeMode = !rogue.displayStealthRangeMode;
            displayLevel();
            refreshSideBar(-1, -1, false);
            if (rogue.displayStealthRangeMode) {
                messageWithColor(KEYBOARD_LABELS ? "Stealth range displayed. Press ']' again to hide." : "Stealth range displayed.",
                                 &teal, 0);
            } else {
                messageWithColor(KEYBOARD_LABELS ? "Stealth range hidden. Press ']' again to display." : "Stealth range hidden.",
                                 &teal, 0);
            }
            break;
        case CALL_KEY:
            call(NULL);
            break;
        case EXPLORE_KEY:
            considerCautiousMode();
            exploreKey(controlKey);
            break;
        case AUTOPLAY_KEY:
            if (confirm("Turn on autopilot?", false)) {
                autoPlayLevel(controlKey);
            }
            break;
        case MESSAGE_ARCHIVE_KEY:
            displayMessageArchive();
            break;
        case BROGUE_HELP_KEY:
            printHelpScreen();
            break;
        case FEATS_KEY:
            displayFeatsScreen();
            break;
        case DISCOVERIES_KEY:
            printDiscoveriesScreen();
            break;
        case CREATE_ITEM_MONSTER_KEY:
            DEBUG {
                dialogCreateItemOrMonster();
            }
            break;
        case SAVE_GAME_KEY:
            if (rogue.playbackMode || serverMode) {
                return;
            }
            if (confirm("Save this game and exit?", false)) {
                saveGame();
            }
            break;
        case NEW_GAME_KEY:
            if (rogue.playerTurnNumber < 50 || confirm("End this game and begin a new game?", false)) {
                rogue.nextGame = NG_NEW_GAME;
                rogue.gameHasEnded = true;
            }
            break;
        case QUIT_KEY:
            if (confirm("Quit and abandon this game? (The save will be deleted.)", false)) {
                recordKeystroke(QUIT_KEY, false, false);
                rogue.quit = true;
                gameOver("Quit", true);
            }
            break;
        case GRAPHICS_KEY:
            if (hasGraphics) {
                graphicsMode = setGraphicsMode((graphicsMode + 1) % 3);
                switch (graphicsMode) {
                    case TEXT_GRAPHICS:
                        messageWithColor(KEYBOARD_LABELS
                            ? "Switched to text mode. Press 'G' again to enable tiles."
                            : "Switched to text mode.", &teal, 0);
                        break;
                    case TILES_GRAPHICS:
                        messageWithColor(KEYBOARD_LABELS
                            ? "Switched to graphical tiles. Press 'G' again to enable hybrid mode."
                            : "Switched to graphical tiles.", &teal, 0);
                        break;
                    case HYBRID_GRAPHICS:
                        messageWithColor(KEYBOARD_LABELS
                            ? "Switched to hybrid mode. Press 'G' again to disable tiles."
                            : "Switched to hybrid mode.", &teal, 0);
                        break;
                }
            }
            break;
        case SEED_KEY:
            /*DEBUG {
                screenDisplayBuffer dbuf;
                copyDisplayBuffer(&dbuf, &displayBuffer);
                funkyFade(dbuf, &white, 0, 100, mapToWindowX(player.loc.x), mapToWindowY(player.loc.y), false);
            }*/
            DEBUG displayLoops();
            DEBUG displayChokeMap();
            DEBUG displayMachines();
            DEBUG displayWaypoints();
            
#ifdef LOG_LIGHTS
            logLights();
#endif
            // DEBUG {displayGrid(safetyMap); displayMoreSign(); displayLevel();}
            // parseFile();
            // DEBUG spawnDungeonFeature(player.loc.x, player.loc.y, &dungeonFeatureCatalog[DF_METHANE_GAS_ARMAGEDDON], true, false);
            printSeed();
            break;
        case EASY_MODE_KEY:
            //if (shiftKey) {
                enableEasyMode();
            //}
            break;
        case PRINTSCREEN_KEY:
            if (takeScreenshot()) {
                flashTemporaryAlert(" Screenshot saved in save directory ", 2000);
            }
            break;
        default:
            break;
    }
    if (direction >= 0) { // if it was a movement command
        hideCursor();
        considerCautiousMode();
        if (controlKey || shiftKey) {
            playerRuns(direction);
        } else {
            playerMoves(direction);
        }
        refreshSideBar(-1, -1, false);
    }

    if (D_SAFETY_VISION) {
        displayGrid(safetyMap);
    }
    if (rogue.trueColorMode || D_SCENT_VISION) {
        displayLevel();
    }

    rogue.cautiousMode = false;
}

boolean getInputTextString(char *inputText,
                           const char *prompt,
                           short maxLength,
                           char *defaultEntry,
                           const char *promptSuffix,
                           short textEntryType,
                           boolean useDialogBox) {
    short charNum, i, x, y, promptSuffixLen, defaultEntrylengthOverflow;
    char keystroke, suffix[100];
    const short textEntryBounds[TEXT_INPUT_TYPES][2] = {{' ', '~'}, {' ', '~'}, {'0', '9'}};
    screenDisplayBuffer dbuf;
    SavedDisplayBuffer rbuf;

    // handle defaultEntry values exceeding maxLength
    promptSuffixLen = strlen(promptSuffix);
    defaultEntrylengthOverflow = strlen(defaultEntry) + promptSuffixLen - maxLength;
    if(defaultEntrylengthOverflow > 0) {
        defaultEntry[strlen(defaultEntry) - defaultEntrylengthOverflow] = '\0';
    }

    // x and y mark the origin for text entry.
    if (useDialogBox) {
        x = (COLS - max(maxLength, strLenWithoutEscapes(prompt))) / 2;
        y = ROWS / 2 - 1;
        clearDisplayBuffer(&dbuf);
        rectangularShading(x - 1, y - 2, max(maxLength, strLenWithoutEscapes(prompt)) + 2,
                           4, &interfaceBoxColor, INTERFACE_OPACITY, &dbuf);
        rbuf = saveDisplayBuffer();
        overlayDisplayBuffer(&dbuf);
        printString(prompt, x, y - 1, &white, &interfaceBoxColor, NULL);
        for (i=0; i<maxLength; i++) {
            plotCharWithColor(' ', (windowpos){ x + i, y }, &black, &black);
        }
        printString(defaultEntry, x, y, &white, &black, 0);
    } else {
        confirmMessages();
        x = mapToWindowX(strLenWithoutEscapes(prompt));
        y = MESSAGE_LINES - 1;
        temporaryMessage(prompt, 0);
        printString(defaultEntry, x, y, &white, &black, 0);
    }

    maxLength = min(maxLength, COLS - x);


    if (inputText != defaultEntry) {
        strcpy(inputText, defaultEntry);
    }
    charNum = strLenWithoutEscapes(inputText);
    for (i = charNum; i < maxLength; i++) {
        inputText[i] = ' ';
    }

    if (promptSuffix[0] == '\0') { // empty suffix
        strcpy(suffix, " "); // so that deleting doesn't leave a white trail
    } else {
        strcpy(suffix, promptSuffix);
    }

    do {
        printString(suffix, charNum + x, y, &gray, &black, 0);
        plotCharWithColor((suffix[0] ? suffix[0] : ' '), (windowpos){ x + charNum, y }, &black, &white);
        keystroke = nextKeyPress(true);
        if (keystroke == DELETE_KEY && charNum > 0) {
            printString(suffix, charNum + x - 1, y, &gray, &black, 0);
            plotCharWithColor(' ', (windowpos){ x + charNum + strlen(suffix) - 1, y }, &black, &black);
            charNum--;
            inputText[charNum] = ' ';
        } else if (keystroke >= textEntryBounds[textEntryType][0]
                   && keystroke <= textEntryBounds[textEntryType][1]) { // allow only permitted input

            if (textEntryType == TEXT_INPUT_FILENAME
                && characterForbiddenInFilename(keystroke)) {

                keystroke = '-';
            }

            inputText[charNum] = keystroke;
            plotCharWithColor(keystroke, (windowpos){ x + charNum, y }, &white, &black);
            if (charNum < maxLength - promptSuffixLen) {
                printString(suffix, charNum + x + 1, y, &gray, &black, 0);
                charNum++;
            }
        }
#ifdef USE_CLIPBOARD
        else if (keystroke == TAB_KEY) {
            char* clipboard = getClipboard();
            for (int i=0; i<(int) min(strlen(clipboard), (unsigned long) (maxLength - charNum)); ++i) {

                char character = clipboard[i];

                if (character >= textEntryBounds[textEntryType][0]
                    && character <= textEntryBounds[textEntryType][1]) { // allow only permitted input
                    if (textEntryType == TEXT_INPUT_FILENAME
                        && characterForbiddenInFilename(character)) {
                        character = '-';
                    }
                    plotCharWithColor(character, (windowpos){ x + charNum, y }, &white, &black);
                    if (charNum < maxLength) {
                        charNum++;
                    }
                }
            }
        }
#endif
    } while (keystroke != RETURN_KEY && keystroke != ESCAPE_KEY);

    if (useDialogBox) {
        restoreDisplayBuffer(&rbuf);
    }

    inputText[charNum] = '\0';

    if (keystroke == ESCAPE_KEY) {
        return false;
    }
    strcat(displayedMessage[0], inputText);
    strcat(displayedMessage[0], suffix);
    return true;
}

void displayCenteredAlert(char *message) {
    printString(message, (COLS - strLenWithoutEscapes(message)) / 2, ROWS / 2, &teal, &black, 0);
}

// Flashes a message on the screen starting at (x, y) lasting for the given time (in ms) and with the given colors.
void flashMessage(char *message, short x, short y, int time, const color *fColor, const color *bColor) {
    boolean fastForward;
    int     i, j, messageLength, percentComplete, previousPercentComplete;
    color backColors[COLS], backColor, foreColor;
    cellDisplayBuffer dbufs[COLS];
    enum displayGlyph dchar;
    short oldRNG;
    const int stepInMs = 16;

    if (rogue.playbackFastForward) {
        return;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    messageLength = strLenWithoutEscapes(message);
    fastForward = false;

    for (j=0; j<messageLength; j++) {
        backColors[j] = colorFromComponents(displayBuffer.cells[j + x][y].backColorComponents);
        dbufs[j] = displayBuffer.cells[j + x][y];
    }

    previousPercentComplete = -1;
    for (i=0; i < time && fastForward == false; i += stepInMs) {
        percentComplete = 100 * i / time;
        percentComplete = percentComplete * percentComplete / 100; // transition is front-loaded
        if (previousPercentComplete != percentComplete) {
            for (j=0; j<messageLength; j++) {
                if (i==0) {
                    backColors[j] = colorFromComponents(displayBuffer.cells[j + x][y].backColorComponents);
                    dbufs[j] = displayBuffer.cells[j + x][y];
                }
                backColor = backColors[j];
                applyColorAverage(&backColor, bColor, 100 - percentComplete);
                if (percentComplete < 50) {
                    dchar = message[j];
                    foreColor = *fColor;
                    applyColorAverage(&foreColor, &backColor, percentComplete * 2);
                } else {
                    dchar = dbufs[j].character;
                    foreColor = colorFromComponents(dbufs[j].foreColorComponents);
                    applyColorAverage(&foreColor, &backColor, (100 - percentComplete) * 2);
                }
                plotCharWithColor(dchar, (windowpos){ j+x, y }, &foreColor, &backColor);
            }
        }
        previousPercentComplete = percentComplete;
        fastForward = pauseBrogue(stepInMs, PAUSE_BEHAVIOR_DEFAULT);
    }
    for (j=0; j<messageLength; j++) {
        foreColor = colorFromComponents(dbufs[j].foreColorComponents);
        plotCharWithColor(dbufs[j].character, (windowpos){ j+x, y }, &foreColor, &(backColors[j]));
    }

    restoreRNG;
}

void flashTemporaryAlert(char *message, int time) {
    flashMessage(message, (COLS - strLenWithoutEscapes(message)) / 2, ROWS / 2, time, &teal, &black);
}

void waitForAcknowledgment() {
    rogueEvent theEvent;

    if (rogue.autoPlayingLevel || (rogue.playbackMode && !rogue.playbackOOS) || nonInteractivePlayback) {
        return;
    }

    do {
        nextBrogueEvent(&theEvent, false, false, false);
        if (theEvent.eventType == KEYSTROKE && theEvent.param1 != ACKNOWLEDGE_KEY && theEvent.param1 != ESCAPE_KEY) {
            flashTemporaryAlert(" -- Press space or click to continue -- ", 500);
        }
    } while (!(theEvent.eventType == KEYSTROKE && (theEvent.param1 == ACKNOWLEDGE_KEY || theEvent.param1 == ESCAPE_KEY)
               || theEvent.eventType == MOUSE_UP));
}

void waitForKeystrokeOrMouseClick() {
    rogueEvent theEvent;
    do {
        nextBrogueEvent(&theEvent, false, false, false);
    } while (theEvent.eventType != KEYSTROKE && theEvent.eventType != MOUSE_UP);
}

boolean confirm(char *prompt, boolean alsoDuringPlayback) {
    short retVal;
    brogueButton buttons[2] = {{{0}}};
    
    char whiteColorEscape[20] = "";
    char yellowColorEscape[20] = "";

    if (rogue.autoPlayingLevel || (!alsoDuringPlayback && rogue.playbackMode)) {
        return true; // oh yes he did
    }

    encodeMessageColor(whiteColorEscape, 0, &white);
    encodeMessageColor(yellowColorEscape, 0, KEYBOARD_LABELS ? &yellow : &white);

    initializeButton(&(buttons[0]));
    sprintf(buttons[0].text, "     %sY%ses     ", yellowColorEscape, whiteColorEscape);
    buttons[0].hotkey[0] = 'y';
    buttons[0].hotkey[1] = 'Y';
    buttons[0].hotkey[2] = RETURN_KEY;
    buttons[0].flags |= (B_WIDE_CLICK_AREA | B_KEYPRESS_HIGHLIGHT);

    initializeButton(&(buttons[1]));
    sprintf(buttons[1].text, "     %sN%so      ", yellowColorEscape, whiteColorEscape);
    buttons[1].hotkey[0] = 'n';
    buttons[1].hotkey[1] = 'N';
    buttons[1].hotkey[2] = ACKNOWLEDGE_KEY;
    buttons[1].hotkey[3] = ESCAPE_KEY;
    buttons[1].flags |= (B_WIDE_CLICK_AREA | B_KEYPRESS_HIGHLIGHT);

    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    retVal = printTextBox(prompt, COLS/3, ROWS/3, COLS/3, &white, &interfaceBoxColor, buttons, 2);
    restoreDisplayBuffer(&rbuf);

    if (retVal == -1 || retVal == 1) { // If they canceled or pressed no.
        return false;
    } else {
        return true;
    }

    confirmMessages();
    return retVal;
}

void displayMonsterFlashes(boolean flashingEnabled) {
    short x[100], y[100], strength[100], count = 0;
    const color *flashColor[100];

    rogue.creaturesWillFlashThisTurn = false;

    if (rogue.autoPlayingLevel || rogue.blockCombatText) {
        return;
    }

    short oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *monst = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        if (monst->bookkeepingFlags & MB_WILL_FLASH) {
            monst->bookkeepingFlags &= ~MB_WILL_FLASH;
            if (flashingEnabled && canSeeMonster(monst) && count < 100) {
                x[count] = monst->loc.x;
                y[count] = monst->loc.y;
                strength[count] = monst->flashStrength;
                flashColor[count] = &(monst->flashColor);
                count++;
            }
        }
    }
    flashForeground(x, y, flashColor, strength, count, 20);
    restoreRNG;
}

static void dequeueEvent() {
    rogueEvent returnEvent;
    nextBrogueEvent(&returnEvent, false, false, true);
}

// Empty the message archive
void clearMessageArchive() {
    messageArchivePosition = 0;
}

// Get a pointer to the archivedMessage the given number of entries back in history.
// Pass zero to get the entry under messageArchivePosition.
static archivedMessage *getArchivedMessage(short back) {
    return &messageArchive[(messageArchivePosition + MESSAGE_ARCHIVE_ENTRIES - back) % MESSAGE_ARCHIVE_ENTRIES];
}

static int formatCountedMessage(char *buffer, size_t size, archivedMessage *m) {
    int length;

    if (m->count <= 1) {
        length = snprintf(buffer, size, "%s", m->message);
    } else if (m->count >= MAX_MESSAGE_REPEATS) {
        length = snprintf(buffer, size, "%s (many)", m->message);
    } else {
        length = snprintf(buffer, size, "%s (x%d)", m->message, m->count);
    }

    return length;
}

// Select and write one or more recent messages to the buffer for further
// formatting.  FOLDABLE messages from the same turn are combined, otherwise
// only one message is taken.
// If turnOutput is not null, it is filled with the player turn number shared
// by the chosen messages.
static short foldMessages(char buffer[COLS*20], short offset, unsigned long *turnOutput) {
    short i, folded, messageLength, lineLength, length;
    unsigned long turn;
    char counted[COLS*2];
    archivedMessage *m;

    folded = 0;
    m = getArchivedMessage(offset + folded + 1);
    if (!m->message[0]) {
        return folded;
    }

    folded++;
    turn = m->turn;
    if (turnOutput) {
        *turnOutput = turn;
    }

    if (!(m->flags & FOLDABLE)) {
        formatCountedMessage(buffer, COLS*2, m);
        return folded;
    }

    // Search back for eligible foldable message.  Only fold messages from the same turn
    m = getArchivedMessage(offset + folded + 1);
    while (folded < MESSAGE_ARCHIVE_ENTRIES && m->message[0] && m->flags & FOLDABLE && turn == m->turn) {
        folded++;
        m = getArchivedMessage(offset + folded + 1);
    }

    lineLength = 0;
    length = 0;
    buffer[length] = '\0';
    for (i = folded; i >= 1; i--) {
        m = getArchivedMessage(offset + i);
        formatCountedMessage(counted, COLS*2, m);
        messageLength = strLenWithoutEscapes(counted);

        if (length == 0) {
            length = snprintf(buffer, COLS*20, "%s", counted);
            lineLength = messageLength;
        } else if (lineLength + 3 + messageLength <= DCOLS) {  // + 3 for semi-colon, space and final period
            length += snprintf(&buffer[length], COLS*20 - length, "; %s", counted);
            lineLength += 2 + messageLength;
        } else {
            length += snprintf(&buffer[length], COLS*20 - length, ".\n%s", counted);
            lineLength = messageLength;
        }
    }

    snprintf(&buffer[length], COLS*20 - length, ".");

    return folded;
}

// Change the given newline-delimited sentences in-place to ensure proper writing.
static void capitalizeAndPunctuateSentences(char *text, short length) {
    short i;
    boolean newSentence;

    newSentence = true;

    for (i = 0; i + 1 < length && text[i] != '\0' && text[i+1] != '\0'; i++) {
        if (text[i] == COLOR_ESCAPE) {
            i += 3; // the loop increment will get the last byte
        } else if (text[i] == '"'
                   && (text[i+1] == '.' || text[i+1] == ',')) {
            // Implement the American quotation mark/period/comma ordering rule.
            text[i] = text[i+1];
            text[i+1] = '"';
        } else if (text[i] == '\n') {
            newSentence = true;
        } else if (newSentence) {
            upperCase(&(text[i]));
            newSentence = false;
        }
    }
}

// Copy \n-delimited lines to the given buffer.  Carry colors across line breaks.
static void splitLines(short lines, char wrapped[COLS*20], char buffer[][COLS*2], short bufferCursor) {
    short linesSeen;
    char color[5], line[COLS*2];
    char *start, *end;

    start = &(wrapped[0]);
    color[0] = '\0';
    line[0] = '\0';

    for (end = start, linesSeen = 0; *end; end++) {
        if (*end == COLOR_ESCAPE) {
            strncpy(color, end, 4);
            color[4] = '\0';
            end += 4;
        } else if (*end == '\n') {
            *end = '\0';

            if (bufferCursor + 1 - lines + linesSeen >= 0) {
                strncpy(line, color, 5);
                strncat(line, start, COLS*2 - strlen(line) - 1);
                line[COLS*2-1] = '\0';
                strncpy(buffer[bufferCursor + 1 - lines + linesSeen], line, COLS*2);
                line[0] = '\0';
            }

            linesSeen++;
            start = end + 1;
        }
    }

    strncpy(line, color, 5);
    strncat(line, start, COLS*2 - strlen(line) - 1);
    line[COLS*2-1] = '\0';
    strncpy(buffer[bufferCursor], line, COLS*2);
}

// Fill the buffer of height lines with archived messages.  Fill from the
// bottom, so that the most recent message appears in the last line of buffer.
// linesFormatted, if not null, is filled with the number of formatted lines
// (rows of buffer filled)
// latestMessageLines, if not null, is filled with the number of formatted
// lines generated by events from the current player turn.
void formatRecentMessages(char buffer[][COLS*2], size_t height, short *linesFormatted, short *latestMessageLines) {
    short lines, bufferCursor, messagesFolded, messagesFormatted;
    unsigned long turn;
    char folded[COLS*20], wrapped[COLS*20];

    bufferCursor = height - 1;
    messagesFormatted = 0;

    if (latestMessageLines) {
        *latestMessageLines = 0;
    }

    while (bufferCursor >= 0 && messagesFormatted < MESSAGE_ARCHIVE_ENTRIES) {
        messagesFolded = foldMessages(folded, messagesFormatted, &turn);
        if (messagesFolded == 0) {
            break;
        }

        capitalizeAndPunctuateSentences(folded, COLS*20);
        lines = wrapText(wrapped, folded, DCOLS);
        splitLines(lines, wrapped, buffer, bufferCursor);

        if (latestMessageLines && turn == rogue.playerTurnNumber) {
            *latestMessageLines += lines;
        }

        bufferCursor -= lines;
        messagesFormatted += messagesFolded;
    }

    if (linesFormatted) {
        *linesFormatted = height - 1 - bufferCursor;
    }

    while (bufferCursor >= 0) {
        buffer[bufferCursor--][0] = '\0';
    }
}

// Display recent archived messages after recalculating message confirmations.
void displayRecentMessages() {
    short i;
    char messageBuffer[MESSAGE_LINES][COLS*2];

    formatRecentMessages(messageBuffer, MESSAGE_LINES, 0, &messagesUnconfirmed);

    for (i = 0; i < MESSAGE_LINES; i++) {
        strcpy(displayedMessage[i], messageBuffer[MESSAGE_LINES - i - 1]);
    }

    updateMessageDisplay();
}

// Draw the given formatted messages to the screen:
// messages: the archived messages after all formatting passes
// length: the number of rows in messages, filled from the "bottom", (unused rows have lower indexes)
// offset: index of oldest (visually highest) message to draw
// height: height in rows of the message archive display area
// rbuf: background display buffer to draw against
static void drawMessageArchive(char messages[MESSAGE_ARCHIVE_LINES][COLS*2], short length, short offset, short height) {
    int i, j, k, fadePercent;
    screenDisplayBuffer dbuf;

    clearDisplayBuffer(&dbuf);

    for (i = 0; (MESSAGE_ARCHIVE_LINES - offset + i) < MESSAGE_ARCHIVE_LINES && i < ROWS && i < height; i++) {
        printString(messages[MESSAGE_ARCHIVE_LINES - offset + i], mapToWindowX(0), i, &white, &black, &dbuf);

        // Set the dbuf opacity, and do a fade from bottom to top to make it clear that the bottom messages are the most recent.
        fadePercent = 50 * (length - offset + i) / length + 50;
        for (j = 0; j < DCOLS; j++) {
            dbuf.cells[mapToWindowX(j)][i].opacity = INTERFACE_OPACITY;
            if (dbuf.cells[mapToWindowX(j)][i].character != ' ') {
                for (k=0; k<3; k++) {
                    dbuf.cells[mapToWindowX(j)][i].foreColorComponents[k] = dbuf.cells[mapToWindowX(j)][i].foreColorComponents[k] * fadePercent / 100;
                }
            }
        }
    }

    
    overlayDisplayBuffer(&dbuf);
}

// Pull-down/pull-up animation.
// open: the desired state of the archived message display.  True when expanding, false when collapsing.
// messages: the archived messages after all formatting passes
// length: the number of rows in messages, filled from the "bottom", (unused rows have lower indexes)
// offset: index of oldest (visually highest) message to draw in the fully expanded state
// height: height in rows of the message archive display area in the fully expanded state
static void animateMessageArchive(boolean opening, char messages[MESSAGE_ARCHIVE_LINES][COLS*2], short length, short offset, short height) {
    short i;
    boolean fastForward;

    fastForward = false;

    for (i = (opening ? MESSAGE_LINES : height);
         (opening ? i <= height : i >= MESSAGE_LINES);
         i += (opening ? 1 : -1)) {

        const SavedDisplayBuffer rbuf = saveDisplayBuffer();

        drawMessageArchive(messages, length, offset - height + i, i);

        if (!fastForward && pauseBrogue(opening ? 2 : 1, PAUSE_BEHAVIOR_DEFAULT)) {
            fastForward = true;
            dequeueEvent();
            i = (opening ? height - 1 : MESSAGE_LINES + 1); // skip to the end
        }
        restoreDisplayBuffer(&rbuf);
    }
}

// Accept keyboard input to navigate or dismiss the opened message archive
// messages: the archived messages after all formatting passes
// length: the number of rows in messages, filled from the "bottom", (unused rows have lower indexes)
// offset: index of oldest (visually highest) message to draw
// height: height in rows of the message archive display area
// rbuf: background display buffer to draw against
//
// returns the new offset, which can change if the player scrolled around before closing
static short scrollMessageArchive(char messages[MESSAGE_ARCHIVE_LINES][COLS*2], short length, short offset, short height) {
    // Set `lastOffset` to a value different from `offset`, so that we always re-render the first time.
    short lastOffset = offset-1;
    boolean exit;
    rogueEvent theEvent;
    signed long keystroke;

    if (rogue.autoPlayingLevel || (rogue.playbackMode && !rogue.playbackOOS)) {
        return offset;
    }

    const SavedDisplayBuffer rbuf = saveDisplayBuffer();

    exit = false;
    do {
        if (offset != lastOffset) {
            restoreDisplayBuffer(&rbuf);
            drawMessageArchive(messages, length, offset, height);
        }
        lastOffset = offset;
        nextBrogueEvent(&theEvent, false, false, false);

        if (theEvent.eventType == KEYSTROKE) {
            keystroke = theEvent.param1;
            stripShiftFromMovementKeystroke(&keystroke);

            switch (keystroke) {
                case UP_KEY:
                case UP_ARROW:
                case NUMPAD_8:
                    if (theEvent.controlKey) {
                        offset = length;
                    } else if (theEvent.shiftKey) {
                        offset++;
                    } else {
                        offset += MESSAGE_ARCHIVE_VIEW_LINES / 3;
                    }
                    break;
                case DOWN_KEY:
                case DOWN_ARROW:
                case NUMPAD_2:
                    if (theEvent.controlKey) {
                        offset = height;
                    } else if (theEvent.shiftKey) {
                        offset--;
                    } else {
                        offset -= MESSAGE_ARCHIVE_VIEW_LINES / 3;
                    }
                    break;
                case ACKNOWLEDGE_KEY:
                case ESCAPE_KEY:
                    exit = true;
                    break;
                default:
                    flashTemporaryAlert(" -- Press space or click to continue -- ", 500);
            }
        }

        if (theEvent.eventType == MOUSE_UP) {
            exit = true;
        }

        offset = max(height, min(offset, length));
    } while (!exit);

    restoreDisplayBuffer(&rbuf);
    return offset;
}

void displayMessageArchive() {
    short length, offset, height;
    char messageBuffer[MESSAGE_ARCHIVE_LINES][COLS*2];

    formatRecentMessages(messageBuffer, MESSAGE_ARCHIVE_LINES, &length, 0);

    if (length <= MESSAGE_LINES) {
        return;
    }

    height = min(length, MESSAGE_ARCHIVE_VIEW_LINES);
    offset = height;


    animateMessageArchive(true, messageBuffer, length, offset, height);
    offset = scrollMessageArchive(messageBuffer, length, offset, height);
    animateMessageArchive(false, messageBuffer, length, offset, height);

    updateFlavorText();
    confirmMessages();
    updateMessageDisplay();
}

// Clears the message area and prints the given message in the area.
// It will disappear when messages are refreshed and will not be archived.
// This is primarily used to display prompts.
void temporaryMessage(const char *msg, unsigned long flags) {
    char message[COLS];
    short i, j;

    assureCosmeticRNG;
    strcpy(message, msg);

    for (i=0; message[i] == COLOR_ESCAPE; i += 4) {
        upperCase(&(message[i]));
    }

    if (flags & REFRESH_SIDEBAR) {
        refreshSideBar(-1, -1, false);
    }

    for (i=0; i<MESSAGE_LINES; i++) {
        for (j=0; j<DCOLS; j++) {
            plotCharWithColor(' ', (windowpos){ mapToWindowX(j), i }, &black, &black);
        }
    }
    printString(message, mapToWindowX(0), mapToWindowY(-1), &white, &black, 0);
    if (flags & REQUIRE_ACKNOWLEDGMENT) {
        waitForAcknowledgment();
        updateMessageDisplay();
    }
    restoreRNG;
}

void messageWithColor(const char *msg, const color *theColor, unsigned long flags) {
    char buf[COLS*2] = "";
    short i;

    i=0;
    i = encodeMessageColor(buf, i, theColor);
    strcpy(&(buf[i]), msg);
    message(buf, flags);
}

void flavorMessage(const char *msg) {
    short i;
    char text[COLS*20];

    for (i=0; i < COLS*2 && msg[i] != '\0'; i++) {
        text[i] = msg[i];
    }
    text[i] = '\0';

    for(i=0; text[i] == COLOR_ESCAPE; i+=4);
    upperCase(&(text[i]));

    printString(text, mapToWindowX(0), ROWS - 2, &flavorTextColor, &black, 0);
    for (i = strLenWithoutEscapes(text); i < DCOLS; i++) {
        plotCharWithColor(' ', (windowpos) { mapToWindowX(i), ROWS - 2 }, &black, &black);
    }
}

// Insert or collapse a new message into the archive and redraw the recent
// message display.  An incoming message may "collapse" into another (be
// dropped in favor of bumping a repetition count) if the two have identical
// content and one of two other conditions is met.  First, if the two messages
// arrived on the same turn, they may collapse.  Alternately, they may collapse
// if the older message is the latest one in the archive and the new one is not
// semi-colon foldable (such as a combat message.)
void message(const char *msg, unsigned long flags) {
    short i;
    archivedMessage *archiveEntry;
    boolean newMessage;

    if (msg == NULL || !msg[0]) {
        return;
    }

    assureCosmeticRNG;

    rogue.disturbed = true;
    if (flags & REQUIRE_ACKNOWLEDGMENT || flags & REFRESH_SIDEBAR) {
        refreshSideBar(-1, -1, false);
    }
    displayCombatText();

    // Add the message to the archive, bumping counts for recent duplicates
    newMessage = true;

    // For each at most MESSAGE_ARCHIVE_ENTRIES - 1 past entries..
    for (i = 1; i < MESSAGE_ARCHIVE_ENTRIES; i++) {
        archiveEntry = getArchivedMessage(i);

        // Consider messages that arrived this turn for collapsing.  Also
        // consider the latest entry (which may be from past turns) if the
        // incoming message is not semi-colon foldable.
        if (!((i == 1 && !(flags & FOLDABLE)) || archiveEntry->turn == rogue.playerTurnNumber)) {
            break;
        }

        if (strcmp(archiveEntry->message, msg) == 0) {
            // We found an suitable older message to collapse into.  So we
            // don't need to add another.  Instead consider the older message
            // as having happened on the current turn, and bump its count if
            // not maxxed out.
            newMessage = false;
            archiveEntry->turn = rogue.playerTurnNumber;
            if (archiveEntry->count < MAX_MESSAGE_REPEATS) {
                archiveEntry->count++;
            }
            break;
        }
    }

    // We didn't collapse the new message, so initialize and insert a new
    // archive entry for it instead.
    if (newMessage) {
        archiveEntry = &messageArchive[messageArchivePosition];
        strcpy(archiveEntry->message, msg);
        archiveEntry->count = 1;
        archiveEntry->turn = rogue.playerTurnNumber;
        archiveEntry->flags = flags;
        messageArchivePosition = (messageArchivePosition + 1) % MESSAGE_ARCHIVE_ENTRIES;
    }

    displayRecentMessages();

    if ((flags & REQUIRE_ACKNOWLEDGMENT) || rogue.cautiousMode) {
        displayMoreSign();
        confirmMessages();
        rogue.cautiousMode = false;
    }

    if (rogue.playbackMode) {
        rogue.playbackDelayThisTurn += min(2000, rogue.playbackDelayPerTurn * 5);
    }

    restoreRNG;
}

// Only used for the "you die..." message, to enable posthumous inventory viewing.
void displayMoreSignWithoutWaitingForAcknowledgment() {
    if (strLenWithoutEscapes(displayedMessage[0]) < DCOLS - 8 || messagesUnconfirmed > 0) {
        printString("--MORE--", COLS - 8, MESSAGE_LINES-1, &black, &white, 0);
    } else {
        printString("--MORE--", COLS - 8, MESSAGE_LINES, &black, &white, 0);
    }
}

void displayMoreSign() {
    short i;

    if (rogue.autoPlayingLevel) {
        return;
    }

    if (strLenWithoutEscapes(displayedMessage[0]) < DCOLS - 8 || messagesUnconfirmed > 0) {
        printString("--MORE--", COLS - 8, MESSAGE_LINES-1, &black, &white, 0);
        waitForAcknowledgment();
        printString("        ", COLS - 8, MESSAGE_LINES-1, &black, &black, 0);
    } else {
        printString("--MORE--", COLS - 8, MESSAGE_LINES, &black, &white, 0);
        waitForAcknowledgment();
        for (i=1; i<=8; i++) {
            refreshDungeonCell((pos){ DCOLS - i, 0 });
        }
    }
}

// Inserts a four-character color escape sequence into a string at the insertion point.
// Does NOT check string lengths, so it could theoretically write over the null terminator.
// Returns the new insertion point.
short encodeMessageColor(char *msg, short i, const color *theColor) {
    boolean needTerminator = false;
    color col = *theColor;

    assureCosmeticRNG;

    bakeColor(&col);

    col.red     = clamp(col.red, 0, 100);
    col.green   = clamp(col.green, 0, 100);
    col.blue    = clamp(col.blue, 0, 100);

    needTerminator = !msg[i] || !msg[i + 1] || !msg[i + 2] || !msg[i + 3];

    msg[i++] = COLOR_ESCAPE;
    msg[i++] = (char) (COLOR_VALUE_INTERCEPT + col.red);
    msg[i++] = (char) (COLOR_VALUE_INTERCEPT + col.green);
    msg[i++] = (char) (COLOR_VALUE_INTERCEPT + col.blue);

    if (needTerminator) {
        msg[i] = '\0';
    }

    restoreRNG;

    return i;
}

// Call this when the i'th character of msg is COLOR_ESCAPE.
// It will return the encoded color, and will advance i past the color escape sequence.
short decodeMessageColor(const char *msg, short i, color *returnColor) {

    if (msg[i] != COLOR_ESCAPE) {
        printf("\nAsked to decode a color escape that didn't exist!");
        *returnColor = white;
    } else {
        i++;
        *returnColor = black;
        returnColor->red    = (short) (msg[i++] - COLOR_VALUE_INTERCEPT);
        returnColor->green  = (short) (msg[i++] - COLOR_VALUE_INTERCEPT);
        returnColor->blue   = (short) (msg[i++] - COLOR_VALUE_INTERCEPT);

        returnColor->red    = clamp(returnColor->red, 0, 100);
        returnColor->green  = clamp(returnColor->green, 0, 100);
        returnColor->blue   = clamp(returnColor->blue, 0, 100);
    }
    return i;
}

// Returns a color for combat text based on the identity of the victim.
const color *messageColorFromVictim(creature *monst) {
    if (monst == &player) {
        return &badMessageColor;
    } else if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
        return &white;
    } else if (monst->creatureState == MONSTER_ALLY) {
        return &badMessageColor;
    } else if (monstersAreEnemies(&player, monst)) {
        return &goodMessageColor;
    } else {
        return &white;
    }
}

void updateMessageDisplay() {
    short i, j, m;
    color messageColor;

    for (i=0; i<MESSAGE_LINES; i++) {
        messageColor = white;

        if (i >= messagesUnconfirmed) {
            applyColorAverage(&messageColor, &black, 50);
            applyColorAverage(&messageColor, &black, 75 * i / MESSAGE_LINES);
        }

        for (j = m = 0; displayedMessage[i][m] && j < DCOLS; j++, m++) {

            while (displayedMessage[i][m] == COLOR_ESCAPE) {
                m = decodeMessageColor(displayedMessage[i], m, &messageColor); // pulls the message color out and advances m
                if (i >= messagesUnconfirmed) {
                    applyColorAverage(&messageColor, &black, 50);
                    applyColorAverage(&messageColor, &black, 75 * i / MESSAGE_LINES);
                }
            }

            plotCharWithColor(displayedMessage[i][m], (windowpos){ mapToWindowX(j), MESSAGE_LINES - i - 1 },
                              &messageColor,
                              &black);
        }
        for (; j < DCOLS; j++) {
            plotCharWithColor(' ', (windowpos){ mapToWindowX(j), MESSAGE_LINES - i - 1 }, &black, &black);
        }
    }
}

// Does NOT clear the message archive.
void deleteMessages() {
    short i;
    for (i=0; i<MESSAGE_LINES; i++) {
        displayedMessage[i][0] = '\0';
    }
    confirmMessages();
}

void confirmMessages() {
    messagesUnconfirmed = 0;
    updateMessageDisplay();
}

void stripShiftFromMovementKeystroke(signed long *keystroke) {
    const unsigned short newKey = *keystroke - ('A' - 'a');
    if (newKey == LEFT_KEY
        || newKey == RIGHT_KEY
        || newKey == DOWN_KEY
        || newKey == UP_KEY
        || newKey == UPLEFT_KEY
        || newKey == UPRIGHT_KEY
        || newKey == DOWNLEFT_KEY
        || newKey == DOWNRIGHT_KEY) {
        *keystroke -= 'A' - 'a';
    }
}

void upperCase(char *theChar) {
    if (*theChar >= 'a' && *theChar <= 'z') {
        (*theChar) += ('A' - 'a');
    }
}

enum entityDisplayTypes {
    EDT_NOTHING = 0,
    EDT_CREATURE,
    EDT_ITEM,
    EDT_TERRAIN,
};

// Refreshes the sidebar.
// Progresses from the closest visible monster to the farthest.
// If a monster, item or terrain is focused, then display the sidebar with that monster/item highlighted,
// in the order it would normally appear. If it would normally not fit on the sidebar at all,
// then list it first.
// Also update `pos rogue.sidebarLocationList[ROWS]` list of locations so that each row of
// the screen is mapped to the corresponding entity, if any.
// FocusedEntityMustGoFirst should usually be false when called externally. This is because
// we won't know if it will fit on the screen in normal order until we try.
// So if we try and fail, this function will call itself again, but with this set to true.
void refreshSideBar(short focusX, short focusY, boolean focusedEntityMustGoFirst) {
    short printY, oldPrintY, shortestDistance, i, j, k, px, py, x = 0, y = 0, displayEntityCount, indirectVision;
    creature *closestMonst = NULL;
    item *theItem, *closestItem = NULL;
    char buf[COLS];
    const void *entityList[ROWS] = {0}, *focusEntity = NULL;
    enum entityDisplayTypes entityType[ROWS] = {0}, focusEntityType = EDT_NOTHING;
    pos terrainLocationMap[ROWS];
    boolean gotFocusedEntityOnScreen = (focusX >= 0 ? false : true);
    char addedEntity[DCOLS][DROWS];
    short oldRNG;

    if (rogue.gameHasEnded || rogue.playbackFastForward) {
        return;
    }

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    if (focusX < 0) {
        focusedEntityMustGoFirst = false; // just in case!
    } else {
        if (pmap[focusX][focusY].flags & (HAS_MONSTER | HAS_PLAYER)) {
            creature *monst = monsterAtLoc((pos){ focusX, focusY });
            if (canSeeMonster(monst) || rogue.playbackOmniscience) {
                focusEntity = monst;
                focusEntityType = EDT_CREATURE;
            }
        }
        if (!focusEntity && (pmap[focusX][focusY].flags & HAS_ITEM)) {
            theItem = itemAtLoc((pos){ focusX, focusY });
            if (playerCanSeeOrSense(focusX, focusY)) {
                focusEntity = theItem;
                focusEntityType = EDT_ITEM;
            }
        }
        if (!focusEntity
            && cellHasTMFlag((pos){ focusX, focusY }, TM_LIST_IN_SIDEBAR)
            && playerCanSeeOrSense(focusX, focusY)) {

            focusEntity = tileCatalog[pmap[focusX][focusY].layers[layerWithTMFlag(focusX, focusY, TM_LIST_IN_SIDEBAR)]].description;
            focusEntityType = EDT_TERRAIN;
        }
    }

    printY = 0;

    px = player.loc.x;
    py = player.loc.y;

    zeroOutGrid(addedEntity);

    // Header information for playback mode.
    if (rogue.playbackMode) {
        printString("   -- PLAYBACK --   ", 0, printY++, &white, &black, 0);
        if (rogue.howManyTurns > 0) {
            sprintf(buf, "Turn %li/%li", rogue.playerTurnNumber, rogue.howManyTurns);
            printProgressBar(0, printY++, buf, rogue.playerTurnNumber, rogue.howManyTurns, &darkPurple, false);
        }
        if (rogue.playbackOOS) {
            printString("    [OUT OF SYNC]   ", 0, printY++, &badMessageColor, &black, 0);
        } else if (rogue.playbackPaused) {
            printString("      [PAUSED]      ", 0, printY++, &gray, &black, 0);
        }
        printString("                    ", 0, printY++, &white, &black, 0);
    }

    // Now list the monsters that we'll be displaying in the order of their proximity to player (listing the focused first if required).

    // Initialization.
    displayEntityCount = 0;
    for (i=0; i<ROWS*2; i++) {
        rogue.sidebarLocationList[i] = INVALID_POS;
    }

    // Player always goes first.
    entityList[displayEntityCount] = &player;
    entityType[displayEntityCount] = EDT_CREATURE;
    displayEntityCount++;
    addedEntity[player.loc.x][player.loc.y] = true;
    // And the item at the player's location, if any
    theItem = itemAtLoc(player.loc);
    if (theItem) {
        entityList[displayEntityCount] = theItem;
        entityType[displayEntityCount] = EDT_ITEM;
        displayEntityCount++;
    }

    // Focused entity, if it must go first.
    if (focusedEntityMustGoFirst && !addedEntity[focusX][focusY]) {
        addedEntity[focusX][focusY] = true;
        entityList[displayEntityCount] = focusEntity;
        entityType[displayEntityCount] = focusEntityType;
        terrainLocationMap[displayEntityCount] = (pos) { focusX, focusY };
        displayEntityCount++;
    }

    for (indirectVision = 0; indirectVision < 2; indirectVision++) {
        // Non-focused monsters.
        do {
            shortestDistance = 10000;
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if ((canDirectlySeeMonster(monst) || (indirectVision && (canSeeMonster(monst) || rogue.playbackOmniscience)))
                    && !addedEntity[monst->loc.x][monst->loc.y]
                    && !(monst->info.flags & MONST_NOT_LISTED_IN_SIDEBAR)
                    && (px - monst->loc.x) * (px - monst->loc.x) + (py - monst->loc.y) * (py - monst->loc.y) < shortestDistance) {

                    shortestDistance = (px - monst->loc.x) * (px - monst->loc.x) + (py - monst->loc.y) * (py - monst->loc.y);
                    closestMonst = monst;
                }
            }
            if (shortestDistance < 10000) {
                addedEntity[closestMonst->loc.x][closestMonst->loc.y] = true;
                entityList[displayEntityCount] = closestMonst;
                entityType[displayEntityCount] = EDT_CREATURE;
                displayEntityCount++;
            }
        } while (shortestDistance < 10000 && displayEntityCount * 2 < ROWS); // Because each entity takes at least 2 rows in the sidebar.

        // Non-focused items.
        do {
            shortestDistance = 10000;
            for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                if ((playerCanDirectlySee(theItem->loc.x, theItem->loc.y) || (indirectVision && (playerCanSeeOrSense(theItem->loc.x, theItem->loc.y) || rogue.playbackOmniscience)))
                    && !addedEntity[theItem->loc.x][theItem->loc.y]
                    && (px - theItem->loc.x) * (px - theItem->loc.x) + (py - theItem->loc.y) * (py - theItem->loc.y) < shortestDistance) {

                    shortestDistance = (px - theItem->loc.x) * (px - theItem->loc.x) + (py - theItem->loc.y) * (py - theItem->loc.y);
                    closestItem = theItem;
                }
            }
            if (shortestDistance < 10000) {
                addedEntity[closestItem->loc.x][closestItem->loc.y] = true;
                entityList[displayEntityCount] = closestItem;
                entityType[displayEntityCount] = EDT_ITEM;
                displayEntityCount++;
            }
        } while (shortestDistance < 10000 && displayEntityCount * 2 < ROWS); // Because each entity takes at least 2 rows in the sidebar.

        // Non-focused terrain.

        // count up the number of candidate locations
        for (k=0; k<max(DROWS, DCOLS); k++) {
            for (i = px-k; i <= px+k; i++) {
                // we are scanning concentric squares. The first and last columns
                // need to be stepped through, but others can be jumped over.
                short step = (i == px-k || i == px+k) ? 1 : 2*k;
                for (j = py-k; j <= py+k; j += step) {
                    if (displayEntityCount >= ROWS - 1) goto no_space_for_more_entities;
                    if (coordinatesAreInMap(i, j)
                        && !addedEntity[i][j]
                        && cellHasTMFlag((pos){ i, j }, TM_LIST_IN_SIDEBAR)
                        && (playerCanDirectlySee(i, j) || (indirectVision && (playerCanSeeOrSense(i, j) || rogue.playbackOmniscience)))) {

                        addedEntity[i][j] = true;
                        entityList[displayEntityCount] = tileCatalog[pmap[i][j].layers[layerWithTMFlag(i, j, TM_LIST_IN_SIDEBAR)]].description;
                        entityType[displayEntityCount] = EDT_TERRAIN;
                        terrainLocationMap[displayEntityCount] = (pos) { i, j };
                        displayEntityCount++;
                    }
                }
            }
        }
        no_space_for_more_entities:;
    }

    // Entities are now listed. Start printing.

    for (i=0; i<displayEntityCount && printY < ROWS - 1; i++) { // Bottom line is reserved for the depth.
        oldPrintY = printY;
        if (entityType[i] == EDT_CREATURE) {
            x = ((creature *) entityList[i])->loc.x;
            y = ((creature *) entityList[i])->loc.y;
            printY = printMonsterInfo((creature *) entityList[i],
                                      printY,
                                      (focusEntity && (x != focusX || y != focusY)),
                                      (x == focusX && y == focusY));

        } else if (entityType[i] == EDT_ITEM) {
            x = ((item *) entityList[i])->loc.x;
            y = ((item *) entityList[i])->loc.y;
            printY = printItemInfo((item *) entityList[i],
                                   printY,
                                   (focusEntity && (x != focusX || y != focusY)),
                                   (x == focusX && y == focusY));
        } else if (entityType[i] == EDT_TERRAIN) {
            x = terrainLocationMap[i].x;
            y = terrainLocationMap[i].y;
            printY = printTerrainInfo(x, y,
                                      printY,
                                      ((const char *) entityList[i]),
                                      (focusEntity && (x != focusX || y != focusY)),
                                      (x == focusX && y == focusY));
        }
        if (focusEntity && (x == focusX && y == focusY) && printY < ROWS) {
            gotFocusedEntityOnScreen = true;
        }
        for (j=oldPrintY; j<printY; j++) {
            rogue.sidebarLocationList[j] = (pos){ x, y };
        }
    }

    if (gotFocusedEntityOnScreen) {
        // Wrap things up.
        for (i=printY; i< ROWS - 1; i++) {
            printString("                    ", 0, i, &white, &black, 0);
        }
        sprintf(buf, "  -- Depth: %i --%s   ", rogue.depthLevel, (rogue.depthLevel < 10 ? " " : ""));
        printString(buf, 0, ROWS - 1, &white, &black, 0);
    } else if (!focusedEntityMustGoFirst) {
        // Failed to get the focusMonst printed on the screen. Try again, this time with the focus first.
        refreshSideBar(focusX, focusY, true);
    }

    restoreRNG;
}

void printString(const char *theString, short x, short y, const color *foreColor, const color *backColor, screenDisplayBuffer *dbuf) {
    short i;

    color fColor = *foreColor;

    for (i=0; theString[i] != '\0' && x < COLS; i++, x++) {
        while (theString[i] == COLOR_ESCAPE) {
            i = decodeMessageColor(theString, i, &fColor);
            if (!theString[i]) {
                return;
            }
        }

        plotCharToBuffer(theString[i], (windowpos){ x, y }, &fColor, backColor, dbuf);
    }
}

// Inserts line breaks into really long words. Optionally adds a hyphen, but doesn't do anything
// clever regarding hyphen placement. Plays nicely with color escapes.
static void breakUpLongWordsIn(char *sourceText, short width, boolean useHyphens) {
    char buf[TEXT_MAX_LENGTH] = "";
    short i, m, nextChar, wordWidth;
    //const short maxLength = useHyphens ? width - 1 : width;

    // i iterates over characters in sourceText; m keeps track of the length of buf.
    wordWidth = 0;
    for (i=0, m=0; sourceText[i] != 0;) {
        if (sourceText[i] == COLOR_ESCAPE) {
            strncpy(&(buf[m]), &(sourceText[i]), 4);
            i += 4;
            m += 4;
        } else if (sourceText[i] == ' ' || sourceText[i] == '\n') {
            wordWidth = 0;
            buf[m++] = sourceText[i++];
        } else {
            if (!useHyphens && wordWidth >= width) {
                buf[m++] = '\n';
                wordWidth = 0;
            } else if (useHyphens && wordWidth >= width - 1) {
                nextChar = i+1;
                while (sourceText[nextChar] == COLOR_ESCAPE) {
                    nextChar += 4;
                }
                if (sourceText[nextChar] && sourceText[nextChar] != ' ' && sourceText[nextChar] != '\n') {
                    buf[m++] = '-';
                    buf[m++] = '\n';
                    wordWidth = 0;
                }
            }
            buf[m++] = sourceText[i++];
            wordWidth++;
        }
    }
    buf[m] = '\0';
    strcpy(sourceText, buf);
}

// Returns the number of lines, including the newlines already in the text.
// Puts the output in "to" only if we receive a "to" -- can make it null and just get a line count.
short wrapText(char *to, const char *sourceText, short width) {
    short i, w, textLength, lineCount;
    char printString[TEXT_MAX_LENGTH];
    short spaceLeftOnLine, wordWidth;

    strcpy(printString, sourceText); // a copy we can write on
    breakUpLongWordsIn(printString, width, true); // break up any words that are wider than the width.

    textLength = strlen(printString); // do NOT discount escape sequences
    lineCount = 1;

    // Now go through and replace spaces with newlines as needed.

    // Fast foward until i points to the first character that is not a color escape.
    for (i=0; printString[i] == COLOR_ESCAPE; i+= 4);
    spaceLeftOnLine = width;

    while (i < textLength) {
        // wordWidth counts the word width of the next word without color escapes.
        // w indicates the position of the space or newline or null terminator that terminates the word.
        wordWidth = 0;
        for (w = i + 1; w < textLength && printString[w] != ' ' && printString[w] != '\n';) {
            if (printString[w] == COLOR_ESCAPE) {
                w += 4;
            } else {
                w++;
                wordWidth++;
            }
        }

        if (1 + wordWidth > spaceLeftOnLine || printString[i] == '\n') {
            printString[i] = '\n';
            lineCount++;
            spaceLeftOnLine = width - wordWidth; // line width minus the width of the word we just wrapped
            //printf("\n\n%s", printString);
        } else {
            spaceLeftOnLine -= 1 + wordWidth;
        }
        i = w; // Advance to the terminator that follows the word.
    }
    if (to) {
        strcpy(to, printString);
    }
    return lineCount;
}

// returns the y-coordinate of the last line
short printStringWithWrapping(const char *theString, short x, short y, short width, const color *foreColor,
                              const color *backColor, screenDisplayBuffer *dbuf) {
    color fColor;
    char printString[TEXT_MAX_LENGTH];
    short i, px, py;

    wrapText(printString, theString, width); // inserts newlines as necessary

    // display the string
    px = x; //px and py are the print insertion coordinates; x and y remain the top-left of the text box
    py = y;
    fColor = *foreColor;

    for (i=0; printString[i] != '\0'; i++) {
        if (printString[i] == '\n') {
            px = x; // back to the leftmost column
            if (py < ROWS - 1) { // don't advance below the bottom of the screen
                py++; // next line
            } else {
                break; // If we've run out of room, stop.
            }
            continue;
        } else if (printString[i] == COLOR_ESCAPE) {
            i = decodeMessageColor(printString, i, &fColor) - 1;
            continue;
        }

        if (locIsInWindow((windowpos){ px, py })) {
            plotCharToBuffer(printString[i], (windowpos){ px, py }, &fColor, backColor, dbuf);
        }

        px++;
    }
    return py;
}

char nextKeyPress(boolean textInput) {
    rogueEvent theEvent;
    do {
        nextBrogueEvent(&theEvent, textInput, false, false);
    } while (theEvent.eventType != KEYSTROKE);
    return theEvent.param1;
}

#define BROGUE_HELP_LINE_COUNT  33

void printHelpScreen() {
    short i, j;
    char helpText[BROGUE_HELP_LINE_COUNT][DCOLS*3] = {
        "",
        "",
        "          -- Commands --",
        "",
        "          mouse  ****move cursor (including to examine monsters and terrain)",
        "          click  ****travel",
        "  control-click  ****advance one space",
        "       <return>  ****enable keyboard cursor control",
        "    <space/esc>  ****disable keyboard cursor control",
        "hjklyubn, arrow keys, or numpad  ****move or attack (control or shift to run)",
        "",
        "a/e/r/t/d/c/R/w  ****apply/equip/remove/throw/drop/call/relabel/swap an item",
        "              T  ****re-throw last item at last monster",
        " i, right-click  ****view inventory",
        "              D  ****list discovered items",
        "",
        "              z  ****rest once",
        "              Z  ****rest for 100 turns or until something happens",
        "              s  ****search for secrets (control-s: long search)",
        "           <, >  ****travel to stairs",
        "              x  ****auto-explore (control-x: fast forward)",
        "              A  ****autopilot (control-A: fast forward)",
        "              M  ****display old messages",
        "              G  ****toggle graphical tiles (when available)",
        "",
        "              S  ****save and exit",
        "              Q  ****quit and abandon game",
        "",
        "              \\  ****disable/enable color effects",
        "              ]  ****display/hide stealth range",
        "    <space/esc>  ****clear message or cancel command",
        "",
        "        -- press space or click to continue --"
    };

    // Replace the "****"s with color escapes.
    for (i=0; i<BROGUE_HELP_LINE_COUNT; i++) {
        for (j=0; helpText[i][j]; j++) {
            if (helpText[i][j] == '*') {
                j = encodeMessageColor(helpText[i], j, &white);
            }
        }
    }

    const SavedDisplayBuffer rbuf = saveDisplayBuffer();

    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);

    // Print the text to the dbuf.
    for (i=0; i<BROGUE_HELP_LINE_COUNT && i < ROWS; i++) {
        printString(helpText[i], mapToWindowX(1), i, &itemMessageColor, &black, &dbuf);
    }

    // Set the dbuf opacity.
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<ROWS; j++) {
            //plotCharWithColor(' ', (windowpos) { mapToWindowX(i), j }, &black, &black);
            dbuf.cells[mapToWindowX(i)][j].opacity = INTERFACE_OPACITY;
        }
    }

    // Display.
    overlayDisplayBuffer(&dbuf);
    waitForAcknowledgment();
    restoreDisplayBuffer(&rbuf);
    updateFlavorText();
    updateMessageDisplay();
}

static void printDiscoveries(short category, short count, unsigned short itemCharacter, short x, short y, screenDisplayBuffer *dbuf) {
    color goodColor, badColor;
    const color *theColor;
    char buf[COLS], buf2[COLS];
    short i, magic, totalFrequency;
    itemTable *theTable = tableForItemCategory(category);

    goodColor = goodMessageColor;
    applyColorAverage(&goodColor, &black, 50);
    badColor = badMessageColor;
    applyColorAverage(&badColor, &black, 50);

    totalFrequency = 0;
    for (i = 0; i < count; i++) {
        if (!theTable[i].identified) {
            totalFrequency += theTable[i].frequency;
        }
    }

    for (i = 0; i < count; i++) {
        if (theTable[i].identified) {
            theColor = &white;
            plotCharToBuffer(itemCharacter, (windowpos){ x, y + i }, &itemColor, &black, dbuf);
        } else {
            theColor = &darkGray;
            magic = magicCharDiscoverySuffix(category, i);
            if (magic == 1) {
                plotCharToBuffer(G_GOOD_MAGIC, (windowpos){ x, y + i }, &goodColor, &black, dbuf);
            } else if (magic == -1) {
                plotCharToBuffer(G_BAD_MAGIC, (windowpos){ x, y + i }, &badColor, &black, dbuf);
            }
        }
        strcpy(buf, theTable[i].name);

        if (!theTable[i].identified
            && theTable[i].frequency > 0
            && totalFrequency > 0) {

            sprintf(buf2, " (%i%%)", theTable[i].frequency * 100 / totalFrequency);
            strcat(buf, buf2);
        }

        upperCase(buf);
        strcat(buf, " ");
        printString(buf, x + 2, y + i, theColor, &black, dbuf);
    }
}

/// @brief Display the feats screen. Lists all feats and their achievement status.
void displayFeatsScreen() {
    char availableColorEscape[5] = "", achievedColorEscape[5] = "", failedColorEscape[5] = "";
    encodeMessageColor(availableColorEscape, 0, &white);
    encodeMessageColor(achievedColorEscape, 0, &advancementMessageColor);
    encodeMessageColor(failedColorEscape, 0, &badMessageColor);

    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);

    // Title
    char buf[COLS*2] = "-- FEATS --";
    short y = 1;
    printString(buf, mapToWindowX((DCOLS - FEAT_NAME_LENGTH - strLenWithoutEscapes(buf)) / 2), y, &flavorTextColor, &black, &dbuf);

    // List of feats, color-coded by status
    char featColorEscape[5] = "", featStatusChar[2];
    for (int i = 0; i < gameConst->numberFeats; i++) {
        if (rogue.featRecord[i] == featTable[i].initialValue) {
            strcpy(featColorEscape, availableColorEscape);
            strcpy(featStatusChar," ");
        } else if (rogue.featRecord[i]) {
            strcpy(featColorEscape, achievedColorEscape);
            strcpy(featStatusChar,"+");
        } else {
            strcpy(featColorEscape, failedColorEscape);
            strcpy(featStatusChar,"-");
        }
        sprintf(buf, "%*s %s%s %s", FEAT_NAME_LENGTH, featTable[i].name, featColorEscape, featStatusChar, featTable[i].description);
        printString(buf, mapToWindowX(0), y + i + 1, &itemMessageColor, &black, &dbuf);
    }
    
    // Legend
    strcpy(buf,"-- LEGEND --");
    printString(buf, mapToWindowX((DCOLS - FEAT_NAME_LENGTH - strLenWithoutEscapes(buf)) / 2), ROWS-5, &gray, &black, &dbuf);
    sprintf(buf, "%sFailed(-)  %sAchieved(+)  ", failedColorEscape, achievedColorEscape);
    printString(buf, mapToWindowX((DCOLS - FEAT_NAME_LENGTH - strLenWithoutEscapes(buf)) / 2), ROWS-4, &white, &black, &dbuf);

    strcpy(buf,KEYBOARD_LABELS ? "-- press any key to continue --" : "-- touch anywhere to continue --");
    printString(buf, mapToWindowX((DCOLS - FEAT_NAME_LENGTH - strLenWithoutEscapes(buf)) / 2), ROWS-2, &itemMessageColor, &black, &dbuf);

    // Set the opacity
    for (int i=0; i<COLS; i++) {
        for (int j=0; j<ROWS; j++) {
            dbuf.cells[i][j].opacity = (i < STAT_BAR_WIDTH ? 0 : INTERFACE_OPACITY);
        }
    }
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    overlayDisplayBuffer(&dbuf);
    waitForKeystrokeOrMouseClick();
    restoreDisplayBuffer(&rbuf);
}

void printDiscoveriesScreen() {
    short i, j, y;
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();

    screenDisplayBuffer dbuf;

    clearDisplayBuffer(&dbuf);

    printString("-- SCROLLS --", mapToWindowX(2), y = mapToWindowY(1), &flavorTextColor, &black, &dbuf);
    printDiscoveries(SCROLL, gameConst->numberScrollKinds, G_SCROLL, mapToWindowX(3), ++y, &dbuf);

    printString("-- RINGS --", mapToWindowX(2), y += gameConst->numberScrollKinds + 1, &flavorTextColor, &black, &dbuf);
    printDiscoveries(RING, NUMBER_RING_KINDS, G_RING, mapToWindowX(3), ++y, &dbuf);

    printString("-- POTIONS --", mapToWindowX(29), y = mapToWindowY(1), &flavorTextColor, &black, &dbuf);
    printDiscoveries(POTION, gameConst->numberPotionKinds, G_POTION, mapToWindowX(30), ++y, &dbuf);

    printString("-- STAFFS --", mapToWindowX(53), y = mapToWindowY(1), &flavorTextColor, &black, &dbuf);
    printDiscoveries(STAFF, NUMBER_STAFF_KINDS, G_STAFF, mapToWindowX(54), ++y, &dbuf);

    printString("-- WANDS --", mapToWindowX(53), y += NUMBER_STAFF_KINDS + 1, &flavorTextColor, &black, &dbuf);
    printDiscoveries(WAND, gameConst->numberWandKinds, G_WAND, mapToWindowX(54), ++y, &dbuf);

    printString(KEYBOARD_LABELS ? "-- press any key to continue --" : "-- touch anywhere to continue --",
                mapToWindowX(20), mapToWindowY(DROWS-2), &itemMessageColor, &black, &dbuf);

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            dbuf.cells[i][j].opacity = (i < STAT_BAR_WIDTH ? 0 : INTERFACE_OPACITY);
        }
    }
    overlayDisplayBuffer(&dbuf);

    waitForKeystrokeOrMouseClick();

    restoreDisplayBuffer(&rbuf);
}

void printHighScores(boolean hiliteMostRecent) {
    short i, hiliteLineNum, maxLength = 0, leftOffset;
    rogueHighScoresEntry list[HIGH_SCORES_COUNT] = {{0}};
    char buf[DCOLS*3];
    color scoreColor;

    hiliteLineNum = getHighScoresList(list);

    if (!hiliteMostRecent) {
        hiliteLineNum = -1;
    }

    blackOutScreen();

    for (i = 0; i < HIGH_SCORES_COUNT && list[i].score > 0; i++) {
        if (strLenWithoutEscapes(list[i].description) > maxLength) {
            maxLength = strLenWithoutEscapes(list[i].description);
        }
    }

    leftOffset = min(COLS - maxLength - 23 - 1, COLS/5);

    scoreColor = black;
    applyColorAverage(&scoreColor, &itemMessageColor, 100);
    printString("-- HIGH SCORES --", (COLS - 17 + 1) / 2, 0, &scoreColor, &black, 0);

    for (i = 0; i < HIGH_SCORES_COUNT && list[i].score > 0; i++) {
        scoreColor = black;
        if (i == hiliteLineNum) {
            applyColorAverage(&scoreColor, &itemMessageColor, 100);
        } else {
            applyColorAverage(&scoreColor, &white, 100);
            applyColorAverage(&scoreColor, &black, (i * 50 / 24));
        }

        // rank
        sprintf(buf, "%s%i)", (i + 1 < 10 ? " " : ""), i + 1);
        printString(buf, leftOffset, i + 2, &scoreColor, &black, 0);

        // score
        sprintf(buf, "%li", list[i].score);
        printString(buf, leftOffset + 5, i + 2, &scoreColor, &black, 0);

        // date
        printString(list[i].date, leftOffset + 12, i + 2, &scoreColor, &black, 0);

        // description
        printString(list[i].description, leftOffset + 23, i + 2, &scoreColor, &black, 0);
    }

    scoreColor = black;
    applyColorAverage(&scoreColor, &goodMessageColor, 100);

    printString(KEYBOARD_LABELS ? "Press space to continue." : "Touch anywhere to continue.",
                (COLS - strLenWithoutEscapes(KEYBOARD_LABELS ? "Press space to continue." : "Touch anywhere to continue.")) / 2,
                ROWS - 1, &scoreColor, &black, 0);

    commitDraws();
    waitForAcknowledgment();
}

void displayGrid(short **map) {
    short i, j, score, topRange, bottomRange;
    color tempColor, foreColor, backColor;
    enum displayGlyph dchar;

    topRange = -30000;
    bottomRange = 30000;
    tempColor = black;

    if (map == safetyMap && !rogue.updatedSafetyMapThisTurn) {
        updateSafetyMap();
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_WAYPOINT_BLOCKER) || (map[i][j] == map[0][0]) || (i == player.loc.x && j == player.loc.y)) {
                continue;
            }
            if (map[i][j] > topRange) {
                topRange = map[i][j];
                //if (topRange == 0) {
                    //printf("\ntop is zero at %i,%i", i, j);
                //}
            }
            if (map[i][j] < bottomRange) {
                bottomRange = map[i][j];
            }
        }
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY | T_LAVA_INSTA_DEATH)
                || (map[i][j] == map[0][0])
                || (i == player.loc.x && j == player.loc.y)) {
                continue;
            }
            score = 300 - (map[i][j] - bottomRange) * 300 / max(1, (topRange - bottomRange));
            tempColor.blue = max(min(score, 100), 0);
            score -= 100;
            tempColor.red = max(min(score, 100), 0);
            score -= 100;
            tempColor.green = max(min(score, 100), 0);
            getCellAppearance((pos){ i, j }, &dchar, &foreColor, &backColor);
            plotCharWithColor(dchar, mapToWindow((pos){ i, j }), &foreColor, &tempColor);
            //colorBlendCell(i, j, &tempColor, 100);//hiliteCell(i, j, &tempColor, 100, false);
        }
    }
    //printf("\ntop: %i; bottom: %i", topRange, bottomRange);
}

/// @brief Display a message with the seed #, turn #, game mode (except normal), and game version
void printSeed() {
    char buf[COLS];
    char mode[14] = "";

    if (rogue.easyMode) {
        strcpy(mode,"easy mode; ");
    } else if (rogue.wizard) {
        strcpy(mode,"wizard mode; ");
    }
    snprintf(buf, COLS, "Dungeon seed #%llu; turn #%lu; %sversion %s", (unsigned long long)rogue.seed, rogue.playerTurnNumber, mode, gameConst->versionString);
    message(buf, 0);
}

void printProgressBar(short x, short y, const char barLabel[COLS], long amtFilled, long amtMax, const color *fillColor, boolean dim) {
    char barText[] = "                    "; // string length is 20
    short i, labelOffset;
    color currentFillColor, textColor, progressBarColor, darkenedBarColor;

    if (y >= ROWS - 1) { // don't write over the depth number
        return;
    }

    if (amtFilled > amtMax) {
        amtFilled = amtMax;
    }

    if (amtMax <= 0) {
        amtMax = 1;
    }

    progressBarColor = *fillColor;
    if (!(y % 2)) {
        applyColorAverage(&progressBarColor, &black, 25);
    }

    if (dim) {
        applyColorAverage(&progressBarColor, &black, 50);
    }
    darkenedBarColor = progressBarColor;
    applyColorAverage(&darkenedBarColor, &black, 75);

    labelOffset = (20 - strlen(barLabel)) / 2;
    for (i = 0; i < (short) strlen(barLabel); i++) {
        barText[i + labelOffset] = barLabel[i];
    }

    amtFilled = clamp(amtFilled, 0, amtMax);

    if (amtMax < 10000000) {
        amtFilled *= 100;
        amtMax *= 100;
    }

    for (i=0; i<20; i++) {
        currentFillColor = (i <= (20 * amtFilled / amtMax) ? progressBarColor : darkenedBarColor);
        if (i == 20 * amtFilled / amtMax) {
            applyColorAverage(&currentFillColor, &black, 75 - 75 * (amtFilled % (amtMax / 20)) / (amtMax / 20));
        }
        textColor = (dim ? gray : white);
        applyColorAverage(&textColor, &currentFillColor, (dim ? 50 : 33));
        plotCharWithColor(barText[i], (windowpos){ x + i, y }, &textColor, &currentFillColor);
    }
}

// Very low-level. Changes displayBuffer directly.
void highlightScreenCell(short x, short y, const color *highlightColor, short strength) {
    color tempColor;

    tempColor = colorFromComponents(displayBuffer.cells[x][y].foreColorComponents);
    applyColorAugment(&tempColor, highlightColor, strength);
    storeColorComponents(displayBuffer.cells[x][y].foreColorComponents, &tempColor);

    tempColor = colorFromComponents(displayBuffer.cells[x][y].backColorComponents);
    applyColorAugment(&tempColor, highlightColor, strength);
    storeColorComponents(displayBuffer.cells[x][y].backColorComponents, &tempColor);
}

// Like `armorValueIfUnenchanted` for the currently-equipped armor, but takes the penalty from
// donning into account.
static short estimatedArmorValue() {
    short retVal = armorValueIfUnenchanted(rogue.armor) - player.status[STATUS_DONNING];
    return max(0, retVal);
}

static short creatureHealthChangePercent(creature *monst) {
    if (monst->previousHealthPoints <= 0) {
        return 0;
    }
    // ignore overhealing from tranference
    return 100 * (monst->currentHP - min(monst->previousHealthPoints, monst->info.maxHP)) / monst->info.maxHP;
}

// returns the y-coordinate after the last line printed
short printMonsterInfo(creature *monst, short y, boolean dim, boolean highlight) {
    char buf[COLS * 2], buf2[COLS * 2], monstName[COLS], tempColorEscape[5], grayColorEscape[5];
    enum displayGlyph monstChar;
    color monstForeColor, monstBackColor, healthBarColor, tempColor;
    short initialY, i, j, highlightStrength, percent;
    boolean inPath;
    short oldRNG;

    const char hallucinationStrings[16][COLS] = {
        "     (Dancing)      ",
        "     (Singing)      ",
        "  (Pontificating)   ",
        "     (Skipping)     ",
        "     (Spinning)     ",
        "      (Crying)      ",
        "     (Laughing)     ",
        "     (Humming)      ",
        "    (Whistling)     ",
        "    (Quivering)     ",
        "    (Muttering)     ",
        "    (Gibbering)     ",
        "     (Giggling)     ",
        "     (Moaning)      ",
        "    (Shrieking)     ",
        "   (Caterwauling)   ",
    };

    if (y >= ROWS - 1) {
        return ROWS - 1;
    }

    initialY = y;

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    if (y < ROWS - 1) {
        printString("                    ", 0, y, &white, &black, 0); // Start with a blank line

        // Unhighlight if it's highlighted as part of the path.
        inPath = (pmapAt(monst->loc)->flags & IS_IN_PATH) ? true : false;
        pmapAt(monst->loc)->flags &= ~IS_IN_PATH;
        getCellAppearance(monst->loc, &monstChar, &monstForeColor, &monstBackColor);
        applyColorBounds(&monstForeColor, 0, 100);
        applyColorBounds(&monstBackColor, 0, 100);
        if (inPath) {
            pmapAt(monst->loc)->flags |= IS_IN_PATH;
        }

        if (dim) {
            applyColorAverage(&monstForeColor, &black, 50);
            applyColorAverage(&monstBackColor, &black, 50);
        } else if (highlight) {
            applyColorAugment(&monstForeColor, &black, 100);
            applyColorAugment(&monstBackColor, &black, 100);
        }
        plotCharWithColor(monstChar, (windowpos){ 0, y }, &monstForeColor, &monstBackColor);
        if(monst->carriedItem) {
            plotCharWithColor(monst->carriedItem->displayChar, (windowpos) { 1, y }, &itemColor, &black);
        }
        monsterName(monstName, monst, false);
        upperCase(monstName);

        if (monst == &player) {
            if (player.status[STATUS_INVISIBLE]) {
                strcat(monstName, " xxxx");
                encodeMessageColor(monstName, strlen(monstName) - 4, &monstForeColor);
                strcat(monstName, "(invisible)");
            } else if (playerInDarkness()) {
                strcat(monstName, " xxxx");
                //encodeMessageColor(monstName, strlen(monstName) - 4, &playerInDarknessColor);
                encodeMessageColor(monstName, strlen(monstName) - 4, &monstForeColor);
                strcat(monstName, "(dark)");
            } else if (!(pmapAt(player.loc)->flags & IS_IN_SHADOW)) {
                strcat(monstName, " xxxx");
                //encodeMessageColor(monstName, strlen(monstName) - 4, &playerInLightColor);
                encodeMessageColor(monstName, strlen(monstName) - 4, &monstForeColor);
                strcat(monstName, "(lit)");
            }
        }

        sprintf(buf, ": %s", monstName);
        printString(buf, monst->carriedItem?2:1, y++, (dim ? &gray : &white), &black, 0);
    }

    // mutation, if any
    if (y < ROWS - 1
        && monst->mutationIndex >= 0
        && (!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience)) {

        strcpy(buf, "                    ");
        sprintf(buf2, "xxxx(%s)", mutationCatalog[monst->mutationIndex].title);
        tempColor = *mutationCatalog[monst->mutationIndex].textColor;
        if (dim) {
            applyColorAverage(&tempColor, &black, 50);
        }
        encodeMessageColor(buf2, 0, &tempColor);
        strcpy(buf + ((strLenWithoutEscapes(buf) - strLenWithoutEscapes(buf2)) / 2), buf2);
        for (i = strlen(buf); i < 20 + 4; i++) {
            buf[i] = ' ';
        }
        buf[24] = '\0';
        printString(buf, 0, y++, (dim ? &gray : &white), &black, 0);
    }

    // hit points
    if (monst->info.maxHP > 1
        && !(monst->info.flags & MONST_INVULNERABLE)) {

        if (monst == &player) {
            healthBarColor = redBar;
            applyColorAverage(&healthBarColor, &blueBar, min(100, 100 * player.currentHP / player.info.maxHP));
        } else {
            healthBarColor = blueBar;
        }
        percent = creatureHealthChangePercent(monst);
        if (monst->currentHP <= 0) {
            strcpy(buf, "Dead");
        } else if (percent != 0) {
            strcpy(buf, "       Health       ");
            sprintf(buf2, "(%s%i%%)", percent > 0 ? "+" : "", percent);
            strcpy(&(buf[20 - strlen(buf2)]), buf2);
        } else {
            strcpy(buf, "Health");
        }
        printProgressBar(0, y++, buf, monst->currentHP, monst->info.maxHP, &healthBarColor, dim);
    }

    if (monst == &player) {
        // nutrition
        if (player.status[STATUS_NUTRITION] > HUNGER_THRESHOLD) {
            printProgressBar(0, y++, "Nutrition", player.status[STATUS_NUTRITION], STOMACH_SIZE, &blueBar, dim);
        } else if (player.status[STATUS_NUTRITION] > WEAK_THRESHOLD) {
            printProgressBar(0, y++, "Nutrition (Hungry)", player.status[STATUS_NUTRITION], STOMACH_SIZE, &blueBar, dim);
        } else if (player.status[STATUS_NUTRITION] > FAINT_THRESHOLD) {
            printProgressBar(0, y++, "Nutrition (Weak)", player.status[STATUS_NUTRITION], STOMACH_SIZE, &blueBar, dim);
        } else if (player.status[STATUS_NUTRITION] > 0) {
            printProgressBar(0, y++, "Nutrition (Faint)", player.status[STATUS_NUTRITION], STOMACH_SIZE, &blueBar, dim);
        } else if (y < ROWS - 1) {
            printString("      STARVING      ", 0, y++, &badMessageColor, &black, NULL);
        }
    }

    if (!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience || monst == &player) {

        for (i=0; i<NUMBER_OF_STATUS_EFFECTS; i++) {
            if (i == STATUS_WEAKENED && monst->status[i] > 0) {
                sprintf(buf, "%s%i", statusEffectCatalog[STATUS_WEAKENED].name, monst->weaknessAmount);
                printProgressBar(0, y++, buf, monst->status[i], monst->maxStatus[i], &redBar, dim);
            } else if (i == STATUS_LEVITATING && monst->status[i] > 0) {
                printProgressBar(0, y++, (monst == &player ? "Levitating" : "Flying"), monst->status[i], monst->maxStatus[i], &redBar, dim);
            } else if (i == STATUS_POISONED
                       && monst->status[i] > 0) {


                if (monst->status[i] * monst->poisonAmount >= monst->currentHP) {
                    strcpy(buf, "Fatal Poison");
                } else {
                    strcpy(buf, "Poisoned");
                }
                if (monst->poisonAmount == 1) {
                    printProgressBar(0, y++, buf, monst->status[i], monst->maxStatus[i], &redBar, dim);
                } else {
                    sprintf(buf2, "%s (x%i)",
                            buf,
                            monst->poisonAmount);
                    printProgressBar(0, y++, buf2, monst->status[i], monst->maxStatus[i], &redBar, dim);
                }
            } else if (statusEffectCatalog[i].name[0] && monst->status[i] > 0) {
                printProgressBar(0, y++, statusEffectCatalog[i].name, monst->status[i], monst->maxStatus[i], &redBar, dim);
            }
        }
        if (posEq(monst->targetCorpseLoc, monst->loc)) {
            printProgressBar(0, y++,  monsterText[monst->info.monsterID].absorbStatus, monst->corpseAbsorptionCounter, 20, &redBar, dim);
        }
    }

    if (monst == &player) {
        if (y < ROWS - 1) {
            tempColorEscape[0] = '\0';
            grayColorEscape[0] = '\0';
            if (player.status[STATUS_WEAKENED]) {
                tempColor = red;
                if (dim) {
                    applyColorAverage(&tempColor, &black, 50);
                }
                encodeMessageColor(tempColorEscape, 0, &tempColor);
                encodeMessageColor(grayColorEscape, 0, (dim ? &darkGray : &gray));
            }

            if (!rogue.armor || rogue.armor->flags & ITEM_IDENTIFIED || rogue.playbackOmniscience) {

                sprintf(buf, "Str: %s%i%s  Armor: %i",
                        tempColorEscape,
                        rogue.strength - player.weaknessAmount,
                        grayColorEscape,
                        displayedArmorValue());
            } else {
                sprintf(buf, "Str: %s%i%s  Armor: %i?",
                        tempColorEscape,
                        rogue.strength - player.weaknessAmount,
                        grayColorEscape,
                        estimatedArmorValue());
            }
            //buf[20] = '\0';
            printString("                    ", 0, y, &white, &black, 0);
            printString(buf, (20 - strLenWithoutEscapes(buf)) / 2, y++, (dim ? &darkGray : &gray), &black, 0);
        }
        if (y < ROWS - 1 && rogue.gold) {
            sprintf(buf, "Gold: %li", rogue.gold);
            buf[20] = '\0';
            printString("                    ", 0, y, &white, &black, 0);
            printString(buf, (20 - strLenWithoutEscapes(buf)) / 2, y++, (dim ? &darkGray : &gray), &black, 0);
        }
        if (y < ROWS - 1) {
            tempColorEscape[0] = '\0';
            grayColorEscape[0] = '\0';
            tempColor = playerInShadowColor;
            percent = (rogue.stealthRange - 2) * 100 / 28;
            applyColorAverage(&tempColor, &black, percent);
            applyColorAugment(&tempColor, &playerInLightColor, percent);
            if (dim) {
                applyColorAverage(&tempColor, &black, 50);
            }
            encodeMessageColor(tempColorEscape, 0, &tempColor);
            encodeMessageColor(grayColorEscape, 0, (dim ? &darkGray : &gray));
            sprintf(buf, "%sStealth range: %i%s",
                    tempColorEscape,
                    rogue.stealthRange,
                    grayColorEscape);
            printString("                    ", 0, y, &white, &black, 0);
            printString(buf, 1, y++, (dim ? &darkGray : &gray), &black, 0);
        }
    } else if (y < ROWS - 1) {
        if (monst->wasNegated && monst->newPowerCount == monst->totalPowerCount
            && (!player.status[STATUS_HALLUCINATING] || rogue.playbackOmniscience )) {
            printString("      Negated       ", 0, y++, (dim ? &darkPink : &pink), &black, 0);
        }
        if (!(monst->info.flags & MONST_INANIMATE) && (y < ROWS - 1)) {
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                printString(hallucinationStrings[rand_range(0, 9)], 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->bookkeepingFlags & MB_CAPTIVE) {
                printString("     (Captive)      ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if ((monst->info.flags & MONST_RESTRICTED_TO_LIQUID)
                        && !cellHasTMFlag(monst->loc, TM_ALLOWS_SUBMERGING)) {
                printString("     (Helpless)     ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->creatureState == MONSTER_SLEEPING) {
                printString("     (Sleeping)     ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->creatureState == MONSTER_ALLY) {
                printString("       (Ally)       ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->creatureState == MONSTER_FLEEING) {
                printString("     (Fleeing)      ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->creatureState == MONSTER_WANDERING) {
                if ((monst->bookkeepingFlags & MB_FOLLOWER) && monst->leader && (monst->leader->info.flags & MONST_IMMOBILE)) {
                    // follower of an immobile leader -- i.e. a totem
                    printString("    (Worshiping)    ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
                } else if ((monst->bookkeepingFlags & MB_FOLLOWER) && monst->leader && (monst->leader->bookkeepingFlags & MB_CAPTIVE)) {
                    // actually a captor/torturer
                    printString("     (Guarding)     ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
                } else {
                    printString("    (Wandering)     ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
                }
            } else if (monst->ticksUntilTurn > max(0, player.ticksUntilTurn) + player.movementSpeed) {
                printString("   (Off balance)    ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            } else if (monst->creatureState == MONSTER_TRACKING_SCENT) {
                printString("     (Hunting)      ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
            }    
        }
    }

    if (y < ROWS - 1) {
        printString("                    ", 0, y++, (dim ? &darkGray : &gray), &black, 0);
    }

    if (highlight) {
        for (i=0; i<20; i++) {
            highlightStrength = smoothHiliteGradient(i, 20-1) / 10;
            for (j=initialY; j < (y == ROWS - 1 ? y : min(y - 1, ROWS - 1)); j++) {
                highlightScreenCell(i, j, &white, highlightStrength);
            }
        }
    }

    restoreRNG;
    return y;
}

void describeHallucinatedItem(char *buf) {
    short kind, maxKinds;
    assureCosmeticRNG;
    enum itemCategory cat = getHallucinatedItemCategory();
    maxKinds = itemKindCount(cat, 0);
    kind = rand_range(0, maxKinds - 1);
    describedItemBasedOnParameters((short)cat, kind, 1, 1, buf);
    restoreRNG;
}

// Returns the y-coordinate after the last line printed.
short printItemInfo(item *theItem, short y, boolean dim, boolean highlight) {
    char name[COLS * 3];
    enum displayGlyph itemChar;
    color itemForeColor, itemBackColor;
    short initialY, i, j, highlightStrength, lineCount;
    boolean inPath;
    short oldRNG;

    if (y >= ROWS - 1) {
        return ROWS - 1;
    }

    initialY = y;

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    if (y < ROWS - 1) {
        // Unhighlight if it's highlighted as part of the path.
        inPath = (pmapAt(theItem->loc)->flags & IS_IN_PATH) ? true : false;
        pmapAt(theItem->loc)->flags &= ~IS_IN_PATH;
        getCellAppearance(theItem->loc, &itemChar, &itemForeColor, &itemBackColor);
        // override the glyph if the item is at the player's location because 
        // getCellAppearance returns the player glyph
        if (theItem->loc.x == player.loc.x && theItem->loc.y == player.loc.y) {
            if (player.status[STATUS_HALLUCINATING] && !rogue.playbackOmniscience) {
                itemChar = getItemCategoryGlyph(getHallucinatedItemCategory());
                itemForeColor = itemColor;
            } else {
                itemChar = theItem->displayChar;
                itemForeColor = *(theItem->foreColor);
            }
        }
        applyColorBounds(&itemForeColor, 0, 100);
        applyColorBounds(&itemBackColor, 0, 100);
        if (inPath) {
            pmapAt(theItem->loc)->flags |= IS_IN_PATH;
        }
        if (dim) {
            applyColorAverage(&itemForeColor, &black, 50);
            applyColorAverage(&itemBackColor, &black, 50);
        }
        plotCharWithColor(itemChar, (windowpos){ 0, y }, &itemForeColor, &itemBackColor);
        printString(":                  ", 1, y, (dim ? &gray : &white), &black, 0);
        if (rogue.playbackOmniscience || !player.status[STATUS_HALLUCINATING]) {
            itemName(theItem, name, true, true, (dim ? &gray : &white));
        } else {
            describeHallucinatedItem(name);
        }
        upperCase(name);
        lineCount = wrapText(NULL, name, 20-3);
        for (i=initialY + 1; i <= initialY + lineCount + 1 && i < ROWS - 1; i++) {
            printString("                    ", 0, i, (dim ? &darkGray : &gray), &black, 0);
        }
        y = printStringWithWrapping(name, 3, y, 20-3, (dim ? &gray : &white), &black, NULL); // Advances y.
    }

    if (highlight) {
        for (i=0; i<20; i++) {
            highlightStrength = smoothHiliteGradient(i, 20-1) / 10;
            for (j=initialY; j <= y && j < ROWS - 1; j++) {
                highlightScreenCell(i, j, &white, highlightStrength);
            }
        }
    }
    y += 2;

    restoreRNG;
    return y;
}

// Returns the y-coordinate after the last line printed.
short printTerrainInfo(short x, short y, short py, const char *description, boolean dim, boolean highlight) {
    enum displayGlyph displayChar;
    color foreColor, backColor;
    short initialY, i, j, highlightStrength, lineCount;
    boolean inPath;
    char name[DCOLS*2];
    color textColor;
    short oldRNG;

    if (py >= ROWS - 1) {
        return ROWS - 1;
    }

    initialY = py;

    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;
    //assureCosmeticRNG;

    if (py < ROWS - 1) {
        // Unhighlight if it's highlighted as part of the path.
        inPath = (pmap[x][y].flags & IS_IN_PATH) ? true : false;
        pmap[x][y].flags &= ~IS_IN_PATH;
        getCellAppearance((pos){ x, y }, &displayChar, &foreColor, &backColor);
        applyColorBounds(&foreColor, 0, 100);
        applyColorBounds(&backColor, 0, 100);
        if (inPath) {
            pmap[x][y].flags |= IS_IN_PATH;
        }
        if (dim) {
            applyColorAverage(&foreColor, &black, 50);
            applyColorAverage(&backColor, &black, 50);
        }
        plotCharWithColor(displayChar, (windowpos){ 0, py }, &foreColor, &backColor);
        printString(":                  ", 1, py, (dim ? &gray : &white), &black, 0);
        strcpy(name, description);
        upperCase(name);
        lineCount = wrapText(NULL, name, 20-3);
        for (i=initialY + 1; i <= initialY + lineCount + 1 && i < ROWS - 1; i++) {
            printString("                    ", 0, i, (dim ? &darkGray : &gray), &black, 0);
        }
        textColor = flavorTextColor;
        if (dim) {
            applyColorScalar(&textColor, 50);
        }
        py = printStringWithWrapping(name, 3, py, 20-3, &textColor, &black, NULL); // Advances y.
    }

    if (highlight) {
        for (i=0; i<20; i++) {
            highlightStrength = smoothHiliteGradient(i, 20-1) / 10;
            for (j=initialY; j <= py && j < ROWS - 1; j++) {
                highlightScreenCell(i, j, &white, highlightStrength);
            }
        }
    }
    py += 2;

    restoreRNG;
    return py;
}

void rectangularShading(short x, short y, short width, short height,
                        const color *backColor, short opacity, screenDisplayBuffer* dbuf) {
    short i, j, dist;

    assureCosmeticRNG;
    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            storeColorComponents(dbuf->cells[i][j].backColorComponents, backColor);

            if (i >= x && i < x + width
                && j >= y && j < y + height) {
                dbuf->cells[i][j].opacity = min(100, opacity);
            } else {
                dist = 0;
                dist += max(0, max(x - i, i - x - width + 1));
                dist += max(0, max(y - j, j - y - height + 1));
                dbuf->cells[i][j].opacity = (int) ((opacity - 10) / max(1, dist));
                if (dbuf->cells[i][j].opacity < 3) {
                    dbuf->cells[i][j].opacity = 0;
                }
            }
        }
    }

//  for (i=0; i<COLS; i++) {
//      for (j=0; j<ROWS; j++) {
//          if (i >= x && i < x + width && j >= y && j < y + height) {
//              plotCharWithColor(' ', (windowpos){ i, j }, &white, &darkGreen);
//          }
//      }
//  }
//  displayMoreSign();

    restoreRNG;
}

#define MIN_DEFAULT_INFO_PANEL_WIDTH    33

// y and width are optional and will be automatically calculated if width <= 0.
// Width will automatically be widened if the text would otherwise fall off the bottom of the
// screen, and x will be adjusted to keep the widening box from spilling off the right of the
// screen.
// If buttons are provided, we'll extend the text box downward, re-position the buttons,
// run a button input loop and return the result.
// (Returns -1 for canceled; otherwise the button index number.)
short printTextBox(char *textBuf, short x, short y, short width,
                   const color *foreColor, const color *backColor,
                   brogueButton *buttons, short buttonCount) {
    short x2, y2, lineCount, i, bx, by, padLines;

    if (width <= 0) {
        // autocalculate y and width
        if (x < DCOLS / 2 - 1) {
            x2 = mapToWindowX(x + 10);
            width = (DCOLS - x) - 20;
        } else {
            x2 = mapToWindowX(10);
            width = x - 20;
        }
        y2 = mapToWindowY(2);

        if (width < MIN_DEFAULT_INFO_PANEL_WIDTH) {
            x2 -= (MIN_DEFAULT_INFO_PANEL_WIDTH - width) / 2;
            width = MIN_DEFAULT_INFO_PANEL_WIDTH;
        }
    } else {
        y2 = y;
        x2 = x;
    }

    while (((lineCount = wrapText(NULL, textBuf, width)) + y2) >= ROWS - 2 && width < COLS-5) {
        // While the text doesn't fit and the width doesn't fill the screen, increase the width.
        width++;
        if (x2 + (width / 2) > COLS / 2) {
            // If the horizontal midpoint of the text box is on the right half of the screen,
            // move the box one space to the left.
            x2--;
        }
    }

    if (buttonCount > 0) {
        padLines = 2;
        bx = x2 + width;
        by = y2 + lineCount + 1;
        for (i=0; i<buttonCount; i++) {
            if (buttons[i].flags & B_DRAW) {
                bx -= strLenWithoutEscapes(buttons[i].text) + 2;
                buttons[i].x = bx;
                buttons[i].y = by;
                if (bx < x2) {
                    // Buttons can wrap to the next line (though are double-spaced).
                    bx = x2 + width - (strLenWithoutEscapes(buttons[i].text) + 2);
                    by += 2;
                    padLines += 2;
                    buttons[i].x = bx;
                    buttons[i].y = by;
                }
            }
        }
    } else {
        padLines = 0;
    }

    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);
    printStringWithWrapping(textBuf, x2, y2, width, foreColor, backColor, &dbuf);
    rectangularShading(x2, y2, width, lineCount + padLines, backColor, INTERFACE_OPACITY, &dbuf);
    overlayDisplayBuffer(&dbuf);

    if (buttonCount > 0) {
        return buttonInputLoop(buttons, buttonCount, x2, y2, width, by - y2 + 1 + padLines, NULL);
    } else {
        return -1;
    }
}

void printMonsterDetails(creature *monst) {
    char textBuf[COLS * 100];
    monsterDetails(textBuf, monst);
    printTextBox(textBuf, monst->loc.x, 0, 0, &white, &black, NULL, 0);
}

// Displays the item info box with the dark blue background.
// If includeButtons is true, we include buttons for item actions.
// Returns the key of an action to take, if any; otherwise -1.
unsigned long printCarriedItemDetails(item *theItem,
                                      short x, short y, short width,
                                      boolean includeButtons) {
    char textBuf[COLS * 100], goldColorEscape[5] = "", whiteColorEscape[5] = "";
    brogueButton buttons[20] = {{{0}}};
    short b;

    itemDetails(textBuf, theItem);

    for (b=0; b<20; b++) {
        initializeButton(&(buttons[b]));
        buttons[b].flags |= B_WIDE_CLICK_AREA;
    }

    b = 0;
    if (includeButtons) {
        encodeMessageColor(goldColorEscape, 0, KEYBOARD_LABELS ? &yellow : &white);
        encodeMessageColor(whiteColorEscape, 0, &white);

        if (theItem->category & (FOOD | SCROLL | POTION | WAND | STAFF | CHARM)) {
            sprintf(buttons[b].text, "   %sa%spply   ", goldColorEscape, whiteColorEscape);
            buttons[b].hotkey[0] = APPLY_KEY;
            b++;
        }
        if (theItem->category & (ARMOR | WEAPON | RING)) {
            if (theItem->flags & ITEM_EQUIPPED) {
                sprintf(buttons[b].text, "  %sr%semove   ", goldColorEscape, whiteColorEscape);
                buttons[b].hotkey[0] = UNEQUIP_KEY;
                b++;
            } else {
                sprintf(buttons[b].text, "   %se%squip   ", goldColorEscape, whiteColorEscape);
                buttons[b].hotkey[0] = EQUIP_KEY;
                b++;
            }
        }
        sprintf(buttons[b].text, "   %sd%srop    ", goldColorEscape, whiteColorEscape);
        buttons[b].hotkey[0] = DROP_KEY;
        b++;

        sprintf(buttons[b].text, "   %st%shrow   ", goldColorEscape, whiteColorEscape);
        buttons[b].hotkey[0] = THROW_KEY;
        b++;

        if (itemCanBeCalled(theItem)) {
            sprintf(buttons[b].text, "   %sc%sall    ", goldColorEscape, whiteColorEscape);
            buttons[b].hotkey[0] = CALL_KEY;
            b++;
        }

        if (KEYBOARD_LABELS) {
            sprintf(buttons[b].text, "  %sR%selabel  ", goldColorEscape, whiteColorEscape);
            buttons[b].hotkey[0] = RELABEL_KEY;
            b++;
        }

        // Add invisible previous and next buttons, so up and down arrows can page through items.
        // Previous
        buttons[b].flags = B_ENABLED; // clear everything else
        buttons[b].hotkey[0] = UP_KEY;
        buttons[b].hotkey[1] = NUMPAD_8;
        buttons[b].hotkey[2] = UP_ARROW;
        b++;
        // Next
        buttons[b].flags = B_ENABLED; // clear everything else
        buttons[b].hotkey[0] = DOWN_KEY;
        buttons[b].hotkey[1] = NUMPAD_2;
        buttons[b].hotkey[2] = DOWN_ARROW;
        b++;
    }
    b = printTextBox(textBuf, x, y, width, &white, &interfaceBoxColor, buttons, b);

    if (!includeButtons) {
        waitForKeystrokeOrMouseClick();
        return -1;
    }

    if (b >= 0) {
        return buttons[b].hotkey[0];
    } else {
        return -1;
    }
}

// Returns true if an action was taken.
void printFloorItemDetails(item *theItem) {
    char textBuf[COLS * 100];
    itemDetails(textBuf, theItem);

    printTextBox(textBuf, theItem->loc.x, 0, 0, &white, &black, NULL, 0);
}

/*
 *  Buttons.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/14/12.
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

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include <time.h>
#include <limits.h>

#define MENU_FLAME_PRECISION_FACTOR     10
#define MENU_FLAME_RISE_SPEED           50
#define MENU_FLAME_SPREAD_SPEED         20
#define MENU_FLAME_COLOR_DRIFT_SPEED    500
#define MENU_FLAME_FADE_SPEED           20
#define MENU_FLAME_UPDATE_DELAY         50
#define MENU_FLAME_ROW_PADDING          2
#define MENU_FLAME_COLOR_SOURCE_COUNT   1136

#define MENU_FLAME_DENOMINATOR          (100 + MENU_FLAME_RISE_SPEED + MENU_FLAME_SPREAD_SPEED)


static void drawMenuFlames(signed short flames[COLS][(ROWS + MENU_FLAME_ROW_PADDING)][3], unsigned char mask[COLS][ROWS]) {
    short i, j, versionStringLength, gameModeStringLength;
    color tempColor = {0};
    const color *maskColor = &black;
    char gameModeString[COLS] = "";
    char dchar;

    versionStringLength = strLenWithoutEscapes(gameConst->versionString);

    if (rogue.wizard) {
        strcpy(gameModeString, "Wizard Mode");
    } else if (rogue.easyMode) {
        strcpy(gameModeString, "Easy Mode");
    }
    gameModeStringLength = strLenWithoutEscapes(gameModeString);

    for (j=0; j<ROWS; j++) {
        for (i=0; i<COLS; i++) {
            if (j == ROWS - 1 && i >= COLS - versionStringLength) {
                dchar = gameConst->versionString[i - (COLS - versionStringLength)];
            } else if (gameModeStringLength && j == ROWS - 1 && i <= gameModeStringLength) {
                dchar = gameModeString[i];
            } else {
                dchar = ' ';
            }

            if (mask[i][j] == 100) {
                plotCharWithColor(dchar, (windowpos){ i, j }, &veryDarkGray, maskColor);
            } else {
                tempColor = black;
                tempColor.red   = flames[i][j][0] / MENU_FLAME_PRECISION_FACTOR;
                tempColor.green = flames[i][j][1] / MENU_FLAME_PRECISION_FACTOR;
                tempColor.blue  = flames[i][j][2] / MENU_FLAME_PRECISION_FACTOR;
                if (mask[i][j] > 0) {
                    applyColorAverage(&tempColor, maskColor, mask[i][j]);
                }
                plotCharWithColor(dchar, (windowpos){ i, j }, &veryDarkGray, &tempColor);
            }
        }
    }
}

static void updateMenuFlames(const color *colors[COLS][(ROWS + MENU_FLAME_ROW_PADDING)],
                      signed short colorSources[MENU_FLAME_COLOR_SOURCE_COUNT][4],
                      signed short flames[COLS][(ROWS + MENU_FLAME_ROW_PADDING)][3]) {

    short i, j, k, l, x, y;
    signed short tempFlames[COLS][3];
    short colorSourceNumber, rand;

    colorSourceNumber = 0;
    for (j=0; j<(ROWS + MENU_FLAME_ROW_PADDING); j++) {
        // Make a temp copy of the current row.
        for (i=0; i<COLS; i++) {
            for (k=0; k<3; k++) {
                tempFlames[i][k] = flames[i][j][k];
            }
        }

        for (i=0; i<COLS; i++) {
            // Each cell is the weighted average of the three color values below and itself.
            // Weight of itself: 100
            // Weight of left and right neighbors: MENU_FLAME_SPREAD_SPEED / 2 each
            // Weight of below cell: MENU_FLAME_RISE_SPEED
            // Divisor: 100 + MENU_FLAME_SPREAD_SPEED + MENU_FLAME_RISE_SPEED

            // Itself:
            for (k=0; k<3; k++) {
                flames[i][j][k] = 100 * flames[i][j][k] / MENU_FLAME_DENOMINATOR;
            }

            // Left and right neighbors:
            for (l = -1; l <= 1; l += 2) {
                x = i + l;
                if (x == -1) {
                    x = COLS - 1;
                } else if (x == COLS) {
                    x = 0;
                }
                for (k=0; k<3; k++) {
                    flames[i][j][k] += MENU_FLAME_SPREAD_SPEED * tempFlames[x][k] / 2 / MENU_FLAME_DENOMINATOR;
                }
            }

            // Below:
            y = j + 1;
            if (y < (ROWS + MENU_FLAME_ROW_PADDING)) {
                for (k=0; k<3; k++) {
                    flames[i][j][k] += MENU_FLAME_RISE_SPEED * flames[i][y][k] / MENU_FLAME_DENOMINATOR;
                }
            }

            // Fade a little:
            for (k=0; k<3; k++) {
                flames[i][j][k] = (1000 - MENU_FLAME_FADE_SPEED) * flames[i][j][k] / 1000;
            }

            if (colors[i][j]) {
                // If it's a color source tile:

                // First, cause the color to drift a little.
                for (k=0; k<4; k++) {
                    colorSources[colorSourceNumber][k] += rand_range(-MENU_FLAME_COLOR_DRIFT_SPEED, MENU_FLAME_COLOR_DRIFT_SPEED);
                    colorSources[colorSourceNumber][k] = clamp(colorSources[colorSourceNumber][k], 0, 1000);
                }

                // Then, add the color to this tile's flames.
                rand = colors[i][j]->rand * colorSources[colorSourceNumber][0] / 1000;
                flames[i][j][0] += (colors[i][j]->red   + (colors[i][j]->redRand    * colorSources[colorSourceNumber][1] / 1000) + rand) * MENU_FLAME_PRECISION_FACTOR;
                flames[i][j][1] += (colors[i][j]->green + (colors[i][j]->greenRand  * colorSources[colorSourceNumber][2] / 1000) + rand) * MENU_FLAME_PRECISION_FACTOR;
                flames[i][j][2] += (colors[i][j]->blue  + (colors[i][j]->blueRand   * colorSources[colorSourceNumber][3] / 1000) + rand) * MENU_FLAME_PRECISION_FACTOR;

                colorSourceNumber++;
            }
        }
    }
}

// Takes a grid of values, each of which is 0 or 100, and fills in some middle values in the interstices.
static void antiAlias(unsigned char mask[COLS][ROWS]) {
    short i, j, x, y, dir, nbCount;
    const short intensity[5] = {0, 0, 35, 50, 60};

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            if (mask[i][j] < 100) {
                nbCount = 0;
                for (dir=0; dir<4; dir++) {
                    x = i + nbDirs[dir][0];
                    y = j + nbDirs[dir][1];
                    if (locIsInWindow((windowpos){ x, y }) && mask[x][y] == 100) {
                        nbCount++;
                    }
                }
                mask[i][j] = intensity[nbCount];
            }
        }
    }
}

#define MENU_TITLE_OFFSET_X (-7)
#define MENU_TITLE_OFFSET_Y (-2)

static void initializeMenuFlames(boolean includeTitle,
                          const color *colors[COLS][(ROWS + MENU_FLAME_ROW_PADDING)],
                          color colorStorage[COLS],
                          signed short colorSources[MENU_FLAME_COLOR_SOURCE_COUNT][4],
                          signed short flames[COLS][(ROWS + MENU_FLAME_ROW_PADDING)][3],
                          unsigned char mask[COLS][ROWS]) {
    short i, j, k, colorSourceCount;

    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            mask[i][j] = 0;
        }
    }

    for (i=0; i<COLS; i++) {
        for (j=0; j<(ROWS + MENU_FLAME_ROW_PADDING); j++) {
            colors[i][j] = NULL;
            for (k=0; k<3; k++) {
                flames[i][j][k] = 0;
            }
        }
    }

    // Seed source color random components.
    for (i=0; i<MENU_FLAME_COLOR_SOURCE_COUNT; i++) {
        for (k=0; k<4; k++) {
            colorSources[i][k] = rand_range(0, 1000);
        }
    }

    // Put some flame source along the bottom row.
    colorSourceCount = 0;
    for (i=0; i<COLS; i++) {
        colorStorage[colorSourceCount] = flameSourceColor;
        applyColorAverage(&(colorStorage[colorSourceCount]), &flameSourceColorSecondary, 100 - (smoothHiliteGradient(i, COLS - 1) + 25));

        colors[i][(ROWS + MENU_FLAME_ROW_PADDING)-1] = &(colorStorage[colorSourceCount]);
        colorSourceCount++;
    }

    if (includeTitle) {
        // Wreathe the title in flames, and mask it in black.
        for (i=0; i<gameConst->mainMenuTitleWidth; i++) {
            for (j=0; j<gameConst->mainMenuTitleHeight; j++) {
                if (mainMenuTitle[j * gameConst->mainMenuTitleWidth + i] != ' ') {
                    colors[(COLS - gameConst->mainMenuTitleWidth)/2 + i + MENU_TITLE_OFFSET_X][(ROWS - gameConst->mainMenuTitleHeight)/2 + j + MENU_TITLE_OFFSET_Y] = &flameTitleColor;
                    colorSourceCount++;
                    mask[(COLS - gameConst->mainMenuTitleWidth)/2 + i + MENU_TITLE_OFFSET_X][(ROWS - gameConst->mainMenuTitleHeight)/2 + j + MENU_TITLE_OFFSET_Y] = 100;
               }
            }
        }

        // Anti-alias the mask.
        antiAlias(mask);
    }

    brogueAssert(colorSourceCount <= MENU_FLAME_COLOR_SOURCE_COUNT);

    // Simulate the background flames for a while
    for (i=0; i<100; i++) {
        updateMenuFlames(colors, colorSources, flames);
    }

}

/// @brief Inititializes a main menu button. Does not position the button.
/// @param button The button to initialize
/// @param textWithHotkey The button text. A string with 2 format specifiers for color escapes,
/// denoting the start and end of the hotkey text (e.g. "%sN%sew Game").
/// @param hotkey1 Keyboard hotkey #1
/// @param hotkey2 Keyboard hotkey #2
static void initializeMainMenuButton(brogueButton *button, char *textWithHotkey, unsigned long hotkey1, unsigned long hotkey2, enum NGCommands command) {

    initializeButton(button);

    char textWithoutHotkey[BUTTON_TEXT_SIZE];
    snprintf(textWithoutHotkey, BUTTON_TEXT_SIZE - 1, textWithHotkey, "","");

    setButtonText(button, textWithHotkey, textWithoutHotkey);
    button->hotkey[0] = hotkey1;
    button->hotkey[1] = hotkey2;
    button->flags |= B_WIDE_CLICK_AREA;
    button->buttonColor = titleButtonColor;
    button->command = command;
}

#define MAIN_MENU_BUTTON_COUNT 4

/// @brief Initializes the main menu buttons
/// @param buttons An array of buttons to initialize
static void initializeMainMenuButtons(brogueButton *buttons) {

    initializeMainMenuButton(&(buttons[0]), "     %sN%sew Game     ", 'n', 'N', NG_NEW_GAME);
    initializeMainMenuButton(&(buttons[1]), " *     %sP%slay       ", 'p', 'P', NG_FLYOUT_PLAY);
    initializeMainMenuButton(&(buttons[2]), " *     %sV%siew       ", 'v', 'V', NG_FLYOUT_VIEW);
    initializeMainMenuButton(&(buttons[3]), "       %sQ%suit       ", 'q', 'Q', NG_QUIT);

    // Add a left-facing triangle to all the buttons except quit
    for (int i=0; i<MAIN_MENU_BUTTON_COUNT-1; i++) {
        buttons[i].symbol[0] = G_LEFT_TRIANGLE;
    }

}

/// @brief Sets button x, y coordinates, stacking them vertically either top to bottom or bottom to top
/// @param buttons An array of buttons to stack
/// @param buttonCount The number of buttons in the array
/// @param startPosition The position of the first button to place
/// @param spacing The number of rows between buttons
/// @param topToBottomFlag If true, @position is the top of the stack. Otherwise it's the bottom and the array is processed in reverse order.
static void stackButtons(brogueButton *buttons, short buttonCount, windowpos startPosition, short spacing, boolean topToBottomFlag) {
    short y = startPosition.window_y;

    if (topToBottomFlag) {
        for (int i = 0; i < buttonCount; i++) {
            buttons[i].x = startPosition.window_x;
            buttons[i].y = y;
            y += spacing;
        }
    } else {
        for (int i = buttonCount - 1; i >= 0; i--) {
            buttons[i].x = startPosition.window_x;
            buttons[i].y = y;
            y -= spacing;
        }
    }
}

/// @brief Initializes a menu with buttons and a background/shadow. Relies on pre-positioned buttons with text.
/// Dynamically determines the menu size based on the button positions and text.
/// @param menu The menu to initialize
/// @param buttons An array of initialized, positioned buttons, with text
/// @param buttonCount The number of buttons in the array
/// @param shadowBuf The display buffer object for the background/shadow
static void initializeMenu(buttonState *menu, brogueButton *buttons, short buttonCount, screenDisplayBuffer *shadowBuf) {
    memset((void *) menu, 0, sizeof( buttonState ));
    short minX, maxX, minY, maxY;
    minX = COLS;
    minY = ROWS;
    maxX = maxY = 0;

    // determine the button frame size and position (upper-left)
    for (int i = 0; i < buttonCount; i++) {
        minX = min(minX, buttons[i].x);
        maxX = max(maxX, buttons[i].x + strLenWithoutEscapes(buttons[i].text));
        minY = min(minY, buttons[i].y);
        maxY = max(maxY, buttons[i].y);
    }

    short width = maxX - minX;
    short height = maxY - minY;

    initializeButtonState(menu, buttons, buttonCount, minX, minY, width, height);

    // Draws a rectangular shaded area of the specified color and opacity to a buffer. Position x, y is the upper/left.
    // The shading effect outside the rectangle decreases with distance.
    // Warning: shading of neighboring rectangles stacks
    clearDisplayBuffer(shadowBuf);
    rectangularShading(minX, minY, width, height + 1, &black, INTERFACE_OPACITY, shadowBuf);
}

/// @brief Initialize the main menu
/// @param menu The main menu
/// @param buttons The main menu buttons
/// @param position The window position of the quit button
/// @param shadowBuf The display buffer object for the background/shadow
static void initializeMainMenu(buttonState *menu, brogueButton *buttons, windowpos position, screenDisplayBuffer *shadowBuf) {
    initializeMainMenuButtons(buttons);
    stackButtons(buttons, MAIN_MENU_BUTTON_COUNT, position, 2, false);

    initializeMenu(menu, buttons, MAIN_MENU_BUTTON_COUNT, shadowBuf);
}

/// @brief Initialize a flyout menu and position the buttons
/// @param menu The menu to initialize
/// @param shadowBuf The display buffer for the menu background/shadow
/// @param buttons The buttons to add to the menu
/// @param position The window position of the anchor button. All buttons are positioned relative to this location.
static void initializeFlyoutMenu(buttonState *menu, screenDisplayBuffer *shadowBuf, brogueButton *buttons, windowpos position) {
    short buttonCount = 0;

    if (rogue.nextGame == NG_FLYOUT_PLAY) {

        buttonCount = 4;
        initializeMainMenuButton(&(buttons[0]), "  New %sS%seeded Game  ", 's', 'S', NG_NEW_GAME_WITH_SEED);
        initializeMainMenuButton(&(buttons[1]), "     %sL%soad Game     ", 'l', 'L', NG_OPEN_GAME);
        initializeMainMenuButton(&(buttons[2]), "  Change V%sa%sriant   ", 'a', 'A', NG_GAME_VARIANT);
        initializeMainMenuButton(&(buttons[3]), "   Change %sM%sode     ", 'm', 'M', NG_GAME_MODE);

    } else if (rogue.nextGame == NG_FLYOUT_VIEW) {

        buttonCount = 3;
        initializeMainMenuButton(&(buttons[0]), "   View %sR%secording  ", 'r', 'R', NG_VIEW_RECORDING);
        initializeMainMenuButton(&(buttons[1]), "    %sH%sigh Scores    ", 'h', 'H', NG_HIGH_SCORES);
        initializeMainMenuButton(&(buttons[2]), "    %sG%same Stats     ", 'g', 'G', NG_GAME_STATS);

    } else {
        return;
    }

    stackButtons(buttons, buttonCount, position, 2, false);
    initializeMenu(menu, buttons, buttonCount, shadowBuf);
}

/// @brief Displays a dialog window for the user to chose a game variant
static void chooseGameVariant() {
    short gameVariantChoice;
    char textBuf[TEXT_MAX_LENGTH] = "", tmpBuf[TEXT_MAX_LENGTH] = "", goldColorEscape[5] = "", whiteColorEscape[5] = "";

    encodeMessageColor(goldColorEscape, 0, &yellow);
    encodeMessageColor(whiteColorEscape, 0, &white);

    snprintf(textBuf, TEXT_MAX_LENGTH, "%sBrogue%s\n", goldColorEscape, whiteColorEscape);
    append(textBuf, "Classic Brogue. The endlessly captivating masterpiece of dungeon adventuring.\n\n", TEXT_MAX_LENGTH);

    snprintf(tmpBuf, TEXT_MAX_LENGTH, "%sRapid Brogue%s\n", goldColorEscape, whiteColorEscape);
    append(textBuf, tmpBuf, TEXT_MAX_LENGTH);
    append(textBuf, "Die faster and more often in this quarter-length version of the classic game!\n\n", TEXT_MAX_LENGTH);

    brogueButton buttons[2];
    initializeMainMenuButton(&(buttons[0]), "  %sR%sapid Brogue     ", 'r', 'R', NG_NOTHING);
    initializeMainMenuButton(&(buttons[1]), "     %sB%srogue        ", 'b', 'B', NG_NOTHING);
    
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    gameVariantChoice = printTextBox(textBuf, 20, 7, 45, &white, &black, buttons, 2);
    restoreDisplayBuffer(&rbuf);

    if (gameVariantChoice == 1) {
        gameVariant = VARIANT_BROGUE;
    } else if (gameVariantChoice == 0) {
        gameVariant = VARIANT_RAPID_BROGUE;
    } else {
        rogue.nextGame = NG_NOTHING;
    }
}

/// @brief Displays a dialog window for the user to chose a game mode. The game mode is displayed in the bottom left
/// on the title screen (except normal mode).
static void chooseGameMode() {
    short gameMode;
    char textBuf[TEXT_MAX_LENGTH] = "", tmpBuf[TEXT_MAX_LENGTH] = "", goldColorEscape[5] = "", whiteColorEscape[5] = "";

    encodeMessageColor(goldColorEscape, 0, &yellow);
    encodeMessageColor(whiteColorEscape, 0, &white);

    snprintf(textBuf, TEXT_MAX_LENGTH, "%sNormal Mode%s\n", goldColorEscape, whiteColorEscape);
    append(textBuf, "Punishingly difficult. Maliciously alluring. Perfectly normal.\n\n", TEXT_MAX_LENGTH);

    snprintf(tmpBuf, TEXT_MAX_LENGTH, "%sEasy Mode%s\n", goldColorEscape, whiteColorEscape);
    append(textBuf, tmpBuf, TEXT_MAX_LENGTH);
    append(textBuf, "Succumb to temptation and transform into a powerful demon, taking 20% as much damage. "
                    "But great power comes at a great price -- you keep only 10% of your score.\n\n", TEXT_MAX_LENGTH);

    snprintf(tmpBuf, TEXT_MAX_LENGTH, "%sWizard Mode%s\n", goldColorEscape, whiteColorEscape);
    append(textBuf, tmpBuf, TEXT_MAX_LENGTH);
    append(textBuf, "Play as an invincible wizard that starts with legendary items and is magically reborn after every "
                    "death. Summon monsters and make them friend or foe. Conjure any item out of thin air. "
                    "(Your score is not saved.)", TEXT_MAX_LENGTH);

    brogueButton buttons[3];
    initializeMainMenuButton(&(buttons[0]), "      %sW%sizard       ", 'w', 'W', NG_NOTHING);
    initializeMainMenuButton(&(buttons[1]), "       %sE%sasy        ", 'e', 'E', NG_NOTHING);
    initializeMainMenuButton(&(buttons[2]), "      %sN%sormal       ", 'n', 'N', NG_NOTHING);
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    gameMode = printTextBox(textBuf, 10, 5, 66, &white, &black, buttons, 3);
    restoreDisplayBuffer(&rbuf);
    if (gameMode == 0) {
        rogue.wizard = true;
        rogue.easyMode = false;
    } else if (gameMode == 1) {
        rogue.wizard = false;
        rogue.easyMode = true;
    } else if (gameMode == 2) {
        rogue.wizard = false;
        rogue.easyMode = false;
    }

    rogue.nextGame = NG_NOTHING;
}

/// @brief Used on the title screen for showing/hiding the flyout menus
/// @return True if rogue.nextGame is a flyout command
static boolean isFlyoutActive() {
    return ((int)rogue.nextGame >= (int)NG_FLYOUT_PLAY && rogue.nextGame <= (int)NG_FLYOUT_OPTIONS);
}

/// @brief Used to align the bottom flyout button of a flyout menu with the position of the
/// main menu button that triggers the flyout
/// @param buttons The array of main menu buttons
/// @return The window position (bottom-left) of the button associated with the current nextGame value
static windowpos getNextGameButtonPos(brogueButton *buttons) {
    for (int i = 0; i < MAIN_MENU_BUTTON_COUNT; i++) {
        if (buttons[i].command == rogue.nextGame) {
            return (windowpos) {buttons[i].x, buttons[i].y};
        }
    }
    return mapToWindow(INVALID_POS);
}

/// @brief Changes the appearance of the main menu buttons based the active flyout menu (if any), so
/// the button associated with the active flyout is more prominently displayed.
/// @param menu The main menu
static void redrawMainMenuButtons(buttonState *menu, screenDisplayBuffer *button_dbuf) {
    enum buttonDrawStates drawState;

    if (rogue.nextGame == NG_NOTHING) {
        drawButtonsInState(menu, button_dbuf);
    } else {
        //darken the main menu buttons not selected
        for (int i = 0; i < MAIN_MENU_BUTTON_COUNT; i++) {
            drawState = (menu->buttons[i].command == rogue.nextGame) ? BUTTON_NORMAL : BUTTON_PRESSED;
            drawButton(&(menu->buttons[i]), drawState, button_dbuf);
        }
    }
}

#define FLYOUT_X 59

static void titleMenu() {
    signed short flames[COLS][(ROWS + MENU_FLAME_ROW_PADDING)][3]; // red, green and blue
    signed short colorSources[MENU_FLAME_COLOR_SOURCE_COUNT][4]; // red, green, blue, and rand, one for each color source (no more than MENU_FLAME_COLOR_SOURCE_COUNT).
    const color *colors[COLS][(ROWS + MENU_FLAME_ROW_PADDING)];
    color colorStorage[COLS];
    unsigned char mask[COLS][ROWS];

    // Main menu
    buttonState mainMenu;
    brogueButton mainButtons[MAIN_MENU_BUTTON_COUNT];
    screenDisplayBuffer mainShadowBuf;

    // Flyout menu
    buttonState flyoutMenu;
    brogueButton flyoutButtons[10];
    screenDisplayBuffer flyoutShadowBuf;

    // Initialize the RNG so the flames aren't always the same.
    seedRandomGenerator(0);

    // Empty nextGamePath and nextGameSeed so that the buttons don't try to load an old game path or seed.
    rogue.nextGamePath[0] = '\0';
    rogue.nextGameSeed = 0;

    blackOutScreen();

    // Initialize the main menu with buttons stacked on top of the quit button
    windowpos quitButtonPosition = {COLS - 20, ROWS - 3};
    initializeMainMenu(&mainMenu, mainButtons, quitButtonPosition, &mainShadowBuf);

    // Display the title and flames
    initializeMenuFlames(true, colors, colorStorage, colorSources, flames, mask);
    rogue.creaturesWillFlashThisTurn = false; // total unconscionable hack

    rogueEvent theEvent;
    short mainIndex = -1;
    short flyoutIndex = -1;
    windowpos bPos;

    // Outer loop for menu navigation (showing/hiding the flyout menus). Repeat until the user selects
    // a button with a command handled by mainBrogueJunction.
    do {
        if (isFlyoutActive()) {
            bPos = getNextGameButtonPos(mainButtons);
            initializeFlyoutMenu(&flyoutMenu, &flyoutShadowBuf, flyoutButtons, (windowpos){FLYOUT_X, bPos.window_y});
        }

        // Inner input loop until the user selects a button or presses a key. For mouse input, a button
        // is considered selected only on the MOUSE_UP event.
        do {
            if (isApplicationActive()) {
                // Update the display.
                updateMenuFlames(colors, colorSources, flames);
                drawMenuFlames(flames, mask);
                overlayDisplayBuffer(&mainShadowBuf);

                // Draw the main menu buttons
                screenDisplayBuffer dbuf;
                clearDisplayBuffer(&dbuf);
                redrawMainMenuButtons(&mainMenu, &dbuf);
                overlayDisplayBuffer(&dbuf);

                //Show flyout if selected
                if (isFlyoutActive()) {
                    overlayDisplayBuffer(&flyoutShadowBuf);
                    screenDisplayBuffer flyout_dbuf;
                    clearDisplayBuffer(&flyout_dbuf);
                    drawButtonsInState(&flyoutMenu, &flyout_dbuf);
                    overlayDisplayBuffer(&flyout_dbuf);
                    mainMenu.buttonDepressed = -1;
                    mainMenu.buttonFocused = -1;
                }
                // Pause briefly.
                if (pauseBrogue(MENU_FLAME_UPDATE_DELAY, (PauseBehavior){.interuptForMouseMove = true})) {
                    // There was input during the pause! Get the input.
                    nextBrogueEvent(&theEvent, true, false, true);

                    // quickstart a new game
                    if (theEvent.eventType == KEYSTROKE && (theEvent.param1 == 'n' || theEvent.param1 == 'N')) {
                        rogue.nextGame = NG_NEW_GAME;
                        break;
                    }

                    // Process the flyout menu input as needed
                    if (isFlyoutActive()) {
                        flyoutIndex = processButtonInput(&flyoutMenu, NULL, &theEvent);
                        if (flyoutIndex != -1 && (theEvent.eventType == MOUSE_UP || theEvent.eventType == KEYSTROKE)) {
                            rogue.nextGame = flyoutButtons[flyoutIndex].command;
                        }
                        if (rogue.nextGame == NG_GAME_MODE) {
                            chooseGameMode();
                        } else if (rogue.nextGame == NG_GAME_VARIANT) {
                            chooseGameVariant();
                        }
                    }
                    // Process the main menu input
                    mainIndex = processButtonInput(&mainMenu, NULL, &theEvent);

                    if (theEvent.eventType == MOUSE_UP || theEvent.eventType == KEYSTROKE) {
                        if (mainIndex != - 1 && rogue.nextGame != mainButtons[mainIndex].command) {
                            rogue.nextGame = mainButtons[mainIndex].command;
                        } else if (flyoutIndex == -1) {
                            // Hide the flyout menu if the user clicked somewhere random, pressed a random key, or re-selected the
                            // button for the active flyout (e.g. 'p', 'p' in succession opens and closes the play flyout)
                            rogue.nextGame = NG_NOTHING;
                        }
                    }
                }
            } else {
                pauseBrogue(64, PAUSE_BEHAVIOR_DEFAULT);
            }
        } while (theEvent.eventType != MOUSE_UP && theEvent.eventType != KEYSTROKE && (isFlyoutActive() || rogue.nextGame == NG_NOTHING));
    } while (isFlyoutActive() || rogue.nextGame == NG_NOTHING);
    drawMenuFlames(flames, mask);
}

// Closes Brogue without any further prompts, animations, or user interaction.
int quitImmediately() {
    // If we are recording a game, save it.
    if (rogue.recording) {
        flushBufferToFile();
        if (rogue.gameInProgress && !rogue.quit && !rogue.gameHasEnded) {
            // Game isn't over yet, create a savegame.
            saveGameNoPrompt();
        } else {
            // Save it as a recording.
            char path[BROGUE_FILENAME_MAX];
            saveRecordingNoPrompt(path);
        }
    }
    return EXIT_STATUS_SUCCESS;
}

void dialogAlert(char *message) {

    brogueButton OKButton;
    initializeButton(&OKButton);
    strcpy(OKButton.text, "     OK     ");
    OKButton.hotkey[0] = RETURN_KEY;
    OKButton.hotkey[1] = ACKNOWLEDGE_KEY;
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    printTextBox(message, COLS/3, ROWS/3, COLS/3, &white, &interfaceBoxColor, &OKButton, 1);
    restoreDisplayBuffer(&rbuf);
}

static boolean stringsExactlyMatch(const char *string1, const char *string2) {
    short i;
    for (i=0; string1[i] && string2[i]; i++) {
        if (string1[i] != string2[i]) {
            return false;
        }
    }
    return string1[i] == string2[i];
}

// Used to compare the dates of two fileEntry variables
// Returns (int):
//      < 0 if 'b' date is lesser than 'a' date
//      = 0 if 'b' date is equal to 'a' date,
//      > 0 if 'b' date is greater than 'a' date
static int fileEntryCompareDates(const void *a, const void *b) {
    fileEntry *f1 = (fileEntry *)a;
    fileEntry *f2 = (fileEntry *)b;
    time_t t1, t2;
    double diff;

    t1 = mktime(&f1->date);
    t2 = mktime(&f2->date);
    diff = difftime(t2, t1);

    //char date_f1[11];
    //char date_f2[11];
    //strftime(date_f1, sizeof(date_f1), DATE_FORMAT, &f1->date);
    //strftime(date_f2, sizeof(date_f2), DATE_FORMAT, &f2->date);
    //printf("\nf1: %s\t%s",date_f1,f1->path);
    //printf("\nf2: %s\t%s",date_f2,f2->path);
    //printf("\ndiff: %f\n", diff);

    return (int)diff;
}

#define FILES_ON_PAGE_MAX               (min(26, ROWS - 7)) // Two rows (top and bottom) for flames, two rows for border, one for prompt, one for heading.
#define MAX_FILENAME_DISPLAY_LENGTH     53
boolean dialogChooseFile(char *path, const char *suffix, const char *prompt) {
    short i, j, count, x, y, width, height, suffixLength, pathLength, maxPathLength, currentPageStart;
    brogueButton buttons[FILES_ON_PAGE_MAX + 2];
    fileEntry *files;
    boolean retval = false, again;
    screenDisplayBuffer dbuf;
    
    const color *dialogColor = &interfaceBoxColor;
    char *membuf;
    char fileDate [11];

    suffixLength = strlen(suffix);
    files = listFiles(&count, &membuf);
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    maxPathLength = strLenWithoutEscapes(prompt);

    // First, we want to filter the list by stripping out any filenames that do not end with suffix.
    // i is the entry we're testing, and j is the entry that we move it to if it qualifies.
    for (i=0, j=0; i<count; i++) {
        pathLength = strlen(files[i].path);
        //printf("\nString 1: %s", &(files[i].path[(max(0, pathLength - suffixLength))]));
        if (stringsExactlyMatch(&(files[i].path[(max(0, pathLength - suffixLength))]), suffix)) {

            // This file counts!
            if (i > j) {
                files[j] = files[i];
                //strftime(fileDate, sizeof(fileDate), DATE_FORMAT, &files[j].date);
                //printf("\nMatching file: %s\twith date: %s", files[j].path, fileDate);
            }
            j++;

            // Keep track of the longest length.
            if (min(pathLength, MAX_FILENAME_DISPLAY_LENGTH) + 10 > maxPathLength) {
                maxPathLength = min(pathLength, MAX_FILENAME_DISPLAY_LENGTH) + 10;
            }
        }
    }
    count = j;

    // Once we have all relevant files, we sort them by date descending
    qsort(files, count, sizeof(fileEntry), &fileEntryCompareDates);

    currentPageStart = 0;

    do { // Repeat to permit scrolling.
        again = false;

        for (i=0; i<min(count - currentPageStart, FILES_ON_PAGE_MAX); i++) {
            initializeButton(&(buttons[i]));
            buttons[i].flags &= ~(B_WIDE_CLICK_AREA | B_GRADIENT);
            buttons[i].buttonColor = *dialogColor;
            if (KEYBOARD_LABELS) {
                sprintf(buttons[i].text, "%c) ", 'a' + i);
            } else {
                buttons[i].text[0] = '\0';
            }
            strncat(buttons[i].text, files[currentPageStart+i].path, MAX_FILENAME_DISPLAY_LENGTH);

            // Clip off the file suffix from the button text.
            buttons[i].text[strlen(buttons[i].text) - suffixLength] = '\0'; // Snip!
            buttons[i].hotkey[0] = 'a' + i;
            buttons[i].hotkey[1] = 'A' + i;

            // Clip the filename length if necessary.
            if (strlen(buttons[i].text) > MAX_FILENAME_DISPLAY_LENGTH) {
                strcpy(&(buttons[i].text[MAX_FILENAME_DISPLAY_LENGTH - 3]), "...");
            }

            //strftime(fileDate, sizeof(fileDate), DATE_FORMAT, &files[currentPageStart+i].date);
            //printf("\nFound file: %s, with date: %s", files[currentPageStart+i].path, fileDate);
        }

        x = (COLS - maxPathLength) / 2;
        width = maxPathLength;
        height = min(count - currentPageStart, FILES_ON_PAGE_MAX) + 2;
        y = max(4, (ROWS - height) / 2);

        for (i=0; i<min(count - currentPageStart, FILES_ON_PAGE_MAX); i++) {
            pathLength = strlen(buttons[i].text);
            for (j=pathLength; j<(width - 10); j++) {
                buttons[i].text[j] = ' ';
            }
            buttons[i].text[j] = '\0';
            strftime(fileDate, sizeof(fileDate), DATE_FORMAT, &files[currentPageStart+i].date);
            strcpy(&(buttons[i].text[j]), fileDate);
            buttons[i].x = x;
            buttons[i].y = y + 1 + i;
        }

        if (count > FILES_ON_PAGE_MAX) {
            // Create up and down arrows.
            initializeButton(&(buttons[i]));
            strcpy(buttons[i].text, "     *     ");
            buttons[i].symbol[0] = G_UP_ARROW;
            if (currentPageStart <= 0) {
                buttons[i].flags &= ~(B_ENABLED | B_DRAW);
            } else {
                buttons[i].hotkey[0] = UP_ARROW;
                buttons[i].hotkey[1] = NUMPAD_8;
                buttons[i].hotkey[2] = PAGE_UP_KEY;
            }
            buttons[i].x = x + (width - 11)/2;
            buttons[i].y = y;

            i++;
            initializeButton(&(buttons[i]));
            strcpy(buttons[i].text, "     *     ");
            buttons[i].symbol[0] = G_DOWN_ARROW;
            if (currentPageStart + FILES_ON_PAGE_MAX >= count) {
                buttons[i].flags &= ~(B_ENABLED | B_DRAW);
            } else {
                buttons[i].hotkey[0] = DOWN_ARROW;
                buttons[i].hotkey[1] = NUMPAD_2;
                buttons[i].hotkey[2] = PAGE_DOWN_KEY;
            }
            buttons[i].x = x + (width - 11)/2;
            buttons[i].y = y + i;
        }

        if (count) {
            clearDisplayBuffer(&dbuf);
            printString(prompt, x, y - 1, &itemMessageColor, dialogColor, &dbuf);
            rectangularShading(x - 1, y - 1, width + 1, height + 1, dialogColor, INTERFACE_OPACITY, &dbuf);
            overlayDisplayBuffer(&dbuf);

//          for (j=0; j<min(count - currentPageStart, FILES_ON_PAGE_MAX); j++) {
//              strftime(fileDate, sizeof(fileDate), DATE_FORMAT, &files[currentPageStart+j].date);
//              printf("\nSanity check BEFORE: %s, with date: %s", files[currentPageStart+j].path, fileDate);
//              printf("\n   (button name)Sanity check BEFORE: %s", buttons[j].text);
//          }

            i = buttonInputLoop(buttons,
                                min(count - currentPageStart, FILES_ON_PAGE_MAX) + (count > FILES_ON_PAGE_MAX ? 2 : 0),
                                x,
                                y,
                                width,
                                height,
                                NULL);

//          for (j=0; j<min(count - currentPageStart, FILES_ON_PAGE_MAX); j++) {
//              strftime(fileDate, sizeof(fileDate), DATE_FORMAT, &files[currentPageStart+j].date);
//              printf("\nSanity check AFTER: %s, with date: %s", files[currentPageStart+j].path, fileDate);
//              printf("\n   (button name)Sanity check AFTER: %s", buttons[j].text);
//          }

            restoreDisplayBuffer(&rbuf);

            if (i < min(count - currentPageStart, FILES_ON_PAGE_MAX)) {
                if (i >= 0) {
                    retval = true;
                    strcpy(path, files[currentPageStart+i].path);
                } else { // i is -1
                    retval = false;
                }
            } else if (i == min(count - currentPageStart, FILES_ON_PAGE_MAX)) { // Up arrow
                again = true;
                currentPageStart -= FILES_ON_PAGE_MAX;
            } else if (i == min(count - currentPageStart, FILES_ON_PAGE_MAX) + 1) { // Down arrow
                again = true;
                currentPageStart += FILES_ON_PAGE_MAX;
            }
        }

    } while (again);

    free(files);
    free(membuf);

    if (count == 0) {
        dialogAlert("No applicable files found.");
        return false;
    } else {
        return retval;
    }
}

typedef struct gameStats {
    int games;
    int escaped;
    int mastered;
    int won;
    float winRate;
    int deepestLevel;
    int cumulativeLevels;
    int highestScore;
    unsigned long cumulativeScore;
    int mostGold;
    unsigned long cumulativeGold;
    int mostLumenstones;
    int cumulativeLumenstones;
    int fewestTurnsWin; // zero means never won
    unsigned long cumulativeTurns;
    int longestWinStreak;
    int longestMasteryStreak;
    int currentWinStreak;
    int currentMasteryStreak;
} gameStats;

/// @brief Updates the given stats to include a run 
/// @param run The run to add
/// @param stats The stats to update
static void addRuntoGameStats(rogueRun *run, gameStats *stats) {

    stats->games++;
    stats->cumulativeScore += run->score;
    stats->cumulativeGold += run->gold;
    stats->cumulativeLumenstones += run->lumenstones;
    stats->cumulativeLevels += run->deepestLevel;
    stats->cumulativeTurns += run->turns;

    stats->highestScore = (run->score > stats->highestScore) ? run->score : stats->highestScore;
    stats->mostGold = (run->gold > stats->mostGold) ? run->gold : stats->mostGold;
    stats->mostLumenstones = (run->lumenstones > stats->mostLumenstones) ? run->lumenstones : stats->mostLumenstones;
    stats->deepestLevel = (run->deepestLevel > stats->deepestLevel) ? run->deepestLevel : stats->deepestLevel;

    if (strcmp(run->result, "Escaped") == 0 || strcmp(run->result, "Mastered") == 0) {
        if (stats->fewestTurnsWin == 0 || run->turns < stats->fewestTurnsWin) {
            stats->fewestTurnsWin = run->turns;
        }
        stats->won++;
        stats->currentWinStreak++;
        if (strcmp(run->result, "Mastered") == 0) {
            stats->currentMasteryStreak++;
            stats->mastered++;
        } else {
            stats->currentMasteryStreak = 0;
            stats->escaped++;
        }
    } else {
        stats->currentWinStreak = stats->currentMasteryStreak = 0;
    }

    if (stats->currentWinStreak > stats->longestWinStreak) {
        stats->longestWinStreak = stats->currentWinStreak;
    }
    if (stats->currentMasteryStreak > stats->longestMasteryStreak) {
        stats->longestMasteryStreak = stats->currentMasteryStreak;
    }

    if (stats->games == 0) {
        stats->winRate = 0.0;
    } else {
        stats->winRate = ((float)stats->won / stats->games) * 100.0;
    }
}

/// @brief Display the game stats screen
/// Includes "All Time" stats and "Recent" stats. The player can reset their recent stats at any time.
static void viewGameStats(void) {

    gameStats allTimeStats = {0};
    gameStats recentStats = {0};

    rogueRun *runHistory = loadRunHistory(); 
    rogueRun *run = runHistory;

    // calculate stats
    while (run != NULL) {
        if (run->seed != 0) {
            addRuntoGameStats(run, &allTimeStats);
            addRuntoGameStats(run, &recentStats);
        } else { // when seed == 0 the run entry means the player reset their recent stats at this point 
            memset(&recentStats, 0, sizeof(gameStats));
        }        
        run = run->nextRun;
    }

    // free run history
    run = runHistory;
    rogueRun *next;
    while (run != NULL) {
        next = run->nextRun;
        free(run);
        run = next;
    }

    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    blackOutScreen();

    screenDisplayBuffer dbuf;
    clearDisplayBuffer(&dbuf);

    char buf[COLS*3];
    int i = 4;
    int offset = 21;
    char whiteColorEscape[5] = "", yellowColorEscape[5] = "";
    encodeMessageColor(whiteColorEscape, 0, &white);
    encodeMessageColor(yellowColorEscape, 0, &itemMessageColor);

    color titleColor = black;
    applyColorAverage(&titleColor, &itemMessageColor, 100);
    printString("-- GAME STATS --", (COLS - 17 + 1) / 2, 0, &titleColor, &black, &dbuf);

    sprintf(buf,"%-30s%16s%16s","", "All Time", "Recent");
    printString(buf, offset, i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Games Played", whiteColorEscape, allTimeStats.games, recentStats.games);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;
    sprintf(buf,"%-30s%s%16i%16i","Won", whiteColorEscape, allTimeStats.won, recentStats.won);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16.1f%16.1f","Win Rate (%)", whiteColorEscape, allTimeStats.winRate, recentStats.winRate);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Escaped", whiteColorEscape, allTimeStats.escaped, recentStats.escaped);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Mastered", whiteColorEscape, allTimeStats.mastered, recentStats.mastered);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;
    sprintf(buf,"%-30s%s%16i%16i","High Score", whiteColorEscape, allTimeStats.highestScore, recentStats.highestScore);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Most Gold", whiteColorEscape, allTimeStats.mostGold, recentStats.mostGold);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Most Lumenstones", whiteColorEscape, allTimeStats.mostLumenstones, recentStats.mostLumenstones);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;
    sprintf(buf,"%-30s%s%16i%16i","Deepest Level", whiteColorEscape, allTimeStats.deepestLevel, recentStats.deepestLevel);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    float allTimeAverageDepth = 0;
    float recentAverageDepth = 0;

    if(allTimeStats.games > 0) {
        allTimeAverageDepth = (float) allTimeStats.cumulativeLevels / allTimeStats.games;
    }
    if(recentStats.games > 0) {
        recentAverageDepth = (float) recentStats.cumulativeLevels / recentStats.games;
    }

    sprintf(buf,"%-30s%s%16.1f%16.1f","Average Depth", whiteColorEscape, allTimeAverageDepth, recentAverageDepth);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    char allTimeFewestTurns[20] = "-";
    char recentFewestTurns[20] = "-";
    if (allTimeStats.fewestTurnsWin > 0) {
        sprintf(allTimeFewestTurns, "%i", allTimeStats.fewestTurnsWin);
    }
    if (recentStats.fewestTurnsWin > 0) {
        sprintf(recentFewestTurns, "%i", recentStats.fewestTurnsWin);
    }
    sprintf(buf,"%-30s%s%16s%16s","Shortest Win (Turns)", whiteColorEscape, allTimeFewestTurns, recentFewestTurns);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;
    sprintf(buf,"%-30s%s%16i%16i","Longest Win Streak", whiteColorEscape, allTimeStats.longestWinStreak, recentStats.longestWinStreak);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%16i%16i","Longest Mastery Streak", whiteColorEscape, allTimeStats.longestMasteryStreak, recentStats.longestMasteryStreak);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;
    sprintf(buf,"%-30s%s%32i","Current Win Streak", whiteColorEscape, recentStats.currentWinStreak);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);

    sprintf(buf,"%-30s%s%32i","Current Mastery Streak", whiteColorEscape, recentStats.currentMasteryStreak);
    printString(buf, offset, ++i, &itemMessageColor, &black, &dbuf);
    i++;

    // Set the dbuf opacity.
    for (int i=0; i<COLS; i++) {
        for (int j=0; j<ROWS; j++) {
            dbuf.cells[i][j].opacity = INTERFACE_OPACITY;
        }
    }

    // Display.
    overlayDisplayBuffer(&dbuf);
    color continueColor = black;
    applyColorAverage(&continueColor, &goodMessageColor, 100);

    printString(KEYBOARD_LABELS ? "Press space or click to continue." : "Touch anywhere to continue.",
                (COLS - strLenWithoutEscapes(KEYBOARD_LABELS ? "Press space or click to continue." : "Touch anywhere to continue.")) / 2,
                ROWS - 1, &continueColor, &black, 0);

    commitDraws();

    if (recentStats.games > 0) {
        i++;
        brogueButton buttons[1];
        initializeButton(&(buttons[0]));
        if (KEYBOARD_LABELS) {
            sprintf(buttons[0].text,  "  %sR%seset  ", yellowColorEscape, whiteColorEscape);
        } else {
            strcpy(buttons[0].text, "  Reset  ");
        }
        buttons[0].hotkey[0] = 'R';
        buttons[0].hotkey[1] = 'r';
        buttons[0].x = 74;
        buttons[0].y = i;

        if (buttonInputLoop(buttons, 1, 74, 25, 10, 3, NULL) == 0 && confirm("Reset recent stats?",false)) {
            saveResetRun();
        }
    } else {
        waitForKeystrokeOrMouseClick();
    }

    restoreDisplayBuffer(&rbuf);
}

// This is the basic program loop.
// When the program launches, or when a game ends, you end up here.
// If the player has already said what he wants to do next
// (by storing it in rogue.nextGame -- possibilities listed in enum NGCommands),
// we'll do it. The path (rogue.nextGamePath) is essentially a parameter for this command, and
// tells NG_VIEW_RECORDING and NG_OPEN_GAME which file to open. If there is a command but no
// accompanying path, and it's a command that should take a path, then pop up a dialog to have
// the player specify a path. If there is no command (i.e. if rogue.nextGame contains NG_NOTHING),
// then we'll display the title screen so the player can choose.
void mainBrogueJunction() {
    rogueEvent theEvent;
    char path[BROGUE_FILENAME_MAX], buf[100], seedDefault[100];
    short i, j, k;

    // clear screen and display buffer
    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            displayBuffer.cells[i][j].character = 0;
            displayBuffer.cells[i][j].opacity = 100;
            for (k=0; k<3; k++) {
                displayBuffer.cells[i][j].foreColorComponents[k] = 0;
                displayBuffer.cells[i][j].backColorComponents[k] = 0;
            }
            plotCharWithColor(' ', (windowpos){ i, j }, &black, &black);
        }
    }

    initializeGameVariant();

    initializeLaunchArguments(&rogue.nextGame, rogue.nextGamePath, &rogue.nextGameSeed);

    do {
        rogue.gameHasEnded = false;
        rogue.playbackFastForward = false;
        rogue.playbackMode = false;
        switch (rogue.nextGame) {
            case NG_NOTHING:
                // Run the main menu to get a decision out of the player.
                titleMenu();
                break;
            case NG_GAME_VARIANT:
                rogue.nextGame = NG_NOTHING;
                initializeGameVariant();
                break;
            case NG_NEW_GAME:
            case NG_NEW_GAME_WITH_SEED:
                rogue.nextGamePath[0] = '\0';
                randomNumbersGenerated = 0;

                rogue.playbackMode = false;
                rogue.playbackFastForward = false;
                rogue.playbackBetweenTurns = false;

                getAvailableFilePath(path, LAST_GAME_NAME, GAME_SUFFIX);
                strcat(path, GAME_SUFFIX);
                strcpy(currentFilePath, path);

                if (rogue.nextGame == NG_NEW_GAME_WITH_SEED) {
                    if (rogue.nextGameSeed == 0) { // Prompt for seed; default is the previous game's seed.
                        if (previousGameSeed == 0) {
                            seedDefault[0] = '\0';
                        } else {
                            sprintf(seedDefault, "%llu", (unsigned long long)previousGameSeed);
                        }
                        if (getInputTextString(buf, "Generate dungeon with seed number:",
                                               20, // length of "18446744073709551615" (2^64 - 1)
                                               seedDefault,
                                               "",
                                               TEXT_INPUT_NUMBERS,
                                               true)
                            && buf[0] != '\0') {
                            if (!tryParseUint64(buf, &rogue.nextGameSeed)) {
                                // seed is too large, default to the largest possible seed
                                rogue.nextGameSeed = 18446744073709551615ULL;
                            }
                        } else {
                            rogue.nextGame = NG_NOTHING;
                            break; // Don't start a new game after all.
                        }
                    }
                } else {
                    rogue.nextGameSeed = 0; // Seed based on clock.
                }

                rogue.nextGame = NG_NOTHING;
                initializeRogue(rogue.nextGameSeed);
                startLevel(rogue.depthLevel, 1); // descending into level 1

                mainInputLoop();
                if(serverMode) {
                    rogue.nextGame = NG_QUIT;
                }
                freeEverything();
                break;
            case NG_OPEN_GAME:
                rogue.nextGame = NG_NOTHING;
                path[0] = '\0';
                if (rogue.nextGamePath[0]) {
                    strcpy(path, rogue.nextGamePath);
                    strcpy(rogue.currentGamePath, rogue.nextGamePath);
                    rogue.nextGamePath[0] = '\0';
                } else {
                    dialogChooseFile(path, GAME_SUFFIX, "Open saved game:");
                    //chooseFile(path, "Open saved game: ", "Saved game", GAME_SUFFIX);
                }

                if (openFile(path)) {
                    if (loadSavedGame()) {
                        mainInputLoop();
                    }
                    freeEverything();
                } else {
                    //dialogAlert("File not found.");
                }
                rogue.playbackMode = false;
                rogue.playbackOOS = false;

                if(serverMode) {
                    rogue.nextGame = NG_QUIT;
                }
                break;
            case NG_VIEW_RECORDING:
                rogue.nextGame = NG_NOTHING;

                path[0] = '\0';
                if (rogue.nextGamePath[0]) {
                    strcpy(path, rogue.nextGamePath);
                    strcpy(rogue.currentGamePath, rogue.nextGamePath);
                    rogue.nextGamePath[0] = '\0';
                } else {
                    dialogChooseFile(path, RECORDING_SUFFIX, "View recording:");
                    //chooseFile(path, "View recording: ", "Recording", RECORDING_SUFFIX);
                }

                if (openFile(path)) {
                    randomNumbersGenerated = 0;
                    rogue.playbackMode = true;
                    initializeRogue(0); // Seed argument is ignored because we're in playback.
                    if (!rogue.gameHasEnded) {
                        startLevel(rogue.depthLevel, 1);
                        if (nonInteractivePlayback) {
                            rogue.playbackPaused = false;
                        } else {
                            rogue.playbackPaused = true;
                        }
                        displayAnnotation(); // in case there's an annotation for turn 0
                    }

                    while(!rogue.gameHasEnded && rogue.playbackMode) {
                        if (rogue.playbackPaused) {
                            rogue.playbackPaused = false;
                            pausePlayback();
                        }
#ifdef ENABLE_PLAYBACK_SWITCH
                        // We are coming from the end of a recording the user has taken over.
                        // No more event checks, that has already been handled
                        if (rogue.gameHasEnded) {
                            break;
                        }
#endif
                        rogue.RNG = RNG_COSMETIC; // dancing terrain colors can't influence recordings
                        rogue.playbackBetweenTurns = true;
                        nextBrogueEvent(&theEvent, false, true, false);
                        rogue.RNG = RNG_SUBSTANTIVE;

                        executeEvent(&theEvent);
                    }

                    freeEverything();
                } else {
                    // announce file not found
                }
                rogue.playbackMode = false;
                rogue.playbackOOS = false;

                if(serverMode || nonInteractivePlayback) {
                    rogue.nextGame = NG_QUIT;
                }
                break;
            case NG_HIGH_SCORES:
                rogue.nextGame = NG_NOTHING;
                printHighScores(false);
                break;
            case NG_GAME_STATS:
                rogue.nextGame = NG_NOTHING;
                viewGameStats();
                break;
            case NG_QUIT:
                // No need to do anything.
                break;
            default:
                break;
        }
    } while (rogue.nextGame != NG_QUIT);
}

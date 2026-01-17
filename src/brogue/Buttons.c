/*
 *  Buttons.c
 *  Brogue
 *
 *  Created by Brian Walker on 11/18/11.
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
#include <math.h>
#include <time.h>

// Draws the smooth gradient that appears on a button when you hover over or depress it.
// Returns the percentage by which the current tile should be averaged toward a hilite color.
short smoothHiliteGradient(const short currentXValue, const short maxXValue) {
    return (short) (100 * sin(3.14159265 * currentXValue / maxXValue));
}

// Draws the button to the screen, or to a display buffer if one is given.
// Button back color fades from -50% intensity at the edges to the back color in the middle.
// Text is white, but can use color escapes.
//      Hovering highlight augments fore and back colors with buttonHoverColor by 20%.
//      Pressed darkens the middle color (or turns it the hover color if the button is black).
void drawButton(brogueButton *button, enum buttonDrawStates highlight, screenDisplayBuffer *dbuf) {
    if (!(button->flags & B_DRAW)) {
        return;
    }
    //assureCosmeticRNG;
    short oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;

    const int width = strLenWithoutEscapes(button->text);
    color bColorBase = button->buttonColor;
    color fColorBase = ((button->flags & B_ENABLED) ? white : gray);

    if (highlight == BUTTON_HOVER && (button->flags & B_HOVER_ENABLED)) {
        //applyColorAugment(&fColorBase, &buttonHoverColor, 20);
        //applyColorAugment(&bColorBase, &buttonHoverColor, 20);
        applyColorAverage(&fColorBase, &buttonHoverColor, 25);
        applyColorAverage(&bColorBase, &buttonHoverColor, 25);
    }

    color bColorEdge  = bColorBase;
    color bColorMid   = bColorBase;
    applyColorAverage(&bColorEdge, &black, 50);

    if (highlight == BUTTON_PRESSED) {
        applyColorAverage(&bColorMid, &black, 75);
        if (COLOR_DIFF(bColorMid, bColorBase) < 50) {
            bColorMid   = bColorBase;
            applyColorAverage(&bColorMid, &buttonHoverColor, 50);
        }
    }
    color bColor = bColorMid;

    short opacity = button->opacity;
    if (highlight == BUTTON_HOVER || highlight == BUTTON_PRESSED) {
        opacity = 100 - ((100 - opacity) * opacity / 100); // Apply the opacity twice.
    }

    short symbolNumber = 0;

    for (int i = 0, textLoc = 0; i < width && i + button->x < COLS; i++, textLoc++) {
        while (button->text[textLoc] == COLOR_ESCAPE) {
            textLoc = decodeMessageColor(button->text, textLoc, &fColorBase);
        }

        color fColor = fColorBase;

        if (button->flags & B_GRADIENT) {
            const int midPercent = smoothHiliteGradient(i, width - 1);
            bColor = bColorEdge;
            applyColorAverage(&bColor, &bColorMid, midPercent);
        }

        if (highlight == BUTTON_PRESSED) {
            applyColorAverage(&fColor, &bColor, 30);
        }

        if (button->opacity < 100) {
            applyColorAverage(&fColor, &bColor, 100 - opacity);
        }

        bakeColor(&fColor);
        bakeColor(&bColor);
        separateColors(&fColor, &bColor);

        enum displayGlyph displayCharacter = button->text[textLoc];
        if (button->text[textLoc] == '*') {
            if (button->symbol[symbolNumber]) {
                displayCharacter = button->symbol[symbolNumber];
            }
            symbolNumber++;
        }

        if (locIsInWindow((windowpos){ button->x + i, button->y })) {
            plotCharToBuffer(displayCharacter, (windowpos){ button->x + i, button->y }, &fColor, &bColor, dbuf);
            if (dbuf) {
                // Only buffers can have opacity set.
                dbuf->cells[button->x + i][button->y].opacity = opacity;
            }
        }
    }
    restoreRNG;
}

void initializeButton(brogueButton *button) {
    memset((void *) button, 0, sizeof( brogueButton ));
    button->text[0] = '\0';
    button->flags |= (B_ENABLED | B_GRADIENT | B_HOVER_ENABLED | B_DRAW | B_KEYPRESS_HIGHLIGHT);
    button->buttonColor = interfaceButtonColor;
    button->opacity = 100;
    button->textColor = white;
    button->hotkeytextColor = KEYBOARD_LABELS ? itemMessageColor : white;
}

/// @brief Sets the text of a button intialized via initializeButton() or otherwise. Relies on
/// the button's textColor and hotkeyTextColor having been set appropriately.
/// @param button The button
/// @param textWithHotkey The button text for platforms with a keyboard. A string with 2 format specifiers
/// for color escapes, denoting the start and end of the hotkey text (e.g. "%sN%sew Game").
/// @param textWithoutHotkey The button text for platforms without a keyboard
void setButtonText(brogueButton *button, const char *textWithHotkey, const char *textWithoutHotkey) {
    char textColorEscape[5] = "";
    char hotkeyColorEscape[5] = "";
    char textBuf[BUTTON_TEXT_SIZE];
    char textBuf2[BUTTON_TEXT_SIZE];

    encodeMessageColor(textColorEscape, 0, &button->textColor);
    encodeMessageColor(hotkeyColorEscape, 0, &button->hotkeytextColor);

    strcpy(textBuf, textColorEscape);
    if (KEYBOARD_LABELS) {
        snprintf(textBuf2, BUTTON_TEXT_SIZE - sizeof(textColorEscape) - 1, textWithHotkey, hotkeyColorEscape, textColorEscape);
    } else {
        strncpy(textBuf2, textWithoutHotkey, BUTTON_TEXT_SIZE - 1);
    }
    strcat(textBuf, textBuf2);
    strncpy(button->text, textBuf, BUTTON_TEXT_SIZE - 1);
}

void drawButtonsInState(buttonState *state, screenDisplayBuffer *dbuf) {
    // Draw the buttons to the dbuf:
    for (int i=0; i < state->buttonCount; i++) {
        if (state->buttons[i].flags & B_DRAW) {
            enum buttonDrawStates buttonDrawState = BUTTON_NORMAL;
            if (i == state->buttonFocused) {
                buttonDrawState = BUTTON_HOVER;
            }
            if (i == state->buttonDepressed) {
                buttonDrawState = BUTTON_PRESSED;
            }
            drawButton(&(state->buttons[i]), buttonDrawState, dbuf);
        }
    }
}

void initializeButtonState(buttonState *state,
                           brogueButton *buttons,
                           short buttonCount,
                           short winX,
                           short winY,
                           short winWidth,
                           short winHeight) {
    // Initialize variables for the state struct:
    state->buttonChosen = state->buttonFocused = state->buttonDepressed = -1;
    state->buttonCount  = buttonCount;
    state->winX         = winX;
    state->winY         = winY;
    state->winWidth     = winWidth;
    state->winHeight    = winHeight;
    for (int i=0; i < state->buttonCount; i++) {
        state->buttons[i] = buttons[i];
    }
}

// Processes one round of user input, and updates `state` accordingly.
// If a button is pressed, draws to the screen and briefly pauses, but then
// immediately reverts the screen state to beforce `processButtonInput` was caled.
//
// Assumes that input has been solicited (via nextBrogueEvent(event, ___, ___, ___)).
// Also relies on the buttonState having been initialized with initializeButtonState() or otherwise.
// Returns the index of a button if one is chosen.
// Otherwise, returns -1. That can be if the user canceled (in which case *canceled is true),
// or, more commonly, if the user's input in this particular split-second round was not decisive.
short processButtonInput(buttonState *state, boolean *canceled, rogueEvent *event) {
    boolean buttonUsed = false;

    // Mouse event:
    if (event->eventType == MOUSE_DOWN
        || event->eventType == MOUSE_UP
        || event->eventType == MOUSE_ENTERED_CELL) {

        int x = event->param1;
        int y = event->param2;

        // Revert the button with old focus, if any.
        if (state->buttonFocused >= 0) {
            state->buttonFocused = -1;
        }

        // Find the button with new focus, if any.
        int focusIndex;
        for (focusIndex=0; focusIndex < state->buttonCount; focusIndex++) {
            if ((state->buttons[focusIndex].flags & B_DRAW)
                && (state->buttons[focusIndex].flags & B_ENABLED)
                && (state->buttons[focusIndex].y == y || ((state->buttons[focusIndex].flags & B_WIDE_CLICK_AREA) && abs(state->buttons[focusIndex].y - y) <= 1))
                && x >= state->buttons[focusIndex].x
                && x < state->buttons[focusIndex].x + strLenWithoutEscapes(state->buttons[focusIndex].text)) {

                state->buttonFocused = focusIndex;
                if (event->eventType == MOUSE_DOWN) {
                    state->buttonDepressed = focusIndex; // Keeps track of which button is down at the moment. Cleared on mouseup.
                }
                break;
            }
        }
        if (focusIndex == state->buttonCount) { // No focus this round.
            state->buttonFocused = -1;
        }

        // Mouseup:
        if (event->eventType == MOUSE_UP) {
            if (state->buttonDepressed == state->buttonFocused && state->buttonFocused >= 0) {
                // If a button is depressed, and the mouseup happened on that button, it has been chosen and we're done.
                buttonUsed = true;
            } else {
                // Otherwise, no button is depressed. If one was previously depressed, redraw it.
                if (state->buttonDepressed < 0 && !(x >= state->winX && x < state->winX + state->winWidth
                             && y >= state->winY && y < state->winY + state->winHeight)) {
                    // Clicking outside of a button means canceling.
                    if (canceled) {
                        *canceled = true;
                    }
                }

                state->buttonDepressed = -1;
            }
        }
    }

    // Keystroke:
    if (event->eventType == KEYSTROKE) {

        // Cycle through all of the hotkeys of all of the buttons.
        for (int i=0; i < state->buttonCount; i++) {
            for (int k = 0; k < 10 && state->buttons[i].hotkey[k]; k++) {
                if (event->param1 == state->buttons[i].hotkey[k]) {
                    // This button was chosen.

                    if (state->buttons[i].flags & B_DRAW) {
                        // Restore the depressed and focused buttons.

                        // If the button likes to flash when keypressed:
                        if (state->buttons[i].flags & B_KEYPRESS_HIGHLIGHT) {
                            // Depress the chosen button.

                            // Update the display.
                            const SavedDisplayBuffer rbuf = saveDisplayBuffer();
                            screenDisplayBuffer dbuf;
                            clearDisplayBuffer(&dbuf);
                            drawButtonsInState(state, &dbuf);
                            overlayDisplayBuffer(&dbuf);

                            if (!rogue.playbackMode || rogue.playbackPaused) {
                                // Wait for a little; then we're done.
                                pauseBrogue(50, PAUSE_BEHAVIOR_DEFAULT);
                            } else {
                                // Wait long enough for the viewer to see what was selected.
                                pauseAnimation(1000, PAUSE_BEHAVIOR_DEFAULT);
                            }

                            // Revert the display to its appearance before the button-press.
                            restoreDisplayBuffer(&rbuf);
                        }
                    }

                    state->buttonDepressed = i;
                    buttonUsed = true;
                    break;
                }
            }
        }

        if (!buttonUsed
            && (event->param1 == ESCAPE_KEY || event->param1 == ACKNOWLEDGE_KEY)) {
            // If the player pressed escape, we're done.
            if (canceled) {
                *canceled = true;
            }
        }
    }

    if (buttonUsed) {
        state->buttonChosen = state->buttonDepressed;
        return state->buttonChosen;
    } else {
        return -1;
    }
}

// Displays a bunch of buttons and collects user input.
// Returns the index number of the chosen button, or -1 if the user cancels.
// A window region is described by winX, winY, winWidth and winHeight.
// Clicking outside of that region will constitute canceling.
short buttonInputLoop(brogueButton *buttons,
                      short buttonCount,
                      short winX,
                      short winY,
                      short winWidth,
                      short winHeight,
                      rogueEvent *returnEvent) {
    short button;
    boolean canceled;
    rogueEvent theEvent;
    buttonState state = {0};

    assureCosmeticRNG;

    canceled = false;
    
    initializeButtonState(&state, buttons, buttonCount, winX, winY, winWidth, winHeight);

    do {
        screenDisplayBuffer dbuf;
        clearDisplayBuffer(&dbuf);
        drawButtonsInState(&state, &dbuf);

        const SavedDisplayBuffer rbuf = saveDisplayBuffer();
        // Update the display.
        overlayDisplayBuffer(&dbuf);

        // Get input.
        nextBrogueEvent(&theEvent, true, false, false);

        // Process the input.
        button = processButtonInput(&state, &canceled, &theEvent);

        // Revert the display.
        restoreDisplayBuffer(&rbuf);

    } while (button == -1 && !canceled);

    if (returnEvent) {
        *returnEvent = theEvent;
    }

    restoreRNG;

    return button;
}

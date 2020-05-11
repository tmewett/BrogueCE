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
#include "IncludeGlobals.h"
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
void drawButton(brogueButton *button, enum buttonDrawStates highlight, cellDisplayBuffer dbuf[COLS][ROWS]) {
    short i, textLoc, width, midPercent, symbolNumber, opacity, oldRNG;
    color fColor, bColor, fColorBase, bColorBase, bColorEdge, bColorMid;
    enum displayGlyph displayCharacter;

    if (!(button->flags & B_DRAW)) {
        return;
    }
    //assureCosmeticRNG;
    oldRNG = rogue.RNG;
    rogue.RNG = RNG_COSMETIC;

    symbolNumber = 0;

    width = strLenWithoutEscapes(button->text);
    bColorBase = button->buttonColor;
    fColorBase = ((button->flags & B_ENABLED) ? white : gray);

    if (highlight == BUTTON_HOVER && (button->flags & B_HOVER_ENABLED)) {
        //applyColorAugment(&fColorBase, &buttonHoverColor, 20);
        //applyColorAugment(&bColorBase, &buttonHoverColor, 20);
        applyColorAverage(&fColorBase, &buttonHoverColor, 25);
        applyColorAverage(&bColorBase, &buttonHoverColor, 25);
    }

    bColorEdge  = bColorBase;
    bColorMid   = bColorBase;
    applyColorAverage(&bColorEdge, &black, 50);

    if (highlight == BUTTON_PRESSED) {
        applyColorAverage(&bColorMid, &black, 75);
        if (COLOR_DIFF(bColorMid, bColorBase) < 50) {
            bColorMid   = bColorBase;
            applyColorAverage(&bColorMid, &buttonHoverColor, 50);
        }
    }
    bColor = bColorMid;

    opacity = button->opacity;
    if (highlight == BUTTON_HOVER || highlight == BUTTON_PRESSED) {
        opacity = 100 - ((100 - opacity) * opacity / 100); // Apply the opacity twice.
    }

    for (i = textLoc = 0; i < width && i + button->x < COLS; i++, textLoc++) {
        while (button->text[textLoc] == COLOR_ESCAPE) {
            textLoc = decodeMessageColor(button->text, textLoc, &fColorBase);
        }

        fColor = fColorBase;

        if (button->flags & B_GRADIENT) {
            midPercent = smoothHiliteGradient(i, width - 1);
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

        displayCharacter = button->text[textLoc];
        if (button->text[textLoc] == '*') {
            if (button->symbol[symbolNumber]) {
                displayCharacter = button->symbol[symbolNumber];
            }
            symbolNumber++;
        }

        if (coordinatesAreInWindow(button->x + i, button->y)) {
            if (dbuf) {
                plotCharToBuffer(displayCharacter, button->x + i, button->y, &fColor, &bColor, dbuf);
                dbuf[button->x + i][button->y].opacity = opacity;
            } else {
                plotCharWithColor(displayCharacter, button->x + i, button->y, &fColor, &bColor);
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
}

void drawButtonsInState(buttonState *state) {
    short i;

    // Draw the buttons to the dbuf:
    for (i=0; i < state->buttonCount; i++) {
        if (state->buttons[i].flags & B_DRAW) {
            drawButton(&(state->buttons[i]), BUTTON_NORMAL, state->dbuf);
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
    short i, j;

    // Initialize variables for the state struct:
    state->buttonChosen = state->buttonFocused = state->buttonDepressed = -1;
    state->buttonCount  = buttonCount;
    state->winX         = winX;
    state->winY         = winY;
    state->winWidth     = winWidth;
    state->winHeight    = winHeight;
    for (i=0; i < state->buttonCount; i++) {
        state->buttons[i] = buttons[i];
    }
    copyDisplayBuffer(state->rbuf, displayBuffer);
    clearDisplayBuffer(state->dbuf);

    drawButtonsInState(state);

    // Clear the rbuf so that it resets only those parts of the screen in which buttons are drawn in the first place:
    for (i=0; i<COLS; i++) {
        for (j=0; j<ROWS; j++) {
            state->rbuf[i][j].opacity = (state->dbuf[i][j].opacity ? 100 : 0);
        }
    }
}

// Processes one round of user input, and bakes the necessary graphical changes into state->dbuf.
// Does NOT display the buttons or revert the display afterward.
// Assumes that the display has already been updated (via overlayDisplayBuffer(state->dbuf, NULL))
// and that input has been solicited (via nextBrogueEvent(event, ___, ___, ___)).
// Also relies on the buttonState having been initialized with initializeButtonState() or otherwise.
// Returns the index of a button if one is chosen.
// Otherwise, returns -1. That can be if the user canceled (in which case *canceled is true),
// or, more commonly, if the user's input in this particular split-second round was not decisive.
short processButtonInput(buttonState *state, boolean *canceled, rogueEvent *event) {
    short i, k, x, y;
    boolean buttonUsed = false;

    // Mouse event:
    if (event->eventType == MOUSE_DOWN
        || event->eventType == MOUSE_UP
        || event->eventType == MOUSE_ENTERED_CELL) {

        x = event->param1;
        y = event->param2;

        // Revert the button with old focus, if any.
        if (state->buttonFocused >= 0) {
            drawButton(&(state->buttons[state->buttonFocused]), BUTTON_NORMAL, state->dbuf);
            state->buttonFocused = -1;
        }

        // Find the button with new focus, if any.
        for (i=0; i < state->buttonCount; i++) {
            if ((state->buttons[i].flags & B_DRAW)
                && (state->buttons[i].flags & B_ENABLED)
                && (state->buttons[i].y == y || ((state->buttons[i].flags & B_WIDE_CLICK_AREA) && abs(state->buttons[i].y - y) <= 1))
                && x >= state->buttons[i].x
                && x < state->buttons[i].x + strLenWithoutEscapes(state->buttons[i].text)) {

                state->buttonFocused = i;
                if (event->eventType == MOUSE_DOWN) {
                    state->buttonDepressed = i; // Keeps track of which button is down at the moment. Cleared on mouseup.
                }
                break;
            }
        }
        if (i == state->buttonCount) { // No focus this round.
            state->buttonFocused = -1;
        }

        if (state->buttonDepressed >= 0) {
            if (state->buttonDepressed == state->buttonFocused) {
                drawButton(&(state->buttons[state->buttonDepressed]), BUTTON_PRESSED, state->dbuf);
            }
        } else if (state->buttonFocused >= 0) {
            // If no button is depressed, then update the appearance of the button with the new focus, if any.
            drawButton(&(state->buttons[state->buttonFocused]), BUTTON_HOVER, state->dbuf);
        }

        // Mouseup:
        if (event->eventType == MOUSE_UP) {
            if (state->buttonDepressed == state->buttonFocused && state->buttonFocused >= 0) {
                // If a button is depressed, and the mouseup happened on that button, it has been chosen and we're done.
                buttonUsed = true;
            } else {
                // Otherwise, no button is depressed. If one was previously depressed, redraw it.
                if (state->buttonDepressed >= 0) {
                    drawButton(&(state->buttons[state->buttonDepressed]), BUTTON_NORMAL, state->dbuf);
                } else if (!(x >= state->winX && x < state->winX + state->winWidth
                             && y >= state->winY && y < state->winY + state->winHeight)) {
                    // Clicking outside of a button means canceling.
                    if (canceled) {
                        *canceled = true;
                    }
                }

                if (state->buttonFocused >= 0) {
                    // Buttons don't hover-highlight when one is depressed, so we have to fix that when the mouse is up.
                    drawButton(&(state->buttons[state->buttonFocused]), BUTTON_HOVER, state->dbuf);
                }
                state->buttonDepressed = -1;
            }
        }
    }

    // Keystroke:
    if (event->eventType == KEYSTROKE) {

        // Cycle through all of the hotkeys of all of the buttons.
        for (i=0; i < state->buttonCount; i++) {
            for (k = 0; k < 10 && state->buttons[i].hotkey[k]; k++) {
                if (event->param1 == state->buttons[i].hotkey[k]) {
                    // This button was chosen.

                    if (state->buttons[i].flags & B_DRAW) {
                        // Restore the depressed and focused buttons.
                        if (state->buttonDepressed >= 0) {
                            drawButton(&(state->buttons[state->buttonDepressed]), BUTTON_NORMAL, state->dbuf);
                        }
                        if (state->buttonFocused >= 0) {
                            drawButton(&(state->buttons[state->buttonFocused]), BUTTON_NORMAL, state->dbuf);
                        }

                        // If the button likes to flash when keypressed:
                        if (state->buttons[i].flags & B_KEYPRESS_HIGHLIGHT) {
                            // Depress the chosen button.
                            drawButton(&(state->buttons[i]), BUTTON_PRESSED, state->dbuf);

                            // Update the display.
                            overlayDisplayBuffer(state->rbuf, NULL);
                            overlayDisplayBuffer(state->dbuf, NULL);

                            // Wait for a little; then we're done.
                            pauseBrogue(50);
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
        // Update the display.
        overlayDisplayBuffer(state.dbuf, NULL);

        // Get input.
        nextBrogueEvent(&theEvent, true, false, false);

        // Process the input.
        button = processButtonInput(&state, &canceled, &theEvent);

        // Revert the display.
        overlayDisplayBuffer(state.rbuf, NULL);

    } while (button == -1 && !canceled);

    if (returnEvent) {
        *returnEvent = theEvent;
    }

    //overlayDisplayBuffer(dbuf, NULL); // hangs around

    restoreRNG;

    return button;
}

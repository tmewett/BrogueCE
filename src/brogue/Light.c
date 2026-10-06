/*
 *  Light.c
 *  Brogue
 *
 *  Created by Brian Walker on 1/21/09.
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

void logLights(void) {
    short i, j;

    printf("    ");
    for (i=0; i<COLS-2; i++) {
        printf("%i", i % 10);
    }
    printf("\n");
    for( j=0; j<DROWS-2; j++ ) {
        if (j < 10) {
            printf(" ");
        }
        printf("%i: ", j);
        for( i=0; i<DCOLS-2; i++ ) {
            if (tmap[i][j].light[0] == 0) {
                printf(" ");
            } else {
                printf("%i", max(0, tmap[i][j].light[0] / 10 - 1));
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Returns true if any part of the light hit cells that are in the player's field of view.
boolean paintLight(const lightSource *theLight, short x, short y, boolean isMinersLight, boolean maintainShadows) {
    short i, j, k;
    short colorComponents[3], randComponent, lightMultiplier;
    short fadeToPercent, radiusRounded;
    fixpt radius;
    char grid[DCOLS][DROWS];
    boolean dispelShadows, overlappedFieldOfView;

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);

    radius = randClump(theLight->lightRadius) * FP_FACTOR / 100;
    radiusRounded = fp_round(radius);

    randComponent = rand_range(0, theLight->lightColor->rand);
    colorComponents[0] = randComponent + theLight->lightColor->red + rand_range(0, theLight->lightColor->redRand);
    colorComponents[1] = randComponent + theLight->lightColor->green + rand_range(0, theLight->lightColor->greenRand);
    colorComponents[2] = randComponent + theLight->lightColor->blue + rand_range(0, theLight->lightColor->blueRand);

    // the miner's light does not dispel IS_IN_SHADOW,
    // so the player can be in shadow despite casting his own light.
    dispelShadows = !maintainShadows && (colorComponents[0] + colorComponents[1] + colorComponents[2]) > 0;

    fadeToPercent = theLight->radialFadeToPercent;

    // zero out only the relevant rectangle of the grid
    for (i = max(0, x - radiusRounded); i < DCOLS && i < x + radiusRounded; i++) {
        for (j = max(0, y - radiusRounded); j < DROWS && j < y + radiusRounded; j++) {
            grid[i][j] = 0;
        }
    }

    getFOVMask(grid, x, y, radius, T_OBSTRUCTS_VISION, (theLight->passThroughCreatures ? 0 : (HAS_MONSTER | HAS_PLAYER)),
               (!isMinersLight));

    overlappedFieldOfView = false;

    for (i = max(0, x - radiusRounded); i < DCOLS && i < x + radiusRounded; i++) {
        for (j = max(0, y - radiusRounded); j < DROWS && j < y + radiusRounded; j++) {
            if (grid[i][j]) {
                lightMultiplier =   100 - (100 - fadeToPercent) * fp_sqrt(((i-x) * (i-x) + (j-y) * (j-y)) * FP_FACTOR) / radius;
                for (k=0; k<3; k++) {
                    tmap[i][j].light[k] += colorComponents[k] * lightMultiplier / 100;;
                }
                if (dispelShadows) {
                    pmap[i][j].flags &= ~IS_IN_SHADOW;
                }
                if (pmap[i][j].flags & (IN_FIELD_OF_VIEW | ANY_KIND_OF_VISIBLE)) {
                    overlappedFieldOfView = true;
                }
            }
        }
    }

    tmap[x][y].light[0] += colorComponents[0];
    tmap[x][y].light[1] += colorComponents[1];
    tmap[x][y].light[2] += colorComponents[2];

    if (dispelShadows) {
        pmap[x][y].flags &= ~IS_IN_SHADOW;
    }

    return overlappedFieldOfView;
}


// sets miner's light strength and characteristics based on rings of illumination, scrolls of darkness and water submersion
void updateMinersLightRadius(void) {
    fixpt base_fraction, fraction, lightRadius;

    lightRadius = 100 * rogue.minersLightRadius;

    if (rogue.lightMultiplier < 0) {
        lightRadius = lightRadius / (-1 * rogue.lightMultiplier + 1);
    } else {
        lightRadius *= rogue.lightMultiplier;
        lightRadius = max(lightRadius, (rogue.lightMultiplier * 2 + 2) * FP_FACTOR);
    }

    if (player.status[STATUS_DARKNESS]) {
        base_fraction = FP_FACTOR - player.status[STATUS_DARKNESS] * FP_FACTOR / player.maxStatus[STATUS_DARKNESS];
        fraction = (base_fraction * base_fraction / FP_FACTOR) * base_fraction / FP_FACTOR;
        //fraction = (double) pow(1.0 - (((double) player.status[STATUS_DARKNESS]) / player.maxStatus[STATUS_DARKNESS]), 3);
        if (fraction < FP_FACTOR / 20) {
            fraction = FP_FACTOR / 20;
        }
        lightRadius = lightRadius * fraction / FP_FACTOR;
    } else {
        fraction = FP_FACTOR;
    }

    if (lightRadius < 2 * FP_FACTOR) {
        lightRadius = 2 * FP_FACTOR;
    }

    if (rogue.inWater && lightRadius > 3 * FP_FACTOR) {
        lightRadius = max(lightRadius / 2, 3 * FP_FACTOR);
    }

    rogue.minersLight.radialFadeToPercent = 35 + (max(0, min(65, rogue.lightMultiplier * 5)) * fraction) / FP_FACTOR;
    rogue.minersLight.lightRadius.upperBound = rogue.minersLight.lightRadius.lowerBound = clamp(lightRadius / FP_FACTOR, -30000, 30000);
}

static void updateDisplayDetail(void) {
    short i, j;

    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            if (tmap[i][j].light[0] < -10
                && tmap[i][j].light[1] < -10
                && tmap[i][j].light[2] < -10) {

                displayDetail[i][j] = DV_DARK;
            } else if (pmap[i][j].flags & IS_IN_SHADOW) {
                displayDetail[i][j] = DV_UNLIT;
            } else {
                displayDetail[i][j] = DV_LIT;
            }
        }
    }
}

void backUpLighting(short lights[DCOLS][DROWS][3]) {
    short i, j, k;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            for (k=0; k<3; k++) {
                lights[i][j][k] = tmap[i][j].light[k];
            }
        }
    }
}

void restoreLighting(short lights[DCOLS][DROWS][3]) {
    short i, j, k;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            for (k=0; k<3; k++) {
                tmap[i][j].light[k] = lights[i][j][k];
            }
        }
    }
}

static void recordOldLights(void) {
    short i, j, k;
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            for (k=0; k<3; k++) {
                tmap[i][j].oldLight[k] = tmap[i][j].light[k];
            }
        }
    }
}

void updateLighting(void) {
    short i, j, k;
    enum dungeonLayers layer;
    enum tileType tile;

    // Copy Light over oldLight
    recordOldLights();

    // and then zero out Light.
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            for (k=0; k<3; k++) {
                tmap[i][j].light[k] = 0;
            }
            pmap[i][j].flags |= IS_IN_SHADOW;
        }
    }

    // Paint all glowing tiles.
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                tile = pmap[i][j].layers[layer];
                if (tileCatalog[tile].glowLight) {
                    paintLight(&(lightCatalog[tileCatalog[tile].glowLight]), i, j, false, false);
                }
            }
        }
    }

    // Cycle through monsters and paint their lights:
    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *monst = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        if (monst->info.intrinsicLightType) {
            paintLight(&lightCatalog[monst->info.intrinsicLightType], monst->loc.x, monst->loc.y, false, false);
        }
        if (monst->mutationIndex >= 0 && mutationCatalog[monst->mutationIndex].light != NO_LIGHT) {
            paintLight(&lightCatalog[mutationCatalog[monst->mutationIndex].light], monst->loc.x, monst->loc.y, false, false);
        }

        if (monst->status[STATUS_BURNING] && !(monst->info.flags & MONST_FIERY)) {
            paintLight(&lightCatalog[BURNING_CREATURE_LIGHT], monst->loc.x, monst->loc.y, false, false);
        }

        if (monsterRevealed(monst)) {
            paintLight(&lightCatalog[TELEPATHY_LIGHT], monst->loc.x, monst->loc.y, false, true);
        }
    }

    // Also paint telepathy lights for dormant monsters.
    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monsterRevealed(monst)) {
            paintLight(&lightCatalog[TELEPATHY_LIGHT], monst->loc.x, monst->loc.y, false, true);
        }
    }

    updateDisplayDetail();

    // Miner's light:
    paintLight(&rogue.minersLight, player.loc.x, player.loc.y, true, true);

    if (player.status[STATUS_INVISIBLE]) {
        player.info.foreColor = &playerInvisibleColor;
    } else if (playerInDarkness()) {
        player.info.foreColor = &playerInDarknessColor;
    } else if (pmapAt(player.loc)->flags & IS_IN_SHADOW) {
        player.info.foreColor = &playerInShadowColor;
    } else {
        player.info.foreColor = &playerInLightColor;
    }
}

boolean playerInDarkness(void) {
    return (tmapAt(player.loc)->light[0] + 10 < minersLightColor.red
            && tmapAt(player.loc)->light[1] + 10 < minersLightColor.green
            && tmapAt(player.loc)->light[2] + 10 < minersLightColor.blue);
}

#define flarePrecision 1000

flare *newFlare(const lightSource *light, short x, short y, short changePerFrame, short limit) {
    flare *theFlare = malloc(sizeof(flare));
    memset(theFlare, '\0', sizeof(flare));
    theFlare->light = light;
    theFlare->loc.x = x;
    theFlare->loc.y = y;
    theFlare->coeffChangeAmount = changePerFrame;
    if (theFlare->coeffChangeAmount == 0) {
        theFlare->coeffChangeAmount = 1; // no change would mean it lasts forever, which usually breaks things
    }
    theFlare->coeffLimit = limit;
    theFlare->coeff = 100 * flarePrecision;
    theFlare->turnNumber = rogue.absoluteTurnNumber;
    return theFlare;
}

// Creates a new fading flare as described and sticks it into the stack so it will fire at the end of the turn.
void createFlare(short x, short y, enum lightType lightIndex) {
    flare *theFlare;

    theFlare = newFlare(&(lightCatalog[lightIndex]), x, y, -15, 0);

    if (rogue.flareCount >= rogue.flareCapacity) {
        rogue.flareCapacity += 10;
        rogue.flares = realloc(rogue.flares, sizeof(flare *) * rogue.flareCapacity);
    }
    rogue.flares[rogue.flareCount] = theFlare;
    rogue.flareCount++;
}

static boolean flareIsActive(flare *theFlare) {
    const boolean increasing = (theFlare->coeffChangeAmount > 0);
    boolean active = true;

    if (theFlare->turnNumber > 0 && theFlare->turnNumber < rogue.absoluteTurnNumber - 1) {
        active = false;
    }
    if (increasing) {
        if ((short) (theFlare->coeff / flarePrecision) > theFlare->coeffLimit) {
            active = false;
        }
    } else {
        if ((short) (theFlare->coeff / flarePrecision) < theFlare->coeffLimit) {
            active = false;
        }
    }
    return active;
}

// Returns true if the flare is still active; false if it's not.
static boolean updateFlare(flare *theFlare) {
    if (!flareIsActive(theFlare)) {
        return false;
    }
    theFlare->coeff += (theFlare->coeffChangeAmount) * flarePrecision / 10;
    theFlare->coeffChangeAmount = theFlare->coeffChangeAmount * 12 / 10;
    return flareIsActive(theFlare);
}

// Returns whether it overlaps with the field of view.
static boolean drawFlareFrame(flare *theFlare) {
    boolean inView;
    lightSource tempLight = *(theFlare->light);
    color tempColor = *(tempLight.lightColor);

    if (!flareIsActive(theFlare)) {
        return false;
    }
    tempLight.lightRadius.lowerBound = ((long) tempLight.lightRadius.lowerBound) * theFlare->coeff / (flarePrecision * 100);
    tempLight.lightRadius.upperBound = ((long) tempLight.lightRadius.upperBound) * theFlare->coeff / (flarePrecision * 100);
    applyColorScalar(&tempColor, theFlare->coeff / flarePrecision);
    tempLight.lightColor = &tempColor;
    inView = paintLight(&tempLight, theFlare->loc.x, theFlare->loc.y, false, true);

    return inView;
}

// Frees the flares as they expire.
void animateFlares(flare **flares, short count) {
    short lights[DCOLS][DROWS][3];
    boolean inView, fastForward, atLeastOneFlareStillActive;
    short i; // i iterates through the flare list

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);

    backUpLighting(lights);
    fastForward = rogue.trueColorMode || rogue.playbackFastForward;

    do {
        inView = false;
        atLeastOneFlareStillActive = false;
        for (i = 0; i < count; i++) {
            if (flares[i]) {
                if (updateFlare(flares[i])) {
                    atLeastOneFlareStillActive = true;
                    if (drawFlareFrame(flares[i])) {
                        inView = true;
                    }
                } else {
                    free(flares[i]);
                    flares[i] = NULL;
                }
            }
        }
        demoteVisibility();
        updateFieldOfViewDisplay(false, true);
        if (!fastForward && (inView || rogue.playbackOmniscience) && atLeastOneFlareStillActive) {
            fastForward = pauseAnimation(10, PAUSE_BEHAVIOR_DEFAULT);
        }
        recordOldLights();
        restoreLighting(lights);
    } while (atLeastOneFlareStillActive);
    updateFieldOfViewDisplay(false, true);
}

void deleteAllFlares(void) {
    short i;
    for (i=0; i<rogue.flareCount; i++) {
        free(rogue.flares[i]);
    }
    rogue.flareCount = 0;
}

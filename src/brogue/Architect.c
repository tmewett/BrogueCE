/*
 *  Architect.c
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

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

short topBlobMinX, topBlobMinY, blobWidth, blobHeight;

boolean cellHasTerrainFlag(pos loc, unsigned long flagMask) {
    brogueAssert(isPosInMap(loc));
    return ((flagMask) & terrainFlags(loc) ? true : false);
}

boolean cellHasTMFlag(pos loc, unsigned long flagMask) {
    return (flagMask & terrainMechFlags(loc)) ? true : false;
}

boolean cellHasTerrainType(pos p, enum tileType terrain) {
    return (
        pmapAt(p)->layers[DUNGEON] == terrain
        || pmapAt(p)->layers[LIQUID] == terrain
        || pmapAt(p)->layers[SURFACE] == terrain
        || pmapAt(p)->layers[GAS] == terrain
    ) ? true : false;
}

static inline boolean cellIsPassableOrDoor(short x, short y) {
    if (!cellHasTerrainFlag((pos){ x, y }, T_PATHING_BLOCKER)) {
        return true;
    }
    return (
        (cellHasTMFlag((pos){ x, y }, (TM_IS_SECRET | TM_PROMOTES_WITH_KEY | TM_CONNECTS_LEVEL)) && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY))
    );
}

static boolean checkLoopiness(short x, short y) {
    short newX, newY, dir, sdir;
    short numStrings, maxStringLength, currentStringLength;

    if (!(pmap[x][y].flags & IN_LOOP)) {
        return false;
    }

    // find an unloopy neighbor to start on
    for (sdir = 0; sdir < DIRECTION_COUNT; sdir++) {
        newX = x + cDirs[sdir][0];
        newY = y + cDirs[sdir][1];
        if (!coordinatesAreInMap(newX, newY)
            || !(pmap[newX][newY].flags & IN_LOOP)) {
            break;
        }
    }
    if (sdir == 8) { // no unloopy neighbors
        return false; // leave cell loopy
    }

    // starting on this unloopy neighbor, work clockwise and count up (a) the number of strings
    // of loopy neighbors, and (b) the length of the longest such string.
    numStrings = maxStringLength = currentStringLength = 0;
    boolean inString = false;
    for (dir = sdir; dir < sdir + 8; dir++) {
        newX = x + cDirs[dir % 8][0];
        newY = y + cDirs[dir % 8][1];
        if (coordinatesAreInMap(newX, newY) && (pmap[newX][newY].flags & IN_LOOP)) {
            currentStringLength++;
            if (!inString) {
                if (numStrings > 0) {
                    return false; // more than one string here; leave loopy
                }
                numStrings++;
                inString = true;
            }
        } else if (inString) {
            if (currentStringLength > maxStringLength) {
                maxStringLength = currentStringLength;
            }
            currentStringLength = 0;
            inString = false;
        }
    }
    if (inString && currentStringLength > maxStringLength) {
        maxStringLength = currentStringLength;
    }
    if (numStrings == 1 && maxStringLength <= 4) {
        pmap[x][y].flags &= ~IN_LOOP;

        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            newX = x + cDirs[dir][0];
            newY = y + cDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)) {
                checkLoopiness(newX, newY);
            }
        }
        return true;
    } else {
        return false;
    }
}

static void auditLoop(short x, short y, char grid[DCOLS][DROWS]) {
    short dir, newX, newY;
    if (coordinatesAreInMap(x, y)
        && !grid[x][y]
        && !(pmap[x][y].flags & IN_LOOP)) {

        grid[x][y] = true;
        for (dir = 0; dir < DIRECTION_COUNT; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)) {
                auditLoop(newX, newY, grid);
            }
        }
    }
}

// Assumes it is called with respect to a passable (startX, startY), and that the same is not already included in results.
// Returns 10000 if the area included an area machine.
static short floodFillCount(char results[DCOLS][DROWS], char passMap[DCOLS][DROWS], short startX, short startY) {
    short dir, newX, newY, count;

    count = (passMap[startX][startY] == 2 ? 5000 : 1);

    if (pmap[startX][startY].flags & IS_IN_AREA_MACHINE) {
        count = 10000;
    }

    results[startX][startY] = true;

    for(dir=0; dir<4; dir++) {
        newX = startX + nbDirs[dir][0];
        newY = startY + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && passMap[newX][newY]
            && !results[newX][newY]) {

            count += floodFillCount(results, passMap, newX, newY);
        }
    }
    return min(count, 10000);
}

// Rotates around the cell, counting up the number of distinct strings of passable neighbors in a single revolution.
//      Zero means there are no impassable tiles adjacent.
//      One means it is adjacent to a wall.
//      Two means it is in a hallway or something similar.
//      Three means it is the center of a T-intersection or something similar.
//      Four means it is in the intersection of two hallways.
//      Five or more means there is a bug.
short passableArcCount(short x, short y) {
    short arcCount, dir, oldX, oldY, newX, newY;

    brogueAssert(coordinatesAreInMap(x, y));

    arcCount = 0;
    for (dir = 0; dir < DIRECTION_COUNT; dir++) {
        oldX = x + cDirs[(dir + 7) % 8][0];
        oldY = y + cDirs[(dir + 7) % 8][1];
        newX = x + cDirs[dir][0];
        newY = y + cDirs[dir][1];
        // Counts every transition from passable to impassable or vice-versa on the way around the cell:
        if ((coordinatesAreInMap(newX, newY) && cellIsPassableOrDoor(newX, newY))
            != (coordinatesAreInMap(oldX, oldY) && cellIsPassableOrDoor(oldX, oldY))) {
            arcCount++;
        }
    }
    return arcCount / 2; // Since we added one when we entered a wall and another when we left.
}

// locates all loops and chokepoints
void analyzeMap(boolean calculateChokeMap) {
    short i, j, i2, j2, dir, newX, newY, oldX, oldY, passableArcCount, cellCount;
    char grid[DCOLS][DROWS], passMap[DCOLS][DROWS];
    boolean designationSurvives;

    // first find all of the loops
    rogue.staleLoopMap = false;

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) {

                pmap[i][j].flags &= ~IN_LOOP;
                passMap[i][j] = false;
            } else {
                pmap[i][j].flags |= IN_LOOP;
                passMap[i][j] = true;
            }
        }
    }

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            checkLoopiness(i, j);
        }
    }

    // remove extraneous loop markings
    zeroOutGrid(grid);
    auditLoop(0, 0, grid);

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (pmap[i][j].flags & IN_LOOP) {
                designationSurvives = false;
                for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && !grid[newX][newY]
                        && !(pmap[newX][newY].flags & IN_LOOP)) {
                        designationSurvives = true;
                        break;
                    }
                }
                if (!designationSurvives) {
                    grid[i][j] = true;
                    pmap[i][j].flags &= ~IN_LOOP;
                }
            }
        }
    }

    // done finding loops; now flag chokepoints
    for(i=1; i<DCOLS-1; i++) {
        for(j=1; j<DROWS-1; j++) {
            pmap[i][j].flags &= ~IS_CHOKEPOINT;
            if (passMap[i][j] && !(pmap[i][j].flags & IN_LOOP)) {
                passableArcCount = 0;
                for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                    oldX = i + cDirs[(dir + 7) % 8][0];
                    oldY = j + cDirs[(dir + 7) % 8][1];
                    newX = i + cDirs[dir][0];
                    newY = j + cDirs[dir][1];
                    if ((coordinatesAreInMap(newX, newY) && passMap[newX][newY])
                        != (coordinatesAreInMap(oldX, oldY) && passMap[oldX][oldY])) {
                        if (++passableArcCount > 2) {
                            if (!passMap[i-1][j] && !passMap[i+1][j] || !passMap[i][j-1] && !passMap[i][j+1]) {
                                pmap[i][j].flags |= IS_CHOKEPOINT;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    if (calculateChokeMap) {

        // Done finding chokepoints; now create a chokepoint map.

        // The chokepoint map is a number for each passable tile. If the tile is a chokepoint,
        // then the number indicates the number of tiles that would be rendered unreachable if the
        // chokepoint were blocked. If the tile is not a chokepoint, then the number indicates
        // the number of tiles that would be rendered unreachable if the nearest exit chokepoint
        // were blocked.
        // The cost of all of this is one depth-first flood-fill per open point that is adjacent to a chokepoint.

        // Start by setting the chokepoint values really high, and roping off room machines.
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                chokeMap[i][j] = 30000;
                if (pmap[i][j].flags & IS_IN_ROOM_MACHINE) {
                    passMap[i][j] = false;
                }
            }
        }

        // Scan through and find a chokepoint next to an open point.

        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (passMap[i][j] && (pmap[i][j].flags & IS_CHOKEPOINT)) {
                    for (dir=0; dir<4; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && passMap[newX][newY]
                            && !(pmap[newX][newY].flags & IS_CHOKEPOINT)) {
                            // OK, (newX, newY) is an open point and (i, j) is a chokepoint.
                            // Pretend (i, j) is blocked by changing passMap, and run a flood-fill cell count starting on (newX, newY).
                            // Keep track of the flooded region in grid[][].
                            zeroOutGrid(grid);
                            passMap[i][j] = false;
                            cellCount = floodFillCount(grid, passMap, newX, newY);
                            passMap[i][j] = true;

                            // CellCount is the size of the region that would be obstructed if the chokepoint were blocked.
                            // CellCounts less than 4 are not useful, so we skip those cases.

                            if (cellCount >= 4) {
                                // Now, on the chokemap, all of those flooded cells should take the lesser of their current value or this resultant number.
                                for(i2=0; i2<DCOLS; i2++) {
                                    for(j2=0; j2<DROWS; j2++) {
                                        if (grid[i2][j2] && cellCount < chokeMap[i2][j2]) {
                                            chokeMap[i2][j2] = cellCount;
                                            pmap[i2][j2].flags &= ~IS_GATE_SITE;
                                        }
                                    }
                                }

                                // The chokepoint itself should also take the lesser of its current value or the flood count.
                                if (cellCount < chokeMap[i][j]) {
                                    chokeMap[i][j] = cellCount;
                                    pmap[i][j].flags |= IS_GATE_SITE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Add some loops to the otherwise simply connected network of rooms.
static void addLoops(short **grid, short minimumPathingDistance) {
    short newX, newY, oppX, oppY;
    short **pathMap, **costMap;
    short i, d, x, y, sCoord[DCOLS*DROWS];
    const short dirCoords[2][2] = {{1, 0}, {0, 1}};

    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);

    if (D_INSPECT_LEVELGEN) {
        colorOverDungeon(&darkGray);
        hiliteGrid(grid, &white, 100);
    }

    pathMap = allocGrid();
    costMap = allocGrid();
    copyGrid(costMap, grid);
    findReplaceGrid(costMap, 0, 0, PDS_OBSTRUCTION);
    findReplaceGrid(costMap, 1, 30000, 1);

    for (i = 0; i < DCOLS*DROWS; i++) {
        x = sCoord[i]/DROWS;
        y = sCoord[i] % DROWS;
        if (!grid[x][y]) {
            for (d=0; d <= 1; d++) { // Try a horizontal door, and then a vertical door.
                newX = x + dirCoords[d][0];
                oppX = x - dirCoords[d][0];
                newY = y + dirCoords[d][1];
                oppY = y - dirCoords[d][1];
                if (coordinatesAreInMap(newX, newY)
                    && coordinatesAreInMap(oppX, oppY)
                    && grid[newX][newY] == 1
                    && grid[oppX][oppY] == 1) { // If the tile being inspected has floor on both sides,

                    fillGrid(pathMap, 30000);
                    pathMap[newX][newY] = 0;
                    dijkstraScan(pathMap, costMap, false);
                    if (pathMap[oppX][oppY] > minimumPathingDistance) { // and if the pathing distance between the two flanking floor tiles exceeds minimumPathingDistance,
                        grid[x][y] = 2;             // then turn the tile into a doorway.
                        costMap[x][y] = 1;          // (Cost map also needs updating.)
                        if (D_INSPECT_LEVELGEN) {
                            pos p = { x, y };
                            plotCharWithColor(G_CLOSED_DOOR, mapToWindow(p), &black, &green);
                        }
                        break;
                    }
                }
            }
        }
    }
    if (D_INSPECT_LEVELGEN) {
        temporaryMessage("Added secondary connections:", REQUIRE_ACKNOWLEDGMENT);
    }
    freeGrid(pathMap);
    freeGrid(costMap);
}

// Assumes (startX, startY) is in the machine.
// Returns true if everything went well, and false if we ran into a machine component
// that was already there, as we don't want to build a machine around it.
static boolean addTileToMachineInteriorAndIterate(char interior[DCOLS][DROWS], short startX, short startY) {
    short dir, newX, newY;
    boolean goodSoFar = true;

    interior[startX][startY] = true;

    for (dir = 0; dir < 4 && goodSoFar; dir++) {
        newX = startX + nbDirs[dir][0];
        newY = startY + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)) {
            if ((pmap[newX][newY].flags & HAS_ITEM)
                || ((pmap[newX][newY].flags & IS_IN_MACHINE) && !(pmap[newX][newY].flags & IS_GATE_SITE))) {
                // Abort if there's an item in the room.
                // Items haven't been populated yet, so the only way this could happen is if another machine
                // previously placed an item here.
                // Also abort if we're touching another machine at any point other than a gate tile.
                return false;
            }
            if (!interior[newX][newY]
                && chokeMap[newX][newY] <= chokeMap[startX][startY] // don't have to worry about walls since they're all 30000
                && !(pmap[newX][newY].flags & IS_IN_MACHINE)) {
                //goodSoFar = goodSoFar && addTileToMachineInteriorAndIterate(interior, newX, newY);
                if (goodSoFar) {
                    goodSoFar = addTileToMachineInteriorAndIterate(interior, newX, newY);
                }
            }
        }
    }
    return goodSoFar;
}

static void copyMap(pcell from[DCOLS][DROWS], pcell to[DCOLS][DROWS]) {
    short i, j;

    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            to[i][j] = from[i][j];
        }
    }
}

static boolean itemIsADuplicate(item *theItem, item **spawnedItems, short itemCount) {
    short i;
    if (theItem->category & (STAFF | WAND | POTION | SCROLL | RING | WEAPON | ARMOR | CHARM)) {
        for (i = 0; i < itemCount; i++) {
            if (spawnedItems[i]->category == theItem->category
                && spawnedItems[i]->kind == theItem->kind) {

                return true;
            }
        }
    }
    return false;
}

static boolean blueprintQualifies(short i, unsigned long requiredMachineFlags) {
    if (blueprintCatalog[i].depthRange[0] > rogue.depthLevel
        || blueprintCatalog[i].depthRange[1] < rogue.depthLevel
                // Must have the required flags:
        || (~(blueprintCatalog[i].flags) & requiredMachineFlags)
                // May NOT have BP_ADOPT_ITEM unless that flag is required:
        || (blueprintCatalog[i].flags & BP_ADOPT_ITEM & ~requiredMachineFlags)
                // May NOT have BP_VESTIBULE unless that flag is required:
        || (blueprintCatalog[i].flags & BP_VESTIBULE & ~requiredMachineFlags)) {

        return false;
    }
    return true;
}

static void abortItemsAndMonsters(item *spawnedItems[MACHINES_BUFFER_LENGTH], creature *spawnedMonsters[MACHINES_BUFFER_LENGTH]) {
    short i, j;

    for (i=0; i<MACHINES_BUFFER_LENGTH && spawnedItems[i]; i++) {
        removeItemFromChain(spawnedItems[i], floorItems);
        removeItemFromChain(spawnedItems[i], packItems); // just in case; can't imagine why this would arise.
        for (j=0; j<MACHINES_BUFFER_LENGTH && spawnedMonsters[j]; j++) {
            // Remove the item from spawned monsters, so it doesn't get double-freed when the creature is killed below.
            if (spawnedMonsters[j]->carriedItem == spawnedItems[i]) {
                spawnedMonsters[j]->carriedItem = NULL;
                break;
            }
        }
        deleteItem(spawnedItems[i]);
        spawnedItems[i] = NULL;
    }
    for (i=0; i<MACHINES_BUFFER_LENGTH && spawnedMonsters[i]; i++) {
        killCreature(spawnedMonsters[i], true);
        spawnedMonsters[i] = NULL;
    }
}

static boolean cellIsFeatureCandidate(short x, short y,
                               short originX, short originY,
                               short distanceBound[2],
                               char interior[DCOLS][DROWS],
                               char occupied[DCOLS][DROWS],
                               char viewMap[DCOLS][DROWS],
                               short **distanceMap,
                               short machineNumber,
                               unsigned long featureFlags,
                               unsigned long bpFlags) {
    short newX, newY, dir, distance;

    // No building in the hallway if it's prohibited.
    // This check comes before the origin check, so an area machine will fail altogether
    // if its origin is in a hallway and the feature that must be built there does not permit as much.
    if ((featureFlags & MF_NOT_IN_HALLWAY)
        && passableArcCount(x, y) > 1) {
        return false;
    }

    // No building along the perimeter of the level if it's prohibited.
    if ((featureFlags & MF_NOT_ON_LEVEL_PERIMETER)
        && (x == 0 || x == DCOLS - 1 || y == 0 || y == DROWS - 1)) {
        return false;
    }

    // The origin is a candidate if the feature is flagged to be built at the origin.
    // If it's a room, the origin (i.e. doorway) is otherwise NOT a candidate.
    if (featureFlags & MF_BUILD_AT_ORIGIN) {
        return ((x == originX && y == originY) ? true : false);
    } else if ((bpFlags & BP_ROOM) && x == originX && y == originY) {
        return false;
    }

    // No building in another feature's personal space!
    if (occupied[x][y]) {
        return false;
    }

    // Must be in the viewmap if the appropriate flag is set.
    if ((featureFlags & (MF_IN_VIEW_OF_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN))
        && !viewMap[x][y]) {
        return false;
    }

    // Do a distance check if the feature requests it.
    if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // Distance is calculated for walls too.
        distance = 10000;
        for (dir = 0; dir < 4; dir++) {
            newX = x + nbDirs[dir][0];
            newY = y + nbDirs[dir][1];
            if (coordinatesAreInMap(newX, newY)
                && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
                && distance > distanceMap[newX][newY] + 1) {

                distance = distanceMap[newX][newY] + 1;
            }
        }
    } else {
        distance = distanceMap[x][y];
    }

    if (distance > distanceBound[1]     // distance exceeds max
        || distance < distanceBound[0]) {   // distance falls short of min
        return false;
    }
    if (featureFlags & MF_BUILD_IN_WALLS) {             // If we're supposed to build in a wall...
        if (!interior[x][y]
            && (pmap[x][y].machineNumber == 0 || pmap[x][y].machineNumber == machineNumber)
            && cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // ...and this location is a wall that's not already machined...
            for (dir=0; dir<4; dir++) {
                newX = x + nbDirs[dir][0];
                newY = y + nbDirs[dir][1];
                if (coordinatesAreInMap(newX, newY)     // ...and it's next to an interior spot or permitted elsewhere and next to passable spot...
                    && ((interior[newX][newY] && !(newX==originX && newY==originY))
                        || ((featureFlags & MF_BUILD_ANYWHERE_ON_LEVEL)
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)
                            && pmap[newX][newY].machineNumber == 0))) {
                    return true;                        // ...then we're golden!
                }
            }
        }
        return false;                                   // Otherwise, no can do.
    } else if (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_PASSABILITY)) { // Can't build in a wall unless instructed to do so.
        return false;
    } else if (featureFlags & MF_BUILD_ANYWHERE_ON_LEVEL) {
        if ((featureFlags & MF_GENERATE_ITEM)
            && (cellHasTerrainFlag((pos){ x, y }, T_OBSTRUCTS_ITEMS | T_PATHING_BLOCKER) || (pmap[x][y].flags & (IS_CHOKEPOINT | IN_LOOP | IS_IN_MACHINE)))) {
            return false;
        } else {
            return !(pmap[x][y].flags & IS_IN_MACHINE);
        }
    } else if (interior[x][y]) {
        return true;
    }
    return false;
}


static void addLocationToKey(item *theItem, short x, short y, boolean disposableHere) {
    short i;

    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++);
    theItem->keyLoc[i].loc = (pos){ x, y };
    theItem->keyLoc[i].disposableHere = disposableHere;
}

static void addMachineNumberToKey(item *theItem, short machineNumber, boolean disposableHere) {
    short i;

    for (i=0; i < KEY_ID_MAXIMUM && (theItem->keyLoc[i].loc.x || theItem->keyLoc[i].machine); i++);
    theItem->keyLoc[i].machine = machineNumber;
    theItem->keyLoc[i].disposableHere = disposableHere;
}

static void expandMachineInterior(char interior[DCOLS][DROWS], short minimumInteriorNeighbors) {
    boolean madeChange;
    short nbcount, newX, newY, i, j, layer;
    enum directions dir;

    do {
        madeChange = false;
        for(i=1; i<DCOLS-1; i++) {
            for(j=1; j < DROWS-1; j++) {
                if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                    && pmap[i][j].machineNumber == 0) {

                    // Count up the number of interior open neighbors out of eight:
                    for (nbcount = dir = 0; dir < DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (interior[newX][newY]
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)) {
                            nbcount++;
                        }
                    }
                    if (nbcount >= minimumInteriorNeighbors) {
                        // Make sure zero exterior open/machine neighbors out of eight:
                        for (nbcount = dir = 0; dir < DIRECTION_COUNT; dir++) {
                            newX = i + nbDirs[dir][0];
                            newY = j + nbDirs[dir][1];
                            if (!interior[newX][newY]
                                && (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY) || pmap[newX][newY].machineNumber != 0)) {
                                nbcount++;
                                break;
                            }
                        }
                        if (!nbcount) {
                            // Eliminate this obstruction; welcome its location into the machine.
                            madeChange = true;
                            interior[i][j] = true;
                            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                                if (tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER) {
                                    pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                                }
                            }
                            for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                                newX = i + nbDirs[dir][0];
                                newY = j + nbDirs[dir][1];
                                if (pmap[newX][newY].layers[DUNGEON] == GRANITE) {
                                    pmap[newX][newY].layers[DUNGEON] = WALL;
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (madeChange);

    // Clear doors and secret doors out of the interior of the machine.
    for(i=1; i<DCOLS-1; i++) {
        for(j=1; j < DROWS-1; j++) {
            if (interior[i][j]
                && (pmap[i][j].layers[DUNGEON] == DOOR || pmap[i][j].layers[DUNGEON] == SECRET_DOOR)) {

                pmap[i][j].layers[DUNGEON] = FLOOR;
            }
        }
    }
}

static boolean fillInteriorForVestibuleMachine(char interior[DCOLS][DROWS], short bp, short originX, short originY) {
    short **distanceMap, **costMap, qualifyingTileCount, totalFreq, sRows[DROWS], sCols[DCOLS], i, j, k;
    boolean success = true;

    zeroOutGrid(interior);

    distanceMap = allocGrid();
    fillGrid(distanceMap, 30000);
    distanceMap[originX][originY] = 0;

    costMap = allocGrid();
    populateGenericCostMap(costMap);
    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (costMap[i][j] == 1 && (pmap[i][j].flags & IS_IN_MACHINE)) { //pmap[i][j].machineNumber) {
                costMap[i][j] = PDS_FORBIDDEN;
            }
        }
    }
    costMap[originX][originY] = 1;
    dijkstraScan(distanceMap, costMap, false);
    freeGrid(costMap);

    qualifyingTileCount = 0; // Keeps track of how many interior cells we've added.
    totalFreq = rand_range(blueprintCatalog[bp].roomSize[0], blueprintCatalog[bp].roomSize[1]); // Keeps track of the goal size.

    fillSequentialList(sCols, DCOLS);
    shuffleList(sCols, DCOLS);
    fillSequentialList(sRows, DROWS);
    shuffleList(sRows, DROWS);

    for (k=0; k<1000 && qualifyingTileCount < totalFreq; k++) {
        for(i=0; i<DCOLS && qualifyingTileCount < totalFreq; i++) {
            for(j=0; j<DROWS && qualifyingTileCount < totalFreq; j++) {
                if (distanceMap[sCols[i]][sRows[j]] == k) {
                    interior[sCols[i]][sRows[j]] = true;
                    qualifyingTileCount++;

                    if (pmap[sCols[i]][sRows[j]].flags & HAS_ITEM) {
                        // Abort if we've engulfed another machine's item.
                        success = false;
                        qualifyingTileCount = totalFreq; // This is a hack to drop out of these three for-loops.
                    }
                }
            }
        }
    }

    // Now make sure the interior map satisfies the machine's qualifications.
    if ((blueprintCatalog[bp].flags & BP_TREAT_AS_BLOCKING)
        && levelIsDisconnectedWithBlockingMap(interior, false)) {
        success = false;
    } else if ((blueprintCatalog[bp].flags & BP_REQUIRE_BLOCKING)
               && levelIsDisconnectedWithBlockingMap(interior, true) < 100) {
        success = false;
    }
    freeGrid(distanceMap);
    return success;
}

static void redesignInterior(char interior[DCOLS][DROWS], short originX, short originY, short theProfileIndex) {
    short i, j, n, newX, newY;
    enum directions dir;
    pos orphanList[20];
    short orphanCount = 0;
    short **grid, **pathingGrid, **costGrid;
    grid = allocGrid();

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (interior[i][j]) {
                if (i == originX && j == originY) {
                    grid[i][j] = 1; // All rooms must grow from this space.
                } else {
                    grid[i][j] = 0; // Other interior squares are fair game for placing rooms.
                }
            } else if (cellIsPassableOrDoor(i, j)) {
                grid[i][j] = 1; // Treat existing level as already built (though shielded by a film of -1s).
                for (dir = 0; dir < 4; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && interior[newX][newY]
                        && (newX != originX || newY != originY)) {

                        orphanList[orphanCount] = (pos){ .x = newX, .y = newY };
                        orphanCount++;
                        grid[i][j] = -1; // Treat the orphaned door as off limits.

                        break;
                    }
                }
            } else {
                grid[i][j] = -1; // Exterior spaces are off limits.
            }
        }
    }
    attachRooms(grid, &dungeonProfileCatalog[theProfileIndex], 40, 40);

    // Connect to preexisting rooms that were orphaned (mostly preexisting machine rooms).
    if (orphanCount > 0) {
        pathingGrid = allocGrid();
        costGrid = allocGrid();
        for (n = 0; n < orphanCount; n++) {

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                copyGrid(pathingGrid, grid);
                findReplaceGrid(pathingGrid, -1, -1, 0);
                hiliteGrid(pathingGrid, &green, 50);
                plotCharWithColor('X', mapToWindow(orphanList[n]), &black, &orange);
                temporaryMessage("Orphan detected:", REQUIRE_ACKNOWLEDGMENT);
            }

            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (interior[i][j]) {
                        if (grid[i][j] > 0) {
                            pathingGrid[i][j] = 0;
                            costGrid[i][j] = 1;
                        } else {
                            pathingGrid[i][j] = 30000;
                            costGrid[i][j] = 1;
                        }
                    } else {
                        pathingGrid[i][j] = 30000;
                        costGrid[i][j] = PDS_OBSTRUCTION;
                    }
                }
            }
            dijkstraScan(pathingGrid, costGrid, false);

            i = orphanList[n].x;
            j = orphanList[n].y;
            while (pathingGrid[i][j] > 0) {
                for (dir = 0; dir < 4; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];

                    if (coordinatesAreInMap(newX, newY)
                        && pathingGrid[newX][newY] < pathingGrid[i][j]) {

                        grid[i][j] = 1;
                        i = newX;
                        j = newY;
                        break;
                    }
                }
                brogueAssert(dir < 4);
                if (D_INSPECT_MACHINES) {
                    dumpLevelToScreen();
                    displayGrid(pathingGrid);
                    pos p = { i, j };
                    plotCharWithColor('X', mapToWindow(p), &black, &orange);
                    temporaryMessage("Orphan connecting:", REQUIRE_ACKNOWLEDGMENT);
                }
            }
        }
        freeGrid(pathingGrid);
        freeGrid(costGrid);
    }

    addLoops(grid, 10);
    for(i=0; i<DCOLS; i++) {
        for(j=0; j<DROWS; j++) {
            if (interior[i][j]) {
                if (grid[i][j] >= 0) {
                    pmap[i][j].layers[SURFACE] = pmap[i][j].layers[GAS] = NOTHING;
                }
                if (grid[i][j] == 0) {
                    pmap[i][j].layers[DUNGEON] = GRANITE;
                    interior[i][j] = false;
                }
                if (grid[i][j] >= 1) {
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                }
            }
        }
    }
    freeGrid(grid);
}

static void prepareInteriorWithMachineFlags(char interior[DCOLS][DROWS], short originX, short originY, unsigned long flags, short dungeonProfileIndex) {
    short i, j, newX, newY;
    enum dungeonLayers layer;
    enum directions dir;

    // If requested, clear and expand the room as far as possible until either it's convex or it bumps into surrounding rooms
    if (flags & BP_MAXIMIZE_INTERIOR) {
        expandMachineInterior(interior, 1);
    } else if (flags & BP_OPEN_INTERIOR) {
        expandMachineInterior(interior, 4);
    }

    // If requested, cleanse the interior -- no interesting terrain allowed.
    if (flags & BP_PURGE_INTERIOR) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                        pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                    }
                }
            }
        }
    }

    // If requested, purge pathing blockers -- no traps allowed.
    if (flags & BP_PURGE_PATHING_BLOCKERS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                        if (tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER) {
                            pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                        }
                    }
                }
            }
        }
    }

    // If requested, purge the liquid layer in the interior -- no liquids allowed.
    if (flags & BP_PURGE_LIQUIDS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]) {
                    pmap[i][j].layers[LIQUID] = NOTHING;
                }
            }
        }
    }

    // Surround with walls if requested.
    if (flags & BP_SURROUND_WITH_WALLS) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j] && !(pmap[i][j].flags & IS_GATE_SITE)) {
                    for (dir=0; dir< DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && !interior[newX][newY]
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)
                            && !(pmap[newX][newY].flags & IS_GATE_SITE)
                            && !pmap[newX][newY].machineNumber
                            && cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)) {
                            for (layer=0; layer<NUMBER_TERRAIN_LAYERS; layer++) {
                                pmap[newX][newY].layers[layer] = (layer == DUNGEON ? WALL : 0);
                            }
                        }
                    }
                }
            }
        }
    }

    // Completely clear the interior, fill with granite, and cut entirely new rooms into it from the gate site.
    // Then zero out any portion of the interior that is still wall.
    if (flags & BP_REDESIGN_INTERIOR) {
        redesignInterior(interior, originX, originY, dungeonProfileIndex);
    }

    // Reinforce surrounding tiles and interior tiles if requested to prevent tunneling in or through.
    if (flags & BP_IMPREGNABLE) {
        for(i=0; i<DCOLS; i++) {
            for(j=0; j<DROWS; j++) {
                if (interior[i][j]
                    && !(pmap[i][j].flags & IS_GATE_SITE)) {

                    pmap[i][j].flags |= IMPREGNABLE;
                    for (dir=0; dir< DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && !interior[newX][newY]
                            && !(pmap[newX][newY].flags & IS_GATE_SITE)) {

                            pmap[newX][newY].flags |= IMPREGNABLE;
                        }
                    }
                }
            }
        }
    }
}

typedef struct machineData {
    // Our boolean grids:
    char interior[DCOLS][DROWS];    // This is the master grid for the machine. All area inside the machine are set to true.
    char occupied[DCOLS][DROWS];    // This keeps track of what is within the personal space of a previously built feature in the same machine.
    char candidates[DCOLS][DROWS];  // This is calculated at the start of each feature, and is true where that feature is eligible for building.
    char blockingMap[DCOLS][DROWS]; // Used during terrain/DF placement in features that are flagged not to tolerate blocking, to see if they block.
    char viewMap[DCOLS][DROWS];     // Used for features with MF_IN_VIEW_OF_ORIGIN, to calculate which cells are in view of the origin.

    pcell levelBackup[DCOLS][DROWS];

    item *spawnedItems[MACHINES_BUFFER_LENGTH];
    item *spawnedItemsSub[MACHINES_BUFFER_LENGTH];
    creature *spawnedMonsters[MACHINES_BUFFER_LENGTH];
    creature *spawnedMonstersSub[MACHINES_BUFFER_LENGTH];

    pos gateCandidates[50];
    short distances[100];
    short sRows[DROWS];
    short sCols[DCOLS];
} machineData;

// Returns true if the machine got built; false if it was aborted.
// If empty array parentSpawnedItems or parentSpawnedMonsters is given, will pass those back for deletion if necessary.
boolean buildAMachine(enum machineTypes bp,
                      short originX, short originY,
                      unsigned long requiredMachineFlags,
                      item *adoptiveItem,
                      item *parentSpawnedItems[MACHINES_BUFFER_LENGTH],
                      creature *parentSpawnedMonsters[MACHINES_BUFFER_LENGTH]) {

    short totalFreq, instance, instanceCount = 0,
        itemCount, monsterCount, qualifyingTileCount,
        **distanceMap = NULL,
        personalSpace, locationFailsafe,
        machineNumber;

    const unsigned long alternativeFlags[2] = {MF_ALTERNATIVE, MF_ALTERNATIVE_2};

    boolean DFSucceeded, terrainSucceeded, generateEverywhere,
        tryAgain, success = false, skipFeature[20];

    creature *monst = NULL, *torchBearer = NULL, *leader = NULL;

    item *theItem = NULL, *torch = NULL;

    const machineFeature *feature;

    machineData *p = malloc(sizeof(machineData));

    memset(p, 0, sizeof(machineData));

    const boolean chooseBP = (((signed short) bp) <= 0 ? true : false);

    const boolean chooseLocation = (originX <= 0 || originY <= 0 ? true : false);

    int failsafe = 10;
    do {
        tryAgain = false;
        if (--failsafe <= 0) {
            if (distanceMap) {
                freeGrid(distanceMap);
            }
            if (D_MESSAGE_MACHINE_GENERATION) {
                if (chooseBP || chooseLocation) {
                    printf("\nDepth %i: Failed to build a machine; gave up after 10 unsuccessful attempts to find a suitable blueprint and/or location.",
                           rogue.depthLevel);
                } else {
                    printf("\nDepth %i: Failed to build a machine; requested blueprint %i:%s and location did not work.",
                           rogue.depthLevel, bp, blueprintCatalog[bp].name);
                }
            }
            free(p);
            return false;
        }

        if (chooseBP) { // If no blueprint is given, then pick one:

            // First, choose the blueprint. We choose from among blueprints
            // that have the required blueprint flags and that satisfy the depth requirements.
            totalFreq = 0;
            for (int i=1; i<gameConst->numberBlueprints; i++) {
                if (blueprintQualifies(i, requiredMachineFlags)) {
                    totalFreq += blueprintCatalog[i].frequency;
                }
            }

            if (!totalFreq) { // If no suitable blueprints are in the library, fail.
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a machine because no suitable blueprints were available.",
                             rogue.depthLevel);
                free(p);
                return false;
            }

            // Pick from among the suitable blueprints.
            int randIndex = rand_range(1, totalFreq);
            for (int i=1; i<gameConst->numberBlueprints; i++) {
                if (blueprintQualifies(i, requiredMachineFlags)) {
                    if (randIndex <= blueprintCatalog[i].frequency) {
                        bp = i;
                        break;
                    } else {
                        randIndex -= blueprintCatalog[i].frequency;
                    }
                }
            }

            // If we don't have a blueprint yet, something went wrong.
            brogueAssert(bp>0);
        }

        // Find a location and map out the machine interior.
        if (blueprintCatalog[bp].flags & BP_ROOM) {
            // If it's a room machine, count up the gates of appropriate
            // choke size and remember where they are. The origin of the room will be the gate location.
            zeroOutGrid(p->interior);

            if (chooseLocation) {
                analyzeMap(true); // Make sure the chokeMap is up to date.
                totalFreq = 0;
                for(int i=0; i<DCOLS; i++) {
                    for(int j=0; j<DROWS && totalFreq < 50; j++) {
                        if ((pmap[i][j].flags & IS_GATE_SITE)
                            && !(pmap[i][j].flags & IS_IN_MACHINE)
                            && chokeMap[i][j] >= blueprintCatalog[bp].roomSize[0]
                            && chokeMap[i][j] <= blueprintCatalog[bp].roomSize[1]) {

                            //DEBUG printf("\nDepth %i: Gate site qualified with interior size of %i.", rogue.depthLevel, chokeMap[i][j]);
                            p->gateCandidates[totalFreq] = (pos){ .x = i, .y = j };
                            totalFreq++;
                        }
                    }
                }

                if (totalFreq) {
                    // Choose the gate.
                    const int randIndex = rand_range(0, totalFreq - 1);
                    originX = p->gateCandidates[randIndex].x;
                    originY = p->gateCandidates[randIndex].y;
                } else {
                    // If no suitable sites, abort.
                    if (distanceMap) {
                        freeGrid(distanceMap);
                    }
                    if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a machine; there was no eligible door candidate for the chosen room machine from blueprint %i:%s.",
                                 rogue.depthLevel,
                                 bp,
                                 blueprintCatalog[bp].name);
                    free(p);
                    return false;
                }
            }

            // Now map out the interior into interior[][].
            // Start at the gate location and do a depth-first floodfill to grab all adjoining tiles with the
            // same or lower choke value, ignoring any tiles that are already part of a machine.
            // If we get false from this, try again. If we've tried too many times already, abort.
            tryAgain = !addTileToMachineInteriorAndIterate(p->interior, originX, originY);
        } else if (blueprintCatalog[bp].flags & BP_VESTIBULE) {
            if (chooseLocation) {
                // Door machines must have locations passed in. We can't pick one ourselves.
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: ERROR: Attempted to build a door machine from blueprint %i:%s without a location being provided.",
                             rogue.depthLevel,
                             bp,
                             blueprintCatalog[bp].name);
                free(p);
                return false;
            }
            if (!fillInteriorForVestibuleMachine(p->interior, bp, originX, originY)) {
                if (distanceMap) {
                    freeGrid(distanceMap);
                }
                if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to build a door machine from blueprint %i:%s; not enough room.",
                             rogue.depthLevel,
                             bp,
                             blueprintCatalog[bp].name);
                free(p);
                return false;
            }
        } else {
            // Find a location and map out the interior for a non-room machine.
            // The strategy here is simply to pick a random location on the map,
            // expand it along a pathing map by one space in all directions until the size reaches
            // the chosen size, and then make sure the resulting space qualifies.
            // If not, try again. If we've tried too many times already, abort.

            locationFailsafe = 10;
            do {
                zeroOutGrid(p->interior);
                tryAgain = false;

                if (chooseLocation) {
                    // Pick a random origin location.
                    pos originLoc = { originX, originY };
                    randomMatchingLocation(&originLoc, FLOOR, NOTHING, -1);
                    originX = originLoc.x;
                    originY = originLoc.y;
                }

                if (!distanceMap) {
                    distanceMap = allocGrid();
                }
                fillGrid(distanceMap, 0);
                calculateDistances(distanceMap, originX, originY, T_PATHING_BLOCKER, NULL, true, false);
                qualifyingTileCount = 0; // Keeps track of how many interior cells we've added.
                totalFreq = rand_range(blueprintCatalog[bp].roomSize[0], blueprintCatalog[bp].roomSize[1]); // Keeps track of the goal size.

                fillSequentialList(p->sCols, DCOLS);
                shuffleList(p->sCols, DCOLS);
                fillSequentialList(p->sRows, DROWS);
                shuffleList(p->sRows, DROWS);

                for (int k=0; k<1000 && qualifyingTileCount < totalFreq; k++) {
                    for(int i=0; i<DCOLS && qualifyingTileCount < totalFreq; i++) {
                        for(int j=0; j<DROWS && qualifyingTileCount < totalFreq; j++) {
                            if (distanceMap[p->sCols[i]][p->sRows[j]] == k) {
                                p->interior[p->sCols[i]][p->sRows[j]] = true;
                                qualifyingTileCount++;

                                if (pmap[p->sCols[i]][p->sRows[j]].flags & (HAS_ITEM | HAS_MONSTER | IS_IN_MACHINE)) {
                                    // Abort if we've entered another machine or engulfed another machine's item or monster.
                                    tryAgain = true;
                                    qualifyingTileCount = totalFreq; // This is a hack to drop out of these three for-loops.
                                }
                            }
                        }
                    }
                }

                // Now make sure the interior map satisfies the machine's qualifications.
                if ((blueprintCatalog[bp].flags & BP_TREAT_AS_BLOCKING)
                    && levelIsDisconnectedWithBlockingMap(p->interior, false)) {
                    tryAgain = true;
                } else if ((blueprintCatalog[bp].flags & BP_REQUIRE_BLOCKING)
                           && levelIsDisconnectedWithBlockingMap(p->interior, true) < 100) {
                    tryAgain = true; // BP_REQUIRE_BLOCKING needs some work to make sure the disconnect is interesting.
                }
                // If locationFailsafe runs out, tryAgain will still be true, and we'll try a different machine.
                // If we're not choosing the blueprint, then don't bother with the locationFailsafe; just use the higher-level failsafe.
            } while (chooseBP && tryAgain && --locationFailsafe);
        }

        // If something went wrong, but we haven't been charged with choosing blueprint OR location,
        // then there is nothing to try again, so just fail.
        if (tryAgain && !chooseBP && !chooseLocation) {
            if (distanceMap) {
                freeGrid(distanceMap);
            }
            free(p);
            return false;
        }

        // Now loop if necessary.
    } while (tryAgain);

    // This is the point of no return. Back up the level so it can be restored if we have to abort this machine after this point.
    copyMap(pmap, p->levelBackup);

    // Perform any transformations to the interior indicated by the blueprint flags, including expanding the interior if requested.
    prepareInteriorWithMachineFlags(p->interior, originX, originY, blueprintCatalog[bp].flags, blueprintCatalog[bp].dungeonProfileType);

    // If necessary, label the interior as IS_IN_AREA_MACHINE or IS_IN_ROOM_MACHINE and mark down the number.
    machineNumber = ++rogue.machineNumber; // Reserve this machine number, starting with 1.
    for(int i=0; i<DCOLS; i++) {
        for(int j=0; j<DROWS; j++) {
            if (p->interior[i][j]) {
                pmap[i][j].flags |= ((blueprintCatalog[bp].flags & BP_ROOM) ? IS_IN_ROOM_MACHINE : IS_IN_AREA_MACHINE);
                pmap[i][j].machineNumber = machineNumber;
                // also clear any secret doors, since they screw up distance mapping and aren't fun inside machines
                if (pmap[i][j].layers[DUNGEON] == SECRET_DOOR) {
                    pmap[i][j].layers[DUNGEON] = DOOR;
                }
                // Clear wired tiles in case we stole them from another machine.
                for (int layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    if (tileCatalog[pmap[i][j].layers[layer]].mechFlags & (TM_IS_WIRED | TM_IS_CIRCUIT_BREAKER)) {
                        pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                    }
                }
            }
        }
    }

//  DEBUG printf("\n\nWorking on blueprint %i, with origin at (%i, %i). Here's the initial interior map:", bp, originX, originY);
//  DEBUG logBuffer(interior);

    // Calculate the distance map (so that features that want to be close to or far from the origin can be placed accordingly)
    // and figure out the 33rd and 67th percentiles for features that want to be near or far from the origin.
    if (!distanceMap) {
        distanceMap = allocGrid();
    }
    fillGrid(distanceMap, 0);
    calculateDistances(distanceMap, originX, originY, T_PATHING_BLOCKER, NULL, true, true);
    qualifyingTileCount = 0;
    for (int i=0; i<100; i++) {
        p->distances[i] = 0;
    }
    for(int i=0; i<DCOLS; i++) {
        for(int j=0; j<DROWS; j++) {
            if (p->interior[i][j]
                && distanceMap[i][j] < 100) {
                p->distances[distanceMap[i][j]]++; // create a histogram of distances -- poor man's sort function
                qualifyingTileCount++;
            }
        }
    }
    int distance25 = (int) (qualifyingTileCount / 4);
    int distance75 = (int) (3 * qualifyingTileCount / 4);
    for (int i=0; i<100; i++) {
        if (distance25 <= p->distances[i]) {
            distance25 = i;
            break;
        } else {
            distance25 -= p->distances[i];
        }
    }
    for (int i=0; i<100; i++) {
        if (distance75 <= p->distances[i]) {
            distance75 = i;
            break;
        } else {
            distance75 -= p->distances[i];
        }
    }
    //DEBUG printf("\nDistances calculated: 33rd percentile of distance is %i, and 67th is %i.", distance25, distance75);

    // Now decide which features will be skipped -- of the features marked MF_ALTERNATIVE, skip all but one, chosen randomly.
    // Then repeat and do the same with respect to MF_ALTERNATIVE_2, to provide up to two independent sets of alternative features per machine.

    for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
        skipFeature[i] = false;
    }
    for (int j = 0; j <= 1; j++) {
        totalFreq = 0;
        for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
            if (blueprintCatalog[bp].feature[i].flags & alternativeFlags[j]) {
                skipFeature[i] = true;
                totalFreq++;
            }
        }
        if (totalFreq > 0) {
            int randIndex = rand_range(1, totalFreq);
            for (int i=0; i<blueprintCatalog[bp].featureCount; i++) {
                if (blueprintCatalog[bp].feature[i].flags & alternativeFlags[j]) {
                    if (randIndex == 1) {
                        skipFeature[i] = false; // This is the alternative that gets built. The rest do not.
                        break;
                    } else {
                        randIndex--;
                    }
                }
            }
        }
    }

    // Keep track of all monsters and items that we spawn -- if we abort, we have to go back and delete them all.
    itemCount = monsterCount = 0;

    // Zero out occupied[][], and use it to keep track of the personal space around each feature that gets placed.
    zeroOutGrid(p->occupied);

    // Now tick through the features and build them.
    for (int feat = 0; feat < blueprintCatalog[bp].featureCount; feat++) {

        if (skipFeature[feat]) {
            continue; // Skip the alternative features that were not selected for building.
        }

        feature = &(blueprintCatalog[bp].feature[feat]);

        // Figure out the distance bounds.
        short distanceBound[2] = { 0, 10000 };
        if (feature->flags & MF_NEAR_ORIGIN) {
            distanceBound[1] = distance25;
        }
        if (feature->flags & MF_FAR_FROM_ORIGIN) {
            distanceBound[0] = distance75;
        }

        if (feature->flags & (MF_IN_VIEW_OF_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN)) {
            zeroOutGrid(p->viewMap);
            if (feature->flags & MF_IN_PASSABLE_VIEW_OF_ORIGIN) {
                getFOVMask(p->viewMap, originX, originY, max(DCOLS, DROWS) * FP_FACTOR, T_PATHING_BLOCKER, 0, false);
            } else {
                getFOVMask(p->viewMap, originX, originY, max(DCOLS, DROWS) * FP_FACTOR, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION), 0, false);
            }
            p->viewMap[originX][originY] = true;

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                hiliteCharGrid(p->viewMap, &omniscienceColor, 75);
                temporaryMessage("Showing visibility.", REQUIRE_ACKNOWLEDGMENT);
            }
        }

        do { // If the MF_REPEAT_UNTIL_NO_PROGRESS flag is set, repeat until we fail to build the required number of instances.

            // Make a master map of candidate locations for this feature.
            qualifyingTileCount = 0;
            for(int i=0; i<DCOLS; i++) {
                for(int j=0; j<DROWS; j++) {
                    if (cellIsFeatureCandidate(i, j,
                                               originX, originY,
                                               distanceBound,
                                               p->interior, p->occupied, p->viewMap, distanceMap,
                                               machineNumber, feature->flags, blueprintCatalog[bp].flags)) {
                        qualifyingTileCount++;
                        p->candidates[i][j] = true;
                    } else {
                        p->candidates[i][j] = false;
                    }
                }
            }

            if (D_INSPECT_MACHINES) {
                dumpLevelToScreen();
                hiliteCharGrid(p->occupied, &red, 75);
                hiliteCharGrid(p->candidates, &green, 75);
                hiliteCharGrid(p->interior, &blue, 75);
                temporaryMessage("Indicating: Occupied (red); Candidates (green); Interior (blue).", REQUIRE_ACKNOWLEDGMENT);
            }

            if (feature->flags & MF_EVERYWHERE & ~MF_BUILD_AT_ORIGIN) {
                // Generate everywhere that qualifies -- instead of randomly picking tiles, keep spawning until we run out of eligible tiles.
                generateEverywhere = true;
            } else {
                // build as many instances as required
                generateEverywhere = false;
                instanceCount = rand_range(feature->instanceCountRange[0], feature->instanceCountRange[1]);
            }

            // Cache the personal space constant.
            personalSpace = feature->personalSpace;

            for (instance = 0; (generateEverywhere || instance < instanceCount) && qualifyingTileCount > 0;) {

                // Find a location for the feature.
                int featX;
                int featY;
                if (feature->flags & MF_BUILD_AT_ORIGIN) {
                    // Does the feature want to be at the origin? If so, put it there. (Just an optimization.)
                    featX = originX;
                    featY = originY;
                } else {
                    // Pick our candidate location randomly, and also strike it from
                    // the candidates map so that subsequent instances of this same feature can't choose it.
                    featX = -1;
                    featY = -1;
                    int randIndex = rand_range(1, qualifyingTileCount);
                    for(int i=0; i<DCOLS && featX < 0; i++) {
                        for(int j=0; j<DROWS && featX < 0; j++) {
                            if (p->candidates[i][j]) {
                                if (randIndex == 1) {
                                    // This is the place!
                                    featX = i;
                                    featY = j;
                                    i = DCOLS;  // break out of the loops
                                    j = DROWS;
                                } else {
                                    randIndex--;
                                }
                            }
                        }
                    }
                }
                // Don't waste time trying the same place again whether or not this attempt succeeds.
                p->candidates[featX][featY] = false;
                qualifyingTileCount--;

                DFSucceeded = terrainSucceeded = true;

                // Try to build the DF first, if any, since we don't want it to be disrupted by subsequently placed terrain.
                if (feature->featureDF) {
                    DFSucceeded = spawnDungeonFeature(featX, featY, &dungeonFeatureCatalog[feature->featureDF], false,
                                                      !(feature->flags & MF_PERMIT_BLOCKING));
                }

                // Now try to place the terrain tile, if any.
                if (DFSucceeded && feature->terrain) {
                    // Must we check for blocking?
                    if (!(feature->flags & MF_PERMIT_BLOCKING)
                        && ((tileCatalog[feature->terrain].flags & T_PATHING_BLOCKER) || (feature->flags & MF_TREAT_AS_BLOCKING))) {
                        // Yes, check for blocking.

                        zeroOutGrid(p->blockingMap);
                        p->blockingMap[featX][featY] = true;
                        terrainSucceeded = !levelIsDisconnectedWithBlockingMap(p->blockingMap, false);
                    }
                    if (terrainSucceeded) {
                        pmap[featX][featY].layers[feature->layer] = feature->terrain;
                    }
                }

                // OK, if placement was successful, clear some personal space around the feature so subsequent features can't be generated too close.
                // Personal space of 0 means nothing gets cleared, 1 means that only the tile itself gets cleared, and 2 means the 3x3 grid centered on it.

                if (DFSucceeded && terrainSucceeded) {
                    for (int i = featX - personalSpace + 1;
                         i <= featX + personalSpace - 1;
                         i++) {
                        for (int j = featY - personalSpace + 1;
                             j <= featY + personalSpace - 1;
                             j++) {
                            if (coordinatesAreInMap(i, j)) {
                                if (p->candidates[i][j]) {
                                    brogueAssert(!p->occupied[i][j] || (i == originX && j == originY)); // Candidates[][] should never be true where occupied[][] is true.
                                    p->candidates[i][j] = false;
                                    qualifyingTileCount--;
                                }
                                p->occupied[i][j] = true;
                            }
                        }
                    }
                    instance++; // we've placed an instance
                    //DEBUG printf("\nPlaced instance #%i of feature %i at (%i, %i).", instance, feat, featX, featY);
                }

                if (DFSucceeded && terrainSucceeded) { // Proceed only if the terrain stuff for this instance succeeded.

                    theItem = NULL;

                    // Mark the feature location as part of the machine, in case it is not already inside of it.
                    pmap[featX][featY].flags |= ((blueprintCatalog[bp].flags & BP_ROOM) ? IS_IN_ROOM_MACHINE : IS_IN_AREA_MACHINE);
                    pmap[featX][featY].machineNumber = machineNumber;

                    // Mark the feature location as impregnable if requested.
                    if (feature->flags & MF_IMPREGNABLE) {
                        pmap[featX][featY].flags |= IMPREGNABLE;
                    }

                    // Generate an item as necessary.
                    if ((feature->flags & MF_GENERATE_ITEM)
                        || (adoptiveItem && (feature->flags & MF_ADOPT_ITEM) && (blueprintCatalog[bp].flags & BP_ADOPT_ITEM))) {
                        // Are we adopting an item instead of generating one?
                        if (adoptiveItem && (feature->flags & MF_ADOPT_ITEM) && (blueprintCatalog[bp].flags & BP_ADOPT_ITEM)) {
                            theItem = adoptiveItem;
                            adoptiveItem = NULL; // can be adopted only once
                        } else {
                            // Have to create an item ourselves.
                            theItem = generateItem(feature->itemCategory, feature->itemKind);
                            failsafe = 1000;
                            while ((theItem->flags & ITEM_CURSED)
                                   || ((feature->flags & MF_REQUIRE_GOOD_RUNIC) && (!(theItem->flags & ITEM_RUNIC))) // runic if requested
                                   || ((feature->flags & MF_NO_THROWING_WEAPONS) && theItem->category == WEAPON && theItem->quantity > 1) // no throwing weapons if prohibited
                                   || itemIsADuplicate(theItem, p->spawnedItems, itemCount)) { // don't want to duplicates of rings, staffs, etc.
                                deleteItem(theItem);
                                theItem = generateItem(feature->itemCategory, feature->itemKind);
                                if (failsafe <= 0) {
                                    break;
                                }
                                failsafe--;
                            }
                            p->spawnedItems[itemCount] = theItem; // Keep a list of generated items so that we can delete them all if construction fails.
                            itemCount++;
                        }
                        theItem->flags |= feature->itemFlags;

                        addLocationToKey(theItem, featX, featY, (feature->flags & MF_KEY_DISPOSABLE) ? true : false);
                        theItem->originDepth = rogue.depthLevel;
                        if (feature->flags & MF_SKELETON_KEY) {
                            addMachineNumberToKey(theItem, machineNumber, (feature->flags & MF_KEY_DISPOSABLE) ? true : false);
                        }
                        if (!(feature->flags & MF_OUTSOURCE_ITEM_TO_MACHINE)
                            && !(feature->flags & MF_MONSTER_TAKE_ITEM)) {
                            // Place the item at the feature location.
                            placeItemAt(theItem, (pos){ featX, featY });
                        }
                    }

                    if (feature->flags & (MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_VESTIBULE)) {
                        // Put this item up for adoption, or generate a door guard machine.
                        // Try to create a sub-machine that qualifies.
                        // If we fail 10 times, abort the entire machine (including any sub-machines already built).
                        // Also, if we build a sub-machine, and it succeeds, but this (its parent machine) fails,
                        // we pass the monsters and items that it spawned back to the parent,
                        // so that if the parent fails, they can all be freed.
                        int i;
                        for (i=10; i > 0; i--) {
                            // First make sure our adopted item, if any, is not on the floor or in the pack already.
                            // Otherwise, a previous attempt to place it may have put it on the floor in a different
                            // machine, only to have that machine fail and be deleted, leaving the item remaining on
                            // the floor where placed.
                            if ((feature->flags & MF_OUTSOURCE_ITEM_TO_MACHINE) && theItem) {
                                removeItemFromChain(theItem, floorItems);
                                removeItemFromChain(theItem, packItems);
                                theItem->nextItem = NULL;
                                success = buildAMachine(-1, -1, -1, BP_ADOPT_ITEM, theItem, p->spawnedItemsSub, p->spawnedMonstersSub);
                            } else if (feature->flags & MF_BUILD_VESTIBULE) {
                                success = buildAMachine(-1, featX, featY, BP_VESTIBULE, NULL, p->spawnedItemsSub, p->spawnedMonstersSub);
                            }

                            // Now put the item up for adoption.
                            if (success) {
                                // Success! Now we have to add that machine's items and monsters to our own list, so they
                                // all get deleted if this machine or its parent fails.
                                for (int j=0; j<MACHINES_BUFFER_LENGTH && p->spawnedItemsSub[j]; j++) {
                                    p->spawnedItems[itemCount] = p->spawnedItemsSub[j];
                                    itemCount++;
                                    p->spawnedItemsSub[j] = NULL;
                                }
                                for (int j=0; j<MACHINES_BUFFER_LENGTH && p->spawnedMonstersSub[j]; j++) {
                                    p->spawnedMonsters[monsterCount] = p->spawnedMonstersSub[j];
                                    monsterCount++;
                                    p->spawnedMonstersSub[j] = NULL;
                                }
                                break;
                            }
                        }

                        if (!i) {
                            if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to place blueprint %i:%s because it requires an adoptive machine and we couldn't place one.", rogue.depthLevel, bp, blueprintCatalog[bp].name);
                            // failure! abort!
                            copyMap(p->levelBackup, pmap);
                            abortItemsAndMonsters(p->spawnedItems, p->spawnedMonsters);
                            freeGrid(distanceMap);
                            free(p);
                            return false;
                        }
                        theItem = NULL;
                    }

                    // Generate a horde as necessary.
                    if ((feature->flags & MF_GENERATE_HORDE)
                        || feature->monsterID) {

                        if (feature->flags & MF_GENERATE_HORDE) {
                            monst = spawnHorde(0,
                                               (pos){ featX, featY },
                                               ((HORDE_IS_SUMMONED | HORDE_LEADER_CAPTIVE) & ~(feature->hordeFlags)),
                                               feature->hordeFlags);
                            if (monst) {
                                monst->bookkeepingFlags |= MB_JUST_SUMMONED;
                            }
                        }

                        if (feature->monsterID) {
                            monst = monsterAtLoc((pos){ featX, featY });
                            if (monst) {
                                killCreature(monst, true); // If there's already a monster here, quietly bury the body.
                            }
                            monst = generateMonster(feature->monsterID, true, true);
                            if (monst) {
                                monst->loc = (pos){ .x = featX, .y = featY };
                                pmapAt(monst->loc)->flags |= HAS_MONSTER;
                                monst->bookkeepingFlags |= MB_JUST_SUMMONED;
                            }
                        }

                        if (monst) {
                            if (!leader) {
                                leader = monst;
                            }

                            // Give our item to the monster leader if appropriate.
                            // Actually just remember that we have to give it to this monster; the actual
                            // hand-off happens after we're sure that the machine will succeed.
                            if (theItem && (feature->flags & MF_MONSTER_TAKE_ITEM)) {
                                torchBearer = monst;
                                torch = theItem;
                            }
                        }

                        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                            creature *monst = nextCreature(&it);
                            if (monst->bookkeepingFlags & MB_JUST_SUMMONED) {

                                // All monsters spawned by a machine are tribemates.
                                // Assign leader/follower roles if they are not yet assigned.
                                if (!(monst->bookkeepingFlags & (MB_LEADER | MB_FOLLOWER))) {
                                    if (leader && leader != monst) {
                                        monst->leader = leader;
                                        monst->bookkeepingFlags &= ~MB_LEADER;
                                        monst->bookkeepingFlags |= MB_FOLLOWER;
                                        leader->bookkeepingFlags |= MB_LEADER;
                                    } else {
                                        leader = monst;
                                    }
                                }

                                monst->bookkeepingFlags &= ~MB_JUST_SUMMONED;
                                p->spawnedMonsters[monsterCount] = monst;
                                monsterCount++;
                                if (feature->flags & MF_MONSTER_SLEEPING) {
                                    monst->creatureState = MONSTER_SLEEPING;
                                }
                                if (feature->flags & MF_MONSTER_FLEEING) {
                                    monst->creatureState = MONSTER_FLEEING;
                                    monst->creatureMode = MODE_PERM_FLEEING;
                                }
                                if (feature->flags & MF_MONSTERS_DORMANT) {
                                    toggleMonsterDormancy(monst);
                                    if (!(feature->flags & MF_MONSTER_SLEEPING) && monst->creatureState != MONSTER_ALLY) {
                                        monst->creatureState = MONSTER_TRACKING_SCENT;
                                    }
                                }
                                monst->machineHome = machineNumber; // Monster remembers the machine that spawned it.
                            }
                        }
                    }
                }
                theItem = NULL;

                // Finished with this instance!
            }
        } while ((feature->flags & MF_REPEAT_UNTIL_NO_PROGRESS) && instance >= feature->minimumInstanceCount);

        //DEBUG printf("\nFinished feature %i. Here's the candidates map:", feat);
        //DEBUG logBuffer(candidates);

        if (instance < feature->minimumInstanceCount && !(feature->flags & MF_REPEAT_UNTIL_NO_PROGRESS)) {
            // failure! abort!

            if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Failed to place blueprint %i:%s because of feature %i; needed %i instances but got only %i.",
                         rogue.depthLevel, bp, blueprintCatalog[bp].name, feat, feature->minimumInstanceCount, instance);

            // Restore the map to how it was before we touched it.
            copyMap(p->levelBackup, pmap);
            abortItemsAndMonsters(p->spawnedItems, p->spawnedMonsters);
            freeGrid(distanceMap);
            free(p);
            return false;
        }
    }

    // Clear out the interior flag for all non-wired cells, if requested.
    if (blueprintCatalog[bp].flags & BP_NO_INTERIOR_FLAG) {
        for(int i=0; i<DCOLS; i++) {
            for(int j=0; j<DROWS; j++) {
                if (pmap[i][j].machineNumber == machineNumber
                    && !cellHasTMFlag((pos){ i, j }, (TM_IS_WIRED | TM_IS_CIRCUIT_BREAKER))) {

                    pmap[i][j].flags &= ~IS_IN_MACHINE;
                    pmap[i][j].machineNumber = 0;
                }
            }
        }
    }

    if (torchBearer && torch) {
        if (torchBearer->carriedItem) {
            deleteItem(torchBearer->carriedItem);
        }
        removeItemFromChain(torch, floorItems);
        torchBearer->carriedItem = torch;
    }

    freeGrid(distanceMap);
    if (D_MESSAGE_MACHINE_GENERATION) printf("\nDepth %i: Built a machine from blueprint %i:%s with an origin at (%i, %i).", rogue.depthLevel, bp, blueprintCatalog[bp].name, originX, originY);

    //Pass created items and monsters to parent where they will be deleted on failure to place parent machine
    if (parentSpawnedItems) {
        for (int i=0; i<itemCount; i++) {
            parentSpawnedItems[i] = p->spawnedItems[i];
        }
    }
    if (parentSpawnedMonsters) {
        for (int i=0; i<monsterCount; i++) {
            parentSpawnedMonsters[i] = p->spawnedMonsters[i];
        }
    }

    free(p);
    return true;
}

// add machines to the dungeon.
static void addMachines() {
    short machineCount, failsafe;
    short randomMachineFactor;

    analyzeMap(true);

    // Add the amulet holder if it's depth 26:
    if (rogue.depthLevel == gameConst->amuletLevel) {
        for (failsafe = 50; failsafe; failsafe--) {
            if (buildAMachine(MT_AMULET_AREA, -1, -1, 0, NULL, NULL, NULL)) {
                break;
            }
        }
    }

    // Add reward rooms, if any:
    machineCount = 0;
    while (rogue.depthLevel <= gameConst->amuletLevel
        && (rogue.rewardRoomsGenerated + machineCount) * gameConst->machinesPerLevelSuppressionMultiplier + gameConst->machinesPerLevelSuppressionOffset < rogue.depthLevel * gameConst->machinesPerLevelIncreaseFactor) {
        // try to build at least one every four levels on average
        machineCount++;
    }
    randomMachineFactor = (rogue.depthLevel <= gameConst->maxLevelForBonusMachines && (rogue.rewardRoomsGenerated + machineCount) == 0 ? 40 : 15);
    while (rand_percent(max(randomMachineFactor, 15 * gameConst->machinesPerLevelIncreaseFactor)) && machineCount < 100) {
        randomMachineFactor = 15;
        machineCount++;
    }

    for (failsafe = 50; machineCount && failsafe; failsafe--) {
        if (buildAMachine(-1, -1, -1, BP_REWARD, NULL, NULL, NULL)) {
            machineCount--;
            rogue.rewardRoomsGenerated++;
        }
    }
}

// Add terrain, DFs and flavor machines. Includes traps, torches, funguses, flavor machines, etc.
// If buildAreaMachines is true, build ONLY the autogenerators that include machines.
// If false, build all EXCEPT the autogenerators that include machines.
static void runAutogenerators(boolean buildAreaMachines) {
    short AG, count, i;
    const autoGenerator *gen;
    char grid[DCOLS][DROWS];

    // Cycle through the autoGenerators.
    for (AG=1; AG<gameConst->numberAutogenerators; AG++) {

        // Shortcut:
        gen = &(autoGeneratorCatalog[AG]);

        if (gen->machine > 0 == buildAreaMachines) {

            // Enforce depth constraints.
            if (rogue.depthLevel < gen->minDepth || rogue.depthLevel > gen->maxDepth) {
                continue;
            }

            // Decide how many of this AG to build.
            count = min((gen->minNumberIntercept + rogue.depthLevel * gen->minNumberSlope) / 100, gen->maxNumber);
            while (rand_percent(gen->frequency) && count < gen->maxNumber) {
                count++;
            }

            // Build that many instances.
            for (i = 0; i < count; i++) {

                // Find a location for DFs and terrain generations.
                //if (randomMatchingLocation(&x, &y, gen->requiredDungeonFoundationType, NOTHING, -1)) {
                //if (randomMatchingLocation(&x, &y, -1, -1, gen->requiredDungeonFoundationType)) {
                pos foundationLoc;
                if (randomMatchingLocation(&foundationLoc, gen->requiredDungeonFoundationType, gen->requiredLiquidFoundationType, -1)) {
                    // Spawn the DF.
                    if (gen->DFType) {
                        spawnDungeonFeature(foundationLoc.x, foundationLoc.y, &(dungeonFeatureCatalog[gen->DFType]), false, true);

                        if (D_INSPECT_LEVELGEN) {
                            dumpLevelToScreen();
                            hiliteCell(foundationLoc.x, foundationLoc.y, &yellow, 50, true);
                            temporaryMessage("Dungeon feature added.", REQUIRE_ACKNOWLEDGMENT);
                        }
                    }

                    // Spawn the terrain if it's got the priority to spawn there and won't disrupt connectivity.
                    if (gen->terrain
                        && tileCatalog[pmapAt(foundationLoc)->layers[gen->layer]].drawPriority >= tileCatalog[gen->terrain].drawPriority) {

                        // Check connectivity.
                        zeroOutGrid(grid);
                        grid[foundationLoc.x][foundationLoc.y] = true;
                        if (!(tileCatalog[gen->terrain].flags & T_PATHING_BLOCKER)
                            || !levelIsDisconnectedWithBlockingMap(grid, false)) {

                            // Build!
                            pmapAt(foundationLoc)->layers[gen->layer] = gen->terrain;

                            if (D_INSPECT_LEVELGEN) {
                                dumpLevelToScreen();
                                hiliteCell(foundationLoc.x, foundationLoc.y, &yellow, 50, true);
                                temporaryMessage("Terrain added.", REQUIRE_ACKNOWLEDGMENT);
                            }
                        }
                    }
                }

                // Attempt to build the machine if requested.
                // Machines will find their own locations, so it will not be at the same place as terrain and DF.
                if (gen->machine > 0) {
                    buildAMachine(gen->machine, -1, -1, 0, NULL, NULL, NULL);
                }
            }
        }
    }
}

// Knock down the boundaries between similar lakes where possible.
static void cleanUpLakeBoundaries() {
    short i, j, x, y, failsafe, layer;
    boolean reverse, madeChange;
    unsigned long subjectFlags;

    reverse = true;

    failsafe = 100;
    do {
        madeChange = false;
        reverse = !reverse;
        failsafe--;

        for (i = (reverse ? DCOLS - 2 : 1);
             (reverse ? i > 0 : i < DCOLS - 1);
             (reverse ? i-- : i++)) {

            for (j = (reverse ? DROWS - 2 : 1);
                 (reverse ? j > 0 : j < DROWS - 1);
                 (reverse ? j-- : j++)) {

                //assert(i >= 1 && i <= DCOLS - 2 && j >= 1 && j <= DROWS - 2);

                //if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                if (cellHasTerrainFlag((pos){ i, j }, T_LAKE_PATHING_BLOCKER | T_OBSTRUCTS_PASSABILITY)
                    && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)
                    && !(pmap[i][j].flags & IMPREGNABLE)) {

                    subjectFlags = terrainFlags((pos){ i, j }) & (T_LAKE_PATHING_BLOCKER | T_OBSTRUCTS_PASSABILITY);

                    x = y = 0;
                    if ((terrainFlags((pos){ i - 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)
                        && !cellHasTMFlag((pos){ i - 1, j }, TM_IS_SECRET)
                        && !cellHasTMFlag((pos){ i + 1, j }, TM_IS_SECRET)
                        && (terrainFlags((pos){ i - 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags) == (terrainFlags((pos){ i + 1, j }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)) {
                        x = i + 1;
                        y = j;
                    } else if ((terrainFlags((pos){ i, j - 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)
                               && !cellHasTMFlag((pos){ i, j - 1 }, TM_IS_SECRET)
                               && !cellHasTMFlag((pos){ i, j + 1 }, TM_IS_SECRET)
                               && (terrainFlags((pos){ i, j - 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags) == (terrainFlags((pos){ i, j + 1 }) & T_LAKE_PATHING_BLOCKER & ~subjectFlags)) {
                        x = i;
                        y = j + 1;
                    }
                    if (x) {
                        madeChange = true;
                        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                            pmap[i][j].layers[layer] = pmap[x][y].layers[layer];
                        }
                        //pmap[i][j].layers[DUNGEON] = CRYSTAL_WALL;
                    }
                }
            }
        }
    } while (madeChange && failsafe > 0);
}

static void removeDiagonalOpenings() {
    short i, j, k, x1, y1, x2, layer;
    boolean diagonalCornerRemoved;

    do {
        diagonalCornerRemoved = false;
        for (i=0; i<DCOLS-1; i++) {
            for (j=0; j<DROWS-1; j++) {
                for (k=0; k<=1; k++) {
                    if (!(tileCatalog[pmap[i + k][j].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + (1-k)][j].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + (1-k)][j].layers[DUNGEON]].flags & T_OBSTRUCTS_DIAGONAL_MOVEMENT)
                        && (tileCatalog[pmap[i + k][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)
                        && (tileCatalog[pmap[i + k][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_DIAGONAL_MOVEMENT)
                        && !(tileCatalog[pmap[i + (1-k)][j+1].layers[DUNGEON]].flags & T_OBSTRUCTS_PASSABILITY)) {

                        if (rand_percent(50)) {
                            x1 = i + (1-k);
                            x2 = i + k;
                            y1 = j;
                        } else {
                            x1 = i + k;
                            x2 = i + (1-k);
                            y1 = j + 1;
                        }
                        if (!(pmap[x1][y1].flags & HAS_MONSTER) && pmap[x1][y1].machineNumber == 0) {
                            diagonalCornerRemoved = true;
                            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                                pmap[x1][y1].layers[layer] = pmap[x2][y1].layers[layer];
                            }
                        }
                    }
                }
            }
        }
    } while (diagonalCornerRemoved == true);
}

static void insertRoomAt(short **dungeonMap, short **roomMap, const short roomToDungeonX, const short roomToDungeonY, const short xRoom, const short yRoom) {
    short newX, newY;
    enum directions dir;

    brogueAssert(coordinatesAreInMap(xRoom + roomToDungeonX, yRoom + roomToDungeonY));

    dungeonMap[xRoom + roomToDungeonX][yRoom + roomToDungeonY] = 1;
    for (dir = 0; dir < 4; dir++) {
        newX = xRoom + nbDirs[dir][0];
        newY = yRoom + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && roomMap[newX][newY]
            && coordinatesAreInMap(newX + roomToDungeonX, newY + roomToDungeonY)
            && dungeonMap[newX + roomToDungeonX][newY + roomToDungeonY] == 0) {

            insertRoomAt(dungeonMap, roomMap, roomToDungeonX, roomToDungeonY, newX, newY);
        }
    }
}

static void designCavern(short **grid, short minWidth, short maxWidth, short minHeight, short maxHeight) {
    short destX, destY;
    short caveX, caveY, caveWidth, caveHeight;
    short fillX = 0, fillY = 0;
    boolean foundFillPoint = false;
    short **blobGrid;
    blobGrid = allocGrid();

    fillGrid(grid, 0);
    createBlobOnGrid(blobGrid,
                     &caveX, &caveY, &caveWidth, &caveHeight,
                     5, minWidth, minHeight, maxWidth, maxHeight, 55, "ffffffttt", "ffffttttt");

//    colorOverDungeon(&darkGray);
//    hiliteGrid(blobGrid, &tanColor, 80);
//    temporaryMessage("Here's the cave:", REQUIRE_ACKNOWLEDGMENT);

    // Position the new cave in the middle of the grid...
    destX = (DCOLS - caveWidth) / 2;
    destY = (DROWS - caveHeight) / 2;
    // ...pick a floodfill insertion point...
    for (fillX = 0; fillX < DCOLS && !foundFillPoint; fillX++) {
        for (fillY = 0; fillY < DROWS && !foundFillPoint; fillY++) {
            if (blobGrid[fillX][fillY]) {
                foundFillPoint = true;
            }
        }
    }
    // ...and copy it to the master grid.
    insertRoomAt(grid, blobGrid, destX - caveX, destY - caveY, fillX, fillY);
    freeGrid(blobGrid);
}

// This is a special room that appears at the entrance to the dungeon on depth 1.
static void designEntranceRoom(short **grid) {
    short roomWidth, roomHeight, roomWidth2, roomHeight2, roomX, roomY, roomX2, roomY2;

    fillGrid(grid, 0);

    roomWidth = 8;
    roomHeight = 10;
    roomWidth2 = 20;
    roomHeight2 = 5;
    roomX = DCOLS/2 - roomWidth/2 - 1;
    roomY = DROWS - roomHeight - 2;
    roomX2 = DCOLS/2 - roomWidth2/2 - 1;
    roomY2 = DROWS - roomHeight2 - 2;

    drawRectangleOnGrid(grid, roomX, roomY, roomWidth, roomHeight, 1);
    drawRectangleOnGrid(grid, roomX2, roomY2, roomWidth2, roomHeight2, 1);
}

static void designCrossRoom(short **grid) {
    short roomWidth, roomHeight, roomWidth2, roomHeight2, roomX, roomY, roomX2, roomY2;

    fillGrid(grid, 0);

    roomWidth = rand_range(3, 12);
    roomX = rand_range(max(0, DCOLS/2 - (roomWidth - 1)), min(DCOLS, DCOLS/2));
    roomWidth2 = rand_range(4, 20);
    roomX2 = (roomX + (roomWidth / 2) + rand_range(0, 2) + rand_range(0, 2) - 3) - (roomWidth2 / 2);

    roomHeight = rand_range(3, 7);
    roomY = (DROWS/2 - roomHeight);

    roomHeight2 = rand_range(2, 5);
    roomY2 = (DROWS/2 - roomHeight2 - (rand_range(0, 2) + rand_range(0, 1)));

    drawRectangleOnGrid(grid, roomX - 5, roomY + 5, roomWidth, roomHeight, 1);
    drawRectangleOnGrid(grid, roomX2 - 5, roomY2 + 5, roomWidth2, roomHeight2, 1);
}

static void designSymmetricalCrossRoom(short **grid) {
    short majorWidth, majorHeight, minorWidth, minorHeight;

    fillGrid(grid, 0);

    majorWidth = rand_range(4, 8);
    majorHeight = rand_range(4, 5);

    minorWidth = rand_range(3, 4);
    if (majorHeight % 2 == 0) {
        minorWidth -= 1;
    }
    minorHeight = 3;//rand_range(2, 3);
    if (majorWidth % 2 == 0) {
        minorHeight -= 1;
    }

    drawRectangleOnGrid(grid, (DCOLS - majorWidth)/2, (DROWS - minorHeight)/2, majorWidth, minorHeight, 1);
    drawRectangleOnGrid(grid, (DCOLS - minorWidth)/2, (DROWS - majorHeight)/2, minorWidth, majorHeight, 1);
}

static void designSmallRoom(short **grid) {
    short width, height;

    fillGrid(grid, 0);
    width = rand_range(3, 6);
    height = rand_range(2, 4);
    drawRectangleOnGrid(grid, (DCOLS - width) / 2, (DROWS - height) / 2, width, height, 1);
}

static void designCircularRoom(short **grid) {
    short radius;

    if (rand_percent(5)) {
        radius = rand_range(4, 10);
    } else {
        radius = rand_range(2, 4);
    }

    fillGrid(grid, 0);
    drawCircleOnGrid(grid, DCOLS/2, DROWS/2, radius, 1);

    if (radius > 6
        && rand_percent(50)) {
        drawCircleOnGrid(grid, DCOLS/2, DROWS/2, rand_range(3, radius - 3), 0);
    }
}

static void designChunkyRoom(short **grid) {
    short i, x, y;
    short minX, maxX, minY, maxY;
    short chunkCount = rand_range(2, 8);

    fillGrid(grid, 0);
    drawCircleOnGrid(grid, DCOLS/2, DROWS/2, 2, 1);
    minX = DCOLS/2 - 3;
    maxX = DCOLS/2 + 3;
    minY = DROWS/2 - 3;
    maxY = DROWS/2 + 3;

    for (i=0; i<chunkCount;) {
        x = rand_range(minX, maxX);
        y = rand_range(minY, maxY);
        if (grid[x][y]) {
//            colorOverDungeon(&darkGray);
//            hiliteGrid(grid, &white, 100);

            drawCircleOnGrid(grid, x, y, 2, 1);
            i++;
            minX = max(1, min(x - 3, minX));
            maxX = min(DCOLS - 2, max(x + 3, maxX));
            minY = max(1, min(y - 3, minY));
            maxY = min(DROWS - 2, max(y + 3, maxY));

//            hiliteGrid(grid, &green, 50);
//            temporaryMessage("Added a chunk:", REQUIRE_ACKNOWLEDGMENT);
        }
    }
}

// If the indicated tile is a wall on the room stored in grid, and it could be the site of
// a door out of that room, then return the outbound direction that the door faces.
// Otherwise, return NO_DIRECTION.
static enum directions directionOfDoorSite(short **grid, short x, short y) {
    enum directions dir, solutionDir;
    short newX, newY, oppX, oppY;

    if (grid[x][y]) { // Already occupied
        return NO_DIRECTION;
    }

    solutionDir = NO_DIRECTION;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        oppX = x - nbDirs[dir][0];
        oppY = y - nbDirs[dir][1];
        if (coordinatesAreInMap(oppX, oppY)
            && coordinatesAreInMap(newX, newY)
            && grid[oppX][oppY] == 1) {

            // This grid cell would be a valid tile on which to place a door that, facing outward, points dir.
            if (solutionDir != NO_DIRECTION) {
                // Already claimed by another direction; no doors here!
                return NO_DIRECTION;
            }
            solutionDir = dir;
        }
    }
    return solutionDir;
}

static void chooseRandomDoorSites(short **roomMap, pos doorSites[4]) {
    short i, j, k, newX, newY;
    enum directions dir;
    short **grid;
    boolean doorSiteFailed;

    grid = allocGrid();
    copyGrid(grid, roomMap);

//    colorOverDungeon(&darkGray);
//    hiliteGrid(grid, &blue, 100);
//    temporaryMessage("Generating this room:", REQUIRE_ACKNOWLEDGMENT);
//    const char dirChars[] = "^v<>";

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (!grid[i][j]) {
                dir = directionOfDoorSite(roomMap, i, j);
                if (dir != NO_DIRECTION) {
                    // Trace a ray 10 spaces outward from the door site to make sure it doesn't intersect the room.
                    // If it does, it's not a valid door site.
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    doorSiteFailed = false;
                    for (k=0; k<10 && coordinatesAreInMap(newX, newY) && !doorSiteFailed; k++) {
                        if (grid[newX][newY]) {
                            doorSiteFailed = true;
                        }
                        newX += nbDirs[dir][0];
                        newY += nbDirs[dir][1];
                    }
                    if (!doorSiteFailed) {
//                        plotCharWithColor(dirChars[dir], mapToWindow((pos){ i, j }), &black, &green);
                        grid[i][j] = dir + 2; // So as not to conflict with 0 or 1, which are used to indicate exterior/interior.
                    }
                }
            }
        }
    }

//    temporaryMessage("Door candidates:", REQUIRE_ACKNOWLEDGMENT);

    // Pick four doors, one in each direction, and store them in doorSites[dir].
    for (dir=0; dir<4; dir++) {
        randomLocationInGrid(grid, &(doorSites[dir].x), &(doorSites[dir].y), dir + 2);
    }

    freeGrid(grid);
}

static void attachHallwayTo(short **grid, pos doorSites[4]) {
    short i, x, y, newX, newY, dirs[4];
    short length;
    enum directions dir, dir2;
    boolean allowObliqueHallwayExit;

    // Pick a direction.
    fillSequentialList(dirs, 4);
    shuffleList(dirs, 4);
    for (i=0; i<4; i++) {
        dir = dirs[i];
        if (doorSites[dir].x != -1
            && doorSites[dir].y != -1
            && coordinatesAreInMap(doorSites[dir].x + nbDirs[dir][0] * HORIZONTAL_CORRIDOR_MAX_LENGTH,
                                   doorSites[dir].y + nbDirs[dir][1] * VERTICAL_CORRIDOR_MAX_LENGTH)) {
                break; // That's our direction!
        }
    }
    if (i==4) {
        return; // No valid direction for hallways.
    }

    if (dir == UP || dir == DOWN) {
        length = rand_range(VERTICAL_CORRIDOR_MIN_LENGTH, VERTICAL_CORRIDOR_MAX_LENGTH);
    } else {
        length = rand_range(HORIZONTAL_CORRIDOR_MIN_LENGTH, HORIZONTAL_CORRIDOR_MAX_LENGTH);
    }

    x = doorSites[dir].x;
    y = doorSites[dir].y;
    for (i = 0; i < length; i++) {
        if (coordinatesAreInMap(x, y)) {
            grid[x][y] = true;
        }
        x += nbDirs[dir][0];
        y += nbDirs[dir][1];
    }
    x = clamp(x - nbDirs[dir][0], 0, DCOLS - 1);
    y = clamp(y - nbDirs[dir][1], 0, DROWS - 1); // Now (x, y) points at the last interior cell of the hallway.
    allowObliqueHallwayExit = rand_percent(15);
    for (dir2 = 0; dir2 < 4; dir2++) {
        newX = x + nbDirs[dir2][0];
        newY = y + nbDirs[dir2][1];

        if ((dir2 != dir && !allowObliqueHallwayExit)
            || !coordinatesAreInMap(newX, newY)
            || grid[newX][newY]) {

            doorSites[dir2] = INVALID_POS;
        } else {
            doorSites[dir2] = (pos){ .x = newX, .y = newY };
        }
    }
}

// Put a random room shape somewhere on the binary grid,
// and optionally record the coordinates of up to four door sites in doorSites.
// If attachHallway is true, then it will bolt a perpendicular hallway onto the room at one of the four standard door sites,
// and then relocate three of the door sites to radiate from the end of the hallway. (The fourth is defunct.)
// RoomTypeFrequencies specifies the probability of each room type, in the following order:
//      0. Cross room
//      1. Small symmetrical cross room
//      2. Small room
//      3. Circular room
//      4. Chunky room
//      5. Cave
//      6. Cavern (the kind that fills a level)
//      7. Entrance room (the big upside-down T room at the start of depth 1)

static void designRandomRoom(short **grid, boolean attachHallway, pos doorSites[4],
                      const short roomTypeFrequencies[ROOM_TYPE_COUNT]) {
    short randIndex, i, sum;
    enum directions dir;

    sum = 0;
    for (i=0; i<ROOM_TYPE_COUNT; i++) {
        sum += roomTypeFrequencies[i];
    }
    randIndex = rand_range(0, sum - 1);
    for (i=0; i<ROOM_TYPE_COUNT; i++) {
        if (randIndex < roomTypeFrequencies[i]) {
            break; // "i" is our room type.
        } else {
            randIndex -= roomTypeFrequencies[i];
        }
    }
    switch (i) {
        case 0:
            designCrossRoom(grid);
            break;
        case 1:
            designSymmetricalCrossRoom(grid);
            break;
        case 2:
            designSmallRoom(grid);
            break;
        case 3:
            designCircularRoom(grid);
            break;
        case 4:
            designChunkyRoom(grid);
            break;
        case 5:
            switch (rand_range(0, 2)) {
                case 0:
                    designCavern(grid, 3, 12, 4, 8); // Compact cave room.
                    break;
                case 1:
                    designCavern(grid, 3, 12, 15, DROWS-2); // Large north-south cave room.
                    break;
                case 2:
                    designCavern(grid, 20, DROWS-2, 4, 8); // Large east-west cave room.
                    break;
                default:
                    break;
            }
            break;
        case 6:
            designCavern(grid, CAVE_MIN_WIDTH, DCOLS - 2, CAVE_MIN_HEIGHT, DROWS - 2);
            break;
        case 7:
            designEntranceRoom(grid);
            break;
        default:
            break;
    }

    if (doorSites) {
        chooseRandomDoorSites(grid, doorSites);
        if (attachHallway) {
            dir = rand_range(0, 3);
            for (i=0; doorSites[dir].x == -1 && i < 3; i++) {
                dir = (dir + 1) % 4; // Each room will have at least 2 valid directions for doors.
            }
            attachHallwayTo(grid, doorSites);
        }
    }
}

static boolean roomFitsAt(short **dungeonMap, short **roomMap, short roomToDungeonX, short roomToDungeonY) {
    short xRoom, yRoom, xDungeon, yDungeon, i, j;

    for (xRoom = 0; xRoom < DCOLS; xRoom++) {
        for (yRoom = 0; yRoom < DROWS; yRoom++) {
            if (roomMap[xRoom][yRoom]) {
                xDungeon = xRoom + roomToDungeonX;
                yDungeon = yRoom + roomToDungeonY;

                for (i = xDungeon - 1; i <= xDungeon + 1; i++) {
                    for (j = yDungeon - 1; j <= yDungeon + 1; j++) {
                        if (!coordinatesAreInMap(i, j)
                            || dungeonMap[i][j] > 0) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

void attachRooms(short **grid, const dungeonProfile *theDP, short attempts, short maxRoomCount) {
    short roomsBuilt, roomsAttempted;
    short **roomMap;
    pos doorSites[4];
    short i, x, y, sCoord[DCOLS*DROWS];
    enum directions dir, oppDir;

    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);

    roomMap = allocGrid();
    for (roomsBuilt = roomsAttempted = 0; roomsBuilt < maxRoomCount && roomsAttempted < attempts; roomsAttempted++) {
        // Build a room in hyperspace.
        fillGrid(roomMap, 0);
        designRandomRoom(roomMap, roomsAttempted <= attempts - 5 && rand_percent(theDP->corridorChance),
                         doorSites, theDP->roomFrequencies);

        if (D_INSPECT_LEVELGEN) {
            colorOverDungeon(&darkGray);
            hiliteGrid(roomMap, &blue, 100);
            if (doorSites[0].x != -1) plotCharWithColor('^', mapToWindow(doorSites[0]), &black, &green);
            if (doorSites[1].x != -1) plotCharWithColor('v', mapToWindow(doorSites[1]), &black, &green);
            if (doorSites[2].x != -1) plotCharWithColor('<', mapToWindow(doorSites[2]), &black, &green);
            if (doorSites[3].x != -1) plotCharWithColor('>', mapToWindow(doorSites[3]), &black, &green);
            temporaryMessage("Generating this room:", REQUIRE_ACKNOWLEDGMENT);
        }

        // Slide hyperspace across real space, in a random but predetermined order, until the room matches up with a wall.
        for (i = 0; i < DCOLS*DROWS; i++) {
            x = sCoord[i] / DROWS;
            y = sCoord[i] % DROWS;

            dir = directionOfDoorSite(grid, x, y);
            oppDir = oppositeDirection(dir);
            if (dir != NO_DIRECTION
                && doorSites[oppDir].x != -1
                && roomFitsAt(grid, roomMap, x - doorSites[oppDir].x, y - doorSites[oppDir].y)) {

                // Room fits here.
                if (D_INSPECT_LEVELGEN) {
                    colorOverDungeon(&darkGray);
                    hiliteGrid(grid, &white, 100);
                }
                insertRoomAt(grid, roomMap, x - doorSites[oppDir].x, y - doorSites[oppDir].y, doorSites[oppDir].x, doorSites[oppDir].y);
                grid[x][y] = 2; // Door site.
                if (D_INSPECT_LEVELGEN) {
                    hiliteGrid(grid, &green, 50);
                    temporaryMessage("Added room.", REQUIRE_ACKNOWLEDGMENT);
                }
                roomsBuilt++;
                break;
            }
        }
    }

    freeGrid(roomMap);
}

static void adjustDungeonProfileForDepth(dungeonProfile *theProfile) {
    const short descentPercent = clamp(100 * (rogue.depthLevel - 1) / (gameConst->amuletLevel - 1), 0, 100);

    theProfile->roomFrequencies[0] += 20 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[1] += 10 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[3] +=  7 * (100 - descentPercent) / 100;
    theProfile->roomFrequencies[5] += 10 * descentPercent / 100;

    theProfile->corridorChance += 80 * (100 - descentPercent) / 100;
}

static void adjustDungeonFirstRoomProfileForDepth(dungeonProfile *theProfile) {
    short i;
    const short descentPercent = clamp(100 * (rogue.depthLevel - 1) / (gameConst->amuletLevel - 1), 0, 100);

    if (rogue.depthLevel == 1) {
        // All dungeons start with the entrance room on depth 1.
        for (i = 0; i < ROOM_TYPE_COUNT; i++) {
            theProfile->roomFrequencies[i] = 0;
        }
        theProfile->roomFrequencies[7] = 1;
    } else {
        theProfile->roomFrequencies[6] += 50 * descentPercent / 100;
    }
}

// Called by digDungeon().
// Slaps a bunch of rooms and hallways into the grid.
// On the grid, a 0 denotes granite, a 1 denotes floor, and a 2 denotes a possible door site.
// -1 denotes off-limits areas -- rooms can't be placed there and also can't sprout off of there.
// Parent function will translate this grid into pmap[][] to make floors, walls, doors, etc.
static void carveDungeon(short **grid) {
    dungeonProfile theDP, theFirstRoomDP;

    theDP = dungeonProfileCatalog[DP_BASIC];
    adjustDungeonProfileForDepth(&theDP);

    theFirstRoomDP = dungeonProfileCatalog[DP_BASIC_FIRST_ROOM];
    adjustDungeonFirstRoomProfileForDepth(&theFirstRoomDP);

    designRandomRoom(grid, false, NULL, theFirstRoomDP.roomFrequencies);

    if (D_INSPECT_LEVELGEN) {
        colorOverDungeon(&darkGray);
        hiliteGrid(grid, &white, 100);
        temporaryMessage("First room placed:", REQUIRE_ACKNOWLEDGMENT);
    }

    attachRooms(grid, &theDP, 35, 35);

//    colorOverDungeon(&darkGray);
//    hiliteGrid(grid, &white, 100);
//    temporaryMessage("How does this finished level look?", REQUIRE_ACKNOWLEDGMENT);
}

static void finishWalls(boolean includingDiagonals) {
    short i, j, x1, y1;
    boolean foundExposure;
    enum directions dir;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].layers[DUNGEON] == GRANITE) {
                foundExposure = false;
                for (dir = 0; dir < (includingDiagonals ? 8 : 4) && !foundExposure; dir++) {
                    x1 = i + nbDirs[dir][0];
                    y1 = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(x1, y1)
                        && (!cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_VISION) || !cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_PASSABILITY))) {

                        pmap[i][j].layers[DUNGEON] = WALL;
                        foundExposure = true;
                    }
                }
            } else if (pmap[i][j].layers[DUNGEON] == WALL) {
                foundExposure = false;
                for (dir = 0; dir < (includingDiagonals ? 8 : 4) && !foundExposure; dir++) {
                    x1 = i + nbDirs[dir][0];
                    y1 = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(x1, y1)
                        && (!cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_VISION) || !cellHasTerrainFlag((pos){ x1, y1 }, T_OBSTRUCTS_PASSABILITY))) {

                        foundExposure = true;
                    }
                }
                if (foundExposure == false) {
                    pmap[i][j].layers[DUNGEON] = GRANITE;
                }
            }
        }
    }
}

static void liquidType(short *deep, short *shallow, short *shallowWidth) {
    short randMin, randMax, rand;

    randMin = (rogue.depthLevel < gameConst->minimumLavaLevel ? 1 : 0);
    randMax = (rogue.depthLevel < gameConst->minimumBrimstoneLevel ? 2 : 3);
    rand = rand_range(randMin, randMax);
    if (rogue.depthLevel == gameConst->deepestLevel) {
        rand = 1;
    }

    switch(rand) {
        case 0:
            *deep = LAVA;
            *shallow = NOTHING;
            *shallowWidth = 0;
            break;
        case 1:
            *deep = DEEP_WATER;
            *shallow = SHALLOW_WATER;
            *shallowWidth = 2;
            break;
        case 2:
            *deep = CHASM;
            *shallow = CHASM_EDGE;
            *shallowWidth = 1;
            break;
        case 3:
            *deep = INERT_BRIMSTONE;
            *shallow = OBSIDIAN;
            *shallowWidth = 2;
            break;
    }
}

// Fills a lake marked in unfilledLakeMap with the specified liquid type, scanning outward to reach other lakes within scanWidth.
// Any wreath of shallow liquid must be done elsewhere.
static void fillLake(short x, short y, short liquid, short scanWidth, char wreathMap[DCOLS][DROWS], short **unfilledLakeMap) {
    short i, j;

    for (i = x - scanWidth; i <= x + scanWidth; i++) {
        for (j = y - scanWidth; j <= y + scanWidth; j++) {
            if (coordinatesAreInMap(i, j) && unfilledLakeMap[i][j]) {
                unfilledLakeMap[i][j] = false;
                pmap[i][j].layers[LIQUID] = liquid;
                wreathMap[i][j] = 1;
                fillLake(i, j, liquid, scanWidth, wreathMap, unfilledLakeMap);  // recursive
            }
        }
    }
}

static void lakeFloodFill(short x, short y, short **floodMap, short **grid, short **lakeMap, short dungeonToGridX, short dungeonToGridY) {
    short newX, newY;
    enum directions dir;

    floodMap[x][y] = true;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && !floodMap[newX][newY]
            && (!cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER) || cellHasTMFlag((pos){ newX, newY }, TM_CONNECTS_LEVEL))
            && !lakeMap[newX][newY]
            && (!coordinatesAreInMap(newX+dungeonToGridX, newY+dungeonToGridY) || !grid[newX+dungeonToGridX][newY+dungeonToGridY])) {

            lakeFloodFill(newX, newY, floodMap, grid, lakeMap, dungeonToGridX, dungeonToGridY);
        }
    }
}

static boolean lakeDisruptsPassability(short **grid, short **lakeMap, short dungeonToGridX, short dungeonToGridY) {
    boolean result;
    short i, j, x, y;
    short **floodMap;

    floodMap = allocGrid();
    fillGrid(floodMap, 0);
    x = y = -1;
    // Get starting location for the fill.
    for (i=0; i<DCOLS && x == -1; i++) {
        for (j=0; j<DROWS && x == -1; j++) {
            if (!cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !lakeMap[i][j]
                && (!coordinatesAreInMap(i+dungeonToGridX, j+dungeonToGridY) || !grid[i+dungeonToGridX][j+dungeonToGridY])) {

                x = i;
                y = j;
            }
        }
    }
    brogueAssert(x != -1);
    // Do the flood fill.
    lakeFloodFill(x, y, floodMap, grid, lakeMap, dungeonToGridX, dungeonToGridY);

    // See if any dry tiles weren't reached by the flood fill.
    result = false;
    for (i=0; i<DCOLS && result == false; i++) {
        for (j=0; j<DROWS && result == false; j++) {
            if (!cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER)
                && !lakeMap[i][j]
                && !floodMap[i][j]
                && (!coordinatesAreInMap(i+dungeonToGridX, j+dungeonToGridY) || !grid[i+dungeonToGridX][j+dungeonToGridY])) {

//                if (D_INSPECT_LEVELGEN) {
//                    dumpLevelToScreen();
//                    hiliteGrid(lakeMap, &darkBlue, 75);
//                    hiliteGrid(floodMap, &white, 20);
//                    plotCharWithColor('X', mapToWindow((pos){ i, j }), &black, &red);
//                    temporaryMessage("Failed here.", REQUIRE_ACKNOWLEDGMENT);
//                }

                result = true;
            }
        }
    }

    freeGrid(floodMap);
    return result;
}

static void designLakes(short **lakeMap) {
    short i, j, k;
    short x, y;
    short lakeMaxHeight, lakeMaxWidth;
    short lakeX, lakeY, lakeWidth, lakeHeight;

    short **grid; // Holds the current lake.

    grid = allocGrid();
    fillGrid(lakeMap, 0);
    for (lakeMaxHeight = 15, lakeMaxWidth = 30; lakeMaxHeight >=10; lakeMaxHeight--, lakeMaxWidth -= 2) { // lake generations

        fillGrid(grid, 0);
        createBlobOnGrid(grid, &lakeX, &lakeY, &lakeWidth, &lakeHeight, 5, 4, 4, lakeMaxWidth, lakeMaxHeight, 55, "ffffftttt", "ffffttttt");

//        if (D_INSPECT_LEVELGEN) {
//            colorOverDungeon(&darkGray);
//            hiliteGrid(grid, &white, 100);
//            temporaryMessage("Generated a lake.", REQUIRE_ACKNOWLEDGMENT);
//        }

        for (k=0; k<20; k++) { // placement attempts
            // propose a position for the top-left of the grid in the dungeon
            x = rand_range(1 - lakeX, DCOLS - lakeWidth - lakeX - 2);
            y = rand_range(1 - lakeY, DROWS - lakeHeight - lakeY - 2);

            if (!lakeDisruptsPassability(grid, lakeMap, -x, -y)) { // level with lake is completely connected
                //printf("Placed a lake!");

                // copy in lake
                for (i = 0; i < lakeWidth; i++) {
                    for (j = 0; j < lakeHeight; j++) {
                        if (grid[i + lakeX][j + lakeY]) {
                            lakeMap[i + lakeX + x][j + lakeY + y] = true;
                            pmap[i + lakeX + x][j + lakeY + y].layers[DUNGEON] = FLOOR;
                        }
                    }
                }

                if (D_INSPECT_LEVELGEN) {
                    dumpLevelToScreen();
                    hiliteGrid(lakeMap, &white, 50);
                    temporaryMessage("Added a lake location.", REQUIRE_ACKNOWLEDGMENT);
                }
                break;
            }
        }
    }
    freeGrid(grid);
}

static void createWreath(short shallowLiquid, short wreathWidth, char wreathMap[DCOLS][DROWS]) {
    short i, j, k, l;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (wreathMap[i][j]) {
                for (k = i-wreathWidth; k<= i+wreathWidth; k++) {
                    for (l = j-wreathWidth; l <= j+wreathWidth; l++) {
                        if (coordinatesAreInMap(k, l) && pmap[k][l].layers[LIQUID] == NOTHING
                            && (i-k)*(i-k) + (j-l)*(j-l) <= wreathWidth*wreathWidth) {
                            pmap[k][l].layers[LIQUID] = shallowLiquid;
                            if (pmap[k][l].layers[DUNGEON] == DOOR) {
                                pmap[k][l].layers[DUNGEON] = FLOOR;
                            }
                        }
                    }
                }
            }
        }
    }
}

static void fillLakes(short **lakeMap) {
    short deepLiquid = CRYSTAL_WALL, shallowLiquid = CRYSTAL_WALL, shallowLiquidWidth = 0;
    char wreathMap[DCOLS][DROWS];
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (lakeMap[i][j]) {
                liquidType(&deepLiquid, &shallowLiquid, &shallowLiquidWidth);
                zeroOutGrid(wreathMap);
                fillLake(i, j, deepLiquid, 4, wreathMap, lakeMap);
                createWreath(shallowLiquid, shallowLiquidWidth, wreathMap);

                if (D_INSPECT_LEVELGEN) {
                    dumpLevelToScreen();
                    hiliteGrid(lakeMap, &white, 75);
                    temporaryMessage("Lake filled.", REQUIRE_ACKNOWLEDGMENT);
                }
            }
        }
    }
}

static void finishDoors() {
    short i, j;
    const short secretDoorChance = clamp((rogue.depthLevel - 1) * 67 / (gameConst->amuletLevel - 1), 0, 67);
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (pmap[i][j].layers[DUNGEON] == DOOR
                && pmap[i][j].machineNumber == 0) {
                if ((!cellHasTerrainFlag((pos){ i+1, j }, T_OBSTRUCTS_PASSABILITY) || !cellHasTerrainFlag((pos){ i-1, j }, T_OBSTRUCTS_PASSABILITY))
                    && (!cellHasTerrainFlag((pos){ i, j+1 }, T_OBSTRUCTS_PASSABILITY) || !cellHasTerrainFlag((pos){ i, j-1 }, T_OBSTRUCTS_PASSABILITY))) {
                    // If there's passable terrain to the left or right, and there's passable terrain
                    // above or below, then the door is orphaned and must be removed.
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                } else if ((cellHasTerrainFlag((pos){ i+1, j }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i-1, j }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i, j+1 }, T_PATHING_BLOCKER) ? 1 : 0)
                           + (cellHasTerrainFlag((pos){ i, j-1 }, T_PATHING_BLOCKER) ? 1 : 0) >= 3) {
                    // If the door has three or more pathing blocker neighbors in the four cardinal directions,
                    // then the door is orphaned and must be removed.
                    pmap[i][j].layers[DUNGEON] = FLOOR;
                } else if (rand_percent(secretDoorChance)) {
                    pmap[i][j].layers[DUNGEON] = SECRET_DOOR;
                }
            }
        }
    }
}

static void clearLevel() {
    short i, j;

    for( i=0; i<DCOLS; i++ ) {
        for( j=0; j<DROWS; j++ ) {
            pmap[i][j].layers[DUNGEON] = GRANITE;
            pmap[i][j].layers[LIQUID] = NOTHING;
            pmap[i][j].layers[GAS] = NOTHING;
            pmap[i][j].layers[SURFACE] = NOTHING;
            pmap[i][j].machineNumber = 0;
            pmap[i][j].rememberedTerrain = NOTHING;
            pmap[i][j].rememberedTerrainFlags = (T_OBSTRUCTS_EVERYTHING);
            pmap[i][j].rememberedTMFlags = 0;
            pmap[i][j].rememberedCellFlags = 0;
            pmap[i][j].rememberedItemCategory = 0;
            pmap[i][j].rememberedItemKind = 0;
            pmap[i][j].rememberedItemQuantity = 0;
            pmap[i][j].rememberedItemOriginDepth = 0;
            pmap[i][j].flags = 0;
            pmap[i][j].volume = 0;
        }
    }
}

// Scans the map in random order looking for a good place to build a bridge.
// If it finds one, it builds a bridge there, halts and returns true.
static boolean buildABridge() {
    short i, j, k, l, i2, j2, nCols[DCOLS], nRows[DROWS];
    short bridgeRatioX, bridgeRatioY;
    boolean foundExposure;

    bridgeRatioX = (short) (100 + (100 + 100 * rogue.depthLevel * gameConst->depthAccelerator / 9) * rand_range(10, 20) / 10);
    bridgeRatioY = (short) (100 + (400 + 100 * rogue.depthLevel * gameConst->depthAccelerator / 18) * rand_range(10, 20) / 10);

    fillSequentialList(nCols, DCOLS);
    shuffleList(nCols, DCOLS);
    fillSequentialList(nRows, DROWS);
    shuffleList(nRows, DROWS);

    for (i2=1; i2<DCOLS-1; i2++) {
        i = nCols[i2];
        for (j2=1; j2<DROWS-1; j2++) {
            j = nRows[j2];
            if (!cellHasTerrainFlag((pos){ i, j }, (T_CAN_BE_BRIDGED | T_PATHING_BLOCKER))
                && !pmap[i][j].machineNumber) {

                // try a horizontal bridge
                foundExposure = false;
                for (k = i + 1;
                     k < DCOLS // Iterate across the prospective length of the bridge.
                     && !pmap[k][j].machineNumber // No bridges in machines.
                     && cellHasTerrainFlag((pos){ k, j }, T_CAN_BE_BRIDGED)  // Candidate tile must be chasm.
                     && !cellHasTMFlag((pos){ k, j }, TM_IS_SECRET) // Can't bridge over secret trapdoors.
                     && !cellHasTerrainFlag((pos){ k, j }, T_OBSTRUCTS_PASSABILITY)  // Candidate tile cannot be a wall.
                     && cellHasTerrainFlag((pos){ k, j-1 }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY))    // Only chasms or walls are permitted next to the length of the bridge.
                     && cellHasTerrainFlag((pos){ k, j+1 }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY));
                     k++) {

                    if (!cellHasTerrainFlag((pos){ k, j-1 }, T_OBSTRUCTS_PASSABILITY) // Can't run against a wall the whole way.
                        && !cellHasTerrainFlag((pos){ k, j+1 }, T_OBSTRUCTS_PASSABILITY)) {
                        foundExposure = true;
                    }
                }
                if (k < DCOLS
                    && (k - i > 3) // Can't have bridges shorter than 3 spaces.
                    && foundExposure
                    && !cellHasTerrainFlag((pos){ k, j }, T_PATHING_BLOCKER | T_CAN_BE_BRIDGED) // Must end on an unobstructed land tile.
                    && !pmap[k][j].machineNumber // Cannot end in a machine.
                    && 100 * pathingDistance(i, j, k, j, T_PATHING_BLOCKER) / (k - i) > bridgeRatioX) { // Must shorten the pathing distance enough.

                    for (l=i+1; l < k; l++) {
                        pmap[l][j].layers[LIQUID] = BRIDGE;
                    }
                    pmap[i][j].layers[SURFACE] = BRIDGE_EDGE;
                    pmap[k][j].layers[SURFACE] = BRIDGE_EDGE;
                    return true;
                }

                // try a vertical bridge
                foundExposure = false;
                for (k = j + 1;
                     k < DROWS
                     && !pmap[i][k].machineNumber
                     && cellHasTerrainFlag((pos){ i, k }, T_CAN_BE_BRIDGED)
                     && !cellHasTMFlag((pos){ i, k }, TM_IS_SECRET)
                     && !cellHasTerrainFlag((pos){ i, k }, T_OBSTRUCTS_PASSABILITY)
                     && cellHasTerrainFlag((pos){ i-1, k }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY))
                     && cellHasTerrainFlag((pos){ i+1, k }, (T_CAN_BE_BRIDGED | T_OBSTRUCTS_PASSABILITY));
                     k++) {

                    if (!cellHasTerrainFlag((pos){ i-1, k }, T_OBSTRUCTS_PASSABILITY)
                        && !cellHasTerrainFlag((pos){ i+1, k }, T_OBSTRUCTS_PASSABILITY)) {
                        foundExposure = true;
                    }
                }
                if (k < DROWS
                    && (k - j > 3)
                    && foundExposure
                    && !cellHasTerrainFlag((pos){ i, k }, T_PATHING_BLOCKER | T_CAN_BE_BRIDGED)
                    && !pmap[i][k].machineNumber // Cannot end in a machine.
                    && 100 * pathingDistance(i, j, i, k, T_PATHING_BLOCKER) / (k - j) > bridgeRatioY) {

                    for (l=j+1; l < k; l++) {
                        pmap[i][l].layers[LIQUID] = BRIDGE;
                    }
                    pmap[i][j].layers[SURFACE] = BRIDGE_EDGE;
                    pmap[i][k].layers[SURFACE] = BRIDGE_EDGE;
                    return true;
                }
            }
        }
    }
    return false;
}

// This is the master function for digging out a dungeon level.
// Finishing touches -- items, monsters, staircases, etc. -- are handled elsewhere.
void digDungeon() {
    short i, j;

    short **grid;

    rogue.machineNumber = 0;

    topBlobMinX = topBlobMinY = blobWidth = blobHeight = 0;

#ifdef AUDIT_RNG
    char RNGMessage[100];
    sprintf(RNGMessage, "\n\n\nDigging dungeon level %i:\n", rogue.depthLevel);
    RNGLog(RNGMessage);
#endif

    // Clear level and fill with granite
    clearLevel();

    grid = allocGrid();
    carveDungeon(grid);
    addLoops(grid, 20);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (grid[i][j] == 1) {
                pmap[i][j].layers[DUNGEON] = FLOOR;
            } else if (grid[i][j] == 2) {
                pmap[i][j].layers[DUNGEON] = (rand_percent(60) && rogue.depthLevel < gameConst->deepestLevel ? DOOR : FLOOR);
            }
        }
    }
    freeGrid(grid);

    finishWalls(false);

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Carved into the granite:", REQUIRE_ACKNOWLEDGMENT);
    }
    //DEBUG printf("\n%i loops created.", numLoops);
    //DEBUG logLevel();

    // Time to add lakes and chasms. Strategy is to generate a series of blob lakes of decreasing size. For each lake,
    // propose a position, and then check via a flood fill that the level would remain connected with that placement (i.e. that
    // each passable tile can still be reached). If not, make 9 more placement attempts before abandoning that lake
    // and proceeding to generate the next smaller one.
    // Canvas sizes start at 30x15 and decrease by 2x1 at a time down to a minimum of 20x10. Min generated size is always 4x4.

    // DEBUG logLevel();

    // Now design the lakes and then fill them with various liquids (lava, water, chasm, brimstone).
    short **lakeMap = allocGrid();
    designLakes(lakeMap);
    fillLakes(lakeMap);
    freeGrid(lakeMap);

    // Run the non-machine autoGenerators.
    runAutogenerators(false);

    // Remove diagonal openings.
    removeDiagonalOpenings();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Diagonal openings removed.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now add some treasure machines.
    addMachines();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Machines added.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Run the machine autoGenerators.
    runAutogenerators(true);

    // Now knock down the boundaries between similar lakes where possible.
    cleanUpLakeBoundaries();

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Lake boundaries cleaned up.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now add some bridges.
    while (buildABridge());

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Bridges added.", REQUIRE_ACKNOWLEDGMENT);
    }

    // Now remove orphaned doors and upgrade some doors to secret doors
    finishDoors();

    // Now finish any exposed granite with walls and revert any unexposed walls to granite
    finishWalls(true);

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        temporaryMessage("Finishing touches added. Level has been generated.", REQUIRE_ACKNOWLEDGMENT);
    }
}

void updateMapToShore() {
    short i, j;
    short **costMap;

    rogue.updatedMapToShoreThisTurn = true;

    costMap = allocGrid();

    // Calculate the map to shore for this level
    if (!rogue.mapToShore) {
        rogue.mapToShore = allocGrid();
        fillGrid(rogue.mapToShore, 0);
    }
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)) {
                costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                rogue.mapToShore[i][j] = 30000;
            } else {
                costMap[i][j] = 1;
                rogue.mapToShore[i][j] = (cellHasTerrainFlag((pos){ i, j }, T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT)
                                          && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)) ? 30000 : 0;
            }
        }
    }
    dijkstraScan(rogue.mapToShore, costMap, true);
    freeGrid(costMap);
}

// Calculates the distance map for the given waypoint.
// This is called on all waypoints during setUpWaypoints(),
// and then one waypoint is recalculated per turn thereafter.
void refreshWaypoint(short wpIndex) {
    short **costMap;

    costMap = allocGrid();
    populateGenericCostMap(costMap);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature* monst = nextCreature(&it);
        if ((monst->creatureState == MONSTER_SLEEPING || (monst->info.flags & MONST_IMMOBILE) || (monst->bookkeepingFlags & MB_CAPTIVE))
            && costMap[monst->loc.x][monst->loc.y] >= 0) {

            costMap[monst->loc.x][monst->loc.y] = PDS_FORBIDDEN;
        }
    }
    fillGrid(rogue.wpDistance[wpIndex], 30000);
    rogue.wpDistance[wpIndex][rogue.wpCoordinates[wpIndex].x][rogue.wpCoordinates[wpIndex].y] = 0;
    dijkstraScan(rogue.wpDistance[wpIndex], costMap, true);
    freeGrid(costMap);
}

void setUpWaypoints() {
    short i, j, sCoord[DCOLS * DROWS], x, y;
    char grid[DCOLS][DROWS];

    zeroOutGrid(grid);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_SCENT)) {
                grid[i][j] = 1;
            }
        }
    }
    rogue.wpCount = 0;
    rogue.wpRefreshTicker = 0;
    fillSequentialList(sCoord, DCOLS*DROWS);
    shuffleList(sCoord, DCOLS*DROWS);
    for (i = 0; i < DCOLS*DROWS && rogue.wpCount < MAX_WAYPOINT_COUNT; i++) {
        x = sCoord[i]/DROWS;
        y = sCoord[i] % DROWS;
        if (!grid[x][y]) {
            getFOVMask(grid, x, y, WAYPOINT_SIGHT_RADIUS * FP_FACTOR, T_OBSTRUCTS_SCENT, 0, false);
            grid[x][y] = true;
            rogue.wpCoordinates[rogue.wpCount] = (pos) { x, y };
            rogue.wpCount++;
//            blackOutScreen();
//            dumpLevelToScreen();
//            hiliteCharGrid(grid, &yellow, 50);
//            temporaryMessage("Waypoint coverage so far:", REQUIRE_ACKNOWLEDGMENT);
        }
    }

    for (i=0; i<rogue.wpCount; i++) {
        refreshWaypoint(i);
//        blackOutScreen();
//        dumpLevelToScreen();
//        displayGrid(rogue.wpDistance[i]);
//        temporaryMessage("Waypoint distance map:", REQUIRE_ACKNOWLEDGMENT);
    }
}

void zeroOutGrid(char grid[DCOLS][DROWS]) {
    short i, j;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            grid[i][j] = 0;
        }
    }
}

short oppositeDirection(short theDir) {
    switch (theDir) {
        case UP:
            return DOWN;
        case DOWN:
            return UP;
        case LEFT:
            return RIGHT;
        case RIGHT:
            return LEFT;
        case UPRIGHT:
            return DOWNLEFT;
        case DOWNLEFT:
            return UPRIGHT;
        case UPLEFT:
            return DOWNRIGHT;
        case DOWNRIGHT:
            return UPLEFT;
        case NO_DIRECTION:
            return NO_DIRECTION;
        default:
            return -1;
    }
}

// blockingMap is optional.
// Returns the size of the connected zone, and marks visited[][] with the zoneLabel.
static short connectCell(short x, short y, short zoneLabel, char blockingMap[DCOLS][DROWS], char zoneMap[DCOLS][DROWS]) {
    enum directions dir;
    short newX, newY, size;

    zoneMap[x][y] = zoneLabel;
    size = 1;

    for (dir = 0; dir < 4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        if (coordinatesAreInMap(newX, newY)
            && zoneMap[newX][newY] == 0
            && (!blockingMap || !blockingMap[newX][newY])
            && cellIsPassableOrDoor(newX, newY)) {

            size += connectCell(newX, newY, zoneLabel, blockingMap, zoneMap);
        }
    }
    return size;
}

// Make a zone map of connected passable regions that include at least one passable
// cell that borders the blockingMap if blockingMap blocks. Keep track of the size of each zone.
// Then pretend that the blockingMap no longer blocks, and grow these zones into the resulting area
// (without changing the stored zone sizes). If two or more zones now touch, then we block.
// At that point, return the size in cells of the smallest of all of the touching regions
// (or just 1, i.e. true, if countRegionSize is false). If no zones touch, then we don't block, and we return zero, i.e. false.
short levelIsDisconnectedWithBlockingMap(char blockingMap[DCOLS][DROWS], boolean countRegionSize) {
    char zoneMap[DCOLS][DROWS];
    short i, j, dir, zoneSizes[200], zoneCount, smallestQualifyingZoneSize, borderingZone;

    zoneCount = 0;
    smallestQualifyingZoneSize = 10000;
    zeroOutGrid(zoneMap);

//  dumpLevelToScreen();
//  hiliteCharGrid(blockingMap, &omniscienceColor, 100);
//  temporaryMessage("Blocking map:", REQUIRE_ACKNOWLEDGMENT);

    // Map out the zones with the blocking area blocked.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (cellIsPassableOrDoor(i, j) && zoneMap[i][j] == 0 && !blockingMap[i][j]) {
                for (dir=0; dir<4; dir++) {
                    if (blockingMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]]) {
                        zoneCount++;
                        zoneSizes[zoneCount - 1] = connectCell(i, j, zoneCount, blockingMap, zoneMap);
                        break;
                    }
                }
            }
        }
    }

    // Expand the zones into the blocking area.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (blockingMap[i][j] && zoneMap[i][j] == 0 && cellIsPassableOrDoor(i, j)) {
                for (dir=0; dir<4; dir++) {
                    borderingZone = zoneMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]];
                    if (borderingZone != 0) {
                        connectCell(i, j, borderingZone, NULL, zoneMap);
                        break;
                    }
                }
            }
        }
    }

    // Figure out which zones touch.
    for (i=1; i<DCOLS-1; i++) {
        for (j=1; j<DROWS-1; j++) {
            if (zoneMap[i][j] != 0) {
                for (dir=0; dir<4; dir++) {
                    borderingZone = zoneMap[i + nbDirs[dir][0]][j + nbDirs[dir][1]];
                    if (zoneMap[i][j] != borderingZone && borderingZone != 0) {
                        if (!countRegionSize) {
                            return true;
                        }
                        smallestQualifyingZoneSize = min(smallestQualifyingZoneSize, zoneSizes[zoneMap[i][j] - 1]);
                        smallestQualifyingZoneSize = min(smallestQualifyingZoneSize, zoneSizes[borderingZone - 1]);
                        break;
                    }
                }
            }
        }
    }
    return (smallestQualifyingZoneSize < 10000 ? smallestQualifyingZoneSize : 0);
}

void resetDFMessageEligibility() {
    short i;

    for (i=0; i<NUMBER_DUNGEON_FEATURES; i++) {
        dungeonFeatureCatalog[i].messageDisplayed = false;
    }
}

boolean fillSpawnMap(enum dungeonLayers layer,
                     enum tileType surfaceTileType,
                     char spawnMap[DCOLS][DROWS],
                     boolean blockedByOtherLayers,
                     boolean refresh,
                     boolean superpriority) {
    short i, j;
    creature *monst;
    item *theItem;
    boolean accomplishedSomething;

    accomplishedSomething = false;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (    // If it's flagged for building in the spawn map,
                spawnMap[i][j]
                    // and the new cell doesn't already contain the fill terrain,
                && pmap[i][j].layers[layer] != surfaceTileType
                    // and the terrain in the layer to be overwritten has a higher priority number (unless superpriority),
                && (superpriority || tileCatalog[pmap[i][j].layers[layer]].drawPriority >= tileCatalog[surfaceTileType].drawPriority)
                    // and we won't be painting into the surface layer when that cell forbids it,
                && !(layer == SURFACE && cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_SURFACE_EFFECTS))
                    // and, if requested, the fill won't violate the priority of the most important terrain in this cell:
                && (!blockedByOtherLayers || tileCatalog[pmap[i][j].layers[highestPriorityLayer(i, j, true)]].drawPriority >= tileCatalog[surfaceTileType].drawPriority)
                ) {

                if ((tileCatalog[surfaceTileType].flags & T_IS_FIRE)
                    && !(tileCatalog[pmap[i][j].layers[layer]].flags & T_IS_FIRE)) {
                    pmap[i][j].flags |= CAUGHT_FIRE_THIS_TURN;
                }

                if ((tileCatalog[pmap[i][j].layers[layer]].flags & T_PATHING_BLOCKER)
                    != (tileCatalog[surfaceTileType].flags & T_PATHING_BLOCKER)) {

                    rogue.staleLoopMap = true;
                }

                pmap[i][j].layers[layer] = surfaceTileType; // Place the terrain!
                accomplishedSomething = true;

                if (refresh) {
                    refreshDungeonCell((pos){ i, j });
                    if (player.loc.x == i && player.loc.y == j && !player.status[STATUS_LEVITATING] && refresh) {
                        flavorMessage(tileFlavor(player.loc.x, player.loc.y));
                    }
                    if (pmap[i][j].flags & (HAS_MONSTER)) {
                        monst = monsterAtLoc((pos){ i, j });
                        applyInstantTileEffectsToCreature(monst);
                        if (rogue.gameHasEnded) {
                            return true;
                        }
                    }
                    if (tileCatalog[surfaceTileType].flags & T_IS_FIRE) {
                        if (pmap[i][j].flags & HAS_ITEM) {
                            theItem = itemAtLoc((pos){ i, j });
                            if (theItem->flags & ITEM_FLAMMABLE) {
                                burnItem(theItem);
                            }
                        }
                    }
                }
            } else {
                spawnMap[i][j] = false; // so that the spawnmap reflects what actually got built
            }
        }
    }
    return accomplishedSomething;
}

static void spawnMapDF(short x, short y,
                enum tileType propagationTerrain,
                boolean requirePropTerrain,
                short startProb,
                short probDec,
                char spawnMap[DCOLS][DROWS]) {

    short i, j, dir, t, x2, y2;
    boolean madeChange;

    spawnMap[x][y] = t = 1; // incremented before anything else happens

    madeChange = true;

    while (madeChange && startProb > 0) {
        madeChange = false;
        t++;
        for (i = 0; i < DCOLS; i++) {
            for (j=0; j < DROWS; j++) {
                if (spawnMap[i][j] == t - 1) {
                    for (dir = 0; dir < 4; dir++) {
                        x2 = i + nbDirs[dir][0];
                        y2 = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(x2, y2)
                            && (!requirePropTerrain || (propagationTerrain > 0 && cellHasTerrainType((pos){ x2, y2 }, propagationTerrain)))
                            && (!cellHasTerrainFlag((pos){ x2, y2 }, T_OBSTRUCTS_SURFACE_EFFECTS) || (propagationTerrain > 0 && cellHasTerrainType((pos){ x2, y2 }, propagationTerrain)))
                            && rand_percent(startProb)) {

                            spawnMap[x2][y2] = t;
                            madeChange = true;
                        }
                    }
                }
            }
        }
        startProb -= probDec;
        if (t > 100) {
            for (i = 0; i < DCOLS; i++) {
                for (j=0; j < DROWS; j++) {
                    if (spawnMap[i][j] == t) {
                        spawnMap[i][j] = 2;
                    } else if (spawnMap[i][j] > 0) {
                        spawnMap[i][j] = 1;
                    }
                }
            }
            t = 2;
        }
    }
    if (requirePropTerrain && !cellHasTerrainType((pos){ x, y }, propagationTerrain)) {
        spawnMap[x][y] = 0;
    }
}

static void evacuateCreatures(char blockingMap[DCOLS][DROWS]) {
    creature *monst;

    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            if (blockingMap[i][j]
                && (pmap[i][j].flags & (HAS_MONSTER | HAS_PLAYER))) {

                monst = monsterAtLoc((pos) { i, j });
                pos newLoc;
                getQualifyingLocNear(&newLoc,
                                     (pos){ i, j },
                                     true,
                                     blockingMap,
                                     forbiddenFlagsForMonster(&(monst->info)),
                                     (HAS_MONSTER | HAS_PLAYER),
                                     false,
                                     false);
                monst->loc = newLoc;
                pmap[i][j].flags &= ~(HAS_MONSTER | HAS_PLAYER);
                pmapAt(newLoc)->flags |= (monst == &player ? HAS_PLAYER : HAS_MONSTER);
            }
        }
    }
}

// returns whether the feature was successfully generated (false if we aborted because of blocking)
boolean spawnDungeonFeature(short x, short y, dungeonFeature *feat, boolean refreshCell, boolean abortIfBlocking) {
    short i, j, layer;
    char blockingMap[DCOLS][DROWS];
    boolean blocking;
    boolean succeeded;

    if ((feat->flags & DFF_RESURRECT_ALLY)
        && !resurrectAlly((pos){ x, y })) {
        return false;
    }

    if (feat->description[0] && !feat->messageDisplayed && playerCanSee(x, y)) {
        feat->messageDisplayed = true;
        message(feat->description, 0);
    }

    zeroOutGrid(blockingMap);

    // Blocking keeps track of whether to abort if it turns out that the DF would obstruct the level.
    blocking = ((abortIfBlocking
                 && !(feat->flags & DFF_PERMIT_BLOCKING)
                 && ((tileCatalog[feat->tile].flags & (T_PATHING_BLOCKER))
                     || (feat->flags & DFF_TREAT_AS_BLOCKING))) ? true : false);

    if (feat->tile) {
        if (feat->layer == GAS) {
            pmap[x][y].volume += feat->startProbability;
            pmap[x][y].layers[GAS] = feat->tile;
            if (refreshCell) {
                refreshDungeonCell((pos){ x, y });
            }
            succeeded = true;
        } else {
            spawnMapDF(x, y,
                       feat->propagationTerrain,
                       (feat->propagationTerrain ? true : false),
                       feat->startProbability,
                       feat->probabilityDecrement,
                       blockingMap);
            if (!blocking || !levelIsDisconnectedWithBlockingMap(blockingMap, false)) {
                if (feat->flags & DFF_EVACUATE_CREATURES_FIRST) { // first, evacuate creatures if necessary, so that they do not re-trigger the tile.
                    evacuateCreatures(blockingMap);
                }

                //succeeded = fillSpawnMap(feat->layer, feat->tile, blockingMap, (feat->flags & DFF_BLOCKED_BY_OTHER_LAYERS), refreshCell, (feat->flags & DFF_SUPERPRIORITY));
                fillSpawnMap(feat->layer,
                             feat->tile,
                             blockingMap,
                             (feat->flags & DFF_BLOCKED_BY_OTHER_LAYERS),
                             refreshCell,
                             (feat->flags & DFF_SUPERPRIORITY)); // this can tweak the spawn map too
                succeeded = true; // fail ONLY if we blocked the level. We succeed even if, thanks to priority, nothing gets built.
            } else {
                succeeded = false;
            }
        }
    } else {
        blockingMap[x][y] = true;
        succeeded = true; // Automatically succeed if there is no terrain to place.
        if (feat->flags & DFF_EVACUATE_CREATURES_FIRST) { // first, evacuate creatures if necessary, so that they do not re-trigger the tile.
            evacuateCreatures(blockingMap);
        }
    }

    if (succeeded && (feat->flags & (DFF_CLEAR_LOWER_PRIORITY_TERRAIN | DFF_CLEAR_OTHER_TERRAIN))) {
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (blockingMap[i][j]) {
                    for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                        if (layer != feat->layer && layer != GAS) {
                            if (feat->flags & DFF_CLEAR_LOWER_PRIORITY_TERRAIN) {
                                if (tileCatalog[pmap[i][j].layers[layer]].drawPriority <= tileCatalog[feat->tile].drawPriority) {
                                    continue;
                                }
                            }
                            pmap[i][j].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING);
                        }
                    }
                }
            }
        }
    }

    if (succeeded) {
        if ((feat->flags & DFF_AGGRAVATES_MONSTERS) && feat->effectRadius) {
            aggravateMonsters(feat->effectRadius, x, y, &gray);
        }
        if (refreshCell && feat->flashColor && feat->effectRadius) {
            colorFlash(feat->flashColor, 0, (IN_FIELD_OF_VIEW | CLAIRVOYANT_VISIBLE), 4, feat->effectRadius, x, y);
        }
        if (refreshCell && feat->lightFlare) {
            createFlare(x, y, feat->lightFlare);
        }
    }

    if (refreshCell
        && (tileCatalog[feat->tile].flags & (T_IS_FIRE | T_AUTO_DESCENT))
        && cellHasTerrainFlag(player.loc, (T_IS_FIRE | T_AUTO_DESCENT))) {

        applyInstantTileEffectsToCreature(&player);
    }
    if (rogue.gameHasEnded) {
        return succeeded;
    }
    //  if (succeeded && feat->description[0] && !feat->messageDisplayed && playerCanSee(x, y)) {
    //      feat->messageDisplayed = true;
    //      message(feat->description, 0);
    //  }
    if (succeeded) {
        if (feat->subsequentDF) {
            if (feat->flags & DFF_SUBSEQ_EVERYWHERE) {
                for (i=0; i<DCOLS; i++) {
                    for (j=0; j<DROWS; j++) {
                        if (blockingMap[i][j]) {
                            spawnDungeonFeature(i, j, &dungeonFeatureCatalog[feat->subsequentDF], refreshCell, abortIfBlocking);
                        }
                    }
                }
            } else {
                spawnDungeonFeature(x, y, &dungeonFeatureCatalog[feat->subsequentDF], refreshCell, abortIfBlocking);
            }
        }
        if (feat->tile
            && (tileCatalog[feat->tile].flags & (T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_AUTO_DESCENT))) {

            rogue.updatedMapToShoreThisTurn = false;
        }

        // awaken dormant creatures?
        if (feat->flags & DFF_ACTIVATE_DORMANT_MONSTER) {
            for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->loc.x == x && monst->loc.y == y || blockingMap[monst->loc.x][monst->loc.y]) {
                    // found it!
                    toggleMonsterDormancy(monst);
                }
            }
        }
    }
    return succeeded;
}

void restoreMonster(creature *monst, short **mapToStairs, short **mapToPit) {
    short i, *x, *y, turnCount;
    boolean foundLeader = false;
    short **theMap;
    enum directions dir;

    x = &(monst->loc.x);
    y = &(monst->loc.y);

    if (monst->status[STATUS_ENTERS_LEVEL_IN] > 0) {
        if (monst->bookkeepingFlags & (MB_APPROACHING_PIT)) {
            theMap = mapToPit;
        } else {
            theMap = mapToStairs;
        }

        pmap[*x][*y].flags &= ~HAS_MONSTER;
        if (theMap) {
            // STATUS_ENTERS_LEVEL_IN accounts for monster speed; convert back to map distance and subtract from distance to stairs
            turnCount = (theMap[monst->loc.x][monst->loc.y] - (monst->status[STATUS_ENTERS_LEVEL_IN] * 100 / monst->movementSpeed));
            for (i=0; i < turnCount; i++) {
                if ((dir = nextStep(theMap, monst->loc.x, monst->loc.y, NULL, true)) != NO_DIRECTION) {
                    monst->loc.x += nbDirs[dir][0];
                    monst->loc.y += nbDirs[dir][1];
                } else {
                    break;
                }
            }
        }
        monst->bookkeepingFlags |= MB_PREPLACED;
    }

    if ((pmap[*x][*y].flags & (HAS_PLAYER | HAS_STAIRS))
        || (monst->bookkeepingFlags & MB_PREPLACED)) {

        if (!(monst->bookkeepingFlags & MB_PREPLACED)) {
            // (If if it's preplaced, it won't have set the HAS_MONSTER flag in the first place,
            // so clearing it might screw up an existing monster.)
            pmap[*x][*y].flags &= ~HAS_MONSTER;
        }
        getQualifyingPathLocNear(x, y, *x, *y, true, T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)), 0,
                                 avoidedFlagsForMonster(&(monst->info)), (HAS_MONSTER | HAS_PLAYER | HAS_STAIRS | IS_IN_MACHINE), true);
    }
    pmap[*x][*y].flags |= HAS_MONSTER;
    monst->bookkeepingFlags &= ~(MB_PREPLACED | MB_APPROACHING_DOWNSTAIRS | MB_APPROACHING_UPSTAIRS | MB_APPROACHING_PIT | MB_ABSORBING);
    monst->status[STATUS_ENTERS_LEVEL_IN] = 0;
    monst->corpseAbsorptionCounter = 0;

    if ((monst->bookkeepingFlags & MB_SUBMERGED) && !cellHasTMFlag((pos){ *x, *y }, TM_ALLOWS_SUBMERGING)) {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }

    if (monst->bookkeepingFlags & MB_FOLLOWER) {
        // is the leader on the same level?
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *leader = nextCreature(&it);
            if (leader == monst->leader) {
                foundLeader = true;
                break;
            }
        }
        // if not, it is time to spread your wings and fly solo
        if (!foundLeader) {
            monst->bookkeepingFlags &= ~MB_FOLLOWER;
            monst->leader = NULL;
        }
    }
}

void restoreItems() {
    item *theItem, *nextItem;
    pos loc;
    item preplaced;
    preplaced.nextItem = NULL;

    // Remove preplaced items from the floor chain
    for (theItem = floorItems->nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;

        if (theItem->flags & ITEM_PREPLACED) {
            theItem->flags &= ~ITEM_PREPLACED;
            removeItemFromChain(theItem, floorItems);
            addItemToChain(theItem, &preplaced);
        }
    }

    // Place items properly
    for (theItem = preplaced.nextItem; theItem != NULL; theItem = nextItem) {
        nextItem = theItem->nextItem;

        // Items can fall into deep water, enclaved lakes, another chasm, even lava!
        getQualifyingLocNear(&loc, theItem->loc, true, 0,
                            (T_OBSTRUCTS_ITEMS),
                            (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), false, false);
        placeItemAt(theItem, loc);
    }
}

// Returns true iff the location is a plain wall, three of the four cardinal neighbors are walls, the remaining cardinal neighbor
// is not a pathing blocker, the two diagonals between the three cardinal walls are also walls, and none of the eight neighbors are in machines.
static boolean validStairLoc(short x, short y) {
    short newX, newY, dir, neighborWallCount;

    if (x < 1 || x >= DCOLS - 1 || y < 1 || y >= DROWS - 1 || pmap[x][y].layers[DUNGEON] != WALL) {
        return false;
    }

    for (dir=0; dir< DIRECTION_COUNT; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (pmap[newX][newY].flags & IS_IN_MACHINE) {
            return false;
        }
    }

    neighborWallCount = 0;
    for (dir=0; dir<4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];

        if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
            // neighbor is a wall
            neighborWallCount++;
        } else {
            // neighbor is not a wall
            if (cellHasTerrainFlag((pos){ newX, newY }, T_PATHING_BLOCKER)
                || passableArcCount(newX, newY) >= 2) {
                return false;
            }
            // now check the two diagonals between the walls

            newX = x - nbDirs[dir][0] + nbDirs[dir][1];
            newY = y - nbDirs[dir][1] + nbDirs[dir][0];
            if (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
                return false;
            }

            newX = x - nbDirs[dir][0] - nbDirs[dir][1];
            newY = y - nbDirs[dir][1] - nbDirs[dir][0];
            if (!cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
                return false;
            }
        }
    }
    if (neighborWallCount != 3) {
        return false;
    }
    return true;
}

// The walls on either side become torches. Any adjacent granite then becomes top_wall. All wall neighbors are un-tunnelable.
// Grid is zeroed out within 5 spaces in all directions.
static void prepareForStairs(short x, short y, char grid[DCOLS][DROWS]) {
    short newX, newY, dir;

    // Add torches to either side.
    for (dir=0; dir<4; dir++) {
        if (!cellHasTerrainFlag((pos){ x + nbDirs[dir][0], y + nbDirs[dir][1] }, T_OBSTRUCTS_PASSABILITY)) {
            newX = x - nbDirs[dir][1];
            newY = y - nbDirs[dir][0];
            pmap[newX][newY].layers[DUNGEON] = TORCH_WALL;
            newX = x + nbDirs[dir][1];
            newY = y + nbDirs[dir][0];
            pmap[newX][newY].layers[DUNGEON] = TORCH_WALL;
            break;
        }
    }
    // Expose granite.
    for (dir=0; dir< DIRECTION_COUNT; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (pmap[newX][newY].layers[DUNGEON] == GRANITE) {
            pmap[newX][newY].layers[DUNGEON] = WALL;
        }
        if (cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_PASSABILITY)) {
            pmap[newX][newY].flags |= IMPREGNABLE;
        }
    }
    // Zero out grid in the vicinity.
    for (newX = max(0, x - 5); newX < min(DCOLS, x + 5); newX++) {
        for (newY = max(0, y - 5); newY < min(DROWS, y + 5); newY++) {
            grid[newX][newY] = false;
        }
    }
}

boolean placeStairs(pos *upStairsLoc) {
    char grid[DCOLS][DROWS];
    short n = rogue.depthLevel - 1;

    for (int i=0; i < DCOLS; i++) {
        for (int j=0; j < DROWS; j++) {
            grid[i][j] = validStairLoc(i, j);
        }
    }

    if (D_INSPECT_LEVELGEN) {
        dumpLevelToScreen();
        hiliteCharGrid(grid, &teal, 100);
        temporaryMessage("Stair location candidates:", REQUIRE_ACKNOWLEDGMENT);
    }

    pos downLoc;
    if (getQualifyingGridLocNear(&downLoc, levels[n].downStairsLoc, grid, false)) {
        prepareForStairs(downLoc.x, downLoc.y, grid);
    } else {
        boolean hasQualifyingLoc = getQualifyingLocNear(&downLoc, levels[n].downStairsLoc, false, 0,
                                                        (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_AUTO_DESCENT | T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_IS_DF_TRAP),
                                                        (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), true, false);
        if (!hasQualifyingLoc) {
            return false;
        }
    }

    if (rogue.depthLevel == gameConst->deepestLevel) {
        pmapAt(downLoc)->layers[DUNGEON] = DUNGEON_PORTAL;
    } else {
        pmapAt(downLoc)->layers[DUNGEON] = DOWN_STAIRS;
    }
    pmapAt(downLoc)->layers[LIQUID]     = NOTHING;
    pmapAt(downLoc)->layers[SURFACE]    = NOTHING;

    if (!levels[n+1].visited) {
        levels[n+1].upStairsLoc = downLoc;
    }
    levels[n].downStairsLoc = downLoc;

    pos upLoc;
    if (getQualifyingGridLocNear(&upLoc, levels[n].upStairsLoc, grid, false)) {
        prepareForStairs(upLoc.x, upLoc.y, grid);
    } else { // Hopefully this never happens.
        boolean hasQualifyingLoc;
        hasQualifyingLoc = getQualifyingLocNear(&upLoc, levels[n].upStairsLoc, false, 0,
                             (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_AUTO_DESCENT | T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH | T_IS_DF_TRAP),
                             (HAS_MONSTER | HAS_ITEM | HAS_STAIRS | IS_IN_MACHINE), true, false);
        if (!hasQualifyingLoc) {
            return false;
        }
    }

    levels[n].upStairsLoc = upLoc;

    if (rogue.depthLevel == 1) {
        pmapAt(upLoc)->layers[DUNGEON] = DUNGEON_EXIT;
    } else {
        pmapAt(upLoc)->layers[DUNGEON] = UP_STAIRS;
    }
    pmapAt(upLoc)->layers[LIQUID] = NOTHING;
    pmapAt(upLoc)->layers[SURFACE] = NOTHING;

    rogue.downLoc = downLoc;
    pmapAt(downLoc)->flags |= HAS_STAIRS;
    rogue.upLoc = upLoc;
    pmapAt(upLoc)->flags |= HAS_STAIRS;

    *upStairsLoc = upLoc;
    return true;
}

// Places the player, monsters, items and stairs.
void initializeLevel(pos upStairsLoc) {
    short i, j, dir;
    short **mapToStairs, **mapToPit;
    char grid[DCOLS][DROWS];

    pos upLoc = upStairsLoc;

    if (!levels[rogue.depthLevel-1].visited) {

        // Run a field of view check from up stairs so that monsters do not spawn within sight of it.
        for (dir=0; dir<4; dir++) {
            pos nextLoc = posNeighborInDirection(upLoc, dir);
            if (isPosInMap(nextLoc) && !cellHasTerrainFlag(nextLoc, T_OBSTRUCTS_PASSABILITY)) {
                upLoc = nextLoc;
                break;
            }
        }
        zeroOutGrid(grid);
        getFOVMask(grid, upLoc.x, upLoc.y, max(DCOLS, DROWS) * FP_FACTOR, (T_OBSTRUCTS_VISION), 0, false);
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                if (grid[i][j]) {
                    pmap[i][j].flags |= IN_FIELD_OF_VIEW;
                }
            }
        }
        populateItems(upLoc);
        populateMonsters();
    }

    // Restore items that fell from the previous depth.
    restoreItems();

    // Restore creatures that fell from the previous depth or that have been pathing toward the stairs.
    mapToStairs = allocGrid();
    fillGrid(mapToStairs, 0);
    mapToPit = allocGrid();
    fillGrid(mapToPit, 0);
    calculateDistances(mapToStairs, player.loc.x, player.loc.y, T_PATHING_BLOCKER, NULL, true, true);
    calculateDistances(mapToPit,
                       levels[rogue.depthLevel - 1].playerExitedVia.x,
                       levels[rogue.depthLevel - 1].playerExitedVia.y,
                       T_PATHING_BLOCKER,
                       NULL,
                       true,
                       true);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        restoreMonster(monst, mapToStairs, mapToPit);
    }
    freeGrid(mapToStairs);
    freeGrid(mapToPit);
}

// fills (*x, *y) with the coordinates of a random cell with
// no creatures, items or stairs and with either a matching liquid and dungeon type
// or at least one layer of type terrainType.
// A dungeon, liquid type of -1 will match anything.
boolean randomMatchingLocation(pos* loc, short dungeonType, short liquidType, short terrainType) {
    short failsafeCount = 0;
    do {
        failsafeCount++;
        loc->x = rand_range(0, DCOLS - 1);
        loc->y = rand_range(0, DROWS - 1);
    } while (failsafeCount < 500 && ((terrainType >= 0 && !cellHasTerrainType(*loc, terrainType))
                                     || (((dungeonType >= 0 && pmapAt(*loc)->layers[DUNGEON] != dungeonType) || (liquidType >= 0 && pmapAt(*loc)->layers[LIQUID] != liquidType)) && terrainType < 0)
                                     || (pmapAt(*loc)->flags & (HAS_PLAYER | HAS_MONSTER | HAS_STAIRS | HAS_ITEM | IS_IN_MACHINE))
                                     || (terrainType < 0 && !(tileCatalog[dungeonType].flags & T_OBSTRUCTS_ITEMS)
                                         && cellHasTerrainFlag(*loc, T_OBSTRUCTS_ITEMS))));
    if (failsafeCount >= 500) {
        return false;
    }
    return true;
}

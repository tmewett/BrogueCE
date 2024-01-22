/*
 *  Dijkstra.c
 *  Brogue
 *
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

// This code was created by Joshua Day, to replace my clunkier and slower Dijkstra scanning algorithm in Movement.c.
// Thanks very much, Joshua.

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

typedef struct pdsLink {
    short distance;
    short cost;
    struct pdsLink *left;
    struct pdsLink *right;
} pdsLink;

typedef struct pdsMap {
    pdsLink front;
    pdsLink links[DCOLS * DROWS];
} pdsMap;

static void pdsUpdate(pdsMap *map, boolean useDiagonals) {
    short dirs = useDiagonals ? 8 : 4;

    pdsLink *head = map->front.right;
    map->front.right = NULL;

    while (head != NULL) {
        for (short dir = 0; dir < dirs; dir++) {
            pdsLink *link = head + (nbDirs[dir][0] + DCOLS * nbDirs[dir][1]);
            if (link < map->links || link >= map->links + DCOLS * DROWS) continue;

            // verify passability
            if (link->cost < 0) continue;
            if (dir >= 4) {
                pdsLink *way1 = head + nbDirs[dir][0];
                pdsLink *way2 = head + DCOLS * nbDirs[dir][1];
                if (way1->cost == PDS_OBSTRUCTION || way2->cost == PDS_OBSTRUCTION) continue;
            }

            if (head->distance + link->cost < link->distance) {
                link->distance = head->distance + link->cost;

                // reinsert the touched cell; it'll be close to the beginning of the list now, so
                // this will be very fast.  start by removing it.

                if (link->right != NULL) link->right->left = link->left;
                if (link->left != NULL) link->left->right = link->right;

                pdsLink *left = head;
                pdsLink *right = head->right;
                while (right != NULL && right->distance < link->distance) {
                    left = right;
                    right = right->right;
                }
                if (left != NULL) left->right = link;
                link->right = right;
                link->left = left;
                if (right != NULL) right->left = link;
            }
        }

        pdsLink *right = head->right;

        head->left = NULL;
        head->right = NULL;

        head = right;
    }
}

static void pdsClear(pdsMap *map, short maxDistance) {
    map->front.right = NULL;

    for (int i=0; i < DCOLS*DROWS; i++) {
        map->links[i].distance = maxDistance;
        map->links[i].left = NULL;
        map->links[i].right = NULL;
    }
}

static void pdsSetDistance(pdsMap *map, short x, short y, short distance) {
    if (x > 0 && y > 0 && x < DCOLS - 1 && y < DROWS - 1) {
        pdsLink *link = PDS_CELL(map, x, y);
        if (link->distance > distance) {
            link->distance = distance;

            if (link->right != NULL) link->right->left = link->left;
            if (link->left != NULL) link->left->right = link->right;

            pdsLink *left = &map->front;
            pdsLink *right = map->front.right;

            while (right != NULL && right->distance < link->distance) {
                left = right;
                right = right->right;
            }

            link->right = right;
            link->left = left;
            left->right = link;
            if (right != NULL) right->left = link;
        }
    }
}

static void pdsBatchInput(pdsMap *map, short **distanceMap, short **costMap, short maxDistance) {
    pdsLink *left = NULL;
    pdsLink *right = NULL;

    map->front.right = NULL;
    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            pdsLink *link = PDS_CELL(map, i, j);

            if (distanceMap != NULL) {
                link->distance = distanceMap[i][j];
            } else {
                if (costMap != NULL) {
                    // totally hackish; refactor
                    link->distance = maxDistance;
                }
            }

            int cost;

            if (i == 0 || j == 0 || i == DCOLS - 1 || j == DROWS - 1) {
                cost = PDS_OBSTRUCTION;
            } else if (costMap == NULL) {
                if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY) && cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT)) cost = PDS_OBSTRUCTION;
                else cost = PDS_FORBIDDEN;
            } else {
                cost = costMap[i][j];
            }

            link->cost = cost;

            if (cost > 0) {
                if (link->distance < maxDistance) {
                    if (right == NULL || right->distance > link->distance) {
                        // left and right are used to traverse the list; if many cells have similar values,
                        // some time can be saved by not clearing them with each insertion.  this time,
                        // sadly, we have to start from the front.

                        left = &map->front;
                        right = map->front.right;
                    }

                    while (right != NULL && right->distance < link->distance) {
                        left = right;
                        right = right->right;
                    }

                    link->right = right;
                    link->left = left;
                    left->right = link;
                    if (right != NULL) right->left = link;

                    left = link;
                } else {
                    link->right = NULL;
                    link->left = NULL;
                }
            } else {
                link->right = NULL;
                link->left = NULL;
            }
        }
    }
}

static void pdsBatchOutput(pdsMap *map, short **distanceMap, boolean useDiagonals) {
    pdsUpdate(map, useDiagonals);
    // transfer results to the distanceMap
    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            distanceMap[i][j] = PDS_CELL(map, i, j)->distance;
        }
    }
}

void dijkstraScan(short **distanceMap, short **costMap, boolean useDiagonals) {
    static pdsMap map;

    pdsBatchInput(&map, distanceMap, costMap, 30000);
    pdsBatchOutput(&map, distanceMap, useDiagonals);
}

void calculateDistances(short **distanceMap,
                        short destinationX, short destinationY,
                        unsigned long blockingTerrainFlags,
                        creature *traveler,
                        boolean canUseSecretDoors,
                        boolean eightWays) {
    static pdsMap map;

    for (int i=0; i<DCOLS; i++) {
        for (int j=0; j<DROWS; j++) {
            signed char cost;
            creature *monst = monsterAtLoc((pos){ i, j });
            if (monst
                && (monst->info.flags & (MONST_IMMUNE_TO_WEAPONS | MONST_INVULNERABLE))
                && (monst->info.flags & (MONST_IMMOBILE | MONST_GETS_TURN_ON_ACTIVATION))) {

                // Always avoid damage-immune stationary monsters.
                cost = PDS_FORBIDDEN;
            } else if (canUseSecretDoors
                && cellHasTMFlag((pos){ i, j }, TM_IS_SECRET)
                && cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && !(discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY)) {

                cost = 1;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                       || (traveler && traveler == &player && !(pmap[i][j].flags & (DISCOVERED | MAGIC_MAPPED)))) {

                cost = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
            } else if ((traveler && monsterAvoids(traveler, (pos){i, j})) || cellHasTerrainFlag((pos){ i, j }, blockingTerrainFlags)) {
                cost = PDS_FORBIDDEN;
            } else {
                cost = 1;
            }

            PDS_CELL(&map, i, j)->cost = cost;
        }
    }

    pdsClear(&map, 30000);
    pdsSetDistance(&map, destinationX, destinationY, 0);
    pdsBatchOutput(&map, distanceMap, eightWays);
}

short pathingDistance(short x1, short y1, short x2, short y2, unsigned long blockingTerrainFlags) {
    short **distanceMap = allocGrid();
    calculateDistances(distanceMap, x2, y2, blockingTerrainFlags, NULL, true, true);
    short retval = distanceMap[x1][y1];
    freeGrid(distanceMap);
    return retval;
}


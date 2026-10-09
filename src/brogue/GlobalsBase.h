/*
 *  GlobalsBase.h
 *  Brogue
 *
 *  Created by Brian Walker on 2/8/09.
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


extern tcell tmap[DCOLS][DROWS];                        // grids with info about the map
extern pcell pmap[DCOLS][DROWS];                        // grids with info about the map

// Returns a pointer to the `tcell` at the given position. The position must be in-bounds.
static inline tcell* tmapAt(pos p) {
  brogueAssert(p.x >= 0 && p.x < DCOLS && p.y >= 0 && p.y < DROWS);
  return &tmap[p.x][p.y];
}
// Returns a pointer to the `pcell` at the given position. The position must be in-bounds.
static inline pcell* pmapAt(pos p) {
  brogueAssert(p.x >= 0 && p.x < DCOLS && p.y >= 0 && p.y < DROWS);
  return &pmap[p.x][p.y];
}

extern const short nbDirs[8][2];

// Returns the `pos` which is one cell away in the provided direction.
// The direction must not be `NO_DIRECTION`.
static inline pos posNeighborInDirection(pos p, enum directions direction_to_step) {
  brogueAssert(direction_to_step >= 0 && direction_to_step < 8);
  return (pos) { .x = p.x + nbDirs[direction_to_step][0], .y = p.y + nbDirs[direction_to_step][1] };
}

extern short **scentMap;
extern screenDisplayBuffer displayBuffer;
extern short terrainRandomValues[DCOLS][DROWS][8];
extern short **safetyMap;                                       // used to help monsters flee
extern short **allySafetyMap;
extern short **chokeMap;

extern const short nbDirs[8][2];
extern const short cDirs[8][2];
extern levelData *levels;
extern creature player;
extern playerCharacter rogue;
extern const gameConstants *gameConst;
extern int gameVariant;
extern creatureList *monsters;
extern creatureList *dormantMonsters;
extern creatureList purgatory;
extern item *floorItems;
extern item *packItems;
extern item *monsterItemsHopper;
extern short numberOfWaypoints;

extern char displayedMessage[MESSAGE_LINES][COLS*2];
extern short messagesUnconfirmed;
extern char combatText[COLS*2];
extern short messageArchivePosition;
extern archivedMessage messageArchive[MESSAGE_ARCHIVE_ENTRIES];

extern char currentFilePath[BROGUE_FILENAME_MAX];
extern unsigned long randomNumbersGenerated;

extern char displayDetail[DCOLS][DROWS];

#ifdef AUDIT_RNG
extern FILE *RNGLogFile;
#endif

#define MENU_TITLE_WIDTH    68
#define MENU_TITLE_HEIGHT   26

extern unsigned char inputRecordBuffer[INPUT_RECORD_BUFFER + 100];
extern unsigned short locationInRecordingBuffer;

extern unsigned long positionInPlaybackFile;
extern unsigned long lengthOfPlaybackFile;
extern unsigned long recordingLocation;
extern unsigned long maxLevelChanges;
extern char annotationPathname[BROGUE_FILENAME_MAX];    // pathname of annotation file
extern uint64_t previousGameSeed;

// basic colors
extern const color white;
extern const color red;
extern const color blue;
extern const color darkRed;
extern const color gray;
extern const color darkGray;
extern const color veryDarkGray;
extern const color black;
extern const color yellow;
extern const color darkYellow;
extern const color teal;
extern const color purple;
extern const color darkPurple;
extern const color brown;
extern const color green;
extern const color darkGreen;
extern const color orange;
extern const color darkOrange;
extern const color darkBlue;
extern const color darkTurquoise;
extern const color lightBlue;
extern const color pink;
extern const color darkPink;
extern const color tanColor;
extern const color rainbow;

extern const monsterBehavior monsterBehaviorCatalog[32];
extern const monsterAbility monsterAbilityCatalog[32];
extern const char monsterBookkeepingFlagDescriptions[32][COLS];

extern const windowpos WINDOW_POSITION_DUNGEON_TOP_LEFT;

// charm constants
#define CHARM_EFFECT_DURATION_INCREMENT_ARRAY_SIZE 50
extern const fixpt POW_0_CHARM_INCREMENT[];
extern const fixpt POW_120_CHARM_INCREMENT[];
extern const fixpt POW_125_CHARM_INCREMENT[];

/*
 *  Include Globals.h
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

extern short **scentMap;
extern cellDisplayBuffer displayBuffer[COLS][ROWS];
extern short terrainRandomValues[DCOLS][DROWS][8];
extern short **safetyMap;                                       // used to help monsters flee
extern short **allySafetyMap;
extern short **chokeMap;

extern const short nbDirs[8][2];
extern const short cDirs[8][2];
extern levelData *levels;
extern creature player;
extern playerCharacter rogue;
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

extern unsigned char inputRecordBuffer[INPUT_RECORD_BUFFER + 100];
extern unsigned short locationInRecordingBuffer;

extern unsigned long positionInPlaybackFile;
extern unsigned long lengthOfPlaybackFile;
extern unsigned long recordingLocation;
extern unsigned long maxLevelChanges;
extern char annotationPathname[BROGUE_FILENAME_MAX];    // pathname of annotation file
extern uint64_t previousGameSeed;

// basic colors
extern color white;
extern color red;
extern color blue;
extern color darkRed;
extern color gray;
extern color darkGray;
extern color veryDarkGray;
extern color black;
extern color yellow;
extern color darkYellow;
extern color teal;
extern color purple;
extern color darkPurple;
extern color brown;
extern color green;
extern color darkGreen;
extern color orange;
extern color darkOrange;
extern color darkBlue;
extern color darkTurquoise;
extern color lightBlue;
extern color pink;
extern color darkPink;
extern color tanColor;
extern color rainbow;

// tile colors
extern color undiscoveredColor;

extern const char monsterBehaviorFlagDescriptions[32][COLS];
extern const char monsterAbilityFlagDescriptions[32][COLS];
extern const char monsterBookkeepingFlagDescriptions[32][COLS];

extern const pos WINDOW_POSITION_DUNGEON_TOP_LEFT;

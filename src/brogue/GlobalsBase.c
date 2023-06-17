/*
 *  GlobalsBase.c
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

tcell tmap[DCOLS][DROWS];                       // grids with info about the map
pcell pmap[DCOLS][DROWS];
short **scentMap;
cellDisplayBuffer displayBuffer[COLS][ROWS];    // used to optimize plotCharWithColor
short terrainRandomValues[DCOLS][DROWS][8];
short **safetyMap;                              // used to help monsters flee
short **allySafetyMap;                          // used to help allies flee
short **chokeMap;                               // used to assess the importance of the map's various chokepoints
const short nbDirs[8][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}, {-1,-1}, {-1,1}, {1,-1}, {1,1}};
const short cDirs[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
short numberOfWaypoints;
levelData *levels;
creature player;
playerCharacter rogue;
gameConstants gameConst;
int gameVariant = VARIANT_BROGUE;
creatureList *monsters;
creatureList *dormantMonsters;
creatureList purgatory;
item *floorItems;
item *packItems;
item *monsterItemsHopper;

char displayedMessage[MESSAGE_LINES][COLS*2];
short messagesUnconfirmed;
char combatText[COLS * 2];
short messageArchivePosition;
archivedMessage messageArchive[MESSAGE_ARCHIVE_ENTRIES];

char currentFilePath[BROGUE_FILENAME_MAX];

char displayDetail[DCOLS][DROWS];       // used to make certain per-cell data accessible to external code (e.g. terminal adaptations)

#ifdef AUDIT_RNG
FILE *RNGLogFile;
#endif

unsigned char inputRecordBuffer[INPUT_RECORD_BUFFER + 100];
unsigned short locationInRecordingBuffer;
unsigned long randomNumbersGenerated;
unsigned long positionInPlaybackFile;
unsigned long lengthOfPlaybackFile;
unsigned long recordingLocation;
unsigned long maxLevelChanges;
char annotationPathname[BROGUE_FILENAME_MAX];   // pathname of annotation file
uint64_t previousGameSeed;

const pos WINDOW_POSITION_DUNGEON_TOP_LEFT = { STAT_BAR_WIDTH + 3, MESSAGE_LINES + 2};

//                                  Red     Green   Blue    RedRand GreenRand   BlueRand    Rand    Dances?
// basic colors
const color white =                 {100,   100,    100,    0,      0,          0,          0,      false};
const color gray =                  {50,    50,     50,     0,      0,          0,          0,      false};
const color darkGray =              {30,    30,     30,     0,      0,          0,          0,      false};
const color veryDarkGray =          {15,    15,     15,     0,      0,          0,          0,      false};
const color black =                 {0,     0,      0,      0,      0,          0,          0,      false};
const color yellow =                {100,   100,    0,      0,      0,          0,          0,      false};
const color darkYellow =            {50,    50,     0,      0,      0,          0,          0,      false};
const color teal =                  {30,    100,    100,    0,      0,          0,          0,      false};
const color purple =                {100,   0,      100,    0,      0,          0,          0,      false};
const color darkPurple =            {50,    0,      50,     0,      0,          0,          0,      false};
const color brown =                 {60,    40,     0,      0,      0,          0,          0,      false};
const color green =                 {0,     100,    0,      0,      0,          0,          0,      false};
const color darkGreen =             {0,     50,     0,      0,      0,          0,          0,      false};
const color orange =                {100,   50,     0,      0,      0,          0,          0,      false};
const color darkOrange =            {50,    25,     0,      0,      0,          0,          0,      false};
const color blue =                  {0,     0,      100,    0,      0,          0,          0,      false};
const color darkBlue =              {0,     0,      50,     0,      0,          0,          0,      false};
const color darkTurquoise =         {0,     40,     65,     0,      0,          0,          0,      false};
const color lightBlue =             {40,    40,     100,    0,      0,          0,          0,      false};
const color pink =                  {100,   60,     66,     0,      0,          0,          0,      false};
const color darkPink =              {50,    30,     33,     0,      0,          0,          0,      false};
const color red  =                  {100,   0,      0,      0,      0,          0,          0,      false};
const color darkRed =               {50,    0,      0,      0,      0,          0,          0,      false};
const color tanColor =              {80,    67,     15,     0,      0,          0,          0,      false};

// basic colors
const color undiscoveredColor =     {0,     0,      0,      0,      0,          0,          0,      false};

const color rainbow =               {-70,   -70,    -70,    170,    170,        170,        0,      true};

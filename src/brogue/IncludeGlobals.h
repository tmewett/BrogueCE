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
#include "Rogue.h"

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

// Returns the `pos` which is one cell away in the provided direction.
// The direction must not be `NO_DIRECTION`.
static inline pos posNeighborInDirection(pos p, enum directions direction_to_step) {
  brogueAssert(direction_to_step >= 0 && direction_to_step < 8);
  return (pos) { .x = p.x + nbDirs[direction_to_step][0], .y = p.y + nbDirs[direction_to_step][1] };
}

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

extern const color fireForeColor;

// light colors
color minersLightColor; //dynamic
extern const color torchLightColor;
extern const color deepWaterLightColor;

extern const color discordColor;
extern const color memoryColor;
extern const color memoryOverlay;
extern const color magicMapColor;
extern const color clairvoyanceColor;
extern const color telepathyMultiplier;
extern const color omniscienceColor;
extern const color basicLightColor;

extern const color colorMultiplier100;

extern const color magicMapFlashColor;

extern const color itemColor;
extern const color blueBar;
extern const color redBar;
extern const color hiliteColor;
extern const color interfaceBoxColor;
extern const color interfaceButtonColor;

extern const color playerInvisibleColor;
extern const color playerInShadowColor;
extern const color playerInLightColor;
extern const color playerInDarknessColor;

extern const color inDarknessMultiplierColor;

extern const color buttonHoverColor;
extern const color titleButtonColor;

// other colors
extern const color confusionGasColor;
extern const color spectralImageColor;

extern const color goodMessageColor;
extern const color badMessageColor;
extern const color advancementMessageColor;
extern const color itemMessageColor;
extern const color backgroundMessageColor;

extern const color flavorTextColor;

extern const color flameSourceColor;
extern const color flameSourceColorSecondary;
extern const color flameTitleColor;

extern const color superVictoryColor;

extern color *dynamicColors[NUMBER_DYNAMIC_COLORS];
extern const color *dynamicColorsBounds[NUMBER_DYNAMIC_COLORS][2];

extern const autoGenerator *autoGeneratorCatalog;
extern const short *lumenstoneDistribution;
extern const short *itemGenerationProbabilities;

extern const floorTileType tileCatalog[NUMBER_TILETYPES];

extern dungeonFeature dungeonFeatureCatalog[NUMBER_DUNGEON_FEATURES];
extern const dungeonProfile dungeonProfileCatalog[NUMBER_DUNGEON_PROFILES];

extern const lightSource lightCatalog[NUMBER_LIGHT_KINDS];

extern const blueprint *blueprintCatalog;

extern creatureType monsterCatalog[NUMBER_MONSTER_KINDS];
extern const monsterWords monsterText[NUMBER_MONSTER_KINDS];
extern const hordeType *hordeCatalog;
extern const mutation mutationCatalog[NUMBER_MUTATORS];
extern const monsterClass monsterClassCatalog[MONSTER_CLASS_COUNT];

extern const feat featTable[FEAT_COUNT];

// ITEMS
extern char itemTitles[NUMBER_ITEM_TITLES][30];
extern char itemColors[NUMBER_ITEM_COLORS][30];
extern char itemWoods[NUMBER_ITEM_WOODS][30];
extern char itemMetals[NUMBER_ITEM_METALS][30];
extern char itemGems[NUMBER_ITEM_GEMS][30];

extern const char itemCategoryNames[NUMBER_ITEM_CATEGORIES][11];
extern const char titlePhonemes[NUMBER_TITLE_PHONEMES][30];
extern const char itemColorsRef[NUMBER_ITEM_COLORS][30];
extern const char itemWoodsRef[NUMBER_ITEM_WOODS][30];
extern const char itemMetalsRef[NUMBER_ITEM_METALS][30];
extern const char itemGemsRef[NUMBER_ITEM_GEMS][30];

extern itemTable keyTable[NUMBER_KEY_TYPES];
extern itemTable foodTable[NUMBER_FOOD_KINDS];
extern itemTable weaponTable[NUMBER_WEAPON_KINDS];
extern itemTable armorTable[NUMBER_ARMOR_KINDS];
extern itemTable *potionTable;
extern itemTable *scrollTable;
extern itemTable *wandTable;
extern itemTable staffTable[NUMBER_STAFF_KINDS];
extern itemTable ringTable[NUMBER_RING_KINDS];
extern itemTable charmTable[NUMBER_CHARM_KINDS];
extern const meteredItemGenerationTable *meteredItemsGenerationTable;

extern const bolt *boltCatalog;

extern const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30];

extern const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30];

extern const char *mainMenuTitle;
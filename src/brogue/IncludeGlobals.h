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
extern color lightBlue;
extern color pink;
extern color darkPink;
extern color tanColor;
extern color sunlight;
extern color rainbow;

// tile colors
extern color undiscoveredColor;

extern color wallForeColor;
extern color wallBackColorStart;
extern color wallBackColorEnd;
extern color wallBackColor;
extern color graniteBackColor;
extern color floorForeColor;
extern color floorBackColor;
extern color doorForeColor;
extern color doorBackColor;

extern color deepWaterForeColor;
extern color deepWaterBackColor;
extern color shallowWaterForeColor;
extern color shallowWaterBackColor;
extern color mudForeColor;
extern color mudBackColor;
extern color chasmForeColor;
extern color chasmEdgeBackColor;
extern color fireForeColor;

// light colors
extern color minersLightColor;
extern color minersLightStartColor;
extern color minersLightEndColor;
extern color torchLightColor;
extern color deepWaterLightColor;
extern color redFlashColor;

extern color discordColor;
extern color memoryColor;
extern color memoryOverlay;
extern color magicMapColor;
extern color clairvoyanceColor;
extern color telepathyMultiplier;
extern color omniscienceColor;
extern color basicLightColor;

extern color colorDim25;
extern color colorMultiplier100;

extern color magicMapFlashColor;

extern color itemColor;
extern color blueBar;
extern color redBar;
extern color hiliteColor;
extern color interfaceBoxColor;
extern color interfaceButtonColor;

extern color playerInvisibleColor;
extern color playerInShadowColor;
extern color playerInLightColor;
extern color playerInDarknessColor;

extern const color inLightMultiplierColor;
extern const color inDarknessMultiplierColor;

extern const color buttonHoverColor;
extern color titleButtonColor;

// other colors
extern color centipedeColor;
extern color confusionGasColor;
extern color lightningColor;
extern color spectralImageColor;

extern color goodMessageColor;
extern color badMessageColor;
extern color advancementMessageColor;
extern color itemMessageColor;
extern color backgroundMessageColor;

extern color flavorTextColor;

extern const color flameSourceColor;
extern const color flameSourceColorSecondary;
extern const color flameTitleColor;

extern const color superVictoryColor;

extern color *dynamicColors[NUMBER_DYNAMIC_COLORS][3];

extern const autoGenerator autoGeneratorCatalog[NUMBER_AUTOGENERATORS];

extern floorTileType tileCatalog[NUMBER_TILETYPES];

extern dungeonFeature dungeonFeatureCatalog[NUMBER_DUNGEON_FEATURES];
extern dungeonProfile dungeonProfileCatalog[NUMBER_DUNGEON_PROFILES];

extern lightSource lightCatalog[NUMBER_LIGHT_KINDS];

extern const blueprint blueprintCatalog[NUMBER_BLUEPRINTS];

extern creatureType monsterCatalog[NUMBER_MONSTER_KINDS];
extern monsterWords monsterText[NUMBER_MONSTER_KINDS];
extern hordeType hordeCatalog[NUMBER_HORDES];
extern const mutation mutationCatalog[NUMBER_MUTATORS];
extern const monsterClass monsterClassCatalog[MONSTER_CLASS_COUNT];

extern const feat featTable[FEAT_COUNT];

// ITEMS
extern char itemCategoryNames[NUMBER_ITEM_CATEGORIES][7];
extern char itemTitles[NUMBER_SCROLL_KINDS][30];
extern char titlePhonemes[NUMBER_TITLE_PHONEMES][30];
extern char itemColors[NUMBER_ITEM_COLORS][30];
extern char itemWoods[NUMBER_ITEM_WOODS][30];
extern char itemMetals[NUMBER_ITEM_METALS][30];
extern char itemGems[NUMBER_ITEM_GEMS][30];

extern char itemColorsRef[NUMBER_ITEM_COLORS][30];
extern char itemWoodsRef[NUMBER_ITEM_WOODS][30];
extern char itemMetalsRef[NUMBER_ITEM_METALS][30];
extern char itemGemsRef[NUMBER_ITEM_GEMS][30];

extern itemTable keyTable[NUMBER_KEY_TYPES];
extern itemTable foodTable[NUMBER_FOOD_KINDS];
extern itemTable weaponTable[NUMBER_WEAPON_KINDS];
extern itemTable armorTable[NUMBER_ARMOR_KINDS];
extern itemTable scrollTable[NUMBER_SCROLL_KINDS];
extern itemTable potionTable[NUMBER_POTION_KINDS];
extern itemTable wandTable[NUMBER_WAND_KINDS];
extern itemTable staffTable[NUMBER_STAFF_KINDS];
extern itemTable ringTable[NUMBER_RING_KINDS];
extern itemTable charmTable[NUMBER_CHARM_KINDS];

extern const bolt boltCatalog[NUMBER_BOLT_KINDS];

extern const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30];

extern const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30];

extern const char monsterBehaviorFlagDescriptions[32][COLS];
extern const char monsterAbilityFlagDescriptions[32][COLS];
extern const char monsterBookkeepingFlagDescriptions[32][COLS];

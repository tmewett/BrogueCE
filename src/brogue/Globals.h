/*
 *  Globals.h
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

// Colors that are used outside of Globals tables - may be set by variants

extern const color discordColor;

extern const color undiscoveredColor;
extern const color fireForeColor;
extern const color deepWaterLightColor;

extern const color confusionGasColor;

extern color minersLightColor; //dynamic
extern const color basicLightColor;
extern const color torchLightColor;

extern const color magicMapFlashColor;
extern const color spectralImageColor;

extern const color colorMultiplier100;
extern const color memoryColor;
extern const color memoryOverlay;
extern const color magicMapColor;
extern const color clairvoyanceColor;
extern const color telepathyMultiplier;
extern const color omniscienceColor;

extern const color itemColor;
extern const color blueBar;
extern const color redBar;
extern const color hiliteColor;
extern const color interfaceBoxColor;
extern const color interfaceButtonColor;
extern const color buttonHoverColor;
extern const color titleButtonColor;

extern const color playerInvisibleColor;
extern const color playerInShadowColor;
extern const color playerInLightColor;
extern const color playerInDarknessColor;
extern const color inDarknessMultiplierColor;

extern const color goodMessageColor;
extern const color badMessageColor;
extern const color advancementMessageColor;
extern const color itemMessageColor;
extern const color flavorTextColor;
extern const color backgroundMessageColor;

extern const color superVictoryColor;

extern const color flameSourceColor;
extern const color flameSourceColorSecondary;
extern const color flameTitleColor;

// Tables that define game play - may be set by variants

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
extern const statusEffect statusEffectCatalog[NUMBER_OF_STATUS_EFFECTS];

extern const feat *featTable;

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
extern itemTable *charmTable;
extern const charmEffectTableEntry *charmEffectTable;

extern const meteredItemGenerationTable *meteredItemsGenerationTable;

extern const bolt *boltCatalog;

extern const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30];

extern const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30];

extern const char *mainMenuTitle;
extern const levelFeeling *levelFeelings;

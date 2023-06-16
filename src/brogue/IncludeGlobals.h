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

void initializeGameConst();
void initializeGameGlobals();

extern color fireForeColor;

// light colors
extern color minersLightColor;
extern color torchLightColor;
extern color deepWaterLightColor;

extern color discordColor;
extern color memoryColor;
extern color memoryOverlay;
extern color magicMapColor;
extern color clairvoyanceColor;
extern color telepathyMultiplier;
extern color omniscienceColor;
extern color basicLightColor;

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

extern color inDarknessMultiplierColor;

extern color buttonHoverColor;
extern color titleButtonColor;

// other colors
extern color confusionGasColor;
extern color spectralImageColor;

extern color goodMessageColor;
extern color badMessageColor;
extern color advancementMessageColor;
extern color itemMessageColor;
extern color backgroundMessageColor;

extern color flavorTextColor;

extern color flameSourceColor;
extern color flameSourceColorSecondary;
extern color flameTitleColor;

extern color superVictoryColor;

extern color *dynamicColors[NUMBER_DYNAMIC_COLORS][3];

extern autoGenerator *autoGeneratorCatalog;
extern short *lumenstoneDistribution;

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
extern char itemCategoryNames[NUMBER_ITEM_CATEGORIES][11];
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
extern itemTable *wandTable;
extern itemTable staffTable[NUMBER_STAFF_KINDS];
extern itemTable ringTable[NUMBER_RING_KINDS];
extern itemTable charmTable[NUMBER_CHARM_KINDS];
extern const meteredItemGenerationTable meteredItemsGenerationTable[NUMBER_METERED_ITEMS];

extern const bolt *boltCatalog;

extern const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30];

extern const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30];
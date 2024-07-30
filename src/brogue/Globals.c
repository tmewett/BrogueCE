/*
 *  Globals.c
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

/*  This file contains constants associated with game objects,
 *  including colors and material descriptions. The file also
 *  contains large tables (e.g. tileCatalog) that consist of
 *  constant data associated with game objects, including creation
 *  probabilities, descriptions and in-game attributes.
 *
 *  Where possible the tables are defined as const arrays,
 *  because the data typically is not dynamic. Some tables,
 *  however, also store some mutable flags, so cannot be made
 *  constant.  For tables that are
 *  variant-specific (e.g. autoGeneratorCatalog), the tables are
 *  declared as pointers (again typically const * since the data
 *  contained is constant).
 *
 *  These tables are defined on a per-variant basis in
 *  Globals{variant}.c. When a variant is selected, the pointers
 *  declared in this file are set to the definitions in the chosen
 *  Globals{variant}.c file.
 */

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

// Global tables, definitions are found in Globals_{variant}.c

const autoGenerator *autoGeneratorCatalog;
const short *lumenstoneDistribution;
const bolt *boltCatalog;
itemTable *potionTable;
itemTable *scrollTable;
itemTable *wandTable;
itemTable *charmTable;
const feat *featTable;
const charmEffectTableEntry *charmEffectTable;
const levelFeeling *levelFeelings;
const hordeType *hordeCatalog;
const blueprint *blueprintCatalog;
const short *itemGenerationProbabilities;
const meteredItemGenerationTable *meteredItemsGenerationTable;
const char *mainMenuTitle;

// bolt colors
const color descentBoltColor =      {-40,   -40,    -40,    0,      0,          80,         80,     true};
const color discordColor =          {25,    0,      25,     66,     0,          0,          0,      true};
const color poisonColor =           {0,     0,      0,      10,     50,         10,         0,      true};
const color beckonColor =           {10,    10,     10,     5,      5,          5,          50,     true};
const color invulnerabilityColor =  {25,    0,      25,     0,      0,          66,         0,      true};
const color fireBoltColor =         {500,   150,    0,      45,     30,         0,          0,      true};
const color yendorLightColor =      {50,    -100,    30,     0,      0,          0,          0,      true};
const color flamedancerCoronaColor ={500,   150,    100,    45,     30,         0,          0,      true};

// tile colors
const color undiscoveredColor =     {0,     0,      0,      0,      0,          0,          0,      false};

const color wallForeColor =         {7,     7,      7,      3,      3,          3,          0,      false};
const color wallBackColorStart =    {45,    40,     40,     15,     0,          5,          20,     false};
const color wallBackColorEnd =      {40,    30,     35,     0,      20,         30,         20,     false};

const color mudWallForeColor =      {55,    45,     0,      5,      5,          5,          1,      false};
const color mudWallBackColor =      {20,    12,     3,      8,      4,          3,          0,      false};

const color graniteBackColor =      {10,    10,     10,     0,      0,          0,          0,      false};

const color floorForeColor =        {30,    30,     30,     0,      0,          0,          35,     false};

const color floorBackColorStart =   {2,     2,      10,     2,      2,          0,          0,      false};
const color floorBackColorEnd =     {5,     5,      5,      2,      2,          0,          0,      false};

const color stairsBackColor =       {15,    15,     5,      0,      0,          0,          0,      false};
const color firstStairsBackColor =  {10,    10,     25,     0,      0,          0,          0,      false};

const color refuseBackColor =       {6,     5,      3,      2,      2,          0,          0,      false};
const color rubbleBackColor =       {7,     7,      8,      2,      2,          1,          0,      false};
const color bloodflowerForeColor =  {30,    5,      40,     5,      1,          3,          0,      false};
const color bloodflowerPodForeColor = {50,  5,      25,     5,      1,          3,          0,      false};
const color bloodflowerBackColor =  {15,    3,      10,     3,      1,          3,          0,      false};
const color bedrollBackColor =      {10,    8,      5,      1,      1,          0,          0,      false};

const color obsidianBackColor =     {6,     0,      8,      2,      0,          3,          0,      false};
const color carpetForeColor =       {23,    30,     38,     0,      0,          0,          0,      false};
const color carpetBackColor =       {15,    8,      5,      0,      0,          0,          0,      false};
const color marbleForeColor =       {30,    23,     38,     0,      0,          0,          0,      false};
const color marbleBackColor =       {6,     5,      13,     1,      0,          1,          0,      false};
const color doorForeColor =         {70,    35,     15,     0,      0,          0,          0,      false};
const color doorBackColor =         {30,    10,     5,      0,      0,          0,          0,      false};
const color ironDoorForeColor =     {500,   500,    500,    0,      0,          0,          0,      false};
const color ironDoorBackColor =     {15,    15,     30,     0,      0,          0,          0,      false};
const color bridgeFrontColor =      {33,    12,     12,     12,     7,          2,          0,      false};
const color bridgeBackColor =       {12,    3,      2,      3,      2,          1,          0,      false};
const color statueBackColor =       {20,    20,     20,     0,      0,          0,          0,      false};
const color glyphColor =            {20,    5,      5,      50,     0,          0,          0,      true};
const color glyphLightColor =       {150,   0,      0,      150,    0,          0,          0,      true};
const color sacredGlyphColor =      {5,     20,     5,      0,      50,         0,          0,      true};
const color sacredGlyphLightColor = {45,    150,    60,     25,     80,         25,         0,      true};

const color minersLightStartColor = {180,   180,    180,    0,      0,          0,          0,      false};
const color minersLightEndColor =   {90,    90,     120,    0,      0,          0,          0,      false};
const color torchColor =            {150,   75,     30,     0,      30,         20,         0,      true};

const color deepWaterForeColor =    {5,     8,      20,     0,      4,          15,         10,     true};
const color deepWaterBackColorStart = {5,   10,     31,     5,      5,          5,          6,      true};
const color deepWaterBackColorEnd = {5,     8,      20,     2,      3,          5,          5,      true};
const color shallowWaterForeColor = {28,    28,     60,     0,      0,          10,         10,     true};
const color shallowWaterBackColorStart ={20,20,     60,     0,      0,          10,         10,     true};
const color shallowWaterBackColorEnd ={12,  15,     40,     0,      0,          5,          5,      true};

const color mudForeColor =          {18,    14,     5,      5,      5,          0,          0,      false};
const color mudBackColor =          {23,    17,     7,      5,      5,          0,          0,      false};
const color chasmForeColor =        {7,     7,      15,     4,      4,          8,          0,      false};
const color chasmEdgeBackColorStart ={5,    5,      25,     2,      2,          2,          0,      false};
const color chasmEdgeBackColorEnd = {8,     8,      20,     2,      2,          2,          0,      false};
const color fireForeColor =         {70,    20,     0,      15,     10,         0,          0,      true};
const color lavaForeColor =         {20,    20,     20,     100,    10,         0,          0,      true};
const color brimstoneForeColor =    {100,   50,     10,     0,      50,         40,         0,      true};
const color brimstoneBackColor =    {18,    12,     9,      0,      0,          5,          0,      false};

const color lavaBackColor =         {70,    20,     0,      15,     10,         0,          0,      true};
const color acidBackColor =         {15,    80,     25,     5,      15,         10,         0,      true};

const color lightningColor =        {100,   150,    500,    50,     50,         0,          50,     true};
const color fungusLightColor =      {2,     11,     11,     4,      3,          3,          0,      true};
const color lavaLightColor =        {47,    13,     0,      10,     7,          0,          0,      true};
const color deepWaterLightColor =   {10,    30,     100,    0,      30,         100,        0,      true};

const color grassColor =            {15,    40,     15,     15,     50,         15,         10,     false};
const color deadGrassColor =        {20,    13,     0,      20,     10,         5,          10,     false};
const color fungusColor =           {15,    50,     50,     0,      25,         0,          30,     true};
const color grayFungusColor =       {30,    30,     30,     5,      5,          5,          10,     false};
const color foliageColor =          {25,    100,    25,     15,     0,          15,         0,      false};
const color deadFoliageColor =      {20,    13,     0,      30,     15,         0,          20,     false};
const color lichenColor =           {50,    5,      25,     10,     0,          5,          0,      true};
const color hayColor =              {70,    55,     5,      0,      20,         20,         0,      false};
const color ashForeColor =          {20,    20,     20,     0,      0,          0,          20,     false};
const color bonesForeColor =        {80,    80,     30,     5,      5,          35,         5,      false};
const color ectoplasmColor =        {45,    20,     55,     25,     0,          25,         5,      false};
const color forceFieldColor =       {0,     25,     25,     0,      25,         25,         0,      true};
const color wallCrystalColor =      {40,    40,     60,     20,     20,         40,         0,      true};
const color altarForeColor =        {5,     7,      9,      0,      0,          0,          0,      false};
const color altarBackColor =        {35,    18,     18,     0,      0,          0,          0,      false};
const color greenAltarBackColor =   {18,    25,     18,     0,      0,          0,          0,      false};
const color goldAltarBackColor =    {25,    24,     12,     0,      0,          0,          0,      false};
const color pedestalBackColor =     {10,    5,      20,     0,      0,          0,          0,      false};

// monster colors
const color goblinColor =           {40,    30,     20,     0,      0,          0,          0,      false};
const color jackalColor =           {60,    42,     27,     0,      0,          0,          0,      false};
const color ogreColor =             {60,    25,     25,     0,      0,          0,          0,      false};
const color eelColor =              {30,    12,     12,     0,      0,          0,          0,      false};
const color goblinConjurerColor =   {67,    10,     100,    0,      0,          0,          0,      false};
const color spectralBladeColor =    {15,    15,     60,     0,      0,          70,         50,     true};
const color spectralImageColor =    {13,    0,      0,      25,     0,          0,          0,      true};
const color toadColor =             {40,    65,     30,     0,      0,          0,          0,      false};
const color trollColor =            {40,    60,     15,     0,      0,          0,          0,      false};
const color centipedeColor =        {75,    25,     85,     0,      0,          0,          0,      false};
const color dragonColor =           {20,    80,     15,     0,      0,          0,          0,      false};
const color krakenColor =           {100,   55,     55,     0,      0,          0,          0,      false};
const color salamanderColor =       {40,    10,     0,      8,      5,          0,          0,      true};
const color pixieColor =            {60,    60,     60,     40,     40,         40,         0,      true};
const color darPriestessColor =     {0,     50,     50,     0,      0,          0,          0,      false};
const color darMageColor =          {50,    50,     0,      0,      0,          0,          0,      false};
const color wraithColor =           {66,    66,     25,     0,      0,          0,          0,      false};
const color pinkJellyColor =        {100,   40,     40,     5,      5,          5,          20,     true};
const color wormColor =             {80,    60,     40,     0,      0,          0,          0,      false};
const color sentinelColor =         {3,     3,      30,     0,      0,          10,         0,      true};
const color goblinMysticColor =     {10,    67,     100,    0,      0,          0,          0,      false};
const color ifritColor =            {50,    10,     100,    75,     0,          20,         0,      true};
const color phoenixColor =          {100,   0,      0,      0,      100,        0,          0,      true};

// light colors

const color torchLightColor =       {75,    38,     15,     0,      15,         7,          0,      true};
const color hauntedTorchColor =     {75,    20,     40,     30,     10,         0,          0,      true};
const color hauntedTorchLightColor ={67,    10,     10,     20,     4,          0,          0,      true};
const color ifritLightColor =       {0,     10,     150,    100,    0,          100,        0,      true};
const color unicornLightColor =     {-50,   -50,    -50,    250,    250,        250,        0,      true};
const color wispLightColor =        {75,    100,    250,    33,     10,         0,          0,      true};
const color summonedImageLightColor ={200,  0,      75,     0,      0,          0,          0,      true};
const color spectralBladeLightColor ={40,   0,      230,    0,      0,          0,          0,      true};
const color ectoplasmLightColor =   {23,    10,     28,     13,     0,          13,         3,      false};
const color explosionColor =        {10,    8,      2,      0,      2,          2,          0,      true};
const color explosiveAuraColor =    {2000,  0,      -1000,  200,    200,        0,          0,      true};
const color sacrificeTargetColor =  {100,   -100,   -300,   0,      100,        100,        0,      true};
const color dartFlashColor =        {500,   500,    500,    0,      2,          2,          0,      true};
const color lichLightColor =        {-50,   80,     30,     0,      0,          20,         0,      true};
const color forceFieldLightColor =  {10,    10,     10,     0,      50,         50,         0,      true};
const color crystalWallLightColor = {10,    10,     10,     0,      0,          50,         0,      true};
const color sunLightColor =         {100,   100,    75,     0,      0,          0,          0,      false};
const color fungusForestLightColor ={30,    40,     60,     0,      0,          0,          40,     true};
const color fungusTrampledLightColor ={10,  10,     10,     0,      50,         50,         0,      true};
const color redFlashColor =         {100,   10,     10,     0,      0,          0,          0,      false};
const color darknessPatchColor =    {-10,   -10,    -10,    0,      0,          0,          0,      false};
const color darknessCloudColor =    {-20,   -20,    -20,    0,      0,          0,          0,      false};
const color magicMapFlashColor =    {60,    20,     60,     0,      0,          0,          0,      false};
const color sentinelLightColor =    {20,    20,     120,    10,     10,         60,         0,      true};
const color telepathyColor =        {30,    30,     130,    0,      0,          0,          0,      false};
const color confusionLightColor =   {10,    10,     10,     10,     10,         10,         0,      true};
const color portalActivateLightColor ={300, 400,    500,    0,      0,          0,          0,      true};
const color descentLightColor =     {20,    20,     70,     0,      0,          0,          0,      false};
const color algaeBlueLightColor =   {20,    15,     50,     0,      0,          0,          0,      false};
const color algaeGreenLightColor =  {15,    50,     20,     0,      0,          0,          0,      false};

// flare colors
const color scrollProtectionColor = {375,   750,    0,      0,      0,          0,          0,      true};
const color scrollEnchantmentColor ={250,   225,    300,    0,      0,          450,        0,      true};
const color potionStrengthColor =   {1000,  0,      400,    600,    0,          0,          0,      true};
const color empowermentFlashColor = {500,   1000,   600,    0,      500,        0,          0,      true};
const color genericFlashColor =     {800,   800,    800,    0,      0,          0,          0,      false};
const color summoningFlashColor =   {0,     0,      0,      600,    0,          1200,       0,      true};
const color fireFlashColor =        {750,   225,    0,      100,    50,         0,          0,      true};
const color explosionFlareColor =   {10000, 6000,   1000,   0,      0,          0,          0,      false};
const color quietusFlashColor =     {0,     -1000,  -200,   0,      0,          0,          0,      true};
const color slayingFlashColor =     {-1000, -200,   0,      0,      0,          0,          0,      true};

// const color multipliers
const color colorDim25 =            {25,    25,     25,     25,     25,         25,         25,     false};
const color colorMultiplier100 =    {100,   100,    100,    100,    100,        100,        100,    false};
const color memoryColor =           {25,    25,     50,     20,     20,         20,         0,      false};
const color memoryOverlay =         {25,    25,     50,     0,      0,          0,          0,      false};
const color magicMapColor =         {60,    20,     60,     60,     20,         60,         0,      false};
const color clairvoyanceColor =     {50,    90,     50,     50,     90,         50,         66,     false};
const color telepathyMultiplier =   {30,    30,     130,    30,     30,         130,        66,     false};
const color omniscienceColor =      {140,   100,    60,     140,    100,        60,         90,     false};
const color basicLightColor =       {180,   180,    180,    180,    180,        180,        180,    false};

// blood colors
const color humanBloodColor =       {60,    20,     10,     15,     0,          0,          15,     false};
const color insectBloodColor =      {10,    60,     20,     0,      15,         0,          15,     false};
const color vomitColor =            {60,    50,     5,      0,      15,         15,         0,      false};
const color urineColor =            {70,    70,     40,     0,      0,          0,          10,     false};
const color methaneColor =          {45,    60,     15,     0,      0,          0,          0,      false};

// gas colors
const color poisonGasColor =        {75,    25,     85,     0,      0,          0,          0,      false};
const color confusionGasColor =     {60,    60,     60,     40,     40,         40,         0,      true};

// interface colors
const color itemColor =             {100,   95,     -30,    0,      0,          0,          0,      false};
const color blueBar =               {15,    10,     50,     0,      0,          0,          0,      false};
const color redBar =                {45,    10,     15,     0,      0,          0,          0,      false};
const color hiliteColor =           {100,   100,    0,      0,      0,          0,          0,      false};
const color interfaceBoxColor =     {7,     6,      15,     0,      0,          0,          0,      false};
const color interfaceButtonColor =  {18,    15,     38,     0,      0,          0,          0,      false};
const color buttonHoverColor =      {100,   70,     40,     0,      0,          0,          0,      false};
const color titleButtonColor =      {23,    15,     30,     0,      0,          0,          0,      false};

const color playerInvisibleColor =  {20,    20,     30,     0,      0,          80,         0,      true};
const color playerInLightColor =    {100,   90,     30,     0,      0,          0,          0,      false};
const color playerInShadowColor =   {60,    60,     100,    0,      0,          0,          0,      false};
const color playerInDarknessColor = {30,    30,     65,     0,      0,          0,          0,      false};

const color inLightMultiplierColor ={150,   150,    75,     150,    150,        75,         100,    true};
const color inDarknessMultiplierColor={66,  66,     120,    66,     66,         120,        66,     true};

const color goodMessageColor =      {60,    50,     100,    0,      0,          0,          0,      false};
const color badMessageColor =       {100,   50,     60,     0,      0,          0,          0,      false};
const color advancementMessageColor ={50,   100,    60,     0,      0,          0,          0,      false};
const color itemMessageColor =      {100,   100,    50,     0,      0,          0,          0,      false};
const color flavorTextColor =       {50,    40,     90,     0,      0,          0,          0,      false};
const color backgroundMessageColor ={60,    20,     70,     0,      0,          0,          0,      false};

const color superVictoryColor =     {150,   100,    300,    0,      0,          0,          0,      false};

const color flameSourceColor = {20, 7, 7, 60, 40, 40, 0, true}; // 8
const color flameSourceColorSecondary = {7, 2, 0, 10, 0, 0, 0, true};

const color flameTitleColor = {0, 0, 0, 9, 9, 15, 0, true}; // *pale blue**

// Constant bounds for dynamic colors
const color *dynamicColorsBounds[NUMBER_DYNAMIC_COLORS][2] = {
    // shallow color               deep color
    {&minersLightStartColor,     &minersLightEndColor},
    {&wallBackColorStart,        &wallBackColorEnd},
    {&deepWaterBackColorStart,   &deepWaterBackColorEnd},
    {&shallowWaterBackColorStart,&shallowWaterBackColorEnd},
    {&floorBackColorStart,       &floorBackColorEnd},
    {&chasmEdgeBackColorStart,   &chasmEdgeBackColorEnd}
};

// Dynamic colors (these change at runtime)
color minersLightColor, wallBackColor, deepWaterBackColor, shallowWaterBackColor, floorBackColor, chasmEdgeBackColor;

color *dynamicColors[NUMBER_DYNAMIC_COLORS] = {
    &minersLightColor,
    &wallBackColor,
    &deepWaterBackColor,
    &shallowWaterBackColor,
    &floorBackColor,
    &chasmEdgeBackColor
};

const floorTileType tileCatalog[NUMBER_TILETYPES] = {

 // promoteChance is in hundredths of a percent per turn

 // dungeon layer (this layer must have all of fore color, back color and char)
 // tileType                    char            fore color          back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*NOTHING*/                    {' ',           &black,             &black,             100,0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       0, 0,                                                                                               "a chilly void",        ""},
 /*GRANITE*/                    {G_GRANITE,     &wallBackColor,     &graniteBackColor,  0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                       "a rough granite wall", "The granite is split open with splinters of rock jutting out at odd angles."},
 /*FLOOR*/                      {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       0, 0,                                                                                               "the ground",           ""},
 /*FLOOR_FLOODABLE*/            {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       0, 0,                                                                                               "the ground",           ""},
 /*CARPET*/                     {G_CARPET,      &carpetForeColor,   &carpetBackColor,   85, 0,  DF_EMBERS,0,0,                              0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                                     "the carpet",           "Ornate carpeting fills this room, a relic of ages past."},
 /*MARBLE_FLOOR*/               {G_CARPET,      &marbleForeColor,   &marbleBackColor,   85, 0,  DF_EMBERS,0,0,                              0,  NO_LIGHT,       0, 0,                                                                                               "the marble ground",    "Light from the nearby crystals catches the grain of the lavish marble floor."},
 /*WALL*/                       {G_WALL,        &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                       "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*DOOR*/                       {G_CLOSED_DOOR, &doorForeColor,     &doorBackColor,     8,  50, DF_EMBERS,0,DF_OPEN_DOOR,                   0,  NO_LIGHT,       (T_OBSTRUCTS_VISION | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP | TM_VISUALLY_DISTINCT), "a wooden door", "you pass through the doorway."},
 /*OPEN_DOOR*/                  {G_OPEN_DOOR,   &doorForeColor,     &doorBackColor,     25, 50, DF_EMBERS,0,DF_CLOSED_DOOR,             10000,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),           "an open door",         "you pass through the doorway."},
 /*SECRET_DOOR*/                {G_WALL,        &wallForeColor,     &wallBackColor,     0,  50, DF_EMBERS,DF_SHOW_DOOR,0,                   0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET),  "a stone wall", "The rough stone wall is firm and unyielding."},
 /*LOCKED_DOOR*/            {G_CLOSED_IRON_DOOR,&ironDoorForeColor, &ironDoorBackColor, 15, 50, DF_EMBERS,0,DF_OPEN_IRON_DOOR_INERT,        0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED),  "a locked iron door",   "you search your pack but do not have a matching key."},
 /*OPEN_IRON_DOOR_INERT*/   {G_OPEN_IRON_DOOR,  &white,             &ironDoorBackColor, 90, 50, DF_EMBERS,0,0,                              0,  NO_LIGHT,       (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VISUALLY_DISTINCT),                           "an open iron door",    "you pass through the doorway."},
 /*DOWN_STAIRS,*/               {G_DOWN_STAIRS, &itemColor,         &stairsBackColor,   30, 0,  DF_PLAIN_FIRE,0,DF_REPEL_CREATURES,         0,  NO_LIGHT,       (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED), "a downward staircase",   "stairs spiral downward into the depths."},
 /*UP_STAIRS*/                  {G_UP_STAIRS,   &itemColor,         &stairsBackColor,   30, 0,  DF_PLAIN_FIRE,0,DF_REPEL_CREATURES,         0,  NO_LIGHT,       (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED), "an upward staircase",    "stairs spiral upward."},
 /*DUNGEON_EXIT*/               {G_DOORWAY,     &lightBlue,       &firstStairsBackColor,30, 0,  DF_PLAIN_FIRE,0,DF_REPEL_CREATURES,         0,  NO_LIGHT,       (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED), "the dungeon exit",       "the gilded doors leading out of the dungeon are sealed by an invisible force."},
 /*DUNGEON_PORTAL*/             {G_DOORWAY,     &wallCrystalColor,&firstStairsBackColor,30, 0,  DF_PLAIN_FIRE,0,DF_REPEL_CREATURES,         0,  INCENDIARY_DART_LIGHT, (T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_PROMOTES_ON_STEP | TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_BRIGHT_MEMORY | TM_INTERRUPT_EXPLORATION_WHEN_SEEN | TM_INVERT_WHEN_HIGHLIGHTED), "a crystal portal",       "dancing lights play across the plane of this sparkling crystal portal."},
 /*TORCH_WALL*/                 {G_TORCH,       &torchColor,        &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,0,                          0,  TORCH_LIGHT,    (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                       "a wall-mounted torch", "The torch is anchored firmly to the wall and sputters quietly in the gloom."},
 /*CRYSTAL_WALL*/               {G_CRYSTAL,     &wallCrystalColor,  &wallCrystalColor,  0,  0,  DF_PLAIN_FIRE,0,0,                          0,  CRYSTAL_WALL_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_DIAGONAL_MOVEMENT), (TM_STAND_IN_TILE | TM_REFLECTS_BOLTS),"a crystal formation", "You feel the crystal's glossy surface and admire the dancing lights beneath."},
 /*PORTCULLIS_CLOSED*/          {G_PORTCULLIS,  &gray,              &floorBackColor,    10, 0,  DF_PLAIN_FIRE,0,DF_OPEN_PORTCULLIS,         0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_CONNECTS_LEVEL), "a heavy portcullis", "The iron bars rattle but will not budge; they are firmly locked in place."},
 /*PORTCULLIS_DORMANT*/         {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,DF_ACTIVATE_PORTCULLIS,     0,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),                                                    "the ground",           ""},
 /*WOODEN_BARRICADE*/           {G_BARRICADE,   &doorForeColor,     &floorBackColor,    8,  100,DF_WOODEN_BARRICADE_BURN,0,0,               0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_CONNECTS_LEVEL),"a dry wooden barricade","The wooden barricade is firmly set but has dried over the years. Might it burn?"},
 /*PILOT_LIGHT_DORMANT*/        {G_TORCH,       &torchLightColor,   &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_PILOT_LIGHT,             0,  TORCH_LIGHT,    (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),            "a wall-mounted torch", "The torch is anchored firmly to the wall, and sputters quietly in the gloom."},
 /*PILOT_LIGHT*/                {G_FIRE,        &fireForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,0,                          0,  TORCH_LIGHT,    (T_OBSTRUCTS_EVERYTHING | T_IS_FIRE), (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR),                      "a fallen torch",       "The torch lies at the foot of the wall, spouting gouts of flame haphazardly."},
 /*HAUNTED_TORCH_DORMANT*/      {G_TORCH,       &torchColor,        &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_HAUNTED_TORCH_TRANSITION,0,  TORCH_LIGHT,    (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),            "a wall-mounted torch", "The torch is anchored firmly to the wall and sputters quietly in the gloom."},
 /*HAUNTED_TORCH_TRANSITIONING*/{G_TORCH,       &torchColor,        &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_HAUNTED_TORCH,        2000,  TORCH_LIGHT,    (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                          "a wall-mounted torch", "The torch is anchored firmly to the wall and sputters quietly in the gloom."},
 /*HAUNTED_TORCH*/              {G_TORCH,       &hauntedTorchColor, &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,0,                          0,  HAUNTED_TORCH_LIGHT, (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                  "a sputtering torch",   "A dim purple flame sputters and spits atop this wall-mounted torch."},
 /*WALL_LEVER_HIDDEN*/          {G_WALL,        &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,DF_REVEAL_LEVER,0,            0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET),           "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*WALL_LEVER*/                 {G_LEVER,       &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_PULL_LEVER,              0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED),"a lever", "The lever moves."},
 /*WALL_LEVER_PULLED*/          {G_LEVER_PULLED,&wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                       "an inactive lever",    "The lever won't budge."},
 /*WALL_LEVER_HIDDEN_DORMANT*/  {G_WALL,        &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_CREATE_LEVER,            0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_IS_WIRED),                                         "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*STATUE_INERT*/               {G_STATUE,      &wallBackColor,     &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE), "a marble statue",  "The cold marble statue has weathered the years with grace."},
 /*STATUE_DORMANT*/             {G_STATUE,      &wallBackColor,     &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,DF_CRACKING_STATUE,         0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),"a marble statue", "The cold marble statue has weathered the years with grace."},
 /*STATUE_CRACKING*/        {G_CRACKED_STATUE,  &wallBackColor,     &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,DF_STATUE_SHATTER,       3500,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),"a cracking statue",    "Deep cracks ramble down the side of the statue even as you watch."},
 /*STATUE_INSTACRACK*/          {G_STATUE,      &wallBackColor,     &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,DF_STATUE_SHATTER,          0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),"a marble statue", "The cold marble statue has weathered the years with grace."},
 /*PORTAL*/                     {G_DOORWAY,     &wallBackColor,     &floorBackColor,    17, 0,  DF_PLAIN_FIRE,0,DF_PORTAL_ACTIVATE,         0,  NO_LIGHT,       (T_OBSTRUCTS_ITEMS), (TM_STAND_IN_TILE | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),  "a stone archway",      "This ancient moss-covered stone archway radiates a strange, alien energy."},
 /*TURRET_DORMANT*/             {G_WALL,        &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_TURRET_EMERGE,           0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),            "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*WALL_MONSTER_DORMANT*/       {G_WALL,        &wallForeColor,     &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_WALL_SHATTER,            0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),            "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*DARK_FLOOR_DORMANT*/         {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,DF_DARKENING_FLOOR,         0,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),                                                    "the ground",           ""},
 /*DARK_FLOOR_DARKENING*/       {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,DF_DARK_FLOOR,           1500,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION),                                                                  "the ground",           ""},
 /*DARK_FLOOR*/                 {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,0,                          0,  DARKNESS_CLOUD_LIGHT, 0, 0,                                                                                         "the ground",           ""},
 /*MACHINE_TRIGGER_FLOOR*/      {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY),                      "the ground",           ""},
 /*ALTAR_INERT*/                {G_ALTAR,       &altarForeColor,    &altarBackColor,    17, 0,  0,0,0,                                      0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                         "a candle-lit altar",   "a gilded altar is adorned with candles that flicker in the breeze."},
 /*ALTAR_KEYHOLE*/              {G_ORB_ALTAR,   &altarForeColor,    &altarBackColor,    17, 0,  0,0,0,                                      0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_PROMOTES_WITH_KEY | TM_IS_WIRED | TM_LIST_IN_SIDEBAR),           "a candle-lit altar",   "ornate gilding spirals around a spherical depression in the top of the altar."},
 /*ALTAR_CAGE_OPEN*/            {G_ALTAR,       &altarForeColor,    &altarBackColor,    17, 0,  0,0,DF_ITEM_CAGE_CLOSE,                     0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_WITHOUT_KEY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),"a candle-lit altar",   "a cage, open on the bottom, hangs over this altar on a retractable chain."},
 /*ALTAR_CAGE_CLOSED*/          {G_CLOSED_CAGE, &altarBackColor,    &veryDarkGray,      17, 0,  0,0,DF_ITEM_CAGE_OPEN,                      0,  CANDLE_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),"an iron cage","the missing item must be replaced before you can access the remaining items."},
 /*ALTAR_SWITCH*/               {G_SAC_ALTAR,   &altarForeColor,    &altarBackColor,    17, 0,  0,0,DF_ALTAR_INERT,                         0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "a candle-lit altar",   "a weathered stone altar is adorned with candles that flicker in the breeze."},
 /*ALTAR_SWITCH_RETRACTING*/    {G_SAC_ALTAR,   &altarForeColor,    &altarBackColor,    17, 0,  0,0,DF_ALTAR_RETRACT,                       0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "a candle-lit altar",   "a weathered stone altar is adorned with candles that flicker in the breeze."},
 /*ALTAR_CAGE_RETRACTABLE*/     {G_CLOSED_CAGE, &altarBackColor,    &veryDarkGray,      17, 0,  0,0,DF_CAGE_DISAPPEARS,                     0,  CANDLE_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),"an iron cage","the cage won't budge. Perhaps there is a way to raise it nearby..."},
 /*PEDESTAL*/                   {G_PEDESTAL,    &altarForeColor,    &pedestalBackColor, 17, 0,  0,0,0,                                      0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), 0,                                                                   "a stone pedestal",     "elaborate carvings wind around this ancient pedestal."},
 /*MONSTER_CAGE_OPEN*/          {G_OPEN_CAGE,   &floorBackColor,    &veryDarkGray,      17, 0,  0,0,0,                                      0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "an open cage",         "the interior of the cage is filthy and reeks of decay."},
 /*MONSTER_CAGE_CLOSED*/        {G_CLOSED_CAGE, &gray,              &darkGray,          17, 0,  0,0,DF_MONSTER_CAGE_OPENS,                  0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_WITH_KEY | TM_LIST_IN_SIDEBAR | TM_INTERRUPT_EXPLORATION_WHEN_SEEN),"a locked iron cage","the bars of the cage are firmly set and will not budge."},
 /*COFFIN_CLOSED*/          {G_CLOSED_COFFIN,   &bridgeFrontColor,  &bridgeBackColor,   17, 20, DF_COFFIN_BURNS,0,DF_COFFIN_BURSTS,         0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_IS_WIRED | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),                  "a sealed coffin",      "a coffin made from thick wooden planks rests in a bed of moss."},
 /*COFFIN_OPEN*/                {G_OPEN_COFFIN, &black,             &bridgeBackColor,   17, 20, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),             "an empty coffin",      "an open wooden coffin rests in a bed of moss."},
 
 // traps (part of dungeon layer):
 // tileType                    char            fore color          back color        prio ign% [fire,discover,promote]Type                         promoteChance   glowLight   flags mechflags                                                                     description             flavorText
 /*GAS_TRAP_POISON_HIDDEN*/     {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_POISON_GAS_CLOUD,DF_SHOW_POISON_GAS_TRAP,0,                  0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*GAS_TRAP_POISON*/            {G_TRAP,        &poisonGasColor,    0,                  30, 0,  DF_POISON_GAS_CLOUD,0,0,                                        0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "a caustic gas trap",   "there is a hidden pressure plate in the floor above a reserve of caustic gas."},
 /*TRAP_DOOR_HIDDEN*/           {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_POISON_GAS_CLOUD,DF_SHOW_TRAPDOOR,0,                         0,  NO_LIGHT,   (T_AUTO_DESCENT), (TM_IS_SECRET),                                                   "the ground",           "you plunge through a hidden trap door!"},
 /*TRAP_DOOR*/                  {G_CHASM,       &chasmForeColor,    &black,             30, 0,  DF_POISON_GAS_CLOUD,0,0,                                        0,  NO_LIGHT,   (T_AUTO_DESCENT), 0,                                                                "a hole",               "you plunge through a hole in the ground!"},
 /*GAS_TRAP_PARALYSIS_HIDDEN*/  {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  0,DF_SHOW_PARALYSIS_GAS_TRAP,0,                                 0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET | TM_IS_WIRED),                                       "the ground",           ""},
 /*GAS_TRAP_PARALYSIS*/         {G_TRAP,        &pink,              0,                  30, 0,  0,0,0,                                                          0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),          "a paralysis trigger",  "there is a hidden pressure plate in the floor."},
 /*MACHINE_PARALYSIS_VENT_HIDDEN*/{G_FLOOR,     &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,DF_DISCOVER_PARALYSIS_VENT,DF_PARALYSIS_VENT_SPEW,0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),                     "the ground",           ""},
 /*MACHINE_PARALYSIS_VENT*/     {G_VENT,        &pink,              0,                  30, 0,  DF_PLAIN_FIRE,0,DF_PARALYSIS_VENT_SPEW,                         0,  NO_LIGHT,   (0), (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                     "an inactive gas vent", "A dormant gas vent is connected to a reserve of paralytic gas."},
 /*GAS_TRAP_CONFUSION_HIDDEN*/  {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_CONFUSION_GAS_TRAP_CLOUD,DF_SHOW_CONFUSION_GAS_TRAP,0,       0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*GAS_TRAP_CONFUSION*/         {G_TRAP,        &confusionGasColor, 0,                  30, 0,  DF_CONFUSION_GAS_TRAP_CLOUD,0,0,                                0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "a confusion trap",     "A hidden pressure plate accompanies a reserve of psychotropic gas."},
 /*FLAMETHROWER_HIDDEN*/        {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_FLAMETHROWER,DF_SHOW_FLAMETHROWER_TRAP,0,                    0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*FLAMETHROWER*/               {G_TRAP,        &red,               0,                  30, 0,  DF_FLAMETHROWER,0,0,                                            0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "a fire trap",          "A hidden pressure plate is connected to a crude flamethrower mechanism."},
 /*FLOOD_TRAP_HIDDEN*/          {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_FLOOD,DF_SHOW_FLOOD_TRAP,0,                                  0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*FLOOD_TRAP*/                 {G_TRAP,        &blue,              0,                  58, 0,  DF_FLOOD,0,0,                                                   0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "a flood trap",         "A hidden pressure plate is connected to floodgates in the walls and ceiling."},
 /*NET_TRAP_HIDDEN*/            {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_NET,DF_SHOW_NET_TRAP,0,                                      0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*NET_TRAP*/                   {G_TRAP,        &tanColor,          0,                  30, 0,  DF_NET,0,0,                                                     0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "a net trap",           "you see netting subtly concealed in the ceiling over a hidden pressure plate."},
 /*ALARM_TRAP_HIDDEN*/          {G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_AGGRAVATE_TRAP,DF_SHOW_ALARM_TRAP,0,                         0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_IS_SECRET),                                                     "the ground",           ""},
 /*ALARM_TRAP*/                 {G_TRAP,        &gray,              0,                  30, 0,  DF_AGGRAVATE_TRAP,0,0,                                          0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                        "an alarm trap",        "a hidden pressure plate is connected to a loud alarm mechanism."},
 /*MACHINE_POISON_GAS_VENT_HIDDEN*/{G_FLOOR,    &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,DF_SHOW_POISON_GAS_VENT,DF_POISON_GAS_VENT_OPEN,  0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),                     "the ground",           ""},
 /*MACHINE_POISON_GAS_VENT_DORMANT*/{G_VENT,    &floorForeColor,    0,                  30, 0,  DF_PLAIN_FIRE,0,DF_POISON_GAS_VENT_OPEN,                        0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "an inactive gas vent", "An inactive gas vent is hidden in a crevice in the ground."},
 /*MACHINE_POISON_GAS_VENT*/    {G_VENT,        &floorForeColor,    0,                  30, 0,  DF_PLAIN_FIRE,0,DF_VENT_SPEW_POISON_GAS,                    10000,  NO_LIGHT,   0, (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                                     "a gas vent",           "Clouds of caustic gas are wafting out of a hidden vent in the floor."},
 /*MACHINE_METHANE_VENT_HIDDEN*/{G_FLOOR,       &floorForeColor,    &floorBackColor,    95, 0,  DF_PLAIN_FIRE,DF_SHOW_METHANE_VENT,DF_METHANE_VENT_OPEN,        0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_SECRET | TM_IS_WIRED),                     "the ground",           ""},
 /*MACHINE_METHANE_VENT_DORMANT*/{G_VENT,       &floorForeColor,    0,                  30, 0,  DF_PLAIN_FIRE,0,DF_METHANE_VENT_OPEN,                           0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "an inactive gas vent", "An inactive gas vent is hidden in a crevice in the ground."},
 /*MACHINE_METHANE_VENT*/       {G_VENT,        &floorForeColor,    0,                  30, 15, DF_EMBERS,0,DF_VENT_SPEW_METHANE,                            5000,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                      "a gas vent",           "Clouds of explosive gas are wafting out of a hidden vent in the floor."},
 /*STEAM_VENT*/                 {G_VENT,        &gray,              0,                  15, 15, DF_EMBERS,0,DF_STEAM_PUFF,                                    250,  NO_LIGHT,   T_OBSTRUCTS_ITEMS, (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                     "a steam vent",         "A natural crevice in the floor periodically vents scalding gouts of steam."},
 /*MACHINE_PRESSURE_PLATE*/     {G_TRAP,        &white,             &chasmEdgeBackColor,15, 0,  0,0,DF_MACHINE_PRESSURE_PLATE_USED,                             0,  NO_LIGHT,   (T_IS_DF_TRAP), (TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),"a pressure plate",        "There is an exposed pressure plate here. A thrown item might trigger it."},
 /*MACHINE_PRESSURE_PLATE_USED*/{G_TRAP,        &darkGray,          &chasmEdgeBackColor,15, 0,  0,0,0,                                                          0,  NO_LIGHT,   0, (TM_LIST_IN_SIDEBAR),                                                            "an inactive pressure plate", "This pressure plate has already been depressed."},
 /*MACHINE_GLYPH*/              {G_MAGIC_GLYPH, &glyphColor,        0,                  42, 0,  0,0,DF_INACTIVE_GLYPH,                                          0,  GLYPH_LIGHT_DIM,(0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY | TM_VISUALLY_DISTINCT),"a magical glyph",      "A strange glyph, engraved into the floor, flickers with magical light."},
 /*MACHINE_GLYPH_INACTIVE*/     {G_MAGIC_GLYPH, &glyphColor,        0,                  42, 0,  0,0,DF_ACTIVE_GLYPH,                                        10000,  GLYPH_LIGHT_BRIGHT,(0), (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                    "a glowing glyph",      "A strange glyph, engraved into the floor, radiates magical light."},
 /*DEWAR_CAUSTIC_GAS*/          {G_DEWAR,       &poisonGasColor,    &darkGray,          10, 20, DF_DEWAR_CAUSTIC,0,DF_DEWAR_CAUSTIC,                            0,  NO_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),"a glass dewar of caustic gas", ""},
 /*DEWAR_CONFUSION_GAS*/        {G_DEWAR,       &confusionGasColor, &darkGray,          10, 20, DF_DEWAR_CONFUSION,0,DF_DEWAR_CONFUSION,                        0,  NO_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),"a glass dewar of confusion gas", ""},
 /*DEWAR_PARALYSIS_GAS*/        {G_DEWAR,       &pink,              &darkGray,          10, 20, DF_DEWAR_PARALYSIS,0,DF_DEWAR_PARALYSIS,                        0,  NO_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),"a glass dewar of paralytic gas", ""},
 /*DEWAR_METHANE_GAS*/          {G_DEWAR,       &methaneColor,      &darkGray,          10, 20, DF_DEWAR_METHANE,0,DF_DEWAR_METHANE,                            0,  NO_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_LIST_IN_SIDEBAR | TM_PROMOTES_ON_PLAYER_ENTRY | TM_INVERT_WHEN_HIGHLIGHTED),"a glass dewar of methane gas", ""},

 // liquid layer
 // tileType                    char        fore color              back color        prio ign% [fire,discover,promote]Type                         promoteChance   glowLight   flags mechflags                                                                     description             flavorText
 /*DEEP_WATER*/                 {G_LIQUID,  &deepWaterForeColor,    &deepWaterBackColor,40, 100,DF_STEAM_ACCUMULATION,0,0,                                      0,  NO_LIGHT,   (T_IS_FLAMMABLE | T_IS_DEEP_WATER), (TM_ALLOWS_SUBMERGING | TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE),"the murky waters", "the current tugs you in all directions."},
 /*SHALLOW_WATER*/              {0,     &shallowWaterForeColor, &shallowWaterBackColor, 55, 0,  DF_STEAM_ACCUMULATION,0,0,                                      0,  NO_LIGHT,   (0), (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),              "shallow water",        "the water is cold and reaches your knees."},
 /*MUD*/                        {G_BOG,     &mudForeColor,          &mudBackColor,      55, 0,  DF_PLAIN_FIRE,0,DF_METHANE_GAS_PUFF,                          100,  NO_LIGHT,   (0), (TM_STAND_IN_TILE | TM_ALLOWS_SUBMERGING),                                     "a bog",                "you are knee-deep in thick, foul-smelling mud."},
 /*CHASM*/                      {G_CHASM,   &chasmForeColor,        &black,             40, 0,  DF_PLAIN_FIRE,0,0,                                              0,  NO_LIGHT,   (T_AUTO_DESCENT), (TM_STAND_IN_TILE),                                               "a chasm",              "you plunge downward into the chasm!"},
 /*CHASM_EDGE*/                 {G_FLOOR,   &white,                 &chasmEdgeBackColor,80, 0,  DF_PLAIN_FIRE,0,0,                                              0,  NO_LIGHT,   0, 0,                                                                               "the brink of a chasm", "chilly winds blow upward from the stygian depths."},
 /*MACHINE_COLLAPSE_EDGE_DORMANT*/{G_FLOOR, &floorForeColor,        &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,DF_SPREADABLE_COLLAPSE,                         0,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),                                    "the ground",           ""},
 /*MACHINE_COLLAPSE_EDGE_SPREADING*/{G_FLOOR,&white,                &chasmEdgeBackColor,45, 0,  DF_PLAIN_FIRE,0,DF_COLLAPSE_SPREADS,                         2500,  NO_LIGHT,   (0), (TM_VANISHES_UPON_PROMOTION),                                                  "the crumbling ground", "cracks are appearing in the ground beneath your feet!"},
 /*LAVA*/                       {G_LIQUID,  &fireForeColor,         &lavaBackColor,     40, 0,  DF_OBSIDIAN,0,0,                                                0,  LAVA_LIGHT, (T_LAVA_INSTA_DEATH), (TM_STAND_IN_TILE | TM_ALLOWS_SUBMERGING),                    "lava",                 "searing heat rises from the lava."},
 /*LAVA_RETRACTABLE*/           {G_LIQUID,  &fireForeColor,         &lavaBackColor,     40, 0,  DF_OBSIDIAN,0,DF_RETRACTING_LAVA,                               0,  LAVA_LIGHT, (T_LAVA_INSTA_DEATH), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_ALLOWS_SUBMERGING),"lava","searing heat rises from the lava."},
 /*LAVA_RETRACTING*/            {G_LIQUID,  &fireForeColor,         &lavaBackColor,     40, 0,  DF_OBSIDIAN,0,DF_OBSIDIAN_WITH_STEAM,                       -1500,  LAVA_LIGHT, (T_LAVA_INSTA_DEATH), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_ALLOWS_SUBMERGING), "cooling lava", "searing heat rises from the lava."},
 /*SUNLIGHT_POOL*/              {G_FLOOR,   &floorForeColor,        &floorBackColor,    90, 0,  DF_PLAIN_FIRE,0,0,                                              0,  SUN_LIGHT,  (0), (TM_STAND_IN_TILE),                                                            "a patch of sunlight",  "sunlight streams through cracks in the ceiling."},
 /*DARKNESS_PATCH*/             {G_FLOOR,   &floorForeColor,        &floorBackColor,    90, 0,  DF_PLAIN_FIRE,0,0,                                              0,  DARKNESS_PATCH_LIGHT,   (0), 0,                                                                 "a patch of shadows",   "this area happens to be cloaked in shadows -- perhaps a safe place to hide."},
 /*ACTIVE_BRIMSTONE*/           {G_ASHES,   &brimstoneForeColor,    &brimstoneBackColor,40, 100,DF_INERT_BRIMSTONE,0,DF_INERT_BRIMSTONE,                       10,  NO_LIGHT,   (T_IS_FLAMMABLE | T_SPONTANEOUSLY_IGNITES), 0,                                      "hissing brimstone",    "the jagged brimstone hisses and spits ominously as it crunches under your feet."},
 /*INERT_BRIMSTONE*/            {G_ASHES,   &brimstoneForeColor,    &brimstoneBackColor,40, 0,  DF_INERT_BRIMSTONE,0,DF_ACTIVE_BRIMSTONE,                     800,  NO_LIGHT,   (T_SPONTANEOUSLY_IGNITES), 0,                                                       "hissing brimstone",    "the jagged brimstone hisses and spits ominously as it crunches under your feet."},
 /*OBSIDIAN*/                   {G_FLOOR,   &darkGray,              &obsidianBackColor, 50, 0,  DF_PLAIN_FIRE,0,0,                                              0,  NO_LIGHT,   0, 0,                                                                               "the obsidian ground",  "the ground has fused into obsidian."},
 /*BRIDGE*/                     {G_BRIDGE,  &bridgeFrontColor,      &bridgeBackColor,   45, 50, DF_BRIDGE_FIRE,0,0,                                             0,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "a rickety rope bridge","the rickety rope bridge creaks underfoot."},
 /*BRIDGE_FALLING*/             {G_BRIDGE,  &bridgeFrontColor,      &bridgeBackColor,   45, 50, DF_BRIDGE_FALL,0,DF_BRIDGE_FALL,                            10000,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "a plummeting bridge",  "the bridge is plunging into the chasm before your eyes!"},
 /*BRIDGE_EDGE*/                {G_BRIDGE,  &bridgeFrontColor,      &bridgeBackColor,   45, 50, DF_PLAIN_FIRE,0,0,                                              0,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "a rickety rope bridge","the rickety rope bridge is staked to the edge of the chasm."},
 /*STONE_BRIDGE*/               {G_FLOOR,   &white,                 &chasmEdgeBackColor,20, 50, DF_BRIDGE_FIRE,0,0,                                             0,  NO_LIGHT,   0, 0,                                                                               "a stone bridge",       "the narrow stone bridge winds precariously across the chasm."},
 /*MACHINE_FLOOD_WATER_DORMANT*/{0,     &shallowWaterForeColor, &shallowWaterBackColor, 60, 0,  DF_STEAM_ACCUMULATION,0,DF_SPREADABLE_WATER,                    0,  NO_LIGHT,   (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING), "shallow water", "the water is cold and reaches your knees."},
 /*MACHINE_FLOOD_WATER_SPREADING*/{0,   &shallowWaterForeColor, &shallowWaterBackColor, 60, 0,  DF_STEAM_ACCUMULATION,0,DF_WATER_SPREADS,                    2500,  NO_LIGHT,   (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING), "shallow water", "the water is cold and reaches your knees."},
 /*MACHINE_MUD_DORMANT*/        {G_FLOOR,   &mudForeColor,          &mudBackColor,      55, 0,  DF_PLAIN_FIRE,0,DF_MUD_ACTIVATE,                                0,  NO_LIGHT,   (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_ALLOWS_SUBMERGING), "a bog",         "you are knee-deep in thick, foul-smelling mud."},
 /*ICE_DEEP*/                   {G_FLOOR,   &white,                 &lightBlue,         35, 100,DF_DEEP_WATER_THAW,0,DF_DEEP_WATER_MELTING,                  -100,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "ice",                  "a sheet of ice extends into the water, ripples frozen into its glossy surface."},
 /*ICE_DEEP_MELT*/              {G_FLOOR,   &black,                 &lightBlue,         35, 100,DF_DEEP_WATER_THAW,0,DF_DEEP_WATER_THAW,                    10000,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "melting ice",          "cracks extend across the surface of the ice as it melts before your eyes."},
 /*ICE_SHALLOW*/                {G_FLOOR,   &white,                 &lightBlue,         35, 100,DF_SHALLOW_WATER_THAW,0,DF_SHALLOW_WATER_MELTING,            -100,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "ice",                  "a sheet of ice extends into the water, ripples frozen into its glossy surface."},
 /*ICE_SHALLOW_MELT*/           {G_FLOOR,   &black,                 &lightBlue,         35, 100,DF_DEEP_WATER_THAW,0,DF_SHALLOW_WATER_THAW,                 10000,  NO_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                     "melting ice",          "cracks extend across the surface of the ice as it melts before your eyes."},

 // surface layer
 // tileType                    char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight   flags mechflags                                                                                         description             flavorText
 /*HOLE*/                       {G_CHASM,   &chasmForeColor,        &black,             9,  0,  DF_PLAIN_FIRE,0,DF_HOLE_DRAIN,          -1000,  NO_LIGHT,       (T_AUTO_DESCENT), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "a hole",               "you plunge downward into the hole!"},
 /*HOLE_GLOW*/                  {G_CHASM,   &chasmForeColor,        &black,             9,  0,  DF_PLAIN_FIRE,0,DF_HOLE_DRAIN,          -1000,  DESCENT_LIGHT,  (T_AUTO_DESCENT), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "a hole",               "you plunge downward into the hole!"},
 /*HOLE_EDGE*/                  {G_FLOOR,   &white,                 &chasmEdgeBackColor,50, 0,  DF_PLAIN_FIRE,0,0,                       -500,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION),                                                                  "translucent ground",   "chilly gusts of air blow upward through the translucent floor."},
 /*FLOOD_WATER_DEEP*/           {G_LIQUID,  &deepWaterForeColor,    &deepWaterBackColor,41, 100,DF_STEAM_ACCUMULATION,0,DF_FLOOD_DRAIN,  -200,  NO_LIGHT,       (T_IS_FLAMMABLE | T_IS_DEEP_WATER), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING), "sloshing water", "roiling water floods the room."},
 /*FLOOD_WATER_SHALLOW*/        {0,     &shallowWaterForeColor, &shallowWaterBackColor, 50, 0,  DF_STEAM_ACCUMULATION,0,DF_PUDDLE,       -100,  NO_LIGHT,       (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING), "shallow water",        "knee-deep water drains slowly into holes in the floor."},
 /*GRASS*/                      {G_GRASS,   &grassColor,            0,                  60, 15, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "grass-like fungus",    "grass-like fungus crunches underfoot."},
 /*DEAD_GRASS*/                 {G_GRASS,   &deadGrassColor,        0,                  60, 40, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "withered fungus",      "dead fungus covers the ground."},
 /*GRAY_FUNGUS*/                {G_GRASS,   &grayFungusColor,       0,                  51, 10, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "withered fungus",      "groping tendrils of pale fungus rise from the muck."},
 /*LUMINESCENT_FUNGUS*/         {G_GRASS,   &fungusColor,           0,                  60, 10, DF_PLAIN_FIRE,0,0,                          0,  FUNGUS_LIGHT,   (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "luminescent fungus",   "luminescent fungus casts a pale, eerie glow."},
 /*LICHEN*/                     {G_LICHEN,  &lichenColor,           0,                  60, 50, DF_PLAIN_FIRE,0,DF_LICHEN_GROW,         10000,  NO_LIGHT,       (T_CAUSES_POISON | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                "deadly lichen",        "venomous barbs cover the quivering tendrils of this fast-growing lichen."},
 /*HAY*/                        {G_GRASS,   &hayColor,              &refuseBackColor,   57, 50, DF_STENCH_BURN,0,0,                         0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "filthy hay",           "a pile of hay, matted with filth, has been arranged here as a makeshift bed."},
 /*RED_BLOOD*/                  {G_FLOOR_ALT,&humanBloodColor,      0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pool of blood",      "the floor is splattered with blood."},
 /*GREEN_BLOOD*/                {G_FLOOR_ALT,&insectBloodColor,     0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pool of green blood", "the floor is splattered with green blood."},
 /*PURPLE_BLOOD*/               {G_FLOOR_ALT,&poisonGasColor,       0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pool of purple blood", "the floor is splattered with purple blood."},
 /*ACID_SPLATTER*/              {G_FLOOR_ALT,&acidBackColor,        0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       0, 0,                                                                                               "a puddle of acid",     "the floor is splattered with acid."},
 /*VOMIT*/                      {G_FLOOR_ALT,&vomitColor,           0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a puddle of vomit",    "the floor is caked with vomit."},
 /*URINE*/                      {G_FLOOR_ALT,&urineColor,           0,                  80, 0,  DF_PLAIN_FIRE,0,0,                        100,  NO_LIGHT,       (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                               "a puddle of urine",    "a puddle of urine covers the ground."},
 /*UNICORN_POOP*/               {G_FLOOR_ALT,&white,                0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  UNICORN_POOP_LIGHT,(0), (TM_STAND_IN_TILE),                                                                         "unicorn poop",         "a pile of lavender-scented unicorn poop sparkles with rainbow light."},
 /*WORM_BLOOD*/                 {G_FLOOR_ALT,&wormColor,            0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pool of worm entrails", "worm viscera cover the ground."},
 /*ASH*/                        {G_ASHES,   &ashForeColor,          0,                  80, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pile of ashes",      "charcoal and ash crunch underfoot."},
 /*BURNED_CARPET*/              {G_ASHES,   &ashForeColor,          0,                  87, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "burned carpet",        "the carpet has been scorched by an ancient fire."},
 /*PUDDLE*/                     {G_FLOOR_ALT,&shallowWaterBackColor,0,                  80, 20, 0,0,0,                                    100,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                  "a puddle of water",    "a puddle of water covers the ground."},
 /*BONES*/                      {G_BONES,   &bonesForeColor,        0,                  70, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pile of bones",      "unidentifiable bones, yellowed with age, litter the ground."},
 /*RUBBLE*/                     {G_RUBBLE,  &gray,                  0,                  70, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pile of rubble",     "rocky rubble covers the ground."},
 /*JUNK*/                       {G_BONES,   &mudBackColor,          &refuseBackColor,   50, 20, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "a pile of filthy effects","primitive tools, carvings and trinkets are strewn about the area."},
 /*BROKEN_GLASS*/               {G_FLOOR_ALT,&white,                0,                  70, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (TM_STAND_IN_TILE),                                                                            "shattered glass",      "jagged chunks of glass from the broken dewar litter the ground."},
 /*ECTOPLASM*/                  {G_FLOOR_ALT,&ectoplasmColor,       0,                  70, 0,  DF_PLAIN_FIRE,0,0,                          0,  ECTOPLASM_LIGHT,(0), (TM_STAND_IN_TILE),                                                                            "ectoplasmic residue",  "a thick, glowing substance has congealed on the ground."},
 /*EMBERS*/                     {G_ASHES,   &fireForeColor,         0,                  70, 0,  DF_PLAIN_FIRE,0,DF_ASH,                   300,  EMBER_LIGHT,    (0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                               "sputtering embers",    "sputtering embers cover the ground."},
 /*SPIDERWEB*/                  {G_WEB,     &white,                 0,                  19, 100,DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_ENTANGLES | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),"a spiderweb",       "thick, sticky spiderwebs fill the area."},
 /*NETTING*/                    {G_NET,     &brown,                 0,                  19, 40, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_ENTANGLES | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),"a net",             "a dense tangle of netting fills the area."},
 /*FOLIAGE*/                    {G_FOLIAGE, &foliageColor,          0,                  45, 15, DF_PLAIN_FIRE,0,DF_TRAMPLED_FOLIAGE,        0,  NO_LIGHT,       (T_OBSTRUCTS_VISION | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP), "dense foliage",   "dense foliage fills the area, thriving on what sunlight trickles in."},
 /*DEAD_FOLIAGE*/               {G_FOLIAGE, &deadFoliageColor,      0,                  45, 80, DF_PLAIN_FIRE,0,DF_SMALL_DEAD_GRASS,        0,  NO_LIGHT,       (T_OBSTRUCTS_VISION | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP), "dead foliage",    "the decaying husk of a fungal growth fills the area."},
 /*TRAMPLED_FOLIAGE*/           {G_GRASS,   &foliageColor,          0,                  60, 15, DF_PLAIN_FIRE,0,DF_FOLIAGE_REGROW,        100,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                                     "trampled foliage",     "dense foliage fills the area, thriving on what sunlight trickles in."},
 /*FUNGUS_FOREST*/              {G_FOLIAGE, &fungusForestLightColor,0,                  45, 15, DF_PLAIN_FIRE,0,DF_TRAMPLED_FUNGUS_FOREST,  0,  FUNGUS_FOREST_LIGHT,(T_OBSTRUCTS_VISION | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_STEP),"a luminescent fungal forest", "luminescent fungal growth fills the area, groping upward from the rich soil."},
 /*TRAMPLED_FUNGUS_FOREST*/     {G_GRASS,   &fungusForestLightColor,0,                  60, 15, DF_PLAIN_FIRE,0,DF_FUNGUS_FOREST_REGROW,  100,  FUNGUS_LIGHT,   (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                                     "trampled fungal foliage", "luminescent fungal growth fills the area, groping upward from the rich soil."},
 /*FORCEFIELD*/                 {G_CRYSTAL, &forceFieldColor,       &forceFieldColor,   0,  0,  0,0,DF_FORCEFIELD_MELT,                  -200,  FORCEFIELD_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_GAS | T_OBSTRUCTS_DIAGONAL_MOVEMENT), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_CREATURE),       "a green crystal",      "The translucent green crystal is melting away in front of your eyes."},
 /*FORCEFIELD_MELT*/            {G_CRYSTAL, &black,                 &forceFieldColor,   0,  0,  0,0,0,                                 -10000,  FORCEFIELD_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_GAS | T_OBSTRUCTS_DIAGONAL_MOVEMENT), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_CREATURE),     "a dissolving crystal",     "The translucent green crystal is melting away in front of your eyes."},
 /*SACRED_GLYPH*/               {G_MAGIC_GLYPH, &sacredGlyphColor,  0,                  7,  0,  0,0,0,                                      0,  SACRED_GLYPH_LIGHT, (T_SACRED), 0,                                                                                  "a sacred glyph",       "a sacred glyph adorns the floor, glowing with a powerful warding enchantment."},
 /*MANACLE_TL*/                 {G_CHAIN_TOP_LEFT,      &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the ceiling."},
 /*MANACLE_BR*/                 {G_CHAIN_BOTTOM_RIGHT,  &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the floor."},
 /*MANACLE_TR*/                 {G_CHAIN_TOP_RIGHT,     &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the ceiling."},
 /*MANACLE_BL*/                 {G_CHAIN_BOTTOM_LEFT,   &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the floor."},
 /*MANACLE_T*/                  {G_CHAIN_TOP,           &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the wall."},
 /*MANACLE_B*/                  {G_CHAIN_BOTTOM,        &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the wall."},
 /*MANACLE_L*/                  {G_CHAIN_LEFT,          &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the wall."},
 /*MANACLE_R*/                  {G_CHAIN_RIGHT,         &gray,      0,                  20, 0,  0,0,0,                                      0,  NO_LIGHT,       0, 0,                                                                                               "an iron manacle",      "a thick iron manacle is anchored to the wall."},
 /*PORTAL_LIGHT*/               {0,                     0,          0,                  1,  0,  0,0,0,                                  10000,  PORTAL_ACTIVATE_LIGHT,(0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                         "blinding light",       "blinding light streams out of the archway."},
 /*GUARDIAN_GLOW*/              {0,                     0,          0,                  100,0,  0,0,0,                                  10000,  GLYPH_LIGHT_BRIGHT,(0), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION),                                            "a red glow",           "a red glow fills the area."},
                                                                            
 // fire tiles                  char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*PLAIN_FIRE*/                 {G_FIRE,    &fireForeColor,         0,                  10, 0,  0,0,DF_EMBERS,                            500,  FIRE_LIGHT,     (T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                "billowing flames",     "flames billow upward."},
 /*BRIMSTONE_FIRE*/             {G_FIRE,    &fireForeColor,         0,                  10, 0,  0,0,0,                                   2500,  BRIMSTONE_FIRE_LIGHT,(T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),           "sulfurous flames",     "sulfurous flames leap from the unstable bed of brimstone."},
 /*FLAMEDANCER_FIRE*/           {G_FIRE,    &fireForeColor,         0,                  10, 0,  0,0,DF_OBSIDIAN,                         5000,  FIRE_LIGHT,     (T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                "clouds of infernal flame", "billowing infernal flames eat at the floor."},
 /*GAS_FIRE*/                   {G_FIRE,    &fireForeColor,         0,                  10, 0,  0,0,0,                                   8000,  FIRE_LIGHT,     (T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                "a cloud of burning gas", "burning gas fills the air with flame."},
 /*GAS_EXPLOSION*/              {G_FIRE,    &yellow,                0,                  10, 0,  0,0,0,                                  10000,  EXPLOSION_LIGHT,(T_IS_FIRE | T_CAUSES_EXPLOSIVE_DAMAGE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT), "a violent explosion", "the force of the explosion slams into you."},
 /*DART_EXPLOSION*/             {G_FIRE,    &white,                 0,                  10, 0,  0,0,0,                                  10000,  INCENDIARY_DART_LIGHT ,(T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),         "a flash of fire",      "flames burst out of the incendiary dart."},
 /*ITEM_FIRE*/                  {G_FIRE,    &white,                 0,                  10, 0,  0,0,DF_EMBERS,                           3000,  FIRE_LIGHT,     (T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                "crackling flames",     "crackling flames rise from the blackened item."},
 /*CREATURE_FIRE*/              {G_FIRE,    &white,                 0,                  10, 0,  0,0,DF_EMBERS,                           3000,  FIRE_LIGHT,     (T_IS_FIRE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT),                "greasy flames",        "greasy flames rise from the corpse."},

 // gas layer
 /*POISON_GAS*/                 {' ',       0,                      &poisonGasColor,    35, 100,DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_IS_FLAMMABLE | T_CAUSES_DAMAGE), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES),                         "a cloud of caustic gas", "you can feel the purple gas eating at your flesh."},
 /*CONFUSION_GAS*/              {' ',       0,                      &confusionGasColor, 35, 100,DF_GAS_FIRE,0,0,                            0,  CONFUSION_GAS_LIGHT,(T_IS_FLAMMABLE | T_CAUSES_CONFUSION), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),          "a cloud of confusion gas", "the rainbow-colored gas tickles your brain."},
 /*ROT_GAS*/                    {' ',       0,                      &vomitColor,        35, 100,DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_IS_FLAMMABLE | T_CAUSES_NAUSEA), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),                 "a cloud of putrescence", "the stench of rotting flesh is overpowering."},
 /*STENCH_SMOKE_GAS*/           {' ',       0,                      &vomitColor,        35, 0,  DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_CAUSES_NAUSEA), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),                                  "a cloud of putrid smoke", "you retch violently at the smell of the greasy smoke."},
 /*PARALYSIS_GAS*/              {' ',       0,                      &pink,              35, 100,DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_IS_FLAMMABLE | T_CAUSES_PARALYSIS), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),              "a cloud of paralytic gas", "the pale gas causes your muscles to stiffen."},
 /*METHANE_GAS*/                {' ',       0,                      &methaneColor,      35, 100,DF_GAS_FIRE,0,DF_EXPLOSION_FIRE,            0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_EXPLOSIVE_PROMOTE),                                        "a cloud of explosive gas", "the smell of explosive swamp gas fills the air."},
 /*STEAM*/                      {' ',       0,                      &white,             35, 0,  DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_CAUSES_DAMAGE), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),                                  "a cloud of scalding steam", "scalding steam fills the air!"},
 /*DARKNESS_CLOUD*/             {' ',       0,                      0,                  35, 0,  DF_GAS_FIRE,0,0,                            0,  DARKNESS_CLOUD_LIGHT,   (0), (TM_STAND_IN_TILE),                                                                    "a cloud of supernatural darkness", "everything is obscured by an aura of supernatural darkness."},
 /*HEALING_CLOUD*/              {' ',       0,                      &darkRed,           35, 0,  DF_GAS_FIRE,0,0,                            0,  NO_LIGHT,       (T_CAUSES_HEALING), (TM_STAND_IN_TILE | TM_GAS_DISSIPATES_QUICKLY),                                 "a cloud of healing spores", "bloodwort spores, renowned for their healing properties, fill the air."},

 // bloodwort pods              char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*BLOODFLOWER_STALK*/  {G_BLOODWORT_STALK, &bloodflowerForeColor,&bloodflowerBackColor,10, 20, DF_PLAIN_FIRE,0,DF_BLOODFLOWER_PODS_GROW, 100,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "a bloodwort stalk", "this spindly plant grows seed pods famous for their healing properties."},
 /*BLOODFLOWER_POD*/    {G_BLOODWORT_POD,   &bloodflowerPodForeColor,0,                 11, 20, DF_BLOODFLOWER_POD_BURST,0,DF_BLOODFLOWER_POD_BURST, 0,NO_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_PLAYER_ENTRY | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED), "a bloodwort pod", "the bloodwort seed pod bursts, releasing a cloud of healing spores."},

 // shrine accoutrements
 /*HAVEN_BEDROLL*/              {G_BEDROLL, &black,                 &bedrollBackColor,  57, 50, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                                     "an abandoned bedroll", "a bedroll lies in the corner, disintegrating with age."},

 // algae
 /*DEEP_WATER_ALGAE_WELL*/      {G_FLOOR,   &floorForeColor,        &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,DF_ALGAE_1,               100,  NO_LIGHT,       0, 0,                                                                                               "the ground",           ""},
 /*DEEP_WATER_ALGAE_1*/         {G_LIQUID,  &deepWaterForeColor,    &deepWaterBackColor,40, 100,DF_STEAM_ACCUMULATION,0,DF_ALGAE_1,       500,  LUMINESCENT_ALGAE_BLUE_LIGHT,(T_IS_FLAMMABLE | T_IS_DEEP_WATER), (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING),  "luminescent waters",   "blooming algae fills the waters with a swirling luminescence."},
 /*DEEP_WATER_ALGAE_2*/         {G_LIQUID,  &deepWaterForeColor,    &deepWaterBackColor,39, 100,DF_STEAM_ACCUMULATION,0,DF_ALGAE_REVERT,  300,  LUMINESCENT_ALGAE_GREEN_LIGHT,(T_IS_FLAMMABLE | T_IS_DEEP_WATER), (TM_STAND_IN_TILE | TM_EXTINGUISHES_FIRE | TM_ALLOWS_SUBMERGING), "luminescent waters",   "blooming algae fills the waters with a swirling luminescence."},

 // ancient spirit terrain
 /*ANCIENT_SPIRIT_VINES*/       {G_VINE,    &lichenColor,           0,                  19, 100,DF_PLAIN_FIRE,0,DF_ANCIENT_SPIRIT_GRASS, 1000,  NO_LIGHT,       (T_ENTANGLES | T_CAUSES_DAMAGE | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_VISUALLY_DISTINCT | TM_PROMOTES_ON_PLAYER_ENTRY),"thorned vines",       "thorned vines make a rustling noise as they quiver restlessly."}, // +tile
 /*ANCIENT_SPIRIT_GRASS*/       {G_GRASS,   &grassColor,            0,                  60, 15, DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_STAND_IN_TILE),                                                               "a tuft of grass",      "tufts of lush grass have improbably pushed upward through the stone ground."},

 // Yendor amulet floor tile
 /*AMULET_SWITCH*/              {G_FLOOR,   &floorForeColor,        &floorBackColor,    95, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       0, (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_PROMOTES_ON_ITEM_PICKUP),                         "the ground",           ""},

 // commutation device          char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*COMMUTATION_ALTAR*/         {G_ORB_ALTAR,&altarForeColor,       &greenAltarBackColor,17, 0,  0,0,DF_ALTAR_COMMUTE,                       0,  NO_LIGHT,       (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_SWAP_ENCHANTS_ACTIVATION | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),    "a commutation altar",  "crude diagrams on this altar and its twin invite you to place items upon them."},
 /*COMMUTATION_ALTAR_INERT*/   {G_ORB_ALTAR,&black,                &greenAltarBackColor,17, 0,  0,0,0,                                      0,  NO_LIGHT,       (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                         "a scorched altar",     "scorch marks cover the surface of the altar, but it is cold to the touch."},
 /*PIPE_GLOWING*/               {G_PIPES,   &veryDarkGray,          0,                  45, 0,  DF_PLAIN_FIRE,0,DF_INERT_PIPE,              0,  CONFUSION_GAS_LIGHT, (0), (TM_IS_WIRED | TM_VANISHES_UPON_PROMOTION),                                               "glowing glass pipes",  "glass pipes are set into the floor and emit a soft glow of shifting color."},
 /*PIPE_INERT*/                 {G_PIPES,   &black,                 0,                  45, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (0), (0),                                                                                           "charred glass pipes",  "the inside of the glass pipes are charred."},

 // resurrection altar
 /*RESURRECTION_ALTAR*/         {G_ALTAR,   &altarForeColor,        &goldAltarBackColor,17, 0,  0,0,DF_ALTAR_RESURRECT,                     0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),           "a resurrection altar", "the souls of the dead surround you. A deceased ally might be called back."},
 /*RESURRECTION_ALTAR_INERT*/   {G_ALTAR,   &black,                 &goldAltarBackColor,16, 0,  0,0,0,                                      0,  NO_LIGHT,       (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),                         "a scorched altar",     "scorch marks cover the surface of the altar, but it is cold to the touch."},
 /*MACHINE_TRIGGER_FLOOR_REPEATING*/{0,     0,                      0,                  95, 0,  0,0,0,                                      0,  NO_LIGHT,       (0), (TM_IS_WIRED | TM_PROMOTES_ON_PLAYER_ENTRY),                                                   "the ground",           ""},

 // sacrifice altar
 /*SACRIFICE_ALTAR_DORMANT*/  {G_SAC_ALTAR, &altarForeColor,        &altarBackColor,    17, 0,  0,0,DF_SACRIFICE_ALTAR,                     0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT), "a sacrificial altar", "demonological symbols decorate this altar."},
 /*SACRIFICE_ALTAR*/          {G_SAC_ALTAR, &altarForeColor,        &altarBackColor,    17, 0,  0,0,DF_SACRIFICE_COMPLETE,                  0,  CANDLE_LIGHT,   (T_OBSTRUCTS_SURFACE_EFFECTS), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_PROMOTES_ON_SACRIFICE_ENTRY), "a sacrifice altar",    "demonological symbols decorate this altar."},
 /*SACRIFICE_LAVA*/             {G_LIQUID,  &fireForeColor,         &lavaBackColor,     40, 0,  DF_OBSIDIAN,0,0,                            0,  LAVA_LIGHT,     (T_LAVA_INSTA_DEATH), (TM_ALLOWS_SUBMERGING | TM_LIST_IN_SIDEBAR),                                  "a sacrificial pit",      "the smell of burnt flesh lingers over this pit of lava."},
 /*SACRIFICE_CAGE_DORMANT*/     {G_WALL,    &altarBackColor,        &veryDarkGray,      17, 0,  0,0,DF_SACRIFICE_CAGE_ACTIVE,               0,  CANDLE_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT),"an iron cage","the cage won't budge. Perhaps there is a way to raise it nearby..."},
 /*DEMONIC_STATUE*/             {G_STATUE,  &wallBackColor,         &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,0,                          0,  DEMONIC_STATUE_LIGHT,   (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE),  "a demonic statue", "An obsidian statue of a leering demon looms over the room."},

 // doorway statues
 /*STATUE_INERT_DOORWAY*/{G_CRACKED_STATUE, &wallBackColor,         &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_CONNECTS_LEVEL),  "a broken statue",  "Once magnificent, the statue has crumbled beyond recognition."},
 /*STATUE_DORMANT_DOORWAY*/     {G_STATUE,  &wallBackColor,         &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,DF_CRACKING_STATUE,         0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_CONNECTS_LEVEL),"a marble statue", "The cold marble statue has weathered the years with grace."},

 // extensible stone bridge     char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*CHASM_WITH_HIDDEN_BRIDGE*/   {G_CHASM,   &chasmForeColor,        &black,             40, 0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_AUTO_DESCENT), (TM_STAND_IN_TILE),                                                               "a chasm",              "you plunge downward into the chasm!"},
 /*CHASM_WITH_HIDDEN_BRIDGE_ACTIVE*/{G_FLOOR,&white,                &chasmEdgeBackColor,40, 0,  DF_PLAIN_FIRE,0,DF_BRIDGE_ACTIVATE,      6000,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION),                                                                  "a stone bridge",       "the narrow stone bridge is extending across the chasm."},
 /*MACHINE_CHASM_EDGE*/         {G_FLOOR,   &white,                 &chasmEdgeBackColor,80, 0,  DF_PLAIN_FIRE,0,DF_BRIDGE_ACTIVATE_ANNOUNCE,0,  NO_LIGHT,       (0), (TM_IS_WIRED),                                                                                 "the brink of a chasm", "chilly winds blow upward from the stygian depths."},

 // rat trap
 /*RAT_TRAP_WALL_DORMANT*/      {G_WALL,    &wallForeColor,         &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_WALL_CRACK,              0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),            "a stone wall",         "The rough stone wall is firm and unyielding."},
 /*RAT_TRAP_WALL_CRACKING*/     {G_WALL,    &wallForeColor,         &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_WALL_SHATTER,          500,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_LIST_IN_SIDEBAR),     "a cracking wall",      "Cracks are running ominously across the base of this rough stone wall."},

 // electric crystals
 /*ELECTRIC_CRYSTAL_OFF*/{G_ELECTRIC_CRYSTAL,&wallCrystalColor,     &graniteBackColor,  0,  0,  DF_PLAIN_FIRE,0,DF_ELECTRIC_CRYSTAL_ON,     0,  NO_LIGHT,       (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_PROMOTES_ON_ELECTRICITY | TM_IS_CIRCUIT_BREAKER | TM_IS_WIRED | TM_LIST_IN_SIDEBAR), "a darkened crystal globe", "A slight electric shock startles you when you touch the inert crystal globe."},
 /*ELECTRIC_CRYSTAL_ON*/{G_ELECTRIC_CRYSTAL,&white,                 &wallCrystalColor,  0,  0,  DF_PLAIN_FIRE,0,0,                          0,  CRYSTAL_WALL_LIGHT,(T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS), (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR), "a shining crystal globe", "Crackling light streams out of the crystal globe."},
 /*TURRET_LEVER*/                {G_LEVER,  &wallForeColor,         &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_TURRET_LEVER,            0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_PROMOTES_ON_PLAYER_ENTRY | TM_LIST_IN_SIDEBAR | TM_VISUALLY_DISTINCT | TM_INVERT_WHEN_HIGHLIGHTED),"a lever", "The lever moves."},

 // worm tunnels
 /*WORM_TUNNEL_MARKER_DORMANT*/ {0,         0,                      0,                  100,0,  0,0,DF_WORM_TUNNEL_MARKER_ACTIVE,           0,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED),                                                    "",                     ""},
 /*WORM_TUNNEL_MARKER_ACTIVE*/  {0,         0,                      0,                  100,0,  0,0,DF_GRANITE_CRUMBLES,                -2000,  NO_LIGHT,       (0), (TM_VANISHES_UPON_PROMOTION),                                                                  "a rough granite wall", "The granite is split open with splinters of rock jutting out at odd angles."},
 /*WORM_TUNNEL_OUTER_WALL*/     {G_WALL,    &wallForeColor,         &wallBackColor,     0,  0,  DF_PLAIN_FIRE,0,DF_WALL_SHATTER,            0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE | TM_VANISHES_UPON_PROMOTION | TM_IS_WIRED | TM_CONNECTS_LEVEL),"a stone wall", "The rough stone wall is firm and unyielding."},

 // zombie crypt
 /*BRAZIER*/                    {G_FIRE,    &fireForeColor,         &statueBackColor,   0,  0,  DF_PLAIN_FIRE,0,0,                          0,  BURNING_CREATURE_LIGHT, (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_ITEMS | T_IS_FIRE), (TM_STAND_IN_TILE | TM_LIST_IN_SIDEBAR),"a ceremonial brazier",      "The ancient brazier smolders with a deep crimson flame."},

 // goblin warren               char        fore color              back color        prio ign% [fire,discover,promote]Type     promoteChance   glowLight       flags mechflags                                                                                     description             flavorText
 /*MUD_FLOOR*/                  {G_FLOOR,   &mudBackColor,          &refuseBackColor,   85, 0,  DF_STENCH_SMOLDER,0,0,                      0,  NO_LIGHT,       (T_IS_FLAMMABLE), (TM_VANISHES_UPON_PROMOTION),                                                     "the mud floor",        "Rotting animal matter has been ground into the mud floor; the stench is awful."},
 /*MUD_WALL*/                   {G_WALL,    &mudWallForeColor,      &mudWallBackColor,  0,  0,  DF_PLAIN_FIRE,0,0,                          0,  NO_LIGHT,       (T_OBSTRUCTS_EVERYTHING), (TM_STAND_IN_TILE),                                                       "a mud-covered wall",   "A malodorous layer of clay and fecal matter coats the wall."},
 /*MUD_DOORWAY*/                {G_DOORWAY, &mudWallForeColor,      &refuseBackColor,   25, 50, DF_EMBERS,0,0,                              0,  NO_LIGHT,       (T_OBSTRUCTS_VISION | T_OBSTRUCTS_GAS | T_IS_FLAMMABLE), (TM_STAND_IN_TILE | TM_VISUALLY_DISTINCT), "hanging animal skins", "you push through the animal skins that hang across the threshold."},
};

unsigned long terrainFlags(pos p) {
    return (
        tileCatalog[pmapAt(p)->layers[DUNGEON]].flags
        | tileCatalog[pmapAt(p)->layers[LIQUID]].flags
        | tileCatalog[pmapAt(p)->layers[SURFACE]].flags
        | tileCatalog[pmapAt(p)->layers[GAS]].flags
    );
}

unsigned long terrainMechFlags(pos loc) {
    return (
        tileCatalog[pmapAt(loc)->layers[DUNGEON]].mechFlags
        | tileCatalog[pmapAt(loc)->layers[LIQUID]].mechFlags
        | tileCatalog[pmapAt(loc)->layers[SURFACE]].mechFlags
        | tileCatalog[pmapAt(loc)->layers[GAS]].mechFlags
    );
}


// Features in the gas layer use the startprob as volume, ignore probdecr, and spawn in only a single point.
// Intercepts and slopes are in units of 0.01.
// This cannot be const, since messageDisplayed is set
dungeonFeature dungeonFeatureCatalog[NUMBER_DUNGEON_FEATURES] = {
    // tileType                 layer       start   decr    fl  txt  flare   fCol fRad  propTerrain subseqDF
    {0}, // nothing
    {GRANITE,                   DUNGEON,    80,     70,     DFF_CLEAR_OTHER_TERRAIN},
    {CRYSTAL_WALL,              DUNGEON,    200,    50,     DFF_CLEAR_OTHER_TERRAIN},
    {LUMINESCENT_FUNGUS,        SURFACE,    60,     8,      DFF_BLOCKED_BY_OTHER_LAYERS},
    {GRASS,                     SURFACE,    75,     5,      DFF_BLOCKED_BY_OTHER_LAYERS},
    {DEAD_GRASS,                SURFACE,    75,     5,      DFF_BLOCKED_BY_OTHER_LAYERS,    "", 0,  0,  0,      0,          DF_DEAD_FOLIAGE},
    {BONES,                     SURFACE,    75,     23,     0},
    {RUBBLE,                    SURFACE,    45,     23,     0},
    {FOLIAGE,                   SURFACE,    100,    33,     (DFF_BLOCKED_BY_OTHER_LAYERS)},
    {FUNGUS_FOREST,             SURFACE,    100,    45,     (DFF_BLOCKED_BY_OTHER_LAYERS)},
    {DEAD_FOLIAGE,              SURFACE,    50,     30,     (DFF_BLOCKED_BY_OTHER_LAYERS)},

    // misc. liquids
    {SUNLIGHT_POOL,             LIQUID,     65,     6,      0},
    {DARKNESS_PATCH,            LIQUID,     65,     11,     0},

    // Dungeon features spawned during gameplay:

    // revealed secrets
    {DOOR,                      DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {GAS_TRAP_POISON,           DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {GAS_TRAP_PARALYSIS,        DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {CHASM_EDGE,                LIQUID,     100,    100,    0, "", GENERIC_FLASH_LIGHT},
    {TRAP_DOOR,                 LIQUID,     0,      0,      DFF_CLEAR_OTHER_TERRAIN, "", GENERIC_FLASH_LIGHT, 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
    {GAS_TRAP_CONFUSION,        DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {FLAMETHROWER,              DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {FLOOD_TRAP,                DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {NET_TRAP,                  DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},
    {ALARM_TRAP,                DUNGEON,    0,      0,      0, "", GENERIC_FLASH_LIGHT},

    // bloods
    // Start probability is actually a percentage for bloods.
    // Base probability is 15 + (damage * 2/3), and then take the given percentage of that.
    // If it's a gas, we multiply the base by an additional 100.
    // Thus to get a starting gas volume of a poison potion (1000), with a hit for 10 damage, use a starting probability of 48.
    {RED_BLOOD,                 SURFACE,    100,    25,     0},
    {GREEN_BLOOD,               SURFACE,    100,    25,     0},
    {PURPLE_BLOOD,              SURFACE,    100,    25,     0},
    {WORM_BLOOD,                SURFACE,    100,    25,     0},
    {ACID_SPLATTER,             SURFACE,    200,    25,     0},
    {ASH,                       SURFACE,    50,     25,     0},
    {EMBERS,                    SURFACE,    125,    25,     0},
    {ECTOPLASM,                 SURFACE,    110,    25,     0},
    {RUBBLE,                    SURFACE,    33,     25,     0},
    {ROT_GAS,                   GAS,        12,     0,      0},

    // monster effects
    {VOMIT,                     SURFACE,    30,     10,     0},
    {POISON_GAS,                GAS,        2000,   0,      0},
    {GAS_EXPLOSION,             SURFACE,    350,    100,    0,  "", EXPLOSION_FLARE_LIGHT},
    {RED_BLOOD,                 SURFACE,    150,    30,     0},
    {FLAMEDANCER_FIRE,          SURFACE,    200,    75,     0},

    // mutation effects
    {GAS_EXPLOSION,             SURFACE,    350,    100,    0,  "The corpse detonates with terrifying force!", EXPLOSION_FLARE_LIGHT},
    {LICHEN,                    SURFACE,    70,     60,     0,  "Poisonous spores burst from the corpse!"},

    // misc
    {NOTHING,                   GAS,        0,      0,      DFF_EVACUATE_CREATURES_FIRST},
    {ROT_GAS,                   GAS,        15,     0,      0},
    {STEAM,                     GAS,        325,    0,      0},
    {STEAM,                     GAS,        15,     0,      0},
    {METHANE_GAS,               GAS,        2,      0,      0},
    {EMBERS,                    SURFACE,    0,      0,      0},
    {URINE,                     SURFACE,    65,     25,     0},
    {UNICORN_POOP,              SURFACE,    65,     40,     0},
    {PUDDLE,                    SURFACE,    13,     25,     0},
    {ASH,                       SURFACE,    0,      0,      0},
    {ECTOPLASM,                 SURFACE,    0,      0,      0},
    {FORCEFIELD,                SURFACE,    100,    50,     0},
    {FORCEFIELD_MELT,           SURFACE,    0,      0,      0},
    {SACRED_GLYPH,              SURFACE,    100,    100,    0,  "", EMPOWERMENT_LIGHT},
    {LICHEN,                    SURFACE,    2,      100,    (DFF_BLOCKED_BY_OTHER_LAYERS)}, // Lichen won't spread through lava.
    {RUBBLE,                    SURFACE,    45,     23,     (DFF_ACTIVATE_DORMANT_MONSTER)},
    {RUBBLE,                    SURFACE,    0,      0,      (DFF_ACTIVATE_DORMANT_MONSTER)},

    {SPIDERWEB,                 SURFACE,    15,     12,     0},
    {SPIDERWEB,                 SURFACE,    100,    39,     0},

    {ANCIENT_SPIRIT_VINES,      SURFACE,    75,     70,     0},
    {ANCIENT_SPIRIT_GRASS,      SURFACE,    50,     47,     0},

    // foliage
    {TRAMPLED_FOLIAGE,          SURFACE,    0,      0,      0},
    {DEAD_GRASS,                SURFACE,    75,     75,     0},
    {FOLIAGE,                   SURFACE,    0,      0,      (DFF_BLOCKED_BY_OTHER_LAYERS)},
    {TRAMPLED_FUNGUS_FOREST,    SURFACE,    0,      0,      0},
    {FUNGUS_FOREST,             SURFACE,    0,      0,      (DFF_BLOCKED_BY_OTHER_LAYERS)},

    // brimstone
    {ACTIVE_BRIMSTONE,          LIQUID,     0,      0,      0},
    {INERT_BRIMSTONE,           LIQUID,     0,      0,      0,  "", 0,  0,  0,      0,          DF_BRIMSTONE_FIRE},

    // bloodwort
    {BLOODFLOWER_POD,           SURFACE,    60,     60,     DFF_EVACUATE_CREATURES_FIRST},
    {BLOODFLOWER_POD,           SURFACE,    10,     10,     DFF_EVACUATE_CREATURES_FIRST},
    {HEALING_CLOUD,             GAS,        350,    0,      0},

    // dewars
    {POISON_GAS,                GAS,        20000,  0,      0, "the dewar shatters and pressurized caustic gas explodes outward!", 0, &poisonGasColor, 4, 0, DF_DEWAR_GLASS},
    {CONFUSION_GAS,             GAS,        20000,  0,      0, "the dewar shatters and pressurized confusion gas explodes outward!", 0, &confusionGasColor, 4, 0, DF_DEWAR_GLASS},
    {PARALYSIS_GAS,             GAS,        20000,  0,      0, "the dewar shatters and pressurized paralytic gas explodes outward!", 0, &pink, 4, 0, DF_DEWAR_GLASS},
    {METHANE_GAS,               GAS,        20000,  0,      0, "the dewar shatters and pressurized methane gas explodes outward!", 0, &methaneColor, 4, 0, DF_DEWAR_GLASS},
    {BROKEN_GLASS,              SURFACE,    100,    70,     0},
    {CARPET,                    DUNGEON,    120,    20,     0},

    // algae
    {DEEP_WATER_ALGAE_WELL,     DUNGEON,    0,      0,      DFF_SUPERPRIORITY},
    {DEEP_WATER_ALGAE_1,        LIQUID,     50,     100,    0,  "", 0,  0,   0,     DEEP_WATER, DF_ALGAE_2},
    {DEEP_WATER_ALGAE_2,        LIQUID,     0,      0,      0},
    {DEEP_WATER,                LIQUID,     0,      0,      DFF_SUPERPRIORITY},

    // doors, item cages, altars, glyphs, guardians -- reusable machine components
    {OPEN_DOOR,                 DUNGEON,    0,      0,      0},
    {DOOR,                      DUNGEON,    0,      0,      0},
    {OPEN_IRON_DOOR_INERT,      DUNGEON,    0,      0,      0,  "", GENERIC_FLASH_LIGHT},
    {ALTAR_CAGE_OPEN,           DUNGEON,    0,      0,      0,  "the cages lift off of the altars as you approach.", GENERIC_FLASH_LIGHT},
    {ALTAR_CAGE_CLOSED,         DUNGEON,    0,      0,      (DFF_EVACUATE_CREATURES_FIRST), "the cages lower to cover the altars.", GENERIC_FLASH_LIGHT},
    {ALTAR_INERT,               DUNGEON,    0,      0,      0},
    {FLOOR_FLOODABLE,           DUNGEON,    0,      0,      0,  "the altar retracts into the ground with a grinding sound.", GENERIC_FLASH_LIGHT},
    {PORTAL_LIGHT,              SURFACE,    0,      0,      (DFF_EVACUATE_CREATURES_FIRST | DFF_ACTIVATE_DORMANT_MONSTER), "the archway flashes, and you catch a glimpse of another world!"},
    {MACHINE_GLYPH_INACTIVE,    DUNGEON,    0,      0,      0},
    {MACHINE_GLYPH,             DUNGEON,    0,      0,      0},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  ""},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  "the glyph beneath you glows, and the guardians take a step!"},
    {GUARDIAN_GLOW,             SURFACE,    0,      0,      0,  "the mirrored totem flashes, reflecting the red glow of the glyph beneath you."},
    {MACHINE_GLYPH,             DUNGEON,    200,    95,     DFF_BLOCKED_BY_OTHER_LAYERS},
    {WALL_LEVER,                DUNGEON,    0,      0,      0,  "you notice a lever hidden behind a loose stone in the wall.", GENERIC_FLASH_LIGHT},
    {WALL_LEVER_PULLED,         DUNGEON,    0,      0,      0},
    {WALL_LEVER_HIDDEN,         DUNGEON,    0,      0,      0},

    {BRIDGE_FALLING,            LIQUID,     200,    100,    0, "", 0, 0, 0, BRIDGE},
    {CHASM,                     LIQUID,     0,      0,      0, "", GENERIC_FLASH_LIGHT, 0, 0, 0, DF_BRIDGE_FALL_PREP},

    // fire
    {PLAIN_FIRE,                SURFACE,    0,      0,      0},
    {GAS_FIRE,                  SURFACE,    0,      0,      0},
    {GAS_EXPLOSION,             SURFACE,    60,     17,     0},
    {DART_EXPLOSION,            SURFACE,    0,      0,      0},
    {BRIMSTONE_FIRE,            SURFACE,    0,      0,      0},
    {0,                         0,          0,      0,      0,  "the rope bridge snaps from the heat and plunges into the chasm!", FALLEN_TORCH_FLASH_LIGHT,    0,  0,      0,      DF_BRIDGE_FALL},
    {PLAIN_FIRE,                SURFACE,    100,    37,     0},
    {EMBERS,                    SURFACE,    0,      0,      0},
    {EMBERS,                    SURFACE,    100,    94,     0},
    {OBSIDIAN,                  SURFACE,    0,      0,      DFF_CLEAR_LOWER_PRIORITY_TERRAIN},
    {ITEM_FIRE,                 SURFACE,    0,      0,      0,  "", FALLEN_TORCH_FLASH_LIGHT},
    {CREATURE_FIRE,             SURFACE,    0,      0,      0,  "", FALLEN_TORCH_FLASH_LIGHT},

    {FLOOD_WATER_SHALLOW,       SURFACE,    225,    37,     0,  "", 0,  0,  0,      0,          DF_FLOOD_2},
    {FLOOD_WATER_DEEP,          SURFACE,    175,    37,     0,  "the area is flooded as water rises through imperceptible holes in the ground."},
    {FLOOD_WATER_SHALLOW,       SURFACE,    10,     25,     0},
    {HOLE,                      SURFACE,    200,    100,    0},
    {HOLE_EDGE,                 SURFACE,    0,      0,      0},

    // ice effects
    {ICE_DEEP,                  LIQUID,     150,    50,     DFF_EVACUATE_CREATURES_FIRST,   "", 0,  0,  0,      DEEP_WATER,         DF_ALGAE_1_FREEZE},
    {ICE_DEEP,                  LIQUID,     150,    50,     DFF_EVACUATE_CREATURES_FIRST,   "", 0,  0,  0,      DEEP_WATER_ALGAE_1, DF_ALGAE_2_FREEZE},
    {ICE_DEEP,                  LIQUID,     150,    50,     DFF_EVACUATE_CREATURES_FIRST,   "", 0,  0,  0,      DEEP_WATER_ALGAE_2, DF_SHALLOW_WATER_FREEZE},
    {ICE_DEEP_MELT,             LIQUID,     0,      0,      0},
    {DEEP_WATER,                LIQUID,     0,      0,      0},
    {ICE_SHALLOW,               LIQUID,     100,    50,     DFF_EVACUATE_CREATURES_FIRST,   "", 0,  0,  0,      SHALLOW_WATER},
    {ICE_SHALLOW_MELT,          LIQUID,     0,      0,      0},
    {SHALLOW_WATER,             LIQUID,     0,      0,      0},

    // gas trap effects
    {POISON_GAS,                GAS,        1000,   0,      0,  "a cloud of caustic gas sprays upward from the floor!"},
    {CONFUSION_GAS,             GAS,        300,    0,      0,  "a sparkling cloud of confusion gas sprays upward from the floor!"},
    {NETTING,                   SURFACE,    300,    90,     0,  "a net falls from the ceiling!"},
    {0,                         0,          0,      0,      DFF_AGGRAVATES_MONSTERS, "a piercing shriek echoes through the nearby rooms!", 0, 0, DCOLS/2},
    {METHANE_GAS,               GAS,        10000,  0,      0}, // debugging toy

    // potions
    {POISON_GAS,                GAS,        1000,   0,      0,  "", 0,  &poisonGasColor,4},
    {PARALYSIS_GAS,             GAS,        1000,   0,      0,  "", 0,  &pink,4},
    {CONFUSION_GAS,             GAS,        1000,   0,      0,  "", 0,  &confusionGasColor, 4},
    {PLAIN_FIRE,                SURFACE,    100,    37,     0,  "", EXPLOSION_FLARE_LIGHT},
    {DARKNESS_CLOUD,            GAS,        200,    0,      0},
    {HOLE_EDGE,                 SURFACE,    300,    100,    0,  "", 0,  &darkBlue,3,0,          DF_HOLE_2},
    {LICHEN,                    SURFACE,    70,     60,     0},

    // other items
    {PLAIN_FIRE,                SURFACE,    100,    45,     0,  "", 0,  &yellow,3},
    {HOLE_GLOW,                 SURFACE,    200,    100,    DFF_SUBSEQ_EVERYWHERE,  "", 0,  &darkBlue,3,0,          DF_STAFF_HOLE_EDGE},
    {HOLE_EDGE,                 SURFACE,    100,    100,    0},

    // machine components

    // commutation altars
    {COMMUTATION_ALTAR_INERT,   DUNGEON,    0,      0,      0,  "the items on the two altars flash with a brilliant light!", SCROLL_ENCHANTMENT_LIGHT},
    {PIPE_GLOWING,              SURFACE,    90,     60,     0},
    {PIPE_INERT,                SURFACE,    0,      0,      0,  "", SCROLL_ENCHANTMENT_LIGHT},

    // resurrection altars
    {RESURRECTION_ALTAR_INERT,  DUNGEON,    0,      0,      DFF_RESURRECT_ALLY, "An old friend emerges from a bloom of sacred light!", EMPOWERMENT_LIGHT},
    {MACHINE_TRIGGER_FLOOR_REPEATING, LIQUID, 300,  100,    DFF_SUPERPRIORITY, "", 0, 0, 0, CARPET},

    // sacrifice altars
    {SACRIFICE_ALTAR,           DUNGEON,    0,      0,      0,  "a demonic presence whispers its demand: \"Bring to me the marked sacrifice!\""},
    {SACRIFICE_LAVA,            DUNGEON,    0,      0,      0,  "demonic cackling echoes through the room as the altar plunges downward!"},
    {ALTAR_CAGE_RETRACTABLE,    DUNGEON,    0,      0,      0},

    // coffin bursts open to reveal vampire:
    {COFFIN_OPEN,               DUNGEON,    0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "the coffin opens and a dark figure rises!", 0, &darkGray, 3},
    {PLAIN_FIRE,                SURFACE,    0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "as flames begin to lick the coffin, its tenant bursts forth!", 0, 0, 0, 0, DF_EMBERS_PATCH},
    {MACHINE_TRIGGER_FLOOR,     DUNGEON,    200,    100,    0},

    // throwing tutorial:
    {ALTAR_INERT,               DUNGEON,    0,      0,      0,  "the cage lifts off of the altar.", GENERIC_FLASH_LIGHT},
    {TRAP_DOOR,                 LIQUID,     225,    100,    (DFF_CLEAR_OTHER_TERRAIN | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
    {LAVA,                      LIQUID,     225,    100,    (DFF_CLEAR_OTHER_TERRAIN)},
    {MACHINE_PRESSURE_PLATE_USED,DUNGEON,   0,      0,      0},

    // rat trap:
    {RAT_TRAP_WALL_CRACKING,    DUNGEON,    0,      0,      0,  "a scratching sound emanates from the nearby walls!", 0, 0, 0, 0, DF_RUBBLE},

    // wooden barricade at entrance:
    {PLAIN_FIRE,                SURFACE,    0,      0,      0,  "flames quickly consume the wooden barricade."},

    // wooden barricade around altar:
    {WOODEN_BARRICADE,          DUNGEON,    220,    100,    (DFF_TREAT_AS_BLOCKING | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, 0, DF_SMALL_DEAD_GRASS},

    // shallow water flood machine:
    {MACHINE_FLOOD_WATER_SPREADING, LIQUID, 0,      0,      0,  "you hear a heavy click, and the nearby water begins flooding the area!"},
    {SHALLOW_WATER,             LIQUID,     0,      0,      0},
    {MACHINE_FLOOD_WATER_SPREADING,LIQUID,  100,    100,    0,  "", 0,  0,  0,      FLOOR_FLOODABLE,            DF_SHALLOW_WATER},
    {MACHINE_FLOOD_WATER_DORMANT,LIQUID,    250,    100,    (DFF_TREAT_AS_BLOCKING), "", 0, 0, 0, 0,            DF_SPREADABLE_DEEP_WATER_POOL},
    {DEEP_WATER,                LIQUID,     90,     100,    (DFF_CLEAR_OTHER_TERRAIN | DFF_PERMIT_BLOCKING)},

    // unstable floor machine:
    {MACHINE_COLLAPSE_EDGE_SPREADING,LIQUID,0,      0,      0,  "you hear a deep rumbling noise as the floor begins to collapse!"},
    {CHASM,                     LIQUID,     0,      0,      DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
    {MACHINE_COLLAPSE_EDGE_SPREADING,LIQUID,100,    100,    0,  "", 0,  0,  0,  FLOOR_FLOODABLE,    DF_COLLAPSE},
    {MACHINE_COLLAPSE_EDGE_DORMANT,LIQUID,  0,      0,      0},

    // levitation bridge machine:
    {CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,LIQUID,100,    100,    0,  "", 0, 0,  0,  CHASM_WITH_HIDDEN_BRIDGE,  DF_BRIDGE_APPEARS},
    {CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,LIQUID,100,    100,    0,  "a stone bridge extends from the floor with a grinding sound.", 0, 0,  0,  CHASM_WITH_HIDDEN_BRIDGE,  DF_BRIDGE_APPEARS},
    {STONE_BRIDGE,              LIQUID,     0,      0,      0},
    {MACHINE_CHASM_EDGE,        LIQUID,     100,    100,    0},

    // retracting lava pool:
    {LAVA_RETRACTABLE,          LIQUID,     100,    100,    0,  "", 0, 0,  0,  LAVA},
    {LAVA_RETRACTING,           LIQUID,     0,      0,      0,  "hissing fills the air as the lava begins to cool."},
    {OBSIDIAN,                  SURFACE,    0,      0,      0,  "", 0,  0,  0,      0,          DF_STEAM_ACCUMULATION},

    // hidden poison vent machine:
    {MACHINE_POISON_GAS_VENT_DORMANT,DUNGEON,0,     0,      0,  "you notice an inactive gas vent hidden in a crevice of the floor.", GENERIC_FLASH_LIGHT},
    {MACHINE_POISON_GAS_VENT,   DUNGEON,    0,      0,      0,  "deadly purple gas starts wafting out of hidden vents in the floor!"},
    {PORTCULLIS_CLOSED,         DUNGEON,    0,      0,      DFF_EVACUATE_CREATURES_FIRST,   "with a heavy mechanical sound, an iron portcullis falls from the ceiling!", GENERIC_FLASH_LIGHT},
    {PORTCULLIS_DORMANT,        DUNGEON,    0,      0,      0,  "the portcullis slowly rises from the ground into a slot in the ceiling.", GENERIC_FLASH_LIGHT},
    {POISON_GAS,                GAS,        25,     0,      0},

    // hidden methane vent machine:
    {MACHINE_METHANE_VENT_DORMANT,DUNGEON,0,        0,      0,  "you notice an inactive gas vent hidden in a crevice of the floor.", GENERIC_FLASH_LIGHT},
    {MACHINE_METHANE_VENT,      DUNGEON,    0,      0,      0,  "explosive methane gas starts wafting out of hidden vents in the floor!", 0, 0, 0, 0, DF_VENT_SPEW_METHANE},
    {METHANE_GAS,               GAS,        60,     0,      0},
    {PILOT_LIGHT,               DUNGEON,    0,      0,      0,  "a torch falls from its mount and lies sputtering on the floor.", FALLEN_TORCH_FLASH_LIGHT},

    // paralysis trap:
    {MACHINE_PARALYSIS_VENT,    DUNGEON,    0,      0,      0,  "you notice an inactive gas vent hidden in a crevice of the floor.", GENERIC_FLASH_LIGHT},
    {PARALYSIS_GAS,             GAS,        350,    0,      0,  "paralytic gas sprays upward from hidden vents in the floor!", 0, 0, 0, 0, DF_REVEAL_PARALYSIS_VENT_SILENTLY},
    {MACHINE_PARALYSIS_VENT,    DUNGEON,    0,      0,      0},

    // thematic dungeon:
    {RED_BLOOD,                 SURFACE,    75,     25,     0},

    // statuary:
    {STATUE_CRACKING,           DUNGEON,    0,      0,      0,  "cracks begin snaking across the marble surface of the statue!", 0, 0, 0, 0, DF_RUBBLE},
    {RUBBLE,                    SURFACE,    120,    100,    DFF_ACTIVATE_DORMANT_MONSTER,   "the statue shatters!", 0, &darkGray, 3, 0, DF_RUBBLE},

    // hidden turrets:
    {WALL,                      DUNGEON,    0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "you hear a click, and the stones in the wall shift to reveal turrets!", 0, 0, 0, 0, DF_RUBBLE},

    // worm tunnels:
    {WORM_TUNNEL_MARKER_DORMANT,LIQUID,     5,      5,      0,  "", 0,  0,  GRANITE},
    {WORM_TUNNEL_MARKER_ACTIVE, LIQUID,     0,      0,      0},
    {FLOOR,                     DUNGEON,    0,      0,      (DFF_SUPERPRIORITY | DFF_ACTIVATE_DORMANT_MONSTER),  "", 0, 0,  0,  0,  DF_TUNNELIZE},
    {FLOOR,                     DUNGEON,    0,      0,      0,  "the nearby wall cracks and collapses in a cloud of dust!", 0, &darkGray,  5,  0,  DF_TUNNELIZE},

    // haunted room:
    {DARK_FLOOR_DARKENING,      DUNGEON,    0,      0,      0,  "the light in the room flickers and you feel a chill in the air."},
    {DARK_FLOOR,                DUNGEON,    0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "", 0, 0, 0, 0, DF_ECTOPLASM_DROPLET},
    {HAUNTED_TORCH_TRANSITIONING,DUNGEON,   0,      0,      0},
    {HAUNTED_TORCH,             DUNGEON,    0,      0,      0},

    // mud pit:
    {MACHINE_MUD_DORMANT,       LIQUID,     100,    100,    0},
    {MUD,                       LIQUID,     0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "across the bog, bubbles rise ominously from the mud."},

    // electric crystals:
    {ELECTRIC_CRYSTAL_ON,       DUNGEON,    0,      0,      0, "the crystal absorbs the electricity and begins to glow.", CHARGE_FLASH_LIGHT},
    {WALL,                      DUNGEON,    0,      0,      DFF_ACTIVATE_DORMANT_MONSTER,   "the wall above the lever shifts to reveal a spark turret!"},

    // idyll:
    {SHALLOW_WATER,             LIQUID,     150,    100,    (DFF_PERMIT_BLOCKING)},
    {DEEP_WATER,                LIQUID,     90,     100,    (DFF_TREAT_AS_BLOCKING | DFF_CLEAR_OTHER_TERRAIN | DFF_SUBSEQ_EVERYWHERE), "", 0, 0, 0, 0, DF_SHALLOW_WATER_POOL},

    // swamp:
    {SHALLOW_WATER,             LIQUID,     30,     100,    0},
    {GRAY_FUNGUS,               SURFACE,    80,     50,     0,  "", 0, 0, 0, 0, DF_SWAMP_MUD},
    {MUD,                       LIQUID,     75,     5,      0,  "", 0, 0, 0, 0, DF_SWAMP_WATER},

    // camp:
    {HAY,                       SURFACE,    90,     87,     0},
    {JUNK,                      SURFACE,    20,     20,     0},

    // remnants:
    {CARPET,                    DUNGEON,    110,    20,     DFF_SUBSEQ_EVERYWHERE,  "", 0, 0, 0, 0, DF_REMNANT_ASH},
    {BURNED_CARPET,             SURFACE,    120,    100,    0},

    // chasm catwalk:
    {CHASM,                     LIQUID,     0,      0,      DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, 0, DF_SHOW_TRAPDOOR_HALO},
    {STONE_BRIDGE,              LIQUID,     0,      0,      DFF_CLEAR_OTHER_TERRAIN},

    // lake catwalk:
    {DEEP_WATER,                LIQUID,     0,      0,      DFF_CLEAR_OTHER_TERRAIN, "", 0, 0, 0, 0, DF_LAKE_HALO},
    {SHALLOW_WATER,             LIQUID,     160,    100,    0},

    // worms pop out of walls:
    {RUBBLE,                    SURFACE,    120,    100,    DFF_ACTIVATE_DORMANT_MONSTER,   "the nearby wall explodes in a shower of stone fragments!", 0, &darkGray, 3, 0, DF_RUBBLE},

    // monster cages open:
    {MONSTER_CAGE_OPEN,         DUNGEON,    0,      0,      0},

    // goblin warren:
    {STENCH_SMOKE_GAS,          GAS,        50,     0,      0, "", 0, 0, 0, 0, DF_PLAIN_FIRE},
    {STENCH_SMOKE_GAS,          GAS,        50,     0,      0, "", 0, 0, 0, 0, DF_EMBERS},
};

const dungeonProfile dungeonProfileCatalog[NUMBER_DUNGEON_PROFILES] = {
    // Room frequencies:
    //      0. Cross room
    //      1. Small symmetrical cross room
    //      2. Small room
    //      3. Circular room
    //      4. Chunky room
    //      5. Cave
    //      6. Cavern (the kind that fills a level)
    //      7. Entrance room (the big upside-down T room at the start of depth 1)

    // Room frequencies
    // 0    1   2   3   4   5   6   7   Corridor chance
    {{2,    1,  1,  1,  7,  1,  0,  0}, 10},    // Basic dungeon generation (further adjusted by depth)
    {{10,   0,  0,  3,  7,  10, 10, 0}, 0},     // First room for basic dungeon generation (further adjusted by depth)

    {{0,    0,  1,  0,  0,  0,  0,  0}, 0},     // Goblin warrens
    {{0,    5,  0,  1,  0,  0,  0,  0}, 0},     // Sentinel sanctuaries
};

// radius is in units of 0.01
const lightSource lightCatalog[NUMBER_LIGHT_KINDS] = {
    //color                 radius range            fade%   passThroughCreatures
    {0},                                                                // NO_LIGHT
    {&minersLightColor,     {0, 0, 1},              35,     true},      // miners light
    {&fireBoltColor,        {300, 400, 1},          0,      false},     // burning creature light
    {&wispLightColor,       {400, 800, 1},          0,      false},     // will-o'-the-wisp light
    {&fireBoltColor,        {300, 400, 1},          0,      false},     // salamander glow
    {&pink,                 {600, 600, 1},          0,      true},      // imp light
    {&pixieColor,           {400, 600, 1},          50,     false},     // pixie light
    {&lichLightColor,       {1500, 1500, 1},        0,      false},     // lich light
    {&flamedancerCoronaColor,{1000, 2000, 1},       0,      false},     // flamedancer light
    {&sentinelLightColor,   {300, 500, 1},          0,      false},     // sentinel light
    {&unicornLightColor,    {300, 400, 1},          0,      false},     // unicorn light
    {&ifritLightColor,      {300, 600, 1},          0,      false},     // ifrit light
    {&fireBoltColor,        {400, 600, 1},          0,      false},     // phoenix light
    {&fireBoltColor,        {150, 300, 1},          0,      false},     // phoenix egg light
    {&yendorLightColor,     {1500, 1500, 1},        0,      false},     // Yendorian light
    {&spectralBladeLightColor,{350, 350, 1},        0,      false},     // spectral blades
    {&summonedImageLightColor,{350, 350, 1},        0,      false},     // weapon images
    {&lightningColor,       {250, 250, 1},          35,     false},     // lightning turret light
    {&explosiveAuraColor,   {150, 200, 1},          0,      true},      // explosive bloat light
    {&lightningColor,       {300, 300, 1},          0,      false},     // bolt glow
    {&telepathyColor,       {200, 200, 1},          0,      true},      // telepathy light
    {&sacrificeTargetColor, {250, 250, 1},          0,      true},      // sacrifice doom light

    // flares:
    {&scrollProtectionColor,{600, 600, 1},          0,      true},      // scroll of protection flare
    {&scrollEnchantmentColor,{600, 600, 1},         0,      true},      // scroll of enchantment flare
    {&potionStrengthColor,  {600, 600, 1},          0,      true},      // potion of strength flare
    {&empowermentFlashColor,{600, 600, 1},          0,      true},      // empowerment flare
    {&genericFlashColor,    {300, 300, 1},          0,      true},      // generic flash flare
    {&fireFlashColor,       {800, 800, 1},          0,      false},     // fallen torch flare
    {&summoningFlashColor,  {600, 600, 1},          0,      true},      // summoning flare
    {&explosionFlareColor,  {5000, 5000, 1},        0,      true},      // explosion (explosive bloat or incineration potion)
    {&quietusFlashColor,    {300, 300, 1},          0,      true},      // quietus activation flare
    {&slayingFlashColor,    {300, 300, 1},          0,      true},      // slaying activation flare
    {&lightningColor,       {800, 800, 1},          0,      false},     // electric crystal activates

    // glowing terrain:
    {&torchLightColor,      {1000, 1000, 1},        50,     false},     // torch
    {&lavaLightColor,       {300, 300, 1},          50,     false},     // lava
    {&sunLightColor,        {200, 200, 1},          25,     true},      // sunlight
    {&darknessPatchColor,   {400, 400, 1},          0,      true},      // darkness patch
    {&fungusLightColor,     {300, 300, 1},          50,     false},     // luminescent fungus
    {&fungusForestLightColor,{500, 500, 1},         0,      false},     // luminescent forest
    {&algaeBlueLightColor,  {300, 300, 1},          0,      false},     // luminescent algae blue
    {&algaeGreenLightColor, {300, 300, 1},          0,      false},     // luminescent algae green
    {&ectoplasmColor,       {200, 200, 1},          50,     false},     // ectoplasm
    {&unicornLightColor,    {200, 200, 1},          0,      false},     // unicorn poop light
    {&lavaLightColor,       {200, 200, 1},          50,     false},     // embers
    {&lavaLightColor,       {500, 1000, 1},         0,      false},     // fire
    {&lavaLightColor,       {200, 300, 1},          0,      false},     // brimstone fire
    {&explosionColor,       {DCOLS*100,DCOLS*100,1},100,    false},     // explosions
    {&dartFlashColor,       {15*100,15*100,1},      0,      false},     // incendiary darts
    {&portalActivateLightColor, {DCOLS*100,DCOLS*100,1},0,  false},     // portal activation
    {&confusionLightColor,  {300, 300, 1},          100,    false},     // confusion gas
    {&darknessCloudColor,   {500, 500, 1},          0,      true},      // darkness cloud
    {&forceFieldLightColor, {200, 200, 1},          50,     false},     // forcefield
    {&crystalWallLightColor,{300, 500, 1},          50,     false},     // crystal wall
    {&torchLightColor,      {200, 400, 1},          0,      false},     // candle light
    {&hauntedTorchColor,    {400, 600, 1},          0,      false},     // haunted torch
    {&glyphLightColor,      {100, 100, 1},          0,      false},     // glyph dim light
    {&glyphLightColor,      {300, 300, 1},          0,      false},     // glyph bright light
    {&sacredGlyphColor,     {300, 300, 1},          0,      false},     // sacred glyph light
    {&descentLightColor,    {600, 600, 1},          0,      false},     // magical pit light
    {&sacrificeTargetColor, {800, 1200, 1},          0,      true},      // demonic statue light
};

// Defines all creatures, which include monsters and the player:
// This cannot be const, since we set monsterIDs
creatureType monsterCatalog[NUMBER_MONSTER_KINDS] = {
    //  name            ch      color           HP      def     acc     damage          reg move    attack  blood           light isLarge     DFChance DFType         bolts       behaviorF, abilityF
    {0, "you",  G_PLAYER,       &playerInLightColor,30, 0,      100,    {1, 2, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MALE | MONST_FEMALE)},

    {0, "rat",          G_RAT,    &gray,          6,      0,      80,     {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,       {0}},
    {0, "kobold",       G_KOBOLD,    &goblinColor,   7,      0,      80,     {1, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0}},
    {0, "jackal",       G_JACKAL,    &jackalColor,   8,      0,      70,     {2, 4, 1},      20, 50,     100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,              {0}},
    {0, "eel",          G_EEL,    &eelColor,      18,     27,     100,    {3, 7, 2},      5,  50,     100,    0,              0,    false,      0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS)},
    {0, "monkey",       G_MONKEY,    &ogreColor,     12,     17,     100,    {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      1,      DF_URINE,       {0},
        (0), (MA_HIT_STEAL_FLEE)},
    {0, "bloat",        G_BLOAT,    &poisonGasColor,4,      0,      100,    {0, 0, 0},      5,  100,    100,    DF_PURPLE_BLOOD,0,    false,      0,      DF_BLOAT_DEATH, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "pit bloat",    G_BLOAT,    &lightBlue,     4,      0,      100,    {0, 0, 0},      5,  100,    100,    DF_PURPLE_BLOOD,0,    false,      0,      DF_HOLE_POTION, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "goblin",       G_GOBLIN,    &goblinColor,   15,     10,     70,     {2, 5, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (0),  (MA_ATTACKS_PENETRATE | MA_AVOID_CORRIDORS)},
    {0, "goblin conjurer",G_GOBLIN_MAGIC,  &goblinConjurerColor, 10,10,    70,     {2, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON | MA_AVOID_CORRIDORS)},
    {0, "goblin mystic",G_GOBLIN_MAGIC,    &goblinMysticColor, 10, 10,     70,     {2, 4, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_SHIELDING},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_AVOID_CORRIDORS)},
    {0, "goblin totem", G_TOTEM, &orange,    30,     0,      0,      {0, 0, 0},      0,  100,    300,    DF_RUBBLE_BLOOD,IMP_LIGHT,    false,0,    0,              {BOLT_HASTE, BOLT_SPARK},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS), (0)},
    {0, "pink jelly",   G_JELLY,    &pinkJellyColor,50,     0,      85,     {1, 3, 1},      0,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0},
        (MONST_NEVER_SLEEPS), (MA_CLONE_SELF_ON_DEFEND)},
    {0, "toad",         G_TOAD,    &toadColor,     18,     0,      90,     {1, 4, 1},      10, 100,    100,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {0},
        (0), (MA_HIT_HALLUCINATE)},
    {0, "vampire bat",  G_BAT,    &gray,          18,     25,     100,    {2, 6, 1},      20, 50,     100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_FLIES | MONST_FLITS), (MA_TRANSFERENCE)},
    {0, "arrow turret", G_TURRET,&black,     30,     0,      90,     {2, 6, 1},      0,  100,    250,    0,              0,    false,      0,      0,              {BOLT_DISTANCE_ATTACK},
        (MONST_TURRET), (0)},
    {0, "acid mound",   G_MOUND,    &acidBackColor, 15,     10,     70,     {1, 3, 1},      5,  100,    100,    DF_ACID_BLOOD,  0,    false,      0,      0,              {0},
        (MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR)},
    {0, "centipede",    G_CENTIPEDE,    &centipedeColor,20,     20,     80,     {4, 12, 1},     20, 100,    100,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {0},
        (0), (MA_CAUSES_WEAKNESS)},
    {0, "ogre",         G_OGRE,    &ogreColor,     55,     60,     125,    {9, 13, 2},     20, 100,    200,    DF_RED_BLOOD,   0,    true,       0,      0,              {0},
        (MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS | MA_ATTACKS_STAGGER)},
    {0, "bog monster",  G_BOG_MONSTER,    &krakenColor,   55,     60,     5000,   {3, 4, 1},      3,  200,    100,    0,              0,    true,       0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_SUBMERGES | MONST_FLITS | MONST_FLEES_NEAR_DEATH), (MA_SEIZES)},
    {0, "ogre totem",   G_TOTEM, &green,     70,     0,      0,      {0, 0, 0},      0,  100,    400,    DF_RUBBLE_BLOOD,LICH_LIGHT,    false,0,   0,              {BOLT_HEALING, BOLT_SLOW_2},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS), (0)},
    {0, "spider",       G_SPIDER,    &white,         20,     70,     90,     {3, 4, 2},      20, 100,    200,    DF_GREEN_BLOOD, 0,    false,      0,      0,              {BOLT_SPIDERWEB},
        (MONST_IMMUNE_TO_WEBS | MONST_CAST_SPELLS_SLOWLY | MONST_ALWAYS_USE_ABILITY), (MA_POISONS)},
    {0, "spark turret", G_TURRET, &lightningColor,80,0,      100,    {0, 0, 0},      0,  100,    150,    0,              SPARK_TURRET_LIGHT,    false, 0,  0,      {BOLT_SPARK},
        (MONST_TURRET), (0)},
    {0, "wisp",         G_WISP,    &wispLightColor,10,     90,     100,    {0, 0, 0},      5,  100,    100,    DF_ASH_BLOOD,   WISP_LIGHT,    false, 0,  0,              {0},
        (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_DIES_IF_NEGATED), (MA_HIT_BURN)},
    {0, "wraith",       G_WRAITH,    &wraithColor,   50,     60,     120,    {6, 13, 2},     5,  50,     100,    DF_GREEN_BLOOD, 0,    true,       0,      0,              {0},
        (MONST_FLEES_NEAR_DEATH)},
    {0, "zombie",       G_ZOMBIE,    &vomitColor,    80,     0,      120,    {7, 12, 1},     0,  100,    100,    DF_ROT_GAS_BLOOD,0,    true,      100,    DF_ROT_GAS_PUFF, {0}},
    {0, "troll",        G_TROLL,    &trollColor,    65,     70,     125,    {10, 15, 3},    1,  100,    100,    DF_RED_BLOOD,   0,    true,       0,      0,              {0},
        (MONST_MALE | MONST_FEMALE)},
    {0, "ogre shaman",  G_OGRE_MAGIC,    &green,         45,     40,     100,    {5, 9, 1},      20, 100,    200,    DF_RED_BLOOD,   0,    true,       0,      0,              {BOLT_HASTE, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_CAST_SPELLS_SLOWLY | MONST_MALE | MONST_FEMALE), (MA_CAST_SUMMON | MA_AVOID_CORRIDORS)},
    {0, "naga",         G_NAGA,    &trollColor,    75,     70,     150,    {7, 11, 4},     10, 100,    100,    DF_GREEN_BLOOD, 0,    true,       100,    DF_PUDDLE,      {0},
        (MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FEMALE), (MA_ATTACKS_ALL_ADJACENT)},
    {0, "salamander",   G_SALAMANDER,    &salamanderColor,60,    70,     150,    {5, 11, 3},     10, 100,    100,    DF_ASH_BLOOD,   SALAMANDER_LIGHT,    true,  100, DF_SALAMANDER_FLAME, {0},
        (MONST_IMMUNE_TO_FIRE | MONST_SUBMERGES | MONST_NEVER_SLEEPS | MONST_FIERY | MONST_MALE), (MA_ATTACKS_EXTEND)},
    {0, "explosive bloat",G_BLOAT,  &orange,        10,     0,      100,    {0, 0, 0},      5,  100,    100,    DF_RED_BLOOD,   EXPLOSIVE_BLOAT_LIGHT,    false,0, DF_BLOAT_EXPLOSION, {0},
        (MONST_FLIES | MONST_FLITS), (MA_KAMIKAZE | MA_DF_ON_DEATH)},
    {0, "dar blademaster",G_DAR_BLADEMASTER,  &purple,        35,     70,     160,    {5, 9, 2},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_BLINKING},
        (MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "dar priestess", G_DAR_PRIESTESS,   &darPriestessColor,20,  60,     100,    {2, 5, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_NEGATION, BOLT_HEALING, BOLT_HASTE, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "dar battlemage",G_DAR_BATTLEMAGE,   &darMageColor,  20,     60,     100,    {1, 3, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {BOLT_FIRE, BOLT_SLOW_2, BOLT_DISCORD},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_MALE | MONST_FEMALE), (MA_AVOID_CORRIDORS)},
    {0, "acidic jelly", G_JELLY,    &acidBackColor, 60,     0,      115,    {2, 6, 1},      0,  100,    100,    DF_ACID_BLOOD,  0,    true,       0,      0,              {0},
        (MONST_DEFEND_DEGRADE_WEAPON), (MA_HIT_DEGRADE_ARMOR | MA_CLONE_SELF_ON_DEFEND)},
    {0, "centaur",      G_CENTAUR,    &tanColor,      35,     50,     175,    {4, 8, 2},      20, 50,     100,    DF_RED_BLOOD,   0,    true,       0,      0,              {BOLT_DISTANCE_ATTACK},
        (MONST_MAINTAINS_DISTANCE | MONST_MALE), (0)},
    {0, "underworm",    G_UNDERWORM,    &wormColor,     80,     40,     160,    {18, 22, 2},    3,  150,    200,    DF_WORM_BLOOD,  0,    true,       0,      0,              {0},
        (MONST_NEVER_SLEEPS)},
    {0, "sentinel",     G_GUARDIAN, &sentinelColor, 50,0,      0,      {0, 0, 0},      0,  100,    175,    DF_RUBBLE_BLOOD,SENTINEL_LIGHT,    false,0,0,             {BOLT_HEALING, BOLT_SPARK},
        (MONST_TURRET | MONST_CAST_SPELLS_SLOWLY | MONST_DIES_IF_NEGATED), (0)},
    {0, "dart turret", G_TURRET, &centipedeColor,20, 0,      140,    {1, 2, 1},      0,  100,    250,    0,              0,    false,      0,      0,              {BOLT_POISON_DART},
        (MONST_TURRET), (MA_CAUSES_WEAKNESS)},
    {0, "kraken",       G_KRAKEN,    &krakenColor,   120,    0,      150,    {15, 20, 3},    1,  50,     100,    0,              0,    true,       0,      0,              {0},
        (MONST_RESTRICTED_TO_LIQUID | MONST_IMMUNE_TO_WATER | MONST_SUBMERGES | MONST_FLITS | MONST_NEVER_SLEEPS | MONST_FLEES_NEAR_DEATH), (MA_SEIZES)},
    {0, "lich",         G_LICH,    &white,         35,     80,     175,    {2, 6, 1},      0,  100,    100,    DF_ASH_BLOOD,   LICH_LIGHT,    true,  0,  0,              {BOLT_FIRE},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25 | MONST_NO_POLYMORPH), (MA_CAST_SUMMON)},
    {0, "phylactery",   G_EGG,&lichLightColor,30,    0,      0,      {0, 0, 0},      0,  100,    150,    DF_RUBBLE_BLOOD,LICH_LIGHT,    false, 0,  0,              {0},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED), (MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "pixie",        G_PIXIE,    &pixieColor,    10,     90,     100,    {1, 3, 1},      20, 50,     100,    DF_GREEN_BLOOD, PIXIE_LIGHT,    false, 0, 0,              {BOLT_NEGATION, BOLT_SLOW_2, BOLT_DISCORD, BOLT_SPARK},
        (MONST_MAINTAINS_DISTANCE | MONST_FLIES | MONST_FLITS | MONST_MALE | MONST_FEMALE), (0)},
    {0, "phantom",      G_PHANTOM,    &ectoplasmColor,35,     70,     160,    {12, 18, 4},    0,  50,     200,    DF_ECTOPLASM_BLOOD, 0,    true,   2,      DF_ECTOPLASM_DROPLET, {0},
        (MONST_INVISIBLE | MONST_FLITS | MONST_FLIES | MONST_IMMUNE_TO_WEBS)},
    {0, "flame turret", G_TURRET, &lavaForeColor,40, 0,      150,    {1, 2, 1},      0,  100,    250,    0,              LAVA_LIGHT,    false, 0,  0,              {BOLT_FIRE},
        (MONST_TURRET), (0)},
    {0, "imp",          G_IMP,    &pink,          35,     90,     225,    {4, 9, 2},      10, 100,    100,    DF_GREEN_BLOOD, IMP_LIGHT,    false,  0,  0,              {BOLT_BLINKING},
        (0), (MA_HIT_STEAL_FLEE)},
    {0, "fury",         G_FURY,    &darkRed,       19,     90,     200,    {6, 11, 4},     20, 50,     100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_NEVER_SLEEPS | MONST_FLIES)},
    {0, "revenant",     G_REVENANT,    &ectoplasmColor,30,     0,      200,    {15, 20, 5},    0,  100,    100,    DF_ECTOPLASM_BLOOD, 0,    true,   0,      0,              {0},
        (MONST_IMMUNE_TO_WEAPONS)},
    {0, "tentacle horror",G_TENTACLE_HORROR,  &centipedeColor,120,    95,     225,    {25, 35, 3},    1,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0}},
    {0, "golem",        G_GOLEM,    &gray,          400,    70,     225,    {4, 8, 1},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    true,       0,      0,              {0},
        (MONST_REFLECT_50 | MONST_DIES_IF_NEGATED)},
    {0, "dragon",       G_DRAGON,    &dragonColor,   150,    90,     250,    {25, 50, 4},    20, 50,     200,    DF_GREEN_BLOOD, 0,    true,       0,      0,              {BOLT_DRAGONFIRE},
        (MONST_IMMUNE_TO_FIRE | MONST_CARRY_ITEM_100), (MA_ATTACKS_ALL_ADJACENT)},

    // bosses
    {0, "goblin warlord",G_GOBLIN_CHIEFTAN,   &blue,          30,     17,     100,    {3, 6, 1},      20, 100,    100,    DF_RED_BLOOD,   0,    false,      0,      0,              {0},
        (MONST_MAINTAINS_DISTANCE | MONST_CARRY_ITEM_25), (MA_CAST_SUMMON | MA_ATTACKS_PENETRATE | MA_AVOID_CORRIDORS)},
    {0, "black jelly",  G_JELLY,    &black,         120,    0,      130,    {3, 8, 1},      0,  100,    100,    DF_PURPLE_BLOOD,0,    true,       0,      0,              {0},
        (0), (MA_CLONE_SELF_ON_DEFEND)},
    {0, "vampire",      G_VAMPIRE,    &white,         75,     60,     120,    {4, 15, 2},     6,  50,     100,    DF_RED_BLOOD,   0,    true,       0,      DF_BLOOD_EXPLOSION, {BOLT_BLINKING, BOLT_DISCORD},
        (MONST_FLEES_NEAR_DEATH | MONST_MALE), (MA_TRANSFERENCE | MA_DF_ON_DEATH | MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "flamedancer",  G_FLAMEDANCER,    &white,         65,     80,     120,    {3, 8, 2},      0,  100,    100,    DF_EMBER_BLOOD, FLAMEDANCER_LIGHT,    true, 100,DF_FLAMEDANCER_CORONA, {BOLT_FIRE},
        (MONST_MAINTAINS_DISTANCE | MONST_IMMUNE_TO_FIRE | MONST_FIERY), (MA_HIT_BURN)},

    // special effect monsters
    {0, "spectral blade",G_WEAPON, &spectralBladeColor,1, 0, 150,    {1, 1, 1},      0,  50,     100,    0,              SPECTRAL_BLADE_LIGHT,    false,0,0,       {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS | MONST_NOT_LISTED_IN_SIDEBAR)},
    {0, "spectral sword",G_WEAPON, &spectralImageColor, 1,0, 150,    {1, 1, 1},      0,  50,     100,    0,              SPECTRAL_IMAGE_LIGHT,    false,0,0,       {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_FLIES | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_IMMUNE_TO_WEBS)},
    {0, "stone guardian",G_GUARDIAN, &white,   1000,   0,      200,    {12, 17, 2},    0,  100,    100,    DF_RUBBLE,      0,    false,      100,      DF_GUARDIAN_STEP, {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_ALWAYS_USE_ABILITY | MONST_GETS_TURN_ON_ACTIVATION), (MA_REFLECT_100)},
    {0, "winged guardian",G_WINGED_GUARDIAN, &blue,   1000,   0,      200,    {12, 17, 2},    0,  100,    100,    DF_RUBBLE,      0,    false,      100,      DF_SILENT_GLYPH_GLOW, {BOLT_BLINKING},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_WILL_NOT_USE_STAIRS | MONST_DIES_IF_NEGATED | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (MA_REFLECT_100)},
    {0, "guardian spirit",G_GUARDIAN, &spectralImageColor,1000,0,200,  {5, 12, 2},     0,  100,    100,    0,              SPECTRAL_IMAGE_LIGHT,    false,100,0,     {0},
        (MONST_INANIMATE | MONST_NEVER_SLEEPS | MONST_IMMUNE_TO_FIRE | MONST_IMMUNE_TO_WEAPONS | MONST_DIES_IF_NEGATED | MONST_ALWAYS_USE_ABILITY), (MA_REFLECT_100)},
    {0, "Warden of Yendor",G_WARDEN, &yendorLightColor,1000,   0,    300,    {12, 17, 2},    0,  200,    200,    DF_RUBBLE,      YENDOR_LIGHT,    true,  100, 0,           {0},
        (MONST_NEVER_SLEEPS | MONST_ALWAYS_HUNTING | MONST_INVULNERABLE | MONST_NO_POLYMORPH)},
    {0, "eldritch totem",G_TOTEM, &glyphColor,80,    0,      0,      {0, 0, 0},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    false,      0,      0,              {0},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY), (MA_CAST_SUMMON)},
    {0, "mirrored totem",G_TOTEM, &beckonColor,80,   0,      0,      {0, 0, 0},      0,  100,    100,    DF_RUBBLE_BLOOD,0,    false,      100,    DF_MIRROR_TOTEM_STEP, {BOLT_BECKONING},
        (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_ALWAYS_HUNTING | MONST_WILL_NOT_USE_STAIRS | MONST_GETS_TURN_ON_ACTIVATION | MONST_ALWAYS_USE_ABILITY | MONST_IMMUNE_TO_WEAPONS | MONST_IMMUNE_TO_FIRE), (MA_REFLECT_100)},

    // legendary allies
    {0, "unicorn",      G_UNICORN, &white,   40,     60,     175,    {2, 10, 2},     20, 50,     100,    DF_RED_BLOOD,   UNICORN_LIGHT,    true, 1,DF_UNICORN_POOP, {BOLT_HEALING, BOLT_SHIELDING},
        (MONST_MAINTAINS_DISTANCE | MONST_MALE | MONST_FEMALE), (0)},
    {0, "ifrit",        G_IFRIT,    &ifritColor,    40,     75,     175,    {5, 13, 2},     1,  50,     100,    DF_ASH_BLOOD,   IFRIT_LIGHT,    true, 0,  0,              {BOLT_DISCORD},
        (MONST_IMMUNE_TO_FIRE | MONST_FLIES | MONST_MALE), (0)},
    {0, "phoenix",      G_PHOENIX,    &phoenixColor,  30,     70,     175,    {4, 10, 2},     0,  50,     100,    DF_ASH_BLOOD,   PHOENIX_LIGHT,    true, 0,0,              {0},
        (MONST_IMMUNE_TO_FIRE| MONST_FLIES | MONST_NO_POLYMORPH)},
    {0, "phoenix egg",  G_EGG,&phoenixColor, 50,     0,      0,      {0, 0, 0},      0,  100,    150,    DF_ASH_BLOOD,   PHOENIX_EGG_LIGHT,    false,  0,  0,      {0},
        (MONST_IMMUNE_TO_FIRE| MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE | MONST_WILL_NOT_USE_STAIRS | MONST_NO_POLYMORPH | MONST_ALWAYS_HUNTING | MONST_IMMUNE_TO_WEAPONS), (MA_CAST_SUMMON | MA_ENTER_SUMMONS)},
    {0, "mangrove dryad",G_ANCIENT_SPIRIT,   &tanColor,      70,     60,     175,    {2, 8, 2},      6,  100,    100,    DF_ASH_BLOOD,   0,    true,       0,      0,              {BOLT_ANCIENT_SPIRIT_VINES},
        (MONST_IMMUNE_TO_WEBS | MONST_ALWAYS_USE_ABILITY | MONST_MAINTAINS_DISTANCE | MONST_NO_POLYMORPH | MONST_MALE | MONST_FEMALE), (0)},
};

const monsterWords monsterText[NUMBER_MONSTER_KINDS] = {
    {"A naked adventurer in an unforgiving place, bereft of equipment and confused about the circumstances.",
        "studying", "Studying",
        {"hit", {0}}},
    {"The rat is a scavenger of the shallows, perpetually in search of decaying animal matter.",
        "gnawing at", "Eating",
        {"scratches", "bites", {0}}},
    {"The kobold is a lizardlike humanoid of the upper dungeon.",
        "poking at", "Examining",
        {"clubs", "bashes", {0}}},
    {"The jackal prowls the caverns for intruders to rend with $HISHER powerful jaws.",
        "tearing at", "Eating",
        {"claws", "bites", "mauls", {0}}},
    {"The eel slips silently through the subterranean lake, waiting for unsuspecting prey to set foot in $HISHER dark waters.",
        "eating", "Eating",
        {"shocks", "bites", {0}}},
    {"Mischievous trickster that $HESHE is, the monkey lives to steal shiny trinkets from passing adventurers.",
        "examining", "Examining",
        {"tweaks", "bites", "punches", {0}}},
    {"A bladder of deadly gas buoys the bloat through the air, $HISHER thin veinous membrane ready to rupture at the slightest stress.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "bursts, leaving behind an expanding cloud of caustic gas!"},
    {"This rare subspecies of bloat is filled with a peculiar vapor that, if released, will cause the floor to vanish out from underneath $HIMHER.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "bursts, causing the floor underneath $HIMHER to disappear!"},
    {"A filthy little primate, the tribalistic goblin often travels in packs and carries a makeshift stone spear.",
        "chanting over", "Chanting",
        {"cuts", "stabs", "skewers", {0}}},
    {"This goblin is covered with glowing sigils that pulse with power. $HESHE can call into existence phantom blades to attack $HISHER foes.",
        "performing a ritual on", "Performing ritual",
        {"thumps", "whacks", "wallops", {0}},
        {0},
        "gestures ominously!"},
    {"This goblin carries no weapon, and $HISHER eyes sparkle with golden light. $HESHE can invoke a powerful shielding magic to protect $HISHER escorts from harm.",
        "performing a ritual on", "Performing ritual",
        {"slaps", "punches", "kicks", {0}}},
    {"Goblins have created this makeshift totem and imbued $HIMHER with a shamanistic power.",
        "gazing at", "Gazing",
        {"hits", {0}}},
    {"This mass of caustic pink goo slips across the ground in search of a warm meal.",
        "absorbing", "Feeding",
        {"smears", "slimes", "drenches"}},
    {"The enormous, warty toad secretes a powerful hallucinogenic slime to befuddle the senses of any creatures that come in contact with $HIMHER.",
        "eating", "Eating",
        {"slimes", "slams", {0}}},
    {"Often hunting in packs, leathery wings and keen senses guide the vampire bat unerringly to $HISHER prey.",
        "draining", "Feeding",
        {"nips", "bites", {0}}},
    {"A mechanical contraption embedded in the wall, the spring-loaded arrow turret will fire volley after volley of arrows at intruders.",
        "gazing at", "Gazing",
        {"shoots", {0}}},
    {"The acid mound squelches softly across the ground, leaving a trail of hissing goo in $HISHER path.",
        "liquefying", "Feeding",
        {"slimes", "douses", "drenches", {0}}},
    {"This monstrous centipede's incisors are imbued with a horrible venom that will slowly kill $HISHER prey.",
        "eating", "Eating",
        {"pricks", "stings", {0}}},
    {"This lumbering creature carries an enormous club that $HESHE can swing with incredible force.",
        "examining", "Studying",
        {"cudgels", "clubs", "batters", {0}}},
    {"The horrifying bog monster dwells beneath the surface of mud-filled swamps. When $HISHER prey ventures into the mud, the bog monster will ensnare the unsuspecting victim in $HISHER pale tentacles and squeeze its life away.",
        "draining", "Feeding",
        {"squeezes", "strangles", "crushes", {0}}},
    {"Ancient ogres versed in the eldritch arts have assembled this totem and imbued $HIMHER with occult power.",
        "gazing at", "Gazing",
        {"hits", {0}}},
    {"The spider's red eyes pierce the darkness in search of enemies to ensnare with $HISHER projectile webs and dissolve with deadly poison.",
        "draining", "Feeding",
        {"bites", "stings", {0}}},
    {"This contraption hums with electrical charge that $HISHER embedded crystals and magical sigils can direct at intruders in deadly arcs.",
        "gazing at", "Gazing",
        {"shocks", {0}}},
    {"An ethereal blue flame dances through the air, flickering and pulsing in time to an otherworldly rhythm.",
        "consuming", "Feeding",
        {"scorches", "burns", {0}}},
    {"The wraith's hollow eye sockets stare hungrily at the world from $HISHER emaciated frame, and $HISHER long, bloodstained nails grope ceaselessly at the air for a fresh victim.",
        "devouring", "Feeding",
        {"clutches", "claws", "bites", {0}}},
    {"The zombie is the accursed product of a long-forgotten ritual. Perpetually decaying flesh hangs from $HISHER bones in shreds and releases a flammable stench that will induce violent nausea with one whiff.",
        "rending", "Eating",
        {"hits", "bites", {0}}},
    {"An enormous, disfigured creature covered in phlegm and warts, the troll regenerates very quickly and attacks with astonishing strength. Many adventures have ended at $HISHER misshapen hands.",
        "eating", "Eating",
        {"cudgels", "clubs", "bludgeons", "pummels", "batters"}},
    {"This ogre is bent with age, but what $HESHE has lost in physical strength, $HESHE has more than gained in occult power.",
        "performing a ritual on", "Performing ritual",
        {"cudgels", "clubs", {0}},
        {0},
        "chants in a harsh, guttural tongue!"},
    {"The serpentine naga live beneath the subterranean waters and emerge to attack unsuspecting adventurers.",
        "studying", "Studying",
        {"claws", "bites", "tail-whips", {0}}},
    {"A serpent wreathed in flames and carrying a burning lash, salamanders dwell in lakes of fire and emerge when they sense a nearby victim, leaving behind a trail of glowing embers.",
        "studying", "Studying",
        {"whips", "lashes", {0}}},
    {"This rare subspecies of bloat is little more than a thin membrane surrounding a bladder of highly explosive gases. The slightest stress will cause $HIMHER to rupture in spectacular and deadly fashion.",
        "gazing at", "Gazing",
        {"bumps", {0}},
        "detonates with terrifying force!"},
    {"An elf of the deep, the dar blademaster leaps toward $HISHER enemies with frightening speed to engage in deadly swordplay.",
        "studying", "Studying",
        {"grazes", "cuts", "slices", "slashes", "stabs"}},
    {"The dar priestess carries a host of religious relics that jangle as $HESHE walks.",
        "praying over", "Praying",
        {"cuts", "slices", {0}}},
    {"The dar battlemage's eyes glow like embers and $HISHER hands radiate an occult heat.",
        "transmuting", "Transmuting",
        {"cuts", {0}}},
    {"A jelly subsisting on a diet of acid mounds will eventually express the characteristics of $HISHER prey, corroding any unprotected weapons or armor that come in contact with $HIMHER.",
        "transmuting", "Transmuting",
        {"burns", {0}}},
    {"Half man and half horse, the centaur is an expert with the bow and arrow -- hunter and steed fused into a single creature.",
        "studying", "Studying",
        {"shoots", {0}}},
    {"A strange and horrifying creature of the earth's deepest places, larger than an ogre but capable of squeezing through tiny openings. When hungry, the underworm will burrow behind the walls of a cavern and lurk dormant and motionless -- often for months -- until $HESHE can feel the telltale vibrations of nearby prey.",
        "consuming", "Consuming",
        {"slams", "bites", "tail-whips", {0}}},
    {"An ancient statue of an unrecognizable humanoid figure, the sentinel holds aloft a crystal that gleams with ancient warding magic. Sentinels are always found in groups, and each will attempt to repair any damage done to the others.",
        "focusing on", "Focusing",
        {"hits", {0}}},
    {"This spring-loaded contraption fires darts that are imbued with a strength-sapping poison.",
        "gazing at", "Gazing",
        {"pricks", {0}}},
    {"This tentacled nightmare will emerge from the subterranean waters to ensnare and devour any creature foolish enough to set foot into $HISHER lake.",
        "devouring", "Feeding",
        {"slaps", "smites", "batters", {0}}},
    {"The desiccated form of an ancient sorcerer, animated by dark arts and lust for power, commands the obedience of the infernal planes. $HISHER essence is anchored to reality by a phylactery that is always in $HISHER possession, and the lich cannot die unless $HISHER phylactery is destroyed.",
        "enchanting", "Enchanting",
        {"touches", {0}},
        {0},
        "rasps a terrifying incantation!"},
    {"This gem was the fulcrum of a dark rite, performed centuries ago, that bound the soul of an ancient and terrible sorcerer. Hurry and destroy the gem, before the lich can gather its power and regenerate its corporeal form!",
        "enchanting", "Enchanting",
        {"touches", {0}},
        {0},
        "swirls with dark sorcery as the lich regenerates its form!"},
    {"A tiny humanoid sparkles in the gloom, the hum of $HISHER beating wings punctuated by intermittent peals of high-pitched laughter. What $HESHE lacks in physical endurance, $HESHE makes up for with $HISHER wealth of mischievous magical abilities.",
        "sprinkling dust on", "Dusting",
        {"pokes", {0}}},
    {"A silhouette of mournful rage against an empty backdrop, the phantom slips through the dungeon invisibly in clear air, leaving behind glowing droplets of ectoplasm and the cries of $HISHER unsuspecting victims.",
        "permeating", "Permeating",
        {"hits", {0}}},
    {"This infernal contraption spits blasts of flame at intruders.",
        "incinerating", "Incinerating",
        {"pricks", {0}}},
    {"This trickster demon moves with astonishing speed and delights in stealing from $HISHER enemies and blinking away.",
        "dissecting", "Dissecting",
        {"slices", "cuts", {0}}},
    {"A creature of inchoate rage made flesh, the fury's moist wings beat loudly in the darkness.",
        "flagellating", "Flagellating",
        {"drubs", "fustigates", "castigates", {0}}},
    {"This unholy specter stalks the deep places of the earth without fear, impervious to conventional attacks.",
        "desecrating", "Desecrating",
        {"hits", {0}}},
    {"This seething, towering nightmare of fleshy tentacles slinks through the bowels of the world. The tentacle horror's incredible strength and regeneration make $HIMHER one of the most fearsome creatures of the dungeon.",
        "sucking on", "Consuming",
        {"slaps", "batters", "crushes", {0}}},
    {"A statue animated by an ancient and tireless magic, the golem does not regenerate and attacks with only moderate strength, but $HISHER stone form can withstand incredible damage before collapsing into rubble.",
        "cradling", "Cradling",
        {"backhands", "punches", "kicks", {0}}},
    {"An ancient serpent of the world's deepest places, the dragon's immense form belies its lightning-quick speed and testifies to $HISHER breathtaking strength. An undying furnace of white-hot flames burns within $HISHER scaly hide, and few could withstand a single moment under $HISHER infernal lash.",
        "consuming", "Consuming",
        {"claws", "tail-whips", "bites", {0}}},

    {"Taller, stronger and smarter than other goblins, the warlord commands the loyalty of $HISHER kind and can summon them into battle.",
        "chanting over", "Chanting",
        {"slashes", "cuts", "stabs", "skewers", {0}},
        {0},
        "lets loose a deafening war cry!"},
    {"This blob of jet-black goo is as rare as $HESHE is deadly. Few creatures of the dungeon can withstand $HISHER caustic assault. Beware.",
        "absorbing", "Feeding",
        {"smears", "slimes", "drenches"}},
    {"This vampire lives a solitary life deep underground, consuming any warm-blooded creature unfortunate enough to venture near $HISHER lair.",
        "draining", "Drinking",
        {"grazes", "bites", "buries $HISHER fangs in", {0}},
        {0},
        "spreads his cloak and bursts into a cloud of bats!"},
    {"An elemental creature from another plane of existence, the infernal flamedancer burns with such intensity that $HESHE is painful to behold.",
        "immolating", "Consuming",
        {"singes", "burns", "immolates", {0}}},

    {"Eldritch forces have coalesced to form this flickering, ethereal weapon.",
        "gazing at", "Gazing",
        {"nicks",  {0}}},
    {"Eldritch energies bound up in your equipment have leapt forth to project this spectral image.",
        "gazing at", "Gazing",
        {"hits",  {0}}},
    {"Guarding the room is a weathered stone statue of a knight carrying a battleaxe, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"A statue of a sword-wielding angel surveys the room, connected to the glowing glyphs on the floor by invisible strands of enchantment.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"A spectral outline of a knight carrying a battleaxe casts an ethereal light on $HISHER surroundings.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"An immortal presence stalks through the dungeon, implacably hunting that which was taken... and the one who took it.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},
    {"This totem sits at the center of a summoning circle that radiates a strange energy.",
        "gazing at", "Gazing",
        {"strikes",  {0}},
        {0},
        "crackles with energy as you touch the glyph!"},
    {"A prism of shoulder-high mirrored surfaces gleams in the darkness.",
        "gazing at", "Gazing",
        {"strikes",  {0}}},

    {"The unicorn's flowing mane and tail shine with rainbow light, $HISHER horn glows with healing and protective magic, and $HISHER eyes implore you to always chase your dreams. Unicorns are rumored to be attracted to virgins -- is there a hint of accusation in $HISHER gaze?",
        "consecrating", "Consecrating",
        {"pokes", "stabs", "gores", {0}}},
    {"A whirling desert storm given human shape, the ifrit's twin scimitars flicker in the darkness and $HISHER eyes burn with otherworldly zeal.",
        "absorbing", "Absorbing",
        {"cuts", "slashes", "lacerates", {0}}},
    {"This legendary bird shines with a brilliant light, and $HISHER wings crackle and pop like embers as they beat the air. When $HESHE dies, legend has it that an egg will form and a newborn phoenix will rise from its ashes.",
        "cremating", "Cremating",
        {"pecks", "scratches", "claws", {0}}},
    {"Cradled in a nest of cooling ashes, the translucent membrane of the phoenix egg reveals a yolk that glows brighter by the second.",
        "cremating", "Cremating",
        {"touches", {0}},
        {0},
        "bursts as a newborn phoenix rises from the ashes!"},
    {"This mangrove dryad is as old as the earth, and $HISHER gnarled figure houses an ancient power. When angered, $HESHE can call upon the forces of nature to bind $HISHER foes and tear them to shreds.",
        "absorbing", "Absorbing",
        {"whips", "lashes", "thrashes", "lacerates", {0}}},
};

const mutation mutationCatalog[NUMBER_MUTATORS] = {
    //Title         textColor       healthFactor    moveSpdMult attackSpdMult   defMult damMult DF% DFtype  light   monstFlags  abilityFlags    forbiddenFlags      forbiddenAbilities      canBeNegated
    {"explosive",   &orange,        50,             100,        100,            50,     100,    0,  DF_MUTATION_EXPLOSION, EXPLOSIVE_BLOAT_LIGHT, 0, MA_DF_ON_DEATH, MONST_SUBMERGES, 0,
        "A rare mutation will cause $HIMHER to explode violently when $HESHE dies.",    true},
    {"infested",    &lichenColor,   50,             100,        100,            50,     100,    0,  DF_MUTATION_LICHEN, 0, 0,   MA_DF_ON_DEATH, 0,               0,
        "$HESHE has been infested by deadly lichen spores; poisonous fungus will spread from $HISHER corpse when $HESHE dies.", true},
    {"agile",       &lightBlue,     100,            50,         100,            150,    100,    -1, 0,      0,      MONST_FLEES_NEAR_DEATH, 0, MONST_FLEES_NEAR_DEATH, 0,
        "A rare mutation greatly enhances $HISHER mobility.",   false},
    {"juggernaut",  &brown,         300,            200,        200,            75,     200,    -1, 0,      0,      0,          MA_ATTACKS_STAGGER, MONST_MAINTAINS_DISTANCE, 0,
        "A rare mutation has hardened $HISHER flesh, increasing $HISHER health and power but compromising $HISHER speed.",  false},
    {"grappling",   &tanColor,      150,            100,        100,            50,     100,    -1, 0,      0,      0,          MA_SEIZES,      MONST_MAINTAINS_DISTANCE, MA_SEIZES,
        "A rare mutation has caused suckered tentacles to sprout from $HISHER frame, increasing $HISHER health and allowing $HIMHER to grapple with $HISHER prey.", true},
    {"vampiric",    &red,           100,            100,        100,            100,    100,    -1, 0,      0,      0,          MA_TRANSFERENCE, MONST_MAINTAINS_DISTANCE, MA_TRANSFERENCE,
        "A rare mutation allows $HIMHER to heal $HIMSELFHERSELF with every attack.",    true},
    {"toxic",       &green,         100,            100,        200,            100,    20,     -1, 0,      0,      0,          (MA_CAUSES_WEAKNESS | MA_POISONS), MONST_MAINTAINS_DISTANCE, (MA_CAUSES_WEAKNESS | MA_POISONS),
        "A rare mutation causes $HIMHER to poison $HISHER victims and sap their strength with every attack.",   true},
    {"reflective",  &darkTurquoise, 100,            100,        100,            100,    100,    -1, 0,      0,      MONST_REFLECT_50, 0,         (MONST_REFLECT_50 | MONST_ALWAYS_USE_ABILITY), 0,
        "A rare mutation has coated $HISHER flesh with reflective scales.",     true},
};

const monsterClass monsterClassCatalog[MONSTER_CLASS_COUNT] = {
    // name             frequency   maxDepth    member list
    {"abomination",     10,         -1,         {MK_BOG_MONSTER, MK_UNDERWORM, MK_KRAKEN, MK_TENTACLE_HORROR}},
    {"dar",             10,         22,         {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE}},
    {"animal",          10,         10,         {MK_RAT, MK_MONKEY, MK_JACKAL, MK_EEL, MK_TOAD, MK_VAMPIRE_BAT, MK_CENTIPEDE, MK_SPIDER}},
    {"goblin",          10,         10,         {MK_GOBLIN, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN_TOTEM, MK_GOBLIN_CHIEFTAN, MK_SPECTRAL_BLADE}},
    {"ogre",            10,         16,         {MK_OGRE, MK_OGRE_SHAMAN, MK_OGRE_TOTEM}},
    {"dragon",          10,         -1,         {MK_DRAGON}},
    {"undead",          10,         -1,         {MK_ZOMBIE, MK_WRAITH, MK_VAMPIRE, MK_PHANTOM, MK_LICH, MK_REVENANT}},
    {"jelly",           10,         15,         {MK_PINK_JELLY, MK_BLACK_JELLY, MK_ACID_JELLY}},
    {"turret",          5,          18,         {MK_ARROW_TURRET, MK_SPARK_TURRET, MK_DART_TURRET, MK_FLAME_TURRET}},
    {"infernal",        10,         -1,         {MK_FLAMEDANCER, MK_IMP, MK_REVENANT, MK_FURY, MK_PHANTOM, MK_IFRIT}},
    {"mage",            10,         -1,         {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_OGRE_SHAMAN, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE, MK_PIXIE, MK_LICH}},
    {"waterborne",      10,         17,         {MK_EEL, MK_NAGA, MK_KRAKEN}},
    {"airborne",        10,         15,         {MK_VAMPIRE_BAT, MK_WILL_O_THE_WISP, MK_PIXIE, MK_PHANTOM, MK_FURY, MK_IFRIT, MK_PHOENIX}},
    {"fireborne",       10,         12,         {MK_WILL_O_THE_WISP, MK_SALAMANDER, MK_FLAMEDANCER, MK_PHOENIX}},
    {"troll",           10,         15,         {MK_TROLL}},
};

// ITEMS

char itemTitles[NUMBER_ITEM_TITLES][30];

const char itemCategoryNames[NUMBER_ITEM_CATEGORIES][11] = {
        "food",
        "weapon",
        "armor",
        "potion",
        "scroll",
        "staff",
        "wand",
        "ring",
        "charm",
        "gold",
        "amulet",
        "lumenstone",
        "key"
};

const char titlePhonemes[NUMBER_TITLE_PHONEMES][30] = {
    "glorp",
    "snarg",
    "gana",
    "flin",
    "herba",
    "pora",
    "nuglo",
    "greep",
    "nur",
    "lofa",
    "poder",
    "nidge",
    "pus",
    "wooz",
    "flem",
    "bloto",
    "porta",
    "ermah",
    "gerd",
    "nurt",
    "flurx",
};

char itemColors[NUMBER_ITEM_COLORS][30];

const char itemColorsRef[NUMBER_ITEM_COLORS][30] = {
    "crimson",
    "scarlet",
    "orange",
    "yellow",
    "green",
    "blue",
    "indigo",
    "violet",
    "puce",
    "mauve",
    "burgundy",
    "turquoise",
    "aquamarine",
    "gray",
    "pink",
    "white",
    "lavender",
    "tan",
    "brown",
    "cyan",
    "black"
};

char itemWoods[NUMBER_ITEM_WOODS][30];

const char itemWoodsRef[NUMBER_ITEM_WOODS][30] = {
    "teak",
    "oak",
    "redwood",
    "rowan",
    "willow",
    "mahogany",
    "pinewood",
    "maple",
    "bamboo",
    "ironwood",
    "pearwood",
    "birch",
    "cherry",
    "eucalyptus",
    "walnut",
    "cedar",
    "rosewood",
    "yew",
    "sandalwood",
    "hickory",
    "hemlock",
};

char itemMetals[NUMBER_ITEM_METALS][30];

const char itemMetalsRef[NUMBER_ITEM_METALS][30] = {
    "bronze",
    "steel",
    "brass",
    "pewter",
    "nickel",
    "copper",
    "aluminum",
    "tungsten",
    "titanium",
    "cobalt",
    "chromium",
    "silver",
};

char itemGems[NUMBER_ITEM_GEMS][30];

const char itemGemsRef[NUMBER_ITEM_GEMS][30] = {
    "diamond",
    "opal",
    "garnet",
    "ruby",
    "amethyst",
    "topaz",
    "onyx",
    "tourmaline",
    "sapphire",
    "obsidian",
    "malachite",
    "aquamarine",
    "emerald",
    "jade",
    "alexandrite",
    "agate",
    "bloodstone",
    "jasper"
};

itemTable keyTable[NUMBER_KEY_TYPES] = {
    {"door key",            "", "", 1, 0,   0, 0, {0,0,0}, true, false, 0, false, "The notches on this ancient iron key are well worn; its leather lanyard is battered by age. What door might it open?"},
    {"cage key",            "", "", 1, 0,   0, 0, {0,0,0}, true, false, 0, false, "The rust accreted on this iron key has been stained with flecks of blood; it must have been used recently. What cage might it open?"},
    {"crystal orb",         "", "", 1, 0,   0, 0, {0,0,0}, true, false, 0, false, "A faceted orb, seemingly cut from a single crystal, sparkling and perpetually warm to the touch. What manner of device might such an object activate?"},
};

itemTable foodTable[NUMBER_FOOD_KINDS] = {
    {"ration of food",      "", "", 3, 25,  0, 1800, {0,0,0}, true, false, 0, false, "A ration of food. Was it left by former adventurers? Is it a curious byproduct of the subterranean ecosystem?"},
    {"mango",               "", "", 1, 15,  0, 1550, {0,0,0}, true, false, 0, false, "An odd fruit to be found so deep beneath the surface of the earth, but only slightly less filling than a ration of food."}
};

itemTable weaponTable[NUMBER_WEAPON_KINDS] = {
    {"dagger",              "", "", 10, 190,        12, 0, {3, 4,  1},     true, false, 0, false, "A simple iron dagger with a well-worn wooden handle. Daggers will deal quintuple damage upon a successful sneak attack instead of triple damage."},
    {"sword",               "", "", 10, 440,        14, 0, {7, 9,  1},     true, false, 0, false, "The razor-sharp length of steel blade shines reassuringly."},
    {"broadsword",          "", "", 10, 990,        19, 0, {14, 22, 1},    true, false, 0, false, "This towering blade inflicts heavy damage by investing its heft into every cut."},

    {"whip",                "", "", 10, 440,        14, 0, {3, 5,  1},     true, false, 0, false, "The lash from this coil of braided leather can tear bark from trees, and it will reach opponents up to five spaces away."},
    {"rapier",              "", "", 10, 440,        15, 0, {3, 5,  1},     true, false, 0, false, "This blade is thin and flexible, designed for deft and rapid maneuvers. It inflicts less damage than comparable weapons, but permits you to attack twice as quickly. If there is one space between you and an enemy and you step directly toward it, you will perform a devastating lunge attack, which deals triple damage and never misses."},
    {"flail",               "", "", 10, 440,        17, 0, {9, 15, 1},     true, false, 0, false, "This spiked iron ball can be whirled at the end of its chain in synchronicity with your movement, allowing you a free attack whenever moving between two spaces that are adjacent to an enemy."},

    {"mace",                "", "", 10, 660,        16, 0, {16, 20, 1},    true, false, 0, false, "The iron flanges at the head of this weapon inflict substantial damage with every weighty blow. Because of its heft, it takes an extra turn to recover when it hits, and will push your opponent backward if there is room."},
    {"war hammer",          "", "", 10, 1100,       20, 0, {25, 35, 1},    true, false, 0, false, "Few creatures can withstand the crushing blow of this towering mass of lead and steel, but only the strongest of adventurers can effectively wield it. Because of its heft, it takes an extra turn to recover when it hits, and will push your opponent backward if there is room."},

    {"spear",               "", "", 10, 330,        13, 0, {4, 5, 1},      true, false, 0, false, "A slender wooden rod tipped with sharpened iron. The reach of the spear permits you to simultaneously attack an adjacent enemy and the enemy directly behind it."},
    {"war pike",            "", "", 10, 880,        18, 0, {11, 15, 1},    true, false, 0, false, "A long steel pole ending in a razor-sharp point. The reach of the pike permits you to simultaneously attack an adjacent enemy and the enemy directly behind it."},

    {"axe",                 "", "", 10, 550,        15, 0, {7, 9, 1},      true, false, 0, false, "The blunt iron edge on this axe glints in the darkness. The arc of its swing permits you to attack all adjacent enemies simultaneously."},
    {"war axe",             "", "", 10, 990,        19, 0, {12, 17, 1},    true, false, 0, false, "The enormous steel head of this war axe puts considerable heft behind each stroke. The arc of its swing permits you to attack all adjacent enemies simultaneously."},

    {"dart",                "", "", 0,  15,         10, 0, {2, 4,  1},     true, false, 0, false, "These simple metal spikes are weighted to fly true and sting their prey with a flick of the wrist."},
    {"incendiary dart",     "", "", 10, 25,         12, 0, {1, 2,  1},     true, false, 0, false, "The barbed spike on each of these darts is designed to stick to its target while the compounds strapped to its length explode into flame."},
    {"javelin",             "", "", 10, 40,         15, 0, {3, 11, 3},     true, false, 0, false, "This length of metal is weighted to keep the spike at its tip foremost as it sails through the air."},
};

itemTable armorTable[NUMBER_ARMOR_KINDS] = {
    {"leather armor",   "", "", 10, 250,        10, 0, {30,30,0},      true, false, 0, false, "This lightweight armor offers basic protection."},
    {"scale mail",      "", "", 10, 350,        12, 0, {40,40,0},      true, false, 0, false, "Bronze scales cover the surface of treated leather, offering greater protection than plain leather with minimal additional weight."},
    {"chain mail",      "", "", 10, 500,        13, 0, {50,50,0},      true, false, 0, false, "Interlocking metal links make for a tough but flexible suit of armor."},
    {"banded mail",     "", "", 10, 800,        15, 0, {70,70,0},      true, false, 0, false, "Overlapping strips of metal horizontally encircle a chain mail base, offering an additional layer of protection at the cost of greater weight."},
    {"splint mail",     "", "", 10, 1000,       17, 0, {90,90,0},      true, false, 0, false, "Thick plates of metal are embedded into a chain mail base, providing the wearer with substantial protection."},
    {"plate armor",     "", "", 10, 1300,       19, 0, {110,110,0},    true, false, 0, false, "Enormous plates of metal are joined together into a suit that provides unmatched protection to any adventurer strong enough to bear its staggering weight."}
};

const char weaponRunicNames[NUMBER_WEAPON_RUNIC_KINDS][30] = {
    "speed",
    "quietus",
    "paralysis",
    "multiplicity",
    "slowing",
    "confusion",
    "force",
    "slaying",
    "mercy",
    "plenty"
};

const char armorRunicNames[NUMBER_ARMOR_ENCHANT_KINDS][30] = {
    "multiplicity",
    "mutuality",
    "absorption",
    "reprisal",
    "immunity",
    "reflection",
    "respiration",
    "dampening",
    "burden",
    "vulnerability",
    "immolation",
};

itemTable staffTable[NUMBER_STAFF_KINDS] = {
    {"lightning",       itemWoods[0], "",   15, 1300,   0, BOLT_LIGHTNING,     {2,4,1}, false, false, 1,  false, "This staff conjures forth deadly arcs of electricity to damage to any number of creatures in a straight line."},
    {"firebolt",        itemWoods[1], "",   15, 1300,   0, BOLT_FIRE,          {2,4,1}, false, false, 1,  false, "This staff unleashes bursts of magical fire. It will ignite flammable terrain and burn any creature that it hits. Creatures with an immunity to fire will be unaffected by the bolt."},
    {"poison",          itemWoods[3], "",   10, 1200,   0, BOLT_POISON,        {2,4,1}, false, false, 1,  false, "The vile blast of this twisted staff will imbue its target with a deadly venom. Each turn, a creature that is poisoned will suffer one point of damage per dose of poison it has received, and poisoned creatures will not regenerate lost health until the poison clears."},
    {"tunneling",       itemWoods[4], "",   10, 1000,   0, BOLT_TUNNELING,     {2,4,1}, false, false, 1,  false, "Bursts of magic from this staff will pass harmlessly through creatures but will reduce most obstructions to rubble."},
    {"blinking",        itemWoods[5], "",   11, 1200,   0, BOLT_BLINKING,      {2,4,1}, false, false, 1,  false, "This staff will allow you to teleport in the chosen direction. Creatures and inanimate obstructions will block the teleportation."},
    {"entrancement",    itemWoods[6], "",   6,  1000,   0, BOLT_ENTRANCEMENT,  {2,4,1}, false, false, 1,  false, "This staff will send creatures into a temporary trance, causing them to mindlessly mirror your movements. You can use the effect to cause one creature to attack another or to step into hazardous terrain, but the spell will be broken if you attack the creature under the effect."},
    {"obstruction",     itemWoods[7], "",   10, 1000,   0, BOLT_OBSTRUCTION,   {2,4,1}, false, false, 1,  false, "This staff will conjure a mass of impenetrable green crystal, preventing anything from moving through the affected area and temporarily entombing anything that is already there. The crystal will dissolve into the air as time passes. Higher level staffs will create larger obstructions."},
    {"discord",         itemWoods[8], "",   10, 1000,   0, BOLT_DISCORD,       {2,4,1}, false, false, 1,  false, "This staff will alter the perception of a creature and cause it to lash out indiscriminately. Strangers and allies alike will turn on the victim."},
    {"conjuration",     itemWoods[9], "",   8,  1000,   0, BOLT_CONJURATION,   {2,4,1}, false, false, 1,  false, "A flick of this staff will summon a number of phantom blades to fight on your behalf."},
    {"healing",         itemWoods[10], "",  5,  1100,   0, BOLT_HEALING,       {2,4,1}, false, false, -1, false, "This staff will heal any creature, friend or foe. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
    {"haste",           itemWoods[11], "",  5,  900,    0, BOLT_HASTE,         {2,4,1}, false, false, -1, false, "This staff will temporarily double the speed of any creature, friend or foe. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
    {"protection",      itemWoods[12], "",  5,  900,    0, BOLT_SHIELDING,     {2,4,1}, false, false, -1, false, "This staff will bathe a creature in a protective light that will absorb all damage until it is depleted. Unfortunately, you cannot use this or any staff on yourself except by reflecting the bolt."},
};

itemTable ringTable[NUMBER_RING_KINDS] = {
    {"clairvoyance",    itemGems[0], "",    1,  900,    0, 0, {1,3,1}, false, false, 1, false, "This ring of eldritch scrying will permit you to see through nearby walls and doors, within a radius determined by the level of the ring. A cursed ring of clairvoyance will blind you to your immediate surroundings."},
    {"stealth",         itemGems[1], "",    1,  800,    0, 0, {1,3,1}, false, false, 1, false, "This ring of silent passage will reduce your stealth range, making enemies less likely to notice you and more likely to lose your trail. Staying motionless and lurking in the shadows will make you even harder to spot. Cursed rings of stealth will increase your stealth range, making you easier to spot and to track."},
    {"regeneration",    itemGems[2], "",    1,  750,    0, 0, {1,3,1}, false, false, 1, false, "This ring of sacred life will allow you to recover lost health at an accelerated rate. Cursed rings will decrease or even halt your natural regeneration."},
    {"transference",    itemGems[3], "",    1,  750,    0, 0, {1,3,1}, false, false, 1, false, "This ring of blood magic will heal you in proportion to the damage you inflict on others. Cursed rings will cause you to lose health when inflicting damage."},
    {"light",           itemGems[4], "",    1,  600,    0, 0, {1,3,1}, false, false, 1, false, "This ring of preternatural vision will allow you to see farther in the dimming light of the deeper dungeon levels. It will not make you more noticeable to enemies."},
    {"awareness",       itemGems[5], "",    1,  700,    0, 0, {1,3,1}, false, false, 1, false, "This ring of effortless vigilance will enable you to notice traps, secret doors and hidden levers more often and from a greater distance. Cursed rings of awareness will dull your senses, making it harder to notice secrets without actively searching for them."},
    {"wisdom",          itemGems[6], "",    1,  700,    0, 0, {1,3,1}, false, false, 1, false, "This ring of arcane power will cause your staffs to recharge at an accelerated rate. Cursed rings of wisdom will cause your staffs to recharge more slowly."},
    {"reaping",         itemGems[7], "",    1,  700,    0, 0, {1,3,1}, false, false, 1, false, "This ring of blood magic will recharge your staffs and charms every time you hit an enemy. Cursed rings of reaping will drain your staffs and charms with every hit."},
};

const monsterBehavior monsterBehaviorCatalog[32] = {
    {"is invisible",                    true},          // MONST_INVISIBLE
    {"is an inanimate object",          false},         // MONST_INANIMATE
    {"cannot move",                     false},         // MONST_IMMOBILE
    {"",                                false},         // MONST_CARRY_ITEM_100
    {"",                                false},         // MONST_CARRY_ITEM_25
    {"",                                false},         // MONST_ALWAYS_HUNTING
    {"flees at low health",             false},         // MONST_FLEES_NEAR_DEATH
    {"",                                false},         // MONST_ATTACKABLE_THRU_WALLS
    {"corrodes weapons when hit",       true},          // MONST_DEFEND_DEGRADE_WEAPON
    {"is immune to weapon damage",      true},          // MONST_IMMUNE_TO_WEAPONS
    {"flies",                           true},          // MONST_FLIES
    {"moves erratically",               true},          // MONST_FLITS
    {"is immune to fire",               true},          // MONST_IMMUNE_TO_FIRE
    {"",                                false},         // MONST_CAST_SPELLS_SLOWLY
    {"cannot be entangled",             false},         // MONST_IMMUNE_TO_WEBS
    {"can reflect magic spells",        true},          // MONST_REFLECT_50
    {"never sleeps",                    false},         // MONST_NEVER_SLEEPS
    {"burns unceasingly",               true},          // MONST_FIERY
    {"is invulnerable",                 false},         // MONST_INVULNERABLE
    {"is at home in water",             false},         // MONST_IMMUNE_TO_WATER
    {"cannot venture onto dry land",    false},         // MONST_RESTRICTED_TO_LIQUID
    {"submerges",                       false},         // MONST_SUBMERGES
    {"keeps $HISHER distance",          true},          // MONST_MAINTAINS_DISTANCE
    {"",                                false},         // MONST_WILL_NOT_USE_STAIRS
    {"is animated purely by magic",     false},         // MONST_DIES_IF_NEGATED
    {"",                                false},         // MONST_MALE
    {"",                                false},         // MONST_FEMALE
    {"",                                false},         // MONST_NOT_LISTED_IN_SIDEBAR
    {"moves only when activated",       false},         // MONST_GETS_TURN_ON_ACTIVATION
};

const monsterAbility monsterAbilityCatalog[32] = {
    {"can induce hallucinations",                   true},  // MA_HIT_HALLUCINATE
    {"can steal items",                             true},  // MA_HIT_STEAL_FLEE
    {"lights enemies on fire when $HESHE hits",     true},  // MA_HIT_BURN
    {"can possess $HISHER summoned allies",         true},  // MA_ENTER_SUMMONS
    {"corrodes armor when $HESHE hits",             true},  // MA_HIT_DEGRADE_ARMOR
    {"can summon allies",                           true},  // MA_CAST_SUMMON
    {"immobilizes $HISHER prey",                    true},  // MA_SEIZES
    {"injects poison when $HESHE hits",             true},  // MA_POISONS
    {"",                                            true},  // MA_DF_ON_DEATH
    {"divides in two when struck",                  true},  // MA_CLONE_SELF_ON_DEFEND
    {"dies when $HESHE attacks",                    true},  // MA_KAMIKAZE
    {"recovers health when $HESHE inflicts damage", true},  // MA_TRANSFERENCE
    {"saps strength when $HESHE inflicts damage",   true},  // MA_CAUSE_WEAKNESS
    {"attacks up to two opponents in a line",       false}, // MA_ATTACKS_PENETRATE
    {"attacks all adjacent opponents at once",      false}, // MA_ATTACKS_ALL_ADJACENT
    {"attacks with a whip",                         false}, // MA_ATTACKS_EXTEND
    {"pushes opponents backward when $HESHE hits",  false}, // MA_ATTACKS_STAGGER
    {"avoids attacking in corridors in a group",    true},  // MA_AVOID_CORRIDORS
};

const char monsterBehaviorFlagDescriptions[32][COLS] = {
    "is invisible",                             // MONST_INVISIBLE
    "is an inanimate object",                   // MONST_INANIMATE
    "cannot move",                              // MONST_IMMOBILE
    "",                                         // MONST_CARRY_ITEM_100
    "",                                         // MONST_CARRY_ITEM_25
    "",                                         // MONST_ALWAYS_HUNTING
    "flees at low health",                      // MONST_FLEES_NEAR_DEATH
    "",                                         // MONST_ATTACKABLE_THRU_WALLS
    "corrodes weapons when hit",                // MONST_DEFEND_DEGRADE_WEAPON
    "is immune to weapon damage",               // MONST_IMMUNE_TO_WEAPONS
    "flies",                                    // MONST_FLIES
    "moves erratically",                        // MONST_FLITS
    "is immune to fire",                        // MONST_IMMUNE_TO_FIRE
    "",                                         // MONST_CAST_SPELLS_SLOWLY
    "cannot be entangled",                      // MONST_IMMUNE_TO_WEBS
    "can reflect magic spells",                 // MONST_REFLECT_50
    "never sleeps",                             // MONST_NEVER_SLEEPS
    "burns unceasingly",                        // MONST_FIERY
    "is invulnerable",                          // MONST_INVULNERABLE
    "is at home in water",                      // MONST_IMMUNE_TO_WATER
    "cannot venture onto dry land",             // MONST_RESTRICTED_TO_LIQUID
    "submerges",                                // MONST_SUBMERGES
    "keeps $HISHER distance",                   // MONST_MAINTAINS_DISTANCE
    "",                                         // MONST_WILL_NOT_USE_STAIRS
    "is animated purely by magic",              // MONST_DIES_IF_NEGATED
    "",                                         // MONST_MALE
    "",                                         // MONST_FEMALE
    "",                                         // MONST_NOT_LISTED_IN_SIDEBAR
    "moves only when activated",                // MONST_GETS_TURN_ON_ACTIVATION
};

const char monsterAbilityFlagDescriptions[32][COLS] = {
    "can induce hallucinations",                // MA_HIT_HALLUCINATE
    "can steal items",                          // MA_HIT_STEAL_FLEE
    "lights enemies on fire when $HESHE hits",  // MA_HIT_BURN
    "can possess $HISHER summoned allies",      // MA_ENTER_SUMMONS
    "corrodes armor when $HESHE hits",          // MA_HIT_DEGRADE_ARMOR
    "can summon allies",                        // MA_CAST_SUMMON
    "immobilizes $HISHER prey",                 // MA_SEIZES
    "injects poison when $HESHE hits",          // MA_POISONS
    "",                                         // MA_DF_ON_DEATH
    "divides in two when struck",               // MA_CLONE_SELF_ON_DEFEND
    "dies when $HESHE attacks",                 // MA_KAMIKAZE
    "recovers health when $HESHE inflicts damage",// MA_TRANSFERENCE
    "saps strength when $HESHE inflicts damage",// MA_CAUSE_WEAKNESS

    "attacks up to two opponents in a line",    // MA_ATTACKS_PENETRATE
    "attacks all adjacent opponents at once",   // MA_ATTACKS_ALL_ADJACENT
    "attacks with a whip",                      // MA_ATTACKS_EXTEND
    "pushes opponents backward when $HESHE hits", // MA_ATTACKS_STAGGER
    "avoids attacking in corridors in a group", // MA_AVOID_CORRIDORS
    "reflects magic spells back at the caster", // MA_REFLECT_100
};

const char monsterBookkeepingFlagDescriptions[32][COLS] = {
    "",                                         // MB_WAS_VISIBLE
    "is telepathically bonded with you",        // MB_TELEPATHICALLY_REVEALED
    "",                                         // MB_PREPLACED
    "",                                         // MB_APPROACHING_UPSTAIRS
    "",                                         // MB_APPROACHING_DOWNSTAIRS
    "",                                         // MB_APPROACHING_PIT
    "",                                         // MB_LEADER
    "",                                         // MB_FOLLOWER
    "",                                         // MB_CAPTIVE
    "has been immobilized",                     // MB_SEIZED
    "is currently holding $HISHER prey immobile",// MB_SEIZING
    "is submerged",                             // MB_SUBMERGED
    "",                                         // MB_JUST_SUMMONED
    "",                                         // MB_WILL_FLASH
    "is anchored to reality by $HISHER summoner",// MB_BOUND_TO_LEADER
    "is marked for demonic sacrifice",          // MB_MARKED_FOR_SACRIFICE
};

const statusEffect statusEffectCatalog[NUMBER_OF_STATUS_EFFECTS] = {
    {"Searching",       false, 0}, // STATUS_SEARCHING
    {"Donning Armor",   false, 0}, // STATUS_DONNING
    {"Weakened: -",     false, 0}, // STATUS_WEAKENED
    {"Telepathic",      true,  1}, // STATUS_TELEPATHIC
    {"Hallucinating",   true,  0}, // STATUS_HALLUCINATING
    {"Levitating",      true,  1}, // STATUS_LEVITATING
    {"Slowed",          true,  0}, // STATUS_SLOWED
    {"Hasted",          true,  0}, // STATUS_HASTED
    {"Confused",        true,  0}, // STATUS_CONFUSED
    {"Burning",         false, 0}, // STATUS_BURNING
    {"Paralyzed",       false, 0}, // STATUS_PARALYZED
    {"Poisoned",        false, 0}, // STATUS_POISONED
    {"Stuck",           false, 0}, // STATUS_STUCK
    {"Nauseous",        false, 0}, // STATUS_NAUSEOUS
    {"Discordant",      true,  0}, // STATUS_DISCORDANT
    {"Immune to Fire",  true,  1}, // STATUS_IMMUNE_TO_FIRE
    {"",                false, 0}, // STATUS_EXPLOSION_IMMUNITY,
    {"",                false, 0}, // STATUS_NUTRITION,
    {"",                false, 0}, // STATUS_ENTERS_LEVEL_IN,
    {"",                false, 0}, // STATUS_ENRAGED,
    {"Frightened",      true,  0}, // STATUS_MAGICAL_FEAR
    {"Entranced",       true,  0}, // STATUS_ENTRANCED
    {"Darkened",        true,  0}, // STATUS_DARKNESS
    {"Lifespan",        false, 0}, // STATUS_LIFESPAN_REMAINING
    {"Shielded",        true,  0}, // STATUS_SHIELDED
    {"Invisible",       true,  0}, // STATUS_INVISIBLE
};

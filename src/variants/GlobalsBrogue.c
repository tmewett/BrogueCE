/*
 *  GlobalsBrogue.c
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

/* This file contains variant-specific constants and tables that
 * contain mostly constant data associated with game objects.
 *
 * This data is associated with the vanilla Brogue CE game.
 *
 * Non-array and table-based constant data is defined in the
 * gameConstants struct (see brogueGameConst).
 *
 * The initializeGameVariantBrogue() function is called when the
 * variant is selected and sets the relevant global
 * variables (mostly defined in Globals.c) to the
 * variant-specific data.
 */

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"
#include "GlobalsBrogue.h"

#define AMULET_LEVEL            26
#define DEEPEST_LEVEL           40

const color dominationColor_Brogue =       {0,     0,      100,    80,     25,         0,          0,      true};
const color beckonColor_Brogue =           {10,    10,     10,     5,      5,          5,          50,     true};
const color empowermentColor_Brogue =      {30,    100,    40,     25,     80,         25,         0,      true};
const color lightningColor_Brogue =        {100,   150,    500,    50,     50,         0,          50,     true};
const color fireBoltColor_Brogue =         {500,   150,    0,      45,     30,         0,          0,      true};
const color poisonColor_Brogue =           {0,     0,      0,      10,     50,         10,         0,      true};
const color forceFieldColor_Brogue =       {0,     25,     25,     0,      25,         25,         0,      true};
const color spectralBladeColor_Brogue =    {15,    15,     60,     0,      0,          70,         50,     true};
const color shieldingColor_Brogue =        {150,   75,     0,      0,      50,         175,        0,      true};
const color dragonFireColor_Brogue =       {500,   150,    0,      45,     30,         45,         0,      true};
const color centipedeColor_Brogue =        {75,    25,     85,     0,      0,          0,          0,      false};


const bolt boltCatalog_Brogue[] = {
    {{0}},
    //name                      bolt description                ability description                         char    foreColor       backColor           boltEffect      magnitude       pathDF      targetDF    forbiddenMonsterFlags       flags
    {"teleportation spell",     "casts a teleport spell",       "can teleport other creatures",             0,      NULL,           &blue,              BE_TELEPORT,    10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"slowing spell",           "casts a slowing spell",        "can slow $HISHER enemies",                 0,      NULL,           &green,             BE_SLOW,        10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"polymorph spell",         "casts a polymorphism spell",   "can polymorph other creatures",            0,      NULL,           &purple,            BE_POLYMORPH,   10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"negation magic",          "casts a negation spell",       "can cast negation",                        0,      NULL,           &pink,              BE_NEGATION,    10,             0,          0,          0,                          (BF_TARGET_ENEMIES)},
    {"domination spell",        "casts a domination spell",     "can dominate other creatures",             0,      NULL,           &dominationColor_Brogue,   BE_DOMINATION,  10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"beckoning spell",         "casts a beckoning spell",      "can cast beckoning",                       0,      NULL,           &beckonColor_Brogue,       BE_BECKONING,   10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"spell of plenty",         "casts a spell of plenty",      "can duplicate other creatures",            0,      NULL,           &rainbow,           BE_PLENTY,      10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
    {"invisibility magic",      "casts invisibility magic",     "can turn creatures invisible",             0,      NULL,           &darkBlue,          BE_INVISIBILITY, 10,            0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"empowerment sorcery",     "casts empowerment",            "can cast empowerment",                     0,      NULL,           &empowermentColor_Brogue,  BE_EMPOWERMENT, 10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
    {"lightning",               "casts lightning",              "can hurl lightning bolts",                 0,      NULL,           &lightningColor_Brogue,    BE_DAMAGE,      10,             0,          0,          0,                          (BF_PASSES_THRU_CREATURES | BF_TARGET_ENEMIES | BF_ELECTRIC)},
    {"flame",                   "casts a gout of flame",        "can hurl gouts of flame",                  0,      NULL,           &fireBoltColor_Brogue,     BE_DAMAGE,      4,              0,          0,          MONST_IMMUNE_TO_FIRE,       (BF_TARGET_ENEMIES | BF_FIERY)},
    {"poison ray",              "casts a poison ray",           "can cast poisonous bolts",                 0,      NULL,           &poisonColor_Brogue,       BE_POISON,      10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"tunneling magic",         "casts tunneling",              "can tunnel",                               0,      NULL,           &brown,             BE_TUNNELING,   10,             0,          0,          0,                          (BF_PASSES_THRU_CREATURES)},
    {"blink trajectory",        "blinks",                       "can blink",                                0,      NULL,           &white,             BE_BLINKING,    5,              0,          0,          0,                          (BF_HALTS_BEFORE_OBSTRUCTION)},
    {"entrancement ray",        "casts entrancement",           "can cast entrancement",                    0,      NULL,           &yellow,            BE_ENTRANCEMENT,10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"obstruction magic",       "casts obstruction",            "can cast obstruction",                     0,      NULL,           &forceFieldColor_Brogue,   BE_OBSTRUCTION, 10,             0,          0,          0,                          (BF_HALTS_BEFORE_OBSTRUCTION)},
    {"spell of discord",        "casts a spell of discord",     "can cast discord",                         0,      NULL,           &discordColor,      BE_DISCORD,     10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"conjuration magic",       "casts a conjuration bolt",     "can cast conjuration",                     0,      NULL,           &spectralBladeColor_Brogue, BE_CONJURATION,10,             0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_HALTS_BEFORE_OBSTRUCTION | BF_TARGET_ENEMIES)},
    {"healing magic",           "casts healing",                "can heal $HISHER allies",                  0,      NULL,           &darkRed,           BE_HEALING,     5,              0,          0,          0,                          (BF_TARGET_ALLIES)},
    {"haste spell",             "casts a haste spell",          "can haste $HISHER allies",                 0,      NULL,           &orange,            BE_HASTE,       2,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"slowing spell",           "casts a slowing spell",        "can slow $HISHER enemies",                 0,      NULL,           &green,             BE_SLOW,        2,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"protection magic",        "casts protection",             "can cast protection",                      0,      NULL,           &shieldingColor_Brogue,    BE_SHIELDING,   5,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"spiderweb",               "launches a sticky web",        "can launch sticky webs",                   '*',    &white,         NULL,               BE_NONE,        10,             DF_WEB_SMALL, DF_WEB_LARGE, (MONST_IMMOBILE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"spark",                   "shoots a spark",               "can throw sparks of lightning",            0,      NULL,           &lightningColor_Brogue,    BE_DAMAGE,      1,              0,          0,          0,                          (BF_PASSES_THRU_CREATURES | BF_TARGET_ENEMIES | BF_ELECTRIC)},
    {"dragonfire",              "breathes a gout of white-hot flame", "can breathe gouts of white-hot flame", 0,    NULL,           &dragonFireColor_Brogue,   BE_DAMAGE,      18,             DF_OBSIDIAN, 0,         MONST_IMMUNE_TO_FIRE,       (BF_TARGET_ENEMIES | BF_FIERY | BF_NOT_LEARNABLE)},
    {"arrow",                   "shoots an arrow",              "attacks from a distance",                  G_WEAPON,&gray,         NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"poisoned dart",           "fires a dart",                 "fires strength-sapping darts",             G_WEAPON,&centipedeColor_Brogue,NULL,              BE_ATTACK,      1,              0,          0,          0,                          (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"growing vines",           "releases carnivorous vines into the ground", "conjures carnivorous vines", G_GRASS,&tanColor,      NULL,               BE_NONE,        5,              DF_ANCIENT_SPIRIT_GRASS, DF_ANCIENT_SPIRIT_VINES, (MONST_INANIMATE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS)},
    {"whip",                    "whips",                        "wields a whip",                            '*',    &tanColor,      NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE | BF_DISPLAY_CHAR_ALONG_LENGTH)},
};

    //name              feat description                                        initial value
const feat featTable_Brogue[] = {
    {"Pure Mage",       "Ascend without using a weapon.",                           true},
    {"Pure Warrior",    "Ascend without using a staff, wand or charm.",             true},
    {"Companion",       "Explore 13 new depths with an ally.",                      false},
    {"Specialist",      "Enchant an item to +16.",                                  false},
    {"Jellymancer",     "Obtain 90 jelly allies simultaneously.",                   false},
    {"Dragonslayer",    "Slay a dragon with a melee attack.",                       false},
    {"Paladin",         "Ascend without attacking an unaware or fleeing creature.", true},
    {"Untempted",       "Ascend without picking up gold.",                          true},
};

// Number of lumenstones on each level past amulet
const short lumenstoneDistribution_Brogue[DEEPEST_LEVEL - AMULET_LEVEL] = {3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1};

// Relative generation probabilities of item categories
//                                                   (GOLD,  SCROLL, POTION, STAFF,  WAND,   WEAPON, ARMOR,  FOOD,   RING,   CHARM,    AMULET,   GEM,    KEY)
const short itemGenerationProbabilities_Brogue[13] = {50,    42,     52,     3,      3,      10,     8,      2,      3,      2,        0,        0,      0};

const autoGenerator autoGeneratorCatalog_Brogue[] = {
//   terrain                    layer   DF                          Machine                     reqDungeon  reqLiquid   >Depth  <Depth          freq    minIncp minSlope    maxNumber
    // Ordinary features of the dungeon
    {0,                         0,      DF_GRANITE_COLUMN,          0,                          FLOOR,      NOTHING,    1,      DEEPEST_LEVEL,  60,     100,    0,          4},
    {0,                         0,      DF_CRYSTAL_WALL,            0,                          WALL,       NOTHING,    14,     DEEPEST_LEVEL,  15,     -325,   25,         5},
    {0,                         0,      DF_LUMINESCENT_FUNGUS,      0,                          FLOOR,      NOTHING,    7,      DEEPEST_LEVEL,  15,     -300,   70,         14},
    {0,                         0,      DF_GRASS,                   0,                          FLOOR,      NOTHING,    0,      10,             0,      1000,   -80,        10},
    {0,                         0,      DF_DEAD_GRASS,              0,                          FLOOR,      NOTHING,    4,      9,              0,      -200,   80,         10},
    {0,                         0,      DF_DEAD_GRASS,              0,                          FLOOR,      NOTHING,    9,      14,             0,      1200,   -80,        10},
    {0,                         0,      DF_BONES,                   0,                          FLOOR,      NOTHING,    12,     DEEPEST_LEVEL-1,30,     0,      0,          4},
    {0,                         0,      DF_RUBBLE,                  0,                          FLOOR,      NOTHING,    0,      DEEPEST_LEVEL-1,30,     0,      0,          4},
    {0,                         0,      DF_FOLIAGE,                 0,                          FLOOR,      NOTHING,    0,      8,              15,     1000,   -333,       10},
    {0,                         0,      DF_FUNGUS_FOREST,           0,                          FLOOR,      NOTHING,    13,     DEEPEST_LEVEL,  30,     -600,   50,         12},
    {0,                         0,      DF_BUILD_ALGAE_WELL,        0,                          FLOOR,      DEEP_WATER, 10,     DEEPEST_LEVEL,  50,     0,      0,          2},
    {STATUE_INERT,              DUNGEON,0,                          0,                          WALL,       NOTHING,    6,      DEEPEST_LEVEL-1,5,      -100,   35,         3},
    {STATUE_INERT,              DUNGEON,0,                          0,                          FLOOR,      NOTHING,    10,     DEEPEST_LEVEL-1,50,     0,      0,          3},
    {TORCH_WALL,                DUNGEON,0,                          0,                          WALL,       NOTHING,    6,      DEEPEST_LEVEL-1,5,      -200,   70,         12},

    // Pre-revealed traps
    {GAS_TRAP_POISON,           DUNGEON,0,                          0,                          FLOOR,      NOTHING,    2,      4,              20,     0,      0,          1},
    {NET_TRAP,                  DUNGEON,0,                          0,                          FLOOR,      NOTHING,    2,      5,              20,     0,      0,          1},
    {0,                         0,      0,                          MT_PARALYSIS_TRAP_AREA,     FLOOR,      NOTHING,    2,      6,              20,     0,      0,          1},
    {ALARM_TRAP,                DUNGEON,0,                          0,                          FLOOR,      NOTHING,    4,      7,              20,     0,      0,          1},
    {GAS_TRAP_CONFUSION,        DUNGEON,0,                          0,                          FLOOR,      NOTHING,    2,      10,             20,     0,      0,          1},
    {FLAMETHROWER,              DUNGEON,0,                          0,                          FLOOR,      NOTHING,    4,      12,             20,     0,      0,          1},
    {FLOOD_TRAP,                DUNGEON,0,                          0,                          FLOOR,      NOTHING,    10,     14,             20,     0,      0,          1},

    // Hidden traps
    {GAS_TRAP_POISON_HIDDEN,    DUNGEON,0,                          0,                          FLOOR,      NOTHING,    5,      DEEPEST_LEVEL-1,20,     100,    0,          3},
    {NET_TRAP_HIDDEN,           DUNGEON,0,                          0,                          FLOOR,      NOTHING,    6,      DEEPEST_LEVEL-1,20,     100,    0,          3},
    {0,                         0,      0,                          MT_PARALYSIS_TRAP_HIDDEN_AREA, FLOOR,   NOTHING,    7,      DEEPEST_LEVEL-1,20,     100,    0,          3},
    {ALARM_TRAP_HIDDEN,         DUNGEON,0,                          0,                          FLOOR,      NOTHING,    8,      DEEPEST_LEVEL-1,20,     100,    0,          2},
    {TRAP_DOOR_HIDDEN,          DUNGEON,0,                          0,                          FLOOR,      NOTHING,    9,      DEEPEST_LEVEL-1,20,     100,    0,          2},
    {GAS_TRAP_CONFUSION_HIDDEN, DUNGEON,0,                          0,                          FLOOR,      NOTHING,    11,     DEEPEST_LEVEL-1,20,     100,    0,          3},
    {FLAMETHROWER_HIDDEN,       DUNGEON,0,                          0,                          FLOOR,      NOTHING,    13,     DEEPEST_LEVEL-1,20,     100,    0,          3},
    {FLOOD_TRAP_HIDDEN,         DUNGEON,0,                          0,                          FLOOR,      NOTHING,    15,     DEEPEST_LEVEL-1,20,     100,    0,          3},
    {0,                         0,      0,                          MT_SWAMP_AREA,              FLOOR,      NOTHING,    1,      DEEPEST_LEVEL-1,30,     0,      0,          2},
    {0,                         0,      DF_SUNLIGHT,                0,                          FLOOR,      NOTHING,    0,      5,              15,     500,    -150,       10},
    {0,                         0,      DF_DARKNESS,                0,                          FLOOR,      NOTHING,    1,      15,             15,     500,    -50,        10},
    {STEAM_VENT,                DUNGEON,0,                          0,                          FLOOR,      NOTHING,    16,     DEEPEST_LEVEL-1,30,     100,    0,          3},
    {CRYSTAL_WALL,              DUNGEON,0,                          0,                          WALL,       NOTHING,    DEEPEST_LEVEL,DEEPEST_LEVEL,100,0,      0,          600},

    // Dewars
    {DEWAR_CAUSTIC_GAS,         DUNGEON,DF_CARPET_AREA,             0,                          FLOOR,      NOTHING,    8,      DEEPEST_LEVEL-1,2,      0,      0,          2},
    {DEWAR_CONFUSION_GAS,       DUNGEON,DF_CARPET_AREA,             0,                          FLOOR,      NOTHING,    8,      DEEPEST_LEVEL-1,2,      0,      0,          2},
    {DEWAR_PARALYSIS_GAS,       DUNGEON,DF_CARPET_AREA,             0,                          FLOOR,      NOTHING,    8,      DEEPEST_LEVEL-1,2,      0,      0,          2},
    {DEWAR_METHANE_GAS,         DUNGEON,DF_CARPET_AREA,             0,                          FLOOR,      NOTHING,    8,      DEEPEST_LEVEL-1,2,      0,      0,          2},

    // Flavor machines
    {0,                         0,      DF_LUMINESCENT_FUNGUS,      0,                          FLOOR,      NOTHING,    DEEPEST_LEVEL,DEEPEST_LEVEL,100,0,      0,          200},
    {0,                         0,      0,                          MT_BLOODFLOWER_AREA,        FLOOR,      NOTHING,    1,      30,             25,     140,    -10,        3},
    {0,                         0,      0,                          MT_SHRINE_AREA,             FLOOR,      NOTHING,    5,      AMULET_LEVEL,   7,      0,      0,          1},
    {0,                         0,      0,                          MT_IDYLL_AREA,              FLOOR,      NOTHING,    1,      5,              15,     0,      0,          1},
    {0,                         0,      0,                          MT_REMNANT_AREA,            FLOOR,      NOTHING,    10,     DEEPEST_LEVEL,  15,     0,      0,          2},
    {0,                         0,      0,                          MT_DISMAL_AREA,             FLOOR,      NOTHING,    7,      DEEPEST_LEVEL,  12,     0,      0,          5},
    {0,                         0,      0,                          MT_BRIDGE_TURRET_AREA,      FLOOR,      NOTHING,    5,      DEEPEST_LEVEL-1,6,      0,      0,          2},
    {0,                         0,      0,                          MT_LAKE_PATH_TURRET_AREA,   FLOOR,      NOTHING,    5,      DEEPEST_LEVEL-1,6,      0,      0,          2},
    {0,                         0,      0,                          MT_TRICK_STATUE_AREA,       FLOOR,      NOTHING,    6,      DEEPEST_LEVEL-1,15,     0,      0,          3},
    {0,                         0,      0,                          MT_SENTINEL_AREA,           FLOOR,      NOTHING,    12,     DEEPEST_LEVEL-1,10,     0,      0,          2},
    {0,                         0,      0,                          MT_WORM_AREA,               FLOOR,      NOTHING,    12,     DEEPEST_LEVEL-1,12,     0,      0,          3},
};

const blueprint blueprintCatalog_Brogue[] = {
    {0}, // nothing
    //BLUEPRINTS:
    //name
    //depths          roomSize    freq    featureCt   dungeonProfileType  flags   (features on subsequent lines)
        //FEATURES:
        //DF        terrain     layer       instanceCtRange minInsts    itemCat     itemKind    monsterKind     reqSpace        hordeFl     itemFlags   featureFlags

    // -- REWARD ROOMS --

    {"Mixed item library -- can check one item out at a time",
    {1, 12},           {30, 50},   30,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,1},      1,          WAND,       -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {3,3},      3,          (WEAPON|ARMOR|WAND),-1, 0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {2,3},      2,          (STAFF|RING|CHARM),-1,  0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    {"Single item category library -- can check one item out at a time",
    {1, 12},           {30, 50},   15,     5,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {3,4},      3,          (RING),     -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {4,5},      4,          (STAFF),    -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    {"Treasure room -- apothecary or archive (potions or scrolls)",
    {8, AMULET_LEVEL}, {20, 40},   20,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         0,          0,              {5,7},      2,          (POTION),   -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {4,6},      2,          (SCROLL),   -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         FUNGUS_FOREST,SURFACE,      {3,4},      0,          0,          -1,         0,              2,              0,          0,          0},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    {"Guaranteed good permanent item on a glowing pedestal (runic weapon/armor or 2 staffs)",
    {5, 16},           {10, 30},   30,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {2,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (WEAPON),   -1,         0,              2,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (ARMOR),    -1,         0,              2,              0,          ITEM_IDENTIFIED,(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {2,2},      2,          (STAFF),    -1,         0,              2,              0,          (ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN),   (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    {"Guaranteed good consumable item on glowing pedestals (scrolls of enchanting, potion of life)",
    {10, AMULET_LEVEL},{10, 30},   30,     5,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (SCROLL),   SCROLL_ENCHANTING, 0,       2,              0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (POTION),   POTION_LIFE,0,              2,              0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    {"Commutation altars",
    {13, AMULET_LEVEL},{10, 30},   50,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {DF_MAGIC_PIPING,COMMUTATION_ALTAR,DUNGEON,{2,2},   2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    {"Resurrection altar",
    {13, AMULET_LEVEL},{10, 30},   30,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_IMPREGNABLE | BP_REWARD), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {DF_MACHINE_FLOOR_TRIGGER_REPEATING, RESURRECTION_ALTAR,DUNGEON, {1,1}, 1, 0, -1,       0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    {"Outsourced item -- same item possibilities as in the good permanent item reward room (plus charms), but directly adopted by 1-2 key machines.",
    {5, 17},           {0, 0},     20,     4,          0,                  (BP_REWARD | BP_NO_INTERIOR_FLAG),  {
        {0,         0,          0,              {1,1},      1,          (WEAPON),   -1,         0,              0,              0,          (ITEM_IDENTIFIED | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_NO_THROWING_WEAPONS | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {1,1},      1,          (ARMOR),    -1,         0,              0,              0,          (ITEM_IDENTIFIED | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_REQUIRE_GOOD_RUNIC | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {2,2},      2,          (STAFF),    -1,         0,              0,              0,          (ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         0,          0,              {1,2},      1,          (CHARM),    -1,         0,              0,              0,          (ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    {"Dungeon -- two allies chained up for the taking",
    {5, AMULET_LEVEL}, {30, 80},   12,     5,          0,                  (BP_ROOM | BP_REWARD),  {
        {0,         VOMIT,      SURFACE,        {2,2},      2,          0,          -1,         0,              2,              (HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING)},
        {DF_AMBIENT_BLOOD,MANACLE_T,SURFACE,    {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,MANACLE_L,SURFACE,    {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_BONES,  0,          0,              {2,3},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_VOMIT,  0,          0,              {2,3},      1,          0,          -1,         0,              1,              0,          0,          0},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)}}},
    {"Kennel -- allies locked in cages in an open room; choose one or two to unlock and take with you.",
    {5, AMULET_LEVEL}, {30, 80},   12,     4,          0,                  (BP_ROOM | BP_REWARD),  {
        {0,         MONSTER_CAGE_CLOSED,DUNGEON,{3,5},      3,          0,          -1,         0,              2,              (HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         0,          0,              {1,2},      1,          KEY,        KEY_CAGE,   0,              1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_SKELETON_KEY | MF_KEY_DISPOSABLE)},
        {DF_AMBIENT_BLOOD, 0,   0,              {3,5},      3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_BONES,  0,          0,              {3,5},      3,          0,          -1,         0,              1,              0,          0,          0},
        {0,         TORCH_WALL, DUNGEON,        {2,3},      2,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)}}},
    {"Vampire lair -- allies locked in cages and chained in a hidden room with a vampire in a coffin; vampire has one cage key.",
    {10, AMULET_LEVEL},{50, 80},   5,      4,          0,                  (BP_ROOM | BP_REWARD | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR), {
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              2,              (HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_AMBIENT_BLOOD,MONSTER_CAGE_CLOSED,DUNGEON,{2,4},2,          0,          -1,         0,              2,              (HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE), 0, (MF_GENERATE_HORDE | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
        {DF_TRIGGER_AREA,COFFIN_CLOSED,0,       {1,1},      1,          KEY,        KEY_CAGE,   MK_VAMPIRE,     1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_SKELETON_KEY | MF_MONSTER_TAKE_ITEM | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN | MF_KEY_DISPOSABLE)},
        {DF_AMBIENT_BLOOD,SECRET_DOOR,DUNGEON,  {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    {"Legendary ally -- approach the altar with the crystal key to activate a portal and summon a legendary ally.",
    {8, AMULET_LEVEL}, {30, 50},   15,     2,          0,                  (BP_ROOM | BP_REWARD),  {
        {DF_LUMINESCENT_FUNGUS, ALTAR_KEYHOLE, DUNGEON, {1,1}, 1,       KEY,        KEY_PORTAL, 0,              2,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS),(MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_NEAR_ORIGIN | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE)},
        {DF_LUMINESCENT_FUNGUS, PORTAL, DUNGEON,{1,1},      1,          0,          -1,         0,              2,              HORDE_MACHINE_LEGENDARY_ALLY,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},
    {"Goblin warren",
    {5, 15},           {100, 200}, 15,     9,          DP_GOBLIN_WARREN,   (BP_ROOM | BP_REWARD | BP_MAXIMIZE_INTERIOR | BP_REDESIGN_INTERIOR),    {
        {0,         MUD_FLOOR,  DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         MUD_DOORWAY,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         MUD_WALL,   DUNGEON,        {1,1},      100,        0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (SCROLL),   SCROLL_ENCHANTING, MK_GOBLIN_CHIEFTAN, 2,   0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_MONSTER_SLEEPING | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         PEDESTAL,   DUNGEON,        {1,1},      1,          (POTION),   POTION_LIFE, MK_GOBLIN_CHIEFTAN, 2,         0,          (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_MONSTER_SLEEPING | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         0,          0,              {5, 8},     5,          0,          -1,         0,              2,              HORDE_MACHINE_GOBLIN_WARREN,    0,  (MF_GENERATE_HORDE | MF_NOT_IN_HALLWAY | MF_MONSTER_SLEEPING)},
        {0,         0,          0,              {2,3},      2,          (WEAPON|ARMOR), -1,     0,              1,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_HAY,    0,          0,              {10, 15},   1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY)},
        {DF_JUNK,   0,          0,              {7, 12},    1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY)}}},
    {"Sentinel sanctuary",
    {10, 23},           {100, 200}, 15,  10,           DP_SENTINEL_SANCTUARY, (BP_ROOM | BP_REWARD | BP_MAXIMIZE_INTERIOR | BP_REDESIGN_INTERIOR), {
        {0,         MARBLE_FLOOR,DUNGEON,       {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         CRYSTAL_WALL,DUNGEON,       {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {0,         PEDESTAL, DUNGEON, {1,1},   1,          (SCROLL),   SCROLL_ENCHANTING,0,    2,              0,              (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         PEDESTAL, DUNGEON, {1,1},   1,          (POTION),   POTION_LIFE,0,          2,              0,              (ITEM_KIND_AUTO_ID),    (MF_GENERATE_ITEM | MF_ALTERNATIVE | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {30, 35},   20,         0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING)},
        {0,         STATUE_INERT,DUNGEON,       {3, 5},     3,          0,          -1,         MK_SENTINEL,    2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         STATUE_INERT,DUNGEON,       {10, 15},   8,          0,          -1,         MK_SENTINEL,    2,              0,          0,          MF_BUILD_IN_WALLS},
        {0,         0,          0,              {4, 6},     4,          0,          -1,         MK_GUARDIAN,    1,              0,          0,          MF_TREAT_AS_BLOCKING},
        {0,         0,          0,              {0, 2},     0,          0,          -1,         MK_WINGED_GUARDIAN, 1,          0,          0,          MF_TREAT_AS_BLOCKING},
        {0,         0,          0,              {2,3},      2,          (SCROLL | POTION), -1,  0,              1,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},

    // -- AMULET HOLDER --
    {"Statuary -- key on an altar, area full of statues; take key to cause statues to burst and reveal monsters",
    {10, AMULET_LEVEL},{35, 40},   0,      4,          0,                  (BP_PURGE_INTERIOR | BP_OPEN_INTERIOR), {
        {DF_LUMINESCENT_FUNGUS, AMULET_SWITCH, DUNGEON, {1,1}, 1,       AMULET,     -1,         0,              2,              0,          0,          (MF_GENERATE_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         FUNGUS_FOREST,SURFACE,      {2,3},      0,          0,          -1,         0,              2,              0,          0,          MF_NOT_IN_HALLWAY},
        {0,         STATUE_INSTACRACK,DUNGEON,  {1,1},      1,          0,          -1,         MK_WARDEN_OF_YENDOR,1,          0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_IMPREGNABLE)},
        {0,         TORCH_WALL, DUNGEON,        {3,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)}}},

    // -- VESTIBULES --

    {"Plain locked door, key guarded by an adoptive room",
    {1, AMULET_LEVEL}, {1, 1},     100,        1,      0,                  (BP_VESTIBULE), {
        {0,         LOCKED_DOOR, DUNGEON,       {1,1},      1,          KEY,        KEY_DOOR,   0,              1,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS), (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_GENERATE_ITEM | MF_OUTSOURCE_ITEM_TO_MACHINE | MF_KEY_DISPOSABLE | MF_IMPREGNABLE)}}},
    {"Plain secret door",
    {2, AMULET_LEVEL}, {1, 1},     1,      1,          0,                  (BP_VESTIBULE), {
        {0,         SECRET_DOOR, DUNGEON,       {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
    {"Lever and either an exploding wall or a portcullis",
    {4, AMULET_LEVEL}, {1, 1},     8,      3,          0,                  (BP_VESTIBULE), {
        {0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},   1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
        {0,         PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_IMPREGNABLE)}}},
    {"Flammable barricade in the doorway -- burn the wooden barricade to enter",
    {1, 6},            {1, 1},     10,     3,          0,                  (BP_VESTIBULE), {
        {0,         WOODEN_BARRICADE,DUNGEON,   {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          WEAPON,     INCENDIARY_DART, 0,         1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_INCINERATION, 0,     1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    {"Statue in the doorway -- use a scroll of shattering to enter",
    {1, AMULET_LEVEL}, {1, 1},     6,      2,          0,                  (BP_VESTIBULE), {
        {0,         STATUE_INERT_DOORWAY,DUNGEON,       {1,1},1,        0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          SCROLL,     SCROLL_SHATTERING, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY)}}},
    {"Statue in the doorway -- bursts to reveal monster",
    {5, AMULET_LEVEL}, {2, 2},     6,      2,          0,                  (BP_VESTIBULE), {
        {0,         STATUE_DORMANT_DOORWAY,DUNGEON,     {1, 1}, 1,      0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_PERMIT_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    1,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    {"Throwing tutorial -- toss an item onto the pressure plate to retract the portcullis",
    {1, 4},            {70, 70},   8,      3,          0,                  (BP_VESTIBULE), {
        {DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,      0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         PORTCULLIS_CLOSED,DUNGEON,  {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1},   1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_IMPREGNABLE | MF_ALTERNATIVE)}}},
    {"Pit traps -- area outside entrance is full of pit traps",
    {1, AMULET_LEVEL}, {30, 60},   8,      3,          0,                  (BP_VESTIBULE | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG),    {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_IMPREGNABLE | MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {60, 60},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)}}},
    {"Beckoning obstacle -- a mirrored totem guards the door, and glyph are around the doorway.",
    {5, AMULET_LEVEL}, {15, 30},   8,      3,          0,                  (BP_VESTIBULE | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR), {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_MIRRORED_TOTEM,3,            0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
    {"Guardian obstacle -- a guardian is in the door on a glyph, with other glyphs scattered around.",
    {6, AMULET_LEVEL}, {25, 25},   8,      4,          0,                  (BP_VESTIBULE | BP_OPEN_INTERIOR),  {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          MK_GUARDIAN,    2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          MK_WINGED_GUARDIAN,2,           0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {10,10},    3,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING| MF_NEAR_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},

    // -- KEY HOLDERS --

    {"Nested item library -- can check one item out at a time, and one is a disposable key to another reward room",
    {1, AMULET_LEVEL}, {30, 50},   35,     7,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM | BP_IMPREGNABLE), {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         WALL,       DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING | MF_BUILD_VESTIBULE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,2},      1,          (WEAPON|ARMOR|WAND),-1, 0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_PLAYER_AVOIDS), (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,2},      1,          (STAFF|RING|CHARM),-1,  0,              2,              0,          (ITEM_IS_KEY | ITEM_KIND_AUTO_ID | ITEM_MAX_CHARGES_KNOWN | ITEM_PLAYER_AVOIDS),    (MF_GENERATE_ITEM | MF_NO_THROWING_WEAPONS | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         ALTAR_CAGE_OPEN,DUNGEON,    {1,1},      1,          0,          -1,         0,              2,              0,          (ITEM_IS_KEY | ITEM_PLAYER_AVOIDS | ITEM_MAX_CHARGES_KNOWN),    (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE)},
        {0,         STATUE_INERT,DUNGEON,       {1,3},      0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_BUILD_IN_WALLS | MF_IMPREGNABLE)}}},
    {"Secret room -- key on an altar in a secret room",
    {1, AMULET_LEVEL}, {15, 100},  1,      2,          0,                  (BP_ROOM | BP_ADOPT_ITEM), {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          ITEM_PLAYER_AVOIDS, (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    {"Throwing tutorial -- toss an item onto the pressure plate to retract the cage and reveal the key",
    {1, 4},            {70, 80},   8,      2,          0,                  (BP_ADOPT_ITEM), {
        {0,         ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},   1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)},
        {DF_MEDIUM_HOLE, MACHINE_PRESSURE_PLATE, LIQUID, {1,1}, 1,      0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    {"Rat trap -- getting the key triggers paralysis vents nearby and also causes rats to burst out of the walls",
    {1,8},             {30, 70},   7,      3,          0,                  (BP_ADOPT_ITEM | BP_ROOM),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{1,1},1,      0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         RAT_TRAP_WALL_DORMANT,DUNGEON,{10,20},  5,          0,          -1,         MK_RAT,         1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
    {"Fun with fire -- trigger the fire trap and coax the fire over to the wooden barricade surrounding the altar and key",
    {3, 10},           {80, 100},  10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
        {DF_SURROUND_WOODEN_BARRICADE,ALTAR_INERT,DUNGEON,{1,1},1,      0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         GRASS,      SURFACE,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE | MF_ALTERNATIVE)},
        {DF_SWAMP,  0,          0,              {4,4},      2,          0,          -1,         0,              2,              0,          0,          (MF_ALTERNATIVE | MF_FAR_FROM_ORIGIN)},
        {0,         FLAMETHROWER_HIDDEN,DUNGEON,{1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NEAR_ORIGIN)},
        {0,         GAS_TRAP_POISON_HIDDEN,DUNGEON,{3, 3},  1,          0,          -1,         0,              5,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_ALTERNATIVE)},
        {0,         0,          0,              {2,2},      1,          POTION,     POTION_LICHEN, 0,           3,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    {"Flood room -- key on an altar in a room with pools of eel-infested waters; take key to flood room with shallow water",
    {3, AMULET_LEVEL}, {80, 180},  10,     4,          0,                  (BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),  {
        {0,         FLOOR_FLOODABLE,LIQUID,     {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              5,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_SPREADABLE_WATER_POOL,0,0,          {2, 4},     1,          0,          -1,         0,              5,              HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0}}},
    {"Fire trap room -- key on an altar, pools of water, fire traps all over the place.",
    {4, AMULET_LEVEL}, {80, 180},  6,      5,          0,                  (BP_ROOM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS | BP_PURGE_PATHING_BLOCKERS | BP_ADOPT_ITEM),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         0,          0,              {1, 1},     1,          0,          -1,         0,              4,              0,          0,          MF_BUILD_AT_ORIGIN},
        {0,         FLAMETHROWER_HIDDEN,DUNGEON,{40, 60},   20,         0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {DF_DEEP_WATER_POOL,0,  0,              {4, 4},     1,          0,          -1,         0,              4,              HORDE_MACHINE_WATER_MONSTER,0,MF_GENERATE_HORDE},
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0}}},
    {"Thief area -- empty altar, monster with item, permanently fleeing.",
    {3, AMULET_LEVEL}, {15, 20},   10,     2,          0,                  (BP_ADOPT_ITEM),    {
        {DF_LUMINESCENT_FUNGUS, ALTAR_INERT,DUNGEON,{1,1},  1,          0,          -1,         0,              2,              HORDE_MACHINE_THIEF,0,          (MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_GENERATE_HORDE | MF_MONSTER_TAKE_ITEM | MF_MONSTER_FLEEING)},
        {0,         STATUE_INERT,0,             {3, 5},     2,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    {"Collapsing floor area -- key on an altar in an area; take key to cause the floor of the area to collapse",
    {1, AMULET_LEVEL}, {45, 65},   13,     3,          0,                  (BP_ADOPT_ITEM | BP_TREAT_AS_BLOCKING), {
        {0,         FLOOR_FLOODABLE,DUNGEON,    {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},  1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {DF_ADD_MACHINE_COLLAPSE_EDGE_DORMANT,0,0,{3, 3},   2,          0,          -1,         0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    {"Pit traps -- key on an altar, room full of pit traps",
    {1, AMULET_LEVEL}, {30, 100},  10,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {30, 40},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)}}},
    {"Levitation challenge -- key on an altar, room filled with pit, levitation or lever elsewhere on level, bridge appears when you grab the key/lever.",
    {1, 13},           {75, 120},  10,     9,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         TORCH_WALL, DUNGEON,        {1,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM,LIQUID,{120, 120},1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,      0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE | MF_IMPREGNABLE)}}},
    {"Web climbing -- key on an altar, room filled with pit, spider at altar to shoot webs, bridge appears when you grab the key",
    {7, AMULET_LEVEL}, {55, 90},   10,     7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         MK_SPIDER,      3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN)},
        {0,         TORCH_WALL, DUNGEON,        {1,4},      0,          0,          0,          0,              1,              0,          0,          (MF_BUILD_IN_WALLS)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM,LIQUID,   {120, 120}, 1,      0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_ADD_DORMANT_CHASM_HALO, CHASM_WITH_HIDDEN_BRIDGE,LIQUID,{1,1},1,0,      0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)}}},
    {"Lava moat room -- key on an altar, room filled with lava, levitation/fire immunity/lever elsewhere on level, lava retracts when you grab the key/lever",
    {3, 13},           {75, 120},  7,      7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         LAVA,       LIQUID,         {60,60},    1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_LAVA_RETRACTABLE, LAVA_RETRACTABLE, LIQUID, {1,1}, 1,       0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_FIRE_IMMUNITY, 0,    1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_ALTERNATIVE | MF_IMPREGNABLE)}}},
    {"Lava moat area -- key on an altar, surrounded with lava, levitation/fire immunity elsewhere on level, lava retracts when you grab the key",
    {3, 13},           {40, 60},   3,      5,          0,                  (BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_TREAT_AS_BLOCKING),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         LAVA,       LIQUID,         {60,60},    1,          0,          0,          0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_LAVA_RETRACTABLE, LAVA_RETRACTABLE, LIQUID, {1,1}, 1,       0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_LEVITATION, 0,       1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_FIRE_IMMUNITY, 0,    1,              0,          0,          (MF_GENERATE_ITEM | MF_BUILD_ANYWHERE_ON_LEVEL | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)}}},
    {"Poison gas -- key on an altar; take key to cause a caustic gas vent to appear and the door to be blocked; there is a hidden trapdoor or an escape item somewhere inside",
    {4, AMULET_LEVEL}, {35, 60},   7,      7,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM), {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING)},
        {0,         MACHINE_POISON_GAS_VENT_HIDDEN,DUNGEON,{1,2}, 1,    0,          -1,         0,              2,              0,          0,          0},
        {0,         TRAP_DOOR_HIDDEN,DUNGEON,   {1,1},      1,          0,          -1,         0,              2,              0,          0,          MF_ALTERNATIVE},
        {0,         0,          0,              {1,1},      1,          SCROLL,     SCROLL_TELEPORT, 0,         2,              0,          0,          (MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,1},      1,          POTION,     POTION_DESCENT, 0,          2,              0,          0,          (MF_GENERATE_ITEM | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         WALL_LEVER_HIDDEN_DORMANT,DUNGEON,{1,1},1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IMPREGNABLE)},
        {0,         PORTCULLIS_DORMANT,DUNGEON,{1,1},       1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)}}},
    {"Explosive situation -- key on an altar; take key to cause a methane gas vent to appear and a pilot light to ignite",
    {7, AMULET_LEVEL}, {80, 90},   10,     5,          0,                  (BP_ROOM | BP_PURGE_LIQUIDS | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM),  {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         FLOOR,      DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
        {0,         MACHINE_METHANE_VENT_HIDDEN,DUNGEON,{1,1}, 1,       0,          -1,         0,              1,              0,          0,          MF_NEAR_ORIGIN},
        {0,         PILOT_LIGHT_DORMANT,DUNGEON,{1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_BUILD_IN_WALLS)}}},
    {"Burning grass -- key on an altar; take key to cause pilot light to ignite grass in room",
    {1, 7},            {40, 110},  10,     6,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM | BP_OPEN_INTERIOR),  {
        {DF_SMALL_DEAD_GRASS,ALTAR_SWITCH_RETRACTING,DUNGEON,{1,1},1,   0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_FAR_FROM_ORIGIN)},
        {DF_DEAD_FOLIAGE,0,     SURFACE,        {2,3},      0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         FOLIAGE,    SURFACE,        {1,4},      0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         GRASS,      SURFACE,        {10,25},    0,          0,          -1,         0,              1,              0,          0,          0},
        {0,         DEAD_GRASS, SURFACE,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         PILOT_LIGHT_DORMANT,DUNGEON,{1,1},      1,          0,          -1,         0,              1,              0,          0,          MF_NEAR_ORIGIN | MF_BUILD_IN_WALLS}}},
    {"Statuary -- key on an altar, area full of statues; take key to cause statues to burst and reveal monsters",
    {10, AMULET_LEVEL},{35, 90},   10,     2,          0,                  (BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         STATUE_DORMANT,DUNGEON,     {3,5},      3,          0,          -1,         0,              2,              HORDE_MACHINE_STATUE,0, (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_FAR_FROM_ORIGIN)}}},
    {"Guardian water puzzle -- key held by a guardian, flood trap in the room, glyphs scattered. Lure the guardian into the water to have him drop the key.",
    {4, AMULET_LEVEL}, {35, 70},   8,      4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_ADOPT_ITEM), {
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_GUARDIAN,    2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTER_TAKE_ITEM)},
        {0,         FLOOD_TRAP,DUNGEON,         {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      4,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_NOT_IN_HALLWAY)}}},
    {"Guardian gauntlet -- key in a room full of guardians, glyphs scattered and unavoidable.",
    {6, AMULET_LEVEL}, {50, 95},   10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM),  {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {0,         0,          0,              {3,6},      3,          0,          -1,         MK_GUARDIAN,    2,              0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         0,          0,              {1,2},      1,          0,          -1,         MK_WINGED_GUARDIAN,2,           0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {10,15},   10,          0,          -1,         0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    {"Guardian corridor -- key in a small room, with a connecting corridor full of glyphs, one guardian blocking the corridor.",
    {4, AMULET_LEVEL}, {85, 100},   5,     7,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),        {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         MK_GUARDIAN,    3,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN  | MF_ALTERNATIVE)},
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         MK_WINGED_GUARDIAN,3,           0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN  | MF_ALTERNATIVE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          0,          0,              2,              0,          0,          MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY},
        {0,         0,          0,              {1,1},      1,          0,          0,          0,              3,              0,          0,          MF_BUILD_AT_ORIGIN},
        {0,         WALL,DUNGEON,               {80,80},    1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      1,          0,          0,          0,              1,              0,          0,          (MF_PERMIT_BLOCKING | MF_EVERYWHERE)},
        {0,         MACHINE_GLYPH,DUNGEON,      {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_NOT_IN_HALLWAY | MF_BUILD_ANYWHERE_ON_LEVEL)}}},
    {"Sacrifice altar -- lure the chosen monster from elsewhere on the level onto the altar to release the key.",
    {4, AMULET_LEVEL}, {20, 60},   12,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_OPEN_INTERIOR | BP_SURROUND_WITH_WALLS),        {
        {DF_BONES,  0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {0,         0,          0,              {1,1},      0,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_IN_WALLS | MF_EVERYWHERE)},
        {DF_TRIGGER_AREA,SACRIFICE_ALTAR_DORMANT,DUNGEON,{1,1},1,       0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         SACRIFICE_CAGE_DORMANT,DUNGEON,{1,1},   1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         DEMONIC_STATUE,DUNGEON,     {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         STATUE_INSTACRACK,DUNGEON,  {1,1},      1,          0,          -1,         0,              2,              (HORDE_SACRIFICE_TARGET), 0, (MF_BUILD_ANYWHERE_ON_LEVEL | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_TREAT_AS_BLOCKING | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY)}}},
    {"Summoning circle -- key in a room with an eldritch totem, glyphs unavoidable. // DISABLED. (Not fun enough.)",
    {12, AMULET_LEVEL}, {50, 100}, 0,      2,          0,                  (BP_ROOM | BP_OPEN_INTERIOR | BP_ADOPT_ITEM),   {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {DF_GLYPH_CIRCLE,0,     0,              {1,1},      1,          0,          -1,         MK_ELDRITCH_TOTEM,3,            0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    {"Beckoning obstacle -- key surrounded by glyphs in a room with a mirrored totem.",
    {5, AMULET_LEVEL}, {60, 100},  10,     4,          0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_ADOPT_ITEM), {
        {DF_GLYPH_CIRCLE,ALTAR_INERT,DUNGEON,   {1,1},      1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN | MF_IN_VIEW_OF_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         MK_MIRRORED_TOTEM,3,            0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {0,         0,          0,              {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         MACHINE_GLYPH,DUNGEON,      {3,5},      2,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)}}},
    {"Worms in the walls -- key on altar; take key to cause underworms to burst out of the walls",
    {12,AMULET_LEVEL}, {7, 7},     7,      2,          0,                  (BP_ADOPT_ITEM),    {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         WALL_MONSTER_DORMANT,DUNGEON,{5,8},     5,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)}}},
    {"Mud pit -- key on an altar, room full of mud, take key to cause bog monsters to spawn in the mud",
    {12, AMULET_LEVEL},{40, 90},   10,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS),  {
        {DF_SWAMP,      0,      0,              {5,5},      0,          0,          -1,         0,              1,              0,          0,          0},
        {DF_SWAMP,  ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {DF_MUD_DORMANT,0,      0,              {3,4},      3,          0,          -1,         0,              1,              HORDE_MACHINE_MUD,0,    (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT)}}},
    {"Electric crystals -- key caged on an altar, darkened crystal globes around the room, lightning the globes to release the key.",
    {6, AMULET_LEVEL},{40, 60},    10,     5,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR | BP_PURGE_INTERIOR),  {
        {0,         CARPET,     DUNGEON,        {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         ELECTRIC_CRYSTAL_OFF,DUNGEON,{3,4},     3,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IMPREGNABLE)},
        {0,         SACRED_GLYPH,  DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         ALTAR_CAGE_RETRACTABLE,DUNGEON,{1,1},   1,          0,          -1,         0,              3,              0,          0,          (MF_ADOPT_ITEM | MF_IMPREGNABLE | MF_NOT_IN_HALLWAY | MF_FAR_FROM_ORIGIN)},
        {0,         TURRET_LEVER, DUNGEON,      {7,9},      4,          0,          -1,         MK_SPARK_TURRET,3,              0,          0,          (MF_BUILD_IN_WALLS | MF_MONSTERS_DORMANT | MF_IMPREGNABLE)}}},
    {"Zombie crypt -- key on an altar; coffins scattered around; brazier in the room; take key to cause zombies to burst out of all of the coffins",
    {12, AMULET_LEVEL},{60, 90},   10,     8,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_INTERIOR), {
        {0,         DOOR,       DUNGEON,        {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {DF_BONES,  0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {DF_ASH,    0,          0,              {3,4},      2,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {DF_AMBIENT_BLOOD,0,    0,              {1,2},      1,          0,          -1,         0,              1,              0,          0,          0},
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         BRAZIER,    DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_NEAR_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         COFFIN_CLOSED, DUNGEON,     {6,8},      1,          0,          0,          MK_ZOMBIE,      2,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY | MF_MONSTERS_DORMANT)}}},
    {"Haunted house -- key on an altar; take key to cause the room to darken, ectoplasm to cover everything and phantoms to appear",
    {16, AMULET_LEVEL},{45, 150},  10,     4,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS), {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         DARK_FLOOR_DORMANT,DUNGEON, {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         DARK_FLOOR_DORMANT,DUNGEON, {4,5},      4,          0,          -1,         MK_PHANTOM,     1,              0,          0,          (MF_MONSTERS_DORMANT)},
        {0,         HAUNTED_TORCH_DORMANT,DUNGEON,{5,10},   3,          0,          -1,         0,              2,              0,          0,          (MF_BUILD_IN_WALLS)}}},
    {"Worm tunnels -- hidden lever causes tunnels to open up revealing worm areas and a key",
    {8, AMULET_LEVEL},{80, 175},   10,     6,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_PURGE_INTERIOR | BP_MAXIMIZE_INTERIOR | BP_SURROUND_WITH_WALLS),  {
        {0,         ALTAR_INERT,DUNGEON,        {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING)},
        {0,         0,          0,              {3,6},      3,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          0},
        {0,         GRANITE,    DUNGEON,        {150,150},  1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_WORM_TUNNEL_MARKER_DORMANT,GRANITE,DUNGEON,{0,0},0,         0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE | MF_PERMIT_BLOCKING)},
        {DF_TUNNELIZE,WORM_TUNNEL_OUTER_WALL,DUNGEON,{1,1}, 1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         WALL_LEVER_HIDDEN,DUNGEON,  {1,1},      1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_IN_WALLS | MF_IN_PASSABLE_VIEW_OF_ORIGIN | MF_BUILD_ANYWHERE_ON_LEVEL | MF_IMPREGNABLE)}}},
    {"Gauntlet -- key on an altar; take key to cause turrets to emerge",
    {5, 24},           {35, 90},   10,     2,          0,                  (BP_ADOPT_ITEM | BP_NO_INTERIOR_FLAG),  {
        {0,         ALTAR_SWITCH,DUNGEON,       {1,1},      1,          0,          -1,         0,              2,              0,          0,          (MF_ADOPT_ITEM | MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {4,6},      4,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    {"Boss -- key is held by a boss atop a pile of bones in a secret room. A few fungus patches light up the area.",
    {5, AMULET_LEVEL}, {40, 100},  18,     3,          0,                  (BP_ROOM | BP_ADOPT_ITEM | BP_SURROUND_WITH_WALLS | BP_PURGE_LIQUIDS), {
        {DF_BONES,  SECRET_DOOR,DUNGEON,        {1,1},      1,          0,          0,          0,              3,              0,          0,          (MF_PERMIT_BLOCKING | MF_BUILD_AT_ORIGIN)},
        {DF_LUMINESCENT_FUNGUS, STATUE_INERT,DUNGEON,{7,7}, 0,          0,          -1,         0,              2,              0,          0,          (MF_TREAT_AS_BLOCKING)},
        {DF_BONES,  0,          0,              {1,1},      1,          0,          -1,         0,              1,              HORDE_MACHINE_BOSS, 0,  (MF_ADOPT_ITEM | MF_FAR_FROM_ORIGIN | MF_MONSTER_TAKE_ITEM | MF_GENERATE_HORDE | MF_MONSTER_SLEEPING)}}},

    // -- FLAVOR MACHINES --

    {"Bloodwort -- bloodwort stalk, some pods, and surrounding grass",
    {1,DEEPEST_LEVEL}, {5, 5},     0,          2,      0,                  (BP_TREAT_AS_BLOCKING), {
        {DF_GRASS,  BLOODFLOWER_STALK, SURFACE, {1, 1},     1,          0,          -1,         0,              0,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_NOT_IN_HALLWAY)},
        {DF_BLOODFLOWER_PODS_GROW_INITIAL,0, 0, {1, 1},     1,          0,          -1,         0,              1,              0,          0,          (MF_BUILD_AT_ORIGIN | MF_TREAT_AS_BLOCKING)}}},
    {"Shrine -- safe haven constructed and abandoned by a past adventurer",
    {1,DEEPEST_LEVEL}, {15, 25},   0,          3,      0,                  (BP_ROOM | BP_PURGE_INTERIOR | BP_SURROUND_WITH_WALLS | BP_OPEN_INTERIOR), {
        {0,         SACRED_GLYPH,  DUNGEON,     {1, 1},     1,          0,          -1,         0,              3,              0,          0,          (MF_BUILD_AT_ORIGIN)},
        {0,         HAVEN_BEDROLL, SURFACE,     {1, 1},     1,          0,          -1,         0,              2,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)},
        {0,         BONES,      SURFACE,        {1, 1},     1,          (POTION|SCROLL|WEAPON|ARMOR|RING),-1,0, 2,              0,          0,          (MF_GENERATE_ITEM | MF_TREAT_AS_BLOCKING | MF_NOT_IN_HALLWAY)}}},
    {"Idyll -- ponds and some grass and forest",
    {1,DEEPEST_LEVEL}, {80, 120},  0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_GRASS,  FOLIAGE,    SURFACE,        {3, 4},     3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_DEEP_WATER_POOL,0,  0,              {2, 3},     2,          0,          -1,         0,              5,              0,          0,          (MF_NOT_IN_HALLWAY)}}},
    {"Swamp -- mud, grass and some shallow water",
    {1,DEEPEST_LEVEL}, {50, 65},   0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_SWAMP,  0,          0,              {6, 8},     3,          0,          -1,         0,              1,              0,          0,          0},
        {DF_DEEP_WATER_POOL,0,  0,              {0, 1},     0,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
    {"Camp -- hay, junk, urine, vomit",
    {1,DEEPEST_LEVEL}, {40, 50},   0,      4,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_HAY,    0,          0,              {1, 3},     1,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {DF_JUNK,   0,          0,              {1, 2},     1,          0,          -1,         0,              3,              0,          0,          (MF_NOT_IN_HALLWAY | MF_IN_VIEW_OF_ORIGIN)},
        {DF_URINE,  0,          0,              {3, 5},     1,          0,          -1,         0,              1,              0,          0,          MF_IN_VIEW_OF_ORIGIN},
        {DF_VOMIT,  0,          0,              {0, 2},     0,          0,          -1,         0,              1,              0,          0,          MF_IN_VIEW_OF_ORIGIN}}},
    {"Remnant -- carpet surrounded by ash and with some statues",
    {1,DEEPEST_LEVEL}, {80, 120},  0,      2,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_REMNANT, 0,         0,              {6, 8},     3,          0,          -1,         0,              1,              0,          0,          0},
        {0,         STATUE_INERT,DUNGEON,       {3, 5},     2,          0,          -1,         0,              1,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING)}}},
    {"Dismal -- blood, bones, charcoal, some rubble",
    {1,DEEPEST_LEVEL}, {60, 70},   0,      3,          0,                  BP_NO_INTERIOR_FLAG, {
        {DF_AMBIENT_BLOOD, 0,   0,              {5,10},     3,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY},
        {DF_ASH,    0,          0,              {4, 8},     2,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY},
        {DF_BONES,  0,          0,              {3, 5},     2,          0,          -1,         0,              1,              0,          0,          MF_NOT_IN_HALLWAY}}},
    {"Chasm catwalk -- narrow bridge over a chasm, possibly under fire from a turret or two",
    {1,DEEPEST_LEVEL-1},{40, 80},  0,      4,          0,                  (BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
        {DF_CHASM_HOLE, 0,      0,              {80, 80},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {DF_CATWALK_BRIDGE,0,   0,              {0,0},      0,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)},
        {0,         MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},  0,          0,          0,          0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {1, 2},     1,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    {"Lake walk -- narrow bridge of shallow water through a lake, possibly under fire from a turret or two",
    {1,DEEPEST_LEVEL}, {40, 80},   0,      3,          0,                  (BP_REQUIRE_BLOCKING | BP_OPEN_INTERIOR | BP_NO_INTERIOR_FLAG), {
        {DF_LAKE_CELL,  0,      0,              {80, 80},   1,          0,          -1,         0,              1,              0,          0,          (MF_TREAT_AS_BLOCKING | MF_REPEAT_UNTIL_NO_PROGRESS)},
        {0,         MACHINE_TRIGGER_FLOOR, DUNGEON, {0,1},  0,          0,          0,          0,              1,              0,          0,          (MF_NEAR_ORIGIN | MF_PERMIT_BLOCKING)},
        {0,         TURRET_DORMANT,DUNGEON,     {1, 2},     1,          0,          -1,         0,              2,              HORDE_MACHINE_TURRET,0, (MF_TREAT_AS_BLOCKING | MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_IN_VIEW_OF_ORIGIN)}}},
    {"Paralysis trap -- already-revealed pressure plate with a few hidden vents nearby.",
    {1,DEEPEST_LEVEL}, {35, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         GAS_TRAP_PARALYSIS, DUNGEON, {1,2},     1,          0,          0,          0,              3,              0,          0,          (MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{3, 4},2,     0,          0,          0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    {"Paralysis trap -- hidden pressure plate with a few vents nearby.",
    {1,DEEPEST_LEVEL}, {35, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         GAS_TRAP_PARALYSIS_HIDDEN, DUNGEON, {1,2},1,        0,          0,          0,              3,              0,          0,          (MF_NEAR_ORIGIN | MF_NOT_IN_HALLWAY)},
        {0,         MACHINE_PARALYSIS_VENT_HIDDEN,DUNGEON,{3, 4},2,     0,          0,          0,              3,              0,          0,          (MF_FAR_FROM_ORIGIN | MF_NOT_IN_HALLWAY)}}},
    {"Statue comes alive -- innocent-looking statue that bursts to reveal a monster when the player approaches",
    {1,DEEPEST_LEVEL}, {5, 5},     0,      3,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         STATUE_DORMANT,DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_AT_ORIGIN | MF_ALTERNATIVE)},
        {0,         STATUE_DORMANT,DUNGEON,     {1, 1},     1,          0,          -1,         0,              1,              HORDE_MACHINE_STATUE,0, (MF_GENERATE_HORDE | MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_ALTERNATIVE | MF_NOT_ON_LEVEL_PERIMETER)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    2,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    {"Worms in the walls -- step on trigger region to cause underworms to burst out of the walls",
    {1,DEEPEST_LEVEL}, {7, 7},     0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         WALL_MONSTER_DORMANT,DUNGEON,{1, 3},    1,          0,          -1,         MK_UNDERWORM,   1,              0,          0,          (MF_MONSTERS_DORMANT | MF_BUILD_IN_WALLS | MF_NOT_ON_LEVEL_PERIMETER)},
        {0,         MACHINE_TRIGGER_FLOOR,DUNGEON,{0,0},    2,          0,          -1,         0,              0,              0,          0,          (MF_EVERYWHERE)}}},
    {"Sentinels",
    {1,DEEPEST_LEVEL}, {40, 40},   0,      2,          0,                  (BP_NO_INTERIOR_FLAG), {
        {0,         STATUE_INERT,DUNGEON,       {3, 3},     3,          0,          -1,         MK_SENTINEL,    2,              0,          0,          (MF_NOT_IN_HALLWAY | MF_TREAT_AS_BLOCKING | MF_IN_VIEW_OF_ORIGIN)},
        {DF_ASH,    0,          0,              {2, 3},     0,          0,          -1,         0,              0,              0,          0,          0}}},
};

// To meter item generation (on level generation):
// .incrementFrequency must be != 0 for frequency biasing
// .levelScaling != 0 for thresholding
const meteredItemGenerationTable meteredItemsGenerationTable_Brogue[] = {
    { .category = SCROLL, .kind = SCROLL_ENCHANTING, .initialFrequency = 60, .incrementFrequency = 30, .decrementFrequency = 50 },
    { .category = SCROLL, .kind = SCROLL_IDENTIFY },
    { .category = SCROLL, .kind = SCROLL_TELEPORT },
    { .category = SCROLL, .kind = SCROLL_REMOVE_CURSE },
    { .category = SCROLL, .kind = SCROLL_RECHARGING },
    { .category = SCROLL, .kind = SCROLL_PROTECT_ARMOR },
    { .category = SCROLL, .kind = SCROLL_PROTECT_WEAPON },
    { .category = SCROLL, .kind = SCROLL_SANCTUARY },
    { .category = SCROLL, .kind = SCROLL_MAGIC_MAPPING },
    { .category = SCROLL, .kind = SCROLL_NEGATION },
    { .category = SCROLL, .kind = SCROLL_SHATTERING },
    { .category = SCROLL, .kind = SCROLL_DISCORD },
    { .category = SCROLL, .kind = SCROLL_AGGRAVATE_MONSTER },
    { .category = SCROLL, .kind = SCROLL_SUMMON_MONSTER },
    { .category = POTION, .kind = POTION_LIFE, .initialFrequency = 0, .incrementFrequency = 34, .decrementFrequency = 150, .genMultiplier = 4, .genIncrement = 3, .levelScaling = 1 },
    { .category = POTION, .kind = POTION_STRENGTH, .initialFrequency = 40, .incrementFrequency = 17, .decrementFrequency = 50 },
    { .category = POTION, .kind = POTION_TELEPATHY },
    { .category = POTION, .kind = POTION_LEVITATION },
    { .category = POTION, .kind = POTION_DETECT_MAGIC },
    { .category = POTION, .kind = POTION_HASTE_SELF },
    { .category = POTION, .kind = POTION_FIRE_IMMUNITY },
    { .category = POTION, .kind = POTION_INVISIBILITY },
    { .category = POTION, .kind = POTION_POISON },
    { .category = POTION, .kind = POTION_PARALYSIS },
    { .category = POTION, .kind = POTION_HALLUCINATION },
    { .category = POTION, .kind = POTION_CONFUSION },
    { .category = POTION, .kind = POTION_INCINERATION },
    { .category = POTION, .kind = POTION_DARKNESS },
    { .category = POTION, .kind = POTION_DESCENT },
    { .category = POTION, .kind = POTION_LICHEN }
};

// levelFeelings[0] -> AMULET_LEVEL, levelFeelings[1] -> DEEPEST_LEVEL
levelFeeling levelFeelings_Brogue[] = {
    { .message = "An alien energy permeates the area. The Amulet of Yendor must be nearby!", .color = &itemMessageColor },
    { .message = "An overwhelming sense of peace and tranquility settles upon you.", .color = &lightBlue }
};

itemTable potionTable_Brogue[] = {
    {"life",                itemColors[1], "",  0,  500,    0, 0, {10,10,0}, false, false, 1,  false, "A swirling elixir that will instantly heal you, cure you of ailments, and permanently increase your maximum health."}, // frequency is dynamically adjusted
    {"strength",            itemColors[2], "",  0,  400,    0, 0, {1,1,0}, false, false, 1,  false, "This powerful medicine will course through your muscles, permanently increasing your strength by one point."}, // frequency is dynamically adjusted
    {"telepathy",           itemColors[3], "",  20, 350,    0, 0, {300,300,0}, false, false, 1,  false, "This mysterious liquid will attune your mind to the psychic signature of distant creatures. Its effects will not reveal inanimate objects, such as totems, turrets and traps."},
    {"levitation",          itemColors[4], "",  15, 250,    0, 0, {100,100,0}, false, false, 1,  false, "This curious liquid will cause you to hover in the air, able to drift effortlessly over lava, water, chasms and traps. Flames, gases and spiderwebs fill the air, and cannot be bypassed while airborne. Creatures that dwell in water or mud will be unable to attack you while you levitate."},
    {"detect magic",        itemColors[5], "",  20, 500,    0, 0, {0,0,0}, false, false, 1,  false, "This mysterious brew will sensitize your mind to the radiance of magic. Items imbued with helpful enchantments will be marked with a full sigil; items corrupted by curses or designed to bring misfortune upon the bearer will be marked with a hollow sigil. The Amulet of Yendor will be revealed by its unique aura."},
    {"speed",               itemColors[6], "",  10, 500,    0, 0, {25,25,0}, false, false, 1,  false, "Quaffing the contents of this flask will enable you to move at blinding speed for several minutes."},
    {"fire immunity",       itemColors[7], "",  15, 500,    0, 0, {150,150,0}, false, false, 1,  false, "This potion will render you impervious to heat and permit you to wander through fire and lava and ignore otherwise deadly bolts of flame. It will not guard against the concussive impact of an explosion, however."},
    {"invisibility",        itemColors[8], "",  15, 400,    0, 0, {75,75,0}, false, false, 1,  false, "Drinking this potion will render you temporarily invisible. Enemies more than two spaces away will be unable to track you."},
    {"caustic gas",         itemColors[9], "",  15, 200,    0, 0, {0,0,0}, false, false, -1, false, "Uncorking or shattering this pressurized glass will cause its contents to explode into a deadly cloud of caustic purple gas. You might choose to fling this potion at distant enemies instead of uncorking it by hand."},
    {"paralysis",           itemColors[10], "", 10, 250,    0, 0, {0,0,0}, false, false, -1, false, "Upon exposure to open air, the liquid in this flask will vaporize into a numbing pink haze. Anyone who inhales the cloud will be paralyzed instantly, unable to move for some time after the cloud dissipates. This item can be thrown at distant enemies to catch them within the effect of the gas."},
    {"hallucination",       itemColors[11], "", 10, 500,    0, 0, {300,300,0}, false, false, -1, false, "This flask contains a vicious and long-lasting hallucinogen. Under its dazzling effect, you will wander through a rainbow wonderland, unable to discern the form of any creatures or items you see."},
    {"confusion",           itemColors[12], "", 15, 450,    0, 0, {0,0,0}, false, false, -1, false, "This unstable chemical will quickly vaporize into a glittering cloud upon contact with open air, causing any creature that inhales it to lose control of the direction of its movements until the effect wears off (although its ability to aim projectile attacks will not be affected). Its vertiginous intoxication can cause creatures and adventurers to careen into one another or into chasms or lava pits, so extreme care should be taken when under its effect. Its contents can be weaponized by throwing the flask at distant enemies."},
    {"incineration",        itemColors[13], "", 15, 500,    0, 0, {0,0,0}, false, false, -1, false, "This flask contains an unstable compound which will burst violently into flame upon exposure to open air. You might throw the flask at distant enemies -- or into a deep lake, to cleanse the cavern with scalding steam."},
    {"darkness",            itemColors[14], "", 7,  150,    0, 0, {400,400,0}, false, false, -1, false, "Drinking this potion will plunge you into darkness. At first, you will be completely blind to anything not illuminated by an independent light source, but over time your vision will regain its former strength. Throwing the potion will create a cloud of supernatural darkness, and enemies will have difficulty seeing or following you if you take refuge under its cover."},
    {"descent",             itemColors[15], "", 15, 500,    0, 0, {0,0,0}, false, false, -1, false, "When this flask is uncorked by hand or shattered by being thrown, the fog that seeps out will temporarily cause the ground in the vicinity to vanish."},
    {"creeping death",      itemColors[16], "", 7,  450,    0, 0, {0,0,0}, false, false, -1, false, "When the cork is popped or the flask is thrown, tiny spores will spill across the ground and begin to grow a deadly lichen. Anything that touches the lichen will be poisoned by its clinging tendrils, and the lichen will slowly grow to fill the area. Fire will purge the infestation."},
};

itemTable scrollTable_Brogue[] = {
    {"enchanting",          itemTitles[0], "",  0,  550,    0, 1, {0,0,0}, false, false, 1,  false, "This ancient enchanting sorcery will imbue a single item with a powerful and permanent magical charge. A staff will increase in power and in number of charges; a weapon will inflict more damage and find its mark more easily; a suit of armor will deflect attacks more often; the magic of a ring will intensify; and a wand will gain expendable charges in the least amount that such a wand can be found with. Weapons and armor will also require less strength to use, and any curses on the item will be lifted."}, // frequency is dynamically adjusted
    {"identify",            itemTitles[1], "",  30, 300,    0, 0, {0,0,0}, false, false, 1,  false, "This scrying magic will permanently reveal all of the secrets of a single item."},
    {"teleportation",       itemTitles[2], "",  10, 500,    0, 0, {0,0,0}, false, false, 1,  false, "This escape spell will instantly relocate you to a random location on the dungeon level. It can be used to escape a dangerous situation with luck. The unlucky reader might find himself in an even more dangerous place."},
    {"remove curse",        itemTitles[3], "",  15, 150,    0, 0, {0,0,0}, false, false, 1,  false, "This redemption spell will instantly strip from the reader's weapon, armor, rings and carried items any evil enchantments that might prevent the wearer from removing them."},
    {"recharging",          itemTitles[4], "",  12, 375,    0, 0, {0,0,0}, false, false, 1,  false, "The power bound up in this parchment will instantly recharge all of your staffs and charms."},
    {"protect armor",       itemTitles[5], "",  10, 400,    0, 0, {0,0,0}, false, false, 1,  false, "This ceremonial shielding magic will permanently proof your armor against degradation by acid."},
    {"protect weapon",      itemTitles[6], "",  10, 400,    0, 0, {0,0,0}, false, false, 1,  false, "This ceremonial shielding magic will permanently proof your weapon against degradation by acid."},
    {"sanctuary",           itemTitles[7], "",  10, 500,    0, 0, {0,0,0}, false, false, 1,  false, "This protection rite will imbue the area with powerful warding glyphs, when released over plain ground. Monsters will not willingly set foot on the affected area."},
    {"magic mapping",       itemTitles[8], "",  12, 500,    0, 0, {0,0,0}, false, false, 1,  false, "This powerful scouting magic will etch a purple-hued image of crystal clarity into your memory, alerting you to the precise layout of the level and revealing all traps, secret doors and hidden levers."},
    {"negation",            itemTitles[9], "",  8,  400,    0, 0, {0,0,0}, false, false, 1,  false, "When this powerful anti-magic is released, all creatures (including yourself) and all items lying on the ground within your field of view will be exposed to its blast and stripped of magic. Creatures animated purely by magic will die. Potions, scrolls, items being held by other creatures and items in your inventory will not be affected."},
    {"shattering",          itemTitles[10],"",  8,  500,    0, 0, {0,0,0}, false, false, 1,  false, "This strange incantation will alter the physical structure of nearby stone, causing it to evaporate into the air over the ensuing minutes."},
    {"discord",             itemTitles[11], "", 8,  400,    0, 0, {0,0,0}, false, false, 1,  false, "This scroll will unleash a powerful blast of mind magic. Any creatures within line of sight will turn against their companions and attack indiscriminately for 30 turns."},
    {"aggravate monsters",  itemTitles[12], "", 15, 50,     0, 0, {0,0,0}, false, false, -1, false, "This scroll will unleash a piercing shriek that will awaken all monsters and alert them to the reader's location."},
    {"summon monsters",     itemTitles[13], "", 10, 50,     0, 0, {0,0,0}, false, false, -1, false, "This summoning incantation will call out to creatures in other planes of existence, drawing them through the fabric of reality to confront the reader."},
};

itemTable wandTable_Brogue[] = {
    {"teleportation",   itemMetals[0], "",  3,  800,    0, BOLT_TELEPORT,      {3,5,1}, false, false, 1,  false, "This wand will teleport a creature to a random place on the level. Aquatic or mud-bound creatures will be rendered helpless on dry land."},
    {"slowness",        itemMetals[1], "",  3,  800,    0, BOLT_SLOW,          {2,5,1}, false, false, 1,  false, "This wand will cause a creature to move at half its ordinary speed for 30 turns."},
    {"polymorphism",    itemMetals[2], "",  3,  700,    0, BOLT_POLYMORPH,     {3,5,1}, false, false, 1,  false, "This mischievous magic will transform a creature into another creature at random. Beware: the tamest of creatures might turn into the most fearsome. The horror of the transformation will turn an allied victim against you."},
    {"negation",        itemMetals[3], "",  3,  550,    0, BOLT_NEGATION,      {4,6,1}, false, false, 1,  false, "This powerful anti-magic will strip a creature of a host of magical traits, including flight, invisibility, acidic corrosiveness, telepathy, magical speed or slowness, hypnosis, magical fear, immunity to physical attack, fire resistance and the ability to blink. Spellcasters will lose their magical abilities and magical totems will be rendered inert. Creatures animated purely by magic will die."},
    {"domination",      itemMetals[4], "",  1,  1000,   0, BOLT_DOMINATION,    {1,2,1}, false, false, 1,  false, "This wand can forever bind an enemy to the caster's will, turning it into a steadfast ally. However, the magic only works effectively against enemies that are near death."},
    {"beckoning",       itemMetals[5], "",  3,  500,    0, BOLT_BECKONING,     {2,4,1}, false, false, 1,  false, "The force of this wand will draw the targeted creature into direct proximity."},
    {"plenty",          itemMetals[6], "",  2,  700,    0, BOLT_PLENTY,        {1,2,1}, false, false, -1, false, "The creature at the other end of this mischievous bit of cloning magic, friend or foe, will be beside itself -- literally!"},
    {"invisibility",    itemMetals[7], "",  3,  100,    0, BOLT_INVISIBILITY,  {3,5,1}, false, false, -1, false, "This wand will render a creature temporarily invisible to the naked eye. Only with telepathy or in the silhouette of a thick gas will an observer discern the creature's hazy outline."},
    {"empowerment",     itemMetals[8], "",  1,  100,    0, BOLT_EMPOWERMENT,   {1,1,1}, false, false, -1, false, "This sacred magic will permanently improve the mind and body of any monster it hits. A wise adventurer will use it on allies, making them stronger in combat and able to learn a new talent from a fallen foe. If the bolt is reflected back at you, it will have no effect."},
};

itemTable charmTable_Brogue[] = {
    {"health",          "", "", 5,  900,    0, 0, {1,2,1}, true, false, 1, false, "A handful of dried bloodwort and mandrake root has been bound together with leather cord and imbued with a powerful healing magic."},
    {"protection",      "", "", 5,  800,    0, 0, {1,2,1}, true, false, 1, false, "Four copper rings have been joined into a tetrahedron. The construct is oddly warm to the touch."},
    {"haste",           "", "", 5,  750,    0, 0, {1,2,1}, true, false, 1, false, "Various animals have been etched into the surface of this brass bangle. It emits a barely audible hum."},
    {"fire immunity",   "", "", 3,  750,    0, 0, {1,2,1}, true, false, 1, false, "Eldritch flames flicker within this polished crystal bauble."},
    {"invisibility",    "", "", 5,  700,    0, 0, {1,2,1}, true, false, 1, false, "A jade figurine depicts a strange humanoid creature. It has a face on both sides of its head, but all four eyes are closed."},
    {"telepathy",       "", "", 3,  700,    0, 0, {1,2,1}, true, false, 1, false, "Seven tiny glass eyes roll freely within this glass sphere. Somehow, they always come to rest facing outward."},
    {"levitation",      "", "", 1,  700,    0, 0, {1,2,1}, true, false, 1, false, "Sparkling dust and fragments of feather waft and swirl endlessly inside this small glass sphere."},
    {"shattering",      "", "", 1,  700,    0, 0, {1,2,1}, true, false, 1, false, "This turquoise crystal, fixed to a leather lanyard, hums with an arcane energy that sets your teeth on edge."},
    {"guardian",        "", "", 5,  700,    0, 0, {1,2,1}, true, false, 1, false, "When you touch this tiny granite statue, a rhythmic booming echoes in your mind."},
//    {"fear",            "", "",   3,  700,    0,{1,2,1}, true, false, "When you gaze into the murky interior of this obsidian cube, you feel as though something predatory is watching you."},
    {"teleportation",   "", "", 4,  700,    0, 0, {1,2,1}, true, false, 1, false, "The surface of this nickel sphere has been etched with a perfect grid pattern. Somehow, the squares of the grid are all exactly the same size."},
    {"recharging",      "", "", 5,  700,    0, 0, {1,2,1}, true, false, 1, false, "A strip of bronze has been wound around a rough wooden sphere. Each time you touch it, you feel a tiny electric shock."},
    {"negation",        "", "", 5,  700,    0, 0, {1,2,1}, true, false, 1, false, "A featureless gray disc hangs from a lanyard. When you touch it, your hand and arm go numb."},
};

const charmEffectTableEntry charmEffectTable_Brogue[] = {
    { .kind = CHARM_HEALTH, .effectDurationBase = 3, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 2500, .rechargeDelayBase = FP_FACTOR * 55 / 100, .rechargeDelayMinTurns = 1, .effectMagnitudeMultiplier = 20 },
    { .kind = CHARM_PROTECTION, .effectDurationBase = 20, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 1000, .rechargeDelayBase = FP_FACTOR * 60 / 100, .rechargeDelayMinTurns = 1, .effectMagnitudeMultiplier = 150 },
    { .kind = CHARM_HASTE, .effectDurationBase = 7, .effectDurationIncrement = POW_120_CHARM_INCREMENT, .rechargeDelayDuration = 800, .rechargeDelayBase = FP_FACTOR * 65 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_FIRE_IMMUNITY, .effectDurationBase = 10, .effectDurationIncrement = POW_125_CHARM_INCREMENT, .rechargeDelayDuration = 800, .rechargeDelayBase = FP_FACTOR * 60 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_INVISIBILITY, .effectDurationBase = 5, .effectDurationIncrement = POW_120_CHARM_INCREMENT, .rechargeDelayDuration = 800, .rechargeDelayBase = FP_FACTOR * 65 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_TELEPATHY, .effectDurationBase = 25, .effectDurationIncrement = POW_125_CHARM_INCREMENT, .rechargeDelayDuration = 800, .rechargeDelayBase = FP_FACTOR * 65 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_LEVITATION, .effectDurationBase = 10, .effectDurationIncrement = POW_125_CHARM_INCREMENT, .rechargeDelayDuration = 800, .rechargeDelayBase = FP_FACTOR * 65 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_SHATTERING, .effectDurationBase = 0, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 2500, .rechargeDelayBase = FP_FACTOR * 60 / 100, .rechargeDelayMinTurns = 1, .effectMagnitudeConstant = 4 },
    { .kind = CHARM_GUARDIAN, .effectDurationBase = 18, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 700, .rechargeDelayBase = FP_FACTOR * 70 / 100, .rechargeDelayMinTurns = 1, .effectMagnitudeConstant = 4, .effectMagnitudeMultiplier = 2 },
    { .kind = CHARM_TELEPORTATION, .effectDurationBase = 0, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 920, .rechargeDelayBase = FP_FACTOR * 60 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_RECHARGING, .effectDurationBase = 0, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 10000, .rechargeDelayBase = FP_FACTOR * 55 / 100, .rechargeDelayMinTurns = 1 },
    { .kind = CHARM_NEGATION, .effectDurationBase = 0, .effectDurationIncrement = POW_0_CHARM_INCREMENT, .rechargeDelayDuration = 2500, .rechargeDelayBase = FP_FACTOR * 60 / 100, .rechargeDelayMinTurns = 1, .effectMagnitudeConstant = 1, .effectMagnitudeMultiplier = 3 }
};

const hordeType hordeCatalog_Brogue[] = {
    // leader       #members    member list                             member numbers                  minL    maxL    freq    spawnsIn        machine         flags
    {MK_RAT,            0,      {0},                                    {{0}},                          1,      5,      150},
    {MK_KOBOLD,         0,      {0},                                    {{0}},                          1,      6,      150},
    {MK_JACKAL,         0,      {0},                                    {{0}},                          1,      3,      100},
    {MK_JACKAL,         1,      {MK_JACKAL},                            {{1, 3, 1}},                    3,      7,      50},
    {MK_EEL,            0,      {0},                                    {{0}},                          2,      17,     100,        DEEP_WATER},
    {MK_MONKEY,         0,      {0},                                    {{0}},                          2,      9,      50},
    {MK_BLOAT,          0,      {0},                                    {{0}},                          2,      13,     30},
    {MK_PIT_BLOAT,      0,      {0},                                    {{0}},                          2,      13,     10},
    {MK_BLOAT,          1,      {MK_BLOAT},                             {{0, 2, 1}},                    14,     26,     30},
    {MK_PIT_BLOAT,      1,      {MK_PIT_BLOAT},                         {{0, 2, 1}},                    14,     26,     10},
    {MK_EXPLOSIVE_BLOAT,0,      {0},                                    {{0}},                          10,     26,     10},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          3,      10,     100},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          3,      10,     60},
    {MK_TOAD,           0,      {0},                                    {{0}},                          4,      11,     100},
    {MK_PINK_JELLY,     0,      {0},                                    {{0}},                          4,      13,     100},
    {MK_GOBLIN_TOTEM,   1,      {MK_GOBLIN},                            {{2,4,1}},                      5,      13,     100,        0,              MT_CAMP_AREA,   HORDE_NO_PERIODIC_SPAWN},
    {MK_ARROW_TURRET,   0,      {0},                                    {{0}},                          5,      13,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_MONKEY,         1,      {MK_MONKEY},                            {{2,4,1}},                      5,      13,     20},
    {MK_VAMPIRE_BAT,    0,      {0},                                    {{0}},                          6,      13,     30},
    {MK_VAMPIRE_BAT,    1,      {MK_VAMPIRE_BAT},                       {{1,2,1}},                      6,      13,     70,      0,              0,              HORDE_NEVER_OOD},
    {MK_ACID_MOUND,     0,      {0},                                    {{0}},                          6,      13,     100},
    {MK_GOBLIN,         3,      {MK_GOBLIN, MK_GOBLIN_MYSTIC, MK_JACKAL},{{2, 3, 1}, {1,2,1}, {1,2,1}}, 6,      12,     40},
    {MK_GOBLIN_CONJURER,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC}, {{0,1,1}, {1,1,1}},             7,      15,     40},
    {MK_CENTIPEDE,      0,      {0},                                    {{0}},                          7,      14,     100},
    {MK_BOG_MONSTER,    0,      {0},                                    {{0}},                          7,      14,     80,     MUD,            0,              HORDE_NEVER_OOD},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      13,     100},
    {MK_EEL,            1,      {MK_EEL},                               {{2, 4, 1}},                    8,      22,     70,     DEEP_WATER},
    {MK_ACID_MOUND,     1,      {MK_ACID_MOUND},                        {{2, 4, 1}},                    9,      13,     30},
    {MK_SPIDER,         0,      {0},                                    {{0}},                          9,      16,     100},
    {MK_DAR_BLADEMASTER,1,      {MK_DAR_BLADEMASTER},                   {{0, 1, 1}},                    10,     14,     100},
    {MK_WILL_O_THE_WISP,0,      {0},                                    {{0}},                          10,     17,     100},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100},
    {MK_GOBLIN_TOTEM,   4,      {MK_GOBLIN_TOTEM, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN}, {{1,2,1},{1,2,1},{1,2,1},{3,5,1}},10,17,80,0,MT_CAMP_AREA,  HORDE_NO_PERIODIC_SPAWN},
    {MK_SPARK_TURRET,   0,      {0},                                    {{0}},                          11,     18,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_ZOMBIE,         0,      {0},                                    {{0}},                          11,     18,     100},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     19,     100},
    {MK_OGRE_TOTEM,     1,      {MK_OGRE},                              {{2,4,1}},                      12,     19,     60,     0,          0,                  HORDE_NO_PERIODIC_SPAWN},
    {MK_BOG_MONSTER,    1,      {MK_BOG_MONSTER},                       {{2,4,1}},                      12,     26,     100,        MUD},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     20,     100,        DEEP_WATER},
    {MK_SALAMANDER,     0,      {0},                                    {{0}},                          13,     20,     100,        LAVA},
    {MK_OGRE_SHAMAN,    1,      {MK_OGRE},                              {{1, 3, 1}},                    14,     20,     100},
    {MK_CENTAUR,        1,      {MK_CENTAUR},                           {{1, 1, 1}},                    14,     21,     100},
    {MK_ACID_JELLY,     0,      {0},                                    {{0}},                          14,     21,     100},
    {MK_DART_TURRET,    0,      {0},                                    {{0}},                          15,     22,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          14,     21,     80},
    {MK_FLAME_TURRET,   0,      {0},                                    {{0}},                          14,     24,     100,        WALL,   0,                      HORDE_NO_PERIODIC_SPAWN},
    {MK_DAR_BLADEMASTER,2,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS}, {{0, 1, 1}, {0, 1, 1}},         15,     17,     100},
    {MK_PINK_JELLY,     2,      {MK_PINK_JELLY, MK_DAR_PRIESTESS},      {{0, 1, 1}, {1, 2, 1}},         17,     23,     70},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          15,     30,     100,        DEEP_WATER},
    {MK_PHANTOM,        0,      {0},                                    {{0}},                          16,     23,     100},
    {MK_WRAITH,         1,      {MK_WRAITH},                            {{1, 4, 1}},                    16,     23,     80},
    {MK_IMP,            0,      {0},                                    {{0}},                          17,     24,     100},
    {MK_DAR_BLADEMASTER,3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,100},
    {MK_FURY,           1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     80},
    {MK_REVENANT,       0,      {0},                                    {{0}},                          19,     27,     100},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          21,     30,     100},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL-1,        100},
    {MK_PHYLACTERY,     0,      {0},                                    {{0}},                          22,     DEEPEST_LEVEL-1,        100},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          24,     DEEPEST_LEVEL-1,        70},
    {MK_DRAGON,         1,      {MK_DRAGON},                            {{1,1,1}},                      27,     DEEPEST_LEVEL-1,        30},
    {MK_GOLEM,          3,      {MK_GOLEM, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE}, {{1, 2, 1}, {0,1,1},{0,1,1}},27,DEEPEST_LEVEL-1,   80},
    {MK_GOLEM,          1,      {MK_GOLEM},                             {{5, 10, 2}},                   30,     DEEPEST_LEVEL-1,    20},
    {MK_KRAKEN,         1,      {MK_KRAKEN},                            {{5, 10, 2}},                   30,     DEEPEST_LEVEL-1,    100,        DEEP_WATER},
    {MK_TENTACLE_HORROR,2,      {MK_TENTACLE_HORROR, MK_REVENANT},      {{1, 3, 1}, {2, 4, 1}},         32,     DEEPEST_LEVEL-1,    20},
    {MK_DRAGON,         1,      {MK_DRAGON},                            {{3, 5, 1}},                    34,     DEEPEST_LEVEL-1,    20},

    // summons
    {MK_GOBLIN_CONJURER,1,      {MK_SPECTRAL_BLADE},                    {{3, 5, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
    {MK_OGRE_SHAMAN,    1,      {MK_OGRE},                              {{1, 1, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_VAMPIRE,        1,      {MK_VAMPIRE_BAT},                       {{3, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_LICH,           1,      {MK_PHANTOM},                           {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_LICH,           1,      {MK_FURY},                              {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_PHYLACTERY,     1,      {MK_LICH},                              {{1,1,1}},                      0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_GOBLIN_CHIEFTAN,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN},        {{1,1,1}, {3,4,1}},             0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_SUMMONED_AT_DISTANCE},
    {MK_PHOENIX_EGG,    1,      {MK_PHOENIX},                           {{1,1,1}},                      0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED},
    {MK_ELDRITCH_TOTEM, 1,      {MK_SPECTRAL_BLADE},                    {{4, 7, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},
    {MK_ELDRITCH_TOTEM, 1,      {MK_FURY},                              {{2, 3, 1}},                    0,      0,      100,    0,          0,                  HORDE_IS_SUMMONED | HORDE_DIES_ON_LEADER_DEATH},

    // captives
    {MK_MONKEY,         1,      {MK_KOBOLD},                            {{1, 2, 1}},                    1,      5,      10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOBLIN,         1,      {MK_GOBLIN},                            {{1, 2, 1}},                    3,      7,      10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_OGRE,           1,      {MK_GOBLIN},                            {{3, 5, 1}},                    4,      10,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOBLIN_MYSTIC,  1,      {MK_KOBOLD},                            {{3, 7, 1}},                    5,      11,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_OGRE,           1,      {MK_OGRE},                              {{1, 2, 1}},                    8,      15,     20,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          1,      {MK_TROLL},                             {{1, 2, 1}},                    14,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_CENTAUR,        1,      {MK_TROLL},                             {{1, 2, 1}},                    12,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          2,      {MK_OGRE, MK_OGRE_SHAMAN},              {{2, 3, 1}, {0, 1, 1}},         17,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_TROLL},                             {{1, 2, 1}},                    12,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_NAGA,           1,      {MK_SALAMANDER},                        {{1, 2, 1}},                    14,     20,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_SALAMANDER,     1,      {MK_NAGA},                              {{1, 2, 1}},                    13,     20,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TROLL,          1,      {MK_SALAMANDER},                        {{1, 2, 1}},                    13,     19,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_IMP,            1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_PIXIE,          1,      {MK_IMP, MK_PHANTOM},                   {{1, 2, 1}, {1, 2, 1}},         14,     21,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BLADEMASTER,1,      {MK_IMP},                               {{2, 3, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_PRIESTESS,  1,      {MK_FURY},                              {{2, 4, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_DAR_BATTLEMAGE, 1,      {MK_IMP},                               {{2, 3, 1}},                    18,     26,     10,     0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_TENTACLE_HORROR,3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},20,26,10,   0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},
    {MK_GOLEM,          3,      {MK_DAR_BLADEMASTER, MK_DAR_PRIESTESS, MK_DAR_BATTLEMAGE},{{1,2,1},{1,1,1},{1,1,1}},18,25,10,   0,          0,                  HORDE_LEADER_CAPTIVE | HORDE_NEVER_OOD},

    // bosses
    {MK_GOBLIN_CHIEFTAN,2,      {MK_GOBLIN_MYSTIC, MK_GOBLIN, MK_GOBLIN_TOTEM}, {{1,1,1}, {2,3,1}, {2,2,1}},2,  10,     50,     0,          0,                  HORDE_MACHINE_BOSS},
    {MK_BLACK_JELLY,    0,      {0},                                    {{0}},                          5,      15,     50,     0,          0,                  HORDE_MACHINE_BOSS},
    {MK_VAMPIRE,        0,      {0},                                    {{0}},                          10,     DEEPEST_LEVEL,  50,  0,     0,                  HORDE_MACHINE_BOSS},
    {MK_FLAMEDANCER,    0,      {0},                                    {{0}},                          10,     DEEPEST_LEVEL,  50,  0,     0,                  HORDE_MACHINE_BOSS},

    // machine water monsters
    {MK_EEL,            0,      {0},                                    {{0}},                          2,      7,      100,        DEEP_WATER, 0,                  HORDE_MACHINE_WATER_MONSTER},
    {MK_EEL,            1,      {MK_EEL},                               {{2, 4, 1}},                    5,      15,     100,        DEEP_WATER, 0,                  HORDE_MACHINE_WATER_MONSTER},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          12,     DEEPEST_LEVEL,  100,    DEEP_WATER, 0,              HORDE_MACHINE_WATER_MONSTER},
    {MK_KRAKEN,         1,      {MK_EEL},                               {{1, 2, 1}},                    12,     DEEPEST_LEVEL,  80, DEEP_WATER, 0,              HORDE_MACHINE_WATER_MONSTER},

    // dungeon captives -- no captors
    {MK_OGRE,           0,      {0},                                    {{0}},                          4,      13,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          12,     20,     50,     0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      8,      100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          10,     20,     50,     0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          8,      16,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          8,      14,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          11,     20,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          17,     23,     100,        0,          0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          20,     AMULET_LEVEL,100,0,         0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          23,     AMULET_LEVEL,100,0,         0,                  HORDE_MACHINE_CAPTIVE | HORDE_LEADER_CAPTIVE},

    // machine statue monsters
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      6,      100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          6,      12,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          12,     19,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          14,     21,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          21,     30,     100,        STATUE_DORMANT, 0,              HORDE_MACHINE_STATUE},
    {MK_DRAGON,         0,      {0},                                    {{0}},                          29,     DEEPEST_LEVEL,  100,    STATUE_DORMANT, 0,          HORDE_MACHINE_STATUE},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          29,     DEEPEST_LEVEL,  100,    STATUE_DORMANT, 0,          HORDE_MACHINE_STATUE},

    // machine turrets
    {MK_ARROW_TURRET,   0,      {0},                                    {{0}},                          5,      13,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_SPARK_TURRET,   0,      {0},                                    {{0}},                          11,     18,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_DART_TURRET,    0,      {0},                                    {{0}},                          15,     22,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},
    {MK_FLAME_TURRET,   0,      {0},                                    {{0}},                          17,     24,     100,        TURRET_DORMANT, 0,              HORDE_MACHINE_TURRET},

    // machine mud monsters
    {MK_BOG_MONSTER,    0,      {0},                                    {{0}},                          12,     26,     100,        MACHINE_MUD_DORMANT, 0,         HORDE_MACHINE_MUD},
    {MK_KRAKEN,         0,      {0},                                    {{0}},                          17,     26,     30,     MACHINE_MUD_DORMANT, 0,         HORDE_MACHINE_MUD},

    // kennel monsters
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      8,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      17,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     23,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_SALAMANDER,     0,      {0},                                    {{0}},                          9,      20,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     26,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          11,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          9,      AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          12,     AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BATTLEMAGE, 0,      {0},                                    {{0}},                          13,     AMULET_LEVEL, 100, MONSTER_CAGE_CLOSED, 0,      HORDE_MACHINE_KENNEL | HORDE_LEADER_CAPTIVE},

    // vampire bloodbags
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      8,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_GOBLIN_MYSTIC,  0,      {0},                                    {{0}},                          2,      9,      100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_OGRE,           0,      {0},                                    {{0}},                          5,      15,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_TROLL,          0,      {0},                                    {{0}},                          10,     19,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_NAGA,           0,      {0},                                    {{0}},                          9,      20,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_PIXIE,          0,      {0},                                    {{0}},                          11,     21,     100,        MONSTER_CAGE_CLOSED, 0,         HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          9,      AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_PRIESTESS,  0,      {0},                                    {{0}},                          12,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},
    {MK_DAR_BATTLEMAGE, 0,      {0},                                    {{0}},                          13,     AMULET_LEVEL,100,MONSTER_CAGE_CLOSED, 0,            HORDE_VAMPIRE_FODDER | HORDE_LEADER_CAPTIVE},

    // key thieves
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      14,     100,     0,          0,                  HORDE_MACHINE_THIEF},
    {MK_IMP,            0,      {0},                                    {{0}},                          15,     DEEPEST_LEVEL,  100, 0,      0,                  HORDE_MACHINE_THIEF},

    // sacrifice victims
    {MK_MONKEY,         0,      {0},                                    {{0}},                          1,      5,      100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          3,      10,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_OGRE,           0,      {0},                                    {{0}},                          7,      13,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_TROLL,          0,      {0},                                    {{0}},                          12,     19,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_WRAITH,         0,      {0},                                    {{0}},                          10,     17,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_NAGA,           0,      {0},                                    {{0}},                          13,     20,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_DAR_BLADEMASTER,0,      {0},                                    {{0}},                          10,     20,     100,        STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_GOLEM,          0,      {0},                                    {{0}},                          21,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_REVENANT,       0,      {0},                                    {{0}},                          21,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},
    {MK_TENTACLE_HORROR,0,      {0},                                    {{0}},                          21,     DEEPEST_LEVEL,100,  STATUE_INSTACRACK, 0,           HORDE_SACRIFICE_TARGET},

    // legendary allies
    {MK_UNICORN,        0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100, 0,     0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_IFRIT,          0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_PHOENIX_EGG,    0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},
    {MK_ANCIENT_SPIRIT, 0,      {0},                                    {{0}},                          1,      DEEPEST_LEVEL,  100,    0,      0,                  HORDE_MACHINE_LEGENDARY_ALLY | HORDE_ALLIED_WITH_PLAYER},

    // goblin warren
    {MK_GOBLIN,         0,      {0},                                    {{0}},                          1,      10,     100,     0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_CONJURER,0,      {0},                                    {{0}},                          1,      10,     60,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_TOTEM,   1,      {MK_GOBLIN},                            {{2,4,1}},                      5,      13,     100,        0,              MT_CAMP_AREA,   HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN,         3,      {MK_GOBLIN, MK_GOBLIN_MYSTIC, MK_JACKAL},{{2, 3, 1}, {1,2,1}, {1,2,1}}, 6,      12,     40,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_CONJURER,2,      {MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC}, {{0,1,1}, {1,1,1}},             7,      15,     40,      0,              0,              HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN_TOTEM,   4,      {MK_GOBLIN_TOTEM, MK_GOBLIN_CONJURER, MK_GOBLIN_MYSTIC, MK_GOBLIN}, {{1,2,1},{1,2,1},{1,2,1},{3,5,1}},10,17,80,0,MT_CAMP_AREA,  HORDE_MACHINE_GOBLIN_WARREN},
    {MK_GOBLIN,         1,      {MK_GOBLIN},                            {{1, 2, 1}},                    3,      7,      10,     0,              0,              HORDE_MACHINE_GOBLIN_WARREN | HORDE_LEADER_CAPTIVE},
};

const char *mainMenuTitle_Brogue =
"                                                                    \
                                                                    \
                                                                    \
                                                                    \
                                                                    \
                                                                    \
                                                                    \
########  ########      ######         ######  ####    ### #########\
 ##   ###  ##   ###   ##     ###     ##     ##  ##      #   ##     #\
 ##    ##  ##    ##  ##       ###   ##       #  ##      #   ##     #\
 ##    ##  ##    ##  #    #    ##   #        #  ##      #   ##      \
 ##    ##  ##    ## ##   ##     ## ##           ##      #   ##    # \
 ##   ##   ##   ##  ##   ###    ## ##           ##      #   ##    # \
 ######    ## ###   ##   ####   ## ##           ##      #   ####### \
 ##    ##  ##  ##   ##   ####   ## ##           ##      #   ##    # \
 ##     ## ##   ##  ##    ###   ## ##     ##### ##      #   ##    # \
 ##     ## ##   ##  ###    ##   ## ###      ##  ##      #   ##      \
 ##     ## ##    ##  ##    #    #   ##      ##  ##      #   ##      \
 ##     ## ##    ##  ###       ##   ###     ##  ###     #   ##     #\
 ##    ##  ##     ##  ###     ##     ###   ###   ###   #    ##     #\
########  ####    ###   ######         ####       #####    #########\
                          ##                                        \
                      ##########                                    \
                          ##                                        \
                          ##                                        \
                         ####                                       ";

// Brogue version: what the user sees in the menu and title
#define BROGUE_VERSION_STRING "CE " STRINGIFY(BROGUE_MAJOR) "." STRINGIFY(BROGUE_MINOR) "." STRINGIFY(BROGUE_PATCH) BROGUE_EXTRA_VERSION

// Recording version. Saved into recordings and save files made by this version.
// Cannot be longer than 16 chars
#define BROGUE_RECORDING_VERSION_STRING "CE " STRINGIFY(BROGUE_MAJOR) "." STRINGIFY(BROGUE_MINOR) "." STRINGIFY(BROGUE_PATCH)

/* Patch pattern. A scanf format string which matches an unsigned short. If this
matches against a recording version string, it defines a "patch version." During
normal play, rogue.patchVersion is set to the match of the game's recording
version above, or 0 if it doesn't match.
The game will only load a recording/save if either a) it has a patch version
which is equal or less than the patch version of the current game
(rogue.patchLevel is set to the recording's); or b) it doesn't match the version
strings, but they are equal (rogue.patchLevel is set to 0).
*/
#define BROGUE_PATCH_VERSION_PATTERN "CE " STRINGIFY(BROGUE_MAJOR) "." STRINGIFY(BROGUE_MINOR) ".%hu"

// Dungeon version - the earliest version where each seed creates a dungeon identical
// to that in the current version. Used in seed catalog output.
#define BROGUE_DUNGEON_VERSION_STRING "CE 1.11"

// Version shown in ./brogue --version
const char *brogueVersion = BROGUE_VERSION_STRING;

const gameConstants brogueGameConst = {
    .majorVersion = BROGUE_MAJOR,
    .minorVersion = BROGUE_MINOR,
    .patchVersion = BROGUE_PATCH,

    .versionString = BROGUE_VERSION_STRING,
    .dungeonVersionString = BROGUE_DUNGEON_VERSION_STRING,
    .patchVersionPattern = BROGUE_PATCH_VERSION_PATTERN,
    .recordingVersionString = BROGUE_RECORDING_VERSION_STRING,

    .variantName = "brogue",

    .deepestLevel = DEEPEST_LEVEL,
    .amuletLevel = AMULET_LEVEL,

    .depthAccelerator = 1,
    .minimumLavaLevel = 4,
    .minimumBrimstoneLevel = 17,
    .mutationsOccurAboveLevel = 10,

    .machinesPerLevelSuppressionMultiplier = 4,
    .machinesPerLevelSuppressionOffset = 2,
    .machinesPerLevelIncreaseFactor = 1,
    .maxLevelForBonusMachines = 2,

    .extraItemsPerLevel = 0,
    .goldAdjustmentStartDepth = 6,

    .playerTransferenceRatio = 20,
    .onHitHallucinateDuration = 20,
    .onHitWeakenDuration = 300,
    .onHitMercyHealPercent = 50,

    .weaponKillsToAutoID = 20,
    .armorDelayToAutoID = 1000,
    .ringDelayToAutoID = 1500,

    .fallDamageMin = 8,
    .fallDamageMax = 10,

    .numberAutogenerators = sizeof(autoGeneratorCatalog_Brogue) / sizeof(autoGenerator),
    .numberBoltKinds = sizeof(boltCatalog_Brogue) / sizeof(bolt),
    .numberBlueprints = sizeof(blueprintCatalog_Brogue) / sizeof(blueprint),
    .numberPotionKinds = sizeof(potionTable_Brogue) / sizeof(itemTable),
    .numberGoodPotionKinds = 8,
    .numberScrollKinds = sizeof(scrollTable_Brogue) / sizeof(itemTable),
    .numberGoodScrollKinds = 12,
    .numberWandKinds = sizeof(wandTable_Brogue) / sizeof(itemTable),
    .numberGoodWandKinds = 6,
    .numberCharmKinds = sizeof(charmTable_Brogue) / sizeof(itemTable),
    .numberMeteredItems = sizeof(meteredItemsGenerationTable_Brogue) / sizeof(meteredItemGenerationTable),
    .numberHordes = sizeof(hordeCatalog_Brogue) / sizeof(hordeType),
    .numberFeats = sizeof(featTable_Brogue) / sizeof(feat),
    .companionFeatRequiredXP = 10400, // about 13 depths

    .mainMenuTitleHeight = MENU_TITLE_HEIGHT,
    .mainMenuTitleWidth = MENU_TITLE_WIDTH
};

void initializeGameVariantBrogue() {

    // Game constants
    gameConst = &brogueGameConst;

    // Global variables
    autoGeneratorCatalog = autoGeneratorCatalog_Brogue;
    lumenstoneDistribution = lumenstoneDistribution_Brogue;
    itemGenerationProbabilities = itemGenerationProbabilities_Brogue;

    meteredItemsGenerationTable = meteredItemsGenerationTable_Brogue;

    potionTable = potionTable_Brogue;
    scrollTable = scrollTable_Brogue;
    wandTable = wandTable_Brogue;
    charmTable = charmTable_Brogue;
    featTable = featTable_Brogue;

    charmEffectTable = charmEffectTable_Brogue;

    boltCatalog = boltCatalog_Brogue;
    hordeCatalog = hordeCatalog_Brogue;
    blueprintCatalog = blueprintCatalog_Brogue;

    mainMenuTitle = mainMenuTitle_Brogue;
    levelFeelings = levelFeelings_Brogue;
}

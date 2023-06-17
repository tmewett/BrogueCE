/*
 *  Globals_Brogue.c
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
#include "IncludeGlobalsBase.h"
#include "IncludeGlobals.h"

#define AMULET_LEVEL            26          // how deep before the amulet appears
#define DEEPEST_LEVEL           40          // how deep the universe goes

#define NUMBER_AUTOGENERATORS 49


const color dominationColor =       {0,     0,      100,    80,     25,         0,          0,      true};
const color beckonColor_Brogue =           {10,    10,     10,     5,      5,          5,          50,     true};
const color empowermentColor =      {30,    100,    40,     25,     80,         25,         0,      true};
const color lightningColor_Brogue =        {100,   150,    500,    50,     50,         0,          50,     true};
const color fireBoltColor_Brogue =         {500,   150,    0,      45,     30,         0,          0,      true};
const color poisonColor_Brogue =           {0,     0,      0,      10,     50,         10,         0,      true};
const color forceFieldColor_Brogue =       {0,     25,     25,     0,      25,         25,         0,      true};
const color spectralBladeColor_Brogue =    {15,    15,     60,     0,      0,          70,         50,     true};
const color shieldingColor =        {150,   75,     0,      0,      50,         175,        0,      true};
const color dragonFireColor =       {500,   150,    0,      45,     30,         45,         0,      true};
const color centipedeColor_Brogue =        {75,    25,     85,     0,      0,          0,          0,      false};


const bolt boltCatalog_Brogue[NUMBER_BOLT_KINDS] = {
    {{0}},
    //name                      bolt description                ability description                         char    foreColor       backColor           boltEffect      magnitude       pathDF      targetDF    forbiddenMonsterFlags       flags
    {"teleportation spell",     "casts a teleport spell",       "can teleport other creatures",             0,      NULL,           &blue,              BE_TELEPORT,    10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"slowing spell",           "casts a slowing spell",        "can slow $HISHER enemies",                 0,      NULL,           &green,             BE_SLOW,        10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"polymorph spell",         "casts a polymorphism spell",   "can polymorph other creatures",            0,      NULL,           &purple,            BE_POLYMORPH,   10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"negation magic",          "casts a negation spell",       "can cast negation",                        0,      NULL,           &pink,              BE_NEGATION,    10,             0,          0,          0,                          (BF_TARGET_ENEMIES)},
    {"domination spell",        "casts a domination spell",     "can dominate other creatures",             0,      NULL,           &dominationColor,   BE_DOMINATION,  10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ENEMIES)},
    {"beckoning spell",         "casts a beckoning spell",      "can cast beckoning",                       0,      NULL,           &beckonColor_Brogue,       BE_BECKONING,   10,             0,          0,          MONST_IMMOBILE,             (BF_TARGET_ENEMIES)},
    {"spell of plenty",         "casts a spell of plenty",      "can duplicate other creatures",            0,      NULL,           &rainbow,           BE_PLENTY,      10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
    {"invisibility magic",      "casts invisibility magic",     "can turn creatures invisible",             0,      NULL,           &darkBlue,          BE_INVISIBILITY, 10,            0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"empowerment sorcery",     "casts empowerment",            "can cast empowerment",                     0,      NULL,           &empowermentColor,  BE_EMPOWERMENT, 10,             0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES | BF_NOT_LEARNABLE)},
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
    {"protection magic",        "casts protection",             "can cast protection",                      0,      NULL,           &shieldingColor,    BE_SHIELDING,   5,              0,          0,          MONST_INANIMATE,            (BF_TARGET_ALLIES)},
    {"spiderweb",               "launches a sticky web",        "can launch sticky webs",                   '*',    &white,         NULL,               BE_NONE,        10,             DF_WEB_SMALL, DF_WEB_LARGE, (MONST_IMMOBILE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"spark",                   "shoots a spark",               "can throw sparks of lightning",            0,      NULL,           &lightningColor_Brogue,    BE_DAMAGE,      1,              0,          0,          0,                          (BF_PASSES_THRU_CREATURES | BF_TARGET_ENEMIES | BF_ELECTRIC)},
    {"dragonfire",              "breathes a gout of white-hot flame", "can breathe gouts of white-hot flame", 0,    NULL,           &dragonFireColor,   BE_DAMAGE,      18,             DF_OBSIDIAN, 0,         MONST_IMMUNE_TO_FIRE,       (BF_TARGET_ENEMIES | BF_FIERY | BF_NOT_LEARNABLE)},
    {"arrow",                   "shoots an arrow",              "attacks from a distance",                  G_WEAPON,&gray,         NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"poisoned dart",           "fires a dart",                 "fires strength-sapping darts",             G_WEAPON,&centipedeColor_Brogue,NULL,              BE_ATTACK,      1,              0,          0,          0,                          (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE)},
    {"growing vines",           "releases carnivorous vines into the ground", "conjures carnivorous vines", G_GRASS,&tanColor,      NULL,               BE_NONE,        5,              DF_ANCIENT_SPIRIT_GRASS, DF_ANCIENT_SPIRIT_VINES, (MONST_INANIMATE | MONST_IMMUNE_TO_WEBS),   (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS)},
    {"whip",                    "whips",                        "wields a whip",                            '*',    &tanColor,      NULL,               BE_ATTACK,      1,              0,          0,          MONST_IMMUNE_TO_WEAPONS,    (BF_TARGET_ENEMIES | BF_NEVER_REFLECTS | BF_NOT_LEARNABLE | BF_DISPLAY_CHAR_ALONG_LENGTH)},
};

// Number of lumenstones on each level past amulet
const short lumenstoneDistribution_Brogue[DEEPEST_LEVEL - AMULET_LEVEL] = {3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1};

const autoGenerator autoGeneratorCatalog_Brogue[NUMBER_AUTOGENERATORS] = {
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

itemTable wandTable_Brogue[NUMBER_WAND_KINDS] = {
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

void initializeGameConst_Brogue() {
    gameConst.numberAutogenerators = NUMBER_AUTOGENERATORS;
    gameConst.deepestLevel = DEEPEST_LEVEL;
    gameConst.amuletLevel = AMULET_LEVEL;
}

void initializeGameGlobals_Brogue() {
    autoGeneratorCatalog = &autoGeneratorCatalog_Brogue;
    lumenstoneDistribution = &lumenstoneDistribution_Brogue;
    wandTable = &wandTable_Brogue;
    boltCatalog = &boltCatalog_Brogue;
}
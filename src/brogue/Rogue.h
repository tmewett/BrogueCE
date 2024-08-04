//
//  RogueMain.h
//  Brogue
//
//  Created by Brian Walker on 12/26/08.
//  Copyright 2012. All rights reserved.
//
//  This file is part of Brogue.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as
//  published by the Free Software Foundation, either version 3 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef ROGUE_H
#define ROGUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "PlatformDefines.h"

// unicode: comment this line to revert to ASCII
#define USE_UNICODE

// Brogue version number (for main engine)
#define BROGUE_MAJOR 1
#define BROGUE_MINOR 14
#define BROGUE_PATCH 0

// Expanding a macro as a string constant requires two levels of macros
#define _str(x) #x
#define STRINGIFY(x) _str(x)

// Macro to compare BROGUE_MAJOR.BROGUE_MINOR.patchVersion to a.b.c
#define BROGUE_VERSION_ATLEAST(a,b,c) (BROGUE_MAJOR != (a) ? BROGUE_MAJOR > (a) : BROGUE_MINOR != (b) ? BROGUE_MINOR > (b) : rogue.patchVersion >= (c))

#define DEBUG                           if (rogue.wizard)
#define MONSTERS_ENABLED                (!rogue.wizard || 1) // Quest room monsters can be generated regardless.
#define ITEMS_ENABLED                   (!rogue.wizard || 1)

#define D_BULLET_TIME                   (rogue.wizard && 0)
#define D_WORMHOLING                    (rogue.wizard && 1)
#define D_IMMORTAL                      (rogue.wizard && 1)

#define D_SAFETY_VISION                 (rogue.wizard && 0)
#define D_SCENT_VISION                  (rogue.wizard && 0)
#define D_DISABLE_BACKGROUND_COLORS     (rogue.wizard && 0)
#define D_OMNISCENCE                    (rogue.wizard && 0)

#define D_INSPECT_LEVELGEN              (rogue.wizard && 0)
#define D_INSPECT_MACHINES              (rogue.wizard && 0)

#define D_MESSAGE_ITEM_GENERATION       (rogue.wizard && 0)
#define D_MESSAGE_MACHINE_GENERATION    (rogue.wizard && 0)

// If enabled, runs a benchmark for the performance of repeatedly updating the screen at the start of the game.
// #define SCREEN_UPDATE_BENCHMARK

// If enabled, logs the light values when '~' is pressed.
// #define LOG_LIGHTS

// set to false to allow multiple loads from the same saved file:
#define DELETE_SAVE_FILE_AFTER_LOADING  true

// set to false to disable references to keystrokes (e.g. for a tablet port)
#define KEYBOARD_LABELS true

//#define BROGUE_ASSERTS        // introduces several assert()s -- useful to find certain array overruns and other bugs
//#define AUDIT_RNG             // VERY slow, but sometimes necessary to debug out-of-sync recording errors
//#define GENERATE_FONT_FILES   // Displays font in grid upon startup, which can be screen-captured into font files for PC.

#ifdef BROGUE_ASSERTS
#include <assert.h>
#define brogueAssert(x)         assert(x)
#else
#define brogueAssert(x)
#endif

#define boolean                 char

#define false                   0
#define true                    1

#define Fl(N)                   ((unsigned long) 1 << (N))

typedef long long fixpt;
#define FP_BASE 16 // Don't change this without recalculating all of the power tables throughout the code!
#define FP_FACTOR (1LL << FP_BASE)
#define FP_MUL(x, y)  ((x) * (y) / FP_FACTOR)
#define FP_DIV(x, y)  ((x) * FP_FACTOR / (y))

// recording and save filenames
#define LAST_GAME_NAME          "LastGame"
#define LAST_RECORDING_NAME     "LastRecording"
#define RECORDING_SUFFIX        ".broguerec"
#define GAME_SUFFIX             ".broguesave"
#define ANNOTATION_SUFFIX       ".txt"
#define RNG_LOG                 "RNGLog.txt"
#define SCREENSHOT_SUFFIX       ".png"

#define BROGUE_FILENAME_MAX     (min(1024*4, FILENAME_MAX))
#define ERROR_MESSAGE_LENGTH    100

// Date format used when listing recordings and high scores
#define DATE_FORMAT             "%Y-%m-%d" // see strftime() documentation

#define MESSAGE_LINES           3
#define MESSAGE_ARCHIVE_VIEW_LINES ROWS
#define MESSAGE_ARCHIVE_LINES   (MESSAGE_ARCHIVE_VIEW_LINES*10)
#define MESSAGE_ARCHIVE_ENTRIES (MESSAGE_ARCHIVE_LINES*4)
#define MAX_MESSAGE_REPEATS     100

// Size of the entire terminal window. These need to be hard-coded here and in Viewport.h
#define COLS                    100
#define ROWS                    (31 + MESSAGE_LINES)

// Maximum string size supported by various functions like printTextBox, wrapText,
// printStringWithWrapping, and breakUpLongWordsIn
#define TEXT_MAX_LENGTH (COLS * ROWS * 2)

// Returns the sign of the input:
// - if (x == 0)  ===> returns 0
// - if (x >= 1)  ===> returns +1
// - if (x <= -1) ===> returns -1
static inline int signum(int x) {
    if (x == 0) {
        return 0;
    }
    return x > 0 ? 1 : -1;
}

// A location within the dungeon.
// Typically, 0 <= x < DCOLS and 0 <= y < DROWS,
// but occasionally coordinates are used which point outside of this region.
typedef struct pos {
    short x;
    short y;
} pos;

#define INVALID_POS ((pos) { .x = -1, .y = -1 })

static inline boolean posEq(pos a, pos b) {
    return a.x == b.x && a.y == b.y;
}

// A location within the window.
// Convert between `windowpos` and `pos` with `mapToWindow` and
// `windowToMap`
typedef struct windowpos {
    short window_x;
    short window_y;
} windowpos;

// Size of the portion of the terminal window devoted to displaying the dungeon:
#define DCOLS                   (COLS - STAT_BAR_WIDTH - 1) // n columns on the left for the sidebar;
                                                            // one column to separate the sidebar from the map.
#define DROWS                   (ROWS - MESSAGE_LINES - 2)  // n lines at the top for messages;
                                                            // one line at the bottom for flavor text;
                                                            // another line at the bottom for the menu bar.

#define STAT_BAR_WIDTH          20          // number of characters in the stats bar to the left of the map

#define LOS_SLOPE_GRANULARITY   32768       // how finely we divide up the squares when calculating slope;
                                            // higher numbers mean fewer artifacts but more memory and processing
#define INTERFACE_OPACITY       95

#define LIGHT_SMOOTHING_THRESHOLD 150       // light components higher than this magnitude will be toned down a little

#define MAX_BOLT_LENGTH         DCOLS*10

#define VISIBILITY_THRESHOLD    50          // how bright cumulative light has to be before the cell is marked visible

#define MACHINES_BUFFER_LENGTH  200

#define INPUT_RECORD_BUFFER     1000        // the threshold size before flushing the record buffer to disk
#define INPUT_RECORD_BUFFER_MAX_SIZE 1100   // the maximum size of the record buffer
#define DEFAULT_PLAYBACK_DELAY  50

#define HIGH_SCORES_COUNT       30

// color escapes
#define COLOR_ESCAPE            25
#define COLOR_VALUE_INTERCEPT   25

// variants supported in this code base
enum gameVariant {
    VARIANT_BROGUE,
    VARIANT_RAPID_BROGUE,
    NUMBER_VARIANTS
};

// display characters:

enum displayGlyph {
    G_UP_ARROW = 128,
    G_DOWN_ARROW,
    G_POTION,
    G_GRASS,
    G_WALL,
    G_DEMON,
    G_OPEN_DOOR,
    G_GOLD,
    G_CLOSED_DOOR,
    G_RUBBLE,
    G_KEY,
    G_BOG,
    G_CHAIN_TOP_LEFT,
    G_CHAIN_BOTTOM_RIGHT,
    G_CHAIN_TOP_RIGHT,
    G_CHAIN_BOTTOM_LEFT,
    G_CHAIN_TOP,
    G_CHAIN_BOTTOM,
    G_CHAIN_LEFT,
    G_CHAIN_RIGHT,
    G_FOOD,
    G_UP_STAIRS,
    G_VENT,
    G_DOWN_STAIRS,
    G_PLAYER,
    G_BOG_MONSTER,
    G_CENTAUR,
    G_DRAGON,
    G_FLAMEDANCER,
    G_GOLEM,
    G_TENTACLE_HORROR,
    G_IFRIT,
    G_JELLY,
    G_KRAKEN,
    G_LICH,
    G_NAGA,
    G_OGRE,
    G_PHANTOM,
    G_REVENANT,
    G_SALAMANDER,
    G_TROLL,
    G_UNDERWORM,
    G_VAMPIRE,
    G_WRAITH,
    G_ZOMBIE,
    G_ARMOR,
    G_STAFF,
    G_WEB,
    G_MOUND,
    G_BLOAT,
    G_CENTIPEDE,
    G_DAR_BLADEMASTER,
    G_EEL,
    G_FURY,
    G_GOBLIN,
    G_IMP,
    G_JACKAL,
    G_KOBOLD,
    G_MONKEY,
    G_PIXIE,
    G_RAT,
    G_SPIDER,
    G_TOAD,
    G_BAT,
    G_WISP,
    G_PHOENIX,
    G_ALTAR,
    G_LIQUID,
    G_FLOOR,
    G_CHASM,
    G_TRAP,
    G_FIRE,
    G_FOLIAGE,
    G_AMULET,
    G_SCROLL,
    G_RING,
    G_WEAPON,
    G_TURRET,
    G_TOTEM,
    G_GOOD_MAGIC,
    G_BAD_MAGIC,
    G_DOORWAY,
    G_CHARM,
    G_WALL_TOP,
    G_DAR_PRIESTESS,
    G_DAR_BATTLEMAGE,
    G_GOBLIN_MAGIC,
    G_GOBLIN_CHIEFTAN,
    G_OGRE_MAGIC,
    G_GUARDIAN,
    G_WINGED_GUARDIAN,
    G_EGG,
    G_WARDEN,
    G_DEWAR,
    G_ANCIENT_SPIRIT,
    G_LEVER,
    G_LEVER_PULLED,
    G_BLOODWORT_STALK,
    G_FLOOR_ALT,
    G_UNICORN,
    G_GEM,
    G_WAND,
    G_GRANITE,
    G_CARPET,
    G_CLOSED_IRON_DOOR,
    G_OPEN_IRON_DOOR,
    G_TORCH,
    G_CRYSTAL,
    G_PORTCULLIS,
    G_BARRICADE,
    G_STATUE,
    G_CRACKED_STATUE,
    G_CLOSED_CAGE,
    G_OPEN_CAGE,
    G_PEDESTAL,
    G_CLOSED_COFFIN,
    G_OPEN_COFFIN,
    G_MAGIC_GLYPH,
    G_BRIDGE,
    G_BONES,
    G_ELECTRIC_CRYSTAL,
    G_ASHES,
    G_BEDROLL,
    G_BLOODWORT_POD,
    G_VINE,
    G_NET,
    G_LICHEN,
    G_PIPES,
    G_SAC_ALTAR,
    G_ORB_ALTAR,
    G_LEFT_TRIANGLE
};

enum graphicsModes {
    TEXT_GRAPHICS,
    TILES_GRAPHICS,
    HYBRID_GRAPHICS, // text for items and creatures, tiles for environment
};

enum eventTypes {
    KEYSTROKE,
    MOUSE_UP,
    MOUSE_DOWN,
    RIGHT_MOUSE_DOWN,
    RIGHT_MOUSE_UP,
    MOUSE_ENTERED_CELL,
    RNG_CHECK,
    SAVED_GAME_LOADED,
    END_OF_RECORDING,
    EVENT_ERROR,
    NUMBER_OF_EVENT_TYPES, // unused
};

enum notificationEventTypes {
	GAMEOVER_QUIT,
	GAMEOVER_DEATH,
	GAMEOVER_VICTORY,
	GAMEOVER_SUPERVICTORY,
	GAMEOVER_RECORDING
};

typedef struct rogueEvent {
    enum eventTypes eventType;
    signed long param1;
    signed long param2;
    boolean controlKey;
    boolean shiftKey;
} rogueEvent;

typedef struct rogueHighScoresEntry {
    signed long score;
    char date[100];
    char description[DCOLS];
} rogueHighScoresEntry;

typedef struct rogueRun {
    uint64_t seed;
    long dateNumber;
    char result[DCOLS];
    char killedBy[DCOLS];
    int gold;
    int lumenstones;
    int score;
    int turns;
    int deepestLevel;
    struct rogueRun *nextRun;
} rogueRun;

typedef struct fileEntry {
    char *path;
    struct tm date;
} fileEntry;

enum RNGs {
    RNG_SUBSTANTIVE,
    RNG_COSMETIC,
    NUMBER_OF_RNGS,
};

enum displayDetailValues {
    DV_UNLIT = 0,
    DV_LIT,
    DV_DARK,
};

enum directions {
    NO_DIRECTION    = -1,
    // Cardinal directions; must be 0-3:
    UP              = 0,
    DOWN            = 1,
    LEFT            = 2,
    RIGHT           = 3,
    // Secondary directions; must be 4-7:
    UPLEFT          = 4,
    DOWNLEFT        = 5,
    UPRIGHT         = 6,
    DOWNRIGHT       = 7,

    DIRECTION_COUNT = 8,
};

enum textEntryTypes {
    TEXT_INPUT_NORMAL = 0,
    TEXT_INPUT_FILENAME,
    TEXT_INPUT_NUMBERS,
    TEXT_INPUT_TYPES,
};

#define NUMBER_DYNAMIC_COLORS   6

enum tileType {
    NOTHING = 0,
    GRANITE,
    FLOOR,
    FLOOR_FLOODABLE,
    CARPET,
    MARBLE_FLOOR,
    WALL,
    DOOR,
    OPEN_DOOR,
    SECRET_DOOR,
    LOCKED_DOOR,
    OPEN_IRON_DOOR_INERT,
    DOWN_STAIRS,
    UP_STAIRS,
    DUNGEON_EXIT,
    DUNGEON_PORTAL,
    TORCH_WALL, // wall lit with a torch
    CRYSTAL_WALL,
    PORTCULLIS_CLOSED,
    PORTCULLIS_DORMANT,
    WOODEN_BARRICADE,
    PILOT_LIGHT_DORMANT,
    PILOT_LIGHT,
    HAUNTED_TORCH_DORMANT,
    HAUNTED_TORCH_TRANSITIONING,
    HAUNTED_TORCH,
    WALL_LEVER_HIDDEN,
    WALL_LEVER,
    WALL_LEVER_PULLED,
    WALL_LEVER_HIDDEN_DORMANT,
    STATUE_INERT,
    STATUE_DORMANT,
    STATUE_CRACKING,
    STATUE_INSTACRACK,
    PORTAL,
    TURRET_DORMANT,
    WALL_MONSTER_DORMANT,
    DARK_FLOOR_DORMANT,
    DARK_FLOOR_DARKENING,
    DARK_FLOOR,
    MACHINE_TRIGGER_FLOOR,
    ALTAR_INERT,
    ALTAR_KEYHOLE,
    ALTAR_CAGE_OPEN,
    ALTAR_CAGE_CLOSED,
    ALTAR_SWITCH,
    ALTAR_SWITCH_RETRACTING,
    ALTAR_CAGE_RETRACTABLE,
    PEDESTAL,
    MONSTER_CAGE_OPEN,
    MONSTER_CAGE_CLOSED,
    COFFIN_CLOSED,
    COFFIN_OPEN,

    GAS_TRAP_POISON_HIDDEN,
    GAS_TRAP_POISON,
    TRAP_DOOR_HIDDEN,
    TRAP_DOOR,
    GAS_TRAP_PARALYSIS_HIDDEN,
    GAS_TRAP_PARALYSIS,
    MACHINE_PARALYSIS_VENT_HIDDEN,
    MACHINE_PARALYSIS_VENT,
    GAS_TRAP_CONFUSION_HIDDEN,
    GAS_TRAP_CONFUSION,
    FLAMETHROWER_HIDDEN,
    FLAMETHROWER,
    FLOOD_TRAP_HIDDEN,
    FLOOD_TRAP,
    NET_TRAP_HIDDEN,
    NET_TRAP,
    ALARM_TRAP_HIDDEN,
    ALARM_TRAP,
    MACHINE_POISON_GAS_VENT_HIDDEN,
    MACHINE_POISON_GAS_VENT_DORMANT,
    MACHINE_POISON_GAS_VENT,
    MACHINE_METHANE_VENT_HIDDEN,
    MACHINE_METHANE_VENT_DORMANT,
    MACHINE_METHANE_VENT,
    STEAM_VENT,
    MACHINE_PRESSURE_PLATE,
    MACHINE_PRESSURE_PLATE_USED,
    MACHINE_GLYPH,
    MACHINE_GLYPH_INACTIVE,
    DEWAR_CAUSTIC_GAS,
    DEWAR_CONFUSION_GAS,
    DEWAR_PARALYSIS_GAS,
    DEWAR_METHANE_GAS,

    DEEP_WATER,
    SHALLOW_WATER,
    MUD,
    CHASM,
    CHASM_EDGE,
    MACHINE_COLLAPSE_EDGE_DORMANT,
    MACHINE_COLLAPSE_EDGE_SPREADING,
    LAVA,
    LAVA_RETRACTABLE,
    LAVA_RETRACTING,
    SUNLIGHT_POOL,
    DARKNESS_PATCH,
    ACTIVE_BRIMSTONE,
    INERT_BRIMSTONE,
    OBSIDIAN,
    BRIDGE,
    BRIDGE_FALLING,
    BRIDGE_EDGE,
    STONE_BRIDGE,
    MACHINE_FLOOD_WATER_DORMANT,
    MACHINE_FLOOD_WATER_SPREADING,
    MACHINE_MUD_DORMANT,
    ICE_DEEP,
    ICE_DEEP_MELT,
    ICE_SHALLOW,
    ICE_SHALLOW_MELT,

    HOLE,
    HOLE_GLOW,
    HOLE_EDGE,
    FLOOD_WATER_DEEP,
    FLOOD_WATER_SHALLOW,
    GRASS,
    DEAD_GRASS,
    GRAY_FUNGUS,
    LUMINESCENT_FUNGUS,
    LICHEN,
    HAY,
    RED_BLOOD,
    GREEN_BLOOD,
    PURPLE_BLOOD,
    ACID_SPLATTER,
    VOMIT,
    URINE,
    UNICORN_POOP,
    WORM_BLOOD,
    ASH,
    BURNED_CARPET,
    PUDDLE,
    BONES,
    RUBBLE,
    JUNK,
    BROKEN_GLASS,
    ECTOPLASM,
    EMBERS,
    SPIDERWEB,
    NETTING,
    FOLIAGE,
    DEAD_FOLIAGE,
    TRAMPLED_FOLIAGE,
    FUNGUS_FOREST,
    TRAMPLED_FUNGUS_FOREST,
    FORCEFIELD,
    FORCEFIELD_MELT,
    SACRED_GLYPH,
    MANACLE_TL,
    MANACLE_BR,
    MANACLE_TR,
    MANACLE_BL,
    MANACLE_T,
    MANACLE_B,
    MANACLE_L,
    MANACLE_R,
    PORTAL_LIGHT,
    GUARDIAN_GLOW,

    PLAIN_FIRE,
    BRIMSTONE_FIRE,
    FLAMEDANCER_FIRE,
    GAS_FIRE,
    GAS_EXPLOSION,
    DART_EXPLOSION,
    ITEM_FIRE,
    CREATURE_FIRE,

    POISON_GAS,
    CONFUSION_GAS,
    ROT_GAS,
    STENCH_SMOKE_GAS,
    PARALYSIS_GAS,
    METHANE_GAS,
    STEAM,
    DARKNESS_CLOUD,
    HEALING_CLOUD,

    BLOODFLOWER_STALK,
    BLOODFLOWER_POD,

    HAVEN_BEDROLL,

    DEEP_WATER_ALGAE_WELL,
    DEEP_WATER_ALGAE_1,
    DEEP_WATER_ALGAE_2,

    ANCIENT_SPIRIT_VINES,
    ANCIENT_SPIRIT_GRASS,

    AMULET_SWITCH,

    COMMUTATION_ALTAR,
    COMMUTATION_ALTAR_INERT,
    PIPE_GLOWING,
    PIPE_INERT,

    RESURRECTION_ALTAR,
    RESURRECTION_ALTAR_INERT,
    MACHINE_TRIGGER_FLOOR_REPEATING,

    SACRIFICE_ALTAR_DORMANT,
    SACRIFICE_ALTAR,
    SACRIFICE_LAVA,
    SACRIFICE_CAGE_DORMANT,
    DEMONIC_STATUE,

    STATUE_INERT_DOORWAY,
    STATUE_DORMANT_DOORWAY,

    CHASM_WITH_HIDDEN_BRIDGE,
    CHASM_WITH_HIDDEN_BRIDGE_ACTIVE,
    MACHINE_CHASM_EDGE,

    RAT_TRAP_WALL_DORMANT,
    RAT_TRAP_WALL_CRACKING,

    ELECTRIC_CRYSTAL_OFF,
    ELECTRIC_CRYSTAL_ON,
    TURRET_LEVER,

    WORM_TUNNEL_MARKER_DORMANT,
    WORM_TUNNEL_MARKER_ACTIVE,
    WORM_TUNNEL_OUTER_WALL,

    BRAZIER,

    MUD_FLOOR,
    MUD_WALL,
    MUD_DOORWAY,

    NUMBER_TILETYPES,
};

enum lightType {
    NO_LIGHT,
    MINERS_LIGHT,
    BURNING_CREATURE_LIGHT,
    WISP_LIGHT,
    SALAMANDER_LIGHT,
    IMP_LIGHT,
    PIXIE_LIGHT,
    LICH_LIGHT,
    FLAMEDANCER_LIGHT,
    SENTINEL_LIGHT,
    UNICORN_LIGHT,
    IFRIT_LIGHT,
    PHOENIX_LIGHT,
    PHOENIX_EGG_LIGHT,
    YENDOR_LIGHT,
    SPECTRAL_BLADE_LIGHT,
    SPECTRAL_IMAGE_LIGHT,
    SPARK_TURRET_LIGHT,
    EXPLOSIVE_BLOAT_LIGHT,
    BOLT_LIGHT_SOURCE,
    TELEPATHY_LIGHT,
    SACRIFICE_MARK_LIGHT,

    SCROLL_PROTECTION_LIGHT,
    SCROLL_ENCHANTMENT_LIGHT,
    POTION_STRENGTH_LIGHT,
    EMPOWERMENT_LIGHT,
    GENERIC_FLASH_LIGHT,
    FALLEN_TORCH_FLASH_LIGHT,
    SUMMONING_FLASH_LIGHT,
    EXPLOSION_FLARE_LIGHT,
    QUIETUS_FLARE_LIGHT,
    SLAYING_FLARE_LIGHT,
    CHARGE_FLASH_LIGHT,

    TORCH_LIGHT,
    LAVA_LIGHT,
    SUN_LIGHT,
    DARKNESS_PATCH_LIGHT,
    FUNGUS_LIGHT,
    FUNGUS_FOREST_LIGHT,
    LUMINESCENT_ALGAE_BLUE_LIGHT,
    LUMINESCENT_ALGAE_GREEN_LIGHT,
    ECTOPLASM_LIGHT,
    UNICORN_POOP_LIGHT,
    EMBER_LIGHT,
    FIRE_LIGHT,
    BRIMSTONE_FIRE_LIGHT,
    EXPLOSION_LIGHT,
    INCENDIARY_DART_LIGHT,
    PORTAL_ACTIVATE_LIGHT,
    CONFUSION_GAS_LIGHT,
    DARKNESS_CLOUD_LIGHT,
    FORCEFIELD_LIGHT,
    CRYSTAL_WALL_LIGHT,
    CANDLE_LIGHT,
    HAUNTED_TORCH_LIGHT,
    GLYPH_LIGHT_DIM,
    GLYPH_LIGHT_BRIGHT,
    SACRED_GLYPH_LIGHT,
    DESCENT_LIGHT,
    DEMONIC_STATUE_LIGHT,
    NUMBER_LIGHT_KINDS
};

#define NUMBER_ITEM_CATEGORIES  13

// Item categories
enum itemCategory {
    FOOD                = Fl(0),
    WEAPON              = Fl(1),
    ARMOR               = Fl(2),
    POTION              = Fl(3),
    SCROLL              = Fl(4),
    STAFF               = Fl(5),
    WAND                = Fl(6),
    RING                = Fl(7),
    CHARM               = Fl(8),
    GOLD                = Fl(9),
    AMULET              = Fl(10),
    GEM                 = Fl(11),
    KEY                 = Fl(12),

    // Categories where the kinds have intrinsic magic polarity; i.e. each kind
    // has a certain polarity (with positive enchant) which doesn't depend on
    // the specific item. NOTE: Rings are considered to be naturally good, but
    // may be bad when negatively enchanted. We also assume that none of the
    // kinds in these categories have neutral polarity.
    HAS_INTRINSIC_POLARITY = (POTION | SCROLL | RING | WAND | STAFF),

    CAN_BE_DETECTED     = (WEAPON | ARMOR | POTION | SCROLL | RING | CHARM | WAND | STAFF | AMULET),
    CAN_BE_ENCHANTED    = (WEAPON | ARMOR | RING | CHARM | WAND | STAFF),
    PRENAMED_CATEGORY   = (FOOD | GOLD | AMULET | GEM | KEY),
    NEVER_IDENTIFIABLE  = (FOOD | CHARM | GOLD | AMULET | GEM | KEY),
    CAN_BE_SWAPPED      = (WEAPON | ARMOR | STAFF | CHARM | RING),
    ALL_ITEMS           = (FOOD|POTION|WEAPON|ARMOR|STAFF|WAND|SCROLL|RING|CHARM|GOLD|AMULET|GEM|KEY),
};

enum keyKind {
    KEY_DOOR,
    KEY_CAGE,
    KEY_PORTAL,
    NUMBER_KEY_TYPES
};

enum foodKind {
    RATION,
    FRUIT,
    NUMBER_FOOD_KINDS
};

enum potionKind {
    POTION_LIFE,
    POTION_STRENGTH,
    POTION_TELEPATHY,
    POTION_LEVITATION,
    POTION_DETECT_MAGIC,
    POTION_HASTE_SELF,
    POTION_FIRE_IMMUNITY,
    POTION_INVISIBILITY,
    POTION_POISON,
    POTION_PARALYSIS,
    POTION_HALLUCINATION,
    POTION_CONFUSION,
    POTION_INCINERATION,
    POTION_DARKNESS,
    POTION_DESCENT,
    POTION_LICHEN,
};

enum weaponKind {
    DAGGER,
    SWORD,
    BROADSWORD,

    WHIP,
    RAPIER,
    FLAIL,

    MACE,
    HAMMER,

    SPEAR,
    PIKE,

    AXE,
    WAR_AXE,

    DART,
    INCENDIARY_DART,
    JAVELIN,
    NUMBER_WEAPON_KINDS
};

enum weaponEnchants {
    W_SPEED,
    W_QUIETUS,
    W_PARALYSIS,
    W_MULTIPLICITY,
    W_SLOWING,
    W_CONFUSION,
    W_FORCE,
    W_SLAYING,
    W_MERCY,
    NUMBER_GOOD_WEAPON_ENCHANT_KINDS = W_MERCY,
    W_PLENTY,
    NUMBER_WEAPON_RUNIC_KINDS
};

enum armorKind {
    LEATHER_ARMOR,
    SCALE_MAIL,
    CHAIN_MAIL,
    BANDED_MAIL,
    SPLINT_MAIL,
    PLATE_MAIL,
    NUMBER_ARMOR_KINDS
};

enum armorEnchants {
    A_MULTIPLICITY,
    A_MUTUALITY,
    A_ABSORPTION,
    A_REPRISAL,
    A_IMMUNITY,
    A_REFLECTION,
    A_RESPIRATION,
    A_DAMPENING,
    A_BURDEN,
    NUMBER_GOOD_ARMOR_ENCHANT_KINDS = A_BURDEN,
    A_VULNERABILITY,
    A_IMMOLATION,
    NUMBER_ARMOR_ENCHANT_KINDS,
};

enum wandKind {
    WAND_TELEPORT,
    WAND_SLOW,
    WAND_POLYMORPH,
    WAND_NEGATION,
    WAND_DOMINATION,
    WAND_BECKONING,
    WAND_PLENTY,
    WAND_INVISIBILITY,
    WAND_EMPOWERMENT
};

enum staffKind {
    STAFF_LIGHTNING,
    STAFF_FIRE,
    STAFF_POISON,
    STAFF_TUNNELING,
    STAFF_BLINKING,
    STAFF_ENTRANCEMENT,
    STAFF_OBSTRUCTION,
    STAFF_DISCORD,
    STAFF_CONJURATION,
    STAFF_HEALING,
    NUMBER_GOOD_STAFF_KINDS = STAFF_HEALING,
    STAFF_HASTE,
    STAFF_PROTECTION,
    NUMBER_STAFF_KINDS
};

// these must be wand bolts, in order, and then staff bolts, in order:
enum boltType {
    BOLT_NONE = 0,
    BOLT_TELEPORT,
    BOLT_SLOW,
    BOLT_POLYMORPH,
    BOLT_NEGATION,
    BOLT_DOMINATION,
    BOLT_BECKONING,
    BOLT_PLENTY,
    BOLT_INVISIBILITY,
    BOLT_EMPOWERMENT,
    BOLT_LIGHTNING,
    BOLT_FIRE,
    BOLT_POISON,
    BOLT_TUNNELING,
    BOLT_BLINKING,
    BOLT_ENTRANCEMENT,
    BOLT_OBSTRUCTION,
    BOLT_DISCORD,
    BOLT_CONJURATION,
    BOLT_HEALING,
    BOLT_HASTE,
    BOLT_SLOW_2,
    BOLT_SHIELDING,
    BOLT_SPIDERWEB,
    BOLT_SPARK,
    BOLT_DRAGONFIRE,
    BOLT_DISTANCE_ATTACK,
    BOLT_POISON_DART,
    BOLT_ANCIENT_SPIRIT_VINES,
    BOLT_WHIP
};

enum ringKind {
    RING_CLAIRVOYANCE,
    RING_STEALTH,
    RING_REGENERATION,
    RING_TRANSFERENCE,
    RING_LIGHT,
    RING_AWARENESS,
    RING_WISDOM,
    RING_REAPING,
    NUMBER_RING_KINDS
};

enum charmKind {
    CHARM_HEALTH,
    CHARM_PROTECTION,
    CHARM_HASTE,
    CHARM_FIRE_IMMUNITY,
    CHARM_INVISIBILITY,
    CHARM_TELEPATHY,
    CHARM_LEVITATION,
    CHARM_SHATTERING,
    CHARM_GUARDIAN,
    CHARM_TELEPORTATION,
    CHARM_RECHARGING,
    CHARM_NEGATION
};

enum scrollKind {
    SCROLL_ENCHANTING,
    SCROLL_IDENTIFY,
    SCROLL_TELEPORT,
    SCROLL_REMOVE_CURSE,
    SCROLL_RECHARGING,
    SCROLL_PROTECT_ARMOR,
    SCROLL_PROTECT_WEAPON,
    SCROLL_SANCTUARY,
    SCROLL_MAGIC_MAPPING,
    SCROLL_NEGATION,
    SCROLL_SHATTERING,
    SCROLL_DISCORD,
    SCROLL_AGGRAVATE_MONSTER,
    SCROLL_SUMMON_MONSTER,
};

typedef struct meteredItem {
    int frequency;
    int numberSpawned;
} meteredItem;

#define MAX_PACK_ITEMS              26

enum monsterTypes {
    MK_YOU,
    MK_RAT,
    MK_KOBOLD,
    MK_JACKAL,
    MK_EEL,
    MK_MONKEY,
    MK_BLOAT,
    MK_PIT_BLOAT,
    MK_GOBLIN,
    MK_GOBLIN_CONJURER,
    MK_GOBLIN_MYSTIC,
    MK_GOBLIN_TOTEM,
    MK_PINK_JELLY,
    MK_TOAD,
    MK_VAMPIRE_BAT,
    MK_ARROW_TURRET,
    MK_ACID_MOUND,
    MK_CENTIPEDE,
    MK_OGRE,
    MK_BOG_MONSTER,
    MK_OGRE_TOTEM,
    MK_SPIDER,
    MK_SPARK_TURRET,
    MK_WILL_O_THE_WISP,
    MK_WRAITH,
    MK_ZOMBIE,
    MK_TROLL,
    MK_OGRE_SHAMAN,
    MK_NAGA,
    MK_SALAMANDER,
    MK_EXPLOSIVE_BLOAT,
    MK_DAR_BLADEMASTER,
    MK_DAR_PRIESTESS,
    MK_DAR_BATTLEMAGE,
    MK_ACID_JELLY,
    MK_CENTAUR,
    MK_UNDERWORM,
    MK_SENTINEL,
    MK_DART_TURRET,
    MK_KRAKEN,
    MK_LICH,
    MK_PHYLACTERY,
    MK_PIXIE,
    MK_PHANTOM,
    MK_FLAME_TURRET,
    MK_IMP,
    MK_FURY,
    MK_REVENANT,
    MK_TENTACLE_HORROR,
    MK_GOLEM,
    MK_DRAGON,

    MK_GOBLIN_CHIEFTAN,
    MK_BLACK_JELLY,
    MK_VAMPIRE,
    MK_FLAMEDANCER,

    MK_SPECTRAL_BLADE,
    MK_SPECTRAL_IMAGE,
    MK_GUARDIAN,
    MK_WINGED_GUARDIAN,
    MK_CHARM_GUARDIAN,
    MK_WARDEN_OF_YENDOR,
    MK_ELDRITCH_TOTEM,
    MK_MIRRORED_TOTEM,

    MK_UNICORN,
    MK_IFRIT,
    MK_PHOENIX,
    MK_PHOENIX_EGG,
    MK_ANCIENT_SPIRIT,

    NUMBER_MONSTER_KINDS
};

#define NUMBER_MUTATORS             8

#define MONSTER_CLASS_COUNT         15

// flavors

#define NUMBER_ITEM_TITLES          14
#define NUMBER_ITEM_COLORS          21
#define NUMBER_TITLE_PHONEMES       21
#define NUMBER_ITEM_WOODS           21
#define NUMBER_POTION_DESCRIPTIONS  18
#define NUMBER_ITEM_METALS          12
#define NUMBER_ITEM_GEMS            18

// Dungeon flags
enum tileFlags {
    DISCOVERED                  = Fl(0),
    VISIBLE                     = Fl(1),    // cell has sufficient light and is in field of view, ready to draw.
    HAS_PLAYER                  = Fl(2),
    HAS_MONSTER                 = Fl(3),
    HAS_DORMANT_MONSTER         = Fl(4),    // hidden monster on the square
    HAS_ITEM                    = Fl(5),
    IN_FIELD_OF_VIEW            = Fl(6),    // player has unobstructed line of sight whether or not there is enough light
    WAS_VISIBLE                 = Fl(7),
    HAS_STAIRS                  = Fl(8),
    SEARCHED_FROM_HERE          = Fl(9),    // player already auto-searched here; can't auto-search here again
    IS_IN_SHADOW                = Fl(10),   // so that a player gains an automatic stealth bonus
    MAGIC_MAPPED                = Fl(11),
    ITEM_DETECTED               = Fl(12),
    CLAIRVOYANT_VISIBLE         = Fl(13),
    WAS_CLAIRVOYANT_VISIBLE     = Fl(14),
    CLAIRVOYANT_DARKENED        = Fl(15),   // magical blindness from a cursed ring of clairvoyance
    CAUGHT_FIRE_THIS_TURN       = Fl(16),   // so that fire does not spread asymmetrically
    PRESSURE_PLATE_DEPRESSED    = Fl(17),   // so that traps do not trigger repeatedly while you stand on them
    STABLE_MEMORY               = Fl(18),   // redraws will be pulled from the memory array, not recalculated
    KNOWN_TO_BE_TRAP_FREE       = Fl(19),   // keep track of where the player has stepped or watched monsters step as he knows no traps are there
    IS_IN_PATH                  = Fl(20),   // the yellow trail leading to the cursor
    IN_LOOP                     = Fl(21),   // this cell is part of a terrain loop
    IS_CHOKEPOINT               = Fl(22),   // if this cell is blocked, part of the map will be rendered inaccessible
    IS_GATE_SITE                = Fl(23),   // consider placing a locked door here
    IS_IN_ROOM_MACHINE          = Fl(24),
    IS_IN_AREA_MACHINE          = Fl(25),
    IS_POWERED                  = Fl(26),   // has been activated by machine power this turn (flag can probably be eliminated if needed)
    IMPREGNABLE                 = Fl(27),   // no tunneling allowed!
    TERRAIN_COLORS_DANCING      = Fl(28),   // colors here will sparkle when the game is idle
    TELEPATHIC_VISIBLE          = Fl(29),   // potions of telepathy let you see through other creatures' eyes
    WAS_TELEPATHIC_VISIBLE      = Fl(30),   // potions of telepathy let you see through other creatures' eyes

    IS_IN_MACHINE               = (IS_IN_ROOM_MACHINE | IS_IN_AREA_MACHINE),    // sacred ground; don't generate items here, or teleport randomly to it

    PERMANENT_TILE_FLAGS = (DISCOVERED | MAGIC_MAPPED | ITEM_DETECTED | HAS_ITEM | HAS_DORMANT_MONSTER
                            | HAS_MONSTER | HAS_STAIRS | SEARCHED_FROM_HERE | PRESSURE_PLATE_DEPRESSED
                            | STABLE_MEMORY | KNOWN_TO_BE_TRAP_FREE | IN_LOOP
                            | IS_CHOKEPOINT | IS_GATE_SITE | IS_IN_MACHINE | IMPREGNABLE),

    ANY_KIND_OF_VISIBLE         = (VISIBLE | CLAIRVOYANT_VISIBLE | TELEPATHIC_VISIBLE),
};

#define TURNS_FOR_FULL_REGEN                300
#define STOMACH_SIZE                        2150
#define HUNGER_THRESHOLD                    (STOMACH_SIZE - 1800)
#define WEAK_THRESHOLD                      150
#define FAINT_THRESHOLD                     50
#define MAX_EXP_LEVEL                       20
#define MAX_EXP                             100000000L

#define XPXP_NEEDED_FOR_TELEPATHIC_BOND     1400 // XPXP required to enable telepathic awareness with the ally

#define ROOM_MIN_WIDTH                      4
#define ROOM_MAX_WIDTH                      20
#define ROOM_MIN_HEIGHT                     3
#define ROOM_MAX_HEIGHT                     7
#define HORIZONTAL_CORRIDOR_MIN_LENGTH      5
#define HORIZONTAL_CORRIDOR_MAX_LENGTH      15
#define VERTICAL_CORRIDOR_MIN_LENGTH        2
#define VERTICAL_CORRIDOR_MAX_LENGTH        9
#define CROSS_ROOM_MIN_WIDTH                3
#define CROSS_ROOM_MAX_WIDTH                12
#define CROSS_ROOM_MIN_HEIGHT               2
#define CROSS_ROOM_MAX_HEIGHT               5
#define MIN_SCALED_ROOM_DIMENSION           2

#define ROOM_TYPE_COUNT                     8

#define CORRIDOR_WIDTH                      1

#define WAYPOINT_SIGHT_RADIUS               10
#define MAX_WAYPOINT_COUNT                  40

#define MAX_ITEMS_IN_MONSTER_ITEMS_HOPPER   100

// Making these larger means cave generation will take more trials; set them too high and the program will hang.
#define CAVE_MIN_WIDTH                      50
#define CAVE_MIN_HEIGHT                     20

// Keyboard commands:
#define UP_KEY              'k'
#define DOWN_KEY            'j'
#define LEFT_KEY            'h'
#define RIGHT_KEY           'l'
#define UP_ARROW            63232
#define LEFT_ARROW          63234
#define DOWN_ARROW          63233
#define RIGHT_ARROW         63235
#define UPLEFT_KEY          'y'
#define UPRIGHT_KEY         'u'
#define DOWNLEFT_KEY        'b'
#define DOWNRIGHT_KEY       'n'
#define DESCEND_KEY         '>'
#define ASCEND_KEY          '<'
#define REST_KEY            'z'
#define AUTO_REST_KEY       'Z'
#define SEARCH_KEY          's'
#define INVENTORY_KEY       'i'
#define ACKNOWLEDGE_KEY     ' '
#define EQUIP_KEY           'e'
#define UNEQUIP_KEY         'r'
#define APPLY_KEY           'a'
#define THROW_KEY           't'
#define RETHROW_KEY         'T'
#define RELABEL_KEY         'R'
#define SWAP_KEY            'w'
#define TRUE_COLORS_KEY     '\\'
#define STEALTH_RANGE_KEY   ']'
#define DROP_KEY            'd'
#define CALL_KEY            'c'
#define QUIT_KEY            'Q'
#define MESSAGE_ARCHIVE_KEY 'M'
#define BROGUE_HELP_KEY     '?'
#define DISCOVERIES_KEY     'D'
#define FEATS_KEY           'F'
#define CREATE_ITEM_MONSTER_KEY 'C'
#define EXPLORE_KEY         'x'
#define AUTOPLAY_KEY        'A'
#define SEED_KEY            '~'
#define EASY_MODE_KEY       '&'
#define ESCAPE_KEY          '\033'
#define RETURN_KEY          '\012'
#define DELETE_KEY          '\177'
#define TAB_KEY             '\t'
#define SHIFT_TAB_KEY       25 // Cocoa reports shift-tab this way for some reason.
#define PERIOD_KEY          '.'
#define VIEW_RECORDING_KEY  'V'
#define LOAD_SAVED_GAME_KEY 'O'
#define SAVE_GAME_KEY       'S'
#define NEW_GAME_KEY        'N'
#define GRAPHICS_KEY        'G'
#define SWITCH_TO_PLAYING_KEY 'P'
#define NUMPAD_0            48
#define NUMPAD_1            49
#define NUMPAD_2            50
#define NUMPAD_3            51
#define NUMPAD_4            52
#define NUMPAD_5            53
#define NUMPAD_6            54
#define NUMPAD_7            55
#define NUMPAD_8            56
#define NUMPAD_9            57
#define PAGE_UP_KEY         63276
#define PAGE_DOWN_KEY       63277
#define PRINTSCREEN_KEY     '\054'

#define UNKNOWN_KEY         (128+19)

#define min(x, y)       (((x) < (y)) ? (x) : (y))
#define max(x, y)       (((x) > (y)) ? (x) : (y))
#define clamp(x, low, hi)   (min(hi, max(x, low))) // pins x to the [y, z] interval

unsigned long terrainFlags(pos loc);
unsigned long terrainMechFlags(pos loc);

boolean cellHasTerrainFlag(pos loc, unsigned long flagMask);
boolean cellHasTMFlag(pos loc, unsigned long flagMask);

boolean cellHasTerrainType(pos loc, enum tileType terrain);

static inline boolean coordinatesAreInMap(short x, short y) {
    return (x >= 0 && x < DCOLS && y >= 0 && y < DROWS);
}

inline static boolean isPosInMap(pos p) {
    return p.x >= 0 && p.x < DCOLS && p.y >= 0 && p.y < DROWS;
}

inline static boolean locIsInWindow(windowpos w) {
    return w.window_x >= 0 && w.window_x < COLS && w.window_y >= 0 && w.window_y < ROWS;
}

inline static pos windowToMap(windowpos w) {
    return (pos) {
        .x = w.window_x - STAT_BAR_WIDTH - 1,
        .y = w.window_y - MESSAGE_LINES,
    };
}

inline static windowpos mapToWindow(pos p) {
    return (windowpos) {
        .window_x = p.x + STAT_BAR_WIDTH + 1,
        .window_y = p.y + MESSAGE_LINES,
    };
}

// Prefer using `mapToWindow` to combine both coordinates together.
#define mapToWindowX(x)                     ((x) + STAT_BAR_WIDTH + 1)
// Prefer using `mapToWindow` to combine both coordinates together.
#define mapToWindowY(y)                     ((y) + MESSAGE_LINES)
// Prefer using `windowToMap` to combine both coordinates together.
#define windowToMapX(x)                     ((x) - STAT_BAR_WIDTH - 1)
// Prefer using `windowToMap` to combine both coordinates together.
#define windowToMapY(y)                     ((y) - MESSAGE_LINES)

#define playerCanDirectlySee(x, y)          (pmap[x][y].flags & VISIBLE)
#define playerCanSee(x, y)                  (pmap[x][y].flags & ANY_KIND_OF_VISIBLE)
#define playerCanSeeOrSense(x, y)           ((pmap[x][y].flags & ANY_KIND_OF_VISIBLE) \
                                            || (rogue.playbackOmniscience \
                                                && (pmap[x][y].layers[DUNGEON] != GRANITE || (pmap[x][y].flags & DISCOVERED))))

#define assureCosmeticRNG                   short oldRNG = rogue.RNG; rogue.RNG = RNG_COSMETIC;
#define restoreRNG                          rogue.RNG = oldRNG;

#define MIN_COLOR_DIFF          600
// weighted sum of the squares of the component differences. Weights are according to color perception.
#define COLOR_DIFF(f, b)         (((f).red - (b).red) * ((f).red - (b).red) * 0.2126 \
+ ((f).green - (b).green) * ((f).green - (b).green) * 0.7152 \
+ ((f).blue - (b).blue) * ((f).blue - (b).blue) * 0.0722)

// structs

enum dungeonLayers {
    NO_LAYER = -1,
    DUNGEON = 0,        // dungeon-level tile   (e.g. walls)
    LIQUID,             // liquid-level tile    (e.g. lava)
    GAS,                // gas-level tile       (e.g. fire, smoke, swamp gas)
    SURFACE,            // surface-level tile   (e.g. grass)
    NUMBER_TERRAIN_LAYERS
};

// keeps track of graphics so we only redraw if the cell has changed:
typedef struct cellDisplayBuffer {
    enum displayGlyph character;
    char foreColorComponents[3];
    char backColorComponents[3];
    char opacity;
} cellDisplayBuffer;

typedef struct screenDisplayBuffer {
    cellDisplayBuffer cells[COLS][ROWS];
} screenDisplayBuffer;

typedef struct pcell {                              // permanent cell; have to remember this stuff to save levels
    enum tileType layers[NUMBER_TERRAIN_LAYERS];    // terrain
    unsigned long flags;                            // non-terrain cell flags
    unsigned short volume;                          // quantity of gas in cell
    unsigned char machineNumber;
    cellDisplayBuffer rememberedAppearance;         // how the player remembers the cell to look
    enum itemCategory rememberedItemCategory;       // what category of item the player remembers lying there
    short rememberedItemKind;                       // what kind of item the player remembers lying there
    short rememberedItemQuantity;                   // how many of the item the player remembers lying there
    short rememberedItemOriginDepth;                // the origin depth of the item the player remembers lying there
    enum tileType rememberedTerrain;                // what the player remembers as the terrain (i.e. highest priority terrain upon last seeing)
    unsigned long rememberedCellFlags;              // map cell flags the player remembers from that spot
    unsigned long rememberedTerrainFlags;           // terrain flags the player remembers from that spot
    unsigned long rememberedTMFlags;                // TM flags the player remembers from that spot
    short exposedToFire;                            // number of times the tile has been exposed to fire since the last environment update
} pcell;

typedef struct tcell {          // transient cell; stuff we don't need to remember between levels
    short light[3];             // RGB components of lighting
    short oldLight[3];          // compare with subsequent lighting to determine whether to refresh cell
} tcell;

typedef struct randomRange {
    short lowerBound;
    short upperBound;
    short clumpFactor;
} randomRange;

typedef struct color {
    // base RGB components:
    short red;
    short green;
    short blue;

    // random RGB components to add to base components:
    short redRand;
    short greenRand;
    short blueRand;

    // random scalar to add to all components:
    short rand;

    // Flag: this color "dances" with every refresh:
    boolean colorDances;
} color;

enum itemFlags {
    ITEM_IDENTIFIED         = Fl(0),
    ITEM_EQUIPPED           = Fl(1),
    ITEM_CURSED             = Fl(2),
    ITEM_PROTECTED          = Fl(3),
    // unused               = Fl(4),
    ITEM_RUNIC              = Fl(5),
    ITEM_RUNIC_HINTED       = Fl(6),
    ITEM_RUNIC_IDENTIFIED   = Fl(7),
    ITEM_CAN_BE_IDENTIFIED  = Fl(8),
    ITEM_PREPLACED          = Fl(9),
    ITEM_FLAMMABLE          = Fl(10),
    ITEM_MAGIC_DETECTED     = Fl(11),
    ITEM_MAX_CHARGES_KNOWN  = Fl(12),
    ITEM_IS_KEY             = Fl(13),

    ITEM_ATTACKS_STAGGER    = Fl(14),   // mace, hammer
    ITEM_ATTACKS_EXTEND     = Fl(15),   // whip
    ITEM_ATTACKS_QUICKLY    = Fl(16),   // rapier
    ITEM_ATTACKS_PENETRATE  = Fl(17),   // spear, pike
    ITEM_ATTACKS_ALL_ADJACENT=Fl(18),   // axe, war axe
    ITEM_LUNGE_ATTACKS      = Fl(19),   // rapier
    ITEM_SNEAK_ATTACK_BONUS = Fl(20),   // dagger
    ITEM_PASS_ATTACKS       = Fl(21),   // flail

    ITEM_KIND_AUTO_ID       = Fl(22),   // the item type will become known when the item is picked up.
    ITEM_PLAYER_AVOIDS      = Fl(23),   // explore and travel will try to avoid picking the item up
};

#define KEY_ID_MAXIMUM  20

typedef struct keyLocationProfile {
    pos loc;
    short machine;
    boolean disposableHere;
} keyLocationProfile;

typedef struct item {
    unsigned short category;
    short kind;
    unsigned long flags;
    randomRange damage;
    short armor;
    short charges;
    short enchant1;
    short enchant2;
    short timesEnchanted;
    enum monsterTypes vorpalEnemy;
    short strengthRequired;
    unsigned short quiverNumber;
    enum displayGlyph displayChar;
    const color *foreColor;
    const color *inventoryColor;
    short quantity;
    char inventoryLetter;
    char inscription[DCOLS];
    pos loc;
    keyLocationProfile keyLoc[KEY_ID_MAXIMUM];
    short originDepth;
    unsigned long spawnTurnNumber;
    unsigned long lastUsed[3];         // Absolute turns last applied
    struct item *nextItem;
} item;

typedef struct itemTable {
    char *name;
    char *flavor;
    char callTitle[30];
    short frequency;
    short marketValue;
    short strengthRequired;
    int power;
    randomRange range;
    boolean identified;
    boolean called;
    int magicPolarity;
    boolean magicPolarityRevealed;
    char description[1500];
} itemTable;

typedef struct charmEffectTableEntry {
    const short kind;
    const int effectDurationBase;
    const fixpt *effectDurationIncrement;
    const int rechargeDelayDuration;
    const int rechargeDelayBase;
    const int rechargeDelayMinTurns;
    const int effectMagnitudeConstant;
    const int effectMagnitudeMultiplier;
} charmEffectTableEntry;

typedef struct meteredItemGenerationTable {
    unsigned short category;
    short kind;
    int initialFrequency;
    int incrementFrequency;
    int decrementFrequency;
    int genMultiplier;
    int genIncrement;
    int levelScaling;
    int levelGuarantee;
    int itemNumberGuarantee;
} meteredItemGenerationTable;

typedef struct levelFeeling {
    const char *message;
    const color *color;
} levelFeeling;

enum dungeonFeatureTypes {
    DF_GRANITE_COLUMN = 1,
    DF_CRYSTAL_WALL,
    DF_LUMINESCENT_FUNGUS,
    DF_GRASS,
    DF_DEAD_GRASS,
    DF_BONES,
    DF_RUBBLE,
    DF_FOLIAGE,
    DF_FUNGUS_FOREST,
    DF_DEAD_FOLIAGE,

    DF_SUNLIGHT,
    DF_DARKNESS,

    DF_SHOW_DOOR,
    DF_SHOW_POISON_GAS_TRAP,
    DF_SHOW_PARALYSIS_GAS_TRAP,
    DF_SHOW_TRAPDOOR_HALO,
    DF_SHOW_TRAPDOOR,
    DF_SHOW_CONFUSION_GAS_TRAP,
    DF_SHOW_FLAMETHROWER_TRAP,
    DF_SHOW_FLOOD_TRAP,
    DF_SHOW_NET_TRAP,
    DF_SHOW_ALARM_TRAP,

    DF_RED_BLOOD,
    DF_GREEN_BLOOD,
    DF_PURPLE_BLOOD,
    DF_WORM_BLOOD,
    DF_ACID_BLOOD,
    DF_ASH_BLOOD,
    DF_EMBER_BLOOD,
    DF_ECTOPLASM_BLOOD,
    DF_RUBBLE_BLOOD,
    DF_ROT_GAS_BLOOD,

    DF_VOMIT,
    DF_BLOAT_DEATH,
    DF_BLOAT_EXPLOSION,
    DF_BLOOD_EXPLOSION,
    DF_FLAMEDANCER_CORONA,

    DF_MUTATION_EXPLOSION,
    DF_MUTATION_LICHEN,

    DF_REPEL_CREATURES,
    DF_ROT_GAS_PUFF,
    DF_STEAM_PUFF,
    DF_STEAM_ACCUMULATION,
    DF_METHANE_GAS_PUFF,
    DF_SALAMANDER_FLAME,
    DF_URINE,
    DF_UNICORN_POOP,
    DF_PUDDLE,
    DF_ASH,
    DF_ECTOPLASM_DROPLET,
    DF_FORCEFIELD,
    DF_FORCEFIELD_MELT,
    DF_SACRED_GLYPHS,
    DF_LICHEN_GROW,
    DF_TUNNELIZE,
    DF_SHATTERING_SPELL,

    // spiderwebs
    DF_WEB_SMALL,
    DF_WEB_LARGE,

    // ancient spirit
    DF_ANCIENT_SPIRIT_VINES,
    DF_ANCIENT_SPIRIT_GRASS,

    // foliage
    DF_TRAMPLED_FOLIAGE,
    DF_SMALL_DEAD_GRASS,
    DF_FOLIAGE_REGROW,
    DF_TRAMPLED_FUNGUS_FOREST,
    DF_FUNGUS_FOREST_REGROW,

    // brimstone
    DF_ACTIVE_BRIMSTONE,
    DF_INERT_BRIMSTONE,

    // bloodwort
    DF_BLOODFLOWER_PODS_GROW_INITIAL,
    DF_BLOODFLOWER_PODS_GROW,
    DF_BLOODFLOWER_POD_BURST,

    // dewars
    DF_DEWAR_CAUSTIC,
    DF_DEWAR_CONFUSION,
    DF_DEWAR_PARALYSIS,
    DF_DEWAR_METHANE,
    DF_DEWAR_GLASS,
    DF_CARPET_AREA,

    // algae
    DF_BUILD_ALGAE_WELL,
    DF_ALGAE_1,
    DF_ALGAE_2,
    DF_ALGAE_REVERT,

    DF_OPEN_DOOR,
    DF_CLOSED_DOOR,
    DF_OPEN_IRON_DOOR_INERT,
    DF_ITEM_CAGE_OPEN,
    DF_ITEM_CAGE_CLOSE,
    DF_ALTAR_INERT,
    DF_ALTAR_RETRACT,
    DF_PORTAL_ACTIVATE,
    DF_INACTIVE_GLYPH,
    DF_ACTIVE_GLYPH,
    DF_SILENT_GLYPH_GLOW,
    DF_GUARDIAN_STEP,
    DF_MIRROR_TOTEM_STEP,
    DF_GLYPH_CIRCLE,
    DF_REVEAL_LEVER,
    DF_PULL_LEVER,
    DF_CREATE_LEVER,

    DF_BRIDGE_FALL_PREP,
    DF_BRIDGE_FALL,

    DF_PLAIN_FIRE,
    DF_GAS_FIRE,
    DF_EXPLOSION_FIRE,
    DF_DART_EXPLOSION,
    DF_BRIMSTONE_FIRE,
    DF_BRIDGE_FIRE,
    DF_FLAMETHROWER,
    DF_EMBERS,
    DF_EMBERS_PATCH,
    DF_OBSIDIAN,
    DF_ITEM_FIRE,
    DF_CREATURE_FIRE,

    DF_FLOOD,
    DF_FLOOD_2,
    DF_FLOOD_DRAIN,
    DF_HOLE_2,
    DF_HOLE_DRAIN,

    DF_DEEP_WATER_FREEZE,
    DF_ALGAE_1_FREEZE,
    DF_ALGAE_2_FREEZE,
    DF_DEEP_WATER_MELTING,
    DF_DEEP_WATER_THAW,
    DF_SHALLOW_WATER_FREEZE,
    DF_SHALLOW_WATER_MELTING,
    DF_SHALLOW_WATER_THAW,

    DF_POISON_GAS_CLOUD,
    DF_CONFUSION_GAS_TRAP_CLOUD,
    DF_NET,
    DF_AGGRAVATE_TRAP,
    DF_METHANE_GAS_ARMAGEDDON,

    // potions
    DF_POISON_GAS_CLOUD_POTION,
    DF_PARALYSIS_GAS_CLOUD_POTION,
    DF_CONFUSION_GAS_CLOUD_POTION,
    DF_INCINERATION_POTION,
    DF_DARKNESS_POTION,
    DF_HOLE_POTION,
    DF_LICHEN_PLANTED,

    // other items
    DF_ARMOR_IMMOLATION,
    DF_STAFF_HOLE,
    DF_STAFF_HOLE_EDGE,

    // commutation altar
    DF_ALTAR_COMMUTE,
    DF_MAGIC_PIPING,
    DF_INERT_PIPE,

    // resurrection altar
    DF_ALTAR_RESURRECT,
    DF_MACHINE_FLOOR_TRIGGER_REPEATING,

    // sacrifice altar
    DF_SACRIFICE_ALTAR,
    DF_SACRIFICE_COMPLETE,
    DF_SACRIFICE_CAGE_ACTIVE,

    // vampire in coffin
    DF_COFFIN_BURSTS,
    DF_COFFIN_BURNS,
    DF_TRIGGER_AREA,

    // throwing tutorial -- button in chasm
    DF_CAGE_DISAPPEARS,
    DF_MEDIUM_HOLE,
    DF_MEDIUM_LAVA_POND,
    DF_MACHINE_PRESSURE_PLATE_USED,

    // rat trap
    DF_WALL_CRACK,

    // wooden barricade at entrance
    DF_WOODEN_BARRICADE_BURN,

    // wooden barricade around altar, dead grass all around
    DF_SURROUND_WOODEN_BARRICADE,

    // pools of water that, when triggered, slowly expand to fill the room
    DF_SPREADABLE_WATER,
    DF_SHALLOW_WATER,
    DF_WATER_SPREADS,
    DF_SPREADABLE_WATER_POOL,
    DF_SPREADABLE_DEEP_WATER_POOL,

    // when triggered, the ground gradually turns into chasm:
    DF_SPREADABLE_COLLAPSE,
    DF_COLLAPSE,
    DF_COLLAPSE_SPREADS,
    DF_ADD_MACHINE_COLLAPSE_EDGE_DORMANT,

    // when triggered, a bridge appears:
    DF_BRIDGE_ACTIVATE,
    DF_BRIDGE_ACTIVATE_ANNOUNCE,
    DF_BRIDGE_APPEARS,
    DF_ADD_DORMANT_CHASM_HALO,

    // when triggered, the lava retracts:
    DF_LAVA_RETRACTABLE,
    DF_RETRACTING_LAVA,
    DF_OBSIDIAN_WITH_STEAM,

    // when triggered, the door seals and caustic gas fills the room
    DF_SHOW_POISON_GAS_VENT,
    DF_POISON_GAS_VENT_OPEN,
    DF_ACTIVATE_PORTCULLIS,
    DF_OPEN_PORTCULLIS,
    DF_VENT_SPEW_POISON_GAS,

    // when triggered, pilot light ignites and explosive gas fills the room
    DF_SHOW_METHANE_VENT,
    DF_METHANE_VENT_OPEN,
    DF_VENT_SPEW_METHANE,
    DF_PILOT_LIGHT,

    // paralysis trap: trigger plate with gas vents nearby
    DF_DISCOVER_PARALYSIS_VENT,
    DF_PARALYSIS_VENT_SPEW,
    DF_REVEAL_PARALYSIS_VENT_SILENTLY,

    // thematic dungeon
    DF_AMBIENT_BLOOD,

    // statues crack for a few turns and then shatter, revealing the monster inside
    DF_CRACKING_STATUE,
    DF_STATUE_SHATTER,

    // a turret appears:
    DF_TURRET_EMERGE,

    // an elaborate worm catacomb opens up
    DF_WORM_TUNNEL_MARKER_DORMANT,
    DF_WORM_TUNNEL_MARKER_ACTIVE,
    DF_GRANITE_CRUMBLES,
    DF_WALL_OPEN,

    // the room gradually darkens
    DF_DARKENING_FLOOR,
    DF_DARK_FLOOR,
    DF_HAUNTED_TORCH_TRANSITION,
    DF_HAUNTED_TORCH,

    // bubbles rise from the mud and bog monsters spawn
    DF_MUD_DORMANT,
    DF_MUD_ACTIVATE,

    // crystals charge when hit by lightning
    DF_ELECTRIC_CRYSTAL_ON,
    DF_TURRET_LEVER,

    // idyll:
    DF_SHALLOW_WATER_POOL,
    DF_DEEP_WATER_POOL,

    // swamp:
    DF_SWAMP_WATER,
    DF_SWAMP,
    DF_SWAMP_MUD,

    // camp:
    DF_HAY,
    DF_JUNK,

    // remnants:
    DF_REMNANT,
    DF_REMNANT_ASH,

    // chasm catwalk:
    DF_CHASM_HOLE,
    DF_CATWALK_BRIDGE,

    // lake catwalk:
    DF_LAKE_CELL,
    DF_LAKE_HALO,

    // worm den:
    DF_WALL_SHATTER,

    // monster cages open:
    DF_MONSTER_CAGE_OPENS,

    // goblin warren:
    DF_STENCH_BURN,
    DF_STENCH_SMOLDER,

    NUMBER_DUNGEON_FEATURES,
};

enum dungeonProfileTypes {
    DP_BASIC,
    DP_BASIC_FIRST_ROOM,

    DP_GOBLIN_WARREN,
    DP_SENTINEL_SANCTUARY,

    NUMBER_DUNGEON_PROFILES,
};

typedef struct lightSource {
    const color *lightColor;
    randomRange lightRadius;
    short radialFadeToPercent;
    boolean passThroughCreatures; // generally no, but miner light does
} lightSource;

typedef struct flare {
    const lightSource *light;           // Flare light
    short coeffChangeAmount;            // The constant amount by which the coefficient changes per frame, e.g. -25 means it gets 25% dimmer per frame.
    short coeffLimit;                   // Flare ends if the coefficient passes this percentage (whether going up or down).
    pos loc;                            // Current flare location.
    long coeff;                         // Current flare coefficient; always starts at 100.
    unsigned long turnNumber;           // So we can eliminate those that fired one or more turns ago.
} flare;

enum DFFlags {
    DFF_EVACUATE_CREATURES_FIRST    = Fl(0),    // Creatures in the DF area get moved outside of it
    DFF_SUBSEQ_EVERYWHERE           = Fl(1),    // Subsequent DF spawns in every cell that this DF spawns in, instead of only the origin
    DFF_TREAT_AS_BLOCKING           = Fl(2),    // If filling the footprint of this DF with walls would disrupt level connectivity, then abort.
    DFF_PERMIT_BLOCKING             = Fl(3),    // Generate this DF without regard to level connectivity.
    DFF_ACTIVATE_DORMANT_MONSTER    = Fl(4),    // Dormant monsters on this tile will appear -- e.g. when a statue bursts to reveal a monster.
    DFF_CLEAR_OTHER_TERRAIN         = Fl(5),    // Erase other terrain in the footprint of this DF.
    DFF_BLOCKED_BY_OTHER_LAYERS     = Fl(6),    // Will not propagate into a cell if any layer in that cell has a superior priority.
    DFF_SUPERPRIORITY               = Fl(7),    // Will overwrite terrain of a superior priority.
    DFF_AGGRAVATES_MONSTERS         = Fl(8),    // Will act as though an aggravate monster scroll of effectRadius radius had been read at that point.
    DFF_RESURRECT_ALLY              = Fl(9),    // Will bring back to life your most recently deceased ally.
    DFF_CLEAR_LOWER_PRIORITY_TERRAIN= Fl(10),   // Erase terrain with a lower priority in the footprint of this DF.
};

enum boltEffects {
    BE_NONE,
    BE_ATTACK,
    BE_TELEPORT,
    BE_SLOW,
    BE_POLYMORPH,
    BE_NEGATION,
    BE_DOMINATION,
    BE_BECKONING,
    BE_PLENTY,
    BE_INVISIBILITY,
    BE_EMPOWERMENT,
    BE_DAMAGE,
    BE_POISON,
    BE_TUNNELING,
    BE_BLINKING,
    BE_ENTRANCEMENT,
    BE_OBSTRUCTION,
    BE_DISCORD,
    BE_CONJURATION,
    BE_HEALING,
    BE_HASTE,
    BE_SHIELDING,
};

enum boltFlags {
    BF_PASSES_THRU_CREATURES        = Fl(0),    // Bolt continues through creatures (e.g. lightning and tunneling)
    BF_HALTS_BEFORE_OBSTRUCTION     = Fl(1),    // Bolt takes effect the space before it terminates (e.g. conjuration, obstruction, blinking)
    BF_TARGET_ALLIES                = Fl(2),    // Staffs/wands/creatures that shoot this bolt will auto-target allies.
    BF_TARGET_ENEMIES               = Fl(3),    // Staffs/wands/creatures that shoot this bolt will auto-target enemies.
    BF_FIERY                        = Fl(4),    // Bolt will light flammable terrain on fire as it passes, and will ignite monsters hit.
    BF_NEVER_REFLECTS               = Fl(6),    // Bolt will never reflect (e.g. spiderweb, arrows).
    BF_NOT_LEARNABLE                = Fl(7),    // This technique cannot be absorbed by empowered allies.
    BF_NOT_NEGATABLE                = Fl(8),    // Won't be erased by negation.
    BF_ELECTRIC                     = Fl(9),    // Activates terrain that has TM_PROMOTES_ON_ELECTRICITY
    BF_DISPLAY_CHAR_ALONG_LENGTH    = Fl(10),   // Display the character along the entire length of the bolt instead of just at the front.
};

typedef struct bolt {
    char name[DCOLS];
    char description[COLS];
    char abilityDescription[COLS*2];
    enum displayGlyph theChar;
    const color *foreColor;
    const color *backColor;
    short boltEffect;
    short magnitude;
    short pathDF;
    short targetDF;
    unsigned long forbiddenMonsterFlags;
    unsigned long flags;
} bolt;

// Level profiles, affecting what rooms get chosen and how they're connected:
typedef struct dungeonProfile {
    // Room type weights (in the natural dungeon, these are also adjusted based on depth):
    short roomFrequencies[ROOM_TYPE_COUNT];

    short corridorChance;
} dungeonProfile;

// Dungeon features, spawned from Architect.c:
typedef struct dungeonFeature {
    // tile info:
    enum tileType tile;
    enum dungeonLayers layer;

    // spawning pattern:
    short startProbability;
    short probabilityDecrement;
    unsigned long flags;
    char description[DCOLS];
    enum lightType lightFlare;
    const color *flashColor;
    short effectRadius;
    enum tileType propagationTerrain;
    enum dungeonFeatureTypes subsequentDF;
    boolean messageDisplayed;
} dungeonFeature;

// Terrain types:
typedef struct floorTileType {
    // appearance:
    enum displayGlyph displayChar;
    const color *foreColor;
    const color *backColor;
    short drawPriority;                     // priority (lower number means higher priority); governs drawing as well as tile replacement comparisons.
    char chanceToIgnite;                    // chance to burn if a flame terrain is on one of the four cardinal neighbors
    enum dungeonFeatureTypes fireType;      // spawn this DF when the terrain ignites (or, if it's T_IS_DF_TRAP, when the pressure plate clicks)
    enum dungeonFeatureTypes discoverType;  // spawn this DF when successfully searched if T_IS_SECRET is set
    enum dungeonFeatureTypes promoteType;   // creates this dungeon spawn type when it promotes for some other reason (random promotion or promotion through machine activation)
    short promoteChance;                    // percent chance per turn to spawn the promotion type; will also vanish upon doing so if T_VANISHES_UPON_PROMOTION is set
    short glowLight;                        // if it glows, this is the ID of the light type
    unsigned long flags;
    unsigned long mechFlags;
    char description[COLS];
    char flavorText[COLS];
} floorTileType;

enum terrainFlagCatalog {
    T_OBSTRUCTS_PASSABILITY         = Fl(0),        // cannot be walked through
    T_OBSTRUCTS_VISION              = Fl(1),        // blocks line of sight
    T_OBSTRUCTS_ITEMS               = Fl(2),        // items can't be on this tile
    T_OBSTRUCTS_SURFACE_EFFECTS     = Fl(3),        // grass, blood, etc. cannot exist on this tile
    T_OBSTRUCTS_GAS                 = Fl(4),        // blocks the permeation of gas
    T_OBSTRUCTS_DIAGONAL_MOVEMENT   = Fl(5),        // can't step diagonally around this tile
    T_SPONTANEOUSLY_IGNITES         = Fl(6),        // monsters avoid unless chasing player or immune to fire
    T_AUTO_DESCENT                  = Fl(7),        // automatically drops creatures down a depth level and does some damage (2d6)
    T_LAVA_INSTA_DEATH              = Fl(8),        // kills any non-levitating non-fire-immune creature instantly
    T_CAUSES_POISON                 = Fl(9),        // any non-levitating creature gets 10 poison
    T_IS_FLAMMABLE                  = Fl(10),       // terrain can catch fire
    T_IS_FIRE                       = Fl(11),       // terrain is a type of fire; ignites neighboring flammable cells
    T_ENTANGLES                     = Fl(12),       // entangles players and monsters like a spiderweb
    T_IS_DEEP_WATER                 = Fl(13),       // steals items 50% of the time and moves them around randomly
    T_CAUSES_DAMAGE                 = Fl(14),       // anything on the tile takes max(1-2, 10%) damage per turn
    T_CAUSES_NAUSEA                 = Fl(15),       // any creature on the tile becomes nauseous
    T_CAUSES_PARALYSIS              = Fl(16),       // anything caught on this tile is paralyzed
    T_CAUSES_CONFUSION              = Fl(17),       // causes creatures on this tile to become confused
    T_CAUSES_HEALING                = Fl(18),       // heals 20% max HP per turn for any player or non-inanimate monsters
    T_IS_DF_TRAP                    = Fl(19),       // spews gas of type specified in fireType when stepped on
    T_CAUSES_EXPLOSIVE_DAMAGE       = Fl(20),       // is an explosion; deals higher of 15-20 or 50% damage instantly, but not again for five turns
    T_SACRED                        = Fl(21),       // monsters that aren't allies of the player will avoid stepping here

    T_OBSTRUCTS_SCENT               = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION | T_AUTO_DESCENT | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES),
    T_PATHING_BLOCKER               = (T_OBSTRUCTS_PASSABILITY | T_AUTO_DESCENT | T_IS_DF_TRAP | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_IS_FIRE | T_SPONTANEOUSLY_IGNITES),
    T_DIVIDES_LEVEL                 = (T_OBSTRUCTS_PASSABILITY | T_AUTO_DESCENT | T_IS_DF_TRAP | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER),
    T_LAKE_PATHING_BLOCKER          = (T_AUTO_DESCENT | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES),
    T_WAYPOINT_BLOCKER              = (T_OBSTRUCTS_PASSABILITY | T_AUTO_DESCENT | T_IS_DF_TRAP | T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES),
    T_MOVES_ITEMS                   = (T_IS_DEEP_WATER | T_LAVA_INSTA_DEATH),
    T_CAN_BE_BRIDGED                = (T_AUTO_DESCENT),
    T_OBSTRUCTS_EVERYTHING          = (T_OBSTRUCTS_PASSABILITY | T_OBSTRUCTS_VISION | T_OBSTRUCTS_ITEMS | T_OBSTRUCTS_GAS | T_OBSTRUCTS_SURFACE_EFFECTS | T_OBSTRUCTS_DIAGONAL_MOVEMENT),
    T_HARMFUL_TERRAIN               = (T_CAUSES_POISON | T_IS_FIRE | T_CAUSES_DAMAGE | T_CAUSES_PARALYSIS | T_CAUSES_CONFUSION | T_CAUSES_EXPLOSIVE_DAMAGE),
    T_RESPIRATION_IMMUNITIES        = (T_CAUSES_DAMAGE | T_CAUSES_CONFUSION | T_CAUSES_PARALYSIS | T_CAUSES_NAUSEA),
};

enum terrainMechanicalFlagCatalog {
    TM_IS_SECRET                    = Fl(0),        // successful search or being stepped on while visible transforms it into discoverType
    TM_PROMOTES_WITH_KEY            = Fl(1),        // promotes if the key is present on the tile (in your pack, carried by monster, or lying on the ground)
    TM_PROMOTES_WITHOUT_KEY         = Fl(2),        // promotes if the key is NOT present on the tile (in your pack, carried by monster, or lying on the ground)
    TM_PROMOTES_ON_CREATURE         = Fl(3),        // promotes when a creature or player is on the tile (whether or not levitating)
    TM_PROMOTES_ON_ITEM             = Fl(4),        // promotes when an item is on the tile
    TM_PROMOTES_ON_ITEM_PICKUP      = Fl(5),        // promotes when an item is lifted from the tile (primarily for altars)
    TM_PROMOTES_ON_PLAYER_ENTRY     = Fl(6),        // promotes when the player enters the tile (whether or not levitating)
    TM_PROMOTES_ON_SACRIFICE_ENTRY  = Fl(7),        // promotes when the sacrifice target enters the tile (whether or not levitating)
    TM_PROMOTES_ON_ELECTRICITY      = Fl(8),        // promotes when hit by a lightning bolt
    TM_ALLOWS_SUBMERGING            = Fl(9),        // allows submersible monsters to submerge in this terrain
    TM_IS_WIRED                     = Fl(10),        // if wired, promotes when powered, and sends power when promoting
    TM_IS_CIRCUIT_BREAKER           = Fl(11),       // prevents power from circulating in its machine
    TM_GAS_DISSIPATES               = Fl(12),       // does not just hang in the air forever
    TM_GAS_DISSIPATES_QUICKLY       = Fl(13),       // dissipates quickly
    TM_EXTINGUISHES_FIRE            = Fl(14),       // extinguishes burning terrain or creatures
    TM_VANISHES_UPON_PROMOTION      = Fl(15),       // vanishes when creating promotion dungeon feature, even if the replacement terrain priority doesn't require it
    TM_REFLECTS_BOLTS               = Fl(16),       // magic bolts reflect off of its surface randomly (similar to pmap flag IMPREGNABLE)
    TM_STAND_IN_TILE                = Fl(17),       // earthbound creatures will be said to stand "in" the tile, not on it
    TM_LIST_IN_SIDEBAR              = Fl(18),       // terrain will be listed in the sidebar with a description of the terrain type
    TM_VISUALLY_DISTINCT            = Fl(19),       // terrain will be color-adjusted if necessary so the character stands out from the background
    TM_BRIGHT_MEMORY                = Fl(20),       // no blue fade when this tile is out of sight
    TM_EXPLOSIVE_PROMOTE            = Fl(21),       // when burned, will promote to promoteType instead of burningType if surrounded by tiles with T_IS_FIRE or TM_EXPLOSIVE_PROMOTE
    TM_CONNECTS_LEVEL               = Fl(22),       // will be treated as passable for purposes of calculating level connectedness, irrespective of other aspects of this terrain layer
    TM_INTERRUPT_EXPLORATION_WHEN_SEEN = Fl(23),    // will generate a message when discovered during exploration to interrupt exploration
    TM_INVERT_WHEN_HIGHLIGHTED      = Fl(24),       // will flip fore and back colors when highlighted with pathing
    TM_SWAP_ENCHANTS_ACTIVATION     = Fl(25),       // in machine, swap item enchantments when two suitable items are on this terrain, and activate the machine when that happens

    TM_PROMOTES_ON_STEP             = (TM_PROMOTES_ON_CREATURE | TM_PROMOTES_ON_ITEM),
};

enum statusEffects {
    STATUS_SEARCHING = 0,
    STATUS_DONNING,
    STATUS_WEAKENED,
    STATUS_TELEPATHIC,
    STATUS_HALLUCINATING,
    STATUS_LEVITATING,
    STATUS_SLOWED,
    STATUS_HASTED,
    STATUS_CONFUSED,
    STATUS_BURNING,
    STATUS_PARALYZED,
    STATUS_POISONED,
    STATUS_STUCK,
    STATUS_NAUSEOUS,
    STATUS_DISCORDANT,
    STATUS_IMMUNE_TO_FIRE,
    STATUS_EXPLOSION_IMMUNITY,
    STATUS_NUTRITION,
    STATUS_ENTERS_LEVEL_IN,
    STATUS_ENRAGED, // temporarily ignores normal MA_AVOID_CORRIDORS behavior
    STATUS_MAGICAL_FEAR,
    STATUS_ENTRANCED,
    STATUS_DARKNESS,
    STATUS_LIFESPAN_REMAINING,
    STATUS_SHIELDED,
    STATUS_INVISIBLE,
    STATUS_AGGRAVATING,
    NUMBER_OF_STATUS_EFFECTS,
};

typedef struct statusEffect {
    char name[COLS];
    boolean isNegatable;
    int playerNegatedValue;
} statusEffect;

enum hordeFlags {
    HORDE_DIES_ON_LEADER_DEATH      = Fl(0),    // if the leader dies, the horde will die instead of electing new leader
    HORDE_IS_SUMMONED               = Fl(1),    // minions summoned when any creature is the same species as the leader and casts summon
    HORDE_SUMMONED_AT_DISTANCE      = Fl(2),    // summons will appear across the level, and will naturally path back to the leader
    HORDE_LEADER_CAPTIVE            = Fl(3),    // the leader is in chains and the followers are guards
    HORDE_NO_PERIODIC_SPAWN         = Fl(4),    // can spawn only when the level begins -- not afterwards
    HORDE_ALLIED_WITH_PLAYER        = Fl(5),

    HORDE_MACHINE_BOSS              = Fl(6),    // used in machines for a boss challenge
    HORDE_MACHINE_WATER_MONSTER     = Fl(7),    // used in machines where the room floods with shallow water
    HORDE_MACHINE_CAPTIVE           = Fl(8),    // powerful captive monsters without any captors
    HORDE_MACHINE_STATUE            = Fl(9),    // the kinds of monsters that make sense in a statue
    HORDE_MACHINE_TURRET            = Fl(10),   // turrets, for hiding in walls
    HORDE_MACHINE_MUD               = Fl(11),   // bog monsters, for hiding in mud
    HORDE_MACHINE_KENNEL            = Fl(12),   // monsters that can appear in cages in kennels
    HORDE_VAMPIRE_FODDER            = Fl(13),   // monsters that are prone to capture and farming by vampires
    HORDE_MACHINE_LEGENDARY_ALLY    = Fl(14),   // legendary allies
    HORDE_NEVER_OOD                 = Fl(15),   // Horde cannot be generated out of depth
    HORDE_MACHINE_THIEF             = Fl(16),   // monsters that can be generated in the key thief area machines
    HORDE_MACHINE_GOBLIN_WARREN     = Fl(17),   // can spawn in goblin warrens
    HORDE_SACRIFICE_TARGET          = Fl(18),   // can be the target of an assassination challenge; leader will get scary light.

    HORDE_MACHINE_ONLY              = (HORDE_MACHINE_BOSS | HORDE_MACHINE_WATER_MONSTER
                                       | HORDE_MACHINE_CAPTIVE | HORDE_MACHINE_STATUE
                                       | HORDE_MACHINE_TURRET | HORDE_MACHINE_MUD
                                       | HORDE_MACHINE_KENNEL | HORDE_VAMPIRE_FODDER
                                       | HORDE_MACHINE_LEGENDARY_ALLY | HORDE_MACHINE_THIEF
                                       | HORDE_MACHINE_GOBLIN_WARREN
                                       | HORDE_SACRIFICE_TARGET),
};

enum monsterBehaviorFlags {
    MONST_INVISIBLE                 = Fl(0),    // monster is invisible
    MONST_INANIMATE                 = Fl(1),    // monster has abbreviated stat bar display and is immune to many things
    MONST_IMMOBILE                  = Fl(2),    // monster won't move or perform melee attacks
    MONST_CARRY_ITEM_100            = Fl(3),    // monster carries an item 100% of the time
    MONST_CARRY_ITEM_25             = Fl(4),    // monster carries an item 25% of the time
    MONST_ALWAYS_HUNTING            = Fl(5),    // monster is never asleep or in wandering mode
    MONST_FLEES_NEAR_DEATH          = Fl(6),    // monster flees when under 25% health and re-engages when over 75%
    MONST_ATTACKABLE_THRU_WALLS     = Fl(7),    // can be attacked when embedded in a wall
    MONST_DEFEND_DEGRADE_WEAPON     = Fl(8),    // hitting the monster damages the weapon
    MONST_IMMUNE_TO_WEAPONS         = Fl(9),    // weapons ineffective
    MONST_FLIES                     = Fl(10),   // permanent levitation
    MONST_FLITS                     = Fl(11),   // moves randomly a third of the time
    MONST_IMMUNE_TO_FIRE            = Fl(12),   // won't burn, won't die in lava
    MONST_CAST_SPELLS_SLOWLY        = Fl(13),   // takes twice the attack duration to cast a spell
    MONST_IMMUNE_TO_WEBS            = Fl(14),   // monster passes freely through webs
    MONST_REFLECT_50                = Fl(15),   // monster reflects ~50% of bolts, as though wearing +4 armor of reflection
    MONST_NEVER_SLEEPS              = Fl(16),   // monster is always awake
    MONST_FIERY                     = Fl(17),   // monster carries an aura of flame (but no automatic fire light)
    MONST_INVULNERABLE              = Fl(18),   // monster is immune to absolutely everything
    MONST_IMMUNE_TO_WATER           = Fl(19),   // monster moves at full speed in deep water and (if player) doesn't drop items
    MONST_RESTRICTED_TO_LIQUID      = Fl(20),   // monster can move only on tiles that allow submersion
    MONST_SUBMERGES                 = Fl(21),   // monster can submerge in appropriate terrain
    MONST_MAINTAINS_DISTANCE        = Fl(22),   // monster tries to keep a distance of 3 tiles between it and player
    MONST_WILL_NOT_USE_STAIRS       = Fl(23),   // monster won't chase the player between levels
    MONST_DIES_IF_NEGATED           = Fl(24),   // monster will die if exposed to negation magic
    MONST_MALE                      = Fl(25),   // monster is male (or 50% likely to be male if also has MONST_FEMALE)
    MONST_FEMALE                    = Fl(26),   // monster is female (or 50% likely to be female if also has MONST_MALE)
    MONST_NOT_LISTED_IN_SIDEBAR     = Fl(27),   // monster doesn't show up in the sidebar
    MONST_GETS_TURN_ON_ACTIVATION   = Fl(28),   // monster never gets a turn, except when its machine is activated
    MONST_ALWAYS_USE_ABILITY        = Fl(29),   // monster will never fail to use special ability if eligible (no random factor)
    MONST_NO_POLYMORPH              = Fl(30),   // monster cannot result from a polymorph spell (liches, phoenixes and Warden of Yendor)

    NEGATABLE_TRAITS                = (MONST_INVISIBLE | MONST_DEFEND_DEGRADE_WEAPON | MONST_IMMUNE_TO_WEAPONS | MONST_FLIES
                                       | MONST_FLITS | MONST_IMMUNE_TO_FIRE | MONST_REFLECT_50 | MONST_FIERY | MONST_MAINTAINS_DISTANCE),
    MONST_TURRET                    = (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE |
                                       MONST_ATTACKABLE_THRU_WALLS | MONST_WILL_NOT_USE_STAIRS),
    LEARNABLE_BEHAVIORS             = (MONST_INVISIBLE | MONST_FLIES | MONST_IMMUNE_TO_FIRE | MONST_REFLECT_50),
    MONST_NEVER_VORPAL_ENEMY        = (MONST_INANIMATE | MONST_INVULNERABLE | MONST_IMMOBILE | MONST_RESTRICTED_TO_LIQUID | MONST_GETS_TURN_ON_ACTIVATION | MONST_MAINTAINS_DISTANCE),
    MONST_NEVER_MUTATED             = (MONST_INVISIBLE | MONST_INANIMATE | MONST_IMMOBILE | MONST_INVULNERABLE),
};

typedef struct monsterBehavior {
    char description[COLS];
    boolean isNegatable;
} monsterBehavior;


enum monsterAbilityFlags {
    MA_HIT_HALLUCINATE              = Fl(0),    // monster can hit to cause hallucinations
    MA_HIT_STEAL_FLEE               = Fl(1),    // monster can steal an item and then run away
    MA_HIT_BURN                     = Fl(2),    // monster can hit to set you on fire
    MA_ENTER_SUMMONS                = Fl(3),    // monster will "become" its summoned leader, reappearing when that leader is defeated (phylactery, phoenix egg, vampire)
    MA_HIT_DEGRADE_ARMOR            = Fl(4),    // monster damages armor
    MA_CAST_SUMMON                  = Fl(5),    // requires that there be one or more summon hordes with this monster type as the leader
    MA_SEIZES                       = Fl(6),    // monster seizes enemies before attacking
    MA_POISONS                      = Fl(7),    // monster's damage is dealt in the form of poison
    MA_DF_ON_DEATH                  = Fl(8),    // monster spawns its DF when it dies
    MA_CLONE_SELF_ON_DEFEND         = Fl(9),    // monster splits in two when struck
    MA_KAMIKAZE                     = Fl(10),   // monster dies instead of attacking
    MA_TRANSFERENCE                 = Fl(11),   // monster recovers 40 or 90% of the damage that it inflicts as health
    MA_CAUSES_WEAKNESS              = Fl(12),   // monster attacks cause weakness status in target
    MA_ATTACKS_PENETRATE            = Fl(13),   // monster attacks all adjacent enemies, like an axe
    MA_ATTACKS_ALL_ADJACENT         = Fl(14),   // monster attacks penetrate one layer of enemies, like a spear
    MA_ATTACKS_EXTEND               = Fl(15),   // monster attacks from a distance in a cardinal direction, like a whip
    MA_ATTACKS_STAGGER              = Fl(16),   // monster attacks will push the player backward by one space if there is room
    MA_AVOID_CORRIDORS              = Fl(17),   // monster will avoid corridors when hunting
    MA_REFLECT_100                  = Fl(18),   // monster reflects 100% of bolts directly back at the caster

    SPECIAL_HIT                     = (MA_HIT_HALLUCINATE | MA_HIT_STEAL_FLEE | MA_HIT_DEGRADE_ARMOR | MA_POISONS
                                       | MA_TRANSFERENCE | MA_CAUSES_WEAKNESS | MA_HIT_BURN | MA_ATTACKS_STAGGER),
    LEARNABLE_ABILITIES             = (MA_TRANSFERENCE | MA_CAUSES_WEAKNESS),

    MA_NON_NEGATABLE_ABILITIES      = (MA_ATTACKS_PENETRATE | MA_ATTACKS_ALL_ADJACENT | MA_ATTACKS_EXTEND | MA_ATTACKS_STAGGER),
    MA_NEVER_VORPAL_ENEMY           = (MA_KAMIKAZE),
    MA_NEVER_MUTATED                = (MA_KAMIKAZE),
};

typedef struct monsterAbility {
    char description[COLS];
    boolean isNegatable;
} monsterAbility;

enum monsterBookkeepingFlags {
    MB_WAS_VISIBLE              = Fl(0),    // monster was visible to player last turn
    MB_TELEPATHICALLY_REVEALED  = Fl(1),    // player can magically see monster and adjacent cells
    MB_PREPLACED                = Fl(2),    // monster dropped onto the level and requires post-processing
    MB_APPROACHING_UPSTAIRS     = Fl(3),    // following the player up the stairs
    MB_APPROACHING_DOWNSTAIRS   = Fl(4),    // following the player down the stairs
    MB_APPROACHING_PIT          = Fl(5),    // following the player down a pit
    MB_LEADER                   = Fl(6),    // monster is the leader of a horde
    MB_FOLLOWER                 = Fl(7),    // monster is a member of a horde
    MB_CAPTIVE                  = Fl(8),    // monster is all tied up
    MB_SEIZED                   = Fl(9),    // monster is being held
    MB_SEIZING                  = Fl(10),   // monster is holding another creature immobile
    MB_SUBMERGED                = Fl(11),   // monster is currently submerged and hence invisible until it attacks
    MB_JUST_SUMMONED            = Fl(12),   // used to mark summons so they can be post-processed
    MB_WILL_FLASH               = Fl(13),   // this monster will flash as soon as control is returned to the player
    MB_BOUND_TO_LEADER          = Fl(14),   // monster will die if the leader dies or becomes separated from the leader
    MB_MARKED_FOR_SACRIFICE     = Fl(15),   // scary glow, monster can be sacrificed in the appropriate machine
    MB_ABSORBING                = Fl(16),   // currently learning a skill by absorbing an enemy corpse
    MB_DOES_NOT_TRACK_LEADER    = Fl(17),   // monster will not follow its leader around
    MB_IS_FALLING               = Fl(18),   // monster is plunging downward at the end of the turn
    MB_IS_DYING                 = Fl(19),   // monster is currently dying; the death is still being processed
    MB_GIVEN_UP_ON_SCENT        = Fl(20),   // to help the monster remember that the scent map is a dead end
    MB_IS_DORMANT               = Fl(21),   // lurking, waiting to burst out
    MB_WEAPON_AUTO_ID           = Fl(22),   // slaying the monster will count toward weapon auto-ID
    MB_ALREADY_SEEN             = Fl(23),   // seeing this monster won't interrupt exploration
    MB_ADMINISTRATIVE_DEATH     = Fl(24),   // like the `administrativeDeath` parameter to `killCreature`
    MB_HAS_DIED                 = Fl(25),   // monster has already been killed but not yet removed from `monsters`
    MB_DOES_NOT_RESURRECT       = Fl(26)    // resurrection altars don't revive monsters summoned by allies
};

// Defines all creatures, which include monsters and the player:
typedef struct creatureType {
    enum monsterTypes monsterID; // index number for the monsterCatalog
    char monsterName[COLS];
    enum displayGlyph displayChar;
    const color *foreColor;
    short maxHP;
    short defense;
    short accuracy;
    randomRange damage;
    long turnsBetweenRegen;     // turns to wait before regaining 1 HP
    short movementSpeed;
    short attackSpeed;
    enum dungeonFeatureTypes bloodType;
    enum lightType intrinsicLightType;
    boolean isLarge;    // used for size of psychic emanation
    short DFChance;                     // percent chance to spawn the dungeon feature per awake turn
    enum dungeonFeatureTypes DFType;    // kind of dungeon feature
    enum boltType bolts[20];
    unsigned long flags;
    unsigned long abilityFlags;
} creatureType;

typedef struct monsterWords {
    char flavorText[COLS*5];
    char absorbing[40];
    char absorbStatus[40];
    char attack[5][30];
    char DFMessage[DCOLS * 2];
    char summonMessage[DCOLS * 2];
} monsterWords;

enum creatureStates {
    MONSTER_SLEEPING,
    MONSTER_TRACKING_SCENT,
    MONSTER_WANDERING,
    MONSTER_FLEEING,
    MONSTER_ALLY,
};

enum creatureModes {
    MODE_NORMAL,
    MODE_PERM_FLEEING
};

typedef struct mutation {
    char title[100];
    const color *textColor;
    short healthFactor;
    short moveSpeedFactor;
    short attackSpeedFactor;
    short defenseFactor;
    short damageFactor;
    short DFChance;
    enum dungeonFeatureTypes DFType;
    enum lightType light;
    unsigned long monsterFlags;
    unsigned long monsterAbilityFlags;
    unsigned long forbiddenFlags;
    unsigned long forbiddenAbilityFlags;
    char description[1000];
    boolean canBeNegated;
} mutation;

typedef struct hordeType {
    enum monsterTypes leaderType;

    // membership information
    short numberOfMemberTypes;
    enum monsterTypes memberType[5];
    randomRange memberCount[5];

    // spawning information
    short minLevel;
    short maxLevel;
    short frequency;
    enum tileType spawnsIn;
    short machine;

    enum hordeFlags flags;
} hordeType;

typedef struct monsterClass {
    char name[30];
    short frequency;
    short maxDepth;
    enum monsterTypes memberList[15];
} monsterClass;

typedef struct creature {
    creatureType info;
    pos loc;
    short depth;
    short currentHP;
    long turnsUntilRegen;
    short regenPerTurn;                 // number of HP to regenerate every single turn
    short weaknessAmount;               // number of points of weakness that are inflicted by the weakness status
    short poisonAmount;                 // number of points of damage per turn from poison
    enum creatureStates creatureState;  // current behavioral state
    enum creatureModes creatureMode;    // current behavioral mode (higher-level than state)

    short mutationIndex;                // what mutation the monster has (or -1 for none)
    boolean wasNegated;                 // the monster has lost abilities due to negation

    // Waypoints:
    short targetWaypointIndex;          // the index number of the waypoint we're pathing toward
    boolean waypointAlreadyVisited[MAX_WAYPOINT_COUNT]; // checklist of waypoints
    pos lastSeenPlayerAt;          // last location at which the monster hunted the player

    pos targetCorpseLoc;           // location of the corpse that the monster is approaching to gain its abilities
    char targetCorpseName[30];          // name of the deceased monster that we're approaching to gain its abilities
    unsigned long absorptionFlags;      // ability/behavior flags that the monster will gain when absorption is complete
    boolean absorbBehavior;             // above flag is behavior instead of ability (ignored if absorptionBolt is set)
    short absorptionBolt;               // bolt index that the monster will learn to cast when absorption is complete
    short corpseAbsorptionCounter;      // used to measure both the time until the monster stops being interested in the corpse,
                                        // and, later, the time until the monster finishes absorbing the corpse.
    short **mapToMe;                    // if a pack leader, this is a periodically updated pathing map to get to the leader
    short **safetyMap;                  // fleeing monsters store their own safety map when out of player FOV to avoid omniscience
    short ticksUntilTurn;               // how long before the creature gets its next move

    // Locally cached statistics that may be temporarily modified:
    short movementSpeed;
    short attackSpeed;

    short previousHealthPoints;         // remembers what your health proportion was at the start of the turn
    short turnsSpentStationary;         // how many (subjective) turns it's been since the creature moved between tiles
    short flashStrength;                // monster will flash soon; this indicates the percent strength of flash
    color flashColor;                   // the color that the monster will flash
    short status[NUMBER_OF_STATUS_EFFECTS];
    short maxStatus[NUMBER_OF_STATUS_EFFECTS]; // used to set the max point on the status bars
    unsigned long bookkeepingFlags;
    short spawnDepth;                   // keep track of the depth of the machine to which they relate (for activation monsters)
    short machineHome;                  // monsters that spawn in a machine keep track of the machine number here (for activation monsters)
    short xpxp;                         // exploration experience (used to time telepathic bonding for allies)
    short newPowerCount;                // how many more times this monster can absorb a fallen monster
    short totalPowerCount;              // how many times has the monster been empowered? Used to recover abilities when negated.

    struct creature *leader;                 // only if monster is a follower
    struct creature *carriedMonster; // when vampires turn into bats, one of the bats restores the vampire when it dies
    struct item *carriedItem;                // only used for monsters
} creature;

typedef struct creatureListNode {
    creature *creature;
    // A list node is just a creature that also knows who comes next.
    struct creatureListNode *nextCreature;
} creatureListNode;

typedef struct creatureList {
    creatureListNode* head;
} creatureList;

typedef struct creatureIterator {
    creatureList *list; // used for restarting
    creatureListNode *next;
} creatureIterator;

enum NGCommands {
    NG_NOTHING = 0,
    NG_FLYOUT_PLAY,
    NG_FLYOUT_VIEW,
    NG_FLYOUT_OPTIONS,
    NG_GAME_VARIANT,
    NG_GAME_MODE,
    NG_NEW_GAME,
    NG_NEW_GAME_WITH_SEED,
    NG_OPEN_GAME,
    NG_VIEW_RECORDING,
    NG_HIGH_SCORES,
    NG_GAME_STATS,
    NG_QUIT,
};

enum featTypes {
    FEAT_PURE_MAGE = 0,
    FEAT_PURE_WARRIOR,
    FEAT_COMPANION,
    FEAT_SPECIALIST,
    FEAT_JELLYMANCER,
    FEAT_DRAGONSLAYER,
    FEAT_PALADIN,
    FEAT_TONE
};

enum exitStatus {
    EXIT_STATUS_SUCCESS,
    EXIT_STATUS_FAILURE_RECORDING_WRONG_VERSION,
    EXIT_STATUS_FAILURE_RECORDING_OOS,
    EXIT_STATUS_FAILURE_PLATFORM_ERROR
};

// Constants for the selected game variant, set in Globals{variant}.c
// Many of these constants were migrated from #defines prior to variant support
typedef struct gameConstants {

    const int majorVersion;                         // variant major version x._._
    const int minorVersion;                         // variant minor version _.y._
    const int patchVersion;                         // variant patch version _._.z

    const char *variantName;                        // variant name in no-spaces, lower-case form (e.g. "brogue")

    const char *versionString;                      // version string <codename> <major>.<minor>.<patch>
    const char *dungeonVersionString;               // earliest version that has identical dungeons to this version
    const char *patchVersionPattern;                // scanf format string for patches that match this release
    const char *recordingVersionString;             // version string used in recordings / saves. Cannot be longer than 16 chars.

    const int deepestLevel;                         // deepest level in the dungeon
    const int amuletLevel;                          // level on which the amulet appears (used in signed arithmetic)

    const int depthAccelerator;                     // factor for how fast depth-dependent features scale compared to usual 26-level dungeon
    const int minimumLavaLevel;                     // how deep before lava can be generated
    const int minimumBrimstoneLevel;                // how deep before brimstone can be generated
    const int mutationsOccurAboveLevel;                // how deep before monster mutations can be generated

    const int extraItemsPerLevel;                   // how many extra items generated per level above vanilla
    const int goldAdjustmentStartDepth;             // depth from which gold is adjusted based on generation so far

    const int machinesPerLevelSuppressionMultiplier; // scale factor for limiting number of machines generated so far against depth
    const int machinesPerLevelSuppressionOffset;     // offset for limiting number of machines generated so far against depth
    const int machinesPerLevelIncreaseFactor;        // scale factor for increasing number of machines generated so far against depth
    const int maxLevelForBonusMachines;              // deepest level that gets bonus machine generation chance

    const int playerTransferenceRatio;              // player transference heal is (enchant / gameConst->playerTransferenceRatio)
    const int onHitHallucinateDuration;             // duration of on-hit hallucination effect on player
    const int onHitWeakenDuration;                  // duration of on-hit weaken effect
    const int onHitMercyHealPercent;                // percentage of damage healed on-hit by mercy weapon effect

    const int fallDamageMin;                        // minimum for fall damage range
    const int fallDamageMax;                        // maximum for fall damage range

    const int weaponKillsToAutoID;                  // number of kills until unknown weapon is IDed
    const int armorDelayToAutoID;                   // number of turns until unknown armor is IDed
    const int ringDelayToAutoID;                    // number of turns until unknown ring is IDed

    const int numberAutogenerators;                 // size of autoGeneratorCatalog table
    const int numberBoltKinds;                      // size of boltKinds table
    const int numberBlueprints;                     // size of blueprintCatalog table
    const int numberHordes;                         // size of the horde table

    const int numberMeteredItems;                   // size of the metered items table
    const int numberCharmKinds;                     // size of the charms table
    const int numberPotionKinds;                    // size of the potion table
    const int numberGoodPotionKinds;                // number of good potions in the game (ordered first in the table)
    const int numberScrollKinds;                    // size of the scroll table
    const int numberGoodScrollKinds;                // number of good scrolls in the game (ordered first in the table)
    const int numberWandKinds;                      // size of the wands table
    const int numberGoodWandKinds;                  // number of good wands in the game (ordered first in the table)

    const int numberFeats;                          // size of feats table
    const int companionFeatRequiredXP;              // Ally XP needed for the companion feat

    const int mainMenuTitleHeight;                  // height of the title screen in characters
    const int mainMenuTitleWidth;                   // width of the title screen in characters
} gameConstants;


// these are basically global variables pertaining to the game state and player's unique variables:
typedef struct playerCharacter {
    boolean wizard;                     // in wizard mode

    short depthLevel;                   // which dungeon level are we on
    short deepestLevel;
    boolean disturbed;                  // player should stop auto-acting
    boolean gameInProgress;             // the game is in progress (the player has not died, won or quit yet)
    boolean gameHasEnded;               // stop everything and go to death screen
    boolean highScoreSaved;             // so that it saves the high score only once
    boolean blockCombatText;            // busy auto-fighting
    boolean autoPlayingLevel;           // seriously, don't interrupt
    boolean automationActive;           // cut some corners during redraws to speed things up
    boolean justRested;                 // previous turn was a rest -- used in stealth
    boolean justSearched;               // previous turn was a search -- used in manual searches
    boolean cautiousMode;               // used to prevent careless deaths caused by holding down a key
    boolean receivedLevitationWarning;  // only warn you once when you're hovering dangerously over liquid
    boolean updatedSafetyMapThisTurn;   // so it's updated no more than once per turn
    boolean updatedAllySafetyMapThisTurn;   // so it's updated no more than once per turn
    boolean updatedMapToSafeTerrainThisTurn;// so it's updated no more than once per turn
    boolean updatedMapToShoreThisTurn;      // so it's updated no more than once per turn
    boolean easyMode;                   // enables easy mode
    boolean inWater;                    // helps with the blue water filter effect
    boolean heardCombatThisTurn;        // so you get only one "you hear combat in the distance" per turn
    boolean creaturesWillFlashThisTurn; // there are creatures out there that need to flash before the turn ends
    boolean staleLoopMap;               // recalculate the loop map at the end of the turn
    boolean alreadyFell;                // so the player can fall only one depth per turn
    boolean eligibleToUseStairs;        // so the player uses stairs only when he steps onto them
    boolean trueColorMode;              // whether lighting effects are disabled
    boolean displayStealthRangeMode;    // whether your stealth range is displayed
    boolean quit;                       // to skip the typical end-game theatrics when the player quits
    uint64_t seed;                      // the master seed for generating the entire dungeon
    short RNG;                          // which RNG are we currently using?
    unsigned long gold;                 // how much gold we have
    unsigned long goldGenerated;        // how much gold has been generated on the levels, not counting gold held by monsters
    short strength;
    unsigned short monsterSpawnFuse;    // how much longer till a random monster spawns

    item *weapon;
    item *armor;
    item *ringLeft;
    item *ringRight;
    item *swappedIn;
    item *swappedOut;

    flare **flares;
    short flareCount;
    short flareCapacity;

    creature *yendorWarden;

    lightSource minersLight;
    fixpt minersLightRadius;
    short ticksTillUpdateEnvironment;   // so that some periodic things happen in objective time
    unsigned short scentTurnNumber;     // helps make scent-casting work
    unsigned long playerTurnNumber;     // number of input turns in recording. Does not increment during paralysis.
    unsigned long absoluteTurnNumber;   // number of turns since the beginning of time. Always increments.
    signed long milliseconds;           // milliseconds since launch, to decide whether to engage cautious mode
    short xpxpThisTurn;                 // how many squares the player explored this turn
    short stealthRange;                 // distance from which monsters will notice you

    short previousPoisonPercent;        // and your poison proportion, to display percentage alerts for each.

    pos upLoc;                          // upstairs location this level
    pos downLoc;                        // downstairs location this level

    pos cursorLoc;                      // used for the return key functionality
    creature *lastTarget;               // to keep track of the last monster the player has thrown at or zapped
    item *lastItemThrown;
    short rewardRoomsGenerated;         // to meter the number of reward machines
    short machineNumber;                // so each machine on a level gets a unique number
    pos sidebarLocationList[ROWS*2];    // to keep track of which location each line of the sidebar references

    // maps
    short **mapToShore;                 // how many steps to get back to shore
    short **mapToSafeTerrain;           // so monsters can get to safety

    // recording info
    boolean recording;                  // whether we are recording the game
    boolean playbackMode;               // whether we're viewing a recording instead of playing
    unsigned short patchVersion;        // what patch version of the game this was recorded on
    char versionString[16];             // the version string saved into the recording file
    unsigned long currentTurnNumber;    // how many turns have elapsed
    unsigned long howManyTurns;         // how many turns are in this recording
    short howManyDepthChanges;          // how many times the player changes depths
    short playbackDelayPerTurn;         // base playback speed; modified per turn by events
    short playbackDelayThisTurn;        // playback speed as modified
    boolean playbackPaused;
    boolean playbackFastForward;        // for loading saved games and such -- disables drawing and prevents pauses
    boolean playbackOOS;                // playback out of sync -- no unpausing allowed
    boolean playbackOmniscience;        // whether to reveal all the map during playback
    boolean playbackBetweenTurns;       // i.e. waiting for a top-level input -- iff, permit playback commands
    unsigned long nextAnnotationTurn;   // the turn number during which to display the next annotation
    char nextAnnotation[5000];          // the next annotation
    unsigned long locationInAnnotationFile; // how far we've read in the annotations file
    int gameExitStatusCode;             // exit status code indicating if brogue exited successfully or with an error

    // metered items
    long long foodSpawned;                    // amount of nutrition units spawned so far this game
    meteredItem *meteredItems;

    // ring bonuses:
    short clairvoyance;
    short stealthBonus;
    short regenerationBonus;
    short lightMultiplier;
    short awarenessBonus;
    short transference;
    short wisdomBonus;
    short reaping;

    // feats:
    boolean *featRecord;

    // waypoints:
    short **wpDistance[MAX_WAYPOINT_COUNT];
    short wpCount;
    pos wpCoordinates[MAX_WAYPOINT_COUNT];
    short wpRefreshTicker;

    // cursor trail:
    short cursorPathIntensity;
    boolean cursorMode;

    // What do you want to do, player -- play, play with seed, resume, recording, high scores or quit?
    // Also used for main menu flyout navigation
    enum NGCommands nextGame;
    char nextGamePath[BROGUE_FILENAME_MAX];
    uint64_t nextGameSeed;

    // Path of the current save game or recording, NULL for a new game
    char currentGamePath[BROGUE_FILENAME_MAX];
} playerCharacter;

// Stores the necessary info about a level so it can be regenerated:
typedef struct levelData {
    boolean visited;
    pcell mapStorage[DCOLS][DROWS];
    struct item *items;
    struct creatureList monsters;
    struct creatureList dormantMonsters;
    short **scentMap;
    uint64_t levelSeed;
    pos upStairsLoc;
    pos downStairsLoc;
    pos playerExitedVia;
    unsigned long awaySince;
} levelData;

enum machineFeatureFlags {
    MF_GENERATE_ITEM                = Fl(0),    // feature entails generating an item (overridden if the machine is adopting an item)
    MF_OUTSOURCE_ITEM_TO_MACHINE    = Fl(1),    // item must be adopted by another machine
    MF_BUILD_VESTIBULE              = Fl(2),    // call this at the origin of a door room to create a new door guard machine there
    MF_ADOPT_ITEM                   = Fl(3),    // this feature will take the adopted item (be it from another machine or a previous feature)
    MF_NO_THROWING_WEAPONS          = Fl(4),    // the generated item cannot be a throwing weapon
    MF_GENERATE_HORDE               = Fl(5),    // generate a monster horde that has all of the horde flags
    MF_BUILD_AT_ORIGIN              = Fl(6),    // generate this feature at the room entrance
    // unused                       = Fl(7),    //
    MF_PERMIT_BLOCKING              = Fl(8),    // permit the feature to block the map's passability (e.g. to add a locked door)
    MF_TREAT_AS_BLOCKING            = Fl(9),    // treat this terrain as though it blocks, for purposes of deciding whether it can be placed there
    MF_NEAR_ORIGIN                  = Fl(10),   // feature must spawn in the rough quarter of tiles closest to the origin
    MF_FAR_FROM_ORIGIN              = Fl(11),   // feature must spawn in the rough quarter of tiles farthest from the origin
    MF_MONSTER_TAKE_ITEM            = Fl(12),   // the item associated with this feature (including if adopted) will be in possession of the horde leader that's generated
    MF_MONSTER_SLEEPING             = Fl(13),   // the monsters should be asleep when generated
    MF_MONSTER_FLEEING              = Fl(14),   // the monsters should be permanently fleeing when generated
    MF_EVERYWHERE                   = Fl(15),   // generate the feature on every tile of the machine (e.g. carpeting)
    MF_ALTERNATIVE                  = Fl(16),   // build only one feature that has this flag per machine; the rest are skipped
    MF_ALTERNATIVE_2                = Fl(17),   // same as MF_ALTERNATIVE, but provides for a second set of alternatives of which only one will be chosen
    MF_REQUIRE_GOOD_RUNIC           = Fl(18),   // generated item must be uncursed runic
    MF_MONSTERS_DORMANT             = Fl(19),   // monsters are dormant, and appear when a dungeon feature with DFF_ACTIVATE_DORMANT_MONSTER spawns on their tile
    // unused                       = Fl(20),   //
    MF_BUILD_IN_WALLS               = Fl(21),   // build in an impassable tile that is adjacent to the interior
    MF_BUILD_ANYWHERE_ON_LEVEL      = Fl(22),   // build anywhere on the level that is not inside the machine
    MF_REPEAT_UNTIL_NO_PROGRESS     = Fl(23),   // keep trying to build this feature set until no changes are made
    MF_IMPREGNABLE                  = Fl(24),   // this feature's location will be immune to tunneling
    MF_IN_VIEW_OF_ORIGIN            = Fl(25),   // this feature must be in view of the origin
    MF_IN_PASSABLE_VIEW_OF_ORIGIN   = Fl(26),   // this feature must be in view of the origin, where "view" is blocked by pathing blockers
    MF_NOT_IN_HALLWAY               = Fl(27),   // the feature location must have a passableArcCount of <= 1
    MF_NOT_ON_LEVEL_PERIMETER       = Fl(28),   // don't build it in the outermost walls of the level
    MF_SKELETON_KEY                 = Fl(29),   // if a key is generated or adopted by this feature, it will open all locks in this machine.
    MF_KEY_DISPOSABLE               = Fl(30),   // if a key is generated or adopted, it will self-destruct after being used at this current location.
};

typedef struct machineFeature {
    // terrain
    enum dungeonFeatureTypes featureDF; // generate this DF at the feature location (0 for none)
    enum tileType terrain;              // generate this terrain tile at the feature location (0 for none)
    enum dungeonLayers layer;           // generate the terrain tile in this layer

    short instanceCountRange[2];        // generate this range of instances of this feature
    short minimumInstanceCount;         // abort if fewer than this

    // items: these will be ignored if the feature is adopting an item
    short itemCategory;                 // generate this category of item (or -1 for random)
    short itemKind;                     // generate this kind of item (or -1 for random)

    short monsterID;                    // generate a monster of this kind if MF_GENERATE_MONSTER is set

    short personalSpace;                // subsequent features must be generated more than this many tiles away from this feature
    unsigned long hordeFlags;           // choose a monster horde based on this
    unsigned long itemFlags;            // assign these flags to the item
    unsigned long flags;                // feature flags
} machineFeature;

enum blueprintFlags {
    BP_ADOPT_ITEM                   = Fl(0),    // the machine must adopt an item (e.g. a door key)
    BP_VESTIBULE                    = Fl(1),    // spawns in a doorway (location must be given) and expands outward, to guard the room
    BP_PURGE_PATHING_BLOCKERS       = Fl(2),    // clean out traps and other T_PATHING_BLOCKERs
    BP_PURGE_INTERIOR               = Fl(3),    // clean out all of the terrain in the interior before generating the machine
    BP_PURGE_LIQUIDS                = Fl(4),    // clean out all of the liquids in the interior before generating the machine
    BP_SURROUND_WITH_WALLS          = Fl(5),    // fill in any impassable gaps in the perimeter (e.g. water, lava, brimstone, traps) with wall
    BP_IMPREGNABLE                  = Fl(6),    // impassable perimeter and interior tiles are locked; tunneling bolts will bounce off harmlessly
    BP_REWARD                       = Fl(7),    // metered reward machines
    BP_OPEN_INTERIOR                = Fl(8),    // clear out walls in the interior, widen the interior until convex or bumps into surrounding areas
    BP_MAXIMIZE_INTERIOR            = Fl(9),    // same as BP_OPEN_INTERIOR but expands the room as far as it can go, potentially surrounding the whole level.
    BP_ROOM                         = Fl(10),   // spawns in a dead-end room that is dominated by a chokepoint of the given size (as opposed to a random place of the given size)
    BP_TREAT_AS_BLOCKING            = Fl(11),   // abort the machine if, were it filled with wall tiles, it would disrupt the level connectivity
    BP_REQUIRE_BLOCKING             = Fl(12),   // abort the machine unless, were it filled with wall tiles, it would disrupt the level connectivity
    BP_NO_INTERIOR_FLAG             = Fl(13),   // don't flag the area as being part of a machine
    BP_REDESIGN_INTERIOR            = Fl(14),   // nuke and pave -- delete all terrain in the interior and build entirely new rooms within the bounds
};

typedef struct blueprint {
    const char *name;                   // machine name
    short depthRange[2];                // machine must be built between these dungeon depths
    short roomSize[2];                  // machine must be generated in a room of this size
    short frequency;                    // frequency (number of tickets this blueprint enters in the blueprint selection raffle)
    short featureCount;                 // how many different types of features follow (max of 20)
    short dungeonProfileType;           // if BP_REDESIGN_INTERIOR is set, which dungeon profile do we use?
    unsigned long flags;                // blueprint flags
    machineFeature feature[20];         // the features themselves
} blueprint;

enum machineTypes {
    // Reward rooms:
    MT_REWARD_MULTI_LIBRARY = 1,
    MT_REWARD_MONO_LIBRARY,
    MT_REWARD_CONSUMABLES,
    MT_REWARD_PEDESTALS_PERMANENT,
    MT_REWARD_PEDESTALS_CONSUMABLE,
    MT_REWARD_COMMUTATION_ALTARS,
    MT_REWARD_RESURRECTION_ALTAR,
    MT_REWARD_ADOPTED_ITEM,
    MT_REWARD_DUNGEON,
    MT_REWARD_KENNEL,
    MT_REWARD_VAMPIRE_LAIR,
    MT_REWARD_ASTRAL_PORTAL,
    MT_REWARD_GOBLIN_WARREN,
    MT_REWARD_SENTINEL_SANCTUARY,

    // Amulet holder:
    MT_AMULET_AREA,

    // Door guard machines:
    MT_LOCKED_DOOR_VESTIBULE,
    MT_SECRET_DOOR_VESTIBULE,
    MT_SECRET_LEVER_VESTIBULE,
    MT_FLAMMABLE_BARRICADE_VESTIBULE,
    MT_STATUE_SHATTERING_VESTIBULE,
    MT_STATUE_MONSTER_VESTIBULE,
    MT_THROWING_TUTORIAL_VESTIBULE,
    MT_PIT_TRAPS_VESTIBULE,
    MT_BECKONING_OBSTACLE_VESTIBULE,
    MT_GUARDIAN_VESTIBULE,

    // Key guard machines:
    MT_KEY_REWARD_LIBRARY,
    MT_KEY_SECRET_ROOM,
    MT_KEY_THROWING_TUTORIAL_AREA,
    MT_KEY_RAT_TRAP_ROOM,
    MT_KEY_FIRE_TRANSPORTATION_ROOM,
    MT_KEY_FLOOD_TRAP_ROOM,
    MT_KEY_FIRE_TRAP_ROOM,
    MT_KEY_THIEF_AREA,
    MT_KEY_COLLAPSING_FLOOR_AREA,
    MT_KEY_PIT_TRAP_ROOM,
    MT_KEY_LEVITATION_ROOM,
    MT_KEY_WEB_CLIMBING_ROOM,
    MT_KEY_LAVA_MOAT_ROOM,
    MT_KEY_LAVA_MOAT_AREA,
    MT_KEY_POISON_GAS_TRAP_ROOM,
    MT_KEY_EXPLOSIVE_TRAP_ROOM,
    MT_KEY_BURNING_TRAP_ROOM,
    MT_KEY_STATUARY_TRAP_AREA,
    MT_KEY_GUARDIAN_WATER_PUZZLE_ROOM,
    MT_KEY_GUARDIAN_GAUNTLET_ROOM,
    MT_KEY_GUARDIAN_CORRIDOR_ROOM,
    MT_KEY_SACRIFICE_ROOM,
    MT_KEY_SUMMONING_CIRCLE_ROOM,
    MT_KEY_BECKONING_OBSTACLE_ROOM,
    MT_KEY_WORM_TRAP_AREA,
    MT_KEY_MUD_TRAP_ROOM,
    MT_KEY_ELECTRIC_CRYSTALS_ROOM,
    MT_KEY_ZOMBIE_TRAP_ROOM,
    MT_KEY_PHANTOM_TRAP_ROOM,
    MT_KEY_WORM_TUNNEL_ROOM,
    MT_KEY_TURRET_TRAP_ROOM,
    MT_KEY_BOSS_ROOM,

    // Thematic machines:
    MT_BLOODFLOWER_AREA,
    MT_SHRINE_AREA,
    MT_IDYLL_AREA,
    MT_SWAMP_AREA,
    MT_CAMP_AREA,
    MT_REMNANT_AREA,
    MT_DISMAL_AREA,
    MT_BRIDGE_TURRET_AREA,
    MT_LAKE_PATH_TURRET_AREA,
    MT_PARALYSIS_TRAP_AREA,
    MT_PARALYSIS_TRAP_HIDDEN_AREA,
    MT_TRICK_STATUE_AREA,
    MT_WORM_AREA,
    MT_SENTINEL_AREA
};

typedef struct autoGenerator {
    // What spawns:
    enum tileType terrain;
    enum dungeonLayers layer;

    enum dungeonFeatureTypes DFType;

    enum machineTypes machine; // Machine placement also respects BP_ placement flags in the machine blueprint

    // Parameters governing when and where it spawns:
    enum tileType requiredDungeonFoundationType;
    enum tileType requiredLiquidFoundationType;
    short minDepth;
    short maxDepth;
    short frequency;
    short minNumberIntercept; // actually intercept * 100
    short minNumberSlope; // actually slope * 100
    short maxNumber;
} autoGenerator;

#define FEAT_NAME_LENGTH 15

typedef struct feat {
    char name[FEAT_NAME_LENGTH + 1];
    char description[200];
    boolean initialValue;
} feat;

#define PDS_FORBIDDEN   -1
#define PDS_OBSTRUCTION -2
#define PDS_CELL(map, x, y) ((map)->links + ((x) + DCOLS * (y)))

#define BUTTON_TEXT_SIZE COLS*3
typedef struct brogueButton {
    char text[BUTTON_TEXT_SIZE];// button label; can include color escapes
    short x;                    // button's leftmost cell will be drawn at (x, y)
    short y;
    signed long hotkey[10];     // up to 10 hotkeys to trigger the button
    color buttonColor;          // background of the button; further gradient-ized when displayed
    color textColor;            // color of the button text
    color hotkeytextColor;      // color of the portion of the button text that highlights the keyboard hotkey
    short opacity;              // further reduced by 50% if not enabled
    enum displayGlyph symbol[COLS];         // Automatically replace the nth asterisk in the button label text with
                                // the nth character supplied here, if one is given.
                                // (Primarily to display magic character and item symbols in the inventory display.)
    unsigned long flags;
    enum NGCommands command;
} brogueButton;

enum buttonDrawStates {
    BUTTON_NORMAL = 0,
    BUTTON_HOVER,
    BUTTON_PRESSED,
};

enum BUTTON_FLAGS {
    B_DRAW                  = Fl(0),
    B_ENABLED               = Fl(1),
    B_GRADIENT              = Fl(2),
    B_HOVER_ENABLED         = Fl(3),
    B_WIDE_CLICK_AREA       = Fl(4),
    B_KEYPRESS_HIGHLIGHT    = Fl(5),
};

typedef struct buttonState {
    // Indices of the buttons that are doing stuff:
    short buttonFocused;
    short buttonDepressed;

    // Index of the selected button:
    short buttonChosen;

    // The buttons themselves:
    short buttonCount;
    brogueButton buttons[50];

    // The window location, to determine whether a click is a cancelation:
    short winX;
    short winY;
    short winWidth;
    short winHeight;
} buttonState;

enum messageFlags {
    REQUIRE_ACKNOWLEDGMENT        = Fl(0),
    REFRESH_SIDEBAR               = Fl(1),
    FOLDABLE                      = Fl(2),
};

enum autoTargetMode {
    AUTOTARGET_MODE_NONE,               // don't autotarget
    AUTOTARGET_MODE_USE_STAFF_OR_WAND,
    AUTOTARGET_MODE_THROW,
    AUTOTARGET_MODE_EXPLORE,            // cycle through anything in the sidebar
};

typedef struct archivedMessage {
    char message[COLS*2];
    unsigned char count;          // how many times this message appears
    unsigned long turn;           // player turn of the first occurrence
    unsigned long flags;
} archivedMessage;

extern boolean serverMode;
extern boolean nonInteractivePlayback;
extern boolean hasGraphics;
extern enum graphicsModes graphicsMode;

#if defined __cplusplus
extern "C" {
#endif

    // Utilities.c - String functions
    boolean endswith(const char *str, const char *ending);
    void append(char *str, char *ending, int bufsize);

    int rogueMain(void);
    void printBrogueVersion(void);
    void executeEvent(rogueEvent *theEvent);
    boolean fileExists(const char *pathname);
    boolean chooseFile(char *path, char *prompt, char *defaultName, char *suffix);
    boolean openFile(const char *path);
    void initializeGameVariant(void);
    int deepestLevelForGameVariant(void);
    void initializeRogue(uint64_t seed);
    void gameOver(char *killedBy, boolean useCustomPhrasing);
    void victory(boolean superVictory);
    void setPlayerDisplayChar(void);
    void initializeDynamicColors(void);
    void enableEasyMode(void);
    boolean tryParseUint64(char *str, uint64_t *num);
    uint64_t rand_64bits(void);
    long rand_range(long lowerBound, long upperBound);
    uint64_t seedRandomGenerator(uint64_t seed);
    short randClumpedRange(short lowerBound, short upperBound, short clumpFactor);
    short randClump(randomRange theRange);
    boolean rand_percent(short percent);
    void shuffleList(short *list, short listLength);
    void fillSequentialList(short *list, short listLength);
    fixpt fp_round(fixpt x);
    fixpt fp_pow(fixpt base, int expn);
    fixpt fp_sqrt(fixpt val);
    short unflag(unsigned long flag);
    void considerCautiousMode(void);
    void refreshScreen(void);
    void displayLevel(void);
    void storeColorComponents(char components[3], const color *theColor);
    boolean separateColors(color *fore, const color *back);
    void bakeColor(color *theColor);
    void shuffleTerrainColors(short percentOfCells, boolean refreshCells);
    void normColor(color *baseColor, const short aggregateMultiplier, const short colorTranslation);
    void getCellAppearance(pos loc, enum displayGlyph *returnChar, color *returnForeColor, color *returnBackColor);
    void logBuffer(char array[DCOLS][DROWS]);
    //void logBuffer(short **array);
    boolean search(short searchStrength);
    boolean proposeOrConfirmLocation(short x, short y, char *failureMessage);
    boolean useStairs(short stairDirection);
    short passableArcCount(short x, short y);
    void analyzeMap(boolean calculateChokeMap);
    boolean buildAMachine(enum machineTypes bp,
                          short originX, short originY,
                          unsigned long requiredMachineFlags,
                          item *adoptiveItem,
                          item *parentSpawnedItems[50],
                          creature *parentSpawnedMonsters[50]);
    void attachRooms(short **grid, const dungeonProfile *theDP, short attempts, short maxRoomCount);
    void digDungeon(void);
    void updateMapToShore(void);
    short levelIsDisconnectedWithBlockingMap(char blockingMap[DCOLS][DROWS], boolean countRegionSize);
    void resetDFMessageEligibility(void);
    boolean fillSpawnMap(enum dungeonLayers layer,
                         enum tileType surfaceTileType,
                         char spawnMap[DCOLS][DROWS],
                         boolean blockedByOtherLayers,
                         boolean refresh,
                         boolean superpriority);
    boolean spawnDungeonFeature(short x, short y, dungeonFeature *feat, boolean refreshCell, boolean abortIfBlocking);
    void restoreMonster(creature *monst, short **mapToStairs, short **mapToPit);
    void restoreItems(void);
    void refreshWaypoint(short wpIndex);
    void setUpWaypoints(void);
    void zeroOutGrid(char grid[DCOLS][DROWS]);
    short oppositeDirection(short theDir);

    void plotChar(enum displayGlyph inputChar,
                  short xLoc, short yLoc,
                  short backRed, short backGreen, short backBlue,
                  short foreRed, short foreGreen, short foreBlue);

    typedef struct PauseBehavior {
        /// If `interuptForMouseMove` is true, then the pause function will return `true`
        /// if a mouse move occurs during the pause.
        /// Otherwise, mouse movements will be ignored.
        boolean interuptForMouseMove;
    } PauseBehavior;
#define PAUSE_BEHAVIOR_DEFAULT ((PauseBehavior) { .interuptForMouseMove = false })

    boolean pauseForMilliseconds(short milliseconds, PauseBehavior behavior);
    boolean isApplicationActive(void);
    void nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance);
    void notifyEvent(short eventId, int data1, int data2, const char *str1, const char *str2);
    boolean takeScreenshot(void);
    enum graphicsModes setGraphicsMode(enum graphicsModes mode);
    boolean controlKeyIsDown(void);
    boolean shiftKeyIsDown(void);
    short getHighScoresList(rogueHighScoresEntry returnList[HIGH_SCORES_COUNT]);
    boolean saveHighScore(rogueHighScoresEntry theEntry);
    void saveRunHistory(char *result, char *killedBy, int score, int lumenstones);
    void saveResetRun(void);
    rogueRun *loadRunHistory(void);
    fileEntry *listFiles(short *fileCount, char **dynamicMemoryBuffer);
    void initializeLaunchArguments(enum NGCommands *command, char *path, uint64_t *seed);

    char nextKeyPress(boolean textInput);
    void refreshSideBar(short focusX, short focusY, boolean focusedEntityMustGoFirst);
    void printHelpScreen(void);
    void displayFeatsScreen(void);
    void printDiscoveriesScreen(void);
    void printHighScores(boolean hiliteMostRecent);
    void displayGrid(short **map);
    void printSeed(void);
    void printProgressBar(short x, short y, const char barLabel[COLS], long amtFilled, long amtMax, const color *fillColor, boolean dim);
    short printMonsterInfo(creature *monst, short y, boolean dim, boolean highlight);
    enum displayGlyph getItemCategoryGlyph(const enum itemCategory theCategory);
    enum itemCategory getHallucinatedItemCategory(void);
    void describedItemName(const item *theItem, char *description, int maxLength);
    void describeHallucinatedItem(char *buf);
    short printItemInfo(item *theItem, short y, boolean dim, boolean highlight);
    short printTerrainInfo(short x, short y, short py, const char *description, boolean dim, boolean highlight);
    void rectangularShading(short x, short y, short width, short height,
                            const color *backColor, short opacity, screenDisplayBuffer *dbuf);
    short printTextBox(char *textBuf, short x, short y, short width,
                       const color *foreColor, const color *backColor,
                       brogueButton *buttons, short buttonCount);
    void setButtonText(brogueButton *button, const char *textWithHotkey, const char *textWithoutHotkey);
    void printMonsterDetails(creature *monst);
    void printFloorItemDetails(item *theItem);
    unsigned long printCarriedItemDetails(item *theItem,
                                          short x, short y, short width,
                                          boolean includeButtons);
    void funkyFade(screenDisplayBuffer *displayBuf, const color *colorStart, const color *colorEnd, short stepCount, short x, short y, boolean invert);
    void displayCenteredAlert(char *message);
    void flashMessage(char *message, short x, short y, int time, const color *fColor, const color *bColor);
    void flashTemporaryAlert(char *message, int time);
    void highlightScreenCell(short x, short y, const color *highlightColor, short strength);
    void waitForAcknowledgment(void);
    void waitForKeystrokeOrMouseClick(void);
    boolean confirm(char *prompt, boolean alsoDuringPlayback);
    void refreshDungeonCell(pos loc);
    void applyColorMultiplier(color *baseColor, const color *multiplierColor);
    void applyColorAverage(color *baseColor, const color *newColor, short averageWeight);
    void applyColorAugment(color *baseColor, const color *augmentingColor, short augmentWeight);
    void applyColorScalar(color *baseColor, short scalar);
    void applyColorBounds(color *baseColor, short lowerBound, short upperBound);
    void desaturate(color *baseColor, short weight);
    void randomizeColor(color *baseColor, short randomizePercent);
    void swapColors(color *color1, color *color2);
    void irisFadeBetweenBuffers(screenDisplayBuffer *fromBuf,
                                screenDisplayBuffer *toBuf,
                                short x, short y,
                                short frameCount,
                                boolean outsideIn);
    void colorBlendCell(short x, short y, const color *hiliteColor, short hiliteStrength);
    void hiliteCell(short x, short y, const color *hiliteColor, short hiliteStrength, boolean distinctColors);
    void colorMultiplierFromDungeonLight(short x, short y, color *editColor);
    void plotCharWithColor(enum displayGlyph inputChar, windowpos loc, const color *cellForeColor, const color *cellBackColor);
    void plotCharToBuffer(enum displayGlyph inputChar, windowpos loc, const color *foreColor, const color *backColor, screenDisplayBuffer *dbuf);
    void plotForegroundChar(enum displayGlyph inputChar, short x, short y, const color *foreColor, boolean affectedByLighting);
    void commitDraws(void);
    void dumpLevelToScreen(void);
    void hiliteCharGrid(char hiliteCharGrid[DCOLS][DROWS], const color *hiliteColor, short hiliteStrength);
    void blackOutScreen(void);
    void colorOverDungeon(const color *color);
    void copyDisplayBuffer(screenDisplayBuffer *toBuf, screenDisplayBuffer *fromBuf);
    void clearDisplayBuffer(screenDisplayBuffer *dbuf);
    color colorFromComponents(const char rgb[3]);
    // A `SavedDisplayBuffer` holds a previous version of the screen. It can be
    // Obtain one by calling `saveDisplayBuffer()` and restore it to the screen
    // by calling `restoreDisplayBuffer()`.
    typedef struct SavedDisplayBuffer {
        screenDisplayBuffer savedScreen;
    } SavedDisplayBuffer;
    SavedDisplayBuffer saveDisplayBuffer(void);
    void restoreDisplayBuffer(const SavedDisplayBuffer *savedBuf);
    void overlayDisplayBuffer(const screenDisplayBuffer *overBuf);
    void flashForeground(short *x, short *y, const color **flashColor, short *flashStrength, short count, short frames);
    void flashCell(const color *theColor, short frames, short x, short y);
    void colorFlash(const color *theColor, unsigned long reqTerrainFlags, unsigned long reqTileFlags, short frames, short maxRadius, short x, short y);
    void printString(const char *theString, short x, short y, const color *foreColor, const color* backColor, screenDisplayBuffer *dbuf);
    short wrapText(char *to, const char *sourceText, short width);
    short printStringWithWrapping(const char *theString, short x, short y, short width, const color *foreColor,
                                  const color *backColor, screenDisplayBuffer *dbuf);
    boolean getInputTextString(char *inputText,
                               const char *prompt,
                               short maxLength,
                               char *defaultEntry,
                               const char *promptSuffix,
                               short textEntryType,
                               boolean useDialogBox);
    boolean pauseBrogue(short milliseconds, PauseBehavior behavior);
    boolean pauseAnimation(short milliseconds, PauseBehavior behavior);
    void nextBrogueEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance, boolean realInputEvenInPlayback);
    void executeMouseClick(rogueEvent *theEvent);
    void executeKeystroke(signed long keystroke, boolean controlKey, boolean shiftKey);
    boolean placeStairs(pos *upStairsLoc);
    void initializeLevel(pos upStairsLoc);
    void startLevel (short oldLevelNumber, short stairDirection);
    void updateMinersLightRadius(void);
    void freeCreature(creature *monst);
    void freeCreatureList(creatureList *list);
    void removeDeadMonsters(void);
    void freeEverything(void);
    boolean randomMatchingLocation(pos *loc, short dungeonType, short liquidType, short terrainType);
    enum dungeonLayers highestPriorityLayer(short x, short y, boolean skipGas);
    enum dungeonLayers layerWithTMFlag(short x, short y, unsigned long flag);
    enum dungeonLayers layerWithFlag(short x, short y, unsigned long flag);
    const char *tileFlavor(short x, short y);
    const char *tileText(short x, short y);
    void describedItemBasedOnParameters(short theCategory, short theKind, short theQuantity, short theItemOriginDepth, char *buf);
    void describeLocation(char buf[DCOLS], short x, short y);
    void printLocationDescription(short x, short y);
    void useKeyAt(item *theItem, short x, short y);
    void playerRuns(short direction);
    void exposeCreatureToFire(creature *monst);
    void updateFlavorText(void);
    void updatePlayerUnderwaterness(void);
    boolean monsterShouldFall(creature *monst);
    void applyInstantTileEffectsToCreature(creature *monst);
    void vomit(creature *monst);
    void becomeAllyWith(creature *monst);
    void freeCaptive(creature *monst);
    boolean freeCaptivesEmbeddedAt(short x, short y);
    boolean handleWhipAttacks(creature *attacker, enum directions dir, boolean *aborted);
    boolean handleSpearAttacks(creature *attacker, enum directions dir, boolean *aborted);
    boolean diagonalBlocked(const short x1, const short y1, const short x2, const short y2, const boolean limitToPlayerKnowledge);
    boolean playerMoves(short direction);
    void calculateDistances(short **distanceMap,
                            short destinationX, short destinationY,
                            unsigned long blockingTerrainFlags,
                            creature *traveler,
                            boolean canUseSecretDoors,
                            boolean eightWays);
    short pathingDistance(short x1, short y1, short x2, short y2, unsigned long blockingTerrainFlags);
    short nextStep(short **distanceMap, short x, short y, creature *monst, boolean reverseDirections);
    void travelRoute(pos path[1000], short steps);
    void travel(short x, short y, boolean autoConfirm);
    void populateGenericCostMap(short **costMap);
    void getLocationFlags(const short x, const short y,
                          unsigned long *tFlags, unsigned long *TMFlags, unsigned long *cellFlags,
                          const boolean limitToPlayerKnowledge);
    void populateCreatureCostMap(short **costMap, creature *monst);
    enum directions adjacentFightingDir(void);
    void getExploreMap(short **map, boolean headingToStairs);
    boolean explore(short frameDelay);
    short getPlayerPathOnMap(pos path[1000], short **map, pos origin);
    void reversePath(pos path[1000], short steps);
    void hilitePath(pos path[1000], short steps, boolean unhilite);
    void clearCursorPath(void);
    void hideCursor(void);
    void showCursor(void);
    void mainInputLoop(void);
    boolean isDisturbed(short x, short y);
    void discover(short x, short y);
    short randValidDirectionFrom(creature *monst, short x, short y, boolean respectAvoidancePreferences);
    boolean exposeTileToElectricity(short x, short y);
    boolean exposeTileToFire(short x, short y, boolean alwaysIgnite);
    boolean cellCanHoldGas(short x, short y);
    void monstersFall(void);
    void updateEnvironment(void);
    void updateAllySafetyMap(void);
    void updateSafetyMap(void);
    void updateSafeTerrainMap(void);
    short staffChargeDuration(const item *theItem);
    void rechargeItemsIncrementally(short multiplier);
    void extinguishFireOnCreature(creature *monst);
    void autoRest(void);
    void manualSearch(void);
    boolean startFighting(enum directions dir, boolean tillDeath);
    void autoFight(boolean tillDeath);
    void synchronizePlayerTimeState(void);
    void playerRecoversFromAttacking(boolean anAttackHit);
    void playerTurnEnded(void);
    void resetScentTurnNumber(void);
    void displayMonsterFlashes(boolean flashingEnabled);
    void clearMessageArchive(void);
    void formatRecentMessages(char buf[][COLS*2], size_t height, short *linesFormatted, short *latestMessageLines);
    void displayRecentMessages(void);
    void displayMessageArchive(void);
    void temporaryMessage(const char *msg1, unsigned long flags);
    void messageWithColor(const char *msg, const color *theColor, unsigned long flags);
    void flavorMessage(const char *msg);
    void message(const char *msg, unsigned long flags);
    void displayMoreSignWithoutWaitingForAcknowledgment(void);
    void displayMoreSign(void);
    short encodeMessageColor(char *msg, short i, const color *theColor);
    short decodeMessageColor(const char *msg, short i, color *returnColor);
    const color *messageColorFromVictim(creature *monst);
    void upperCase(char *theChar);
    void updateMessageDisplay(void);
    void deleteMessages(void);
    void confirmMessages(void);
    void stripShiftFromMovementKeystroke(signed long *keystroke);

    void storeMemories(const short x, const short y);
    void updateFieldOfViewDisplay(boolean updateDancingTerrain, boolean refreshDisplay);
    void updateFieldOfView(short xLoc, short yLoc, short radius, boolean paintScent,
                           boolean passThroughCreatures, boolean setFieldOfView, short theColor[3], short fadeToPercent);
    void betweenOctant1andN(short *x, short *y, short x0, short y0, short n);

    void getFOVMask(char grid[DCOLS][DROWS], short xLoc, short yLoc, fixpt maxRadius,
                    unsigned long forbiddenTerrain, unsigned long forbiddenFlags, boolean cautiousOnWalls);
    void scanOctantFOV(char grid[DCOLS][DROWS], short xLoc, short yLoc, short octant, fixpt maxRadius,
                       short columnsRightFromOrigin, long startSlope, long endSlope, unsigned long forbiddenTerrain,
                       unsigned long forbiddenFlags, boolean cautiousOnWalls);

    creature *generateMonster(short monsterID, boolean itemPossible, boolean mutationPossible);
    void initializeMonster(creature *monst, boolean itemPossible);
    void mutateMonster(creature *monst, short mutationIndex);
    short chooseMonster(short forLevel);
    creature *spawnHorde(short hordeID, pos loc, unsigned long forbiddenFlags, unsigned long requiredFlags);
    void fadeInMonster(creature *monst);

    creatureList createCreatureList(void);
    creatureIterator iterateCreatures(creatureList *list);
    boolean hasNextCreature(creatureIterator iter);
    creature *nextCreature(creatureIterator *iter);
    void prependCreature(creatureList *list, creature *add);
    boolean removeCreature(creatureList *list, creature *remove);
    creature *firstCreature(creatureList *list);

    boolean canNegateCreatureStatusEffects(creature *monst);
    void negateCreatureStatusEffects(creature *monst);
    boolean monsterIsNegatable(creature *monst);

    boolean monsterWillAttackTarget(const creature *attacker, const creature *defender);
    boolean monstersAreTeammates(const creature *monst1, const creature *monst2);
    boolean monstersAreEnemies(const creature *monst1, const creature *monst2);
    void initializeGender(creature *monst);
    boolean stringsMatch(const char *str1, const char *str2);
    void resolvePronounEscapes(char *text, creature *monst);
    short pickHordeType(short depth, enum monsterTypes summonerType, unsigned long forbiddenFlags, unsigned long requiredFlags);
    creature *cloneMonster(creature *monst, boolean announce, boolean placeClone);
    void empowerMonster(creature *monst);
    unsigned long forbiddenFlagsForMonster(creatureType *monsterType);
    unsigned long avoidedFlagsForMonster(creatureType *monsterType);
    boolean monsterCanSubmergeNow(creature *monst);
    void populateMonsters(void);
    void updateMonsterState(creature *monst);
    void decrementMonsterStatus(creature *monst);
    boolean specifiedPathBetween(short x1, short y1, short x2, short y2,
                                 unsigned long blockingTerrain, unsigned long blockingFlags);
    boolean traversiblePathBetween(creature *monst, short x2, short y2);
    boolean openPathBetween(const pos startLoc, const pos targetLoc);
    creature *monsterAtLoc(pos p);
    creature *dormantMonsterAtLoc(pos p);
    pos perimeterCoords(short n);
    boolean monsterBlinkToPreferenceMap(creature *monst, short **preferenceMap, boolean blinkUphill);
    boolean monsterSummons(creature *monst, boolean alwaysUse);
    boolean resurrectAlly(const pos loc);
    void unAlly(creature *monst);
    boolean monsterFleesFrom(creature *monst, creature *defender);
    void monstersTurn(creature *monst);
    boolean getRandomMonsterSpawnLocation(short *x, short *y);
    void spawnPeriodicHorde(void);
    void initializeStatus(creature *monst);
    void handlePaladinFeat(creature *defender);
    void moralAttack(creature *attacker, creature *defender);
    void splitMonster(creature *monst, creature *attacker);
    short runicWeaponChance(item *theItem, boolean customEnchantLevel, fixpt enchantLevel);
    void magicWeaponHit(creature *defender, item *theItem, boolean backstabbed);
    void disentangle(creature *monst);
    void teleport(creature *monst, pos destination, boolean respectTerrainAvoidancePreferences);
    void chooseNewWanderDestination(creature *monst);
    boolean canPass(creature *mover, creature *blocker);
    boolean isPassableOrSecretDoor(pos loc);
    boolean knownToPlayerAsPassableOrSecretDoor(pos loc);
    void setMonsterLocation(creature *monst, pos newLoc);
    boolean moveMonster(creature *monst, short dx, short dy);
    unsigned long burnedTerrainFlagsAtLoc(pos loc);
    unsigned long discoveredTerrainFlagsAtLoc(pos loc);
    boolean monsterAvoids(creature *monst, pos p);
    short distanceBetween(pos loc1, pos loc2);
    void alertMonster(creature *monst);
    void wakeUp(creature *monst);
    boolean monsterRevealed(creature *monst);
    boolean monsterHiddenBySubmersion(const creature *monst, const creature *observer);
    boolean monsterIsHidden(const creature *monst, const creature *observer);
    boolean canSeeMonster(creature *monst);
    boolean canDirectlySeeMonster(creature *monst);
    void monsterName(char *buf, creature *monst, boolean includeArticle);
    boolean monsterIsInClass(const creature *monst, const short monsterClass);
    boolean chooseTarget(pos *returnLoc, short maxDistance, enum autoTargetMode targetingMode, const item *theItem);
    fixpt strengthModifier(item *theItem);
    fixpt netEnchant(item *theItem);
    short hitProbability(creature *attacker, creature *defender);
    boolean attackHit(creature *attacker, creature *defender);
    void applyArmorRunicEffect(char returnString[DCOLS], creature *attacker, short *damage, boolean melee);
    void processStaggerHit(creature *attacker, creature *defender);
    boolean attack(creature *attacker, creature *defender, boolean lungeAttack);
    void inflictLethalDamage(creature *attacker, creature *defender);
    boolean inflictDamage(creature *attacker, creature *defender,
                          short damage, const color *flashColor, boolean ignoresProtectionShield);
    void addPoison(creature *monst, short totalDamage, short concentrationIncrement);
    void killCreature(creature *decedent, boolean administrativeDeath);
    void buildHitList(const creature **hitList, const creature *attacker, creature *defender, const boolean sweep);
    void addScentToCell(short x, short y, short distance);
    void populateItems(pos upstairs);
    item *placeItemAt(item *theItem, pos dest);
    void removeItemAt(pos loc);
    void pickUpItemAt(pos loc);
    item *addItemToPack(item *theItem);
    void aggravateMonsters(short distance, short x, short y, const color *flashColor);
    short getLineCoordinates(pos listOfCoordinates[], const pos originLoc, const pos targetLoc, const bolt *theBolt);
    void getImpactLoc(pos *returnLoc, const pos originLoc, const pos targetLoc,
                      const short maxDistance, const boolean returnLastEmptySpace, const bolt *theBolt);
    boolean negate(creature *monst);
    short monsterAccuracyAdjusted(const creature *monst);
    fixpt monsterDamageAdjustmentAmount(const creature *monst);
    short monsterDefenseAdjusted(const creature *monst);
    void weaken(creature *monst, short maxDuration);
    void slow(creature *monst, short turns);
    void haste(creature *monst, short turns);
    void heal(creature *monst, short percent, boolean panacea);
    boolean projectileReflects(creature *attacker, creature *defender);
    short reflectBolt(short targetX, short targetY, pos listOfCoordinates[], short kinkCell, boolean retracePath);
    void checkForMissingKeys(short x, short y);
    enum boltEffects boltEffectForItem(item *theItem);
    enum boltType boltForItem(item *theItem);
    boolean zap(pos originLoc, pos targetLoc, bolt *theBolt, boolean hideDetails, boolean reverseBoltDir);
    boolean nextTargetAfter(const item *theItem,
                            pos *returnLoc,
                            pos targetLoc,
                            enum autoTargetMode targetingMode,
                            boolean reverseDirection);
    boolean moveCursor(boolean *targetConfirmed,
                       boolean *canceled,
                       boolean *tabKey,
                       pos *targetLoc,
                       rogueEvent *event,
                       buttonState *state,
                       boolean colorsDance,
                       boolean keysMoveCursor,
                       boolean targetCanLeaveMap);
    void identifyItemKind(item *theItem);
    void autoIdentify(item *theItem);
    short numberOfItemsInPack(void);
    char nextAvailableInventoryCharacter(void);
    void checkForDisenchantment(item *theItem);
    void updateFloorItems(void);
    void itemKindName(item *theItem, char *kindName);
    void itemRunicName(item *theItem, char *runicName);
    void itemName(item *theItem, char *root, boolean includeDetails, boolean includeArticle, const color *baseColor);
    int itemKindCount(enum itemCategory category, int magicPolarity);
    char displayInventory(unsigned short categoryMask,
                          unsigned long requiredFlags,
                          unsigned long forbiddenFlags,
                          boolean waitForAcknowledge,
                          boolean includeButtons);
    short numberOfMatchingPackItems(unsigned short categoryMask,
                                    unsigned long requiredFlags, unsigned long forbiddenFlags,
                                    boolean displayErrors);
    void clearInventory(char keystroke);
    item *initializeItem(void);
    item *generateItem(unsigned short theCategory, short theKind);
    short chooseKind(const itemTable *theTable, short numKinds);
    item *makeItemInto(item *theItem, unsigned long itemCategory, short itemKind);
    void updateEncumbrance(void);
    short displayedArmorValue(void);
    short armorValueIfUnenchanted(item *theItem);
    void strengthCheck(item *theItem, boolean noisy);
    void recalculateEquipmentBonuses(void);
    boolean equipItem(item *theItem, boolean force, item *unequipHint);
    void equip(item *theItem);
    item *keyInPackFor(pos loc);
    item *keyOnTileAt(pos loc);
    void unequip(item *theItem);
    void drop(item *theItem);
    void findAlternativeHomeFor(creature *monst, short *x, short *y, boolean chooseRandomly);
    boolean getQualifyingLocNear(pos *loc,
                                 pos target,
                                 boolean hallwaysAllowed,
                                 char blockingMap[DCOLS][DROWS],
                                 unsigned long forbiddenTerrainFlags,
                                 unsigned long forbiddenMapFlags,
                                 boolean forbidLiquid,
                                 boolean deterministic);
    boolean getQualifyingGridLocNear(pos *loc,
                                     pos target,
                                     boolean grid[DCOLS][DROWS],
                                     boolean deterministic);

    // Grid operations
    short **allocGrid(void);
    void freeGrid(short **array);
    void copyGrid(short **to, short **from);
    void fillGrid(short **grid, short fillValue);
    void hiliteGrid(short **grid, const color *hiliteColor, short hiliteStrength);
    void findReplaceGrid(short **grid, short findValueMin, short findValueMax, short fillValue);
    short floodFillGrid(short **grid, short x, short y, short eligibleValueMin, short eligibleValueMax, short fillValue);
    void drawRectangleOnGrid(short **grid, short x, short y, short width, short height, short value);
    void drawCircleOnGrid(short **grid, short x, short y, short radius, short value);
    void getTerrainGrid(short **grid, short value, unsigned long terrainFlags, unsigned long mapFlags);
    void getTMGrid(short **grid, short value, unsigned long TMflags);
    short validLocationCount(short **grid, short validValue);
    void randomLocationInGrid(short **grid, short *x, short *y, short validValue);
    boolean getQualifyingPathLocNear(short *retValX, short *retValY,
                                     short x, short y,
                                     boolean hallwaysAllowed,
                                     unsigned long blockingTerrainFlags,
                                     unsigned long blockingMapFlags,
                                     unsigned long forbiddenTerrainFlags,
                                     unsigned long forbiddenMapFlags,
                                     boolean deterministic);
    void createBlobOnGrid(short **grid,
                          short *retMinX, short *retMinY, short *retWidth, short *retHeight,
                          short roundCount,
                          short minBlobWidth, short minBlobHeight,
                          short maxBlobWidth, short maxBlobHeight, short percentSeeded,
                          char birthParameters[9], char survivalParameters[9]);

    void checkForContinuedLeadership(creature *monst);
    void demoteMonsterFromLeadership(creature *monst);
    void toggleMonsterDormancy(creature *monst);
    void monsterDetails(char buf[], creature *monst);
    void makeMonsterDropItem(creature *monst);
    void throwCommand(item *theItem, boolean autoThrow);
    void relabel(item *theItem);
    void swapLastEquipment(void);
    void apply(item *theItem);
    boolean eat(item *theItem, boolean recordCommands);
    boolean itemCanBeCalled(item *theItem);
    void call(item *theItem);
    short chooseVorpalEnemy(void);
    void describeMonsterClass(char *buf, const short classID, boolean conjunctionAnd);
    void identify(item *theItem);
    void updateIdentifiableItem(item *theItem);
    void updateIdentifiableItems(void);
    boolean readScroll(item *theItem);
    void updateRingBonuses(void);
    void updatePlayerRegenerationDelay(void);
    boolean removeItemFromChain(item *theItem, item *theChain);
    void addItemToChain(item *theItem, item *theChain);
    boolean drinkPotion(item *theItem);
    item *promptForItemOfType(unsigned short category,
                              unsigned long requiredFlags,
                              unsigned long forbiddenFlags,
                              char *prompt,
                              boolean allowInventoryActions);
    item *itemOfPackLetter(char letter);
    boolean unequipItem(item *theItem, boolean force);
    short magicCharDiscoverySuffix(short category, short kind);
    int itemMagicPolarity(item *theItem);
    item *itemAtLoc(pos loc);
    item *dropItem(item *theItem);
    itemTable *tableForItemCategory(enum itemCategory theCat);
    boolean isVowelish(char *theChar);
    short charmEffectDuration(short charmKind, short enchant);
    short charmRechargeDelay(short charmKind, short enchant);
    boolean itemIsCarried(item *theItem);
    void itemDetails(char *buf, item *theItem);
    void deleteItem(item *theItem);
    void shuffleFlavors(void);
    unsigned long itemValue(item *theItem);
    short strLenWithoutEscapes(const char *str);
    void combatMessage(char *theMsg, const color *theColor);
    void displayCombatText(void);
    void flashMonster(creature *monst, const color *theColor, short strength);

    void logLights(void);
    boolean paintLight(const lightSource *theLight, short x, short y, boolean isMinersLight, boolean maintainShadows);
    void backUpLighting(short lights[DCOLS][DROWS][3]);
    void restoreLighting(short lights[DCOLS][DROWS][3]);
    void updateLighting(void);
    boolean playerInDarkness(void);
    flare *newFlare(const lightSource *light, short x, short y, short changePerFrame, short limit);
    void createFlare(short x, short y, enum lightType lightIndex);
    void animateFlares(flare **flares, short count);
    void deleteAllFlares(void);
    void demoteVisibility(void);
    void discoverCell(const short x, const short y);
    void updateVision(boolean refreshDisplay);
    void burnItem(item *theItem);
    void activateMachine(short machineNumber);
    boolean circuitBreakersPreventActivation(short machineNumber);
    void promoteTile(short x, short y, enum dungeonLayers layer, boolean useFireDF);
    void autoPlayLevel(boolean fastForward);
    void updateClairvoyance(void);
    short scentDistance(short x1, short y1, short x2, short y2);
    short armorStealthAdjustment(item *theArmor);
    short currentStealthRange(void);

    void initRecording(void);
    void flushBufferToFile(void);
    void fillBufferFromFile(void);
    void recordEvent(rogueEvent *event);
    void recallEvent(rogueEvent *event);
    void pausePlayback(void);
    void displayAnnotation(void);
    boolean loadSavedGame(void);
    void switchToPlaying(void);
    void recordKeystroke(int keystroke, boolean controlKey, boolean shiftKey);
    void cancelKeystroke(void);
    void recordKeystrokeSequence(unsigned char *commandSequence);
    void recordMouseClick(short x, short y, boolean controlKey, boolean shiftKey);
    void OOSCheck(unsigned long x, short numberOfBytes);
    void RNGCheck(void);
    boolean executePlaybackInput(rogueEvent *recordingInput);
    void getAvailableFilePath(char *filePath, const char *defaultPath, const char *suffix);
    boolean characterForbiddenInFilename(const char theChar);
    void saveGame(void);
    void saveGameNoPrompt(void);
    void saveRecording(char *filePath);
    void saveRecordingNoPrompt(char *filePath);
    void parseFile(void);
    void RNGLog(char *message);

    short wandDominate(creature *monst);
    short staffDamageLow(fixpt enchant);
    short staffDamageHigh(fixpt enchant);
    short staffDamage(fixpt enchant);
    int staffPoison(fixpt enchant);
    short staffBlinkDistance(fixpt enchant);
    short staffHasteDuration(fixpt enchant);
    short staffBladeCount(fixpt enchant);
    short staffDiscordDuration(fixpt enchant);
    int staffProtection(fixpt enchant);
    short staffEntrancementDuration(fixpt enchant);
    fixpt ringWisdomMultiplier(fixpt enchant);
    short charmHealing(fixpt enchant);
    int charmProtection(fixpt enchant);
    short charmShattering(fixpt enchant);
    short charmGuardianLifespan(fixpt enchant);
    short charmNegationRadius(fixpt enchant);
    short weaponParalysisDuration(fixpt enchant);
    short weaponConfusionDuration(fixpt enchant);
    short weaponForceDistance(fixpt enchant);
    short weaponSlowDuration(fixpt enchant);
    short weaponImageCount(fixpt enchant);
    short weaponImageDuration(fixpt enchant);
    short armorReprisalPercent(fixpt enchant);
    short armorAbsorptionMax(fixpt enchant);
    short armorImageCount(fixpt enchant);
    short reflectionChance(fixpt enchant);
    long turnsForFullRegenInThousandths(fixpt bonus);
    fixpt damageFraction(fixpt netEnchant);
    fixpt accuracyFraction(fixpt netEnchant);
    fixpt defenseFraction(fixpt netDefense);

    void checkForDungeonErrors(void);

    boolean dialogChooseFile(char *path, const char *suffix, const char *prompt);
    void dialogCreateItemOrMonster(void);
    int quitImmediately(void);
    void dialogAlert(char *message);
    void mainBrogueJunction(void);
    int printSeedCatalog(uint64_t startingSeed, uint64_t numberOfSeedsToScan, unsigned int scanThroughDepth, boolean isCsvFormat, char *errorMessage);

    void initializeButton(brogueButton *button);
    void drawButtonsInState(buttonState *state, screenDisplayBuffer *button_dbuf);
    void initializeButtonState(buttonState *state,
                               brogueButton *buttons,
                               short buttonCount,
                               short winX,
                               short winY,
                               short winWidth,
                               short winHeight);
    short processButtonInput(buttonState *state, boolean *canceled, rogueEvent *event);
    short smoothHiliteGradient(const short currentXValue, const short maxXValue);
    void drawButton(brogueButton *button, enum buttonDrawStates highlight, screenDisplayBuffer* dbuf);
    short buttonInputLoop(brogueButton *buttons,
                          short buttonCount,
                          short winX,
                          short winY,
                          short winWidth,
                          short winHeight,
                          rogueEvent *returnEvent);

    void dijkstraScan(short **distanceMap, short **costMap, boolean useDiagonals);

#if defined __cplusplus
}
#endif

#endif

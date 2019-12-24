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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "PlatformDefines.h"

// unicode: comment this line to revert to ASCII

#define USE_UNICODE

// Brogue version: what the user sees in the menu and title
#define BROGUE_VERSION_STRING "CE 1.8.1"
// Recording version: replay/saves from a different version won't load
// Cannot be longer than 16 chars
#define BROGUE_RECORDING_VERSION_STRING "CE 1.8"

// debug macros -- define DEBUGGING as 1 to enable wizard mode.

#ifndef DEBUGGING
#define DEBUGGING                       0
#endif

#define DEBUG                           if (DEBUGGING)
#define MONSTERS_ENABLED                (!DEBUGGING || 1) // Quest room monsters can be generated regardless.
#define ITEMS_ENABLED                   (!DEBUGGING || 1)

#define D_BULLET_TIME                   (DEBUGGING && 0)
#define D_WORMHOLING                    (DEBUGGING && 1)
#define D_IMMORTAL                      (DEBUGGING && 1)

#define D_SAFETY_VISION                 (DEBUGGING && 0)
#define D_SCENT_VISION                  (DEBUGGING && 0)
#define D_DISABLE_BACKGROUND_COLORS     (DEBUGGING && 0)

#define D_INSPECT_LEVELGEN              (DEBUGGING && 0)
#define D_INSPECT_MACHINES              (DEBUGGING && 0)

#define D_MESSAGE_ITEM_GENERATION       (DEBUGGING && 0)
#define D_MESSAGE_MACHINE_GENERATION    (DEBUGGING && 0)

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
#define LAST_GAME_PATH          "LastGame.broguesave"
#define LAST_GAME_NAME          "LastGame"
#define LAST_RECORDING_NAME     "LastRecording"
#define RECORDING_SUFFIX        ".broguerec"
#define GAME_SUFFIX             ".broguesave"
#define ANNOTATION_SUFFIX       ".txt"
#define RNG_LOG                 "RNGLog.txt"

#define BROGUE_FILENAME_MAX     (min(1024*4, FILENAME_MAX))

// Date format used when listing recordings and high scores
#define DATE_FORMAT             "%Y-%m-%d" // see strftime() documentation

// Allows unicode characters:
#define uchar                   unsigned short

#define MESSAGE_LINES           3
#define MESSAGE_ARCHIVE_LINES   ROWS

// Size of the entire terminal window. These need to be hard-coded here and in Viewport.h
#define COLS                    100
#define ROWS                    (31 + MESSAGE_LINES)

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

#define AMULET_LEVEL            26          // how deep before the amulet appears
#define DEEPEST_LEVEL           40          // how deep the universe goes

#define MACHINES_FACTOR         FP_FACTOR   // use this to adjust machine frequency

#define MACHINES_BUFFER_LENGTH  200

#define WEAPON_KILLS_TO_AUTO_ID 20
#define ARMOR_DELAY_TO_AUTO_ID  1000
#define RING_DELAY_TO_AUTO_ID   1500

#define FALL_DAMAGE_MIN         8
#define FALL_DAMAGE_MAX         10

#define INPUT_RECORD_BUFFER     1000        // how many bytes of input data to keep in memory before saving it to disk
#define DEFAULT_PLAYBACK_DELAY  50

#define HIGH_SCORES_COUNT       30

// color escapes
#define COLOR_ESCAPE            25
#define COLOR_VALUE_INTERCEPT   25

// display characters:

#ifdef USE_UNICODE

#define FLOOR_CHAR      0x00b7
#define LIQUID_CHAR     '~'
#define CHASM_CHAR      0x2237
#define TRAP_CHAR       0x25c7
#define FIRE_CHAR       0x22CF
#define GRASS_CHAR      '"'
#define BRIDGE_CHAR     '='
#define DESCEND_CHAR    '>'
#define ASCEND_CHAR     '<'
#define WALL_CHAR       '#'
#define DOOR_CHAR       '+'
#define OPEN_DOOR_CHAR  '\''
#define ASH_CHAR        '\''
#define BONES_CHAR      ','
#define MUD_CHAR        ','
#define WEB_CHAR        ':'
//#define FOLIAGE_CHAR  0x03A8 // lower-case psi
#define FOLIAGE_CHAR    0x2648 // Aries symbol
#define VINE_CHAR       ':'
#define ALTAR_CHAR      '|'
#define LEVER_CHAR      '/'
#define LEVER_PULLED_CHAR '\\'
#define STATUE_CHAR     0x00df
#define VENT_CHAR       '='
#define DEWAR_CHAR      '&'

#define TRAMPLED_FOLIAGE_CHAR   '"'     // 0x2034 // 0x2037

#define PLAYER_CHAR     '@'

#define AMULET_CHAR     0x2640
#define FOOD_CHAR       ';'
#define SCROLL_CHAR     0x266A//'?'     // 0x039E
#define RING_CHAR       0x26AA
//#define RING_CHAR     0xffee
#define CHARM_CHAR      0x03DF
#define POTION_CHAR     '!'
#define ARMOR_CHAR      '['
#define WEAPON_CHAR     0x2191
#define STAFF_CHAR      '\\'
#define WAND_CHAR       '~'
#define GOLD_CHAR       '*'
#define GEM_CHAR        0x25cf
#define TOTEM_CHAR      0x26b2
#define TURRET_CHAR     0x25cf
#define UNICORN_CHAR    0x00da
#define KEY_CHAR        '-'
#define ELECTRIC_CRYSTAL_CHAR 0x00A4

#define UP_ARROW_CHAR       0x2191
#define DOWN_ARROW_CHAR     0x2193
#define LEFT_ARROW_CHAR     0x2190
#define RIGHT_ARROW_CHAR    0x2192
#define UP_TRIANGLE_CHAR    0x2206
#define DOWN_TRIANGLE_CHAR  0x2207
#define OMEGA_CHAR          0x03A9
#define THETA_CHAR          0x03B8
#define LAMDA_CHAR          0x03BB
#define KOPPA_CHAR          0x03DE
#define LOZENGE_CHAR        0x29EB
#define CROSS_PRODUCT_CHAR  0x2A2F

#define CHAIN_TOP_LEFT      '\\'
#define CHAIN_BOTTOM_RIGHT  '\\'
#define CHAIN_TOP_RIGHT     '/'
#define CHAIN_BOTTOM_LEFT   '/'
#define CHAIN_TOP           '|'
#define CHAIN_BOTTOM        '|'
#define CHAIN_LEFT          '-'
#define CHAIN_RIGHT         '-'

#define BAD_MAGIC_CHAR      0x29F2
#define GOOD_MAGIC_CHAR     0x29F3

#else

#define FLOOR_CHAR      '.'
#define LIQUID_CHAR     '~'
#define CHASM_CHAR      ':'
#define TRAP_CHAR       '%'
#define FIRE_CHAR       '^'
#define GRASS_CHAR      '"'
#define BRIDGE_CHAR     '='
#define DESCEND_CHAR    '>'
#define ASCEND_CHAR     '<'
#define WALL_CHAR       '#'
#define DOOR_CHAR       '+'
#define OPEN_DOOR_CHAR  '\''
#define ASH_CHAR        '\''
#define BONES_CHAR      ','
#define MUD_CHAR        ','
#define WEB_CHAR        ':'
#define FOLIAGE_CHAR    '&'
#define VINE_CHAR       ':'
#define ALTAR_CHAR      '|'
#define LEVER_CHAR      '/'
#define LEVER_PULLED_CHAR '\\'
#define STATUE_CHAR     '&'
#define VENT_CHAR       '='
#define DEWAR_CHAR      '&'

#define TRAMPLED_FOLIAGE_CHAR   '"'

#define PLAYER_CHAR     '@'

#define AMULET_CHAR     ','
#define FOOD_CHAR       ';'
#define SCROLL_CHAR     '?'
#define RING_CHAR       '='
#define CHARM_CHAR      '+'
#define POTION_CHAR     '!'
#define ARMOR_CHAR      '['
#define WEAPON_CHAR     '('
#define STAFF_CHAR      '\\'
#define WAND_CHAR       '~'
#define GOLD_CHAR       '*'
#define GEM_CHAR        '+'
#define TOTEM_CHAR      '0'
#define TURRET_CHAR     '*'
#define UNICORN_CHAR    'U'
#define KEY_CHAR        '-'
#define ELECTRIC_CRYSTAL_CHAR '$'

#define UP_ARROW_CHAR       '^'
#define DOWN_ARROW_CHAR     'v'
#define LEFT_ARROW_CHAR     '<'
#define RIGHT_ARROW_CHAR    '>'
#define UP_TRIANGLE_CHAR    '^'
#define DOWN_TRIANGLE_CHAR  'v'
#define OMEGA_CHAR          '^'
#define THETA_CHAR          '0'
#define LAMDA_CHAR          '\\'
#define KOPPA_CHAR          'k'
#define LOZENGE_CHAR        '+'
#define CROSS_PRODUCT_CHAR  'x'

#define CHAIN_TOP_LEFT      '\\'
#define CHAIN_BOTTOM_RIGHT  '\\'
#define CHAIN_TOP_RIGHT     '/'
#define CHAIN_BOTTOM_LEFT   '/'
#define CHAIN_TOP           '|'
#define CHAIN_BOTTOM        '|'
#define CHAIN_LEFT          '-'
#define CHAIN_RIGHT         '-'

#define BAD_MAGIC_CHAR      '+'
#define GOOD_MAGIC_CHAR     '$'

#endif

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

    CAN_BE_DETECTED     = (WEAPON | ARMOR | POTION | SCROLL | RING | CHARM | WAND | STAFF | AMULET),
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
    NUMBER_POTION_KINDS
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
    WAND_EMPOWERMENT,
    NUMBER_WAND_KINDS
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
    BOLT_WHIP,
    NUMBER_BOLT_KINDS
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
    CHARM_NEGATION,
    NUMBER_CHARM_KINDS
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
    NUMBER_SCROLL_KINDS
};

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

#define NUMBER_HORDES               177

#define MONSTER_CLASS_COUNT         15

// flavors

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
                            | HAS_STAIRS | SEARCHED_FROM_HERE | PRESSURE_PLATE_DEPRESSED
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
#define RELABEL_KEY         'R'
#define TRUE_COLORS_KEY     '\\'
#define AGGRO_DISPLAY_KEY   ']'
#define DROP_KEY            'd'
#define CALL_KEY            'c'
#define QUIT_KEY            'Q'
#define MESSAGE_ARCHIVE_KEY 'M'
#define HELP_KEY            '?'
#define DISCOVERIES_KEY     'D'
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

#define UNKNOWN_KEY         (128+19)

#define min(x, y)       (((x) < (y)) ? (x) : (y))
#define max(x, y)       (((x) > (y)) ? (x) : (y))
#define clamp(x, low, hi)   (min(hi, max(x, low))) // pins x to the [y, z] interval

#define terrainFlags(x, y)                  (tileCatalog[pmap[x][y].layers[DUNGEON]].flags \
                                            | tileCatalog[pmap[x][y].layers[LIQUID]].flags \
                                            | tileCatalog[pmap[x][y].layers[SURFACE]].flags \
                                            | tileCatalog[pmap[x][y].layers[GAS]].flags)

#define terrainMechFlags(x, y)              (tileCatalog[pmap[x][y].layers[DUNGEON]].mechFlags \
                                            | tileCatalog[pmap[x][y].layers[LIQUID]].mechFlags \
                                            | tileCatalog[pmap[x][y].layers[SURFACE]].mechFlags \
                                            | tileCatalog[pmap[x][y].layers[GAS]].mechFlags)

#ifdef BROGUE_ASSERTS
boolean cellHasTerrainFlag(short x, short y, unsigned long flagMask);
#else
#define cellHasTerrainFlag(x, y, flagMask)  ((flagMask) & terrainFlags((x), (y)) ? true : false)
#endif
#define cellHasTMFlag(x, y, flagMask)       ((flagMask) & terrainMechFlags((x), (y)) ? true : false)

#define cellHasTerrainType(x, y, terrain)   ((pmap[x][y].layers[DUNGEON] == (terrain) \
                                            || pmap[x][y].layers[LIQUID] == (terrain) \
                                            || pmap[x][y].layers[SURFACE] == (terrain) \
                                            || pmap[x][y].layers[GAS] == (terrain)) ? true : false)

#define cellHasKnownTerrainFlag(x, y, flagMask) ((flagMask) & pmap[(x)][(y)].rememberedTerrainFlags ? true : false)

#define cellIsPassableOrDoor(x, y)          (!cellHasTerrainFlag((x), (y), T_PATHING_BLOCKER) \
                                            || (cellHasTMFlag((x), (y), (TM_IS_SECRET | TM_PROMOTES_WITH_KEY | TM_CONNECTS_LEVEL)) \
                                                && cellHasTerrainFlag((x), (y), T_OBSTRUCTS_PASSABILITY)))

#define coordinatesAreInMap(x, y)           ((x) >= 0 && (x) < DCOLS    && (y) >= 0 && (y) < DROWS)
#define coordinatesAreInWindow(x, y)        ((x) >= 0 && (x) < COLS     && (y) >= 0 && (y) < ROWS)
#define mapToWindowX(x)                     ((x) + STAT_BAR_WIDTH + 1)
#define mapToWindowY(y)                     ((y) + MESSAGE_LINES)
#define windowToMapX(x)                     ((x) - STAT_BAR_WIDTH - 1)
#define windowToMapY(y)                     ((y) - MESSAGE_LINES)

#define playerCanDirectlySee(x, y)          (pmap[x][y].flags & VISIBLE)
#define playerCanSee(x, y)                  (pmap[x][y].flags & ANY_KIND_OF_VISIBLE)
#define playerCanSeeOrSense(x, y)           ((pmap[x][y].flags & ANY_KIND_OF_VISIBLE) \
                                            || (rogue.playbackOmniscience \
                                                && (pmap[x][y].layers[DUNGEON] != GRANITE || (pmap[x][y].flags & DISCOVERED))))

#define CYCLE_MONSTERS_AND_PLAYERS(x)       for ((x) = &player; (x) != NULL; (x) = ((x) == &player ? monsters->nextCreature : (x)->nextCreature))

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
    uchar character;
    char foreColorComponents[3];
    char backColorComponents[3];
    char opacity;
    boolean needsUpdate;
} cellDisplayBuffer;

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
    short x;
    short y;
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
    uchar displayChar;
    color *foreColor;
    color *inventoryColor;
    short quantity;
    char inventoryLetter;
    char inscription[DCOLS];
    short xLoc;
    short yLoc;
    keyLocationProfile keyLoc[KEY_ID_MAXIMUM];
    short originDepth;
    struct item *nextItem;
} item;

typedef struct itemTable {
    char *name;
    char *flavor;
    char callTitle[30];
    short frequency;
    short marketValue;
    short strengthRequired;
    randomRange range;
    boolean identified;
    boolean called;
    char description[1500];
} itemTable;

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
    lightSource *light;                 // Flare light
    short coeffChangeAmount;            // The constant amount by which the coefficient changes per frame, e.g. -25 means it gets 25% dimmer per frame.
    short coeffLimit;                   // Flare ends if the coefficient passes this percentage (whether going up or down).
    short xLoc, yLoc;                   // Current flare location.
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
    uchar theChar;
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
    uchar displayChar;
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
    TM_PROMOTES_ON_STEP             = Fl(3),        // promotes when a creature, player or item is on the tile (whether or not levitating)
    TM_PROMOTES_ON_ITEM_PICKUP      = Fl(4),        // promotes when an item is lifted from the tile (primarily for altars)
    TM_PROMOTES_ON_PLAYER_ENTRY     = Fl(5),        // promotes when the player enters the tile (whether or not levitating)
    TM_PROMOTES_ON_SACRIFICE_ENTRY  = Fl(6),        // promotes when the sacrifice target enters the tile (whether or not levitating)
    TM_PROMOTES_ON_ELECTRICITY      = Fl(7),        // promotes when hit by a lightning bolt
    TM_ALLOWS_SUBMERGING            = Fl(8),        // allows submersible monsters to submerge in this terrain
    TM_IS_WIRED                     = Fl(9),        // if wired, promotes when powered, and sends power when promoting
    TM_IS_CIRCUIT_BREAKER           = Fl(10),       // prevents power from circulating in its machine
    TM_GAS_DISSIPATES               = Fl(11),       // does not just hang in the air forever
    TM_GAS_DISSIPATES_QUICKLY       = Fl(12),       // dissipates quickly
    TM_EXTINGUISHES_FIRE            = Fl(13),       // extinguishes burning terrain or creatures
    TM_VANISHES_UPON_PROMOTION      = Fl(14),       // vanishes when creating promotion dungeon feature, even if the replacement terrain priority doesn't require it
    TM_REFLECTS_BOLTS               = Fl(15),       // magic bolts reflect off of its surface randomly (similar to pmap flag IMPREGNABLE)
    TM_STAND_IN_TILE                = Fl(16),       // earthbound creatures will be said to stand "in" the tile, not on it
    TM_LIST_IN_SIDEBAR              = Fl(17),       // terrain will be listed in the sidebar with a description of the terrain type
    TM_VISUALLY_DISTINCT            = Fl(18),       // terrain will be color-adjusted if necessary so the character stands out from the background
    TM_BRIGHT_MEMORY                = Fl(19),       // no blue fade when this tile is out of sight
    TM_EXPLOSIVE_PROMOTE            = Fl(20),       // when burned, will promote to promoteType instead of burningType if surrounded by tiles with T_IS_FIRE or TM_EXPLOSIVE_PROMOTE
    TM_CONNECTS_LEVEL               = Fl(21),       // will be treated as passable for purposes of calculating level connectedness, irrespective of other aspects of this terrain layer
    TM_INTERRUPT_EXPLORATION_WHEN_SEEN = Fl(22),    // will generate a message when discovered during exploration to interrupt exploration
    TM_INVERT_WHEN_HIGHLIGHTED      = Fl(23),       // will flip fore and back colors when highlighted with pathing
    TM_SWAP_ENCHANTS_ACTIVATION     = Fl(24),       // in machine, swap item enchantments when two suitable items are on this terrain, and activate the machine when that happens
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
    STATUS_MAGICAL_FEAR,
    STATUS_ENTRANCED,
    STATUS_DARKNESS,
    STATUS_LIFESPAN_REMAINING,
    STATUS_SHIELDED,
    STATUS_INVISIBLE,
    STATUS_AGGRAVATING,
    NUMBER_OF_STATUS_EFFECTS,
};

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
    MONST_REFLECT_4                 = Fl(15),   // monster reflects projectiles as though wearing +4 armor of reflection
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
                                       | MONST_FLITS | MONST_IMMUNE_TO_FIRE | MONST_REFLECT_4 | MONST_FIERY | MONST_MAINTAINS_DISTANCE),
    MONST_TURRET                    = (MONST_IMMUNE_TO_WEBS | MONST_NEVER_SLEEPS | MONST_IMMOBILE | MONST_INANIMATE |
                                       MONST_ATTACKABLE_THRU_WALLS | MONST_WILL_NOT_USE_STAIRS),
    LEARNABLE_BEHAVIORS             = (MONST_INVISIBLE | MONST_FLIES | MONST_IMMUNE_TO_FIRE | MONST_REFLECT_4),
    MONST_NEVER_VORPAL_ENEMY        = (MONST_INANIMATE | MONST_INVULNERABLE | MONST_IMMOBILE | MONST_RESTRICTED_TO_LIQUID | MONST_GETS_TURN_ON_ACTIVATION | MONST_MAINTAINS_DISTANCE),
    MONST_NEVER_MUTATED             = (MONST_INVISIBLE | MONST_INANIMATE | MONST_IMMOBILE | MONST_INVULNERABLE),
};

enum monsterAbilityFlags {
    MA_HIT_HALLUCINATE              = Fl(0),    // monster can hit to cause hallucinations
    MA_HIT_STEAL_FLEE               = Fl(1),    // monster can steal an item and then run away
    MA_HIT_BURN                     = Fl(2),    // monster can hit to set you on fire
    MA_ENTER_SUMMONS                = Fl(3),    // monster will "become" its summoned leader, reappearing when that leader is defeated
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

    SPECIAL_HIT                     = (MA_HIT_HALLUCINATE | MA_HIT_STEAL_FLEE | MA_HIT_DEGRADE_ARMOR | MA_POISONS
                                       | MA_TRANSFERENCE | MA_CAUSES_WEAKNESS | MA_HIT_BURN | MA_ATTACKS_STAGGER),
    LEARNABLE_ABILITIES             = (MA_TRANSFERENCE | MA_CAUSES_WEAKNESS),

    MA_NON_NEGATABLE_ABILITIES      = (MA_ATTACKS_PENETRATE | MA_ATTACKS_ALL_ADJACENT | MA_ATTACKS_EXTEND | MA_ATTACKS_STAGGER),
    MA_NEVER_VORPAL_ENEMY           = (MA_KAMIKAZE),
    MA_NEVER_MUTATED                = (MA_KAMIKAZE),
};

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
    MB_IS_DYING                 = Fl(19),   // monster has already been killed and is awaiting the end-of-turn graveyard sweep (or in purgatory)
    MB_GIVEN_UP_ON_SCENT        = Fl(20),   // to help the monster remember that the scent map is a dead end
    MB_IS_DORMANT               = Fl(21),   // lurking, waiting to burst out
    MB_HAS_SOUL                 = Fl(22),   // slaying the monster will count toward weapon auto-ID
    MB_ALREADY_SEEN             = Fl(23),   // seeing this monster won't interrupt exploration
};

// Defines all creatures, which include monsters and the player:
typedef struct creatureType {
    enum monsterTypes monsterID; // index number for the monsterCatalog
    char monsterName[COLS];
    uchar displayChar;
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
    short xLoc;
    short yLoc;
    short depth;
    short currentHP;
    long turnsUntilRegen;
    short regenPerTurn;                 // number of HP to regenerate every single turn
    short weaknessAmount;               // number of points of weakness that are inflicted by the weakness status
    short poisonAmount;                 // number of points of damage per turn from poison
    enum creatureStates creatureState;  // current behavioral state
    enum creatureModes creatureMode;    // current behavioral mode (higher-level than state)

    short mutationIndex;                // what mutation the monster has (or -1 for none)

    // Waypoints:
    short targetWaypointIndex;          // the index number of the waypoint we're pathing toward
    boolean waypointAlreadyVisited[MAX_WAYPOINT_COUNT]; // checklist of waypoints
    short lastSeenPlayerAt[2];          // last location at which the monster hunted the player

    short targetCorpseLoc[2];           // location of the corpse that the monster is approaching to gain its abilities
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

    struct creature *leader;            // only if monster is a follower
    struct creature *carriedMonster;    // when vampires turn into bats, one of the bats restores the vampire when it dies
    struct creature *nextCreature;
    struct item *carriedItem;           // only used for monsters
} creature;

enum NGCommands {
    NG_NOTHING = 0,
    NG_NEW_GAME,
    NG_NEW_GAME_WITH_SEED,
    NG_OPEN_GAME,
    NG_VIEW_RECORDING,
    NG_HIGH_SCORES,
    NG_SCUM,
    NG_QUIT,
};

enum featTypes {
    FEAT_PURE_MAGE = 0,
    FEAT_PURE_WARRIOR,
    FEAT_PACIFIST,
    FEAT_ARCHIVIST,
    FEAT_COMPANION,
    FEAT_SPECIALIST,
    FEAT_JELLYMANCER,
    FEAT_INDOMITABLE,
    FEAT_MYSTIC,
    FEAT_DRAGONSLAYER,
    FEAT_PALADIN,

    FEAT_COUNT,
};

// these are basically global variables pertaining to the game state and player's unique variables:
typedef struct playerCharacter {
    short depthLevel;                   // which dungeon level are we on
    short deepestLevel;
    boolean disturbed;                  // player should stop auto-acting
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
    boolean displayAggroRangeMode;      // whether your stealth range is displayed
    boolean quit;                       // to skip the typical end-game theatrics when the player quits
    unsigned long seed;                 // the master seed for generating the entire dungeon
    short RNG;                          // which RNG are we currently using?
    unsigned long gold;                 // how much gold we have
    unsigned long goldGenerated;        // how much gold has been generated on the levels, not counting gold held by monsters
    short strength;
    unsigned short monsterSpawnFuse;    // how much longer till a random monster spawns

    item *weapon;
    item *armor;
    item *ringLeft;
    item *ringRight;

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
    short aggroRange;                   // distance from which monsters will notice you

    short previousPoisonPercent;        // and your poison proportion, to display percentage alerts for each.

    short upLoc[2];                     // upstairs location this level
    short downLoc[2];                   // downstairs location this level

    short cursorLoc[2];                 // used for the return key functionality
    creature *lastTarget;               // to keep track of the last monster the player has thrown at or zapped
    short rewardRoomsGenerated;         // to meter the number of reward machines
    short machineNumber;                // so each machine on a level gets a unique number
    short sidebarLocationList[ROWS*2][2];   // to keep track of which location each line of the sidebar references

    // maps
    short **mapToShore;                 // how many steps to get back to shore
    short **mapToSafeTerrain;           // so monsters can get to safety

    // recording info
    boolean playbackMode;               // whether we're viewing a recording instead of playing
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

    // metered items
    long long foodSpawned;                    // amount of nutrition units spawned so far this game
    short lifePotionFrequency;
    short lifePotionsSpawned;
    short strengthPotionFrequency;
    short enchantScrollFrequency;

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
    boolean featRecord[FEAT_COUNT];

    // waypoints:
    short **wpDistance[MAX_WAYPOINT_COUNT];
    short wpCount;
    short wpCoordinates[MAX_WAYPOINT_COUNT][2];
    short wpRefreshTicker;

    // cursor trail:
    short cursorPathIntensity;
    boolean cursorMode;

    // What do you want to do, player -- play, play with seed, resume, recording, high scores or quit?
    enum NGCommands nextGame;
    char nextGamePath[BROGUE_FILENAME_MAX];
    unsigned long nextGameSeed;
} playerCharacter;

// Stores the necessary info about a level so it can be regenerated:
typedef struct levelData {
    boolean visited;
    pcell mapStorage[DCOLS][DROWS];
    struct item *items;
    struct creature *monsters;
    struct creature *dormantMonsters;
    short **scentMap;
    unsigned long levelSeed;
    short upStairsLoc[2];
    short downStairsLoc[2];
    short playerExitedVia[2];
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
    MT_SENTINEL_AREA,

    NUMBER_BLUEPRINTS,
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

#define NUMBER_AUTOGENERATORS 49

typedef struct feat {
    char name[100];
    char description[200];
    boolean initialValue;
} feat;

#define PDS_FORBIDDEN   -1
#define PDS_OBSTRUCTION -2
#define PDS_CELL(map, x, y) ((map)->links + ((x) + DCOLS * (y)))

typedef struct pdsLink pdsLink;
typedef struct pdsMap pdsMap;

typedef struct brogueButton {
    char text[COLS*3];          // button label; can include color escapes
    short x;                    // button's leftmost cell will be drawn at (x, y)
    short y;
    signed long hotkey[10];     // up to 10 hotkeys to trigger the button
    color buttonColor;          // background of the button; further gradient-ized when displayed
    short opacity;              // further reduced by 50% if not enabled
    uchar symbol[COLS];         // Automatically replace the nth asterisk in the button label text with
                                // the nth character supplied here, if one is given.
                                // (Primarily to display magic character and item symbols in the inventory display.)
    unsigned long flags;
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

    // Graphical buffers:
    cellDisplayBuffer dbuf[COLS][ROWS]; // Where buttons are drawn.
    cellDisplayBuffer rbuf[COLS][ROWS]; // Reversion screen state.
} buttonState;

#if defined __cplusplus
extern "C" {
#endif

    fixpt fp_sqrt(fixpt val);

    void rogueMain();
    void executeEvent(rogueEvent *theEvent);
    boolean fileExists(const char *pathname);
    boolean chooseFile(char *path, char *prompt, char *defaultName, char *suffix);
    boolean openFile(const char *path);
    void initializeRogue(unsigned long seed);
    void gameOver(char *killedBy, boolean useCustomPhrasing);
    void victory(boolean superVictory);
    void notifyEvent(short eventId, int data1, int data2, const char *str1, const char *str2);
    void enableEasyMode();
    int rand_range(int lowerBound, int upperBound);
    unsigned long seedRandomGenerator(unsigned long seed);
    short randClumpedRange(short lowerBound, short upperBound, short clumpFactor);
    short randClump(randomRange theRange);
    boolean rand_percent(short percent);
    void shuffleList(short *list, short listLength);
    void fillSequentialList(short *list, short listLength);
    fixpt fp_round(fixpt x);
    fixpt fp_pow(fixpt base, fixpt expn);
    short unflag(unsigned long flag);
    void considerCautiousMode();
    void refreshScreen();
    void displayLevel();
    void storeColorComponents(char components[3], const color *theColor);
    boolean separateColors(color *fore, color *back);
    void bakeColor(color *theColor);
    void shuffleTerrainColors(short percentOfCells, boolean refreshCells);
    void normColor(color *baseColor, const short aggregateMultiplier, const short colorTranslation);
    void getCellAppearance(short x, short y, uchar *returnChar, color *returnForeColor, color *returnBackColor);
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
    void digDungeon();
    void updateMapToShore();
    short levelIsDisconnectedWithBlockingMap(char blockingMap[DCOLS][DROWS], boolean countRegionSize);
    void resetDFMessageEligibility();
    boolean fillSpawnMap(enum dungeonLayers layer,
                         enum tileType surfaceTileType,
                         char spawnMap[DCOLS][DROWS],
                         boolean blockedByOtherLayers,
                         boolean refresh,
                         boolean superpriority);
    boolean spawnDungeonFeature(short x, short y, dungeonFeature *feat, boolean refreshCell, boolean abortIfBlocking);
    void restoreMonster(creature *monst, short **mapToStairs, short **mapToPit);
    void restoreItem(item *theItem);
    void refreshWaypoint(short wpIndex);
    void setUpWaypoints();
    void zeroOutGrid(char grid[DCOLS][DROWS]);
    short oppositeDirection(short theDir);

    void plotChar(uchar inputChar,
                  short xLoc, short yLoc,
                  short backRed, short backGreen, short backBlue,
                  short foreRed, short foreGreen, short foreBlue);
    boolean pauseForMilliseconds(short milliseconds);
    boolean isApplicationActive();
    void nextKeyOrMouseEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance);
    boolean controlKeyIsDown();
    boolean shiftKeyIsDown();
    short getHighScoresList(rogueHighScoresEntry returnList[HIGH_SCORES_COUNT]);
    boolean saveHighScore(rogueHighScoresEntry theEntry);
    fileEntry *listFiles(short *fileCount, char **dynamicMemoryBuffer);
    void initializeLaunchArguments(enum NGCommands *command, char *path, unsigned long *seed);

    char nextKeyPress(boolean textInput);
    void refreshSideBar(short focusX, short focusY, boolean focusedEntityMustGoFirst);
    void printHelpScreen();
    void printDiscoveriesScreen();
    void printHighScores(boolean hiliteMostRecent);
    void displayGrid(short **map);
    void printSeed();
    void printProgressBar(short x, short y, const char barLabel[COLS], long amtFilled, long amtMax, color *fillColor, boolean dim);
    short printMonsterInfo(creature *monst, short y, boolean dim, boolean highlight);
    void describeHallucinatedItem(char *buf);
    short printItemInfo(item *theItem, short y, boolean dim, boolean highlight);
    short printTerrainInfo(short x, short y, short py, const char *description, boolean dim, boolean highlight);
    void rectangularShading(short x, short y, short width, short height,
                            const color *backColor, short opacity, cellDisplayBuffer dbuf[COLS][ROWS]);
    short printTextBox(char *textBuf, short x, short y, short width,
                       color *foreColor, color *backColor,
                       cellDisplayBuffer rbuf[COLS][ROWS],
                       brogueButton *buttons, short buttonCount);
    void printMonsterDetails(creature *monst, cellDisplayBuffer rbuf[COLS][ROWS]);
    void printFloorItemDetails(item *theItem, cellDisplayBuffer rbuf[COLS][ROWS]);
    unsigned long printCarriedItemDetails(item *theItem,
                                          short x, short y, short width,
                                          boolean includeButtons,
                                          cellDisplayBuffer rbuf[COLS][ROWS]);
    void funkyFade(cellDisplayBuffer displayBuf[COLS][ROWS], const color *colorStart, const color *colorEnd, short stepCount, short x, short y, boolean invert);
    void displayCenteredAlert(char *message);
    void flashMessage(char *message, short x, short y, int time, color *fColor, color *bColor);
    void flashTemporaryAlert(char *message, int time);
    void waitForAcknowledgment();
    void waitForKeystrokeOrMouseClick();
    boolean confirm(char *prompt, boolean alsoDuringPlayback);
    void refreshDungeonCell(short x, short y);
    void applyColorMultiplier(color *baseColor, const color *multiplierColor);
    void applyColorAverage(color *baseColor, const color *newColor, short averageWeight);
    void applyColorAugment(color *baseColor, const color *augmentingColor, short augmentWeight);
    void applyColorScalar(color *baseColor, short scalar);
    void applyColorBounds(color *baseColor, short lowerBound, short upperBound);
    void desaturate(color *baseColor, short weight);
    void randomizeColor(color *baseColor, short randomizePercent);
    void swapColors(color *color1, color *color2);
    void irisFadeBetweenBuffers(cellDisplayBuffer fromBuf[COLS][ROWS],
                                cellDisplayBuffer toBuf[COLS][ROWS],
                                short x, short y,
                                short frameCount,
                                boolean outsideIn);
    void colorBlendCell(short x, short y, color *hiliteColor, short hiliteStrength);
    void hiliteCell(short x, short y, const color *hiliteColor, short hiliteStrength, boolean distinctColors);
    void colorMultiplierFromDungeonLight(short x, short y, color *editColor);
    void plotCharWithColor(uchar inputChar, short xLoc, short yLoc, const color *cellForeColor, const color *cellBackColor);
    void plotCharToBuffer(uchar inputChar, short x, short y, color *foreColor, color *backColor, cellDisplayBuffer dbuf[COLS][ROWS]);
    void plotForegroundChar(uchar inputChar, short x, short y, color *foreColor, boolean affectedByLighting);
    void commitDraws();
    void dumpLevelToScreen();
    void hiliteCharGrid(char hiliteCharGrid[DCOLS][DROWS], color *hiliteColor, short hiliteStrength);
    void blackOutScreen();
    void colorOverDungeon(const color *color);
    void copyDisplayBuffer(cellDisplayBuffer toBuf[COLS][ROWS], cellDisplayBuffer fromBuf[COLS][ROWS]);
    void clearDisplayBuffer(cellDisplayBuffer dbuf[COLS][ROWS]);
    color colorFromComponents(char rgb[3]);
    void overlayDisplayBuffer(cellDisplayBuffer overBuf[COLS][ROWS], cellDisplayBuffer previousBuf[COLS][ROWS]);
    void flashForeground(short *x, short *y, color **flashColor, short *flashStrength, short count, short frames);
    void flashCell(color *theColor, short frames, short x, short y);
    void colorFlash(const color *theColor, unsigned long reqTerrainFlags, unsigned long reqTileFlags, short frames, short maxRadius, short x, short y);
    void printString(const char *theString, short x, short y, color *foreColor, color*backColor, cellDisplayBuffer dbuf[COLS][ROWS]);
    short wrapText(char *to, const char *sourceText, short width);
    short printStringWithWrapping(char *theString, short x, short y, short width, color *foreColor,
                                  color*backColor, cellDisplayBuffer dbuf[COLS][ROWS]);
    boolean getInputTextString(char *inputText,
                               const char *prompt,
                               short maxLength,
                               const char *defaultEntry,
                               const char *promptSuffix,
                               short textEntryType,
                               boolean useDialogBox);
    void displayChokeMap();
    void displayLoops();
    boolean pauseBrogue(short milliseconds);
    void nextBrogueEvent(rogueEvent *returnEvent, boolean textInput, boolean colorsDance, boolean realInputEvenInPlayback);
    void executeMouseClick(rogueEvent *theEvent);
    void executeKeystroke(signed long keystroke, boolean controlKey, boolean shiftKey);
    void initializeLevel();
    void startLevel (short oldLevelNumber, short stairDirection);
    void updateMinersLightRadius();
    void freeCreature(creature *monst);
    void emptyGraveyard();
    void freeEverything();
    boolean randomMatchingLocation(short *x, short *y, short dungeonType, short liquidType, short terrainType);
    enum dungeonLayers highestPriorityLayer(short x, short y, boolean skipGas);
    enum dungeonLayers layerWithTMFlag(short x, short y, unsigned long flag);
    enum dungeonLayers layerWithFlag(short x, short y, unsigned long flag);
    char *tileFlavor(short x, short y);
    char *tileText(short x, short y);
    void describedItemBasedOnParameters(short theCategory, short theKind, short theQuantity, short theItemOriginDepth, char *buf);
    void describeLocation(char buf[DCOLS], short x, short y);
    void printLocationDescription(short x, short y);
    void useKeyAt(item *theItem, short x, short y);
    void playerRuns(short direction);
    void exposeCreatureToFire(creature *monst);
    void updateFlavorText();
    void updatePlayerUnderwaterness();
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
    void travelRoute(short path[1000][2], short steps);
    void travel(short x, short y, boolean autoConfirm);
    void populateGenericCostMap(short **costMap);
    void getLocationFlags(const short x, const short y,
                          unsigned long *tFlags, unsigned long *TMFlags, unsigned long *cellFlags,
                          const boolean limitToPlayerKnowledge);
    void populateCreatureCostMap(short **costMap, creature *monst);
    enum directions adjacentFightingDir();
    void getExploreMap(short **map, boolean headingToStairs);
    boolean explore(short frameDelay);
    short getPlayerPathOnMap(short path[1000][2], short **map, short originX, short originY);
    void reversePath(short path[1000][2], short steps);
    void hilitePath(short path[1000][2], short steps, boolean unhilite);
    void clearCursorPath();
    void hideCursor();
    void showCursor();
    void mainInputLoop();
    boolean isDisturbed(short x, short y);
    void discover(short x, short y);
    short randValidDirectionFrom(creature *monst, short x, short y, boolean respectAvoidancePreferences);
    boolean exposeTileToElectricity(short x, short y);
    boolean exposeTileToFire(short x, short y, boolean alwaysIgnite);
    boolean cellCanHoldGas(short x, short y);
    void monstersFall();
    void updateEnvironment();
    void updateAllySafetyMap();
    void updateSafetyMap();
    void updateSafeTerrainMap();
    short staffChargeDuration(const item *theItem);
    void rechargeItemsIncrementally(short multiplier);
    void extinguishFireOnCreature(creature *monst);
    void autoRest();
    void manualSearch();
    boolean startFighting(enum directions dir, boolean tillDeath);
    void autoFight(boolean tillDeath);
    void synchronizePlayerTimeState();
    void playerRecoversFromAttacking(boolean anAttackHit);
    void playerTurnEnded();
    void resetScentTurnNumber();
    void displayMonsterFlashes(boolean flashingEnabled);
    void displayMessageArchive();
    void temporaryMessage(char *msg1, boolean requireAcknowledgment);
    void messageWithColor(char *msg, color *theColor, boolean requireAcknowledgment);
    void flavorMessage(char *msg);
    void message(const char *msg, boolean requireAcknowledgment);
    void displayMoreSignWithoutWaitingForAcknowledgment();
    void displayMoreSign();
    short encodeMessageColor(char *msg, short i, const color *theColor);
    short decodeMessageColor(const char *msg, short i, color *returnColor);
    color *messageColorFromVictim(creature *monst);
    void upperCase(char *theChar);
    void updateMessageDisplay();
    void deleteMessages();
    void confirmMessages();
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
    short chooseMonster(short forLevel);
    creature *spawnHorde(short hordeID, short x, short y, unsigned long forbiddenFlags, unsigned long requiredFlags);
    void fadeInMonster(creature *monst);
    boolean removeMonsterFromChain(creature *monst, creature *theChain);
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
    void populateMonsters();
    void updateMonsterState(creature *monst);
    void decrementMonsterStatus(creature *monst);
    boolean specifiedPathBetween(short x1, short y1, short x2, short y2,
                                 unsigned long blockingTerrain, unsigned long blockingFlags);
    boolean traversiblePathBetween(creature *monst, short x2, short y2);
    boolean openPathBetween(short x1, short y1, short x2, short y2);
    creature *monsterAtLoc(short x, short y);
    creature *dormantMonsterAtLoc(short x, short y);
    void perimeterCoords(short returnCoords[2], short n);
    boolean monsterBlinkToPreferenceMap(creature *monst, short **preferenceMap, boolean blinkUphill);
    boolean monsterSummons(creature *monst, boolean alwaysUse);
    boolean resurrectAlly(const short x, const short y);
    void unAlly(creature *monst);
    boolean monsterFleesFrom(creature *monst, creature *defender);
    void monstersTurn(creature *monst);
    boolean getRandomMonsterSpawnLocation(short *x, short *y);
    void spawnPeriodicHorde();
    void clearStatus(creature *monst);
    void moralAttack(creature *attacker, creature *defender);
    short runicWeaponChance(item *theItem, boolean customEnchantLevel, fixpt enchantLevel);
    void magicWeaponHit(creature *defender, item *theItem, boolean backstabbed);
    void teleport(creature *monst, short x, short y, boolean respectTerrainAvoidancePreferences);
    void chooseNewWanderDestination(creature *monst);
    boolean canPass(creature *mover, creature *blocker);
    boolean isPassableOrSecretDoor(short x, short y);
    boolean knownToPlayerAsPassableOrSecretDoor(short x, short y);
    void setMonsterLocation(creature *monst, short newX, short newY);
    boolean moveMonster(creature *monst, short dx, short dy);
    unsigned long burnedTerrainFlagsAtLoc(short x, short y);
    unsigned long discoveredTerrainFlagsAtLoc(short x, short y);
    boolean monsterAvoids(creature *monst, short x, short y);
    short distanceBetween(short x1, short y1, short x2, short y2);
    void alertMonster(creature *monst);
    void wakeUp(creature *monst);
    boolean monsterRevealed(creature *monst);
    boolean monsterHiddenBySubmersion(const creature *monst, const creature *observer);
    boolean monsterIsHidden(const creature *monst, const creature *observer);
    boolean canSeeMonster(creature *monst);
    boolean canDirectlySeeMonster(creature *monst);
    void monsterName(char *buf, creature *monst, boolean includeArticle);
    boolean monsterIsInClass(const creature *monst, const short monsterClass);
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
    void buildHitList(creature **hitList, const creature *attacker, creature *defender, const boolean sweep);
    void addScentToCell(short x, short y, short distance);
    void populateItems(short upstairsX, short upstairsY);
    item *placeItem(item *theItem, short x, short y);
    void removeItemFrom(short x, short y);
    void pickUpItemAt(short x, short y);
    item *addItemToPack(item *theItem);
    void aggravateMonsters(short distance, short x, short y, const color *flashColor);
    short getLineCoordinates(short listOfCoordinates[][2], const short originLoc[2], const short targetLoc[2]);
    void getImpactLoc(short returnLoc[2], const short originLoc[2], const short targetLoc[2],
                      const short maxDistance, const boolean returnLastEmptySpace);
    void negate(creature *monst);
    short monsterAccuracyAdjusted(const creature *monst);
    fixpt monsterDamageAdjustmentAmount(const creature *monst);
    short monsterDefenseAdjusted(const creature *monst);
    void weaken(creature *monst, short maxDuration);
    void slow(creature *monst, short turns);
    void haste(creature *monst, short turns);
    void heal(creature *monst, short percent, boolean panacea);
    boolean projectileReflects(creature *attacker, creature *defender);
    short reflectBolt(short targetX, short targetY, short listOfCoordinates[][2], short kinkCell, boolean retracePath);
    void checkForMissingKeys(short x, short y);
    enum boltEffects boltEffectForItem(item *theItem);
    enum boltType boltForItem(item *theItem);
    boolean zap(short originLoc[2], short targetLoc[2], bolt *theBolt, boolean hideDetails);
    boolean nextTargetAfter(short *returnX,
                            short *returnY,
                            short targetX,
                            short targetY,
                            boolean targetEnemies,
                            boolean targetAllies,
                            boolean targetItems,
                            boolean targetTerrain,
                            boolean requireOpenPath,
                            boolean reverseDirection);
    boolean moveCursor(boolean *targetConfirmed,
                       boolean *canceled,
                       boolean *tabKey,
                       short targetLoc[2],
                       rogueEvent *event,
                       buttonState *state,
                       boolean colorsDance,
                       boolean keysMoveCursor,
                       boolean targetCanLeaveMap);
    void identifyItemKind(item *theItem);
    void autoIdentify(item *theItem);
    short numberOfItemsInPack();
    char nextAvailableInventoryCharacter();
    void checkForDisenchantment(item *theItem);
    void updateFloorItems();
    void itemName(item *theItem, char *root, boolean includeDetails, boolean includeArticle, color *baseColor);
    char displayInventory(unsigned short categoryMask,
                          unsigned long requiredFlags,
                          unsigned long forbiddenFlags,
                          boolean waitForAcknowledge,
                          boolean includeButtons);
    short numberOfMatchingPackItems(unsigned short categoryMask,
                                    unsigned long requiredFlags, unsigned long forbiddenFlags,
                                    boolean displayErrors);
    void clearInventory(char keystroke);
    item *initializeItem();
    item *generateItem(unsigned short theCategory, short theKind);
    short chooseKind(itemTable *theTable, short numKinds);
    item *makeItemInto(item *theItem, unsigned long itemCategory, short itemKind);
    void updateEncumbrance();
    short displayedArmorValue();
    void strengthCheck(item *theItem);
    void recalculateEquipmentBonuses();
    void equipItem(item *theItem, boolean force);
    void equip(item *theItem);
    item *keyInPackFor(short x, short y);
    item *keyOnTileAt(short x, short y);
    void unequip(item *theItem);
    void drop(item *theItem);
    void findAlternativeHomeFor(creature *monst, short *x, short *y, boolean chooseRandomly);
    boolean getQualifyingLocNear(short loc[2],
                                 short x, short y,
                                 boolean hallwaysAllowed,
                                 char blockingMap[DCOLS][DROWS],
                                 unsigned long forbiddenTerrainFlags,
                                 unsigned long forbiddenMapFlags,
                                 boolean forbidLiquid,
                                 boolean deterministic);
    boolean getQualifyingGridLocNear(short loc[2],
                                     short x, short y,
                                     boolean grid[DCOLS][DROWS],
                                     boolean deterministic);

    // Grid operations
    short **allocGrid();
    void freeGrid(short **array);
    void copyGrid(short **to, short **from);
    void fillGrid(short **grid, short fillValue);
    void hiliteGrid(short **grid, color *hiliteColor, short hiliteStrength);
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
    void throwCommand(item *theItem);
    void relabel(item *theItem);
    void apply(item *theItem, boolean recordCommands);
    boolean itemCanBeCalled(item *theItem);
    void call(item *theItem);
    short chooseVorpalEnemy();
    void describeMonsterClass(char *buf, const short classID, boolean conjunctionAnd);
    void identify(item *theItem);
    void updateIdentifiableItem(item *theItem);
    void updateIdentifiableItems();
    void readScroll(item *theItem);
    void updateRingBonuses();
    void updatePlayerRegenerationDelay();
    boolean removeItemFromChain(item *theItem, item *theChain);
    void addItemToChain(item *theItem, item *theChain);
    void drinkPotion(item *theItem);
    item *promptForItemOfType(unsigned short category,
                              unsigned long requiredFlags,
                              unsigned long forbiddenFlags,
                              char *prompt,
                              boolean allowInventoryActions);
    item *itemOfPackLetter(char letter);
    void unequipItem(item *theItem, boolean force);
    short magicCharDiscoverySuffix(short category, short kind);
    uchar itemMagicChar(item *theItem);
    item *itemAtLoc(short x, short y);
    item *dropItem(item *theItem);
    itemTable *tableForItemCategory(enum itemCategory theCat, short *kindCount);
    boolean isVowelish(char *theChar);
    short charmEffectDuration(short charmKind, short enchant);
    short charmRechargeDelay(short charmKind, short enchant);
    boolean itemIsCarried(item *theItem);
    void itemDetails(char *buf, item *theItem);
    void deleteItem(item *theItem);
    void shuffleFlavors();
    unsigned long itemValue(item *theItem);
    short strLenWithoutEscapes(const char *str);
    void combatMessage(char *theMsg, color *theColor);
    void displayCombatText();
    void flashMonster(creature *monst, const color *theColor, short strength);

    boolean paintLight(lightSource *theLight, short x, short y, boolean isMinersLight, boolean maintainShadows);
    void backUpLighting(short lights[DCOLS][DROWS][3]);
    void restoreLighting(short lights[DCOLS][DROWS][3]);
    void updateLighting();
    boolean playerInDarkness();
    flare *newFlare(lightSource *light, short x, short y, short changePerFrame, short limit);
    void createFlare(short x, short y, enum lightType lightIndex);
    void animateFlares(flare **flares, short count);
    void deleteAllFlares();
    void demoteVisibility();
    void discoverCell(const short x, const short y);
    void updateVision(boolean refreshDisplay);
    void burnItem(item *theItem);
    void activateMachine(short machineNumber);
    boolean circuitBreakersPreventActivation(short machineNumber);
    void promoteTile(short x, short y, enum dungeonLayers layer, boolean useFireDF);
    void autoPlayLevel(boolean fastForward);
    void updateClairvoyance();
    short scentDistance(short x1, short y1, short x2, short y2);
    short armorAggroAdjustment(item *theArmor);
    short currentAggroValue();

    void initRecording();
    void flushBufferToFile();
    void fillBufferFromFile();
    void recordEvent(rogueEvent *event);
    void recallEvent(rogueEvent *event);
    void pausePlayback();
    void displayAnnotation();
    void loadSavedGame();
    void recordKeystroke(uchar keystroke, boolean controlKey, boolean shiftKey);
    void recordKeystrokeSequence(unsigned char *commandSequence);
    void recordMouseClick(short x, short y, boolean controlKey, boolean shiftKey);
    void OOSCheck(unsigned long x, short numberOfBytes);
    void RNGCheck();
    boolean executePlaybackInput(rogueEvent *recordingInput);
    void getAvailableFilePath(char *filePath, const char *defaultPath, const char *suffix);
    boolean characterForbiddenInFilename(const char theChar);
    void saveGame();
    void saveRecording(char *filePath);
    void saveRecordingNoPrompt(char *filePath);
    void parseFile();
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

    void checkForDungeonErrors();

    boolean dialogChooseFile(char *path, const char *suffix, const char *prompt);
    void dialogAlert(char *message);
    void mainBrogueJunction();

    void initializeButton(brogueButton *button);
    void drawButtonsInState(buttonState *state);
    void initializeButtonState(buttonState *state,
                               brogueButton *buttons,
                               short buttonCount,
                               short winX,
                               short winY,
                               short winWidth,
                               short winHeight);
    short processButtonInput(buttonState *state, boolean *canceled, rogueEvent *event);
    short smoothHiliteGradient(const short currentXValue, const short maxXValue);
    void drawButton(brogueButton *button, enum buttonDrawStates highlight, cellDisplayBuffer dbuf[COLS][ROWS]);
    short buttonInputLoop(brogueButton *buttons,
                          short buttonCount,
                          short winX,
                          short winY,
                          short winWidth,
                          short winHeight,
                          rogueEvent *returnEvent);

    void dijkstraScan(short **distanceMap, short **costMap, boolean useDiagonals);
    void pdsClear(pdsMap *map, short maxDistance, boolean eightWays);
    void pdsSetDistance(pdsMap *map, short x, short y, short distance);
    void pdsBatchOutput(pdsMap *map, short **distanceMap);

#if defined __cplusplus
}
#endif

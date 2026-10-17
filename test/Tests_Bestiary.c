// Tests derived from rules/09-monsters-bestiary.md.
//
// Headless-testable subset: bestiary trait/ability flags and base stats, read
// directly from each monster's catalog entry via the new predicates.  Behavior
// that depends on RNG, level generation, pathing, spell selection, messages or
// a screen is omitted.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static creature *subject;

static void standPlayerOn(short terrain) {
    pos p = player.loc;
    setTerrain(p, terrain);
    setPlayerPos(p);
}

static void trait_setup(enum monsterTypes type) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = spawnMonster(type, posOffset(player.loc, 5, 0));
}

// Convenience macros to keep each test tiny.
#define TRAIT_TEST(name, type, flag, msg) \
static void name##_setup(void) { trait_setup(type); } \
static boolean name##_verify(void) { \
    TEST_ASSERT(monsterHasTrait(subject, flag) != 0, msg); \
    return !testFailed; \
}

#define ABILITY_TEST(name, type, flag, msg) \
static void name##_setup(void) { trait_setup(type); } \
static boolean name##_verify(void) { \
    TEST_ASSERT(monsterHasAbility(subject, flag) != 0, msg); \
    return !testFailed; \
}

#define BOLT_TEST(name, type, bolt, msg) \
static void name##_setup(void) { trait_setup(type); } \
static boolean name##_verify(void) { \
    TEST_ASSERT(monsterCastsBolt(subject, bolt) != 0, msg); \
    return !testFailed; \
}

// ---- Traits (MONST_*) ----
TRAIT_TEST(br39474, MK_VAMPIRE_BAT, MONST_FLIES, "vampire bat flies")
TRAIT_TEST(br10411, MK_BLOAT, MONST_FLITS, "bloat moves erratically")
TRAIT_TEST(br60148, MK_EEL, MONST_RESTRICTED_TO_LIQUID, "eel liquid-bound")
TRAIT_TEST(br47597, MK_EEL, MONST_IMMUNE_TO_WATER, "eel full speed in water")
TRAIT_TEST(br65633, MK_EEL, MONST_SUBMERGES, "eel submerges")
TRAIT_TEST(br99362, MK_GOBLIN_TOTEM, MONST_IMMOBILE, "goblin totem immobile")
TRAIT_TEST(br72064, MK_CENTAUR, MONST_MAINTAINS_DISTANCE, "centaur keeps distance")
TRAIT_TEST(br30757, MK_GOBLIN_TOTEM, MONST_WILL_NOT_USE_STAIRS, "totem won't use stairs")
TRAIT_TEST(br91557, MK_SPIDER, MONST_IMMUNE_TO_WEBS, "spider immune to webs")
TRAIT_TEST(br13361, MK_ARROW_TURRET, MONST_ATTACKABLE_THRU_WALLS, "turret attackable thru walls")
TRAIT_TEST(br53451, MK_DRAGON, MONST_CARRY_ITEM_100, "dragon carries item")
TRAIT_TEST(br48675, MK_SPECTRAL_BLADE, MONST_NOT_LISTED_IN_SIDEBAR, "spectral blade not in sidebar")
TRAIT_TEST(br67941, MK_EEL, MONST_NEVER_SLEEPS, "eel never sleeps")
TRAIT_TEST(br46423, MK_WARDEN_OF_YENDOR, MONST_ALWAYS_HUNTING, "warden always hunting")
TRAIT_TEST(br51688, MK_WRAITH, MONST_FLEES_NEAR_DEATH, "wraith flees near death")
TRAIT_TEST(br16717, MK_GUARDIAN, MONST_GETS_TURN_ON_ACTIVATION, "guardian acts on activation")
TRAIT_TEST(br85956, MK_SPIDER, MONST_ALWAYS_USE_ABILITY, "spider always uses ability")
TRAIT_TEST(br39641, MK_SPIDER, MONST_CAST_SPELLS_SLOWLY, "spider casts slowly")
TRAIT_TEST(br21958, MK_GOBLIN_TOTEM, MONST_INANIMATE, "totem inanimate")
TRAIT_TEST(br90225, MK_WARDEN_OF_YENDOR, MONST_INVULNERABLE, "warden invulnerable")
TRAIT_TEST(br95842, MK_REVENANT, MONST_IMMUNE_TO_WEAPONS, "revenant immune to weapons")
TRAIT_TEST(br37585, MK_ACID_MOUND, MONST_DEFEND_DEGRADE_WEAPON, "acid mound corrodes weapon")
TRAIT_TEST(br13523, MK_DRAGON, MONST_IMMUNE_TO_FIRE, "dragon immune to fire")
TRAIT_TEST(br57968, MK_WILL_O_THE_WISP, MONST_FIERY, "wisp burns")
TRAIT_TEST(br18737, MK_GOLEM, MONST_REFLECT_50, "golem reflects half")
TRAIT_TEST(br12704, MK_LICH, MONST_NO_POLYMORPH, "lich no polymorph")
TRAIT_TEST(br49476, MK_WILL_O_THE_WISP, MONST_DIES_IF_NEGATED, "wisp dies if negated")
TRAIT_TEST(br56920, MK_PHANTOM, MONST_INVISIBLE, "phantom invisible")

// ---- Abilities (MA_*) ----
ABILITY_TEST(br46956, MK_TOAD, MA_HIT_HALLUCINATE, "toad hallucinate")
ABILITY_TEST(br18776, MK_MONKEY, MA_HIT_STEAL_FLEE, "monkey steal+flee")
ABILITY_TEST(br32322, MK_WILL_O_THE_WISP, MA_HIT_BURN, "wisp sets alight")
ABILITY_TEST(br80252, MK_ACID_MOUND, MA_HIT_DEGRADE_ARMOR, "acid mound corrode armor")
ABILITY_TEST(br45824, MK_SPIDER, MA_POISONS, "spider poisons")
ABILITY_TEST(br32139, MK_CENTIPEDE, MA_CAUSES_WEAKNESS, "centipede weakness")
ABILITY_TEST(br54374, MK_KRAKEN, MA_SEIZES, "kraken grabs")
ABILITY_TEST(br15511, MK_BLOAT, MA_KAMIKAZE, "bloat kamikaze")
ABILITY_TEST(br83324, MK_VAMPIRE_BAT, MA_TRANSFERENCE, "bat transference")
ABILITY_TEST(br97180, MK_PINK_JELLY, MA_CLONE_SELF_ON_DEFEND, "jelly splits")
ABILITY_TEST(br12637, MK_BLOAT, MA_DF_ON_DEATH, "bloat DF on death")
ABILITY_TEST(br40544, MK_LICH, MA_CAST_SUMMON, "lich summons")
ABILITY_TEST(br70715, MK_PHYLACTERY, MA_ENTER_SUMMONS, "phylactery enter summons")
ABILITY_TEST(br56605, MK_GUARDIAN, MA_REFLECT_100, "guardian reflect 100")
ABILITY_TEST(br88806, MK_OGRE, MA_ATTACKS_STAGGER, "ogre stagger")
ABILITY_TEST(br48373, MK_OGRE, MA_AVOID_CORRIDORS, "ogre avoids corridors")

// Transposed comment handling
ABILITY_TEST(br43379, MK_GOBLIN, MA_ATTACKS_PENETRATE, "goblin penetrates")
ABILITY_TEST(br97260, MK_NAGA, MA_ATTACKS_ALL_ADJACENT, "naga attacks all adjacent")
ABILITY_TEST(br17980, MK_SALAMANDER, MA_ATTACKS_EXTEND, "salamander extends")

// ---- Bolts ----
BOLT_TEST(br53754, MK_GOBLIN_MYSTIC, BOLT_SHIELDING, "goblin mystic shields")
BOLT_TEST(br47059, MK_OGRE_TOTEM, BOLT_HEALING, "ogre totem heals")
BOLT_TEST(br73986, MK_OGRE_SHAMAN, BOLT_HASTE, "ogre shaman hastes")
BOLT_TEST(br32475, MK_OGRE_TOTEM, BOLT_SLOW_2, "ogre totem slows")
BOLT_TEST(br68026, MK_DAR_BATTLEMAGE, BOLT_DISCORD, "dar battlemage discord")
BOLT_TEST(br81389, MK_DAR_PRIESTESS, BOLT_NEGATION, "dar priestess negates")
BOLT_TEST(br60584, MK_DAR_BATTLEMAGE, BOLT_FIRE, "dar battlemage fire")
BOLT_TEST(br18935, MK_SPIDER, BOLT_SPIDERWEB, "spider web")
BOLT_TEST(br16647, MK_DAR_BLADEMASTER, BOLT_BLINKING, "dar blademaster blinking")
BOLT_TEST(br71983, MK_MIRRORED_TOTEM, BOLT_BECKONING, "mirrored totem beckoning")
BOLT_TEST(br81211, MK_ANCIENT_SPIRIT, BOLT_ANCIENT_SPIRIT_VINES, "dryad vines")
BOLT_TEST(br88052, MK_DART_TURRET, BOLT_POISON_DART, "dart turret poison dart")
BOLT_TEST(br77502, MK_DRAGON, BOLT_DRAGONFIRE, "dragon dragonfire")

// ---- Base stats (rat) ----
static void rat_stats_setup(void) { trait_setup(MK_RAT); }
static boolean rat_maxhp(void) { TEST_ASSERT_EQ(monsterMaxHealth(subject), 6, "rat max hp"); return !testFailed; }
static boolean rat_def(void) { TEST_ASSERT_EQ(monsterBaseDefense(subject), 0, "rat defense"); return !testFailed; }
static boolean rat_acc(void) { TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 80, "rat accuracy"); return !testFailed; }
static boolean rat_dmg_low(void) { TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 1, "rat dmg low"); return !testFailed; }
static boolean rat_dmg_high(void) { TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 3, "rat dmg high"); return !testFailed; }
static boolean rat_movespeed(void) { TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 100, "rat move speed"); return !testFailed; }
static boolean rat_attackspeed(void) { TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100, "rat attack speed"); return !testFailed; }

const testCase tests_bestiary[] = {
    { "BR39474_flies", br39474_setup, NULL, br39474_verify },
    { "BR10411_erratic", br10411_setup, NULL, br10411_verify },
    { "BR60148_liquid_bound", br60148_setup, NULL, br60148_verify },
    { "BR47597_full_speed_water", br47597_setup, NULL, br47597_verify },
    { "BR65633_submerges", br65633_setup, NULL, br65633_verify },
    { "BR99362_immobile", br99362_setup, NULL, br99362_verify },
    { "BR72064_keeps_distance", br72064_setup, NULL, br72064_verify },
    { "BR30757_wont_use_stairs", br30757_setup, NULL, br30757_verify },
    { "BR91557_immune_webs", br91557_setup, NULL, br91557_verify },
    { "BR13361_attackable_thru_walls", br13361_setup, NULL, br13361_verify },
    { "BR53451_carries_item", br53451_setup, NULL, br53451_verify },
    { "BR48675_not_in_sidebar", br48675_setup, NULL, br48675_verify },
    { "BR67941_never_sleeps", br67941_setup, NULL, br67941_verify },
    { "BR46423_always_hunting", br46423_setup, NULL, br46423_verify },
    { "BR51688_flees_near_death", br51688_setup, NULL, br51688_verify },
    { "BR16717_acts_on_activation", br16717_setup, NULL, br16717_verify },
    { "BR85956_always_uses_ability", br85956_setup, NULL, br85956_verify },
    { "BR39641_casts_slowly", br39641_setup, NULL, br39641_verify },
    { "BR21958_inanimate", br21958_setup, NULL, br21958_verify },
    { "BR90225_invulnerable", br90225_setup, NULL, br90225_verify },
    { "BR95842_immune_weapons", br95842_setup, NULL, br95842_verify },
    { "BR37585_corrodes_weapon", br37585_setup, NULL, br37585_verify },
    { "BR13523_immune_fire", br13523_setup, NULL, br13523_verify },
    { "BR57968_burns", br57968_setup, NULL, br57968_verify },
    { "BR18737_reflects_half", br18737_setup, NULL, br18737_verify },
    { "BR12704_no_polymorph", br12704_setup, NULL, br12704_verify },
    { "BR49476_dies_if_negated", br49476_setup, NULL, br49476_verify },
    { "BR56920_invisible", br56920_setup, NULL, br56920_verify },
    { "BR46956_hallucinate", br46956_setup, NULL, br46956_verify },
    { "BR18776_steal_flee", br18776_setup, NULL, br18776_verify },
    { "BR32322_set_alight", br32322_setup, NULL, br32322_verify },
    { "BR80252_corrode_armor", br80252_setup, NULL, br80252_verify },
    { "BR45824_poison", br45824_setup, NULL, br45824_verify },
    { "BR32139_weakness", br32139_setup, NULL, br32139_verify },
    { "BR54374_grabs", br54374_setup, NULL, br54374_verify },
    { "BR15511_kamikaze", br15511_setup, NULL, br15511_verify },
    { "BR83324_transference", br83324_setup, NULL, br83324_verify },
    { "BR97180_splits", br97180_setup, NULL, br97180_verify },
    { "BR12637_df_on_death", br12637_setup, NULL, br12637_verify },
    { "BR40544_summons", br40544_setup, NULL, br40544_verify },
    { "BR70715_enter_summons", br70715_setup, NULL, br70715_verify },
    { "BR56605_reflect_100", br56605_setup, NULL, br56605_verify },
    { "BR88806_stagger", br88806_setup, NULL, br88806_verify },
    { "BR48373_avoids_corridors", br48373_setup, NULL, br48373_verify },
    { "BR43379_penetrate", br43379_setup, NULL, br43379_verify },
    { "BR97260_attacks_all_adjacent", br97260_setup, NULL, br97260_verify },
    { "BR17980_attacks_extend", br17980_setup, NULL, br17980_verify },
    { "BR53754_shield", br53754_setup, NULL, br53754_verify },
    { "BR47059_heal", br47059_setup, NULL, br47059_verify },
    { "BR73986_haste", br73986_setup, NULL, br73986_verify },
    { "BR32475_slow", br32475_setup, NULL, br32475_verify },
    { "BR68026_discord", br68026_setup, NULL, br68026_verify },
    { "BR81389_negation", br81389_setup, NULL, br81389_verify },
    { "BR60584_fire", br60584_setup, NULL, br60584_verify },
    { "BR18935_web", br18935_setup, NULL, br18935_verify },
    { "BR16647_blinking", br16647_setup, NULL, br16647_verify },
    { "BR71983_beckoning", br71983_setup, NULL, br71983_verify },
    { "BR81211_vines", br81211_setup, NULL, br81211_verify },
    { "BR88052_poison_dart", br88052_setup, NULL, br88052_verify },
    { "BR77502_dragonfire", br77502_setup, NULL, br77502_verify },
    { "BR83147_rat_max_health", rat_stats_setup, NULL, rat_maxhp },
    { "BR83147_rat_defense", rat_stats_setup, NULL, rat_def },
    { "BR83147_rat_accuracy", rat_stats_setup, NULL, rat_acc },
    { "BR83147_rat_damage_low", rat_stats_setup, NULL, rat_dmg_low },
    { "BR83147_rat_damage_high", rat_stats_setup, NULL, rat_dmg_high },
    { "BR83147_rat_movement_speed", rat_stats_setup, NULL, rat_movespeed },
    { "BR83147_rat_attack_speed", rat_stats_setup, NULL, rat_attackspeed },
    { NULL, NULL, NULL, NULL }
};

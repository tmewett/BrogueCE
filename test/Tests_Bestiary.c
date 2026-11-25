// Tests derived from rules/09-monsters-bestiary.md.
//
// Headless-testable subset: bestiary trait/ability flags and base stats, read
// directly from each monster's catalog entry via the new predicates, plus the
// regeneration period, which runs deterministically in the turn loop, and the
// glow a lit tile reports with and without its source standing on it.
// Behavior that depends on RNG, level generation, pathing, spell selection,
// messages or a screen is omitted.

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

// ============================================================
// BR18852 — Phylactery.  Health 30, defense 0, accuracy 0, damage 0; no
// regeneration; normal movement but a slow attack (150).  Immobile, never
// sleeping, inanimate, always hunting, will not follow between levels, cannot
// be entangled, dies if negated; summons allies; glows.
// ============================================================

static creature *br18852_subject;
static int br18852_light_with_glow;
static int br18852_light_without_glow;

// Measure how much light the subject alone sheds on its own tile: the tile is
// brightest while the subject stands there and dimmer once it has stepped
// away, so the difference isolates the subject's glow from every other light.
static void measureMonsterGlow(creature *glowing, int *withGlow, int *withoutGlow) {
    pos here = monsterPosition(glowing);
    recomputeLighting();
    *withGlow = lightAt(here);
    pos away = (here.x + 20 < levelWidth())
        ? posAt(here.x + 20, here.y)
        : posAt(here.x - 20, here.y);
    setMonsterPos(glowing, away);
    recomputeLighting();
    *withoutGlow = lightAt(here);
    setMonsterPos(glowing, here);
    recomputeLighting();
}

static void br18852_setup(void) {
    trait_setup(MK_PHYLACTERY);
    br18852_subject = subject;
    measureMonsterGlow(br18852_subject, &br18852_light_with_glow,
                       &br18852_light_without_glow);
    // An ally is a harmless bystander while time passes, and a wounded
    // creature would heal during the wait if it regenerated.
    makeMonsterAlly(br18852_subject);
    setMonsterHealth(br18852_subject, monsterMaxHealth(br18852_subject) / 2);
}

static boolean br18852_verify(void) {
    creature *s = br18852_subject;
    TEST_ASSERT(s != NULL, "phylactery spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(s), 30, "phylactery health");
    TEST_ASSERT_EQ(monsterBaseDefense(s), 0, "phylactery defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(s), 0, "phylactery accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(s), 0, "phylactery damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(s), 0, "phylactery damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(s), 100, "phylactery moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(s), 150, "phylactery attacks slowly");
    TEST_ASSERT(monsterHasTrait(s, MONST_IMMOBILE), "phylactery is immobile");
    TEST_ASSERT(monsterHasTrait(s, MONST_NEVER_SLEEPS), "phylactery never sleeps");
    TEST_ASSERT(monsterHasTrait(s, MONST_INANIMATE), "phylactery is inanimate");
    TEST_ASSERT(monsterHasTrait(s, MONST_ALWAYS_HUNTING), "phylactery always hunts");
    TEST_ASSERT(monsterHasTrait(s, MONST_WILL_NOT_USE_STAIRS),
                "phylactery will not follow between levels");
    TEST_ASSERT(monsterHasTrait(s, MONST_IMMUNE_TO_WEBS), "phylactery cannot be entangled");
    TEST_ASSERT(monsterHasTrait(s, MONST_DIES_IF_NEGATED), "phylactery dies if negated");
    TEST_ASSERT(monsterHasAbility(s, MA_CAST_SUMMON), "phylactery summons allies");
    TEST_ASSERT(!monsterRegenerates(s), "phylactery has no natural regeneration");
    TEST_ASSERT_EQ(monsterHealth(s), monsterMaxHealth(s) / 2,
                   "a wounded phylactery stays wounded as time passes");
    TEST_ASSERT(br18852_light_with_glow > br18852_light_without_glow,
                "phylactery glows");
    return !testFailed;
}

// ============================================================
// BR67777 — Spectral Sword.  Health 1, defense 0, accuracy 150, damage 1; no
// regeneration; moves twice as fast (50) with a normal attack (100).
// Inanimate, never sleeping, flying, will not follow between levels, cannot
// be entangled, dies if negated; glows.
// ============================================================

static creature *br67777_subject;
static int br67777_light_with_glow;
static int br67777_light_without_glow;

static void br67777_setup(void) {
    trait_setup(MK_SPECTRAL_IMAGE);
    br67777_subject = subject;
    measureMonsterGlow(br67777_subject, &br67777_light_with_glow,
                       &br67777_light_without_glow);
}

static boolean br67777_verify(void) {
    creature *s = br67777_subject;
    TEST_ASSERT(s != NULL, "spectral sword spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(s), 1, "spectral sword health");
    TEST_ASSERT_EQ(monsterBaseDefense(s), 0, "spectral sword defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(s), 150, "spectral sword accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(s), 1, "spectral sword damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(s), 1, "spectral sword damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(s), 50, "spectral sword moves twice as fast");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(s), 100, "spectral sword attacks at normal speed");
    TEST_ASSERT(monsterHasTrait(s, MONST_INANIMATE), "spectral sword is inanimate");
    TEST_ASSERT(monsterHasTrait(s, MONST_NEVER_SLEEPS), "spectral sword never sleeps");
    TEST_ASSERT(monsterHasTrait(s, MONST_FLIES), "spectral sword flies");
    TEST_ASSERT(monsterHasTrait(s, MONST_WILL_NOT_USE_STAIRS),
                "spectral sword will not follow between levels");
    TEST_ASSERT(monsterHasTrait(s, MONST_IMMUNE_TO_WEBS), "spectral sword cannot be entangled");
    TEST_ASSERT(monsterHasTrait(s, MONST_DIES_IF_NEGATED), "spectral sword dies if negated");
    TEST_ASSERT(!monsterRegenerates(s), "spectral sword has no natural regeneration");
    TEST_ASSERT(br67777_light_with_glow > br67777_light_without_glow,
                "spectral sword glows");
    return !testFailed;
}

// ============================================================
// BR45724 — Monkey.  Health 12, defense 17, accuracy 100, damage
// 1-3; regenerates every 20 turns; normal speeds.  It steals an
// item and flees when it hits (BR18776) and has a 1% chance per
// turn to leave a urine trail.
//
// The stat block, the natural-regeneration flag and the
// steal-and-flee ability are read from the spawned monkey's
// catalog entry.  The 1%-per-turn urine trail depends on the RNG
// stream and is not checked here; the 20-turn regen period is
// checked behaviorally below.
// ============================================================

static void br45724_setup(void) { trait_setup(MK_MONKEY); }

static boolean br45724_verify(void) {
    TEST_ASSERT(subject != NULL, "monkey spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 12, "monkey health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 17, "monkey defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 100, "monkey accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 1, "monkey damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 3, "monkey damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 100, "monkey moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100, "monkey attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "monkey regenerates naturally");
    TEST_ASSERT(monsterHasAbility(subject, MA_HIT_STEAL_FLEE),
                "monkey steals an item and flees when its attack lands");
    return !testFailed;
}

// The regen period, watched turn by turn: a wounded monkey with no enemies
// about can only regain health through regeneration, so it must sit at 1 HP
// for 19 turns and regain exactly one point on the 20th.
static int br45724_regenTurns;

static void br45724_regen_setup(void) {
    trait_setup(MK_MONKEY);
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br45724_regenTurns = 0;
}

static boolean br45724_regen_step(void) {
    restOneTurn();
    br45724_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br45724_regenTurns < 60;
}

static boolean br45724_regen_verify(void) {
    TEST_ASSERT(subject != NULL, "monkey spawned");
    TEST_ASSERT_EQ(br45724_regenTurns, 20,
                   "the monkey should wait 20 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the monkey should regain exactly one point of health per period");
    return !testFailed;
}

// ============================================================
// BR69414 — Imp.  Health 35, defense 90, accuracy 225, damage 4-9;
// regenerates every 10 turns; normal speeds.  It steals an item and flees
// when it hits (BR18776) and blinks around the battlefield (BR16647).  It
// glows.
// ============================================================

static int br69414_light_with_glow;
static int br69414_light_without_glow;

static void br69414_setup(void) {
    trait_setup(MK_IMP);
    measureMonsterGlow(subject, &br69414_light_with_glow, &br69414_light_without_glow);
}

static boolean br69414_verify(void) {
    TEST_ASSERT(subject != NULL, "imp spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 35, "imp health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 90, "imp defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 225, "imp accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 4, "imp damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 9, "imp damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 100, "imp moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100, "imp attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "imp regenerates naturally");
    TEST_ASSERT(monsterHasAbility(subject, MA_HIT_STEAL_FLEE),
                "imp steals an item and flees when its attack lands");
    TEST_ASSERT(monsterCastsBolt(subject, BOLT_BLINKING),
                "imp blinks around the battlefield");
    TEST_ASSERT(br69414_light_with_glow > br69414_light_without_glow,
                "imp glows");
    return !testFailed;
}

// The 10-turn regeneration period, watched turn by turn: a wounded imp with
// no enemies about can only regain health through regeneration, so it must
// sit at 1 HP for 9 turns and regain exactly one point on the 10th.
static int br69414_regenTurns;

static void br69414_regen_setup(void) {
    trait_setup(MK_IMP);
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br69414_regenTurns = 0;
}

static boolean br69414_regen_step(void) {
    restOneTurn();
    br69414_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br69414_regenTurns < 40;
}

static boolean br69414_regen_verify(void) {
    TEST_ASSERT(subject != NULL, "imp spawned");
    TEST_ASSERT_EQ(br69414_regenTurns, 10,
                   "the imp should wait 10 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the imp should regain exactly one point of health per period");
    return !testFailed;
}

// ============================================================
// BR73845 — Goblin.  Health 15, defense 10, accuracy 70, damage
// 2–5; regenerates every 20 turns; normal speeds.  Its attacks
// penetrate to a second enemy in a line (BR43379), and it avoids
// corridors while hunting (BR48373).
//
// The stat block, the natural-regeneration flag and the two
// abilities are read from the spawned goblin's catalog entry.
// The 20-turn regeneration period is watched turn by turn in the
// same test: a wounded ally with no enemies about can only
// regain health through regeneration, so it must sit at 1 HP
// for 19 turns and regain exactly one point on the 20th.
// ============================================================

static int br73845_regenTurns;

static void br73845_setup(void) {
    trait_setup(MK_GOBLIN);
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br73845_regenTurns = 0;
}

static boolean br73845_step(void) {
    restOneTurn();
    br73845_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br73845_regenTurns < 60;
}

static boolean br73845_verify(void) {
    TEST_ASSERT(subject != NULL, "goblin spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 15, "goblin health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 10, "goblin defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 70, "goblin accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 2, "goblin damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 5, "goblin damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 100, "goblin moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100, "goblin attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "goblin regenerates naturally");
    TEST_ASSERT(monsterHasAbility(subject, MA_ATTACKS_PENETRATE),
                "goblin attacks penetrate to a second enemy in a line");
    TEST_ASSERT(monsterHasAbility(subject, MA_AVOID_CORRIDORS),
                "goblin avoids corridors while hunting");
    TEST_ASSERT_EQ(br73845_regenTurns, 20,
                   "the goblin should wait 20 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the goblin should regain exactly one point of health per period");
    return !testFailed;
}

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
    { "BR18852_phylactery_stats_and_traits", br18852_setup, wait_100_turns_step, br18852_verify },
    { "BR67777_spectral_sword_stats_and_traits", br67777_setup, NULL, br67777_verify },
    { "BR45724_monkey_stats_and_traits", br45724_setup, NULL, br45724_verify },
    { "BR45724_monkey_regenerates_every_20_turns", br45724_regen_setup, br45724_regen_step, br45724_regen_verify },
    { "BR69414_imp_stats_and_traits", br69414_setup, NULL, br69414_verify },
    { "BR69414_imp_regenerates_every_10_turns", br69414_regen_setup, br69414_regen_step, br69414_regen_verify },
    { "BR73845_goblin_stats_and_regeneration", br73845_setup, br73845_step, br73845_verify },
    { NULL, NULL, NULL, NULL }
};

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
    pos p = getPlayerPos();
    setTerrain(p, terrain);
    setPlayerPos(p);
}

static void trait_setup(enum monsterTypes type) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = spawnMonster(type, posOffset(getPlayerPos(), 5, 0));
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
// BR72064 — keeps its distance.  In addition to the catalog flag, exercise the
// live behavior: a hostile centaur spawned in melee range must back off to
// about three tiles and must never settle back into melee range.  A monster
// without the trait would stay in melee and attack, so the retreat proves the
// trait is doing work.  Distances are Chebyshev (diagonal counts as one step),
// measured from the player after each turn.
static int br72064_startTurn;
static boolean br72064_everAdjacent;
static int br72064_maxDist;

static int chebyshevToPlayer(pos p) {
    pos pl = getPlayerPos();
    int dx = p.x - pl.x; if (dx < 0) dx = -dx;
    int dy = p.y - pl.y; if (dy < 0) dy = -dy;
    return dx > dy ? dx : dy;
}

static void br72064_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = placeHostileMonster(MK_CENTAUR, posOffset(getPlayerPos(), 1, 0)); // start in melee range
    br72064_startTurn = turnNumber();
    br72064_everAdjacent = false;
    br72064_maxDist = 0;
}

static boolean br72064_step(void) {
    restOneTurn(); // the centaur acts; from melee range it must back off
    int d = chebyshevToPlayer(monsterPosition(subject));
    if (d > br72064_maxDist) br72064_maxDist = d;
    if (monsterIsAdjacentToPlayer(subject)) br72064_everAdjacent = true;
    return (turnNumber() - br72064_startTurn) < 8;
}

static boolean br72064_verify(void) {
    TEST_ASSERT(subject != NULL && monsterIsAlive(subject), "centaur spawned");
    TEST_ASSERT(monsterHasTrait(subject, MONST_MAINTAINS_DISTANCE) != 0, "centaur keeps distance");
    TEST_ASSERT(!br72064_everAdjacent,
                "centaur should keep its distance and never settle in melee range");
    TEST_ASSERT(br72064_maxDist >= 3,
                "centaur should back off to about three tiles");
    return !testFailed;
}

// BR30757 — will not follow between levels.  A control monster without the
// trait chases the player down the stairs, so the totem's absence from the new
// level proves the trait, not a broken follow mechanic.  Both are tracked by
// the level they stand on, so a creature the new level generated on its own
// cannot stand in for the control or the totem.
static creature *br30757_totem;
static creature *br30757_follower;
static int br30757_oldDepth;
static int br30757_turns;

static void br30757_setup(void) {
    setArenaRadius(8, FLOOR);
    br30757_totem = spawnMonster(MK_GOBLIN_TOTEM, posOffset(getPlayerPos(), 3, 0));
    br30757_follower = spawnMonster(MK_RAT, posOffset(getPlayerPos(), -3, 0));
    makeMonsterHostile(br30757_totem);
    makeMonsterHostile(br30757_follower);
    br30757_oldDepth = currentDepth();
    br30757_turns = 0;
    descendToNextLevel();
}

static boolean br30757_step(void) {
    if (br30757_follower != NULL && monsterDepth(br30757_follower) == currentDepth()) {
        return false;
    }
    restOneTurn();
    return ++br30757_turns < 60;
}

static boolean br30757_verify(void) {
    TEST_ASSERT(br30757_totem != NULL && br30757_follower != NULL,
                "both monsters spawned");
    if (br30757_totem == NULL || br30757_follower == NULL) {
        return false;
    }
    TEST_ASSERT(monsterHasTrait(br30757_totem, MONST_WILL_NOT_USE_STAIRS),
                "control: the totem's catalog entry says it won't use stairs");
    TEST_ASSERT_EQ(currentDepth(), br30757_oldDepth + 1,
                   "the player should have descended one level");
    TEST_ASSERT(monsterIsAlive(br30757_follower),
                "the follower should still be alive");
    TEST_ASSERT_EQ(monsterDepth(br30757_follower), currentDepth(),
                   "a monster without the trait should follow you between levels");
    TEST_ASSERT(monsterIsAlive(br30757_totem),
                "the totem should still be alive on its old level");
    TEST_ASSERT_EQ(monsterDepth(br30757_totem), br30757_oldDepth,
                   "the totem should stay on its own dungeon level");
    return !testFailed;
}

TRAIT_TEST(br91557, MK_SPIDER, MONST_IMMUNE_TO_WEBS, "spider immune to webs")
TRAIT_TEST(br13361, MK_ARROW_TURRET, MONST_ATTACKABLE_THRU_WALLS, "turret attackable thru walls")
// BR53451 — carries an item: the dragon's catalog entry says it carries one
// every time, so a freshly generated dragon must already be holding an item,
// and the item it holds must drop onto the floor when it dies.  The kill is a
// real melee blow (health trimmed to 1 first so one landed hit finishes it),
// because only the engine's own death path drops what a creature was carrying.
static boolean br53451_generatedHoldingItem;
static int br53451_floorItemsBeforeKill;

static void br53451_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = spawnMonsterWithItemChance(MK_DRAGON, posOffset(getPlayerPos(), 5, 0));
    br53451_generatedHoldingItem = monsterIsCarryingItem(subject);
    br53451_floorItemsBeforeKill = countFloorItems();
    // Stand it next to you and strike it down with a plain sword.
    setMonsterPos(subject, posOffset(getPlayerPos(), 1, 0));
    givePlayerItem(WEAPON, SWORD, 0);
    equipItemFromPack(WEAPON, SWORD);
    setMonsterHealth(subject, 1);
    for (int attacks = 0; monsterIsAlive(subject) && attacks < 50; attacks++) {
        playerAttacksMonster(subject);
    }
}

static boolean br53451_verify(void) {
    TEST_ASSERT(subject != NULL, "dragon spawned");
    TEST_ASSERT(monsterHasTrait(subject, MONST_CARRY_ITEM_100),
                "control: the dragon's catalog entry says it carries an item");
    TEST_ASSERT(br53451_generatedHoldingItem,
                "the dragon should be generated holding an item");
    TEST_ASSERT(monsterIsDead(subject), "the dragon should die from the blows");
    TEST_ASSERT(countFloorItems() == br53451_floorItemsBeforeKill + 1,
                "the item the dragon carried should drop when it dies");
    return !testFailed;
}
TRAIT_TEST(br48675, MK_SPECTRAL_BLADE, MONST_NOT_LISTED_IN_SIDEBAR, "spectral blade not in sidebar")
// BR67941 — never sleeps.  The catalog flag alone only proves the potential,
// so the setup also checks the live behavior: the eel starts awake and
// hunting, never asleep.  A creature without the trait would start asleep
// roughly three times in four (Monsters.c:111), so the two control
// assertions pin the mechanism and the clean starting state.
static void br67941_setup(void) {
    trait_setup(MK_EEL);
}

static boolean br67941_verify(void) {
    TEST_ASSERT(monsterHasTrait(subject, MONST_NEVER_SLEEPS),
                "control: the eel's catalog entry carries the never-sleeps flag");
    TEST_ASSERT(!monsterIsAsleep(subject),
                "the eel should start awake");
    TEST_ASSERT(monsterIsTrackingPlayer(subject),
                "the eel should start hunting, not sleeping");
    return !testFailed;
}
TRAIT_TEST(br46423, MK_WARDEN_OF_YENDOR, MONST_ALWAYS_HUNTING, "warden always hunting")
TRAIT_TEST(br51688, MK_WRAITH, MONST_FLEES_NEAR_DEATH, "wraith flees near death")
TRAIT_TEST(br16717, MK_GUARDIAN, MONST_GETS_TURN_ON_ACTIVATION, "guardian acts on activation")
TRAIT_TEST(br85956, MK_SPIDER, MONST_ALWAYS_USE_ABILITY, "spider always uses ability")
TRAIT_TEST(br39641, MK_SPIDER, MONST_CAST_SPELLS_SLOWLY, "spider casts slowly")
TRAIT_TEST(br21958, MK_GOBLIN_TOTEM, MONST_INANIMATE, "totem inanimate")
TRAIT_TEST(br90225, MK_WARDEN_OF_YENDOR, MONST_INVULNERABLE, "warden invulnerable")
TRAIT_TEST(br95842, MK_REVENANT, MONST_IMMUNE_TO_WEAPONS, "revenant immune to weapons")
// BR37585 — corrodes the weapon that strikes it.  The rule is about the
// attacker's weapon losing an enchantment, so make a real blow against a
// sleeping acid mound (a sleeping defender is always hit) and watch the
// equipped dagger weaken.  The catalog flag is checked only as a control.
// The dagger must be plain: a randomly rolled quietus or slaying runic would
// kill the mound outright and leave the survival control with nothing to see.
static item *br37585_weapon;
static int br37585_before;

static void br37585_setup(void) {
    clearArena();
    removePackItems(WEAPON, DAGGER);
    br37585_weapon = givePlayerPlainWeapon(DAGGER, 3);
    equipItemFromPack(WEAPON, DAGGER);
    subject = placeHostileMonster(MK_ACID_MOUND, posOffset(getPlayerPos(), 1, 0));
    putMonsterToSleep(subject); // a sleeping defender is always hit
    setMonsterHealth(subject, 100); // survive the blow so the corrosion is the only change
    br37585_before = itemEnchant(br37585_weapon);
}

static boolean br37585_step(void) {
    playerAttacksMonster(subject);
    return false; // one blow; verify checks its effect
}

static boolean br37585_verify(void) {
    TEST_ASSERT(subject != NULL && monsterIsAlive(subject),
                "control: the acid mound survived the blow");
    TEST_ASSERT(monsterHasTrait(subject, MONST_DEFEND_DEGRADE_WEAPON),
                "control: the acid mound's catalog entry says it corrodes weapons");
    TEST_ASSERT(equippedWeapon() == br37585_weapon,
                "the dagger should be the equipped weapon");
    TEST_ASSERT_EQ(itemEnchant(br37585_weapon), br37585_before - 1,
                   "hitting the acid mound should weaken the weapon by one enchantment");
    TEST_ASSERT(messageWasShown("weakens"),
                "the corrosion should announce that the weapon weakens");
    return !testFailed;
}
TRAIT_TEST(br13523, MK_DRAGON, MONST_IMMUNE_TO_FIRE, "dragon immune to fire")
TRAIT_TEST(br57968, MK_WILL_O_THE_WISP, MONST_FIERY, "wisp burns")
// BR18737 — reflects about half of magical bolts.  The catalog flag alone only
// proves the potential, so fire real healing bolts at a golem kept at 1 health:
// a bolt that reaches it heals it, while a reflected bolt leaves it at 1.  The
// reflection roll is independent per bolt, so 200 bolts put the count far from
// the tails (about 48% expected, well inside the 30-70% window).  A golem
// without the flag would never reflect, and a creature that reflected every
// bolt would heal on none of them.
#define BR18737_TRIALS 200
static item *br18737_staff;
static int br18737_reflected;
static int br18737_notReflected;

static void br18737_setup(void) {
    setArenaRadius(8, FLOOR);
    subject = placeHostileMonster(MK_GOLEM, posOffset(getPlayerPos(), 1, 0));
    br18737_staff = givePlayerItem(STAFF, STAFF_HEALING, 0);
    br18737_reflected = 0;
    br18737_notReflected = 0;
}

static boolean br18737_step(void) {
    setMonsterHealth(subject, 1);
    playerZapsAt(br18737_staff, subject);
    if (monsterHealth(subject) > 1) {
        br18737_notReflected++;
    } else {
        br18737_reflected++;
    }
    return br18737_reflected + br18737_notReflected < BR18737_TRIALS;
}

static boolean br18737_verify(void) {
    TEST_ASSERT(subject != NULL && monsterIsAlive(subject), "golem spawned");
    TEST_ASSERT(monsterHasTrait(subject, MONST_REFLECT_50),
                "control: the golem's catalog entry reflects half of bolts");
    TEST_ASSERT_EQ(br18737_reflected + br18737_notReflected, BR18737_TRIALS,
                   "all of the bolts should have been fired");
    TEST_ASSERT(br18737_reflected > 0,
                "the golem should reflect some bolts");
    TEST_ASSERT(br18737_notReflected > 0,
                "the golem should not reflect every bolt");
    TEST_ASSERT(br18737_reflected >= BR18737_TRIALS * 30 / 100
                && br18737_reflected <= BR18737_TRIALS * 70 / 100,
                "the golem should reflect about half of the bolts");
    return !testFailed;
}
TRAIT_TEST(br12704, MK_LICH, MONST_NO_POLYMORPH, "lich no polymorph")
TRAIT_TEST(br49476, MK_WILL_O_THE_WISP, MONST_DIES_IF_NEGATED, "wisp dies if negated")
TRAIT_TEST(br56920, MK_PHANTOM, MONST_INVISIBLE, "phantom invisible")

// ---- Abilities (MA_*) ----
ABILITY_TEST(br46956, MK_TOAD, MA_HIT_HALLUCINATE, "toad hallucinate")
ABILITY_TEST(br18776, MK_MONKEY, MA_HIT_STEAL_FLEE, "monkey steal+flee")
// BR32322 — sets the victim alight: a successful hit sets the target on
// fire.  The catalog flag alone only proves the potential, so the test
// lands a real blow through the engine's combat path.  The engine's attack
// roll auto-hits a paralyzed defender (against 35 defense this blow would
// otherwise land only about two times in three), and the wisp's damage is
// {0, 0, 0}, so the victim survives to be set alight.  The two control
// assertions pin the mechanism and the clean starting state, so the final
// assertion can only pass through the hit itself.
static void br32322_setup(void) {
    clearMonsters();
    standPlayerOn(FLOOR);
    subject = spawnMonster(MK_WILL_O_THE_WISP, posOffset(getPlayerPos(), 1, 0));
    applyStatusToPlayer(STATUS_PARALYZED, 100);
}

static boolean br32322_verify(void) {
    TEST_ASSERT(monsterHasAbility(subject, MA_HIT_BURN),
                "control: the wisp's catalog entry carries the burn ability");
    TEST_ASSERT(!isPlayerBurning(),
                "control: you should not be burning before the hit");
    TEST_ASSERT(monsterAttacksPlayer(subject),
                "the wisp's blow should land on a paralyzed target");
    TEST_ASSERT(isPlayerBurning(),
                "a successful hit should set the target alight");
    return !testFailed;
}

ABILITY_TEST(br80252, MK_ACID_MOUND, MA_HIT_DEGRADE_ARMOR, "acid mound corrode armor")
ABILITY_TEST(br45824, MK_SPIDER, MA_POISONS, "spider poisons")
ABILITY_TEST(br32139, MK_CENTIPEDE, MA_CAUSES_WEAKNESS, "centipede weakness")
ABILITY_TEST(br54374, MK_KRAKEN, MA_SEIZES, "kraken grabs")
ABILITY_TEST(br15511, MK_BLOAT, MA_KAMIKAZE, "bloat kamikaze")
ABILITY_TEST(br83324, MK_VAMPIRE_BAT, MA_TRANSFERENCE, "bat transference")
ABILITY_TEST(br97180, MK_PINK_JELLY, MA_CLONE_SELF_ON_DEFEND, "jelly splits")
ABILITY_TEST(br12637, MK_BLOAT, MA_DF_ON_DEATH, "bloat DF on death")
// BR40544 — summons allies: the catalog flag alone only proves the
// potential, so the step gives the lich turns of its own until it calls up
// new minions (while it has none yet, a hostile summoner rolls 50% per turn),
// and the verify requires the newcomers to be hostile creatures fighting
// beside the lich.  A lich without the ability could never raise the count.
static int br40544_monstersBefore;
static int br40544_turns;

static void br40544_setup(void) {
    setArenaRadius(8, FLOOR);
    // Aggravate yourself and run one real turn first: this pins the engine's
    // stealth range high and recomputes the field of view over the cleared
    // arena, so the lich stays aware of you and keeps hunting — and so keeps
    // summoning — instead of dropping into a wandering state.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn();
    subject = placeHostileMonster(MK_LICH, posOffset(getPlayerPos(), 5, 0));
    br40544_monstersBefore = monsterCount();
    br40544_turns = 0;
}

static boolean br40544_step(void) {
    setPlayerHealth(playerMaxHealth()); // it may melee or bolt you while it hunts
    monsterTakesTurn(subject);
    br40544_turns++;
    return monsterCount() <= br40544_monstersBefore && br40544_turns < 60;
}

static boolean br40544_verify(void) {
    creature *minion = NULL;
    for (int i = 0; i < monsterCount() && minion == NULL; i++) {
        creature *m = monsterAtIndex(i);
        if (m != NULL && m != subject && monsterIsAlive(m)) {
            minion = m;
        }
    }
    TEST_ASSERT(subject != NULL && monsterIsAlive(subject), "lich spawned");
    if (subject == NULL) {
        return false;
    }
    TEST_ASSERT_EQ(br40544_monstersBefore, 1,
                   "control: the cleared arena should start with only the lich");
    TEST_ASSERT(monsterHasAbility(subject, MA_CAST_SUMMON),
                "control: the lich's catalog entry carries the summon ability");
    TEST_ASSERT(monsterCount() > br40544_monstersBefore,
                "the lich should call up new minions");
    TEST_ASSERT(minion != NULL,
                "the summoned minion should share the level with the lich");
    TEST_ASSERT(minion != NULL && monsterDepth(minion) == currentDepth(),
                "the summoned minion should stand on the lich's level");
    TEST_ASSERT(minion != NULL && monsterIsHostile(minion),
                "the summoned minion should be hostile to you");
    TEST_ASSERT(minion != NULL && monstersAreAlliesOf(minion, subject),
                "the summoned minion should fight beside the lich that raised it");
    return !testFailed;
}
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
// BR68026 — casts discord on enemies: it turns the victim against its own
// allies so it attacks them, and it will not intentionally target you with it.
// The spell list alone only proves the potential, so the step lets the
// battlemage actually hurl discord at your two allies.  It rolls a fresh 30%
// chance to cast on each of its turns, so the step allows up to 200 of them
// (the chance of no discord in that window is under 1 in 10^6).  Your allies
// are dragons: fire-immune, so the battlemage's flame is never cast at them,
// and — unlike reflective creatures such as the golem — they never send a bolt
// back at the caster or scatter one sideways onto you.  You are aggravated
// throughout (as in br60584_setup below), which keeps the battlemage hunting
// you instead of wandering; it may lob flame at you while it hunts, so every
// step tops your health back up.  The clearing stays open while it casts: the
// battlemage keeps its distance from you and roams this whole arena as it
// hunts, so a screen of wall standing from the outset would eventually find
// itself between the caster and the dragons and silence the very cast being
// tested.  Instead the step raises the screen the instant one of your allies
// turns, before that victim gets a single act — the battlemage is stepped off
// the cells if it happens to be standing on them.  From then on the victim can
// neither reach you nor fire at you (its white-hot dragonfire would kill you
// outright), so every turn it gets is spent on its fellow dragon, which stands
// one tile away while only the victim acts: a wound on that dragon can only
// have come from the victim turning on it.
static creature *br68026_ally1;
static creature *br68026_ally2;
static creature *br68026_victim;
static creature *br68026_other;
static boolean br68026_casterHasDiscord;
static int br68026_otherBaseHealth;
static boolean br68026_attacked;
static int br68026_castTurns;
static int br68026_attackTurns;

// Whether the cell is plain floor inside the level with nothing standing on it.
static boolean br68026_cellIsFree(pos cell) {
    if (cell.x < 0 || cell.y < 0 || cell.x >= levelWidth() || cell.y >= levelHeight()) {
        return false;
    }
    if (terrainAt(cell) != FLOOR) {
        return false;
    }
    for (int i = 0; i < monsterCount(); i++) {
        creature *m = monsterAtIndex(i);
        if (m != NULL && posEq(monsterPosition(m), cell)) {
            return false;
        }
    }
    return true;
}

// An empty floor tile to move a creature off the screen with: anywhere in the
// arena except the column about to be walled up.
static pos br68026_refugeNear(pos screenCell) {
    for (int dx = -8; dx <= 8; dx++) {
        for (int dy = -8; dy <= 8; dy++) {
            pos cand = posOffset(getPlayerPos(), dx, dy);
            if (cand.x == screenCell.x) {
                continue;
            }
            if (br68026_cellIsFree(cand)) {
                return cand;
            }
        }
    }
    return INVALID_POS;
}

// Raise the screen of wall between you and your dragons the instant one of
// them turns, before that victim takes a single act.  The battlemage roams the
// whole clearing while it hunts, so it may be standing on one of the cells;
// step it aside rather than wall it in.  With the screen up the discorded
// victim can neither reach you nor fire at you, so every turn it gets is spent
// on the dragon it once called a friend.
static void br68026_raiseScreen(void) {
    pos refuge = INVALID_POS;
    for (int dy = -1; dy >= -5; dy--) {
        pos cell = posOffset(getPlayerPos(), 2, dy);
        for (int i = 0; i < monsterCount(); i++) {
            creature *m = monsterAtIndex(i);
            if (m == NULL || !posEq(monsterPosition(m), cell)) {
                continue;
            }
            if (posEq(refuge, INVALID_POS)) {
                refuge = br68026_refugeNear(cell);
            }
            if (!posEq(refuge, INVALID_POS)) {
                setMonsterPos(m, refuge);
            }
        }
        setTerrain(cell, WALL);
    }
}

static void br68026_setup(void) {
    setArenaRadius(8, FLOOR);
    // Aggravate yourself and run one real turn first: this pins the engine's
    // stealth range high and recomputes the field of view over the cleared
    // arena, so the battlemage notices you immediately instead of wandering on
    // a per-turn awareness roll and never getting around to casting.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn();
    subject = placeHostileMonster(MK_DAR_BATTLEMAGE, posOffset(getPlayerPos(), 6, 0));
    br68026_casterHasDiscord = monsterCastsBolt(subject, BOLT_DISCORD);
    // Two of your dragons, standing side by side with a clear shot between
    // them and the battlemage, and you out of the line of fire.
    br68026_ally1 = spawnMonster(MK_DRAGON, posOffset(getPlayerPos(), 3, -3));
    makeMonsterAlly(br68026_ally1);
    br68026_ally2 = spawnMonster(MK_DRAGON, posOffset(getPlayerPos(), 4, -3));
    makeMonsterAlly(br68026_ally2);
    br68026_victim = NULL;
    br68026_other = NULL;
    br68026_otherBaseHealth = 0;
    br68026_attacked = false;
    br68026_castTurns = 0;
    br68026_attackTurns = 0;
}

static boolean br68026_step(void) {
    // The battlemage may lob flame at you while it hunts; topping your health
    // back up each step means it can never end the test.
    setPlayerHealth(playerMaxHealth());
    if (br68026_victim == NULL) {
        monsterTakesTurn(subject);
        if (monsterHasStatus(br68026_ally1, STATUS_DISCORDANT)) {
            br68026_victim = br68026_ally1;
            br68026_other = br68026_ally2;
        } else if (monsterHasStatus(br68026_ally2, STATUS_DISCORDANT)) {
            br68026_victim = br68026_ally2;
            br68026_other = br68026_ally1;
        }
        if (br68026_victim != NULL) {
            br68026_otherBaseHealth = monsterHealth(br68026_other);
            br68026_raiseScreen();
            return true; // keep stepping: the victim still has its turn to take
        }
        return ++br68026_castTurns < 200;
    }
    // Only the victim acts from here on, and the screen raised above keeps it
    // from ever reaching you, so any wound its fellow ally takes now can only
    // have come from the victim attacking it.  (Its dragonfire cannot wound a
    // fire-immune dragon, so only melee can drop that health.)
    monsterTakesTurn(br68026_victim);
    if (monsterHealth(br68026_other) < br68026_otherBaseHealth) {
        br68026_attacked = true;
    }
    return !br68026_attacked && ++br68026_attackTurns < 20;
}

static boolean br68026_verify(void) {
    TEST_ASSERT(br68026_casterHasDiscord,
                "control: the dar battlemage should carry the discord bolt");
    TEST_ASSERT(br68026_victim != NULL,
                "the battlemage should cast discord at one of your allies");
    TEST_ASSERT(br68026_victim != NULL
                && monstersAreFoesOf(br68026_victim, br68026_other),
                "discord should turn the victim against its own allies");
    TEST_ASSERT(br68026_attacked,
                "the discorded victim should attack the ally it once called a friend");
    TEST_ASSERT(!playerHasStatus(STATUS_DISCORDANT),
                "the battlemage should not intentionally target you with discord");
    return !testFailed;
}

BOLT_TEST(br81389, MK_DAR_PRIESTESS, BOLT_NEGATION, "dar priestess negates")
// BR60584 — hurls fire bolts: a searing bolt that damages the target and can
// set it alight.  The spell list alone only proves the potential, so the step
// lets the flame turret actually hurl one at you and records the damage and
// the fire it leaves.  The turret is the simplest creature that hurls them:
// immobile, with the fire bolt as its only spell, so it never closes to melee
// and every bolt it casts is the searing one.  It rolls a fresh 30% chance to
// cast on each of its turns, so the step allows up to 60 of them (the chance
// of no bolt in that window is under 1 in 10^9).
static creature *br60584_turret;
static boolean br60584_hit;
static boolean br60584_burning;
static int br60584_turns;

static void br60584_setup(void) {
    setArenaRadius(6, FLOOR);
    // Aggravate yourself and run one real turn first: this pins the engine's
    // stealth range high and recomputes the field of view over the cleared
    // arena, so the turret notices you immediately instead of waking on a
    // per-turn awareness roll and sleeping through the test.
    applyStatusToPlayer(STATUS_AGGRAVATING, 10000);
    restOneTurn();
    br60584_turret = placeHostileMonster(MK_FLAME_TURRET, posOffset(getPlayerPos(), -4, 0));
    br60584_hit = false;
    br60584_burning = false;
    br60584_turns = 0;
}

static boolean br60584_step(void) {
    monsterTakesTurn(br60584_turret);
    if (isPlayerHurt()) {
        br60584_hit = true;
    }
    if (isPlayerBurning()) {
        br60584_burning = true;
    }
    br60584_turns++;
    return !(br60584_hit && br60584_burning) && br60584_turns < 60;
}

static boolean br60584_verify(void) {
    TEST_ASSERT(monsterCastsBolt(br60584_turret, BOLT_FIRE),
                "control: the flame turret should carry the fire bolt");
    TEST_ASSERT(br60584_hit,
                "a searing bolt should damage the target it hits");
    TEST_ASSERT(br60584_burning,
                "a searing bolt should set the target it hits alight");
    return !testFailed;
}

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
// BR24264 — Jackal.  Health 8, defense 0, accuracy 70, damage 2-4;
// regenerates every 20 turns; moves twice as fast as normal
// (speed 50) with a normal attack (100).  It has a 1% chance
// each turn to leave a trail of urine.
//
// The stat block, the two speeds and the urine trail (its chance
// and kind) are read from the spawned jackal's catalog entry.
// The 20-turn regeneration period is watched turn by turn: a
// wounded ally with no enemies about can only regain health
// through regeneration, so it must sit at 1 HP for 19 turns and
// regain exactly one point on the 20th.
// ============================================================

static int br24264_regenTurns;

static void br24264_setup(void) {
    trait_setup(MK_JACKAL);
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br24264_regenTurns = 0;
}

static boolean br24264_step(void) {
    restOneTurn();
    br24264_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br24264_regenTurns < 60;
}

static boolean br24264_verify(void) {
    TEST_ASSERT(subject != NULL, "jackal spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 8, "jackal health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 0, "jackal defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 70, "jackal accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 2, "jackal damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 4, "jackal damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 50,
                   "jackal moves twice as fast as normal");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100,
                   "jackal attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "jackal regenerates naturally");
    TEST_ASSERT_EQ(monsterTrailChance(subject), 1,
                   "jackal leaves a trail on 1% of its turns");
    TEST_ASSERT(monsterLeavesUrineTrail(subject), "jackal's trail is urine");
    TEST_ASSERT_EQ(br24264_regenTurns, 20,
                   "the jackal should wait 20 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the jackal should regain exactly one point of health per period");
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

// ============================================================
// BR27618 — Warden of Yendor.  Health 1000, defense 0, accuracy
// 300, damage 12–17; no regeneration; moves half as fast as
// normal (speed 200) and attacks at half rate (attack speed
// 200).  A large, never-sleeping, always-hunting boss that is
// invulnerable — it cannot be harmed at all — and cannot be
// polymorphed.  It glows.
//
// The stat block, the two half-speed figures, the regeneration
// flag and the four traits are read from the spawned warden's
// catalog entry.  The glow is measured on its own tile and with
// the warden stepped away: its Yendorian light colours the tile
// differently (its green component is negative), so any change in
// the measured light proves an intrinsic light source is there.
// "Cannot be harmed at all" is exercised behaviorally: the
// player strikes it (its defense of 0 means the blow lands) and
// the warden must still be at full health afterwards.
// ============================================================

static int br27618_light_with_glow;
static int br27618_light_without_glow;

static void br27618_setup(void) {
    trait_setup(MK_WARDEN_OF_YENDOR);
    measureMonsterGlow(subject, &br27618_light_with_glow,
                       &br27618_light_without_glow);
    // Stand it next to you and strike: the hit is guaranteed against
    // defense 0, so any damage at all would show the rule is broken.
    setMonsterPos(subject, posOffset(getPlayerPos(), 1, 0));
    playerAttacksMonster(subject);
}

static boolean br27618_verify(void) {
    creature *w = subject;
    TEST_ASSERT(w != NULL, "warden spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(w), 1000, "warden health");
    TEST_ASSERT_EQ(monsterBaseDefense(w), 0, "warden defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(w), 300, "warden accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(w), 12, "warden damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(w), 17, "warden damage high");
    TEST_ASSERT(!monsterRegenerates(w), "warden has no natural regeneration");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(w), 200,
                   "warden moves at half normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(w), 200,
                   "warden attacks at half the normal rate");
    TEST_ASSERT(monsterHasTrait(w, MONST_NEVER_SLEEPS), "warden never sleeps");
    TEST_ASSERT(monsterHasTrait(w, MONST_ALWAYS_HUNTING), "warden always hunts");
    TEST_ASSERT(monsterHasTrait(w, MONST_INVULNERABLE), "warden is invulnerable");
    TEST_ASSERT(monsterHasTrait(w, MONST_NO_POLYMORPH),
                "warden cannot be polymorphed");
    TEST_ASSERT(monsterIsHealthy(w),
                "the warden cannot be harmed at all, even by a guaranteed hit");
    // Its Yendorian light has a negative green component, so standing on
    // the tile changes the summed light either way; an intrinsic light
    // source changes it, no light source leaves it identical.
    TEST_ASSERT(br27618_light_with_glow != br27618_light_without_glow,
                "warden glows with its own light");
    return !testFailed;
}

// ============================================================
// BR17917 — Bloat.  Health 4, defense 0, accuracy 100, damage
// 0 (it never makes a normal attack); regenerates every 5
// turns; normal speeds.  It flies, moves erratically, explodes
// rather than attacking, and releases a cloud on death.
//
// The stat block, the two traits and the two abilities are read
// from the spawned bloat's catalog entry.  The 5-turn
// regeneration period is watched turn by turn: a wounded ally
// with no enemies about can only regain health through
// regeneration, so it must sit at 1 HP for 4 turns and regain
// exactly one point on the 5th.
// ============================================================

static int br17917_regenTurns;

static void br17917_setup(void) {
    trait_setup(MK_BLOAT);
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br17917_regenTurns = 0;
}

static boolean br17917_step(void) {
    restOneTurn();
    br17917_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br17917_regenTurns < 30;
}

static boolean br17917_verify(void) {
    TEST_ASSERT(subject != NULL, "bloat spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 4, "bloat health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 0, "bloat defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 100, "bloat accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 0, "bloat damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 0, "bloat damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 100, "bloat moves at normal speed");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100, "bloat attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "bloat regenerates naturally");
    TEST_ASSERT(monsterHasTrait(subject, MONST_FLIES), "bloat flies");
    TEST_ASSERT(monsterHasTrait(subject, MONST_FLITS), "bloat moves erratically");
    TEST_ASSERT(monsterHasAbility(subject, MA_KAMIKAZE),
                "bloat explodes rather than attacking");
    TEST_ASSERT(monsterHasAbility(subject, MA_DF_ON_DEATH),
                "bloat releases a cloud on death");
    TEST_ASSERT_EQ(br17917_regenTurns, 5,
                   "the bloat should wait 5 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the bloat should regain exactly one point of health per period");
    return !testFailed;
}

// ============================================================
// BR89674 — Pixie.  Health 10, defense 90, accuracy 100, damage
// 1–3; regenerates every 20 turns; moves twice as fast (speed 50)
// with a normal attack (100).  It flies, moves erratically, and
// keeps its distance; it strips magical effects with negation,
// slows enemies, casts discord, and fires lightning bolts; it
// glows.
//
// The stat block, the natural-regeneration flag, the three
// traits and the four spells are read from the spawned pixie's
// catalog entry, and the glow is measured on its own tile and
// with the pixie stepped away.  The 20-turn regeneration period
// is watched turn by turn: a wounded ally with no enemies about
// can only regain health through regeneration, so it must sit at
// 1 HP for 19 turns and regain exactly one point on the 20th.
// ============================================================

static int br89674_light_with_glow;
static int br89674_light_without_glow;
static int br89674_regenTurns;

static void br89674_setup(void) {
    trait_setup(MK_PIXIE);
    measureMonsterGlow(subject, &br89674_light_with_glow,
                       &br89674_light_without_glow);
    // An ally is a harmless bystander while time passes, and a wounded
    // creature would heal during the wait if it did not regenerate on
    // schedule.
    makeMonsterAlly(subject);
    setMonsterHealth(subject, 1);
    br89674_regenTurns = 0;
}

static boolean br89674_step(void) {
    restOneTurn();
    br89674_regenTurns++;
    if (monsterHealth(subject) > 1) {
        return false;
    }
    return br89674_regenTurns < 60;
}

static boolean br89674_verify(void) {
    TEST_ASSERT(subject != NULL, "pixie spawned");
    TEST_ASSERT_EQ(monsterMaxHealth(subject), 10, "pixie health");
    TEST_ASSERT_EQ(monsterBaseDefense(subject), 90, "pixie defense");
    TEST_ASSERT_EQ(monsterBaseAccuracy(subject), 100, "pixie accuracy");
    TEST_ASSERT_EQ(monsterBaseDamageLow(subject), 1, "pixie damage low");
    TEST_ASSERT_EQ(monsterBaseDamageHigh(subject), 3, "pixie damage high");
    TEST_ASSERT_EQ(monsterBaseMovementSpeed(subject), 50,
                   "pixie moves twice as fast as normal");
    TEST_ASSERT_EQ(monsterBaseAttackSpeed(subject), 100,
                   "pixie attacks at normal speed");
    TEST_ASSERT(monsterRegenerates(subject), "pixie regenerates naturally");
    TEST_ASSERT(monsterHasTrait(subject, MONST_FLIES), "pixie flies");
    TEST_ASSERT(monsterHasTrait(subject, MONST_FLITS), "pixie moves erratically");
    TEST_ASSERT(monsterHasTrait(subject, MONST_MAINTAINS_DISTANCE),
                "pixie keeps its distance");
    TEST_ASSERT(monsterCastsBolt(subject, BOLT_NEGATION),
                "pixie strips magical effects with negation");
    TEST_ASSERT(monsterCastsBolt(subject, BOLT_SLOW_2),
                "pixie slows its enemies");
    TEST_ASSERT(monsterCastsBolt(subject, BOLT_DISCORD),
                "pixie casts discord");
    TEST_ASSERT(monsterCastsBolt(subject, BOLT_SPARK),
                "pixie fires lightning bolts");
    TEST_ASSERT(br89674_light_with_glow > br89674_light_without_glow,
                "pixie glows");
    TEST_ASSERT_EQ(br89674_regenTurns, 20,
                   "the pixie should wait 20 turns before regenerating");
    TEST_ASSERT_EQ(monsterHealth(subject), 2,
                   "the pixie should regain exactly one point of health per period");
    return !testFailed;
}

const testCase tests_bestiary[] = {
    { "BR39474_flies", br39474_setup, NULL, br39474_verify },
    { "BR10411_erratic", br10411_setup, NULL, br10411_verify },
    { "BR60148_liquid_bound", br60148_setup, NULL, br60148_verify },
    { "BR47597_full_speed_water", br47597_setup, NULL, br47597_verify },
    { "BR65633_submerges", br65633_setup, NULL, br65633_verify },
    { "BR99362_immobile", br99362_setup, NULL, br99362_verify },
    { "BR72064_keeps_distance", br72064_setup, br72064_step, br72064_verify },
    { "BR30757_wont_use_stairs", br30757_setup, br30757_step, br30757_verify },
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
    { "BR37585_corrodes_weapon", br37585_setup, br37585_step, br37585_verify },
    { "BR13523_immune_fire", br13523_setup, NULL, br13523_verify },
    { "BR57968_burns", br57968_setup, NULL, br57968_verify },
    { "BR18737_reflects_half", br18737_setup, br18737_step, br18737_verify },
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
    { "BR40544_summons", br40544_setup, br40544_step, br40544_verify },
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
    { "BR68026_discord", br68026_setup, br68026_step, br68026_verify },
    { "BR81389_negation", br81389_setup, NULL, br81389_verify },
    { "BR60584_fire", br60584_setup, br60584_step, br60584_verify },
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
    { "BR24264_jackal_stats_trail_and_regeneration", br24264_setup, br24264_step, br24264_verify },
    { "BR69414_imp_stats_and_traits", br69414_setup, NULL, br69414_verify },
    { "BR69414_imp_regenerates_every_10_turns", br69414_regen_setup, br69414_regen_step, br69414_regen_verify },
    { "BR73845_goblin_stats_and_regeneration", br73845_setup, br73845_step, br73845_verify },
    { "BR27618_warden_of_yendor", br27618_setup, NULL, br27618_verify },
    { "BR17917_bloat_stats_and_regeneration", br17917_setup, br17917_step, br17917_verify },
    { "BR89674_pixie_stats_and_regeneration", br89674_setup, br89674_step, br89674_verify },
    { NULL, NULL, NULL, NULL }
};

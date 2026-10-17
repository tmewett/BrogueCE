#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static creature *subject;

static void clearArena(void) {
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            setTerrain(posOffset(player.loc, dx, dy), FLOOR);
        }
    }
    setPlayerPos(player.loc);
    clearMonsters();
}

// BR100001 — killed by monster melee
static creature *killer;
static void melee_death_setup(void) {
    clearArena();
    killer = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(killer);
    revealCell(killer->loc);
    setPlayerHealth(1);
    applyStatusToPlayer(STATUS_PARALYZED, 100);
}
static boolean melee_death_verify(void) {
    monsterAttacksPlayer(killer);
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "rat melee should kill");
    return !testFailed;
}

// BR100005 — starvation
static void starve_setup(void) {
    clearArena();
    setPlayerNutrition(0);
    setPlayerHealth(1);
}
static boolean starve_verify(void) {
    restOneTurn();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "starvation should kill");
    return !testFailed;
}

// BR100006 — poison
static void poison_death_setup(void) {
    clearArena();
    setPlayerHealth(1);
    poisonPlayer(10, 100);
}
static boolean poison_death_verify(void) {
    restOneTurn();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "poison should kill");
    return !testFailed;
}

// BR100007 — burning
static void burn_death_setup(void) {
    clearArena();
    setPlayerHealth(1);
    applyStatusToPlayer(STATUS_BURNING, 10);
}
static boolean burn_death_verify(void) {
    restOneTurn();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "burning should kill");
    return !testFailed;
}

// BR100008 — lava
static void lava_death_setup(void) {
    clearArena();
    setTerrain(player.loc, LAVA);
    setPlayerPos(player.loc);
    revealCell(player.loc);
    setPlayerHealth(1);
}
static boolean lava_death_verify(void) {
    applyTileEffectsToPlayer();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "lava should kill");
    return !testFailed;
}

// BR100009 — explosive terrain
static void explosive_death_setup(void) {
    clearArena();
    setTerrain(player.loc, GAS_EXPLOSION);
    setPlayerPos(player.loc);
    revealCell(player.loc);
    setPlayerHealth(1);
}
static boolean explosive_death_verify(void) {
    applyTileEffectsToPlayer();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "explosion should kill");
    return !testFailed;
}

// BR100002 — killed by monster bolt
static void bolt_death_setup(void) {
    clearArena();
    subject = spawnMonster(MK_LICH, posOffset(player.loc, 4, 0));
    makeMonsterHostile(subject);
    revealCell(subject->loc);
    setPlayerHealth(1);
}
static boolean bolt_death_verify(void) {
    monsterTakesTurn(subject);
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "lich bolt should kill");
    return !testFailed;
}

// BR100011 — killed by fall
static void fall_death_setup(void) {
    clearArena();
    setTerrain(player.loc, CHASM);
    setPlayerPos(player.loc);
    revealCell(player.loc);
    player.bookkeepingFlags |= MB_IS_FALLING;
    setPlayerHealth(1);
}
static boolean fall_death_verify(void) {
    restOneTurn();
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "fall should kill");
    return !testFailed;
}

// BR100004 — drained by cursed ring
static void cursed_ring_death_setup(void) {
    clearArena();
    subject = spawnMonster(MK_RAT, posOffset(player.loc, 1, 0));
    makeMonsterHostile(subject);
    revealCell(subject->loc);
    setMonsterHealth(subject, 1);
    setPlayerHealth(1);
    rogue.transference = -100;
}
static boolean cursed_ring_death_verify(void) {
    playerAttacksMonster(subject);
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "cursed ring drain should kill");
    return !testFailed;
}

// BR100003 — killed by reflected bolt
static void reflected_bolt_death_setup(void) {
    clearArena();
    subject = spawnMonster(MK_GUARDIAN, posOffset(player.loc, 4, 0));
    makeMonsterHostile(subject);
    revealCell(subject->loc);
    setPlayerHealth(1);
}
static boolean reflected_bolt_death_verify(void) {
    zap(player.loc, subject->loc, &boltCatalog[BOLT_FIRE], false, false);
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "reflected bolt should kill");
    return !testFailed;
}

// BR100010 — damaging terrain
static void damaging_death_setup(void) {
    clearArena();
    setTerrain(player.loc, STEAM);
    setPlayerPos(player.loc);
    revealCell(player.loc);
    setPlayerHealth(1);
}
static boolean damaging_death_verify(void) {
    restTurns(3);
    TEST_ASSERT(rogue.gameHasEnded || isPlayerDead(), "damaging terrain should kill");
    return !testFailed;
}

const testCase tests_death[] = {
    { "BR100001_melee_death", melee_death_setup, NULL, melee_death_verify },
    { "BR100005_starvation", starve_setup, NULL, starve_verify },
    { "BR100006_poison", poison_death_setup, NULL, poison_death_verify },
    { "BR100007_burning", burn_death_setup, NULL, burn_death_verify },
    { "BR100008_lava", lava_death_setup, NULL, lava_death_verify },
    { "BR100009_explosive", explosive_death_setup, NULL, explosive_death_verify },
    { "BR100002_bolt", bolt_death_setup, NULL, bolt_death_verify },
    { "BR100011_fall", fall_death_setup, NULL, fall_death_verify },
    { "BR100004_cursed_ring", cursed_ring_death_setup, NULL, cursed_ring_death_verify },
    { "BR100003_reflected_bolt", reflected_bolt_death_setup, NULL, reflected_bolt_death_verify },
    { "BR100010_damaging", damaging_death_setup, NULL, damaging_death_verify },
    { NULL, NULL, NULL, NULL }
};

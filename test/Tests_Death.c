#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

static creature *monst;
static item *theItem;

static void death_setup(void) {
    clearMonsters();
    clearArena();
    setPlayerHealth(1);
    setPlayerPos(player.loc);
    revealCell(player.loc);
}

static creature *spawn_test_monster(enum monsterTypes monstTyp, short xOffset) {
    monst = spawnMonster(monstTyp, posOffset(player.loc, xOffset, 0));
    makeMonsterHostile(monst);
    revealCell(monst->loc);
    return monst;
}

static boolean wait_until_dead_step(void) {
    if (!isPlayerDead() && turnNumber() < 1000) {
        restOneTurn();
        return true; // Keep going
    }
    return false; // Done
}

static boolean death_verify(void) {
    TEST_ASSERT(isPlayerDead() && turnNumber() < 1000, "player should have died");
    return !testFailed;
}

// BR10001 — killed by monster melee
static creature *killer;
static void melee_death_setup(void) {
    death_setup();

    // TODO:
    // - Pick random animate monsters to kill the player with
    // - Verify the death message includes the monster name
    spawn_test_monster(MK_TROLL,1);
    applyStatusToPlayer(STATUS_PARALYZED, 100);
}

// BR10005 — starvation
static void starve_setup(void) {
    death_setup();
    setPlayerNutrition(0);
}

// BR10006 — poison
static void poison_death_setup(void) {
    death_setup();
    poisonPlayer(10, 100);
}

// BR10007 — burning
static void burn_death_setup(void) {
    death_setup();
    applyStatusToPlayer(STATUS_BURNING, 10);
}

// BR10008 — lava
static void lava_death_setup(void) {
    death_setup();
    setTerrain(player.loc, LAVA);
}

// BR10009 — explosive terrain
static void explosive_death_setup(void) {
    death_setup();

    // TODO:
    // - Pick random explosive monsters to kill the player with (eg. explosive mutations)
    spawn_test_monster(MK_EXPLOSIVE_BLOAT,1);
}

// BR10002 — killed by monster bolt
static void bolt_death_setup(void) {
    death_setup();

    // TODO:
    // - Pick random damage-bolt casting monsters to kill the player with
    // - Verify the death message includes the monster name
    spawn_test_monster(MK_SPARK_TURRET, 2);
}

// BR10011 — killed by fall
static void fall_death_setup(void) {
    death_setup();
    setTerrain(player.loc, CHASM);
}

// BR10004 — drained by cursed ring
static void cursed_ring_death_setup(void) {
    death_setup();
    spawn_test_monster(MK_KRAKEN, 1);   // Kraken out of water is helpless

    // Cursed ring
    item *ring = givePlayerItem(RING, RING_TRANSFERENCE, -10);
    equip(ring);
}

static boolean cursed_ring_death_step() {
    moveRight(); // Attack monster
    if (isPlayerDead()) return false;
    return true;
}

// BR10003 — killed by reflected bolt
static void reflected_bolt_death_setup(void) {
    death_setup();
    spawn_test_monster(MK_GUARDIAN, 4);

    // TODO: Test different damaging bolts (fire, lightning, etc)
    theItem = givePlayerItem(STAFF, STAFF_LIGHTNING, 10);
    theItem->charges = 100;
}

static boolean reflected_bolt_death_step(void) {
    playerZapsAt(theItem, monst);
    if (isPlayerDead()) return false;
    return true;
}

// BR10010 — damaging terrain
static void damaging_death_setup(void) {
    death_setup();
    setTerrain(player.loc, STEAM);
}

const testCase tests_death[] = {
    { "BR10001_melee_death",          melee_death_setup,          wait_until_dead_step, death_verify },
    { "BR10005_starvation",           starve_setup,               wait_until_dead_step, death_verify },
    { "BR10006_poison_death",         poison_death_setup,         wait_until_dead_step, death_verify },
    { "BR10007_burning_death",        burn_death_setup,           wait_until_dead_step, death_verify },
    { "BR10008_lava_death",           lava_death_setup,           wait_until_dead_step, death_verify },
    { "BR10009_explosive_death",      explosive_death_setup,      wait_until_dead_step, death_verify },
    { "BR10002_bolt_death",           bolt_death_setup,           wait_until_dead_step, death_verify },
    { "BR10011_fall_death",           fall_death_setup,           wait_until_dead_step, death_verify },
    { "BR10004_cursed_ring",          cursed_ring_death_setup,    cursed_ring_death_step, death_verify },
    { "BR10003_reflected_bolt_death", reflected_bolt_death_setup, reflected_bolt_death_step, death_verify },
    { "BR10010_damaging_death",       damaging_death_setup,       wait_until_dead_step, death_verify },
    { NULL, NULL, NULL, NULL }
};

// Tests derived from rules/05-items.md.
//
// These cover the deterministic, headless-observable item rules: the effects
// of drinking potions, reading a scroll, picking up gold and the Amulet,
// stacking and pack slots, generated charge counts and gold values, and the
// defense an enchanted suit of armor contributes.
//
// Omitted are everything that depends on level/population generation, random
// chances (curses, runics, mutations, spawn counts), flavour shuffling,
// identification RNG, targeting and trajectory prompts, bolt physics and
// reflection, throwing, dropping, machine interactions, and any rule that
// needs a screen or is otherwise not reproducible headlessly.

#include "TestFramework.h"
#include "Globals.h"
#include "GlobalsBase.h"

// ---------------------------------------------
// Shared helpers
// ---------------------------------------------

static creature *subject;
static int before;
static int packedBefore;

static void emptyRoomSetup(void) {
    clearMonsters();
    pos p = player.loc;
    setTerrain(p, FLOOR);
    setPlayerPos(p);
}

// Place a monster of the given kind on a patch of terrain near the player.
static creature *monsterOn(enum monsterTypes type, int dx, int dy, short terrain) {
    pos p = posOffset(player.loc, dx, dy);
    setTerrain(p, terrain);
    return spawnMonster(type, p);
}

// ============================================================
// Potions
// ============================================================

static void life_potion_setup(void) {
    emptyRoomSetup();
    setPlayerHealth(1);
    before = playerMaxHealth();
    drink(POTION_LIFE);
}

static boolean life_potion_raises_max_health(void) {
    TEST_ASSERT(playerMaxHealth() > before,
                "a potion of life should raise maximum health");
    TEST_ASSERT_EQ(playerHealth(), playerMaxHealth(),
                   "a potion of life should heal you completely");
    return !testFailed;
}

static void strength_potion_setup(void) {
    emptyRoomSetup();
    weakenPlayer(50);
    before = playerStrength();
    drink(POTION_STRENGTH);
}

static boolean strength_potion_raises_strength(void) {
    TEST_ASSERT(playerStrength() > before,
                "a potion of strength should raise strength");
    TEST_ASSERT_EQ(playerWeaknessAmount(), 0,
                   "a potion of strength should clear weakness");
    return !testFailed;
}

static void telepathy_potion_setup(void) {
    emptyRoomSetup();
    drink(POTION_TELEPATHY);
}

static boolean telepathy_potion_grants_telepathy(void) {
    TEST_ASSERT(playerHasStatus(STATUS_TELEPATHIC),
                "a potion of telepathy should make you telepathic");
    return !testFailed;
}

static void levitation_potion_setup(void) {
    emptyRoomSetup();
    drink(POTION_LEVITATION);
}

static boolean levitation_potion_makes_you_float(void) {
    TEST_ASSERT(playerHasStatus(STATUS_LEVITATING),
                "a potion of levitation should make you levitate");
    return !testFailed;
}

static void haste_potion_setup(void) {
    emptyRoomSetup();
    drink(POTION_HASTE_SELF);
}

static boolean haste_potion_hastes_you(void) {
    TEST_ASSERT(playerHasStatus(STATUS_HASTED),
                "a potion of haste should haste you");
    return !testFailed;
}

static void fire_immunity_potion_setup(void) {
    emptyRoomSetup();
    applyStatusToPlayer(STATUS_BURNING, 10);
    drink(POTION_FIRE_IMMUNITY);
}

static boolean fire_immunity_potion_douses_fire(void) {
    TEST_ASSERT(playerHasStatus(STATUS_IMMUNE_TO_FIRE),
                "a potion of fire immunity should make you immune to fire");
    TEST_ASSERT(!isPlayerBurning(),
                "a potion of fire immunity should extinguish fire on you");
    return !testFailed;
}

static void invisibility_potion_setup(void) {
    emptyRoomSetup();
    drink(POTION_INVISIBILITY);
}

static boolean invisibility_potion_hides_you(void) {
    TEST_ASSERT(playerHasStatus(STATUS_INVISIBLE),
                "a potion of invisibility should make you invisible");
    return !testFailed;
}

static void darkness_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_DARKNESS);
}

static boolean darkness_potion_darkens_you(void) {
    TEST_ASSERT(playerHasStatus(STATUS_DARKNESS),
                "a potion of darkness should darken your vision");
    return !testFailed;
}

static void hallucination_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_HALLUCINATION);
}

static boolean hallucination_potion_confuses_colors(void) {
    TEST_ASSERT(playerHasStatus(STATUS_HALLUCINATING),
                "a potion of hallucination should make you hallucinate");
    return !testFailed;
}

static void poison_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_POISON);
}

static boolean poison_potion_releases_gas(void) {
    TEST_ASSERT(cellHasGas(player.loc),
                "a potion of poison should release gas where you stand");
    return !testFailed;
}

static void paralysis_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_PARALYSIS);
}

static boolean paralysis_potion_releases_gas(void) {
    TEST_ASSERT(cellHasGas(player.loc),
                "a potion of paralysis should release gas where you stand");
    return !testFailed;
}

static void confusion_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_CONFUSION);
}

static boolean confusion_potion_releases_gas(void) {
    TEST_ASSERT(cellHasGas(player.loc),
                "a potion of confusion should release gas where you stand");
    return !testFailed;
}

static void incineration_potion_setup(void) {
    emptyRoomSetup();
    queueConfirmYes();
    drink(POTION_INCINERATION);
}

static boolean incineration_potion_sets_you_on_fire(void) {
    TEST_ASSERT(isPlayerBurning(),
                "a potion of incineration should set you on fire");
    return !testFailed;
}

// ============================================================
// Scrolls
// ============================================================

static void aggravate_scroll_setup(void) {
    emptyRoomSetup();
    subject = monsterOn(MK_RAT, 5, 0, FLOOR);
    putMonsterToSleep(subject);
    queueConfirmYes();
    readScrollOfKind(SCROLL_AGGRAVATE_MONSTER);
}

static boolean aggravate_scroll_wakes_monsters(void) {
    TEST_ASSERT(!monsterIsAsleep(subject),
                "a scroll of aggravate monsters should wake every monster");
    return !testFailed;
}

static void consume_scroll_setup(void) {
    emptyRoomSetup();
    readScrollOfKind(SCROLL_RECHARGING);
}

static boolean reading_scroll_consumes_it(void) {
    TEST_ASSERT_EQ(packStackCount(SCROLL, SCROLL_RECHARGING), 0,
                   "reading a scroll should consume it");
    return !testFailed;
}

// ============================================================
// Picking things up
// ============================================================

static void gold_pickup_setup(void) {
    emptyRoomSetup();
    pos p = posOffset(player.loc, 1, 0);
    setTerrain(p, FLOOR);
    item *gold = placeItemOnFloor(GOLD, 0, p);
    before = itemQuantity(gold);
    packedBefore = playerGold();
    moveRight();
}

static boolean gold_pickup_adds_to_total(void) {
    TEST_ASSERT_EQ(playerGold(), packedBefore + before,
                   "picking up gold should add its amount to your total");
    TEST_ASSERT(messageWasShown("pieces of gold"),
                "picking up gold should print the amount found");
    return !testFailed;
}

static void second_amulet_setup(void) {
    emptyRoomSetup();
    givePlayerItem(AMULET, 0, 0);
    pos p = posOffset(player.loc, 1, 0);
    setTerrain(p, FLOOR);
    placeItemOnFloor(AMULET, 0, p);
    moveRight();
}

static boolean second_amulet_cannot_be_taken(void) {
    TEST_ASSERT_EQ(packUnitCount(AMULET, 0), 1,
                   "you should never carry more than one Amulet of Yendor");
    TEST_ASSERT(messageWasShown("already have the Amulet"),
                "picking up a second Amulet should be refused");
    return !testFailed;
}

static boolean amulet_is_worth_35000(void) {
    item *amulet = givePlayerItem(AMULET, 0, 0);
    TEST_ASSERT_EQ((int)itemGoldValue(amulet), 35000,
                   "the Amulet of Yendor should be worth 35,000 gold");
    return !testFailed;
}

static boolean lumenstone_is_worth_5000(void) {
    item *gem = givePlayerItem(GEM, 0, 0);
    TEST_ASSERT_EQ((int)itemGoldValue(gem), 5000,
                   "a lumenstone should be worth 5,000 gold");
    return !testFailed;
}

static void gold_pile_setup(void) {
    emptyRoomSetup();
    pos p = posOffset(player.loc, 1, 0);
    setTerrain(p, FLOOR);
    item *gold = placeItemOnFloor(GOLD, 0, p);
    before = itemQuantity(gold);
}

static boolean gold_pile_is_in_range(void) {
    int depth = currentDepth();
    TEST_ASSERT(before >= 50 + 10 * depth && before <= 100 + 15 * depth,
                "a gold pile should be worth between 50+10*depth and 100+15*depth");
    return !testFailed;
}

// ============================================================
// Generated charges and stacking
// ============================================================

static boolean staff_starts_with_charges(void) {
    item *staff = givePlayerItem(STAFF, STAFF_LIGHTNING, 0);
    TEST_ASSERT(itemCharges(staff) >= 2,
                "a staff should start with at least 2 charges");
    return !testFailed;
}

static boolean charm_starts_ready(void) {
    item *charm = givePlayerItem(CHARM, CHARM_HEALTH, 0);
    TEST_ASSERT_EQ(itemCharges(charm), 0,
                   "a charm should start fully ready to use");
    return !testFailed;
}

static boolean same_potions_stack(void) {
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
    TEST_ASSERT_EQ(packStackCount(POTION, POTION_LIFE), 1,
                   "potions of the same kind should share one stack");
    TEST_ASSERT_EQ(packUnitCount(POTION, POTION_LIFE), 2,
                   "both potions should be in that stack");
    return !testFailed;
}

static void potion_slots_setup(void) {
    emptyRoomSetup();
    packedBefore = packItemCount();
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
    givePlayerItem(POTION, POTION_LIFE, 0);
}

static boolean potions_take_one_slot_each(void) {
    TEST_ASSERT_EQ(packItemCount(), packedBefore + 3,
                   "three potions should occupy three pack slots");
    return !testFailed;
}

static boolean items_have_distinct_letters(void) {
    item *a = givePlayerItem(POTION, POTION_LIFE, 0);
    item *b = givePlayerItem(POTION, POTION_STRENGTH, 0);
    char la = itemInventoryLetter(a);
    char lb = itemInventoryLetter(b);
    TEST_ASSERT(la >= 'a' && la <= 'z' && lb >= 'a' && lb <= 'z' && la != lb,
                "each carried item should get a distinct inventory letter a-z");
    return !testFailed;
}

// ============================================================
// Equipment
// ============================================================

static boolean armor_adds_ten_defense_per_enchant(void) {
    emptyRoomSetup();
    wearArmorOfEnchant(LEATHER_ARMOR, 0);
    int plainDefense = playerDefense();
    wearArmorOfEnchant(LEATHER_ARMOR, 1);
    int enchantedDefense = playerDefense();
    TEST_ASSERT_EQ(enchantedDefense - plainDefense, 10,
                   "each enchantment level of armor should add 10 defense");
    return !testFailed;
}

static boolean donning_armor_reduces_defense(void) {
    emptyRoomSetup();
    wearArmorOfEnchant(LEATHER_ARMOR, 0);
    int donnedDefense = playerDefense();
    takeOffArmor();
    wearArmor(LEATHER_ARMOR); // worn without force, so you are still donning it
    int midDonningDefense = playerDefense();
    TEST_ASSERT(midDonningDefense < donnedDefense,
                "armor should add less defense while it is being donned");
    return !testFailed;
}

// ============================================================

const testCase tests_items[] = {
    { "BR83362_potion_of_life_raises_max_health",   life_potion_setup,          NULL, life_potion_raises_max_health },
    { "BR50436_potion_of_strength_raises_strength", strength_potion_setup,      NULL, strength_potion_raises_strength },
    { "BR53042_potion_of_telepathy_grants_telepathy", telepathy_potion_setup,   NULL, telepathy_potion_grants_telepathy },
    { "BR37943_potion_of_levitation_makes_you_float", levitation_potion_setup,  NULL, levitation_potion_makes_you_float },
    { "BR26830_potion_of_haste_hastes_you",         haste_potion_setup,         NULL, haste_potion_hastes_you },
    { "BR89885_potion_of_fire_immunity_douses_fire", fire_immunity_potion_setup, NULL, fire_immunity_potion_douses_fire },
    { "BR91656_potion_of_invisibility_hides_you",   invisibility_potion_setup,  NULL, invisibility_potion_hides_you },
    { "BR86288_potion_of_darkness_darkens_you",     darkness_potion_setup,      NULL, darkness_potion_darkens_you },
    { "BR37436_potion_of_hallucination_confuses_colors", hallucination_potion_setup, NULL, hallucination_potion_confuses_colors },
    { "BR12169_potion_of_poison_releases_gas",      poison_potion_setup,        NULL, poison_potion_releases_gas },
    { "BR29835_potion_of_paralysis_releases_gas",   paralysis_potion_setup,     NULL, paralysis_potion_releases_gas },
    { "BR83854_potion_of_confusion_releases_gas",   confusion_potion_setup,     NULL, confusion_potion_releases_gas },
    { "BR43470_potion_of_incineration_sets_you_on_fire", incineration_potion_setup, NULL, incineration_potion_sets_you_on_fire },

    { "BR29492_scroll_of_aggravate_wakes_monsters", aggravate_scroll_setup,     NULL, aggravate_scroll_wakes_monsters },
    { "BR95938_reading_scroll_consumes_it",         consume_scroll_setup,       NULL, reading_scroll_consumes_it },

    { "BR30836_gold_pickup_adds_to_total",          gold_pickup_setup,          NULL, gold_pickup_adds_to_total },
    { "BR31757_second_amulet_cannot_be_taken",      second_amulet_setup,        NULL, second_amulet_cannot_be_taken },
    { "BR13028_amulet_is_worth_35000",              NULL,                       NULL, amulet_is_worth_35000 },
    { "BR13028_lumenstone_is_worth_5000",           NULL,                       NULL, lumenstone_is_worth_5000 },
    { "BR54346_gold_pile_is_in_range",              gold_pile_setup,            NULL, gold_pile_is_in_range },

    { "BR39162_staff_starts_with_charges",          NULL,                       NULL, staff_starts_with_charges },
    { "BR22095_charm_starts_ready",                 NULL,                       NULL, charm_starts_ready },
    { "BR35670_same_potions_stack",                 NULL,                       NULL, same_potions_stack },
    { "BR36426_potions_take_one_slot_each",         potion_slots_setup,         NULL, potions_take_one_slot_each },
    { "BR32867_items_have_distinct_letters",        NULL,                       NULL, items_have_distinct_letters },

    { "BR30796_armor_adds_ten_defense_per_enchant", NULL,                       NULL, armor_adds_ten_defense_per_enchant },
    { "BR30796_donning_armor_reduces_defense",      NULL,                       NULL, donning_armor_reduces_defense },

    { NULL, NULL, NULL, NULL }
};
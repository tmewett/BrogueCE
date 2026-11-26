// Tests derived from rules/05-items.md.
//
// These cover the deterministic, headless-observable item rules: the effects
// of drinking potions, a scroll, picking up gold and the Amulet, the tile a
// generated charge counts and gold
// values, the defense an enchanted suit of armor contributes, the specialist
// feat, the number of items each level generates, that generated items avoid
// hallways and cluster in rooms reached through secret doors, that generated
// throwing weapons come out plain (never cursed, runic, or enchanted), the runes
// fading from runic gear whose enchantment drops to zero or less, which
// targets a zap's auto-targeting will and will not pick, the pressure plate
// an item placed on it depresses, and the two-ring limit with its swap prompt.
//
// Omitted are everything else about level population (what kinds are
// generated and placements other than the corridor/secret-room bias), random
// chances (curses, runics, mutations, spawn counts), flavour
// shuffling, identification RNG, the rest of the targeting prompts (cursor
// movement and tab-cycling), dropping, machine interactions other than
// switch promotion and pressure plates, and any rule that needs a screen or
// is otherwise not reproducible headlessly.
// Two aim rules do reach the screen: the throw trajectory's highlight and
// the auto-targeted zap's trajectory are compared frame by frame through
// the screen-colour snapshot helpers.

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
    pos p = getPlayerPos();
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

    // Apply all the statuses to the player
    applyStatusToPlayer(STATUS_WEAKENED, 10);
    applyStatusToPlayer(STATUS_HALLUCINATING, 10);
    applyStatusToPlayer(STATUS_CONFUSED, 10);
    applyStatusToPlayer(STATUS_POISONED, 10);
    applyStatusToPlayer(STATUS_NAUSEOUS, 10);
    applyStatusToPlayer(STATUS_DARKNESS, 10);
    applyStatusToPlayer(STATUS_SLOWED, 10);

    applyStatusToPlayer(STATUS_TELEPATHIC, 10);
    applyStatusToPlayer(STATUS_LEVITATING, 10);
    applyStatusToPlayer(STATUS_HASTED, 10);
    applyStatusToPlayer(STATUS_BURNING, 10);
    applyStatusToPlayer(STATUS_STUCK, 10);
    applyStatusToPlayer(STATUS_IMMUNE_TO_FIRE, 10);
    applyStatusToPlayer(STATUS_SHIELDED, 10);
    applyStatusToPlayer(STATUS_INVISIBLE, 10);

    drink(POTION_LIFE);
}

static boolean life_potion_raises_max_health(void) {
    boolean statusCured = 
           !playerHasStatus(STATUS_WEAKENED)
        && !playerHasStatus(STATUS_HALLUCINATING)
        && !playerHasStatus(STATUS_CONFUSED)
        && !playerHasStatus(STATUS_POISONED)
        && !playerHasStatus(STATUS_NAUSEOUS)
        && !playerHasStatus(STATUS_DARKNESS)
        && !playerHasStatus(STATUS_SLOWED);

    boolean statusRemains =
           playerHasStatus(STATUS_TELEPATHIC)
        && playerHasStatus(STATUS_LEVITATING)
        && playerHasStatus(STATUS_HASTED)
        && playerHasStatus(STATUS_BURNING)
        && playerHasStatus(STATUS_STUCK)
        && playerHasStatus(STATUS_IMMUNE_TO_FIRE)
        && playerHasStatus(STATUS_SHIELDED)
        && playerHasStatus(STATUS_INVISIBLE);


    TEST_ASSERT(statusCured && statusRemains,
                "potion of life cures curable ailments and not incurable statuses");

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
    // The cloud settles over the tile you drank it on; run the engine's own
    // tile-effect pass so the gas can take effect before the assertions.
    applyTileEffectsToPlayer();
}

static boolean paralysis_potion_releases_gas(void) {
    TEST_ASSERT(cellHasGas(getPlayerPos()),
                "a potion of paralysis should release gas where you stand");
    TEST_ASSERT(isPlayerParalyzed(),
                "the gas a potion of paralysis releases should be paralyzing");
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
    subject = monsterOn(MK_RAT, 3, 0, FLOOR);
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
// BR47462 — a scroll of enchanting also removes the chosen item's curse
// ============================================================

static item *enchantedItem;
static char enchantedItemLetter;

static void enchanting_removes_curse_setup(void) {
    clearArena();
    enchantedItem = givePlayerItem(WEAPON, DAGGER, 0);
    curseItem(enchantedItem);
    enchantedItemLetter = itemInventoryLetter(enchantedItem);
    givePlayerItem(SCROLL, SCROLL_ENCHANTING, 0);
    queueKeystroke(enchantedItemLetter, false, false);
}

static boolean enchanting_removes_curse_step(void) {
    readScrollOfKind(SCROLL_ENCHANTING);
    return false;
}

static boolean curse_removed_by_enchanting(void) {
    TEST_ASSERT(!itemIsCursed(enchantedItem),
                "a scroll of enchanting should remove the chosen item's curse");
    TEST_ASSERT(messageWasShown("malevolent force leaves"),
                "removing a curse should announce it");
    return !testFailed;
}

// ============================================================
// BR46995 — a weapon, armor, staff, ring, or charm that reaches
// +16 enchantment earns the "specialist" feat
// ============================================================

static item *specialistWeapon;

static void specialist_earned_setup(void) {
    clearArena();
    specialistWeapon = givePlayerItem(WEAPON, DAGGER, 15);
    TEST_ASSERT(!playerHasFeat(FEAT_SPECIALIST),
                "setup: the feat must not be earned before the enchantment reaches +16");
    queueKeystroke(itemInventoryLetter(specialistWeapon), false, false);
}

static boolean specialist_earned_step(void) {
    readScrollOfKind(SCROLL_ENCHANTING);
    return false;
}

static boolean specialist_feat_at_plus_16(void) {
    TEST_ASSERT_EQ(itemEnchant(specialistWeapon), 16,
                   "the scroll should raise the dagger from +15 to +16");
    TEST_ASSERT(playerHasFeat(FEAT_SPECIALIST),
                "reaching +16 enchantment on a weapon should earn the specialist feat");
    return !testFailed;
}

static item *subSpecialistWeapon;

static void specialist_denied_setup(void) {
    clearArena();
    subSpecialistWeapon = givePlayerItem(WEAPON, DAGGER, 14);
    queueKeystroke(itemInventoryLetter(subSpecialistWeapon), false, false);
}

static boolean specialist_denied_step(void) {
    readScrollOfKind(SCROLL_ENCHANTING);
    return false;
}

static boolean no_specialist_below_plus_16(void) {
    TEST_ASSERT_EQ(itemEnchant(subSpecialistWeapon), 15,
                   "the scroll should raise the dagger from +14 to +15");
    TEST_ASSERT(!playerHasFeat(FEAT_SPECIALIST),
                "an enchantment below +16 should not earn the specialist feat");
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

// ============================================================
// BR61649 — picking an item up triggers the effects of the tile
// it was lifted from (for example a switch tile)
// ============================================================

static pos pickupSwitchLoc;
static pos pickupFloorLoc;

static void pickup_promotes_switch_setup(void) {
    emptyRoomSetup();

    // A switch tile promotes when an item is lifted off it.
    pickupSwitchLoc = posOffset(player.loc, 1, 0);
    setTerrain(pickupSwitchLoc, ALTAR_SWITCH);
    placeItemOnFloor(GOLD, 0, pickupSwitchLoc);
    moveRight();

    // Control: the same pickup from ordinary floor leaves the tile alone.
    pickupFloorLoc = posOffset(player.loc, 1, 0);
    setTerrain(pickupFloorLoc, FLOOR);
    placeItemOnFloor(GOLD, 0, pickupFloorLoc);
    moveRight();
}

static boolean pickup_triggers_tile_effects(void) {
    TEST_ASSERT(findFloorItem(GOLD, 0) == NULL,
                "both piles should have been picked up");
    TEST_ASSERT(cellIsTerrain(pickupSwitchLoc, ALTAR_INERT),
                "picking an item up from a switch tile should promote the tile");
    TEST_ASSERT(cellIsTerrain(pickupFloorLoc, FLOOR),
                "picking an item up from ordinary floor should leave the tile as floor");
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

static boolean potions_do_not_stack(void) {
    givePlayerItem(POTION, POTION_LIFE, 0);
    int onePotion = numberOfItemsInPack();
    givePlayerItem(POTION, POTION_LIFE, 0);
    int twoPotion = numberOfItemsInPack();
    TEST_ASSERT(onePotion < twoPotion, "potions should not stack");
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
    applyEquipmentStatusToPlayer(STATUS_DONNING, 0);
    int donning0 = playerDefense();
    applyEquipmentStatusToPlayer(STATUS_DONNING, 1);
    int donning1 = playerDefense();
    applyEquipmentStatusToPlayer(STATUS_DONNING, 2);
    int donning2 = playerDefense();
    TEST_ASSERT(donning0 > donning1 && donning1 > donning2,
                "armor should add less defense while it is being donned");
    return !testFailed;
}

// ============================================================
// BR83562 — a suit of armor identifies itself after it has been
// worn for a set number of turns.
//
// The armor starts unidentified and carries the wearing countdown
// the engine will run down while it stays worn.  Resting one turn
// short of that countdown must leave it unidentified; the next
// turn must identify it.  Reading the countdown from the item,
// rather than assuming a fixed number, keeps the test tied to the
// rule instead of a particular game variant.
// ============================================================

static item *br83562_armor;
static int br83562_delay;
static int br83562_startTurn;

static void br83562_setup(void) {
    clearArena();
    br83562_armor = givePlayerRunicArmor(LEATHER_ARMOR, A_DAMPENING, 5, false);
    TEST_ASSERT(br83562_armor != NULL, "setup: the armor should be created");
    if (br83562_armor == NULL) {
        return;
    }
    TEST_ASSERT(equippedArmor() == br83562_armor,
                "setup: the armor should be worn");
    TEST_ASSERT(!itemIsIdentified(br83562_armor),
                "setup: the armor should start unidentified");
    br83562_delay = itemCharges(br83562_armor);
    TEST_ASSERT(br83562_delay > 1,
                "setup: the armor should carry a wearing countdown");
    br83562_startTurn = playerTurnCount();
}

static boolean br83562_step(void) {
    if (playerTurnCount() - br83562_startTurn < br83562_delay - 1) {
        restOneTurn();
        return true;
    }
    return false;
}

static boolean br83562_armor_identifies_after_wearing_delay(void) {
    if (br83562_armor == NULL) {
        return false;
    }
    TEST_ASSERT(!itemIsIdentified(br83562_armor),
                "the armor should still be unidentified one turn before the wearing delay ends");
    restOneTurn();
    TEST_ASSERT(itemIsIdentified(br83562_armor),
                "the armor should identify itself once it has been worn for the whole delay");
    return !testFailed;
}

// ============================================================
// BR56988 — enchanting a ring raises its enchantment level, and
// enchanting a ring of clairvoyance immediately updates your vision
// ============================================================

static item *enchantedRing;
static int ringEnchantBefore;

static void ring_enchant_setup(void) {
    clearArena();
    enchantedRing = givePlayerItem(RING, RING_STEALTH, 1);
    ringEnchantBefore = itemEnchant(enchantedRing);
    queueKeystroke(itemInventoryLetter(enchantedRing), false, false);
}

static boolean ring_enchant_step(void) {
    readScrollOfKind(SCROLL_ENCHANTING);
    return false;
}

static boolean enchanting_a_ring_raises_its_enchant(void) {
    TEST_ASSERT(itemEnchant(enchantedRing) > ringEnchantBefore,
                "enchanting a ring should raise its enchantment level");
    return !testFailed;
}

static item *clairvoyanceRing;
static pos visionCell;
static boolean visionCellVisibleBefore;

static void ring_clairvoyance_setup(void) {
    clearArena();
    pos p = getPlayerPos();
    // A wall east of the player keeps the target cell out of ordinary sight.
    for (int dy = -4; dy <= 4; dy++) {
        setTerrain(posOffset(p, 1, dy), GRANITE);
    }
    visionCell = posOffset(p, 2, 2);
    setTerrain(visionCell, FLOOR);
    clairvoyanceRing = givePlayerItem(RING, RING_CLAIRVOYANCE, 1);
    equipItemFromPack(RING, RING_CLAIRVOYANCE);
}

static boolean ring_clairvoyance_step(void) {
    restOneTurn(); // refresh sight with the wall in place
    visionCellVisibleBefore = cellIsVisible(visionCell);
    queueKeystroke(itemInventoryLetter(clairvoyanceRing), false, false);
    readScrollOfKind(SCROLL_ENCHANTING);
    return false;
}

static boolean enchanting_clairvoyance_updates_vision(void) {
    TEST_ASSERT(!visionCellVisibleBefore,
                "the cell behind the wall should be out of sight before the enchant");
    TEST_ASSERT(cellIsVisible(visionCell),
                "enchanting a ring of clairvoyance should immediately update your vision");
    return !testFailed;
}

// ============================================================
// BR40130 — wearing one or two rings adds their enchantment to your
// clairvoyance, stealth, awareness, wisdom, regeneration,
// transference, light, and reaping bonuses
// ============================================================

static const short br40130_ringKinds[8] = {
    RING_CLAIRVOYANCE, RING_STEALTH, RING_REGENERATION, RING_TRANSFERENCE,
    RING_LIGHT, RING_AWARENESS, RING_WISDOM, RING_REAPING
};
static int (*const br40130_bonusOf[8])(void) = {
    playerClairvoyanceBonus, playerStealthBonus, playerRegenerationBonus,
    playerTransferenceBonus, playerLightMultiplier, playerAwarenessBonus,
    playerWisdomBonus, playerReapingBonus
};
// Awareness is the one bonus the engine scales: 20 points per enchant.
static const int br40130_perEnchant[8] = {1, 1, 1, 1, 1, 20, 1, 1};
static int br40130_singleBefore[8], br40130_singleAfter[8];
static int br40130_pairBefore[2], br40130_pairAfter[2];

static void ring_bonuses_setup(void) {
    for (int i = 0; i < 8; i++) {
        br40130_singleBefore[i] = br40130_bonusOf[i]();
    }
    // One ring at a time: wear it, read the bonuses, take it off again.
    for (int i = 0; i < 8; i++) {
        givePlayerItem(RING, br40130_ringKinds[i], 3);
        equipItemFromPack(RING, br40130_ringKinds[i]);
        br40130_singleAfter[i] = br40130_bonusOf[i]();
        unequipItemFromPack(RING, br40130_ringKinds[i]);
    }

    // Two rings at once, one on each finger, with different enchants.  The
    // spent rings are dropped so each findInventoryItem() picks the ring
    // just given.
    removePackItems(RING, RING_STEALTH);
    removePackItems(RING, RING_WISDOM);
    br40130_pairBefore[0] = playerStealthBonus();
    br40130_pairBefore[1] = playerWisdomBonus();
    givePlayerItem(RING, RING_STEALTH, 2);
    equipItemFromPack(RING, RING_STEALTH);
    givePlayerItem(RING, RING_WISDOM, 5);
    equipItemFromPack(RING, RING_WISDOM);
    br40130_pairAfter[0] = playerStealthBonus();
    br40130_pairAfter[1] = playerWisdomBonus();
}

static boolean ring_enchant_adds_to_each_bonus(void) {
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQ(br40130_singleAfter[i] - br40130_singleBefore[i],
                       3 * br40130_perEnchant[i],
                       "wearing a +3 ring should add its enchant to the matching bonus");
    }
    TEST_ASSERT_EQ(br40130_pairAfter[0] - br40130_pairBefore[0], 2,
                   "wearing two rings should add the stealth ring's enchant");
    TEST_ASSERT_EQ(br40130_pairAfter[1] - br40130_pairBefore[1], 5,
                   "wearing two rings should add the wisdom ring's enchant");
    return !testFailed;
}

// ============================================================
// BR15388 — a thrown weapon that hits deals its damage (modified
// by its enchantment); a thrown weapon that misses lands on the floor
// ============================================================

static creature *throwTarget;
static int throwTargetHealthBefore;
static int daggersBeforeThrow;

static void throw_hit_setup(void) {
    clearArena();
    // A paralyzed target is always hit, so the damage assertion is
    // independent of the to-hit roll.
    throwTarget = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 2, 0));
    applyStatusToMonster(throwTarget, STATUS_PARALYZED, 100);
    setMonsterHealth(throwTarget, 50);
    throwTargetHealthBefore = monsterHealth(throwTarget);
    // A plain dagger: a randomly rolled W_MERCY runic heals the target back to
    // full and would erase the damage this test asserts on.
    item *dagger = givePlayerPlainWeapon(DAGGER, 3);
    daggersBeforeThrow = packUnitCount(WEAPON, DAGGER);
    queueKeystroke(itemInventoryLetter(dagger), false, false);
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RETURN_KEY, false, false); // confirm the targeted monster
    {
        item *eq = equippedWeapon();
        item *ar = equippedArmor();
    }
}

static boolean throw_hit_step(void) {
    // A step that moves and steps back forces the engine to rebuild its
    // sidebar entity list, so throw auto-targeting deterministically selects
    // the target (headless tests otherwise see a stale list).
    moveRight();
    moveLeft();
    injectKeystroke(THROW_KEY, false, false);
    return false;
}

static boolean thrown_weapon_hit_deals_damage(void) {
    TEST_ASSERT(monsterHealth(throwTarget) < throwTargetHealthBefore,
                "a thrown weapon that hits should deal its damage to the target");
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DAGGER), daggersBeforeThrow - 1,
                   "a thrown weapon should leave the pack when thrown");
    if (testFailed) {
        pos tp = monsterPosition(throwTarget);
        pos pp = getPlayerPos();
        item *floor = findFloorItem(WEAPON, DAGGER);
        item *inpack = findInventoryItem(WEAPON, DAGGER);
    }
    return !testFailed;
}

static void throw_miss_setup(void) {
    clearArena();
    item *dagger = givePlayerItem(WEAPON, DAGGER, 0);
    daggersBeforeThrow = packUnitCount(WEAPON, DAGGER);
    queueKeystroke(itemInventoryLetter(dagger), false, false);
    // Aim at empty floor: move the targeting cursor off your own tile before
    // confirming, so nothing can be hit.
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RETURN_KEY, false, false);
}

static boolean throw_miss_step(void) {
    injectKeystroke(THROW_KEY, false, false);
    return false;
}

static boolean thrown_weapon_miss_lands_on_floor(void) {
    TEST_ASSERT(findFloorItem(WEAPON, DAGGER) != NULL,
                "a thrown weapon that misses should not be destroyed, but land on the floor");
    TEST_ASSERT_EQ(packUnitCount(WEAPON, DAGGER), daggersBeforeThrow - 1,
                   "the thrown weapon should leave the pack when thrown");
    return !testFailed;
}

// ============================================================
// BR28082 — each level generates 3 items plus bonus rolls, with
// two extra items on depths 1-2 and one extra on depths 3-4
// ============================================================

static boolean level_item_counts_follow_depth_bands(void) {
    // Nothing may clear or move items before these counts: each level is
    // counted as it was generated, before any turns pass.
    TEST_ASSERT(countFloorItems() >= 5,
                "depth 1 should hold at least 5 items (3 base + 2 for depths 1-2)");

    descendToNextLevel();
    TEST_ASSERT_EQ(currentDepth(), 2, "the player should now be on depth 2");
    TEST_ASSERT(countFloorItems() >= 5,
                "depth 2 should hold at least 5 items (3 base + 2 for depths 1-2)");

    descendToNextLevel();
    TEST_ASSERT(countFloorItems() >= 4,
                "depth 3 should hold at least 4 items (3 base + 1 for depths 3-4)");

    descendToNextLevel();
    TEST_ASSERT(countFloorItems() >= 4,
                "depth 4 should hold at least 4 items (3 base + 1 for depths 3-4)");

    descendToNextLevel();
    TEST_ASSERT(countFloorItems() >= 3,
                "depth 5 should hold at least 3 items (the base count)");
    return !testFailed;
}

// ============================================================
// BR94199 — a wand of beckoning pulls a monster adjacent to you
// ============================================================

static creature *br94199_monster;
static item *br94199_wand;
static pos br94199_playerStart;
static boolean br94199_startAdjacent;

// A clear arena with the rat five tiles east: far enough that the zap has an
// open, straight line to travel and the rat starts out of melee reach.
static void br94199_setup(void) {
    setArenaRadius(6, FLOOR);
    br94199_playerStart = getPlayerPos();
    br94199_monster = placeHostileMonster(MK_RAT, posOffset(br94199_playerStart, 5, 0));
    br94199_startAdjacent = monsterIsAdjacentToPlayer(br94199_monster);
    br94199_wand = givePlayerItem(WAND, WAND_BECKONING, 5);
}

static boolean br94199_step(void) {
    playerZapsAt(br94199_wand, br94199_monster);
    return false;
}

static boolean beckoning_pulls_the_monster_adjacent(void) {
    TEST_ASSERT(br94199_monster != NULL, "the rat should spawn for the beckoning test");
    TEST_ASSERT(!br94199_startAdjacent,
                "the rat should start five tiles away, out of melee reach");
    TEST_ASSERT(monsterIsAlive(br94199_monster),
                "beckoning must not kill the target");
    TEST_ASSERT(monsterIsAdjacentToPlayer(br94199_monster),
                "a wand of beckoning should pull the monster adjacent to you");
    TEST_ASSERT(playerIsAt(br94199_playerStart),
                "beckoning moves the monster, not the player");
    return !testFailed;
}

// ============================================================
// BR43662 — placing an item on a pressure plate depresses it, which
// triggers the trap it controls and reports the click
// ============================================================

static pos br43662_plate;

static void br43662_plate_setup(void) {
    clearArena();
    br43662_plate = posOffset(getPlayerPos(), 1, 0);
    setTerrain(br43662_plate, MACHINE_PRESSURE_PLATE);
    revealCell(br43662_plate);
    TEST_ASSERT(cellIsTrap(br43662_plate), "setup: the tile should hold a pressure plate trap");
    TEST_ASSERT(!isTrapTriggered(br43662_plate), "setup: the plate should start undepressed");
}

static boolean br43662_place_item_step(void) {
    placeItemOnFloor(FOOD, RATION, br43662_plate);
    return false;
}

static boolean br43662_plate_depresses(void) {
    item *placed = findFloorItem(FOOD, RATION);
    TEST_ASSERT(placed != NULL, "control: the ration should now be lying on the level");
    TEST_ASSERT_POS_EQ(itemPosition(placed), br43662_plate,
                        "control: the ration should lie on the plate");
    TEST_ASSERT(isTrapTriggered(br43662_plate),
                "an item placed on a pressure plate should depress it");
    TEST_ASSERT(messageWasShown("a pressure plate clicks underneath"),
                "you should be told that the plate clicked underneath the item");
    return !testFailed;
}

// ============================================================
// BR63185 — A potion of detect magic reveals whether each magical
// item on the level and in your pack is of good or bad magic.
//
// Two pack items (one good, one bad) plus a bad scroll lying on the
// level start with their polarity unknown; drinking the potion must
// mark every one of them as detected and leave each item's good/bad
// polarity readable.
// ============================================================

static item *br63185_packGood;
static item *br63185_packBad;
static item *br63185_levelBad;

static void br63185_detect_magic_setup(void) {
    clearArena();
    br63185_packGood = givePlayerItem(POTION, POTION_LIFE, 0);
    br63185_packBad = givePlayerItem(POTION, POTION_HALLUCINATION, 0);
    br63185_levelBad = placeItemOnFloor(SCROLL, SCROLL_AGGRAVATE_MONSTER,
                                        posOffset(getPlayerPos(), 2, 0));
    TEST_ASSERT(itemMagicPolarityValue(br63185_packGood) > 0,
                "setup: the life potion should be good magic");
    TEST_ASSERT(itemMagicPolarityValue(br63185_packBad) < 0,
                "setup: the hallucination potion should be bad magic");
    TEST_ASSERT(itemMagicPolarityValue(br63185_levelBad) < 0,
                "setup: the aggravate scroll should be bad magic");
}

static boolean br63185_detect_magic_step(void) {
    // Nothing is revealed yet, so the reveal below is the potion's doing.
    TEST_ASSERT(!itemMagicWasDetected(br63185_packGood),
                "a pack item's magic should be unknown before the potion");
    TEST_ASSERT(!itemMagicWasDetected(br63185_packBad),
                "a pack item's magic should be unknown before the potion");
    TEST_ASSERT(!itemMagicWasDetected(br63185_levelBad),
                "a level item's magic should be unknown before the potion");
    if (testFailed) {
        return false;
    }
    drink(POTION_DETECT_MAGIC);
    return false;
}

static boolean br63185_detect_magic_reveals_polarity(void) {
    TEST_ASSERT(itemMagicWasDetected(br63185_packGood),
                "detect magic should reveal the good magic of a pack item");
    TEST_ASSERT(itemMagicWasDetected(br63185_packBad),
                "detect magic should reveal the bad magic of a pack item");
    TEST_ASSERT(itemMagicWasDetected(br63185_levelBad),
                "detect magic should reveal the bad magic of an item on the level");
    TEST_ASSERT(itemMagicPolarityValue(br63185_packGood) > 0,
                "the pack item should read as good magic");
    TEST_ASSERT(itemMagicPolarityValue(br63185_packBad) < 0,
                "the pack item should read as bad magic");
    TEST_ASSERT(itemMagicPolarityValue(br63185_levelBad) < 0,
                "the level item should read as bad magic");
    TEST_ASSERT(messageWasShown("you can somehow feel the presence of magic"),
                "the potion should report feeling magic in the pack and on the level");
    return !testFailed;
}

// ============================================================
// BR59433 — once magic detection has revealed an item's polarity, the
// inventory screen draws a glowing good-magic or bad-magic symbol beside
// it, and a plain marker for neutral magic.
//
// Each test carries one item of the relevant polarity.  Before the
// potion its inventory row shows no magic mark; after drinking a potion
// of detect magic the row must show the mark matching that polarity.
// ============================================================

static item *br59433_item;
static char br59433_letter;
static int br59433_markerBefore;

static void br59433_good_setup(void) {
    clearArena();
    br59433_item = givePlayerItem(POTION, POTION_LIFE, 0);
    TEST_ASSERT(br59433_item != NULL, "setup: the pack should accept the potion");
    TEST_ASSERT(itemMagicPolarityValue(br59433_item) > 0,
                "setup: the potion of life should be good magic");
    TEST_ASSERT(!itemMagicWasDetected(br59433_item),
                "setup: the potion's magic should not be revealed yet");
    br59433_letter = itemInventoryLetter(br59433_item);
}

static void br59433_bad_setup(void) {
    clearArena();
    br59433_item = givePlayerItem(POTION, POTION_HALLUCINATION, 0);
    TEST_ASSERT(br59433_item != NULL, "setup: the pack should accept the potion");
    TEST_ASSERT(itemMagicPolarityValue(br59433_item) < 0,
                "setup: the potion of hallucination should be bad magic");
    TEST_ASSERT(!itemMagicWasDetected(br59433_item),
                "setup: the potion's magic should not be revealed yet");
    br59433_letter = itemInventoryLetter(br59433_item);
}

static void br59433_neutral_setup(void) {
    clearArena();
    // An unenchanted, non-runic weapon has neutral magic.
    br59433_item = givePlayerPlainWeapon(DAGGER, 0);
    TEST_ASSERT(br59433_item != NULL, "setup: the pack should accept the dagger");
    TEST_ASSERT_EQ(itemMagicPolarityValue(br59433_item), 0,
                   "setup: the unenchanted dagger should be neutral magic");
    TEST_ASSERT(!itemMagicWasDetected(br59433_item),
                "setup: the dagger's magic should not be revealed yet");
    br59433_letter = itemInventoryLetter(br59433_item);
}

static boolean br59433_detect_magic_step(void) {
    // The mark seen after the potion must have come from the potion.
    br59433_markerBefore = inventoryScreenMagicMarkerFor(br59433_letter);
    drink(POTION_DETECT_MAGIC);
    return false;
}

static boolean br59433_good_magic_shows_good_marker(void) {
    TEST_ASSERT_EQ(br59433_markerBefore, -2,
                   "no magic mark should be drawn in the inventory before detection");
    TEST_ASSERT(itemMagicWasDetected(br59433_item),
                "detect magic should reveal the potion's polarity");
    TEST_ASSERT_EQ(inventoryScreenMagicMarkerFor(br59433_letter), 1,
                   "a good-magic item should show the glowing good-magic mark");
    return !testFailed;
}

static boolean br59433_bad_magic_shows_bad_marker(void) {
    TEST_ASSERT_EQ(br59433_markerBefore, -2,
                   "no magic mark should be drawn in the inventory before detection");
    TEST_ASSERT(itemMagicWasDetected(br59433_item),
                "detect magic should reveal the potion's polarity");
    TEST_ASSERT_EQ(inventoryScreenMagicMarkerFor(br59433_letter), -1,
                   "a bad-magic item should show the glowing bad-magic mark");
    return !testFailed;
}

static boolean br59433_neutral_magic_shows_plain_marker(void) {
    TEST_ASSERT_EQ(br59433_markerBefore, -2,
                   "no magic mark should be drawn in the inventory before detection");
    TEST_ASSERT(itemMagicWasDetected(br59433_item),
                "detect magic should reveal the dagger's polarity");
    TEST_ASSERT_EQ(inventoryScreenMagicMarkerFor(br59433_letter), 0,
                   "a neutral-magic item should show the plain neutral marker");
    return !testFailed;
}

// ============================================================
// BR90259 — a staff with no charges cannot be fired: you are told
// it is out of charges for now, and its maximum charges become known
// ============================================================

static item *br90259_staff;
static int br90259_drainTurns;

static void br90259_setup(void) {
    clearArena();
    // A found staff enters the pack unidentified; stepping onto it picks
    // it up (BR75019).
    placeItemOnFloor(STAFF, STAFF_LIGHTNING, posOffset(getPlayerPos(), 1, 0));
    moveRight();
    br90259_staff = findInventoryItem(STAFF, STAFF_LIGHTNING);
    TEST_ASSERT(br90259_staff != NULL, "setup: the staff should be picked up into the pack");
    TEST_ASSERT(!itemIsIdentified(br90259_staff),
                "setup: a found staff should not be identified yet");
    TEST_ASSERT(itemCharges(br90259_staff) > 0, "setup: the staff should start with charges");

    // Fire the staff east until it is empty: a direction key moves the
    // targeting cursor and return confirms it.
    br90259_drainTurns = 0;
    while (itemCharges(br90259_staff) > 0 && br90259_drainTurns < 20) {
        queueKeystroke(RIGHT_KEY, false, false);
        queueKeystroke(RETURN_KEY, false, false);
        applyPackItem(STAFF, STAFF_LIGHTNING);
        br90259_drainTurns++;
    }
    TEST_ASSERT_EQ(itemCharges(br90259_staff), 0,
                   "setup: zapping should drain the staff to zero charges");
    clearMessages(); // only the empty-staff attempt should be judged
}

static boolean br90259_step(void) {
    queueKeystroke(RIGHT_KEY, false, false);
    queueKeystroke(RETURN_KEY, false, false);
    applyPackItem(STAFF, STAFF_LIGHTNING);      // the empty staff fizzles instead of firing
    injectKeystroke(INVENTORY_KEY, false, false); // draw the pack so its charge display can be read
    return false;
}

static boolean br90259_empty_staff_cannot_fire(void) {
    TEST_ASSERT(messageWasShown("out of charges for now"),
                "an empty staff should report that it is out of charges for now");
    TEST_ASSERT(!messageWasShown("you zap your"),
                "an empty staff should not fire");
    TEST_ASSERT(screenTextContains("[?/"),
                "using the empty staff should make its maximum charges known ([?/max] in the pack)");
    return !testFailed;
}

// ============================================================
// BR71867 — food, charms, gold, the Amulet of Yendor, lumenstones, and
// keys are known to you from the moment they appear
// ============================================================

static item *bornFood, *bornCharm, *bornGold, *bornAmulet, *bornGem, *bornKey, *bornPotion;

static void born_known_setup(void) {
    clearArena();
    pos p = getPlayerPos();
    bornFood   = placeItemOnFloor(FOOD, RATION,        posOffset(p, -1, -1));
    bornCharm  = placeItemOnFloor(CHARM, CHARM_HEALTH, posOffset(p,  0, -1));
    bornGold   = placeItemOnFloor(GOLD, 0,             posOffset(p,  1, -1));
    bornAmulet = placeItemOnFloor(AMULET, 0,           posOffset(p, -1,  0));
    bornGem    = placeItemOnFloor(GEM, 0,              posOffset(p,  1,  0));
    bornKey    = placeItemOnFloor(KEY, 0,              posOffset(p,  0,  1));
    bornPotion = placeItemOnFloor(POTION, POTION_LIFE, posOffset(p,  1,  1));
}

static boolean known_the_moment_they_appear(void) {
    // Each listed item carries its identity from the moment of generation.
    TEST_ASSERT(itemIsIdentified(bornFood),
                "food should be born known");
    TEST_ASSERT(itemIsIdentified(bornCharm),
                "a charm should be born known");
    TEST_ASSERT(itemIsIdentified(bornAmulet),
                "the Amulet of Yendor should be born known");
    TEST_ASSERT(itemIsIdentified(bornGem),
                "a lumenstone should be born known");
    TEST_ASSERT(itemIsIdentified(bornKey),
                "a key should be born known");

    // Their kinds are known without ever having to be learned.
    TEST_ASSERT(itemKindIsKnown(FOOD, RATION),
                "a ration of food should be a known kind");
    TEST_ASSERT(itemKindIsKnown(CHARM, CHARM_HEALTH),
                "a charm should be a known kind");
    TEST_ASSERT(itemKindIsKnown(GOLD, 0),
                "gold should be a known kind");
    TEST_ASSERT(itemKindIsKnown(AMULET, 0),
                "the Amulet of Yendor should be a known kind");
    TEST_ASSERT(itemKindIsKnown(GEM, 0),
                "a lumenstone should be a known kind");
    TEST_ASSERT(itemKindIsKnown(KEY, 0),
                "a key should be a known kind");

    // An ordinary item outside the list is not known the same way, which is
    // what keeps the assertions above from passing vacuously.
    TEST_ASSERT(!itemIsIdentified(bornPotion),
                "a potion should not be born known");
    TEST_ASSERT(!itemKindIsKnown(POTION, POTION_LIFE),
                "a potion should not be a known kind at first");
    return !testFailed;
}

// ============================================================

// ============================================================
// BR90956 — when you aim a throw, the highlighted trajectory stops
// at the target you selected rather than continuing past it
// ============================================================

static void throw_aim_setup(void) {
    clearArena();
    // An open, revealed corridor east of you: the aim line runs along it, and
    // every cell it could pass through is discovered, so the highlight is not
    // cut short by unseen terrain (BR96156).
    for (int i = 1; i <= 9; i++) {
        pos p = posOffset(getPlayerPos(), i, 0);
        setTerrain(p, FLOOR);
        revealCell(p);
    }
    item *dagger = givePlayerItem(WEAPON, DAGGER, 0);
    queueKeystroke(itemInventoryLetter(dagger), false, false); // choose the thrown item
    for (int i = 0; i < 5; i++) {
        queueKeystroke(RIGHT_KEY, false, false); // move the cursor five cells east
    }
}

static boolean throw_aim_step(void) {
    // The throw runs the whole aim from the queue; the last frame captured
    // before the queue empties has the trajectory drawn to the cursor, five
    // cells east.  Remember that frame, then ask for one more input: the
    // request it makes captures the frame with the aim closed and the
    // trajectory erased again.
    injectKeystroke(THROW_KEY, false, false);
    rememberScreenColors();
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean throw_aim_trajectory_stops_at_target(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 2, 0)),
                "aiming a throw should highlight the trajectory through the cells on the way to the target");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 7, 0)),
                "the highlighted trajectory should stop at the selected target, not continue past it");
    return !testFailed;
}

// ============================================================
// BR61428 — A wand of domination may bind a monster to your
// will, or the monster resists.
//
// The bind roll is the missing fraction of the target's
// health: a monster at full health offers 0% (an always-resist)
// and once below a fifth of its maximum health the roll is 100%
// (an always-bind), so one target zapped twice walks both arms
// of the rule deterministically.  The zaps are fired directly
// at the target and no player turn runs between them, so the
// target never moves, acts, or regains health in between.
// ============================================================

static item *br61428_wand;
static creature *br61428_target;

static void br61428_setup(void) {
    setArenaRadius(6, FLOOR);
    br61428_target = placeHostileMonster(MK_WILL_O_THE_WISP, posOffset(getPlayerPos(), 5, 0));
    TEST_ASSERT(br61428_target != NULL, "setup: the domination target should spawn");
    // The arena is carved after the level's vision pass, so grant telepathy:
    // the wand's bind/resist report is only spoken about a target the player
    // knows, and telepathy reveals every living creature's location.
    applyStatusToPlayer(STATUS_TELEPATHIC, 1000);
    TEST_ASSERT(monsterMaxHealth(br61428_target) > 5,
                "setup: the target needs more than 5 maximum health, so 1 health is below a fifth of it");
    TEST_ASSERT(monsterIsHostile(br61428_target), "setup: the target should start hostile");
    br61428_wand = givePlayerItem(WAND, WAND_DOMINATION, 5);
}

static boolean br61428_step(void) {
    TEST_ASSERT(!monsterIsAlly(br61428_target),
                "the target should start as your enemy, not your ally");
    playerZapsAt(br61428_wand, br61428_target);
    TEST_ASSERT(!monsterIsAlly(br61428_target),
                "a monster at full health has a 0% bind chance and should resist the bolt");
    setMonsterHealth(br61428_target, 1);
    playerZapsAt(br61428_wand, br61428_target);
    return false;
}

static boolean br61428_domination_binds_or_resists(void) {
    TEST_ASSERT(messageWasShown("resists the bolt of domination"),
                "the wand should report that the monster resisted domination");
    TEST_ASSERT(messageWasShown("is bound to your will"),
                "the wand should report that the wounded monster was bound to your will");
    TEST_ASSERT(monsterIsAlly(br61428_target),
                "a monster zapped below a fifth of its health should become your ally");
    return !testFailed;
}

// ============================================================
// BR69013 — a wand of domination's chance of succeeding
// depends on the target's current health.
//
// The bind chance is the missing fraction of the target's health:
// a target at three-quarters health is wounded only a little and
// should usually resist, while one at half health should be bound
// about half the time.  Each trial resets the same monster to the
// chosen health and zaps it directly, with no turn in between, so
// health is the only thing that differs between the two runs.
// Enough trials are run that a chance which ignores health, or
// which treats the two health levels alike, cannot pass by luck.
// ============================================================

#define BR69013_TRIALS 160

static item *br69013_wand;
static creature *br69013_target;
static int br69013_maxHealth;

static void br69013_setup(void) {
    setArenaRadius(6, FLOOR);
    br69013_target = placeHostileMonster(MK_OGRE, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(br69013_target != NULL, "setup: the domination target should spawn");
    br69013_maxHealth = monsterMaxHealth(br69013_target);
    TEST_ASSERT(br69013_maxHealth >= 20,
                "setup: the target needs enough health to sit at distinct quarters");
    br69013_wand = givePlayerItem(WAND, WAND_DOMINATION, 5);
}

static int br69013_bindsAtHealth(int health) {
    int binds = 0;
    for (int trial = 0; trial < BR69013_TRIALS; trial++) {
        makeMonsterHostile(br69013_target);
        setMonsterHealth(br69013_target, health);
        playerZapsAt(br69013_wand, br69013_target);
        if (monsterIsAlly(br69013_target)) {
            binds++;
        }
    }
    return binds;
}

static boolean br69013_chance_depends_on_current_health(void) {
    const int bindsAtThreeQuarters = br69013_bindsAtHealth(br69013_maxHealth * 3 / 4);
    const int bindsAtHalf = br69013_bindsAtHealth(br69013_maxHealth / 2);

    TEST_ASSERT(bindsAtThreeQuarters > 0 && bindsAtThreeQuarters < BR69013_TRIALS * 2 / 5,
                "a target at three-quarters health should usually resist (only about a quarter of the zaps should bind)");
    TEST_ASSERT(bindsAtHalf > BR69013_TRIALS / 3 && bindsAtHalf < BR69013_TRIALS * 2 / 3,
                "a target at half health should be bound about half the time");
    TEST_ASSERT(bindsAtHalf > bindsAtThreeQuarters,
                "a target at half health should be bound more often than one at three-quarters health");
    return !testFailed;
}

// ============================================================
// BR84582 — Bolts can be reflected by reflective creatures (or
// by reflective terrain, or by a ring or armor of reflection);
// a reflected bolt takes a new path and can strike its own
// caster.
//
// The guardian reflects every bolt that reaches it, with no
// roll, so zapping it with a staff of haste must leave the
// guardian untouched and send the haste bolt back onto the
// caster instead.  The bolt's own path starts one tile beyond
// the caster, so the player can only be hastened by a bolt that
// was reflected off the guardian and travelled back; if the
// reflection were broken the bolt would apply its haste to the
// guardian and the player would be left alone.  Both halves of
// the assertion catch that.
// ============================================================

static creature *br84582_guardian;

static void br84582_setup(void) {
    setArenaRadius(8, FLOOR);
    br84582_guardian = placeHostileMonster(MK_GUARDIAN, posOffset(getPlayerPos(), 1, 0));
    item *staff = givePlayerItem(STAFF, STAFF_HASTE, 0);
    playerZapsAt(staff, br84582_guardian);
}

static boolean br84582_verify(void) {
    TEST_ASSERT(playerHasStatus(STATUS_HASTED),
                "a bolt reflected by a reflective creature should strike its own caster");
    TEST_ASSERT(messageWasShown("you feel yourself speed up"),
                "the reflected bolt should arrive as a haste spell on the caster");
    TEST_ASSERT(!monsterHasStatus(br84582_guardian, STATUS_HASTED),
                "the reflective creature should deflect the bolt, not be affected by it");
    return !testFailed;
}

// ============================================================
// BR76873 — After identifying itself, a scroll of identify lets
// you identify one other item in your pack.
//
// The target is a freshly donned suit of runic armor that has
// not been identified (the setup asserts as much), and the
// queued keystroke answers the scroll's "Identify what?"
// prompt with the armor's inventory letter.
// ============================================================

static item *br76873_target;

static void br76873_setup(void) {
    clearArena();
    br76873_target = givePlayerRunicArmor(CHAIN_MAIL, A_DAMPENING, 1, false);
    TEST_ASSERT(!itemIsIdentified(br76873_target),
                "setup: the armor must start unidentified");
    queueKeystroke(itemInventoryLetter(br76873_target), false, false);
}

static boolean br76873_step(void) {
    readScrollOfKind(SCROLL_IDENTIFY);
    return false;
}

static boolean br76873_identifies_one_other_item(void) {
    TEST_ASSERT(messageWasShown("this is a scroll of identify."),
                "the scroll of identify should identify itself first");
    TEST_ASSERT(itemIsIdentified(br76873_target),
                "a scroll of identify should identify one other item in your pack");
    return !testFailed;
}

// ============================================================
// BR58888 — Throwing weapons are never cursed, runic, or enchanted.
//
// Thirty of them are generated through the game's own item generation
// (ten each of darts, incendiary darts and javelins), which rolls the
// usual weapon enchantment, curse and runic chances before stripping
// them from throwing weapons.
// ============================================================

static void br58888_setup(void) {
    setArenaRadius(4, FLOOR);
}

static boolean throwing_weapons_are_never_magical(void) {
    static const short kinds[] = { DART, INCENDIARY_DART, JAVELIN };
    pos origin = getPlayerPos();
    int generated = 0;

    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < 10; i++) {
            pos loc = posOffset(origin, (i % 5) - 2, (i / 5) - 1);
            item *theItem = placeItemOnFloor(WEAPON, kinds[k], loc);
            TEST_ASSERT(theItem != NULL, "setup: a throwing weapon should be generated");
            if (theItem == NULL) {
                continue;
            }
            generated++;

            // The stack size is chosen by the very branch of item
            // generation that strips the magic, so seeing a throwing
            // weapon's characteristic stack proves that branch ran.
            int quantity = itemQuantity(theItem);
            if (kinds[k] == INCENDIARY_DART) {
                TEST_ASSERT(quantity >= 3 && quantity <= 6,
                            "setup: incendiary darts come in stacks of 3-6 "
                            "(the throwing-weapon branch of generation should have run)");
            } else {
                TEST_ASSERT(quantity >= 5 && quantity <= 18,
                            "setup: darts and javelins come in stacks of 5-18 "
                            "(the throwing-weapon branch of generation should have run)");
            }

            TEST_ASSERT(itemEnchant(theItem) == 0,
                        "a generated throwing weapon should not be enchanted");
            TEST_ASSERT(!itemIsCursed(theItem),
                        "a generated throwing weapon should not be cursed");
            TEST_ASSERT(!itemIsRunic(theItem),
                        "a generated throwing weapon should not be runic");
        }
    }
    TEST_ASSERT_EQ(generated, 30,
                   "setup: ten of each throwing weapon should be generated");
    return !testFailed;
}

// ============================================================
// BR69624 — when a runic weapon or armor's enchantment drops to
// zero or less, its runes fade and it ceases to be runic.
//
// Attacking an acid mound corrodes the wielded weapon by one (a
// sleeping defender is always hit); being struck by one corrodes
// worn armor by one (a paralyzed defender is always hit).
// Each corrosion runs the disenchantment check: starting from
// +1, the corroding point lands on zero and the runes fade with
// the "the runes fade" announcement; starting from +2, it stops
// at +1 and the gear stays runic — the negative test that pins
// the fade to zero or below.
// ============================================================

static item *br69624_weapon;
static boolean br69624_weaponWasRunic;
static int br69624_weaponEnchantWas;
static boolean br69624_weaponHitLanded;

static void br69624_weapon_setup(void) {
    setArenaRadius(4, FLOOR);
    br69624_weapon = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 1, true);
    TEST_ASSERT(br69624_weapon != NULL, "setup: the runic dagger should be created");
    if (br69624_weapon == NULL) {
        return;
    }
    br69624_weaponEnchantWas = itemEnchant(br69624_weapon);
    TEST_ASSERT_EQ(br69624_weaponEnchantWas, 1,
                   "setup: the runic dagger should start at +1");
    br69624_weaponWasRunic = itemIsRunic(br69624_weapon);
    TEST_ASSERT(br69624_weaponWasRunic, "setup: the dagger should start runic");

    creature *mound = placeHostileMonster(MK_ACID_MOUND, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(mound != NULL, "setup: an acid mound should spawn");
    if (mound != NULL) {
        putMonsterToSleep(mound); // a sleeping defender is always hit
    }
}

static boolean br69624_weapon_attacks_step(void) {
    creature *mound = findMonster(MK_ACID_MOUND);
    TEST_ASSERT(mound != NULL, "step: the acid mound should be present");
    if (mound == NULL) {
        return false;
    }
    br69624_weaponHitLanded = playerAttacksMonster(mound);
    return false;
}

static boolean br69624_weapon_runes_fade_at_zero(void) {
    TEST_ASSERT(br69624_weaponWasRunic,
                "precondition: the dagger was runic before the corroding blow");
    TEST_ASSERT_EQ(br69624_weaponEnchantWas, 1,
                   "precondition: the dagger's enchantment started at +1");
    TEST_ASSERT(br69624_weaponHitLanded,
                "control: a sleeping defender is always hit");
    TEST_ASSERT_EQ(itemEnchant(br69624_weapon), 0,
                   "the corroding blow should drop the dagger's enchantment to zero");
    TEST_ASSERT(!itemIsRunic(br69624_weapon),
                "a runic weapon whose enchantment has dropped to zero should cease to be runic");
    TEST_ASSERT(messageWasShown("runes fade"),
                "the fading of the runes should be announced");
    return !testFailed;
}

static void br69624_above_setup(void) {
    setArenaRadius(4, FLOOR);
    br69624_weapon = giveAndEquipRunicWeapon(DAGGER, W_SPEED, 2, true);
    TEST_ASSERT(br69624_weapon != NULL, "setup: the runic dagger should be created");
    if (br69624_weapon == NULL) {
        return;
    }
    br69624_weaponEnchantWas = itemEnchant(br69624_weapon);
    TEST_ASSERT_EQ(br69624_weaponEnchantWas, 2,
                   "setup: the runic dagger should start at +2");
    br69624_weaponWasRunic = itemIsRunic(br69624_weapon);
    TEST_ASSERT(br69624_weaponWasRunic, "setup: the dagger should start runic");

    creature *mound = placeHostileMonster(MK_ACID_MOUND, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(mound != NULL, "setup: an acid mound should spawn");
    if (mound != NULL) {
        putMonsterToSleep(mound);
    }
}

static boolean br69624_above_zero_keeps_runes(void) {
    TEST_ASSERT(br69624_weaponWasRunic,
                "precondition: the dagger was runic before the corroding blow");
    TEST_ASSERT_EQ(br69624_weaponEnchantWas, 2,
                   "precondition: the dagger's enchantment started at +2");
    TEST_ASSERT(br69624_weaponHitLanded,
                "control: a sleeping defender is always hit");
    TEST_ASSERT_EQ(itemEnchant(br69624_weapon), 1,
                   "the corroding blow should stop one above zero");
    TEST_ASSERT(itemIsRunic(br69624_weapon),
                "a runic weapon above zero enchantment should keep its runes");
    TEST_ASSERT(!messageWasShown("runes fade"),
                "the runes should not fade while the enchantment stays above zero");
    return !testFailed;
}

static item *br69624_armor;
static boolean br69624_armorWasRunic;
static int br69624_armorEnchantWas;
static boolean br69624_armorHitLanded;

static void br69624_armor_setup(void) {
    setArenaRadius(4, FLOOR);
    setPlayerHealth(playerMaxHealth());
    applyStatusToPlayer(STATUS_PARALYZED, 100); // a paralyzed defender is always hit
    br69624_armor = givePlayerRunicArmor(LEATHER_ARMOR, A_DAMPENING, 1, true);
    TEST_ASSERT(br69624_armor != NULL, "setup: the runic armor should be created");
    if (br69624_armor == NULL) {
        return;
    }
    TEST_ASSERT(equippedArmor() == br69624_armor,
                "setup: the runic armor should be the worn armor");
    br69624_armorEnchantWas = itemEnchant(br69624_armor);
    TEST_ASSERT_EQ(br69624_armorEnchantWas, 1,
                   "setup: the runic armor should start at +1");
    br69624_armorWasRunic = itemIsRunic(br69624_armor);
    TEST_ASSERT(br69624_armorWasRunic, "setup: the armor should start runic");

    creature *mound = placeHostileMonster(MK_ACID_MOUND, posOffset(getPlayerPos(), 1, 0));
    TEST_ASSERT(mound != NULL, "setup: an acid mound should spawn");
}

static boolean br69624_armor_struck_step(void) {
    creature *mound = findMonster(MK_ACID_MOUND);
    TEST_ASSERT(mound != NULL, "step: the acid mound should be present");
    if (mound == NULL) {
        return false;
    }
    br69624_armorHitLanded = monsterAttacksPlayer(mound);
    return false;
}

static boolean br69624_armor_runes_fade_at_zero(void) {
    TEST_ASSERT(br69624_armorWasRunic,
                "precondition: the armor was runic before the corroding blow");
    TEST_ASSERT_EQ(br69624_armorEnchantWas, 1,
                   "precondition: the armor's enchantment started at +1");
    TEST_ASSERT(br69624_armorHitLanded,
                "control: a paralyzed defender is always hit");
    TEST_ASSERT_EQ(itemEnchant(br69624_armor), 0,
                   "the corroding blow should drop the armor's enchantment to zero");
    TEST_ASSERT(!itemIsRunic(br69624_armor),
                "a runic armor whose enchantment has dropped to zero should cease to be runic");
    TEST_ASSERT(messageWasShown("runes fade"),
                "the fading of the runes should be announced");
    return !testFailed;
}

// ============================================================
// BR75263 — auto-targeting only picks monsters you can see with
// an unobstructed path, avoids allies for harmful effects, and
// skips creatures that are immune to the effect (such as
// fire-immune targets for flame).
//
// Opening a zap of the staff of flame auto-selects a target from
// the sidebar's visible creatures.  Four monsters ring the
// player: a plain hostile enemy to the north (the only valid
// one), a hostile enemy to the east carrying the fire-immune
// status (skipped for flame), an ally to the west (a harmful
// effect never picks allies), and a hostile enemy to the south
// behind a granite cell (no unobstructed path).  The aim frame's
// trajectory highlight — captured at the prompt's first input
// request — must run north toward the one valid target, and must
// not light a single cell on the way to any skipped candidate; a
// later frame, taken after the aim closes, shows the highlight
// erased again.  Every cell a wrongly-drawn trajectory would
// cross is revealed in setup, so a broken skip would actually
// light it up in the captured frame.
// ============================================================

static void br75263_setup(void) {
    setArenaRadius(6, FLOOR);

    // The one valid target: a plain hostile enemy to the north.
    creature *valid = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 0, -3));
    TEST_ASSERT(valid != NULL, "setup: the plain enemy should spawn");

    // Skipped for flame's fire: a hostile enemy carrying fire immunity.
    creature *immune = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 3, 0));
    TEST_ASSERT(immune != NULL, "setup: the fire-immune enemy should spawn");
    if (immune != NULL) {
        applyStatusToMonster(immune, STATUS_IMMUNE_TO_FIRE, 10000);
    }

    // Skipped as an ally: flame only targets enemies.
    creature *ally = spawnMonster(MK_RAT, posOffset(getPlayerPos(), -3, 0));
    TEST_ASSERT(ally != NULL, "setup: the ally should spawn");
    if (ally != NULL) {
        makeMonsterAlly(ally);
        revealCell(monsterPosition(ally));
    }

    // Skipped for want of an unobstructed path: a hostile enemy behind granite.
    setTerrain(posOffset(getPlayerPos(), 0, 2), GRANITE);
    creature *walled = placeHostileMonster(MK_RAT, posOffset(getPlayerPos(), 0, 3));
    TEST_ASSERT(walled != NULL, "setup: the walled-off enemy should spawn");

    // Reveal every cell a trajectory toward a skipped candidate would cross,
    // so a broken skip shows up as a highlight in the captured frame.
    revealCell(posOffset(getPlayerPos(), 0, -1)); // toward the valid target
    revealCell(posOffset(getPlayerPos(), 0, -2));
    revealCell(posOffset(getPlayerPos(), 1, 0));  // toward the immune enemy
    revealCell(posOffset(getPlayerPos(), 2, 0));
    revealCell(posOffset(getPlayerPos(), -1, 0)); // toward the ally
    revealCell(posOffset(getPlayerPos(), -2, 0));
    revealCell(posOffset(getPlayerPos(), 0, 1));  // toward the walled enemy

    // The staff's kind must be known for the immunity and ally gates to run.
    item *staff = givePlayerItem(STAFF, STAFF_FIRE, 0);
    TEST_ASSERT(staff != NULL, "setup: the staff of flame should be created");
    TEST_ASSERT(itemKindIsKnown(STAFF, STAFF_FIRE),
                "setup: the staff's kind should be known");

    // Populate the sidebar's entity list, which auto-targeting reads.
    TEST_ASSERT(sidebarContains("Rat"),
                "setup: the sidebar should list a visible monster before auto-targeting runs");
}

static boolean br75263_zap_aim_step(void) {
    applyPackItem(STAFF, STAFF_FIRE);
    // The auto-selected target's trajectory is drawn before the prompt's
    // first input request, whose captured frame applyPackItem leaves as the
    // most recent snapshot...
    rememberScreenColors();
    // ... and one more input pass captures a frame with the aim closed and
    // the trajectory erased again.
    runQueuedInputThroughMainLoop();
    return false;
}

static boolean br75263_auto_target_picks_only_a_valid_target(void) {
    TEST_ASSERT(screenCellColorsChanged(posOffset(getPlayerPos(), 0, -1)),
                "auto-targeting should aim at the one valid enemy, north of you");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 1, 0)),
                "auto-targeting should skip a fire-immune enemy when aiming flame");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), -1, 0)),
                "auto-targeting should avoid allies for a harmful effect");
    TEST_ASSERT(!screenCellColorsChanged(posOffset(getPlayerPos(), 0, 1)),
                "auto-targeting should skip a monster without an unobstructed path");
    return !testFailed;
}

// ============================================================
// BR44912 — a scroll of teleportation teleports you to
// another part of the level.
//
// The destination is a random legal square you cannot
// currently see and that lies at least two squares from where
// you stand, so reading the scroll has to change your position
// while the depth you are on stays the same.
// ============================================================

static pos br44912_start;
static int br44912_depth;

static void br44912_setup(void) {
    br44912_start = getPlayerPos();
    br44912_depth = currentDepth();
}

static boolean br44912_step(void) {
    readScrollOfKind(SCROLL_TELEPORT);
    return false;
}

static boolean br44912_teleports_to_another_part_of_the_level(void) {
    TEST_ASSERT(!playerIsAt(br44912_start),
                "the scroll should teleport the player to another square of the level");
    TEST_ASSERT_EQ(currentDepth(), br44912_depth,
                   "the scroll should teleport you within the level, not off it");
    return !testFailed;
}

// ============================================================
// BR18415 — you can wear at most two rings; equipping a third asks
// you to choose which worn ring to remove first, and a ring you are
// already wearing cannot be equipped again
// ============================================================

static item *br18415_first;
static item *br18415_second;
static item *br18415_third;

// Wear two rings through the direct equip helper, leaving a third in the pack.
static void br18415_wear_two_rings(void) {
    clearArena();
    br18415_first = givePlayerItem(RING, RING_STEALTH, 2);
    uncurseItem(br18415_first);
    equipItemFromPack(RING, RING_STEALTH);
    br18415_second = givePlayerItem(RING, RING_WISDOM, 2);
    uncurseItem(br18415_second);
    equipItemFromPack(RING, RING_WISDOM);
    br18415_third = givePlayerItem(RING, RING_AWARENESS, 2);
    uncurseItem(br18415_third);
}

static void br18415_swap_setup(void) {
    br18415_wear_two_rings();
    // The answer to "remove which first?" is the first worn ring.
    queueKeystroke(itemInventoryLetter(br18415_first), false, false);
    equipItemFromPackAsPlayer(RING, RING_AWARENESS);
}

static boolean br18415_third_ring_replaces_the_chosen_one(void) {
    TEST_ASSERT_EQ(equippedRingCount(), 2,
                   "only two rings should be worn after the swap");
    TEST_ASSERT(itemIsEquipped(br18415_third),
                "answering the choice prompt should put the third ring on");
    TEST_ASSERT(!itemIsEquipped(br18415_first),
                "the worn ring named in the prompt should come off");
    TEST_ASSERT(itemIsEquipped(br18415_second),
                "the worn ring not named in the prompt should stay on");
    return !testFailed;
}

static void br18415_cancel_setup(void) {
    br18415_wear_two_rings();
    equipItemFromPackAsPlayer(RING, RING_AWARENESS);
}

static boolean br18415_third_ring_waits_for_a_choice(void) {
    TEST_ASSERT_EQ(equippedRingCount(), 2,
                   "a third ring must not be worn while two are already on");
    TEST_ASSERT(!itemIsEquipped(br18415_third),
                "with the choice prompt cancelled the third ring stays in the pack");
    TEST_ASSERT(itemIsEquipped(br18415_first) && itemIsEquipped(br18415_second),
                "cancelling the choice should leave both worn rings on");
    return !testFailed;
}

static void br18415_repeat_setup(void) {
    clearArena();
    br18415_first = givePlayerItem(RING, RING_STEALTH, 2);
    equipItemFromPack(RING, RING_STEALTH);
    equipItemFromPackAsPlayer(RING, RING_STEALTH);
}

static boolean br18415_worn_ring_cannot_be_worn_again(void) {
    TEST_ASSERT(itemIsEquipped(br18415_first),
                "the ring should still be worn after trying to equip it again");
    TEST_ASSERT_EQ(equippedRingCount(), 1,
                   "a worn ring must not occupy a second finger");
    TEST_ASSERT(messageWasShown("already wearing that ring"),
                "the game should say the ring is already being worn");
    return !testFailed;
}

// ============================================================
// BR47394 — ordinary items are placed away from hallways and
// are biased toward regions reached only through doors and
// secret doors, so secret rooms tend to collect several items
// ============================================================
//
// The level is rebuilt as two identical rooms joined by a one-tile
// corridor, with a secret door in the middle of the corridor.  Running
// the real item population on this map many times gives ordinary items
// only two valid kinds of home: the open room with the up staircase, or
// the sealed room beyond the secret door.  Corridor tiles are hallways,
// so no item may ever land on one, and the weighting that favors the
// secret door should send the large majority of items into the sealed
// room rather than splitting them evenly.
//
// The trials make this a distribution check rather than a single draw:
// the game reseeds its random numbers after start-up, so a lone trial
// cannot assert an exact position.

#define BR47394_TRIALS 100

static void br47394_setup(void) {
    for (int x = 0; x < levelWidth(); x++) {
        for (int y = 0; y < levelHeight(); y++) {
            setTerrain(posAt(x, y), GRANITE);
        }
    }

    // Two equal rooms (8 x 11) joined by a straight corridor.
    for (int x = 5; x <= 12; x++) {
        for (int y = 9; y <= 19; y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    for (int x = 66; x <= 73; x++) {
        for (int y = 9; y <= 19; y++) {
            setTerrain(posAt(x, y), FLOOR);
        }
    }
    for (int x = 13; x <= 65; x++) {
        setTerrain(posAt(x, 14), FLOOR);
    }
    setTerrain(posAt(39, 14), SECRET_DOOR);

    setPlayerPos(posAt(8, 14));
}

static boolean br47394_verify(void) {
    int staircaseRoomItems = 0;
    int secretRoomItems = 0;
    int otherItems = 0;

    for (int trial = 0; trial < BR47394_TRIALS; trial++) {
        clearFloorItems();
        populateLevelItemsFrom(posAt(8, 14));

        int itemCount = countFloorItems();
        TEST_ASSERT(itemCount > 0,
                    "item population should place ordinary items on the prepared level");

        for (int i = 0; i < itemCount; i++) {
            pos p = itemPosition(floorItemAtIndex(i));

            TEST_ASSERT(!cellIsHallway(p),
                        "an ordinary item was placed in a hallway");

            if (p.x >= 66 && p.x <= 73 && p.y >= 9 && p.y <= 19) {
                secretRoomItems++;
            } else if (p.x >= 5 && p.x <= 12 && p.y >= 9 && p.y <= 19) {
                staircaseRoomItems++;
            } else {
                otherItems++; // corridor or solid rock
            }
        }
    }

    TEST_ASSERT_EQ(otherItems, 0,
                   "every ordinary item should land in one of the two rooms");
    TEST_ASSERT(staircaseRoomItems > 0,
                "the room holding the up staircase should still receive some items");
    TEST_ASSERT(secretRoomItems > 4 * staircaseRoomItems,
                "the secret room should receive the large majority of ordinary items");

    return !testFailed;
}

// ============================================================
// BR54814 — invoking an item command without naming an item
// opens the inventory so you can pick one; if nothing suitable
// is available you are told "You have nothing suitable."
// ============================================================

static char br54814_letter;

static void br54814_available_setup(void) {
    clearArena();
    removePackFood(); // the starting ration is applicable; keep only the potion
    item *potion = givePlayerItem(POTION, POTION_LIFE, 0);
    TEST_ASSERT(potion != NULL, "setup: the pack should accept a potion");
    br54814_letter = itemInventoryLetter(potion);
    queueKeystroke(br54814_letter, false, false);
    injectKeystroke(APPLY_KEY, false, false);
}

static boolean br54814_available_verify(void) {
    TEST_ASSERT(messageWasShown("Apply what?"),
                "applying without naming an item should ask which item to use");
    TEST_ASSERT(screenTextContains("potion of life"),
                "the apply command should open the inventory with the potion listed");
    TEST_ASSERT_EQ(packUnitCount(POTION, POTION_LIFE), 0,
                   "the potion chosen from the inventory should be applied");
    return !testFailed;
}

static void br54814_nothing_setup(void) {
    clearArena();
    // Every remaining item is equipped, and an equip command forbids equipped
    // items, so there is nothing suitable to choose.
    removePackFood();
    removePackItems(WEAPON, DART);
    injectKeystroke(EQUIP_KEY, false, false);
}

static boolean br54814_nothing_verify(void) {
    TEST_ASSERT(messageWasShown("You have nothing suitable."),
                "an item command with no suitable item should say there is nothing suitable");
    return !testFailed;
}

const testCase tests_items[] = {
    { "BR58888_throwing_weapons_never_magical",     br58888_setup,             NULL, throwing_weapons_are_never_magical },

    { "BR54814_apply_opens_inventory_to_choose_item", br54814_available_setup, NULL, br54814_available_verify },
    { "BR54814_apply_with_no_suitable_item_says_so",  br54814_nothing_setup,   NULL, br54814_nothing_verify },

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
    { "BR63185_detect_magic_reveals_polarity",      br63185_detect_magic_setup, br63185_detect_magic_step, br63185_detect_magic_reveals_polarity },
    { "BR59433_good_magic_shows_good_marker",       br59433_good_setup,    br59433_detect_magic_step, br59433_good_magic_shows_good_marker },
    { "BR59433_bad_magic_shows_bad_marker",         br59433_bad_setup,     br59433_detect_magic_step, br59433_bad_magic_shows_bad_marker },
    { "BR59433_neutral_magic_shows_plain_marker",   br59433_neutral_setup, br59433_detect_magic_step, br59433_neutral_magic_shows_plain_marker },

    { "BR29492_scroll_of_aggravate_wakes_monsters", aggravate_scroll_setup,     NULL, aggravate_scroll_wakes_monsters },
    { "BR95938_reading_scroll_consumes_it",         consume_scroll_setup,       NULL, reading_scroll_consumes_it },
    { "BR47462_enchanting_removes_curse",           enchanting_removes_curse_setup, enchanting_removes_curse_step, curse_removed_by_enchanting },
    { "BR76873_identify_scroll_identifies_one_other_item", br76873_setup, br76873_step, br76873_identifies_one_other_item },

    { "BR46995_enchant_to_16_earns_specialist",     specialist_earned_setup,    specialist_earned_step, specialist_feat_at_plus_16 },
    { "BR46995_enchant_below_16_grants_nothing",    specialist_denied_setup,    specialist_denied_step, no_specialist_below_plus_16 },

    { "BR30836_gold_pickup_adds_to_total",          gold_pickup_setup,          NULL, gold_pickup_adds_to_total },
    { "BR61649_pickup_promotes_switch_tile",        pickup_promotes_switch_setup, NULL, pickup_triggers_tile_effects },
    { "BR31757_second_amulet_cannot_be_taken",      second_amulet_setup,        NULL, second_amulet_cannot_be_taken },
    { "BR13028_amulet_is_worth_35000",              NULL,                       NULL, amulet_is_worth_35000 },
    { "BR13028_lumenstone_is_worth_5000",           NULL,                       NULL, lumenstone_is_worth_5000 },
    { "BR54346_gold_pile_is_in_range",              gold_pile_setup,            NULL, gold_pile_is_in_range },

    { "BR39162_staff_starts_with_charges",          NULL,                       NULL, staff_starts_with_charges },
    { "BR22095_charm_starts_ready",                 NULL,                       NULL, charm_starts_ready },
    { "BR35670_potions_do_not_stack",               NULL,                       NULL, potions_do_not_stack },
    { "BR32867_items_have_distinct_letters",        NULL,                       NULL, items_have_distinct_letters },

    { "BR30796_armor_adds_ten_defense_per_enchant", NULL,                       NULL, armor_adds_ten_defense_per_enchant },
    { "BR30796_donning_armor_reduces_defense",      NULL,  NULL, donning_armor_reduces_defense },

    { "BR83562_armor_identifies_after_wearing_delay", br83562_setup, br83562_step, br83562_armor_identifies_after_wearing_delay },

    { "BR56988_enchanting_a_ring_raises_its_enchant", ring_enchant_setup,        ring_enchant_step, enchanting_a_ring_raises_its_enchant },
    { "BR56988_enchanting_clairvoyance_updates_vision", ring_clairvoyance_setup, ring_clairvoyance_step, enchanting_clairvoyance_updates_vision },
    { "BR40130_ring_enchant_adds_to_bonuses",        ring_bonuses_setup,         NULL,               ring_enchant_adds_to_each_bonus },

    { "BR15388_thrown_weapon_hit_deals_damage",     throw_hit_setup,             throw_hit_step, thrown_weapon_hit_deals_damage },
    { "BR15388_thrown_weapon_miss_lands_on_floor",  throw_miss_setup,            throw_miss_step, thrown_weapon_miss_lands_on_floor },

    { "BR90956_throw_aim_stops_at_target",          throw_aim_setup,             throw_aim_step, throw_aim_trajectory_stops_at_target },

    { "BR28082_level_item_counts_follow_depth_bands", NULL, NULL, level_item_counts_follow_depth_bands },
    { "BR47394_items_avoid_hallways_and_favor_secret_rooms", br47394_setup, NULL, br47394_verify },

    { "BR94199_beckoning_pulls_monster_adjacent", br94199_setup, br94199_step, beckoning_pulls_the_monster_adjacent },

    { "BR61428_domination_binds_or_resists",      br61428_setup,  br61428_step,  br61428_domination_binds_or_resists },
    { "BR69013_domination_chance_depends_on_current_health", br69013_setup, NULL, br69013_chance_depends_on_current_health },

    { "BR43662_item_on_pressure_plate_depresses_it", br43662_plate_setup, br43662_place_item_step, br43662_plate_depresses },

    { "BR90259_empty_staff_cannot_fire",            br90259_setup,        br90259_step,           br90259_empty_staff_cannot_fire },

    { "BR71867_kinds_known_the_moment_they_appear", born_known_setup,     NULL,                   known_the_moment_they_appear },

    { "BR84582_reflected_bolt_strikes_its_own_caster", br84582_setup, NULL, br84582_verify },

    { "BR69624_runic_weapon_runes_fade_at_zero",  br69624_weapon_setup, br69624_weapon_attacks_step, br69624_weapon_runes_fade_at_zero },
    { "BR69624_runic_weapon_keeps_runes_above_zero", br69624_above_setup, br69624_weapon_attacks_step, br69624_above_zero_keeps_runes },
    { "BR69624_runic_armor_runes_fade_at_zero",   br69624_armor_setup,   br69624_armor_struck_step,   br69624_armor_runes_fade_at_zero },

    { "BR75263_auto_target_picks_only_a_valid_target", br75263_setup, br75263_zap_aim_step, br75263_auto_target_picks_only_a_valid_target },

    { "BR44912_scroll_teleportation_moves_you",   br44912_setup,  br44912_step, br44912_teleports_to_another_part_of_the_level },

    { "BR18415_third_ring_replaces_the_chosen_worn_ring", br18415_swap_setup, NULL, br18415_third_ring_replaces_the_chosen_one },
    { "BR18415_third_ring_waits_for_a_choice",    br18415_cancel_setup, NULL, br18415_third_ring_waits_for_a_choice },
    { "BR18415_worn_ring_cannot_be_equipped_again", br18415_repeat_setup, NULL, br18415_worn_ring_cannot_be_worn_again },

    { NULL, NULL, NULL, NULL }
};

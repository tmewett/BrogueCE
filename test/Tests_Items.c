// Tests derived from rules/05-items.md.
//
// These cover the deterministic, headless-observable item rules: the effects
// of drinking potions, a scroll, picking up gold and the Amulet, the tile a
// pickup promotes, stacking and pack slots, generated charge counts and gold
// values, the defense an enchanted suit of armor contributes, the specialist
// feat, the number of items each level generates, and the pressure plate an
// item placed on it depresses.
//
// Omitted are everything else about level population (what is generated and
// where), random chances (curses, runics, mutations, spawn counts), flavour
// shuffling, identification RNG, targeting and trajectory prompts, bolt
// physics and reflection, dropping, machine interactions other than switch
// promotion and pressure plates, and any rule that needs a screen or is
// otherwise not reproducible headlessly.

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
    applyStatusToPlayer(STATUS_DONNING,0);
    recalculateEquipmentBonuses();
    int donning0 = playerDefense();
    applyStatusToPlayer(STATUS_DONNING,1);
    recalculateEquipmentBonuses();
    int donning1 = playerDefense();
    applyStatusToPlayer(STATUS_DONNING,2);
    recalculateEquipmentBonuses();
    int donning2 = playerDefense();
    TEST_ASSERT(donning0 > donning1 && donning1 > donning2,
                "armor should add less defense while it is being donned");
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
    item *dagger = givePlayerItem(WEAPON, DAGGER, 3);
    // generateItem() may have rolled the dagger a runic; a W_MERCY hit heals
    // the target back to full and would erase the damage this test asserts on.
    // Strip it so the dagger is always plain.
    dagger->flags &= ~(ITEM_RUNIC | ITEM_CURSED);
    dagger->enchant2 = 0;
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
    { "BR63185_detect_magic_reveals_polarity",      br63185_detect_magic_setup, br63185_detect_magic_step, br63185_detect_magic_reveals_polarity },

    { "BR29492_scroll_of_aggravate_wakes_monsters", aggravate_scroll_setup,     NULL, aggravate_scroll_wakes_monsters },
    { "BR95938_reading_scroll_consumes_it",         consume_scroll_setup,       NULL, reading_scroll_consumes_it },
    { "BR47462_enchanting_removes_curse",           enchanting_removes_curse_setup, enchanting_removes_curse_step, curse_removed_by_enchanting },

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

    { "BR56988_enchanting_a_ring_raises_its_enchant", ring_enchant_setup,        ring_enchant_step, enchanting_a_ring_raises_its_enchant },
    { "BR56988_enchanting_clairvoyance_updates_vision", ring_clairvoyance_setup, ring_clairvoyance_step, enchanting_clairvoyance_updates_vision },
    { "BR40130_ring_enchant_adds_to_bonuses",        ring_bonuses_setup,         NULL,               ring_enchant_adds_to_each_bonus },

    { "BR15388_thrown_weapon_hit_deals_damage",     throw_hit_setup,             throw_hit_step, thrown_weapon_hit_deals_damage },
    { "BR15388_thrown_weapon_miss_lands_on_floor",  throw_miss_setup,            throw_miss_step, thrown_weapon_miss_lands_on_floor },

    { "BR28082_level_item_counts_follow_depth_bands", NULL, NULL, level_item_counts_follow_depth_bands },

    { "BR94199_beckoning_pulls_monster_adjacent", br94199_setup, br94199_step, beckoning_pulls_the_monster_adjacent },

    { "BR43662_item_on_pressure_plate_depresses_it", br43662_plate_setup, br43662_place_item_step, br43662_plate_depresses },

    { "BR90259_empty_staff_cannot_fire",            br90259_setup,        br90259_step,           br90259_empty_staff_cannot_fire },

    { NULL, NULL, NULL, NULL }
};

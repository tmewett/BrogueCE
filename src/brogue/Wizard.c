/*
 *  Wizard.c
 *  Brogue
 *
 *  Copyright 2012. All rights reserved.
 *
 *  This file is part of Brogue.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Rogue.h"
#include "GlobalsBase.h"
#include "Globals.h"

static void initializeCreateItemButton(brogueButton *button, char *text) {
    char buttonText[COLS * 3];

    initializeButton(button);
    strcpy(buttonText,text);
    upperCase(buttonText);
    strcpy(button->text,buttonText);
}

#define DIALOG_CREATE_ITEM_MAX_BUTTONS 26

// Display a dialog window for the user to select a single entry from a list
static short dialogSelectEntryFromList(
        brogueButton *buttons,
        short buttonCount,
        char *windowTitle
) {

    short x=0, y=0, width=0, height=0;
    screenDisplayBuffer dbuf;
    short i, selectedButton, len, maxLen;
    char buttonText[COLS];

    maxLen = strlen(windowTitle);

    for (i=0; i < buttonCount; i++) {
        buttons[i].flags &= ~(B_WIDE_CLICK_AREA | B_GRADIENT);
        buttons[i].buttonColor = interfaceBoxColor;
        buttons[i].hotkey[0] = 'a' + i;
        buttons[i].hotkey[1] = 'A' + i;
        buttons[i].x = WINDOW_POSITION_DUNGEON_TOP_LEFT.window_x;
        buttons[i].y = WINDOW_POSITION_DUNGEON_TOP_LEFT.window_y + 1 + i;
        if (KEYBOARD_LABELS) {
            sprintf(buttonText, "%c) %s", (int)buttons[i].hotkey[0], buttons[i].text);
            strcpy(buttons[i].text, buttonText);
            len = strlen(buttonText);
            if (len > maxLen) {
                maxLen = len;
            }
        }
    }

    width = maxLen + 1;
    height = buttonCount + 2;
    x = WINDOW_POSITION_DUNGEON_TOP_LEFT.window_x;
    y = WINDOW_POSITION_DUNGEON_TOP_LEFT.window_y;
    clearDisplayBuffer(&dbuf);

    //Dialog Title
    printString(windowTitle, x , y - 1, &itemMessageColor, &interfaceBoxColor, &dbuf);
    //Dialog background
    rectangularShading(x - 1, y - 1, width + 1, height + 1, &interfaceBoxColor, INTERFACE_OPACITY, &dbuf);
    //Display the title/background and save the prior display state
    const SavedDisplayBuffer rbuf = saveDisplayBuffer();
    overlayDisplayBuffer(&dbuf);
    //Display the buttons and wait for user selection
    selectedButton = buttonInputLoop(buttons, buttonCount, x, y, width, height, NULL);
    //Revert the display state
    restoreDisplayBuffer(&rbuf);

    return selectedButton;
}

// Display a dialog window for the user to chose a vorpal enemy. Remove the runic if none selected.
static short dialogCreateItemChooseVorpalEnemy() {
    char buttonText[COLS];
    short i;
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];

    for (i = 0; i < MONSTER_CLASS_COUNT; i++) {
        strcpy(buttonText,monsterClassCatalog[i].name);
        initializeCreateItemButton(&(buttons[i]), buttonText);
    }

    return dialogSelectEntryFromList(buttons, i, "Choose a vorpal enemy:");
}

// Display a dialog window for the user to select a runic for the given armor or weapon.
// Assigns the selected runic and vorpal enemy (if applicable) to the item. No return value.
static void dialogCreateItemChooseRunic(item *theItem){
    char buttonText[COLS];
    short i=0, runicOffset =0, noRunic, selectedRunic, selectedVorpalEnemy;
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];

    if (!(theItem->category & (WEAPON | ARMOR))) {
        return;
    }

    // Heavy weapons can only have bad runics. The logic below but differs from that used in dungeon generation, which
    // is based on minimum weapon damage, but has the same net effect. Might be nice to consolidate. Perhaps add a flag
    // to the kind (itemtable).
    if (theItem->category == WEAPON) {
        if (theItem->kind == HAMMER || theItem->kind == WAR_AXE || theItem->kind == PIKE || theItem->kind == BROADSWORD) {
            for (i=0; i<NUMBER_WEAPON_RUNIC_KINDS - NUMBER_GOOD_WEAPON_ENCHANT_KINDS; i++) {
                strcpy(buttonText, weaponRunicNames[i + NUMBER_GOOD_WEAPON_ENCHANT_KINDS]);
                initializeCreateItemButton(&(buttons[i]), buttonText);
            }
            runicOffset = NUMBER_GOOD_WEAPON_ENCHANT_KINDS;
            // Other weapons (except thrown), can have any runic
        } else if (theItem->quiverNumber == 0) {
            for (i=0; i<NUMBER_WEAPON_RUNIC_KINDS; i++) {
                strcpy(buttonText, weaponRunicNames[i]);
                initializeCreateItemButton(&(buttons[i]), buttonText);
            }
        } else { // No runics allowed
            return;
        }
    } else if (theItem->category == ARMOR) {
        if (theItem->kind == PLATE_MAIL) { //bad runics only
            for (i=0; i<NUMBER_ARMOR_ENCHANT_KINDS - NUMBER_GOOD_ARMOR_ENCHANT_KINDS; i++) {
                strcpy(buttonText, armorRunicNames[i + NUMBER_GOOD_ARMOR_ENCHANT_KINDS]);
                initializeCreateItemButton(&(buttons[i]), buttonText);
            }
            runicOffset = NUMBER_GOOD_ARMOR_ENCHANT_KINDS;
        } else {
            for (i=0; i<NUMBER_ARMOR_ENCHANT_KINDS; i++) { //any runic
                strcpy(buttonText, armorRunicNames[i]);
                initializeCreateItemButton(&(buttons[i]), buttonText);
            }
        }
    }
    // add an extra button for choosing no runic
    initializeCreateItemButton(&(buttons[i]), "No Runic");
    noRunic = i;

    selectedRunic = dialogSelectEntryFromList(buttons, i+1, "Choose a runic:");

    if (selectedRunic >=0 && selectedRunic != noRunic) {
        theItem->enchant2 = selectedRunic + runicOffset;
        theItem->flags |= ITEM_RUNIC;

        if ((theItem->enchant2 == W_SLAYING && theItem->category == WEAPON)
            || (theItem->enchant2 == A_IMMUNITY && theItem->category == ARMOR)) {
            selectedVorpalEnemy = dialogCreateItemChooseVorpalEnemy();

            if (selectedVorpalEnemy >=0) {
                theItem->vorpalEnemy = selectedVorpalEnemy;
            } else { // remove the runic if no vorpal enemy chosen
                theItem->enchant2 = 0;
                theItem->flags &= ~(ITEM_RUNIC);
            }
        }
    } else if (selectedRunic == noRunic) {
        theItem->enchant2 = 0;
        theItem->flags &= ~(ITEM_RUNIC);
    }
}

// Display a dialog window for the user to select an item kind from the specified catogory. Returns the selected kind.
static short dialogCreateItemChooseKind(enum itemCategory category) {
    char buttonText[COLS];
    short i;
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];
    itemTable *kindTable;
    int kindCount;
    char title[50] = "";

    kindTable = tableForItemCategory(category);
    kindCount = itemKindCount(category, 0);

    for (i=0; i<kindCount; i++) {
        strncpy(buttonText, kindTable[i].name, COLS);
        initializeCreateItemButton(&(buttons[i]), buttonText);
    }

    sprintf(title,"Create %s:",itemCategoryNames[unflag(category)]);
    return dialogSelectEntryFromList(buttons, i, title);
}

// Display an input dialog for the user to enter the enchantment level for the item being created.
// Assigns the enchantment level to the item. Does not return a value.
static void dialogCreateItemChooseEnchantmentLevel(item *theItem) {
    short minVal = 0, maxVal = 50, defaultVal, maxInputLength = 2;
    char prompt[100], buf[100], inputBuf[100]="";

    defaultVal = theItem->enchant1;

    // Item generation creates valid enchantment values except for the case of runics. So in all other cases we can
    // retain the generated enchantment value as the default, if the user doesn't provide a value.
    if (theItem->category == WAND) {
        minVal = 0;
        defaultVal = theItem->charges;
    } else if (theItem->category == CHARM) {
        minVal = 1;
    } else if (theItem->category == STAFF) {
        minVal = 2;
    } else if (theItem->category == RING) {
        minVal = -3;
    } else if (theItem->category & (WEAPON | ARMOR)) {
        if (!(theItem->flags & ITEM_RUNIC)) {
            minVal = -3;
        } else { // bad runics can only be negatively enchanted
            if ((theItem->category == ARMOR && theItem->enchant2 >= NUMBER_GOOD_ARMOR_ENCHANT_KINDS)
                || (theItem->category == WEAPON && theItem->enchant2 >= NUMBER_GOOD_WEAPON_ENCHANT_KINDS)) {
                minVal = -3;
                maxVal = -1;
                theItem->enchant1 = rand_range(-3,-1);
            } else { // good runics can only be positively enchanted
                minVal = 1;
                if (theItem->enchant1 < 1) {
                    theItem->enchant1 = rand_range(1,3);
                }
            }
            defaultVal = theItem->enchant1;
        }
    }

    sprintf(prompt, "How many enchants (%d to %d) [default %d]?", minVal, maxVal, defaultVal);
    getInputTextString(inputBuf, prompt, maxInputLength, "", "", TEXT_INPUT_NORMAL, true);

    // Validate the input
    int enchants;
    if (strlen(inputBuf)                      // we need some input
        && sscanf(inputBuf, "%d", &enchants)  // try to convert to number
        && sprintf(buf, "%d", enchants)       // convert back to string
        && strcmp(buf, inputBuf) == 0) {       // compare (0 if equal)
        // handle out of range input
        if (enchants > maxVal) {
            enchants = maxVal;
        } else if (enchants < minVal) {
            enchants = defaultVal;
        }

        // set enchantment level based on item category
        if (theItem->category == WAND) {
            theItem->charges = enchants;
        } else if (theItem->category == STAFF) {
            theItem->charges = theItem->enchant1 = enchants;
        } else {
            theItem->enchant1 = enchants;
        }
    }

    if (theItem->enchant1 < 0) {
        theItem->flags |= ITEM_CURSED;
    }
}

static int creatureTypeCompareMonsterNames (const void * a, const void * b) {
    creatureType *c1 = (creatureType *)a;
    creatureType *c2 = (creatureType *)b;

    char name1[COLS];
    char name2[COLS];
    strncpy(name1,c1->monsterName,COLS);
    strncpy(name2,c2->monsterName,COLS);
    upperCase(name1);
    upperCase(name2);

    return strcmp(name1, name2);
}

// Display a dialog window for the user to select a mutation for the specified monster.
static void dialogCreateMonsterChooseMutation(creature *theMonster) {
    char buttonText[COLS];
    short i, j = 0, noMutation, selectedMutation;
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];

    for (i=0; i<NUMBER_MUTATORS; i++) {
        if (!(theMonster->info.flags & mutationCatalog[i].forbiddenFlags)
            && !(theMonster->info.abilityFlags & mutationCatalog[i].forbiddenAbilityFlags)) {
            strncpy(buttonText, mutationCatalog[i].title, COLS);
            initializeCreateItemButton(&(buttons[j]), buttonText);
            j++;
        }
    }
    // add an extra button for choosing no mutation
    initializeCreateItemButton(&(buttons[j]), "No mutation");
    noMutation = j;

    selectedMutation = dialogSelectEntryFromList(buttons, j+1, "Choose a mutation:");

    if (selectedMutation != noMutation) {
        mutateMonster(theMonster, selectedMutation);
    }
}

// Display a series of dialog windows for creating an arbitrary monster chosen by the user
static void dialogCreateMonster() {
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];
    char buttonText[COLS];
    pos selectedPosition;
    boolean locationIsValid = true;
    short i, j=0, selectedMonster, monsterOffset, buttonCount=0;
    const short MONSTERS_PER_PAGE = 24;
    const short CREATABLE_MONSTER_KINDS = NUMBER_MONSTER_KINDS - 2;
    char theMessage[COLS] = "";
    char monsterRangeText[COLS];
    creature *theMonster, *oldMonster;
    creatureType monsterKinds[NUMBER_MONSTER_KINDS];

    // There are too many monsters to fit on a page so we break them into ranges. First we copy the monster catalog and
    // sort it alphabetically by name. Also, we exclude creation of lich and phoenix as those are summoned by phylactery
    // and phoenix egg respectively.
    for (i=0; i < NUMBER_MONSTER_KINDS; i++) {
        if (!(monsterCatalog[i].monsterID == MK_LICH) && !(monsterCatalog[i].monsterID == MK_PHOENIX)) {
            monsterKinds[j] = monsterCatalog[i];
            upperCase(monsterKinds[j].monsterName);
            j++;
        }
    }

    qsort(monsterKinds,CREATABLE_MONSTER_KINDS,sizeof(creatureType),&creatureTypeCompareMonsterNames);

    for (i=0; i < CREATABLE_MONSTER_KINDS; i+=MONSTERS_PER_PAGE) {
        if (i + MONSTERS_PER_PAGE < CREATABLE_MONSTER_KINDS) {
            sprintf(monsterRangeText,"%s - %s",monsterKinds[i].monsterName, monsterKinds[i + MONSTERS_PER_PAGE-1].monsterName);
        } else {
            sprintf(monsterRangeText,"%s - %s",monsterKinds[i].monsterName, monsterKinds[CREATABLE_MONSTER_KINDS-1].monsterName);
        }
        initializeCreateItemButton(&(buttons[buttonCount]), monsterRangeText);
        buttonCount++;
    }

    // choose a monster range
    monsterOffset = dialogSelectEntryFromList(buttons, buttonCount, "Create monster:");
    if (monsterOffset == -1) {
        return;
    }
    monsterOffset*= MONSTERS_PER_PAGE;

    // populate menu of monsters in the selected range
    buttonCount = 0;
    for (i=monsterOffset; i < monsterOffset + MONSTERS_PER_PAGE && i < CREATABLE_MONSTER_KINDS; i++) {
        strncpy(buttonText, monsterKinds[i].monsterName, COLS);
        initializeCreateItemButton(&(buttons[buttonCount]), buttonText);
        buttonCount++;
    }

    // choose a monster
    selectedMonster = dialogSelectEntryFromList(buttons, buttonCount, "Create monster:");
    if (selectedMonster == -1) {
        return;
    }

    selectedMonster+= monsterOffset;

    theMonster = generateMonster(monsterKinds[selectedMonster].monsterID, false, false);

    // Choose a mutation
    if (!(theMonster->info.flags & MONST_NEVER_MUTATED) && !(theMonster->info.abilityFlags & MA_NEVER_MUTATED)) {
        dialogCreateMonsterChooseMutation(theMonster);
    }
    initializeMonster(theMonster, false);
    
    if (theMonster->info.displayChar == G_TURRET) {
        sprintf(theMessage, "Create %s where? Choose a visible wall.", theMonster->info.monsterName);
    } else {
        sprintf(theMessage, "Create %s where? Choose a visible unobstructed location.", theMonster->info.monsterName);
    }
    temporaryMessage(theMessage, REFRESH_SIDEBAR);

    // pick a location
    if (chooseTarget(&selectedPosition, 0, AUTOTARGET_MODE_NONE, NULL)) {
        confirmMessages();

        if (!playerCanSeeOrSense(selectedPosition.x, selectedPosition.y)) {
            locationIsValid = false;
        }
        if (theMonster->info.displayChar == G_TURRET && (!(pmapAt(selectedPosition)->layers[DUNGEON] == WALL))) {
            locationIsValid = false;
        }
        if (!(theMonster->info.displayChar == G_TURRET) && cellHasTerrainFlag(selectedPosition, T_OBSTRUCTS_PASSABILITY)) {
            locationIsValid = false;
        }

        if (!locationIsValid) {
            sprintf(theMessage, "Invalid location. %s", KEYBOARD_LABELS ? "-- Press space or click to continue --" : "-- Touch anywhere to continue --");
            temporaryMessage(theMessage, REQUIRE_ACKNOWLEDGMENT);
            killCreature(theMonster, true);
            removeDeadMonsters();
            return;
        }

        // If there's already a monster here, quietly bury the body.
        oldMonster = monsterAtLoc(selectedPosition);
        if (oldMonster) {
            killCreature(oldMonster, true);
            removeDeadMonsters();
        }

        theMonster->loc = selectedPosition;
        pmapAt(theMonster->loc)->flags |= HAS_MONSTER;
        theMonster->creatureState = MONSTER_WANDERING;
        fadeInMonster(theMonster);
        refreshSideBar(-1, -1, false);
        refreshDungeonCell(theMonster->loc);

        if (!(theMonster->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
            || theMonster->info.monsterID == MK_PHOENIX_EGG
            || theMonster->info.monsterID == MK_PHYLACTERY
                ) {
            sprintf(theMessage, "Make the %s your ally?", theMonster->info.monsterName);
            if (confirm(theMessage, false)) {
                becomeAllyWith(theMonster);
                sprintf(theMessage, "Allied %s created.", theMonster->info.monsterName);
            } else {
                sprintf(theMessage, "%s created.", theMonster->info.monsterName);
            }
        } else {
            sprintf(theMessage, "%s created.", theMonster->info.monsterName);
        }
        message(theMessage, 0);
    } else {  // no location chosen
        confirmMessages();
        killCreature(theMonster, true);
        removeDeadMonsters();
    }
    refreshSideBar(-1, -1, false);
}

// Display a series of dialog windows for creating an arbitrary item chosen by the user
static void dialogCreateItem() {
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];
    char buttonText[COLS];
    short i, selectedCategory, selectedKind;
    char message[COLS] = "";
    item *theItem;
    char theItemName[100]="";

    if (numberOfItemsInPack() == MAX_PACK_ITEMS) {
        messageWithColor("Your pack is already full.", &itemMessageColor, 0);
        return;
    }

    for (i=0; i < NUMBER_ITEM_CATEGORIES; i++) {
        strncpy(buttonText, itemCategoryNames[i], COLS);
        initializeCreateItemButton(&(buttons[i]), buttonText);
    }

    selectedCategory = dialogSelectEntryFromList(buttons, i, "Create item:");

    if (tableForItemCategory(Fl(selectedCategory))) {
        selectedKind = dialogCreateItemChooseKind(Fl(selectedCategory));
    } else if (selectedCategory == -1) {
        return;
    } else {
        selectedKind = 0;
    }

    if (selectedKind >= 0) {
        theItem = generateItem(Fl(selectedCategory), selectedKind);
        theItem->flags &= ~(ITEM_CURSED); // we'll add a curse later if the item is negatively enchanted
    } else {
        return;
    }

    if (Fl(selectedCategory) == GEM) {
        theItem->originDepth = rogue.depthLevel;
    }

    if (Fl(selectedCategory) == GOLD) {
        rogue.gold += theItem->quantity;
        sprintf(message, "you found %i pieces of gold.%s", theItem->quantity, rogue.featRecord[FEAT_TONE] ? ".. and strike a tone of disappointment." : "");
        rogue.featRecord[FEAT_TONE] = false;
        messageWithColor(message, &itemMessageColor, 0);
        deleteItem(theItem);
        return;
    }

    if (Fl(selectedCategory) & (ARMOR | WEAPON)) {
        dialogCreateItemChooseRunic(theItem);
    }

    if (Fl(selectedCategory) & CAN_BE_ENCHANTED  && theItem->quiverNumber == 0) {
        dialogCreateItemChooseEnchantmentLevel(theItem);
    }

    if (theItem->flags & ITEM_CAN_BE_IDENTIFIED && !(Fl(selectedCategory) & NEVER_IDENTIFIABLE)) {
        if (confirm("Identify the item?", false)) {
            identify(theItem);
        }
    }

    theItem = addItemToPack(theItem);
    itemName(theItem, theItemName, true, true, NULL);

    sprintf(message,"you now have %s (%c).", theItemName, theItem->inventoryLetter);
    messageWithColor(message, &itemMessageColor, 0);
}

void dialogCreateItemOrMonster() {
    brogueButton buttons[DIALOG_CREATE_ITEM_MAX_BUTTONS];
    short selectedType;

    initializeCreateItemButton(&(buttons[0]), "Item");
    initializeCreateItemButton(&(buttons[1]), "Monster");

    selectedType = dialogSelectEntryFromList(buttons, 2, "Create:");

    if (selectedType == 0) {
        dialogCreateItem();
    } else if (selectedType == 1) {
        dialogCreateMonster();
    }
}

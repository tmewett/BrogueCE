/*
 *  Time.c
 *  Brogue
 *
 *  Created by Brian Walker on 6/21/13.
 *  Copyright 2013. All rights reserved.
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

void exposeCreatureToFire(creature *monst) {
    char buf[COLS], buf2[COLS];
    if ((monst->bookkeepingFlags & MB_IS_DYING)
        || monst->status[STATUS_IMMUNE_TO_FIRE]
        || (monst->info.flags & MONST_INVULNERABLE)
        || (monst->bookkeepingFlags & MB_SUBMERGED)
        || ((!monst->status[STATUS_LEVITATING]) && cellHasTMFlag(monst->loc, TM_EXTINGUISHES_FIRE))) {
        return;
    }
    if (monst->status[STATUS_BURNING] == 0) {
        if (monst == &player) {
            rogue.minersLight.lightColor = &fireForeColor;
            player.info.foreColor = &torchLightColor;
            refreshDungeonCell(player.loc);
            //updateVision(); // this screws up the firebolt visual effect by erasing it while a message is displayed
            combatMessage("you catch fire", &badMessageColor);
        } else if (canDirectlySeeMonster(monst)) {
            monsterName(buf, monst, true);
            sprintf(buf2, "%s catches fire", buf);
            combatMessage(buf2, messageColorFromVictim(monst));
        }
    }
    monst->status[STATUS_BURNING] = monst->maxStatus[STATUS_BURNING] = max(monst->status[STATUS_BURNING], 7);
}

void updateFlavorText() {
    char buf[DCOLS * 3];
    if (rogue.disturbed && !rogue.gameHasEnded) {
        if (rogue.armor
            && (rogue.armor->flags & ITEM_RUNIC)
            && rogue.armor->enchant2 == A_RESPIRATION
            && tileCatalog[pmapAt(player.loc)->layers[highestPriorityLayer(player.loc.x, player.loc.y, false)]].flags & T_RESPIRATION_IMMUNITIES) {

            flavorMessage("A pocket of cool, clean air swirls around you.");
        } else if (player.status[STATUS_LEVITATING]) {
            describeLocation(buf, player.loc.x, player.loc.y);
            flavorMessage(buf);
        } else {
            flavorMessage(tileFlavor(player.loc.x, player.loc.y));
        }
    }
}

void updatePlayerUnderwaterness() {
    if (rogue.inWater) {
        if (!cellHasTerrainFlag(player.loc, T_IS_DEEP_WATER) || player.status[STATUS_LEVITATING]
            || cellHasTerrainFlag(player.loc, (T_ENTANGLES | T_OBSTRUCTS_PASSABILITY))) {

            rogue.inWater = false;
            updateMinersLightRadius();
            updateVision(true);
            displayLevel();
        }
    } else {
        if (cellHasTerrainFlag(player.loc, T_IS_DEEP_WATER) && !player.status[STATUS_LEVITATING]
            && !cellHasTerrainFlag(player.loc, (T_ENTANGLES | T_OBSTRUCTS_PASSABILITY))) {

            rogue.inWater = true;
            updateMinersLightRadius();
            updateVision(true);
            displayLevel();
        }
    }
}

boolean monsterShouldFall(creature *monst) {
    return (!(monst->status[STATUS_LEVITATING])
            && cellHasTerrainFlag(monst->loc, T_AUTO_DESCENT)
            && !cellHasTerrainFlag(monst->loc, T_ENTANGLES | T_OBSTRUCTS_PASSABILITY)
            && !(monst->bookkeepingFlags & MB_PREPLACED));
}

// Called at least every 100 ticks; may be called more frequently.
void applyInstantTileEffectsToCreature(creature *monst) {
    char buf[COLS], buf2[COLS], buf3[COLS];
    const char *s;
    short *x = &(monst->loc.x), *y = &(monst->loc.y), damage;
    enum dungeonLayers layer;
    item *theItem;

    if (monst->bookkeepingFlags & MB_IS_DYING) {
        return; // the monster is already dead.
    }

    if (monst == &player) {
        if (!player.status[STATUS_LEVITATING]) {
            pmap[*x][*y].flags |= KNOWN_TO_BE_TRAP_FREE;
        }
    } else if (!player.status[STATUS_HALLUCINATING]
               && !monst->status[STATUS_LEVITATING]
               && canSeeMonster(monst)
               && !(cellHasTerrainFlag((pos){ *x, *y }, T_IS_DF_TRAP))) {
        pmap[*x][*y].flags |= KNOWN_TO_BE_TRAP_FREE;
    }

    // You will discover the secrets of any tile you stand on.
    if (monst == &player
        && !(monst->status[STATUS_LEVITATING])
        && cellHasTMFlag((pos){ *x, *y }, TM_IS_SECRET)
        && playerCanSee(*x, *y)) {

        discover(*x, *y);
    }

    // Submerged monsters in terrain that doesn't permit submersion should immediately surface.
    if ((monst->bookkeepingFlags & MB_SUBMERGED) && !cellHasTMFlag((pos){ *x, *y }, TM_ALLOWS_SUBMERGING)) {
        monst->bookkeepingFlags &= ~MB_SUBMERGED;
    }

    // Visual effect for submersion in water.
    if (monst == &player) {
        updatePlayerUnderwaterness();
    }

    // Obstructed krakens can't seize their prey.
    if ((monst->bookkeepingFlags & MB_SEIZING)
        && (cellHasTerrainFlag((pos){ *x, *y }, T_OBSTRUCTS_PASSABILITY))
        && !(monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)) {

        monst->bookkeepingFlags &= ~MB_SEIZING;
    }

    // Creatures plunge into chasms and through trap doors.
    if (monsterShouldFall(monst)) {
        if (monst == &player) {
            // player falling takes place at the end of the turn
            if (!(monst->bookkeepingFlags & MB_IS_FALLING)) {
                monst->bookkeepingFlags |= MB_IS_FALLING;
            }
            return;
        } else { // it's a monster
            monst->bookkeepingFlags |= MB_IS_FALLING; // handled at end of turn
        }
    }

    // lava
    if (!(monst->status[STATUS_LEVITATING])
        && !(monst->status[STATUS_IMMUNE_TO_FIRE])
        && !(monst->info.flags & MONST_INVULNERABLE)
        && !cellHasTerrainFlag((pos){ *x, *y }, (T_ENTANGLES | T_OBSTRUCTS_PASSABILITY))
        && !cellHasTMFlag((pos){ *x, *y }, TM_EXTINGUISHES_FIRE)
        && cellHasTerrainFlag((pos){ *x, *y }, T_LAVA_INSTA_DEATH)) {

        if (monst == &player) {
            sprintf(buf, "you plunge into %s!",
                    tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_LAVA_INSTA_DEATH)]].description);
            message(buf, REQUIRE_ACKNOWLEDGMENT);
            sprintf(buf, "Killed by %s",
                    tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_LAVA_INSTA_DEATH)]].description);
            gameOver(buf, true);
            return;
        } else { // it's a monster
            if (canSeeMonster(monst)) {
                monsterName(buf, monst, true);
                s = tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_LAVA_INSTA_DEATH)]].description;
                // Skip over articles
                if (strncmp(s, "a ", 2) == 0) {
                    s += 2;
                } else if (strncmp(s, "an ", 3) == 0) {
                    s += 3;
                }
                sprintf(buf2, "%s is consumed by the %s instantly!", buf, s);
                messageWithColor(buf2, messageColorFromVictim(monst), 0);
            }
            killCreature(monst, false);
            spawnDungeonFeature(*x, *y, &(dungeonFeatureCatalog[DF_CREATURE_FIRE]), true, false);
            refreshDungeonCell((pos){ *x, *y });
            return;
        }
    }

    // Water puts out fire.
    if (cellHasTMFlag((pos){ *x, *y }, TM_EXTINGUISHES_FIRE)
        && monst->status[STATUS_BURNING]
        && !monst->status[STATUS_LEVITATING]
        && !(monst->info.flags & MONST_ATTACKABLE_THRU_WALLS)
        && !(monst->info.flags & MONST_FIERY)) {
        extinguishFireOnCreature(monst);
    }

    // If you see a monster use a secret door, you discover it.
    if (playerCanSee(*x, *y)
        && cellHasTMFlag((pos){ *x, *y }, TM_IS_SECRET)
        && (cellHasTerrainFlag((pos){ *x, *y }, T_OBSTRUCTS_PASSABILITY))) {
        discover(*x, *y);
    }

    // Pressure plates.
    if (!(monst->status[STATUS_LEVITATING])
        && !(monst->bookkeepingFlags & MB_SUBMERGED)
        && (!cellHasTMFlag((pos){ *x, *y }, TM_ALLOWS_SUBMERGING) || !(monst->info.flags & MONST_SUBMERGES))
        && cellHasTerrainFlag((pos){ *x, *y }, T_IS_DF_TRAP)
        && !(pmap[*x][*y].flags & PRESSURE_PLATE_DEPRESSED)) {

        pmap[*x][*y].flags |= PRESSURE_PLATE_DEPRESSED;
        if (playerCanSee(*x, *y) && cellHasTMFlag((pos){ *x, *y }, TM_IS_SECRET)) {
            discover(*x, *y);
            refreshDungeonCell((pos){ *x, *y });
        }
        if (canSeeMonster(monst)) {
            monsterName(buf, monst, true);
            sprintf(buf2, "a pressure plate clicks underneath %s!", buf);
            message(buf2, REQUIRE_ACKNOWLEDGMENT);
        } else if (playerCanSee(*x, *y)) {
            // usually means an invisible monster
            message("a pressure plate clicks!", 0);
        }
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[*x][*y].layers[layer]].flags & T_IS_DF_TRAP) {
                spawnDungeonFeature(*x, *y, &(dungeonFeatureCatalog[tileCatalog[pmap[*x][*y].layers[layer]].fireType]), true, false);
                promoteTile(*x, *y, layer, false);
            }
        }
    }

    if (cellHasTMFlag((pos){ *x, *y }, TM_PROMOTES_ON_CREATURE)) { // flying creatures activate too
        // Because this uses no pressure plate to keep track of whether it's already depressed,
        // it will trigger every time this function is called while the monster or player is on the tile.
        // Because this function can be called several times per turn, multiple promotions can
        // happen unpredictably if the tile does not promote to a tile without the T_PROMOTES_ON_STEP
        // attribute. That's acceptable for some effects, e.g. doors opening,
        // but not for others, e.g. magical glyphs activating.
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[*x][*y].layers[layer]].mechFlags & TM_PROMOTES_ON_CREATURE) {
                promoteTile(*x, *y, layer, false);
            }
        }
    }

    if (cellHasTMFlag((pos){ *x, *y }, TM_PROMOTES_ON_PLAYER_ENTRY) && monst == &player) {
        // Subject to same caveats as T_PROMOTES_ON_STEP above.
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[*x][*y].layers[layer]].mechFlags & TM_PROMOTES_ON_PLAYER_ENTRY) {
                promoteTile(*x, *y, layer, false);
            }
        }
    }

    if (cellHasTMFlag((pos){ *x, *y }, TM_PROMOTES_ON_SACRIFICE_ENTRY)
        && monst->machineHome == pmap[*x][*y].machineNumber
        && (monst->bookkeepingFlags & MB_MARKED_FOR_SACRIFICE)) {
        // Subject to same caveats as T_PROMOTES_ON_STEP above.
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[*x][*y].layers[layer]].mechFlags & TM_PROMOTES_ON_SACRIFICE_ENTRY) {
                promoteTile(*x, *y, layer, false);
            }
        }
    }

    // spiderwebs
    if (cellHasTerrainFlag((pos){ *x, *y }, T_ENTANGLES) && !monst->status[STATUS_STUCK]
        && !(monst->info.flags & (MONST_IMMUNE_TO_WEBS | MONST_INVULNERABLE))
        && !(monst->bookkeepingFlags & MB_SUBMERGED)) {

        monst->status[STATUS_STUCK] = monst->maxStatus[STATUS_STUCK] = rand_range(3, 7);
        if (monst == &player) {
            if (!rogue.automationActive) {
                // Don't interrupt exploration with this message.
                sprintf(buf2, "you are stuck fast in %s!",
                        tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_ENTANGLES)]].description);
                message(buf2, 0);
            }
        } else if (canDirectlySeeMonster(monst)) { // it's a monster
            if (!rogue.automationActive) {
                monsterName(buf, monst, true);
                sprintf(buf2, "%s is stuck fast in %s!", buf,
                        tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_ENTANGLES)]].description);
                message(buf2, 0);
            }
        }
    }

    // explosions
    if (cellHasTerrainFlag((pos){ *x, *y }, T_CAUSES_EXPLOSIVE_DAMAGE) && !monst->status[STATUS_EXPLOSION_IMMUNITY]
        && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
        damage = rand_range(15, 20);
        damage = max(damage, monst->info.maxHP / 2);
        monst->status[STATUS_EXPLOSION_IMMUNITY] = 5;
        if (monst == &player) {
            rogue.disturbed = true;
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS && !(tileCatalog[pmap[*x][*y].layers[layer]].flags & T_CAUSES_EXPLOSIVE_DAMAGE); layer++);
            message(tileCatalog[pmap[*x][*y].layers[layer]].flavorText, 0);
            if (rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_DAMPENING) {
                itemName(rogue.armor, buf2, false, false, NULL);
                sprintf(buf, "Your %s pulses and absorbs the damage.", buf2);
                messageWithColor(buf, &goodMessageColor, 0);
                autoIdentify(rogue.armor);
            } else if (inflictDamage(NULL, &player, damage, &yellow, false)) {
                killCreature(&player, false);
                strcpy(buf2, tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_CAUSES_EXPLOSIVE_DAMAGE)]].description);
                sprintf(buf, "Killed by %s", buf2);
                gameOver(buf, true);
                return;
            }
        } else { // it's a monster
            if (monst->creatureState == MONSTER_SLEEPING) {
                monst->creatureState = MONSTER_TRACKING_SCENT;
            }
            monsterName(buf, monst, true);

            // Get explosive layer before damage in case a death DF replaces the explosion
            strcpy(buf3, tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_CAUSES_EXPLOSIVE_DAMAGE)]].description);
            if (inflictDamage(NULL, monst, damage, &yellow, false)) {
                // if killed
                sprintf(buf2, "%s %s %s.", buf,
                        (monst->info.flags & MONST_INANIMATE) ? "is destroyed by" : "dies in",
                        buf3);
                messageWithColor(buf2, messageColorFromVictim(monst), 0);
                killCreature(monst, false);
                refreshDungeonCell((pos){ *x, *y });
                return;
            } else {
                // if survived
                sprintf(buf2, "%s engulfs %s.",
                        tileCatalog[pmap[*x][*y].layers[layerWithFlag(*x, *y, T_CAUSES_EXPLOSIVE_DAMAGE)]].description, buf);
                messageWithColor(buf2, messageColorFromVictim(monst), 0);
            }
        }
    }

    // Toxic gases!
    // If it's the player, and he's wearing armor of respiration, then no effect from toxic gases.
    if (monst == &player
        && cellHasTerrainFlag((pos){ *x, *y }, T_RESPIRATION_IMMUNITIES)
        && rogue.armor
        && (rogue.armor->flags & ITEM_RUNIC)
        && rogue.armor->enchant2 == A_RESPIRATION) {
        if (!(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)) {
            message("Your armor trembles and a pocket of clean air swirls around you.", 0);
            autoIdentify(rogue.armor);
        }
    } else {

        // zombie gas
        if (cellHasTerrainFlag((pos){ *x, *y }, T_CAUSES_NAUSEA)
            && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
            && !(monst->bookkeepingFlags & MB_SUBMERGED)) {
            if (monst == &player) {
                rogue.disturbed = true;
            }
            if (canDirectlySeeMonster(monst) && !(monst->status[STATUS_NAUSEOUS])) {
                if (monst->creatureState == MONSTER_SLEEPING) {
                    monst->creatureState = MONSTER_TRACKING_SCENT;
                }
                flashMonster(monst, &brown, 100);
                monsterName(buf, monst, true);
                sprintf(buf2, "%s choke%s and gag%s on the overpowering stench of decay.", buf,
                        (monst == &player ? "": "s"), (monst == &player ? "": "s"));
                message(buf2, 0);
            }
            monst->status[STATUS_NAUSEOUS] = monst->maxStatus[STATUS_NAUSEOUS] = max(monst->status[STATUS_NAUSEOUS], 20);
        }

        // confusion gas
        if (cellHasTerrainFlag((pos){ *x, *y }, T_CAUSES_CONFUSION) && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))) {
            if (monst == &player) {
                rogue.disturbed = true;
            }
            if (canDirectlySeeMonster(monst) && !(monst->status[STATUS_CONFUSED])) {
                if (monst->creatureState == MONSTER_SLEEPING) {
                    monst->creatureState = MONSTER_TRACKING_SCENT;
                }
                flashMonster(monst, &confusionGasColor, 100);
                monsterName(buf, monst, true);
                sprintf(buf2, "%s %s very confused!", buf, (monst == &player ? "feel": "looks"));
                message(buf2, 0);
            }
            monst->status[STATUS_CONFUSED] = monst->maxStatus[STATUS_CONFUSED] = max(monst->status[STATUS_CONFUSED], 25);
        }

        // paralysis gas
        if (cellHasTerrainFlag((pos){ *x, *y }, T_CAUSES_PARALYSIS)
            && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
            && !(monst->bookkeepingFlags & MB_SUBMERGED)) {

            if (canDirectlySeeMonster(monst) && !monst->status[STATUS_PARALYZED]) {
                flashMonster(monst, &pink, 100);
                monsterName(buf, monst, true);
                sprintf(buf2, "%s %s paralyzed!", buf, (monst == &player ? "are": "is"));
                message(buf2, (monst == &player) ? REQUIRE_ACKNOWLEDGMENT : 0);
            }
            monst->status[STATUS_PARALYZED] = monst->maxStatus[STATUS_PARALYZED] = max(monst->status[STATUS_PARALYZED], 20);
            if (monst == &player) {
                rogue.disturbed = true;
            }
        }
    }

    // poisonous lichen
    if (cellHasTerrainFlag((pos){ *x, *y }, T_CAUSES_POISON)
        && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
        && !monst->status[STATUS_LEVITATING]) {

        if (monst == &player && !player.status[STATUS_POISONED]) {
            rogue.disturbed = true;
        }
        if (canDirectlySeeMonster(monst) && !(monst->status[STATUS_POISONED])) {
            if (monst->creatureState == MONSTER_SLEEPING) {
                monst->creatureState = MONSTER_TRACKING_SCENT;
            }
            flashMonster(monst, &green, 100);
            monsterName(buf, monst, true);
            sprintf(buf2, "the lichen's grasping tendrils poison %s.", buf);
            messageWithColor(buf2, messageColorFromVictim(monst), 0);
        }
        damage = max(0, 5 - monst->status[STATUS_POISONED]);
        addPoison(monst, damage, 0); // Lichen doesn't increase poison concentration above 1.
    }

    // fire
    if (cellHasTerrainFlag((pos){ *x, *y }, T_IS_FIRE)) {
        exposeCreatureToFire(monst);
    } else if (cellHasTerrainFlag((pos){ *x, *y }, T_IS_FLAMMABLE)
            // We should only expose to fire if it is flammable and not on fire. However, when
            // gas burns, it only sets the volume to 0 and doesn't clear the layer (for visual
            // reasons). This can cause crashes if the fire tile fails to spawn, so we also exclude it.
               && !(pmap[*x][*y].layers[GAS] != NOTHING && pmap[*x][*y].volume == 0)
               && !cellHasTerrainFlag((pos){ *x, *y }, T_IS_FIRE)
               && monst->status[STATUS_BURNING]
               && !(monst->bookkeepingFlags & (MB_SUBMERGED | MB_IS_FALLING))) {
        exposeTileToFire(*x, *y, true);
    }

    // keys
    if (cellHasTMFlag((pos){ *x, *y }, TM_PROMOTES_WITH_KEY) && (theItem = keyOnTileAt((pos){ *x, *y }))) {
        useKeyAt(theItem, *x, *y);
    }
}

static void applyGradualTileEffectsToCreature(creature *monst, short ticks) {
    short itemCandidates, randItemIndex;
    short x = monst->loc.x, y = monst->loc.y, damage;
    char buf[COLS * 5], buf2[COLS * 3];
    item *theItem;
    enum dungeonLayers layer;

    if (!(monst->status[STATUS_LEVITATING])
        && cellHasTerrainFlag((pos){ x, y }, T_IS_DEEP_WATER)
        && !cellHasTerrainFlag((pos){ x, y }, (T_ENTANGLES | T_OBSTRUCTS_PASSABILITY))
        && !(monst->info.flags & MONST_IMMUNE_TO_WATER)) {
        if (monst == &player) {
            if (!(pmap[x][y].flags & HAS_ITEM) && rand_percent(ticks * 50 / 100)) {
                itemCandidates = numberOfMatchingPackItems(ALL_ITEMS, 0, (ITEM_EQUIPPED), false);
                if (itemCandidates) {
                    randItemIndex = rand_range(1, itemCandidates);
                    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
                        if (!(theItem->flags & (ITEM_EQUIPPED))) {
                            if (randItemIndex == 1) {
                                break;
                            } else {
                                randItemIndex--;
                            }
                        }
                    }
                    theItem = dropItem(theItem);
                    if (theItem) {
                        itemName(theItem, buf2, false, true, NULL);
                        sprintf(buf, "%s float%s away in the current!",
                                buf2,
                                (theItem->quantity == 1 ? "s" : ""));
                        messageWithColor(buf, &itemMessageColor, 0);
                    }
                }
            }
        } else if (monst->carriedItem && !(pmap[x][y].flags & HAS_ITEM) && rand_percent(ticks * 50 / 100)) { // it's a monster with an item
            makeMonsterDropItem(monst);
        }
    }

    if (cellHasTerrainFlag((pos){ x, y }, T_CAUSES_DAMAGE)
        && !(monst->info.flags & (MONST_INANIMATE | MONST_INVULNERABLE))
        && !(monst->bookkeepingFlags & MB_SUBMERGED)) {

        damage = (monst->info.maxHP / 15) * ticks / 100;
        damage = max(1, damage);
        for (layer = 0; layer < NUMBER_TERRAIN_LAYERS && !(tileCatalog[pmap[x][y].layers[layer]].flags & T_CAUSES_DAMAGE); layer++);
        if (monst == &player) {
            if (rogue.armor && (rogue.armor->flags & ITEM_RUNIC) && rogue.armor->enchant2 == A_RESPIRATION) {
                if (!(rogue.armor->flags & ITEM_RUNIC_IDENTIFIED)) {
                    message("Your armor trembles and a pocket of clean air swirls around you.", 0);
                    autoIdentify(rogue.armor);
                }
            } else {
                rogue.disturbed = true;
                messageWithColor(tileCatalog[pmap[x][y].layers[layer]].flavorText, &badMessageColor, 0);
                if (inflictDamage(NULL, &player, damage, tileCatalog[pmap[x][y].layers[layer]].backColor, true)) {
                    killCreature(&player, false);
                    sprintf(buf, "Killed by %s", tileCatalog[pmap[x][y].layers[layer]].description);
                    gameOver(buf, true);
                    return;
                }
            }
        } else { // it's a monster
            if (monst->creatureState == MONSTER_SLEEPING) {
                monst->creatureState = MONSTER_TRACKING_SCENT;
            }
            if (inflictDamage(NULL, monst, damage, tileCatalog[pmap[x][y].layers[layer]].backColor, true)) {
                if (canSeeMonster(monst)) {
                    monsterName(buf, monst, true);
                    sprintf(buf2, "%s dies.", buf);
                    messageWithColor(buf2, messageColorFromVictim(monst), 0);
                }
                killCreature(monst, false);
                refreshDungeonCell((pos){ x, y });
                return;
            }
        }
    }

    if (cellHasTerrainFlag((pos){ x, y }, T_CAUSES_HEALING)
        && !(monst->info.flags & MONST_INANIMATE)
        && !(monst->bookkeepingFlags & MB_SUBMERGED)) {

        damage = (monst->info.maxHP / 15) * ticks / 100;
        damage = max(1, damage);
        if (monst->currentHP < monst->info.maxHP) {
            monst->currentHP = min(monst->currentHP + damage, monst->info.maxHP);
            if (monst == &player) {
                messageWithColor("you feel much better.", &goodMessageColor, 0);
            }
        }
    }
}

void updateClairvoyance() {
    short i, j, clairvoyanceRadius, dx, dy;
    boolean cursed;
    unsigned long cFlags;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {

            pmap[i][j].flags &= ~WAS_CLAIRVOYANT_VISIBLE;

            if (pmap[i][j].flags & CLAIRVOYANT_VISIBLE) {
                pmap[i][j].flags |= WAS_CLAIRVOYANT_VISIBLE;
            }

            pmap[i][j].flags &= ~(CLAIRVOYANT_VISIBLE | CLAIRVOYANT_DARKENED);
        }
    }

    cursed = (rogue.clairvoyance < 0);
    if (cursed) {
        clairvoyanceRadius = (rogue.clairvoyance - 1) * -1;
        cFlags = CLAIRVOYANT_DARKENED;
    } else {
        clairvoyanceRadius = (rogue.clairvoyance > 0) ? rogue.clairvoyance + 1 : 0;
        cFlags = CLAIRVOYANT_VISIBLE | DISCOVERED;
    }

    for (i = max(0, player.loc.x - clairvoyanceRadius); i < min(DCOLS, player.loc.x + clairvoyanceRadius + 1); i++) {
        for (j = max(0, player.loc.y - clairvoyanceRadius); j < min(DROWS, player.loc.y + clairvoyanceRadius + 1); j++) {

            dx = (player.loc.x - i);
            dy = (player.loc.y - j);

            if (dx*dx + dy*dy < clairvoyanceRadius*clairvoyanceRadius + clairvoyanceRadius
                && (pmap[i][j].layers[DUNGEON] != GRANITE || pmap[i][j].flags & DISCOVERED)) {

                if (cFlags & DISCOVERED) {
                    discoverCell(i, j);
                }
                pmap[i][j].flags |= cFlags;
                if (!(pmap[i][j].flags & HAS_PLAYER) && !cursed) {
                    pmap[i][j].flags &= ~STABLE_MEMORY;
                }
            }
        }
    }
}

static void updateTelepathy() {
    short i, j;
    boolean grid[DCOLS][DROWS];

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            pmap[i][j].flags &= ~WAS_TELEPATHIC_VISIBLE;
            if (pmap[i][j].flags & TELEPATHIC_VISIBLE) {
                pmap[i][j].flags |= WAS_TELEPATHIC_VISIBLE;
            }
            pmap[i][j].flags &= ~(TELEPATHIC_VISIBLE);
        }
    }

    zeroOutGrid(grid);
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monsterRevealed(monst)) {
            getFOVMask(grid, monst->loc.x, monst->loc.y, 2 * FP_FACTOR, T_OBSTRUCTS_VISION, 0, false);
            pmapAt(monst->loc)->flags |= TELEPATHIC_VISIBLE;
            discoverCell(monst->loc.x, monst->loc.y);
        }
    }
    for (creatureIterator it = iterateCreatures(dormantMonsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monsterRevealed(monst)) {
            getFOVMask(grid, monst->loc.x, monst->loc.y, 2 * FP_FACTOR, T_OBSTRUCTS_VISION, 0, false);
            pmapAt(monst->loc)->flags |= TELEPATHIC_VISIBLE;
            discoverCell(monst->loc.x, monst->loc.y);
        }
    }
    for (i = 0; i < DCOLS; i++) {
        for (j = 0; j < DROWS; j++) {
            if (grid[i][j]) {
                pmap[i][j].flags |= TELEPATHIC_VISIBLE;
                discoverCell(i, j);
            }
        }
    }
}

short scentDistance(short x1, short y1, short x2, short y2) {
    if (abs(x1 - x2) > abs(y1 - y2)) {
        return 2 * abs(x1 - x2) + abs(y1 - y2);
    } else {
        return abs(x1 - x2) + 2 * abs(y1 - y2);
    }
}

static void updateScent() {
    short i, j;
    char grid[DCOLS][DROWS];

    zeroOutGrid(grid);

    getFOVMask(grid, player.loc.x, player.loc.y, DCOLS * FP_FACTOR, T_OBSTRUCTS_SCENT, 0, false);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (grid[i][j]) {
                addScentToCell(i, j, scentDistance(player.loc.x, player.loc.y, i, j));
            }
        }
    }
    addScentToCell(player.loc.x, player.loc.y, 0);
}

short armorStealthAdjustment(item *theArmor) {
    if (!theArmor
        || !(theArmor->category & ARMOR)) {

        return 0;
    }
    return max(0, armorTable[theArmor->kind].strengthRequired - 12);
}

short currentStealthRange() {
    // Default value of 14 in the light.
    short stealthRange = 14;

    if (player.status[STATUS_INVISIBLE]) {
        stealthRange = 1; // Invisibility means stealth range of 1, no matter what.
    } else {
        if (playerInDarkness()) {
            // In darkness, halve, rounded down.
            stealthRange = stealthRange / 2;
        }
        if (pmapAt(player.loc)->flags & IS_IN_SHADOW) {
            // When not standing in a lit area, halve, rounded down (stacks with darkness halving).
            stealthRange = stealthRange / 2;
        }

        // Add 1 for each point of your armor's natural (unenchanted) strength requirement above 12.
        stealthRange += armorStealthAdjustment(rogue.armor);

        // Halve (rounded up) if you just rested.
        if (rogue.justRested) {
            stealthRange = (stealthRange + 1) / 2;
        }

        if (player.status[STATUS_AGGRAVATING] > 0) {
            stealthRange += player.status[STATUS_AGGRAVATING];
        }

        // Subtract your bonuses from rings of stealth.
        // (Cursed rings of stealth will end up adding here.)
        stealthRange -= rogue.stealthBonus;

        // Can't go below 2 unless you just rested.
        if (stealthRange < 2 && !rogue.justRested) {
            stealthRange = 2;
        } else if (stealthRange < 1) { // Can't go below 1, ever.
            stealthRange = 1;
        }
    }
    return stealthRange;
}

void demoteVisibility() {
    short i, j;

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            pmap[i][j].flags &= ~WAS_VISIBLE;
            if (pmap[i][j].flags & VISIBLE) {
                pmap[i][j].flags &= ~VISIBLE;
                pmap[i][j].flags |= WAS_VISIBLE;
            }
        }
    }
}

void discoverCell(const short x, const short y) {
    pmap[x][y].flags &= ~STABLE_MEMORY;
    if (!(pmap[x][y].flags & DISCOVERED)) {
        pmap[x][y].flags |= DISCOVERED;
        if (!cellHasTerrainFlag((pos){ x, y }, T_PATHING_BLOCKER)) {
            rogue.xpxpThisTurn++;
        }
    }
}

void updateVision(boolean refreshDisplay) {
    short i, j;
    char grid[DCOLS][DROWS];
    item *theItem;

    demoteVisibility();
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            pmap[i][j].flags &= ~IN_FIELD_OF_VIEW;
        }
    }

    // Calculate player's field of view (distinct from what is visible, as lighting hasn't been done yet).
    zeroOutGrid(grid);
    getFOVMask(grid, player.loc.x, player.loc.y, (DCOLS + DROWS) * FP_FACTOR, (T_OBSTRUCTS_VISION), 0, false);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (grid[i][j]) {
                pmap[i][j].flags |= IN_FIELD_OF_VIEW;
            }
        }
    }
    pmapAt(player.loc)->flags |= IN_FIELD_OF_VIEW | VISIBLE;

    if (rogue.clairvoyance < 0) {
        discoverCell(player.loc.x, player.loc.y);
    }

    if (rogue.clairvoyance != 0) {
        updateClairvoyance();
    }

    updateTelepathy();
    updateLighting();
    updateFieldOfViewDisplay(true, refreshDisplay);

    //  for (i=0; i<DCOLS; i++) {
    //      for (j=0; j<DROWS; j++) {
    //          if (pmap[i][j].flags & VISIBLE) {
    //              plotCharWithColor(' ', mapToWindow((pos){ i, j }), &yellow, &yellow);
    //          } else if (pmap[i][j].flags & IN_FIELD_OF_VIEW) {
    //              plotCharWithColor(' ', mapToWindow((pos){ i, j }), &blue, &blue);
    //          }
    //      }
    //  }
    //  displayMoreSign();

    if (player.status[STATUS_HALLUCINATING] > 0) {
        for (theItem = floorItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
            if ((pmapAt(theItem->loc)->flags & DISCOVERED) && refreshDisplay) {
                refreshDungeonCell(theItem->loc);
            }
        }
        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            if ((pmapAt(monst->loc)->flags & DISCOVERED) && refreshDisplay) {
                refreshDungeonCell(monst->loc);
            }
        }
    }
}

static void checkNutrition() {
    item *theItem;
    char buf[DCOLS*3], foodWarning[DCOLS*3];

    if (numberOfMatchingPackItems(FOOD, 0, 0, false) == 0) {
        sprintf(foodWarning, " and have no food");
    } else {
        foodWarning[0] = '\0';
    }

    if (player.status[STATUS_NUTRITION] == HUNGER_THRESHOLD) {
        player.status[STATUS_NUTRITION]--;
        sprintf(buf, "you are hungry%s.", foodWarning);
        message(buf, foodWarning[0] ? REQUIRE_ACKNOWLEDGMENT : 0);
    } else if (player.status[STATUS_NUTRITION] == WEAK_THRESHOLD) {
        player.status[STATUS_NUTRITION]--;
        sprintf(buf, "you feel weak with hunger%s.", foodWarning);
        message(buf, REQUIRE_ACKNOWLEDGMENT);
    } else if (player.status[STATUS_NUTRITION] == FAINT_THRESHOLD) {
        player.status[STATUS_NUTRITION]--;
        sprintf(buf, "you feel faint with hunger%s.", foodWarning);
        message(buf, REQUIRE_ACKNOWLEDGMENT);
    } else if (player.status[STATUS_NUTRITION] <= 1) {
        // Force the player to eat something if he has it
        for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
            if (theItem->category == FOOD) {
                sprintf(buf, "unable to control your hunger, you eat a %s.", (theItem->kind == FRUIT ? "mango" : "ration of food"));
                messageWithColor(buf, &itemMessageColor, REQUIRE_ACKNOWLEDGMENT);
                confirmMessages();
                eat(theItem, false);
                playerTurnEnded();
                break;
            }
        }
    }

    if (player.status[STATUS_NUTRITION] == 1) { // Didn't manage to eat any food above.
        player.status[STATUS_NUTRITION] = 0;    // So the status bar changes in time for the message:
        message("you are starving to death!", REQUIRE_ACKNOWLEDGMENT);
    }
}

void burnItem(item *theItem) {
    short x, y;
    char buf1[COLS * 3], buf2[COLS * 3];
    itemName(theItem, buf1, false, true, NULL);
    sprintf(buf2, "%s burn%s up!",
            buf1,
            theItem->quantity == 1 ? "s" : "");
    x = theItem->loc.x;
    y = theItem->loc.y;
    removeItemFromChain(theItem, floorItems);
    deleteItem(theItem);
    pmap[x][y].flags &= ~(HAS_ITEM | ITEM_DETECTED);
    if (pmap[x][y].flags & (ANY_KIND_OF_VISIBLE | DISCOVERED | ITEM_DETECTED)) {
        refreshDungeonCell((pos){ x, y });
    }
    if (playerCanSee(x, y)) {
        messageWithColor(buf2, &itemMessageColor, 0);
    }
    spawnDungeonFeature(x, y, &(dungeonFeatureCatalog[DF_ITEM_FIRE]), true, false);
}

static void flashCreatureAlert(creature *monst, char msg[200], const color *foreColor, const color *backColor) {
    short x, y;
    if (monst->loc.y > DROWS / 2) {
        y = mapToWindowY(monst->loc.y - 2);
    } else {
        y = mapToWindowY(monst->loc.y + 2);
    }
    x = mapToWindowX(monst->loc.x - strLenWithoutEscapes(msg) / 2);
    if (x > COLS - strLenWithoutEscapes(msg)) {
        x = COLS - strLenWithoutEscapes(msg);
    }
    flashMessage(msg, x, y, (rogue.playbackMode ? 100 : 1000), foreColor, backColor);
    rogue.disturbed = true;
    rogue.autoPlayingLevel = false;
}

static void handleHealthAlerts() {
    short i, currentPercent, previousPercent,
    thresholds[] = {5, 10, 25, 40},
    pThresholds[] = {100, 90, 50};
    char buf[DCOLS];

    const short healthThresholdsCount = 4,
    poisonThresholdsCount = 3;

    assureCosmeticRNG;

    currentPercent = player.currentHP * 100 / player.info.maxHP;
    previousPercent = player.previousHealthPoints * 100 / player.info.maxHP;

    if (currentPercent < previousPercent && !rogue.gameHasEnded) {
        for (i=0; i < healthThresholdsCount; i++) {
            if (currentPercent < thresholds[i] && previousPercent >= thresholds[i]) {
                sprintf(buf, " <%i%% health ", thresholds[i]);
                flashCreatureAlert(&player, buf, &badMessageColor, &darkRed);
                break;
            }
        }
    }

    if (!rogue.gameHasEnded) {
        currentPercent = player.status[STATUS_POISONED] * player.poisonAmount * 100 / player.currentHP;

        if (currentPercent > rogue.previousPoisonPercent && !rogue.gameHasEnded) {
            for (i=0; i < poisonThresholdsCount; i++) {
                if (currentPercent > pThresholds[i] && rogue.previousPoisonPercent <= pThresholds[i]) {
                    if (currentPercent < 100) {
                        sprintf(buf, " >%i%% poisoned ", pThresholds[i]);
                    } else {
                        strcpy(buf, " Fatally poisoned ");
                    }
                    flashCreatureAlert(&player, buf, &yellow, &darkGreen);
                    break;
                }
            }
        }
        rogue.previousPoisonPercent = currentPercent;
    }

    restoreRNG;
}

/// @brief Add experience to the given monster. Allies gain experience when the player discovers new pathable tiles.
/// @param monst The ally that gains experience
static void addXPXPToAlly(creature *monst) {
    if (!(monst->creatureState == MONSTER_ALLY) || monst->info.flags & (MONST_INANIMATE | MONST_IMMOBILE)) {
        return;
    }

    monst->xpxp += rogue.xpxpThisTurn;

    // Telepathic bond
    if (!(monst->bookkeepingFlags & MB_TELEPATHICALLY_REVEALED) && monst->xpxp >= XPXP_NEEDED_FOR_TELEPATHIC_BOND) {

        monst->bookkeepingFlags |= MB_TELEPATHICALLY_REVEALED;
        updateVision(true);
        char theMonsterName[100], buf[200];
        monsterName(theMonsterName, monst, false);
        sprintf(buf, "you have developed a telepathic bond with your %s.", theMonsterName);
        messageWithColor(buf, &advancementMessageColor, 0);
    }

    // Companion feat
    if (!(rogue.featRecord[FEAT_COMPANION]) && monst->xpxp >= gameConst->companionFeatRequiredXP) {
        rogue.featRecord[FEAT_COMPANION] = true;
    }
}

/// @brief Allies gain experience if they are within 1 depth level of the player
static void handleXPXP() {

    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        addXPXPToAlly(monst);
    }
    if (rogue.depthLevel > 1) {
        for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel - 2].monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            addXPXPToAlly(monst);
        }
    }
    if (rogue.depthLevel < gameConst->deepestLevel) {
        for (creatureIterator it = iterateCreatures(&levels[rogue.depthLevel].monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            addXPXPToAlly(monst);
        }
    }
    rogue.xpxpThisTurn = 0;
}

static void playerFalls() {
    short damage;
    short layer;

    if (cellHasTMFlag(player.loc, TM_IS_SECRET)
        && playerCanSee(player.loc.x, player.loc.y)) {

        discover(player.loc.x, player.loc.y);
    }

    monstersFall(); // Monsters must fall with the player rather than getting suspended on the previous level.
    updateFloorItems(); // Likewise, items should fall with the player rather than getting suspended above.

    layer = layerWithFlag(player.loc.x, player.loc.y, T_AUTO_DESCENT);
    if (layer >= 0) {
        message(tileCatalog[pmapAt(player.loc)->layers[layer]].flavorText, REQUIRE_ACKNOWLEDGMENT);
    } else if (layer == -1) {
        message("You plunge downward!", REQUIRE_ACKNOWLEDGMENT);
    }

    player.bookkeepingFlags &= ~(MB_IS_FALLING | MB_SEIZED | MB_SEIZING);
    rogue.disturbed = true;

    if (rogue.depthLevel < gameConst->deepestLevel) {
        rogue.depthLevel++;
        startLevel(rogue.depthLevel - 1, 0);
        damage = randClumpedRange(gameConst->fallDamageMin, gameConst->fallDamageMax, 2);
        boolean killed = false;
        if (terrainFlags(player.loc) & T_IS_DEEP_WATER) {
            messageWithColor("You fall into deep water, unharmed.", &badMessageColor, 0);
        } else {
            if (cellHasTMFlag(player.loc, TM_ALLOWS_SUBMERGING)) {
                damage /= 2; // falling into liquid (shallow water, bog, etc.) hurts less than hitting hard floor
            }
            messageWithColor("You are injured by the fall.", &badMessageColor, 0);
            if (inflictDamage(NULL, &player, damage, &red, false)) {
                killCreature(&player, false);
                gameOver("Killed by a fall", true);
                killed = true;
            }
        }
        if (!killed && rogue.depthLevel > rogue.deepestLevel) {
            rogue.deepestLevel = rogue.depthLevel;
        }
    } else {
        message("A strange force seizes you as you fall.", 0);
        teleport(&player, INVALID_POS, true);
    }
    createFlare(player.loc.x, player.loc.y, GENERIC_FLASH_LIGHT);
    animateFlares(rogue.flares, rogue.flareCount);
    rogue.flareCount = 0;
}



void activateMachine(short machineNumber) {
    short i, j, x, y, layer, sRows[DROWS], sCols[DCOLS], monsterCount, maxMonsters;

    fillSequentialList(sCols, DCOLS);
    shuffleList(sCols, DCOLS);
    fillSequentialList(sRows, DROWS);
    shuffleList(sRows, DROWS);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            x = sCols[i];
            y = sRows[j];
            if ((pmap[x][y].flags & IS_IN_MACHINE)
                && pmap[x][y].machineNumber == machineNumber
                && !(pmap[x][y].flags & IS_POWERED)
                && cellHasTMFlag((pos){ x, y }, TM_IS_WIRED)) {

                pmap[x][y].flags |= IS_POWERED;
                for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_IS_WIRED) {
                        promoteTile(x, y, layer, false);
                    }
                }
            }
        }
    }

    monsterCount = maxMonsters = 0;
    creature **activatedMonsterList = NULL;
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if (monst->machineHome == machineNumber
            && monst->spawnDepth == rogue.depthLevel
            && (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)) {

            monsterCount++;

            if (monsterCount > maxMonsters) {
                maxMonsters += 10;
                activatedMonsterList = realloc(activatedMonsterList, sizeof(creature *) * maxMonsters);
            }
            activatedMonsterList[monsterCount - 1] = monst;
        }
    }
    for (i=0; i<monsterCount; i++) {
        if (!(activatedMonsterList[i]->bookkeepingFlags & MB_IS_DYING)) {
            monstersTurn(activatedMonsterList[i]);
        }
    }

    if (activatedMonsterList) {
        free(activatedMonsterList);
    }
}

boolean circuitBreakersPreventActivation(short machineNumber) {
    short i, j;
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].machineNumber == machineNumber
                && cellHasTMFlag((pos){ i, j }, TM_IS_CIRCUIT_BREAKER)) {

                return true;
            }
        }
    }
    return false;
}

void promoteTile(short x, short y, enum dungeonLayers layer, boolean useFireDF) {
    short i, j;
    enum dungeonFeatureTypes DFType;
    const floorTileType *tile;

    tile = &(tileCatalog[pmap[x][y].layers[layer]]);

    DFType = (useFireDF ? tile->fireType : tile->promoteType);

    if ((tile->mechFlags & TM_VANISHES_UPON_PROMOTION)) {
        if (tileCatalog[pmap[x][y].layers[layer]].flags & T_PATHING_BLOCKER) {
            rogue.staleLoopMap = true;
        }
        pmap[x][y].layers[layer] = (layer == DUNGEON ? FLOOR : NOTHING); // even the dungeon layer implicitly has floor underneath it
        if (layer == GAS) {
            pmap[x][y].volume = 0;
        }
        refreshDungeonCell((pos){ x, y });
    }
    if (DFType) {
        spawnDungeonFeature(x, y, &dungeonFeatureCatalog[DFType], true, false);
    }

    if (!useFireDF && (tile->mechFlags & TM_IS_WIRED)
        && !(pmap[x][y].flags & IS_POWERED)
        && !circuitBreakersPreventActivation(pmap[x][y].machineNumber)) {
        // Send power through all cells in the same machine that are not IS_POWERED,
        // and on any such cell, promote each terrain layer that is T_IS_WIRED.
        // Note that machines need not be contiguous.
        pmap[x][y].flags |= IS_POWERED;
        activateMachine(pmap[x][y].machineNumber); // It lives!!!

        // Power fades from the map immediately after we finish.
        for (i=0; i<DCOLS; i++) {
            for (j=0; j<DROWS; j++) {
                pmap[i][j].flags &= ~IS_POWERED;
            }
        }
    }
}

boolean exposeTileToElectricity(short x, short y) {
    enum dungeonLayers layer;
    boolean promotedSomething = false;

    if (!cellHasTMFlag((pos){ x, y }, TM_PROMOTES_ON_ELECTRICITY)) {
        return false;
    }
    for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if (tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_PROMOTES_ON_ELECTRICITY) {
            promoteTile(x, y, layer, false);
            promotedSomething = true;
        }
    }
    return promotedSomething;
}

boolean exposeTileToFire(short x, short y, boolean alwaysIgnite) {
    enum dungeonLayers layer;
    short ignitionChance = 0, bestExtinguishingPriority = 1000, explosiveNeighborCount = 0;
    short newX, newY;
    enum directions dir;
    boolean fireIgnited = false, explosivePromotion = false;

    if (!cellHasTerrainFlag((pos){ x, y }, T_IS_FLAMMABLE) || pmap[x][y].exposedToFire >= 12) {
        return false;
    }

    pmap[x][y].exposedToFire++;

    // Pick the extinguishing layer with the best priority.
    for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if ((tileCatalog[pmap[x][y].layers[layer]].mechFlags & TM_EXTINGUISHES_FIRE)
            && tileCatalog[pmap[x][y].layers[layer]].drawPriority < bestExtinguishingPriority) {
            bestExtinguishingPriority = tileCatalog[pmap[x][y].layers[layer]].drawPriority;
        }
    }

    // Pick the fire type of the most flammable layer that is either gas or equal-or-better priority than the best extinguishing layer.
    for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
        if ((tileCatalog[pmap[x][y].layers[layer]].flags & T_IS_FLAMMABLE)
            && (layer == GAS || tileCatalog[pmap[x][y].layers[layer]].drawPriority <= bestExtinguishingPriority)
            && tileCatalog[pmap[x][y].layers[layer]].chanceToIgnite > ignitionChance) {
            ignitionChance = tileCatalog[pmap[x][y].layers[layer]].chanceToIgnite;
        }
    }

    if (alwaysIgnite || (ignitionChance && rand_percent(ignitionChance))) { // If it ignites...
        fireIgnited = true;

        // Count explosive neighbors.
        if (cellHasTMFlag((pos){ x, y }, TM_EXPLOSIVE_PROMOTE)) {
            for (dir = 0, explosiveNeighborCount = 0; dir < DIRECTION_COUNT; dir++) {
                newX = x + nbDirs[dir][0];
                newY = y + nbDirs[dir][1];
                if (coordinatesAreInMap(newX, newY)
                    && (cellHasTerrainFlag((pos){ newX, newY }, T_IS_FIRE | T_OBSTRUCTS_GAS) || cellHasTMFlag((pos){ newX, newY }, TM_EXPLOSIVE_PROMOTE))) {

                    explosiveNeighborCount++;
                }
            }
            if (explosiveNeighborCount >= 8) {
                explosivePromotion = true;
            }
        }

        // Flammable layers are consumed.
        for (layer=0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
            if (tileCatalog[pmap[x][y].layers[layer]].flags & T_IS_FLAMMABLE) {
                // pmap[x][y].layers[GAS] is not cleared here, which is a bug.
                // We preserve the layer anyways because this results in nicer gas burning behavior.
                if (layer == GAS) {
                    pmap[x][y].volume = 0; // Flammable gas burns its volume away.
                }
                promoteTile(x, y, layer, !explosivePromotion);
            }
        }
        refreshDungeonCell((pos){ x, y });
    }
    return fireIgnited;
}

// Only the gas layer can be volumetric.
static void updateVolumetricMedia() {
    short i, j, newX, newY, numSpaces;
    unsigned long highestNeighborVolume;
    unsigned long sum;
    enum tileType gasType;
    enum directions dir;
    unsigned short newGasVolume[DCOLS][DROWS];

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            newGasVolume[i][j] = 0;
        }
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (!cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_GAS)) {
                sum = pmap[i][j].volume;
                numSpaces = 1;
                highestNeighborVolume = pmap[i][j].volume;
                gasType = pmap[i][j].layers[GAS];
                for (dir=0; dir< DIRECTION_COUNT; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_GAS)) {

                        sum += pmap[newX][newY].volume;
                        numSpaces++;
                        if (pmap[newX][newY].volume > highestNeighborVolume) {
                            highestNeighborVolume = pmap[newX][newY].volume;
                            gasType = pmap[newX][newY].layers[GAS];
                        }
                    }
                }
                if (cellHasTerrainFlag((pos){ i, j }, T_AUTO_DESCENT)) { // if it's a chasm tile or trap door,
                    numSpaces++; // this will allow gas to escape from the level entirely
                }
                newGasVolume[i][j] += sum / max(1, numSpaces);
                if ((unsigned) rand_range(0, numSpaces - 1) < (sum % numSpaces)) {
                    newGasVolume[i][j]++; // stochastic rounding
                }
                if (pmap[i][j].layers[GAS] != gasType && newGasVolume[i][j] > 3) {
                    if (pmap[i][j].layers[GAS] != NOTHING) {
                        newGasVolume[i][j] = min(3, newGasVolume[i][j]); // otherwise interactions between gases are crazy
                    }
                    pmap[i][j].layers[GAS] = gasType;
                } else if (pmap[i][j].layers[GAS] && newGasVolume[i][j] < 1) {
                    pmap[i][j].layers[GAS] = NOTHING;
                    refreshDungeonCell((pos){ i, j });
                }
                if (pmap[i][j].volume > 0) {
                    if (tileCatalog[pmap[i][j].layers[GAS]].mechFlags & TM_GAS_DISSIPATES_QUICKLY) {
                        newGasVolume[i][j] -= (rand_percent(50) ? 1 : 0);
                    } else if (tileCatalog[pmap[i][j].layers[GAS]].mechFlags & TM_GAS_DISSIPATES) {
                        newGasVolume[i][j] -= (rand_percent(20) ? 1 : 0);
                    }
                }
            } else if (pmap[i][j].volume > 0) { // if has gas but can't hold gas,
                // disperse gas instantly into neighboring tiles that can hold gas
                numSpaces = 0;
                for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                    newX = i + nbDirs[dir][0];
                    newY = j + nbDirs[dir][1];
                    if (coordinatesAreInMap(newX, newY)
                        && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_GAS)) {

                        numSpaces++;
                    }
                }
                if (numSpaces > 0) {
                    for (dir = 0; dir < DIRECTION_COUNT; dir++) {
                        newX = i + nbDirs[dir][0];
                        newY = j + nbDirs[dir][1];
                        if (coordinatesAreInMap(newX, newY)
                            && !cellHasTerrainFlag((pos){ newX, newY }, T_OBSTRUCTS_GAS)) {

                            newGasVolume[newX][newY] += (pmap[i][j].volume / numSpaces);
                            if (pmap[i][j].volume / numSpaces) {
                                pmap[newX][newY].layers[GAS] = pmap[i][j].layers[GAS];
                            }
                        }
                    }
                }
                newGasVolume[i][j] = 0;
                pmap[i][j].layers[GAS] = NOTHING;
            }
        }
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (pmap[i][j].volume != newGasVolume[i][j]) {
                pmap[i][j].volume = newGasVolume[i][j];
                refreshDungeonCell((pos){ i, j });
            }
        }
    }
}

static void updateYendorWardenTracking() {
    short n;

    if (!rogue.yendorWarden) {
        return;
    }
    if (rogue.yendorWarden->depth == rogue.depthLevel) {
        return;
    }
    if (!(rogue.yendorWarden->bookkeepingFlags & MB_PREPLACED)) {
        levels[rogue.yendorWarden->depth - 1].mapStorage[rogue.yendorWarden->loc.x][rogue.yendorWarden->loc.y].flags &= ~HAS_MONSTER;
    }
    n = rogue.yendorWarden->depth - 1;

    // remove traversing monster from other level monster chain
    removeCreature(&levels[n].monsters, rogue.yendorWarden);

    if (rogue.yendorWarden->depth > rogue.depthLevel) {
        rogue.yendorWarden->depth = rogue.depthLevel + 1;
        n = rogue.yendorWarden->depth - 1;
        rogue.yendorWarden->bookkeepingFlags |= MB_APPROACHING_UPSTAIRS;
        rogue.yendorWarden->loc.x = levels[n].downStairsLoc.x;
        rogue.yendorWarden->loc.y = levels[n].downStairsLoc.y;
    } else {
        rogue.yendorWarden->depth = rogue.depthLevel - 1;
        n = rogue.yendorWarden->depth - 1;
        rogue.yendorWarden->bookkeepingFlags |= MB_APPROACHING_DOWNSTAIRS;
        rogue.yendorWarden->loc.x = levels[n].upStairsLoc.x;
        rogue.yendorWarden->loc.y = levels[n].upStairsLoc.y;
    }
    prependCreature(&levels[rogue.yendorWarden->depth - 1].monsters, rogue.yendorWarden);
    rogue.yendorWarden->bookkeepingFlags |= MB_PREPLACED;
    rogue.yendorWarden->status[STATUS_ENTERS_LEVEL_IN] = 50;
}

// Monsters who are over chasms or other descent tiles won't fall until this is called.
// This is to avoid having the monster chain change unpredictably in the middle of a turn.
void monstersFall() {
    short x, y;
    char buf[DCOLS], buf2[DCOLS];

    // monsters plunge into chasms at the end of the turn
    for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
        creature *monst = nextCreature(&it);
        if ((monst->bookkeepingFlags & MB_IS_FALLING) || monsterShouldFall(monst)) {
            monst->bookkeepingFlags |= MB_IS_FALLING;

            x = monst->loc.x;
            y = monst->loc.y;

            if (canSeeMonster(monst)) {
                monsterName(buf, monst, true);
                sprintf(buf2, "%s plunges out of sight!", buf);
                messageWithColor(buf2, messageColorFromVictim(monst), 0);
            }

            if (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION) {
                // Guardians and mirrored totems never survive the fall. If they did, they might block the level below.
                killCreature(monst, false);
            } else if (!inflictDamage(NULL, monst, randClumpedRange(6, 12, 2), &red, false)) {
                demoteMonsterFromLeadership(monst);

                monst->status[STATUS_ENTRANCED] = 0;
                monst->bookkeepingFlags |= MB_PREPLACED;
                monst->bookkeepingFlags &= ~(MB_IS_FALLING | MB_SEIZED | MB_SEIZING);
                monst->targetCorpseLoc = INVALID_POS;

                // remove from monster chain
                removeCreature(monsters, monst);

                // add to next level's chain
                prependCreature(&levels[rogue.depthLevel-1 + 1].monsters, monst);

                monst->depth = rogue.depthLevel + 1;

                if (monst == rogue.yendorWarden) {
                    updateYendorWardenTracking();
                }
            } else {
                killCreature(monst, false);
            }

            pmap[x][y].flags &= ~HAS_MONSTER;
            refreshDungeonCell((pos){ x, y });
        }
    }
}

void updateEnvironment() {
    short i, j, direction, newX, newY, promotions[DCOLS][DROWS];
    long promoteChance;
    enum dungeonLayers layer;
    const floorTileType *tile;
    boolean isVolumetricGas = false;

    monstersFall();

    // reset exposedToFire
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            pmap[i][j].exposedToFire = 0;
        }
    }

    // update gases twice
    for (i=0; i<DCOLS && !isVolumetricGas; i++) {
        for (j=0; j<DROWS && !isVolumetricGas; j++) {
            if (!isVolumetricGas && pmap[i][j].layers[GAS]) {
                isVolumetricGas = true;
            }
        }
    }
    if (isVolumetricGas) {
        updateVolumetricMedia();
        updateVolumetricMedia();
    }

    // Do random tile promotions in two passes to keep generations distinct.
    // First pass, make a note of each terrain layer at each coordinate that is going to promote:
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            promotions[i][j] = 0;
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                tile = &(tileCatalog[pmap[i][j].layers[layer]]);
                if (tile->promoteChance < 0) {
                    promoteChance = 0;
                    for (direction = 0; direction < 4; direction++) {
                        if (coordinatesAreInMap(i + nbDirs[direction][0], j + nbDirs[direction][1])
                            && !cellHasTerrainFlag((pos){ i + nbDirs[direction][0], j + nbDirs[direction][1] }, T_OBSTRUCTS_PASSABILITY)
                            && pmap[i + nbDirs[direction][0]][j + nbDirs[direction][1]].layers[layer] != pmap[i][j].layers[layer]
                            && !(pmap[i][j].flags & CAUGHT_FIRE_THIS_TURN)) {
                            promoteChance += -1 * tile->promoteChance;
                        }
                    }
                } else {
                    promoteChance = tile->promoteChance;
                }
                if (promoteChance
                    && !(pmap[i][j].flags & CAUGHT_FIRE_THIS_TURN)
                    && rand_range(0, 10000) < promoteChance) {
                    promotions[i][j] |= Fl(layer);
                    //promoteTile(i, j, layer, false);
                }
            }
        }
    }
    // Second pass, do the promotions:
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                if ((promotions[i][j] & Fl(layer))) {
                    //&& (tileCatalog[pmap[i][j].layers[layer]].promoteChance != 0)){
                    // make sure that it's still a promotable layer
                    promoteTile(i, j, layer, false);
                }
            }
        }
    }

    // Bookkeeping for fire, pressure plates and key-activated tiles.
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            pmap[i][j].flags &= ~(CAUGHT_FIRE_THIS_TURN);
            if (!(pmap[i][j].flags & (HAS_PLAYER | HAS_MONSTER | HAS_ITEM))
                && (pmap[i][j].flags & PRESSURE_PLATE_DEPRESSED)) {

                pmap[i][j].flags &= ~PRESSURE_PLATE_DEPRESSED;
            }
            if (cellHasTMFlag((pos){ i, j }, TM_PROMOTES_WITHOUT_KEY) && !keyOnTileAt((pos){ i, j })) {
                for (layer = 0; layer < NUMBER_TERRAIN_LAYERS; layer++) {
                    if (tileCatalog[pmap[i][j].layers[layer]].mechFlags & TM_PROMOTES_WITHOUT_KEY) {
                        promoteTile(i, j, layer, false);
                    }
                }
            }
        }
    }

    // Update fire.
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_IS_FIRE) && !(pmap[i][j].flags & CAUGHT_FIRE_THIS_TURN)) {
                exposeTileToFire(i, j, false);
                for (direction=0; direction<4; direction++) {
                    newX = i + nbDirs[direction][0];
                    newY = j + nbDirs[direction][1];
                    if (coordinatesAreInMap(newX, newY)) {
                        exposeTileToFire(newX, newY, false);
                    }
                }
            }
        }
    }

    // Terrain that affects items and vice versa
    updateFloorItems();
}

void updateAllySafetyMap() {
    short i, j;
    short **playerCostMap, **monsterCostMap;

    rogue.updatedAllySafetyMapThisTurn = true;

    playerCostMap = allocGrid();
    monsterCostMap = allocGrid();

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            allySafetyMap[i][j] = 30000;

            playerCostMap[i][j] = monsterCostMap[i][j] = 1;

            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && (!cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY))) {

                playerCostMap[i][j] = monsterCostMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER & ~T_OBSTRUCTS_PASSABILITY)) {
                playerCostMap[i][j] = monsterCostMap[i][j] = PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_SACRED)) {
                playerCostMap[i][j] = 1;
                monsterCostMap[i][j] = PDS_FORBIDDEN;
            } else if ((pmap[i][j].flags & HAS_MONSTER) && monstersAreEnemies(&player, monsterAtLoc((pos){ i, j }))) {
                playerCostMap[i][j] = 1;
                monsterCostMap[i][j] = PDS_FORBIDDEN;
                allySafetyMap[i][j] = 0;
            }
        }
    }

    playerCostMap[player.loc.x][player.loc.y] = PDS_FORBIDDEN;
    monsterCostMap[player.loc.x][player.loc.y] = PDS_FORBIDDEN;

    dijkstraScan(allySafetyMap, playerCostMap, false);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (monsterCostMap[i][j] < 0) {
                continue;
            }

            if (allySafetyMap[i][j] == 30000) {
                allySafetyMap[i][j] = 150;
            }

            allySafetyMap[i][j] = 50 * allySafetyMap[i][j] / (50 + allySafetyMap[i][j]);

            allySafetyMap[i][j] *= -3;

            if (pmap[i][j].flags & IN_LOOP) {
                allySafetyMap[i][j] -= 10;
            }
        }
    }
    dijkstraScan(allySafetyMap, monsterCostMap, false);

    freeGrid(playerCostMap);
    freeGrid(monsterCostMap);
}

static void resetDistanceCellInGrid(short **grid, short x, short y) {
    enum directions dir;
    short newX, newY;
    for (dir = 0; dir < 4; dir++) {
        newX = x + nbDirs[dir][0];
        newY = y + nbDirs[dir][1];
        if (coordinatesAreInMap(newX, newY)
            && grid[x][y] > grid[newX][newY] + 1) {

            grid[x][y] = grid[newX][newY] + 1;
        }
    }
}

void updateSafetyMap() {
    short i, j;
    short **playerCostMap, **monsterCostMap;
    creature *monst;

    rogue.updatedSafetyMapThisTurn = true;

    playerCostMap = allocGrid();
    monsterCostMap = allocGrid();

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            safetyMap[i][j] = 30000;

            playerCostMap[i][j] = monsterCostMap[i][j] = 1; // prophylactic

            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && (!cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY))) {

                playerCostMap[i][j] = monsterCostMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_SACRED)) {
                playerCostMap[i][j] = 1;
                monsterCostMap[i][j] = PDS_FORBIDDEN;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_LAVA_INSTA_DEATH)) {
                monsterCostMap[i][j] = PDS_FORBIDDEN;
                if (player.status[STATUS_LEVITATING] || !player.status[STATUS_IMMUNE_TO_FIRE]) {
                    playerCostMap[i][j] = 1;
                } else {
                    playerCostMap[i][j] = PDS_FORBIDDEN;
                }
            } else {
                if (pmap[i][j].flags & HAS_MONSTER) {
                    monst = monsterAtLoc((pos){ i, j });
                    if ((monst->creatureState == MONSTER_SLEEPING
                         || monst->turnsSpentStationary > 2
                         || (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)
                         || monst->creatureState == MONSTER_ALLY)
                        && monst->creatureState != MONSTER_FLEEING) {

                        playerCostMap[i][j] = 1;
                        monsterCostMap[i][j] = PDS_FORBIDDEN;
                        continue;
                    }
                }

                if (cellHasTerrainFlag((pos){ i, j }, (T_AUTO_DESCENT | T_IS_DF_TRAP))) {
                    monsterCostMap[i][j] = PDS_FORBIDDEN;
                    if (player.status[STATUS_LEVITATING]) {
                        playerCostMap[i][j] = 1;
                    } else {
                        playerCostMap[i][j] = PDS_FORBIDDEN;
                    }
                } else if (cellHasTerrainFlag((pos){ i, j }, T_IS_FIRE)) {
                    monsterCostMap[i][j] = PDS_FORBIDDEN;
                    if (player.status[STATUS_IMMUNE_TO_FIRE]) {
                        playerCostMap[i][j] = 1;
                    } else {
                        playerCostMap[i][j] = PDS_FORBIDDEN;
                    }
                } else if (cellHasTerrainFlag((pos){ i, j }, (T_IS_DEEP_WATER | T_SPONTANEOUSLY_IGNITES))) {
                    if (player.status[STATUS_LEVITATING]) {
                        playerCostMap[i][j] = 1;
                    } else {
                        playerCostMap[i][j] = 5;
                    }
                    monsterCostMap[i][j] = 5;
                } else if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                           && cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY)
                           && !(pmap[i][j].flags & IN_FIELD_OF_VIEW)) {
                    // Secret door that the player can't currently see
                    playerCostMap[i][j] = 100;
                    monsterCostMap[i][j] = 1;
                } else {
                    playerCostMap[i][j] = monsterCostMap[i][j] = 1;
                }
            }
        }
    }

    safetyMap[player.loc.x][player.loc.y] = 0;
    playerCostMap[player.loc.x][player.loc.y] = 1;
    monsterCostMap[player.loc.x][player.loc.y] = PDS_FORBIDDEN;

    playerCostMap[rogue.upLoc.x][rogue.upLoc.y] = PDS_FORBIDDEN;
    monsterCostMap[rogue.upLoc.x][rogue.upLoc.y] = PDS_FORBIDDEN;
    playerCostMap[rogue.downLoc.x][rogue.downLoc.y] = PDS_FORBIDDEN;
    monsterCostMap[rogue.downLoc.x][rogue.downLoc.y] = PDS_FORBIDDEN;

    dijkstraScan(safetyMap, playerCostMap, false);

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) && !(discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY)
                && !(pmap[i][j].flags & IN_FIELD_OF_VIEW)) {

                // Secret doors that the player can't see are not particularly safe themselves;
                // the areas behind them are.
                resetDistanceCellInGrid(safetyMap, i, j);
            }
        }
    }

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (monsterCostMap[i][j] < 0) {
                continue;
            }

            if (safetyMap[i][j] == 30000) {
                safetyMap[i][j] = 150;
            }

            safetyMap[i][j] = 50 * safetyMap[i][j] / (50 + safetyMap[i][j]);

            safetyMap[i][j] *= -3;

            if (pmap[i][j].flags & IN_LOOP) {
                safetyMap[i][j] -= 10;
            }
        }
    }
    dijkstraScan(safetyMap, monsterCostMap, false);
    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            if (monsterCostMap[i][j] < 0) {
                safetyMap[i][j] = 30000;
            }
        }
    }
    freeGrid(playerCostMap);
    freeGrid(monsterCostMap);
}

void updateSafeTerrainMap() {
    short i, j;
    short **costMap;
    creature *monst;

    rogue.updatedMapToSafeTerrainThisTurn = true;
    costMap = allocGrid();

    for (i=0; i<DCOLS; i++) {
        for (j=0; j<DROWS; j++) {
            monst = monsterAtLoc((pos){ i, j });
            if (cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_PASSABILITY)
                && (!cellHasTMFlag((pos){ i, j }, TM_IS_SECRET) || (discoveredTerrainFlagsAtLoc((pos){ i, j }) & T_OBSTRUCTS_PASSABILITY))) {

                costMap[i][j] = cellHasTerrainFlag((pos){ i, j }, T_OBSTRUCTS_DIAGONAL_MOVEMENT) ? PDS_OBSTRUCTION : PDS_FORBIDDEN;
                rogue.mapToSafeTerrain[i][j] = 30000; // OOS prophylactic
            } else if ((monst && (monst->turnsSpentStationary > 1 || (monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)))
                       || (cellHasTerrainFlag((pos){ i, j }, T_PATHING_BLOCKER & ~T_HARMFUL_TERRAIN) && !cellHasTMFlag((pos){ i, j }, TM_IS_SECRET))) {

                costMap[i][j] = PDS_FORBIDDEN;
                rogue.mapToSafeTerrain[i][j] = 30000;
            } else if (cellHasTerrainFlag((pos){ i, j }, T_HARMFUL_TERRAIN) || pmap[i][j].layers[DUNGEON] == DOOR) {
                // The door thing is an aesthetically offensive but necessary hack to make sure
                // that monsters trying to find their way out of caustic gas do not sprint for
                // the doors. Doors are superficially free of gas, but as soon as they are opened,
                // gas will fill their tile, so they are not actually safe. Without this fix,
                // allies will fidget back and forth in a doorway while they asphyxiate.
                // This will have to do. It's a difficult problem to solve elegantly.
                costMap[i][j] = 1;
                rogue.mapToSafeTerrain[i][j] = 30000;
            } else {
                costMap[i][j] = 1;
                rogue.mapToSafeTerrain[i][j] = 0;
            }
        }
    }
    dijkstraScan(rogue.mapToSafeTerrain, costMap, false);
    freeGrid(costMap);
}

static void processIncrementalAutoID() {
    item *theItem, *autoIdentifyItems[3] = {rogue.armor, rogue.ringLeft, rogue.ringRight};
    char buf[DCOLS*3], theItemName[DCOLS*3];
    short i;

    for (i=0; i<3; i++) {
        theItem = autoIdentifyItems[i];
        if (theItem
            && theItem->charges > 0
            && (!(theItem->flags & ITEM_IDENTIFIED) || ((theItem->category & RING) && !ringTable[theItem->kind].identified))) {

            theItem->charges--;
            if (theItem->charges <= 0) {
                itemName(theItem, theItemName, false, false, NULL);
                sprintf(buf, "you are now familiar enough with your %s to identify it.", theItemName);
                messageWithColor(buf, &itemMessageColor, 0);

                if (theItem->category & ARMOR) {
                    // Don't necessarily reveal the armor's runic specifically, just that it has one.
                    theItem->flags |= ITEM_IDENTIFIED;
                } else if (theItem->category & RING) {
                    identify(theItem);
                }
                updateIdentifiableItems();

                itemName(theItem, theItemName, true, true, NULL);
                sprintf(buf, "%s %s.", (theItem->quantity > 1 ? "they are" : "it is"), theItemName);
                messageWithColor(buf, &itemMessageColor, 0);
            }
        }
    }
}

short staffChargeDuration(const item *theItem) {
    // staffs of blinking and obstruction recharge half as fast so they're less powerful
    return (theItem->kind == STAFF_BLINKING || theItem->kind == STAFF_OBSTRUCTION ? 10000 : 5000) / theItem->enchant1;
}

// Multiplier can be negative, in which case staffs and charms will be drained instead of recharged.
void rechargeItemsIncrementally(short multiplier) {
    item *theItem;
    char buf[DCOLS*3], theItemName[DCOLS*3];
    short rechargeIncrement, staffRechargeDuration;

    if (rogue.wisdomBonus) {
        // at level 27, you recharge anything to full in one turn
        rechargeIncrement = 10 * ringWisdomMultiplier(rogue.wisdomBonus * FP_FACTOR) / FP_FACTOR;
    } else {
        rechargeIncrement = 10;
    }

    rechargeIncrement *= multiplier;

    for (theItem = packItems->nextItem; theItem != NULL; theItem = theItem->nextItem) {
        if (theItem->category & STAFF) {
            if (theItem->charges < theItem->enchant1 && rechargeIncrement > 0
                || theItem->charges > 0 && rechargeIncrement < 0) {

                theItem->enchant2 -= rechargeIncrement;
            }
            staffRechargeDuration = staffChargeDuration(theItem);
            while (theItem->enchant2 <= 0) {
                // if it's time to add a staff charge
                if (theItem->charges < theItem->enchant1) {
                    theItem->charges++;
                }
                theItem->enchant2 += randClumpedRange(max(staffRechargeDuration / 3, 1), staffRechargeDuration * 5 / 3, 3);
            }
            while (theItem->enchant2 > staffRechargeDuration * 5 / 3) {
                // if it's time to drain a staff charge
                if (theItem->charges > 0) {
                    theItem->charges--;
                }
                theItem->enchant2 -= staffRechargeDuration;
            }
        } else if ((theItem->category & CHARM) && (theItem->charges > 0)) {
            theItem->charges = clamp(theItem->charges - multiplier, 0, charmRechargeDelay(theItem->kind, theItem->enchant1));
            if (theItem->charges == 0) {
                itemName(theItem, theItemName, false, false, NULL);
                sprintf(buf, "your %s has recharged.", theItemName);
                message(buf, 0);
            }
        }
    }
}

void extinguishFireOnCreature(creature *monst) {

    monst->status[STATUS_BURNING] = 0;
    if (monst == &player) {
        player.info.foreColor = &white;
        rogue.minersLight.lightColor = &minersLightColor;
        refreshDungeonCell(player.loc);
        updateVision(true);
        message("you are no longer on fire.", 0);
    }
}

// n is the monster's depthLevel - 1.
static void monsterEntersLevel(creature *monst, short n) {
    char monstName[COLS], buf[COLS];
    boolean pit = false;

    levels[n].mapStorage[monst->loc.x][monst->loc.y].flags &= ~HAS_MONSTER;

    // place traversing monster near the stairs on this level
    if (monst->bookkeepingFlags & MB_APPROACHING_DOWNSTAIRS) {
        monst->loc = rogue.upLoc;
    } else if (monst->bookkeepingFlags & MB_APPROACHING_UPSTAIRS) {
        monst->loc = rogue.downLoc;
    } else if (monst->bookkeepingFlags & MB_APPROACHING_PIT) { // jumping down pit
        pit = true;
        monst->loc = levels[n].playerExitedVia;
    } else {
        brogueAssert(false);
    }
    monst->depth = rogue.depthLevel;
    monst->targetCorpseLoc = INVALID_POS;

    if (!pit) {
        getQualifyingPathLocNear(&(monst->loc.x), &(monst->loc.y), monst->loc.x, monst->loc.y, true,
                                 T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(monst->info)), 0,
                                 avoidedFlagsForMonster(&(monst->info)), HAS_STAIRS, false);
    }
    if (!pit
        && (pmapAt(monst->loc)->flags & (HAS_PLAYER | HAS_MONSTER))
        && !(terrainFlags(monst->loc) & avoidedFlagsForMonster(&(monst->info)))) {
        // Monsters using the stairs will displace any creatures already located there, to thwart stair-dancing.
        creature *prevMonst = monsterAtLoc(monst->loc);
        brogueAssert(prevMonst);
        getQualifyingPathLocNear(&(prevMonst->loc.x), &(prevMonst->loc.y), monst->loc.x, monst->loc.y, true,
                                 T_DIVIDES_LEVEL & avoidedFlagsForMonster(&(prevMonst->info)), 0,
                                 avoidedFlagsForMonster(&(prevMonst->info)), (HAS_MONSTER | HAS_PLAYER | HAS_STAIRS), false);
        pmapAt(monst->loc)->flags &= ~(HAS_PLAYER | HAS_MONSTER);
        pmapAt(prevMonst->loc)->flags |= (prevMonst == &player ? HAS_PLAYER : HAS_MONSTER);
        refreshDungeonCell(prevMonst->loc);
        //DEBUG printf("\nBumped a creature (%s) from (%i, %i) to (%i, %i).", prevMonst->info.monsterName, monst->loc.x, monst->loc.y, prevMonst->loc.x, prevMonst->loc.y);
    }

    // remove traversing monster from other level monster chain
    removeCreature(&levels[n].monsters, monst);
    // prepend traversing monster to current level monster chain
    prependCreature(monsters, monst);

    monst->status[STATUS_ENTERS_LEVEL_IN] = 0;
    monst->bookkeepingFlags |= MB_PREPLACED;
    monst->bookkeepingFlags &= ~MB_IS_FALLING;
    restoreMonster(monst, NULL, NULL);
    //DEBUG printf("\nPlaced a creature (%s) at (%i, %i).", monst->info.monsterName, monst->loc.x, monst->loc.y);
    monst->ticksUntilTurn = monst->movementSpeed;
    refreshDungeonCell(monst->loc);

    if (pit) {
        monsterName(monstName, monst, true);
        if (!monst->status[STATUS_LEVITATING]) {
            if (inflictDamage(NULL, monst, randClumpedRange(6, 12, 2), &red, false)) {
                if (canSeeMonster(monst)) {
                    sprintf(buf, "%s plummets from above and splatters against the ground!", monstName);
                    messageWithColor(buf, messageColorFromVictim(monst), 0);
                }
                killCreature(monst, false);
            } else {
                if (canSeeMonster(monst)) {
                    sprintf(buf, "%s falls from above and crashes to the ground!", monstName);
                    message(buf, 0);
                }
            }
        } else if (canSeeMonster(monst)) {
            sprintf(buf, "%s swoops into the cavern from above.", monstName);
            message(buf, 0);
        }
    }
}

static void monstersApproachStairs() {
    short n;

    for (n = rogue.depthLevel - 2; n <= rogue.depthLevel; n += 2) { // cycle through previous and next level
        if (n >= 0 && n < gameConst->deepestLevel && levels[n].visited) {
            for (creatureIterator it = iterateCreatures(&levels[n].monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                if (monst->status[STATUS_ENTERS_LEVEL_IN] > 1) {
                    monst->status[STATUS_ENTERS_LEVEL_IN]--;
                } else if (monst->status[STATUS_ENTERS_LEVEL_IN] == 1) {
                    monsterEntersLevel(monst, n);
                }
            }
        }
    }

    if (rogue.yendorWarden
        && abs(rogue.depthLevel - rogue.yendorWarden->depth) > 1) {

        updateYendorWardenTracking();
    }
}

static void decrementPlayerStatus() {
    // Handle hunger.
    if (!player.status[STATUS_PARALYZED]) {
        // No nutrition is expended while paralyzed.
        if (player.status[STATUS_NUTRITION] > 0) {
            if (!numberOfMatchingPackItems(AMULET, 0, 0, false) || rand_percent(20)) {
                player.status[STATUS_NUTRITION]--;
            }
        }
        checkNutrition();
    }

    if (player.status[STATUS_TELEPATHIC] > 0 && !--player.status[STATUS_TELEPATHIC]) {
        updateVision(true);
        message("your preternatural mental sensitivity fades.", 0);
    }

    if (player.status[STATUS_DARKNESS] > 0) {
        player.status[STATUS_DARKNESS]--;
        updateMinersLightRadius();
        if (!player.status[STATUS_DARKNESS]) {
            message("the cloak of darkness lifts from your vision.", 0);
        }
    }

    if (player.status[STATUS_HALLUCINATING] > 0 && !--player.status[STATUS_HALLUCINATING]) {
        displayLevel();
        message("your hallucinations fade.", 0);
    }

    if (player.status[STATUS_LEVITATING] > 0 && !--player.status[STATUS_LEVITATING]) {
        message("you are no longer levitating.", 0);
    }

    if (player.status[STATUS_CONFUSED] > 0 && !--player.status[STATUS_CONFUSED]) {
        message("you no longer feel confused.", 0);
    }

    if (player.status[STATUS_NAUSEOUS] > 0 && !--player.status[STATUS_NAUSEOUS]) {
        message("you feel less nauseous.", 0);
    }

    if (player.status[STATUS_PARALYZED] > 0 && !--player.status[STATUS_PARALYZED]) {
        message("you can move again.", 0);
    }

    if (player.status[STATUS_HASTED] > 0 && !--player.status[STATUS_HASTED]) {
        player.movementSpeed = player.info.movementSpeed;
        player.attackSpeed = player.info.attackSpeed;
        synchronizePlayerTimeState();
        message("your supernatural speed fades.", 0);
    }

    if (player.status[STATUS_SLOWED] > 0 && !--player.status[STATUS_SLOWED]) {
        player.movementSpeed = player.info.movementSpeed;
        player.attackSpeed = player.info.attackSpeed;
        synchronizePlayerTimeState();
        message("your normal speed resumes.", 0);
    }

    if (player.status[STATUS_WEAKENED] > 0 && !--player.status[STATUS_WEAKENED]) {
        player.weaknessAmount = 0;
        message("strength returns to your muscles as the weakening toxin wears off.", 0);
        updateEncumbrance();
    }

    if (player.status[STATUS_DONNING]) {
        player.status[STATUS_DONNING]--;
        recalculateEquipmentBonuses();
    }

    if (player.status[STATUS_IMMUNE_TO_FIRE] > 0 && !--player.status[STATUS_IMMUNE_TO_FIRE]) {
        message("you no longer feel immune to fire.", 0);
    }

    if (player.status[STATUS_STUCK] && !cellHasTerrainFlag(player.loc, T_ENTANGLES)) {
        player.status[STATUS_STUCK] = 0;
    }

    if (player.status[STATUS_EXPLOSION_IMMUNITY]) {
        player.status[STATUS_EXPLOSION_IMMUNITY]--;
    }

    if (player.status[STATUS_DISCORDANT]) {
        player.status[STATUS_DISCORDANT]--;
    }

    if (player.status[STATUS_AGGRAVATING]) {
        player.status[STATUS_AGGRAVATING]--;
    }

    if (player.status[STATUS_SHIELDED]) {
        player.status[STATUS_SHIELDED] -= player.maxStatus[STATUS_SHIELDED] / 20;
        if (player.status[STATUS_SHIELDED] <= 0) {
            player.status[STATUS_SHIELDED] = player.maxStatus[STATUS_SHIELDED] = 0;
        }
    }

    if (player.status[STATUS_INVISIBLE] > 0 && !--player.status[STATUS_INVISIBLE]) {
        message("you are no longer invisible.", 0);
    }

    if (rogue.monsterSpawnFuse <= 0) {
        spawnPeriodicHorde();
        rogue.monsterSpawnFuse = rand_range(125, 175);
    }
}

static boolean dangerChanged(boolean danger[4]) {
    for (enum directions dir = 0; dir < 4; dir++) {
        const pos newLoc = posNeighborInDirection(player.loc, dir);
        if (danger[dir] != monsterAvoids(&player, newLoc)) {
            return true;
        }
    }
    return false;
}

void autoRest() {
    boolean danger[4];
    for (enum directions dir = 0; dir < 4; dir++) {
        const pos newLoc = posNeighborInDirection(player.loc, dir);
        danger[dir] = monsterAvoids(&player, newLoc);
    }

    rogue.disturbed = false;
    rogue.automationActive = true;
    // Stop as soon as we're free from crystal.
    const boolean initiallyEmbedded = cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY);

    if ((player.currentHP < player.info.maxHP
         || player.status[STATUS_HALLUCINATING]
         || player.status[STATUS_CONFUSED]
         || player.status[STATUS_NAUSEOUS]
         || player.status[STATUS_POISONED]
         || player.status[STATUS_DARKNESS]
         || initiallyEmbedded)
        && !rogue.disturbed) {

        int i = 0;
        while (i++ < TURNS_FOR_FULL_REGEN
               && (player.currentHP < player.info.maxHP
                   || player.status[STATUS_HALLUCINATING]
                   || player.status[STATUS_CONFUSED]
                   || player.status[STATUS_NAUSEOUS]
                   || player.status[STATUS_POISONED]
                   || player.status[STATUS_DARKNESS]
                   || cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY))
               && !rogue.disturbed
               && (!initiallyEmbedded || cellHasTerrainFlag(player.loc, T_OBSTRUCTS_PASSABILITY))) {

            recordKeystroke(REST_KEY, false, false);
            rogue.justRested = true;
            playerTurnEnded();
            if (dangerChanged(danger) || pauseAnimation(1, PAUSE_BEHAVIOR_DEFAULT)) {
                rogue.disturbed = true;
            }
        }
    } else {
        for (int i=0; i<100 && !rogue.disturbed; i++) {
            recordKeystroke(REST_KEY, false, false);
            rogue.justRested = true;
            playerTurnEnded();
            if (dangerChanged(danger) || pauseAnimation(1, PAUSE_BEHAVIOR_DEFAULT)) {
                rogue.disturbed = true;
            }
        }
    }
    rogue.automationActive = false;
}

void manualSearch() {
    recordKeystroke(SEARCH_KEY, false, false);

    if (player.status[STATUS_SEARCHING] <= 0) {
        player.status[STATUS_SEARCHING] = 0;
        player.maxStatus[STATUS_SEARCHING] = 5;
    }

    player.status[STATUS_SEARCHING] += 1;

    /* The search strength values were chosen based on equating the expected
    number of cells discovered by 5x 80 searches (1.7.4) and 1x 200 search
    (1.7.5). 1x200 discovers an average of 932 cells; 5.65 times more cells than
    the 165 of 5x80. This factor is intepreted as the advantage of undelayed
    searching. Hence, we chose a short radius r and a long radius s such that

        4 * 5.65 * E_r + E_s ~= 932

    where E_x is the expected no. of cells discovered with radius x. We choose
    r=60, s=160, giving 852 < 932 (under to account for removal of 1.7.5 stealth
    range doubling).
    */
    short searchStrength = 0;
    if (player.status[STATUS_SEARCHING] < 5) {
        searchStrength = (rogue.awarenessBonus >= 0 ? 60 : 30);
    } else {
        // Do a final, larger-radius search on the fifth search in a row
        searchStrength = 160;
        message("you finish your detailed search of the area.", 0);
        player.status[STATUS_SEARCHING] = 0;
    }

    // ensure our search is no weaker than the current passive search
    search(max(searchStrength, rogue.awarenessBonus + 30));

    rogue.justSearched = true;
    playerTurnEnded();
}

// Call this periodically (when haste/slow wears off and when moving between depths)
// to keep environmental updates in sync with player turns.
void synchronizePlayerTimeState() {
    rogue.ticksTillUpdateEnvironment = player.ticksUntilTurn;
}

void playerRecoversFromAttacking(boolean anAttackHit) {
    if (player.ticksUntilTurn >= 0) {
        // Don't do this if the player's weapon of speed just fired.
        if (rogue.weapon && (rogue.weapon->flags & ITEM_ATTACKS_STAGGER) && anAttackHit) {
            player.ticksUntilTurn += 2 * player.attackSpeed;
        } else if (rogue.weapon && (rogue.weapon->flags & ITEM_ATTACKS_QUICKLY)) {
            player.ticksUntilTurn += player.attackSpeed / 2;
        } else {
            player.ticksUntilTurn += player.attackSpeed;
        }
    }
}


static void recordCurrentCreatureHealths() {

    boolean handledPlayer = false;
    for (creatureIterator it = iterateCreatures(monsters); !handledPlayer || hasNextCreature(it);) {
        creature *monst = !handledPlayer ? &player : nextCreature(&it);
        handledPlayer = true;
        monst->previousHealthPoints = monst->currentHP;
    }
}

// This is the dungeon schedule manager, called every time the player's turn comes to an end.
// It hands control over to monsters until they've all expended their accumulated ticks,
// updating the environment (gas spreading, flames spreading and burning out, etc.) every
// 100 ticks.
void playerTurnEnded() {
    short soonestTurn, damage, turnsRequiredToShore, turnsToShore;
    char buf[COLS], buf2[COLS];
    boolean fastForward = false;
    short oldRNG;

    brogueAssert(rogue.RNG == RNG_SUBSTANTIVE);

    handleXPXP();
    resetDFMessageEligibility();
    recordCurrentCreatureHealths();

    if (player.bookkeepingFlags & MB_IS_FALLING) {
        playerFalls();
        if (!rogue.gameHasEnded) {
            handleHealthAlerts();
        }
        return;
    }

    // This happens in updateEnvironment, but some monsters move faster than the
    // environment updates in the loop below. This means they need to fall at
    // the start of the turn to avoid them being able to act while suspended
    // over a chasm
    monstersFall();

    do {
        if (rogue.gameHasEnded) {
            return;
        }

        if (!player.status[STATUS_PARALYZED]) {
            rogue.playerTurnNumber++; // So recordings don't register more turns than you actually have.
        }
        rogue.absoluteTurnNumber++;

        if (player.status[STATUS_INVISIBLE]) {
            rogue.scentTurnNumber += 10; // Your scent fades very quickly while you are invisible.
        } else {
            rogue.scentTurnNumber += 3; // this must happen per subjective player time
        }
        if (rogue.scentTurnNumber > 20000) {
            resetScentTurnNumber();
        }

        //updateFlavorText();

        // Regeneration/starvation:
        if (player.status[STATUS_NUTRITION] <= 0) {
            player.currentHP--;
            if (player.currentHP <= 0) {
                gameOver("Starved to death", true);
                return;
            }
        } else if (player.currentHP < player.info.maxHP
                   && !player.status[STATUS_POISONED]) {
            if ((player.turnsUntilRegen -= 1000) <= 0) {
                player.currentHP++;
                if (player.previousHealthPoints < player.currentHP) {
                    player.previousHealthPoints++; // Regeneration doesn't display on the status bar.
                }
                player.turnsUntilRegen += player.info.turnsBetweenRegen;
            }
            if (player.regenPerTurn) {
                player.currentHP += player.regenPerTurn;
                if (player.previousHealthPoints < player.currentHP) {
                    player.previousHealthPoints = min(player.currentHP, player.previousHealthPoints + player.regenPerTurn);
                }
            }
        }

        if (rogue.awarenessBonus > -30 && !(pmapAt(player.loc)->flags & SEARCHED_FROM_HERE)) {
            // Low-grade auto-search wherever you step, but only once per tile.
            search(rogue.awarenessBonus + 30);
            pmapAt(player.loc)->flags |= SEARCHED_FROM_HERE;
        }
        if (!rogue.justSearched && player.status[STATUS_SEARCHING] > 0) {
            // Searching only "charges up" when done on consecutive turns
            player.status[STATUS_SEARCHING] = 0;
        }
        if (rogue.staleLoopMap) {
            analyzeMap(false); // Don't need to update the chokemap.
        }

        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            if ((monst->bookkeepingFlags & MB_BOUND_TO_LEADER)
                && (!monst->leader || !(monst->bookkeepingFlags & MB_FOLLOWER))
                && (monst->creatureState != MONSTER_ALLY)) {

                killCreature(monst, false);
                if (canSeeMonster(monst)) {
                    monsterName(buf2, monst, true);
                    sprintf(buf, "%s dissipates into thin air", buf2);
                    combatMessage(buf, messageColorFromVictim(monst));
                }
            }
        }

        if (player.status[STATUS_BURNING] > 0) {
            damage = rand_range(1, 3);
            if (!(player.status[STATUS_IMMUNE_TO_FIRE]) && inflictDamage(NULL, &player, damage, &orange, true)) {
                killCreature(&player, false);
                gameOver("Burned to death", true);
            }
            if (!--player.status[STATUS_BURNING]) {
                extinguishFireOnCreature(&player);
            }
        }

        if (player.status[STATUS_POISONED] > 0) {
            player.status[STATUS_POISONED]--;
            if (inflictDamage(NULL, &player, player.poisonAmount, &green, true)) {
                killCreature(&player, false);
                gameOver("Died from poison", true);
            }
            if (!player.status[STATUS_POISONED]) {
                player.poisonAmount = 0;
            }
        }

        if (player.ticksUntilTurn == 0) { // attacking adds ticks elsewhere
            player.ticksUntilTurn += player.movementSpeed;
        } else if (player.ticksUntilTurn < 0) { // if he gets a free turn
            player.ticksUntilTurn = 0;
        }

        updateScent();
//      updateVision(true);
//        rogue.stealthRange = currentStealthRange();
//        if (rogue.displayStealthRangeMode) {
//            displayLevel();
//        }
        rogue.updatedSafetyMapThisTurn          = false;
        rogue.updatedAllySafetyMapThisTurn      = false;
        rogue.updatedMapToSafeTerrainThisTurn   = false;

        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            if (D_SAFETY_VISION || monst->creatureState == MONSTER_FLEEING && pmapAt(monst->loc)->flags & IN_FIELD_OF_VIEW) {
                updateSafetyMap(); // only if there is a fleeing monster who can see the player
                break;
            }
        }

        if (D_BULLET_TIME && !rogue.justRested) {
            player.ticksUntilTurn = 0;
        }

        applyGradualTileEffectsToCreature(&player, player.ticksUntilTurn);

        if (rogue.gameHasEnded) {
            return;
        }

        rogue.heardCombatThisTurn = false;

        while (player.ticksUntilTurn > 0) {
            soonestTurn = 10000;
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                soonestTurn = min(soonestTurn, monst->ticksUntilTurn);
            }
            soonestTurn = min(soonestTurn, player.ticksUntilTurn);
            soonestTurn = min(soonestTurn, rogue.ticksTillUpdateEnvironment);
            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                creature *monst = nextCreature(&it);
                monst->ticksUntilTurn -= soonestTurn;
            }
            rogue.ticksTillUpdateEnvironment -= soonestTurn;
            if (rogue.ticksTillUpdateEnvironment <= 0) {
                rogue.ticksTillUpdateEnvironment += 100;

                // stuff that happens periodically according to an objective time measurement goes here:
                rechargeItemsIncrementally(1); // staffs recharge every so often
                processIncrementalAutoID();   // become more familiar with worn armor and rings
                rogue.monsterSpawnFuse--; // monsters spawn in the level every so often

                for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                    creature *monst = nextCreature(&it);
                    applyInstantTileEffectsToCreature(monst);
                }

                for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                    creature *monst = nextCreature(&it);
                    decrementMonsterStatus(monst);
                }

                // monsters with a dungeon feature spawn it every so often
                for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
                    creature *monst = nextCreature(&it);

                    if (monst->info.DFChance
                        && !(monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)
                        && rand_percent(monst->info.DFChance)) {

                        spawnDungeonFeature(monst->loc.x, monst->loc.y, &dungeonFeatureCatalog[monst->info.DFType], true, false);
                    }
                }

                updateEnvironment(); // Update fire and gas, items floating around in water, monsters falling into chasms, etc.
                decrementPlayerStatus();
                applyInstantTileEffectsToCreature(&player);
                if (rogue.gameHasEnded) { // caustic gas, lava, trapdoor, etc.
                    return;
                }
                monstersApproachStairs();

                if (player.ticksUntilTurn > 100 && !fastForward) {
                    fastForward = rogue.playbackFastForward || pauseAnimation(25, PAUSE_BEHAVIOR_DEFAULT);
                }

                // Rolling waypoint refresh:
                rogue.wpRefreshTicker++;
                if (rogue.wpRefreshTicker >= rogue.wpCount) {
                    rogue.wpRefreshTicker = 0;
                }
                refreshWaypoint(rogue.wpRefreshTicker);
            }

            for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it) && rogue.gameHasEnded == false;) {
                creature *monst = nextCreature(&it);
                if (monst->ticksUntilTurn <= 0) {
                    if (monst->currentHP > monst->info.maxHP) {
                        monst->currentHP = monst->info.maxHP;
                    }

                    if ((monst->info.flags & MONST_GETS_TURN_ON_ACTIVATION)
                        || monst->status[STATUS_PARALYZED]
                        || monst->status[STATUS_ENTRANCED]
                        || (monst->bookkeepingFlags & MB_CAPTIVE)) {

                        // Do not pass go; do not collect 200 gold.
                        monst->ticksUntilTurn = monst->movementSpeed;
                    } else {
                        monstersTurn(monst);
                    }

                    for (creatureIterator it2 = iterateCreatures(monsters); hasNextCreature(it2);) {
                        creature *monst2 = nextCreature(&it2);
                        if (monst2 == monst) { // monst still alive and on the level
                            applyGradualTileEffectsToCreature(monst, monst->ticksUntilTurn);
                            break;
                        }
                    }
                }
            }

            player.ticksUntilTurn -= soonestTurn;

            if (rogue.gameHasEnded) {
                return;
            }
        }
        // DEBUG displayLevel();
        //checkForDungeonErrors();

        updateVision(true);
        rogue.stealthRange = currentStealthRange();
        if (rogue.displayStealthRangeMode) {
            displayLevel();
        }

        for (creatureIterator it = iterateCreatures(monsters); hasNextCreature(it);) {
            creature *monst = nextCreature(&it);
            if (canSeeMonster(monst) && !(monst->bookkeepingFlags & (MB_WAS_VISIBLE | MB_ALREADY_SEEN))) {
                if (monst->creatureState != MONSTER_ALLY) {
                    rogue.disturbed = true;
                    if (rogue.cautiousMode || rogue.automationActive) {
                        oldRNG = rogue.RNG;
                        rogue.RNG = RNG_COSMETIC;
                        //assureCosmeticRNG;
                        monsterName(buf2, monst, false);
                        sprintf(buf, "you %s a%s %s",
                                playerCanDirectlySee(monst->loc.x, monst->loc.y) ? "see" : "sense",
                                (isVowelish(buf2) ? "n" : ""),
                                buf2);
                        if (rogue.cautiousMode) {
                            strcat(buf, ".");
                            message(buf, REQUIRE_ACKNOWLEDGMENT);
                        } else {
                            combatMessage(buf, 0);
                        }
                        restoreRNG;
                    }
                }
            }

            if (canSeeMonster(monst)) {
                monst->bookkeepingFlags |= MB_WAS_VISIBLE;
                if (cellHasTerrainFlag(monst->loc, T_OBSTRUCTS_PASSABILITY)
                    && cellHasTMFlag(monst->loc, TM_IS_SECRET)) {

                    discover(monst->loc.x, monst->loc.y);
                }
                if (canDirectlySeeMonster(monst)) {
                    if (rogue.weapon && rogue.weapon->flags & ITEM_RUNIC
                        && rogue.weapon->enchant2 == W_SLAYING
                        && !(rogue.weapon->flags & ITEM_RUNIC_HINTED)
                        && monsterIsInClass(monst, rogue.weapon->vorpalEnemy)) {

                        rogue.weapon->flags |= ITEM_RUNIC_HINTED;
                        itemName(rogue.weapon, buf2, false, false, NULL);
                        sprintf(buf, "the runes on your %s gleam balefully.", buf2);
                        messageWithColor(buf, &itemMessageColor, REQUIRE_ACKNOWLEDGMENT);
                    }
                    if (rogue.armor && rogue.armor->flags & ITEM_RUNIC
                        && rogue.armor->enchant2 == A_IMMUNITY
                        && !(rogue.armor->flags & ITEM_RUNIC_HINTED)
                        && monsterIsInClass(monst, rogue.armor->vorpalEnemy)) {

                        rogue.armor->flags |= ITEM_RUNIC_HINTED;
                        itemName(rogue.armor, buf2, false, false, NULL);
                        sprintf(buf, "the runes on your %s glow protectively.", buf2);
                        messageWithColor(buf, &itemMessageColor, REQUIRE_ACKNOWLEDGMENT);
                    }
                }
            }

            if (!canSeeMonster(monst)
                && (monst->bookkeepingFlags & MB_WAS_VISIBLE)
                && !(monst->bookkeepingFlags & MB_CAPTIVE)) {
                // For captives we never unset MB_WAS_VISIBLE because captives are not moving,
                // so we don't want to get "You see a ..." every time they come back into view.

                monst->bookkeepingFlags &= ~MB_WAS_VISIBLE;
            }
        }

        displayCombatText();

        if (player.status[STATUS_PARALYZED]) {
            if (!fastForward) {
                fastForward = rogue.playbackFastForward || pauseAnimation(25, PAUSE_BEHAVIOR_DEFAULT);
            }
        }

        //checkNutrition(); // Now handled within decrementPlayerStatus().
        if (!rogue.playbackFastForward) {
            shuffleTerrainColors(100, false);
        }

        displayAnnotation();

        refreshSideBar(-1, -1, false);

        applyInstantTileEffectsToCreature(&player);
        if (rogue.gameHasEnded) { // caustic gas, lava, trapdoor, etc.
            return;
        }

        if (player.currentHP > player.info.maxHP) {
            player.currentHP = player.info.maxHP;
        }

        if (player.bookkeepingFlags & MB_IS_FALLING) {
            playerFalls();
            handleHealthAlerts();
            return;
        }

    } while (player.status[STATUS_PARALYZED]);

    rogue.justRested = false;
    rogue.justSearched = false;
    updateFlavorText();

    if (!rogue.updatedMapToShoreThisTurn) {
        updateMapToShore();
    }

    // "point of no return" check
    if ((player.status[STATUS_LEVITATING] && cellHasTerrainFlag(player.loc, T_LAVA_INSTA_DEATH | T_IS_DEEP_WATER | T_AUTO_DESCENT))
        || (player.status[STATUS_IMMUNE_TO_FIRE] && cellHasTerrainFlag(player.loc, T_LAVA_INSTA_DEATH))) {
        if (!rogue.receivedLevitationWarning) {
            turnsRequiredToShore = rogue.mapToShore[player.loc.x][player.loc.y] * player.movementSpeed / 100;
            if (cellHasTerrainFlag(player.loc, T_LAVA_INSTA_DEATH)) {
                turnsToShore = max(player.status[STATUS_LEVITATING], player.status[STATUS_IMMUNE_TO_FIRE]) * 100 / player.movementSpeed;
            } else {
                turnsToShore = player.status[STATUS_LEVITATING] * 100 / player.movementSpeed;
            }
            if (turnsRequiredToShore == turnsToShore || turnsRequiredToShore + 1 == turnsToShore) {
                message("better head back to solid ground!", REQUIRE_ACKNOWLEDGMENT);
                rogue.receivedLevitationWarning = true;
            } else if (turnsRequiredToShore > turnsToShore
                       && turnsRequiredToShore < 10000) {
                message("you're past the point of no return!", REQUIRE_ACKNOWLEDGMENT);
                rogue.receivedLevitationWarning = true;
            }
        }
    } else {
        rogue.receivedLevitationWarning = false;
    }

    removeDeadMonsters();
    rogue.playbackBetweenTurns = true;
    RNGCheck();
    handleHealthAlerts();

    if (rogue.flareCount > 0) {
        animateFlares(rogue.flares, rogue.flareCount);
        rogue.flareCount = 0;
    }
}

void resetScentTurnNumber() { // don't want player.scentTurnNumber to roll over the short maxint!
    short i, j, d;
    rogue.scentTurnNumber -= 15000;
    for (d = 0; d < gameConst->deepestLevel; d++) {
        if (levels[d].visited) {
            for (i=0; i<DCOLS; i++) {
                for (j=0; j<DROWS; j++) {
                    if (levels[d].scentMap[i][j] > 15000) {
                        levels[d].scentMap[i][j] -= 15000;
                    } else {
                        levels[d].scentMap[i][j] = 0;
                    }
                }
            }
        }
    }
}

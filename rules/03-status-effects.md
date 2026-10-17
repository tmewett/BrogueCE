**BR74170** — Every creature tracks 27 distinct status effects, each storing a current value and a maximum value; only positive values matter, and most effects count down toward zero. The effects are searching, donning, weakened, telepathic, hallucinating, levitating, slowed, hasted, confused, burning, paralyzed, poisoned, stuck, nauseous, discordant, immune to fire, explosion immunity, nutrition, entering the level, enraged, magical fear, entranced, darkness, remaining lifespan, shielded, invisible, and aggravating.
[Rogue.h:1989-2024](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L1989-L2024)
**BR97529** — For the player, most statuses lose exactly 1 point on each periodic environmental update, which happens about every 100 ticks; this covers telepathy, darkness, hallucinating, levitating, confusion, nausea, paralysis, haste, slow, weakness, donning, fire immunity, explosion immunity, discord, aggravation, and invisibility.
[Time.c:1979-2085](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1979-L2085) [Time.c:2399-2431](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2399-L2431)
**BR70838** — For monsters, most statuses decrement by 1 on the periodic environmental update.
[Monsters.c:1825-1992](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1825-L1992) [Monsters.c:1969-1972](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1969-L1972)
**BR72967** — A creature that inherently flies, burns, is immune to fire, or is invisible keeps that status permanently set to 1000 so it never decreases.
[Monsters.c:3904-3926](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3904-L3926)
**BR79287** — Burning and poison break the periodic cadence: they damage the creature and count down once per that creature's own turn rather than on the environmental update, while the shield effect and the other statuses count down on the environmental update.
[Time.c:2328-2348](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2328-L2348) [Monsters.c:1868-1922](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1868-L1922)
**BR29664** — Negation strips a specific subset of "negatable" statuses; each effect either is or is not in that subset, and the player's value is replaced with a stored value for the effect rather than always cleared. Which effects are negatable, and the value the player is left with, come from the game's fixed table of statuses rather than from the negation code itself.
[Monsters.c:2460-2497](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2460-L2497) [Items.c:3738-3828](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3738-L3828)
**BR62908** — Negation never affects an invulnerable creature, and a negation bolt is also ignored by a creature that always reflects projectiles.
[Items.c:3694-3700](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3694-L3700)
**BR16601** — A creature that dies when negated is destroyed outright rather than merely stripped of its traits, with a message that depends on whether it flies or is inanimate.
[Items.c:3757-3767](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3757-L3767)
**BR82205** — A monster's natural regeneration is suppressed while it is poisoned, and the player also does not naturally regenerate health while poisoned.
[Monsters.c:1831-1833](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1831-L1833) [Time.c:2283-2284](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2283-L2284)
**BR76412** — The searching counter starts at 0 and is capped at a maximum of 5; each search made on consecutive turns adds 1, and any turn spent without searching resets it to 0, so only back-to-back searches accumulate.
[Time.c:2156-2164](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2156-L2164) [Time.c:2305-2308](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2305-L2308)
**BR41784** — Each search before the fifth one uses strength 60 (or 30 if the player's awareness bonus is negative); the fifth consecutive search uses a larger strength of 160 and then resets the counter to 0.
[Time.c:2178-2186](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2178-L2186)
**BR13701** — A weak automatic search also happens the first time the player steps onto a tile, using a strength based on the player's awareness, and a tile is remembered as already searched so it is not repeated.
[Time.c:2300-2304](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2300-L2304)
**BR95942** — Putting on armor without forcing it sets the donning counter to one tenth of that armor's armor value; the counter then decreases by 1 on each periodic update.
[Items.c:7748-7751](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7748-L7751) [Time.c:2045-2048](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2045-L2048)
**BR30764** — While the donning counter is positive it subtracts from the armor's effective enchantment and therefore from the player's defense, so freshly donned armor protects less until the counter reaches 0.
[Items.c:7710-7718](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7710-L7718)
**BR37051** — Removing armor resets the donning counter to 0 immediately.
[Items.c:7835-7839](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7835-L7839)
**BR49074** — Weakening adds 1 to a creature's weakness amount, up to a maximum of 10, and sets the weakened counter to the given duration (keeping whichever is larger).
[Items.c:3830-3842](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3830-L3842)
**BR74699** — Weakness reduces the player's effective strength for item requirements, while each point of weakness lowers a monster's defense by 25 (never below 0) and reduces its accuracy and damage.
[Combat.c:60-67](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L60-L67) [Combat.c:78-101](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L78-L101)
**BR55648** — When the weakened counter reaches 0 the weakness amount is reset to 0; for the player this message is "strength returns to your muscles as the weakening toxin wears off" and encumbrance is recomputed.
[Time.c:2039-2043](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2039-L2043) [Monsters.c:1855-1859](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1855-L1859)
**BR99021** — A potion of strength raises the player's strength and immediately clears the weakness amount, leaving the weakened counter at 1 if it was active.
[Items.c:7306-7315](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7306-L7315)
**BR28518** — The telepathy counter decreases by 1 per periodic update, with the message "your preternatural mental sensitivity fades" when it reaches 0.
[Time.c:1991-1994](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1991-L1994)
**BR83937** — While the player is telepathic, every non-inanimate monster is revealed to the player (as if the player could see its location), and a small field of view is lit around each revealed monster.
[Time.c:610-649](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L610-L649) [Monsters.c:163-177](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L163-L177)
**BR97054** — Granting telepathy refreshes every monster's display, and if no monsters exist on the current depth the player is told they are alone at that moment.
[Items.c:3979-3990](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3979-L3990)
**BR46642** — The hallucination counter decreases by 1 per periodic update.
[Time.c:2004-2007](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2004-L2007)
**BR44751** — A monster with the hallucinate-on-hit ability adds a fixed duration to the player's hallucination counter each time it hits.
[Combat.c:410-419](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L410-L419)
**BR59998** — A potion of hallucination sets the counter to a rolled magnitude, producing the message "colors are everywhere! The walls are singing!".
[Items.c:7281-7284](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7281-L7284)
**BR68400** — While the player is hallucinating and not telepathic, monsters the player can see are displayed under the name of a randomly chosen monster kind.
[Monsters.c:255-282](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L255-L282)
**BR13847** — While the player is hallucinating, item descriptions are replaced with hallucinated ones.
[Movement.c:131-138](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L131-L138)
**BR74771** — While levitating, a creature does not fall into chasms or through trapdoors.
[Time.c:160-171](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L160-L171)
**BR38788** — While levitating, a creature does not trigger pressure plates.
[Time.c:225-230](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L225-L230)
**BR55229** — While levitating, a creature is not poisoned by poisonous lichen.
[Time.c:426-445](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L426-L445)
**BR27834** — While levitating, a creature does not extinguish its own burning by entering fire-extinguishing terrain such as water.
[Time.c:209-216](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L209-L216)
**BR73170** — While levitating, the player does not count as underwater over deep water.
[Time.c:71-91](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L71-L91)
**BR10266** — While levitating, the player does not discover the secrets of the tile they stand on.
[Time.c:113-130](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L113-L130)
**BR98623** — While levitating, location flavor text is shown instead of terrain flavor text.
[Time.c:53-68](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L53-L68)
**BR78750** — Items carried into deep water by a non-levitating player can float away in the current, and a monster's carried item can likewise be lost; levitation prevents this loss.
[Time.c:474-505](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L474-L505)
**BR45966** — The player refuses to step into lava with the message "that would be certain death!" unless levitating or immune to fire.
[Movement.c:1048-1058](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1048-L1058)
**BR91998** — The player must confirm before diving into a chasm, and levitation removes the confirmation.
[Movement.c:1060-1070](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1060-L1070)
**BR51927** — Monsters choosing a path treat lava, chasms, and deep water as impassable when their remaining levitation or fire immunity is too short to reach the location and still return to safety.
[Movement.c:1804-1824](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1804-L1824)
**BR93528** — A liquid-bound monster cannot attack a levitating opponent.
[Combat.c:1044-1046](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1044-L1046)
**BR35089** — The levitation counter decreases by 1 per periodic update (with a creature that inherently flies keeping it permanently).
[Time.c:2009-2011](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2009-L2011) [Monsters.c:1844-1848](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1844-L1848)
**BR84098** — A potion or charm of levitation sets the levitation counter to its rolled or scaled duration.
[Items.c:6726-6776](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6726-L6776) [Items.c:7328-7332](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7328-L7332)
**BR20897** — Gaining levitation immediately releases the player from any monster holding them.
[Items.c:6772-6775](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6772-L6775) [Items.c:7328-7331](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7328-L7331)
**BR50918** — Slowing a creature sets the slow counter to the given duration, clears any haste, and doubles its movement and attack time so it acts at half speed.
[Items.c:3908-3920](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3908-L3920)
**BR37844** — For the player the slow multiplier is applied when encumbrance is recomputed: halved speed when hasted, doubled speed when slowed.
[Items.c:3171-3189](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3171-L3189)
**BR22853** — When the slow counter reaches 0 normal speed is restored; the player also has their time state resynchronized, and a monster resets to its base speed.
[Time.c:2032-2037](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2032-L2037) [Monsters.c:1849-1854](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1849-L1854)
**BR40736** — Haste clears any slow, sets the haste counter to the given duration, and halves the creature's movement and attack time so it acts at double speed.
[Items.c:3922-3934](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3922-L3934)
**BR71884** — When the haste counter reaches 0 the creature's normal movement and attack speeds are restored, and for the player the time state is resynchronized.
[Time.c:2025-2030](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2025-L2030) [Monsters.c:1860-1866](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1860-L1866)
**BR28481** — Haste and slow are mutually exclusive: applying either one clears the other.
[Items.c:3908-3934](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3908-L3934)
**BR84218** — The confusion counter decreases by 1 per periodic update.
[Time.c:2013-2015](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2013-L2015)
**BR14011** — Confusion gas sets the confusion counter to at least 25 on a creature that is neither inanimate nor invulnerable.
[Time.c:391-406](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L391-L406)
**BR24556** — A weapon of confusion sets the target's confusion duration based on the weapon's enchantment.
[Combat.c:751-759](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L751-L759)
**BR12812** — A confused player's chosen movement direction is replaced with a random valid direction.
[Movement.c:894-907](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L894-L907)
**BR88965** — The game warns before a confused player's random step could carry them into lava.
[Movement.c:867-892](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L867-L892)
**BR67805** — A confused monster moves in a random valid direction instead of its intended direction, unless it is entranced.
[Monsters.c:3750-3765](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3750-L3765)
**BR19868** — Confusion suppresses safety confirmations: a confused or non-telepathic hallucinating player is not prompted before attacking discordant allies or acidic targets.
[Movement.c:612-631](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L612-L631)
**BR21973** — A confused player cannot use auto-explore ("Not while you're confused.").
[Movement.c:1945-1953](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1945-L1953)
**BR59674** — A confused player who attacks an allied monster does not cause that ally to stop being allied.
[Combat.c:335-342](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L335-L342)
**BR34635** — A confused monster with the steal-and-flee ability does not steal from the player.
[Combat.c:426-431](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L426-L431)
**BR47057** — Exposing a creature to fire sets its burning counter to at least 7 if it was not already burning, producing the message "you catch fire" for the player; a creature that is fiery, immune to fire, invulnerable, submerged, or standing on fire-extinguishing terrain without levitating is unaffected.
[Time.c:28-51](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L28-L51)
**BR30194** — A burning creature takes 1 to 3 damage per turn, and its burning counter decreases by 1 per turn until it is extinguished; a fiery creature never stops burning.
[Time.c:2328-2337](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2328-L2337) [Monsters.c:1868-1892](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1868-L1892)
**BR36079** — A burning creature standing on flammable terrain sets that terrain alight.
[Time.c:448-459](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L448-L459)
**BR85805** — Terrain that extinguishes fire puts out the fire on any non-levitating creature that is not fiery and not attackable through walls.
[Time.c:209-216](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L209-L216)
**BR44913** — The player is warned before stepping into flame unless already burning or immune to fire.
[Movement.c:1072-1082](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1072-L1082)
**BR12024** — Paralysis gas sets the paralysis counter to at least 20 for a non-inanimate, non-invulnerable, non-submerged creature.
[Time.c:408-423](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L408-L423)
**BR36538** — A paralyzed defender is always hit, takes triple damage, and does not get to retaliate.
[Combat.c:136-146](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L136-L146) [Combat.c:1066-1119](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1066-L1119)
**BR62028** — A paralyzed creature takes no action on its turn.
[Monsters.c:3340-3349](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3340-L3349) [Time.c:2456-2462](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2456-L2462)
**BR24204** — The player expends no nutrition while paralyzed.
[Time.c:1980-1989](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1980-L1989)
**BR74294** — The paralysis counter decreases by 1 per periodic update.
[Time.c:2021-2023](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2021-L2023)
**BR73717** — Attacking a paralyzed creature clears its paralysis and gives it a brief moment to react before the attacker acts again.
[Combat.c:314-325](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L314-L325)
**BR64646** — Poison has both a duration and a concentration: adding poison increases the concentration by the given amount (minimum 1) and adds to the duration, and the maximum value is set to the creature's maximum health divided by the concentration.
[Combat.c:1615-1635](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1615-L1635)
**BR75638** — On each of the creature's turns the poison counter decreases by 1 and the creature takes damage equal to its poison concentration; when the counter reaches 0 the concentration is reset to 0.
[Time.c:2339-2348](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2339-L2348) [Monsters.c:1907-1922](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1907-L1922)
**BR81488** — Poisonous lichen damages and poisons a non-levitating creature but is capped so it never raises the poison concentration above 1.
[Time.c:426-445](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L426-L445)
**BR52651** — A panacea-style heal clears poison entirely (concentration and duration set to 0).
[Items.c:3957-3960](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3957-L3960)
**BR33833** — A panacea-style heal also reduces hallucination, confusion, nausea, and slow to at most 1, clears weakness and its amount, and clears darkness.
[Items.c:3939-3956](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3939-L3956) [Items.c:3961-3967](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3961-L3967)
**BR45733** — A bolt of poison adds poison with a concentration increment of 1 and a duration scaled to the bolt's magnitude.
[Items.c:4571-4573](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4571-L4573)
**BR55761** — A monster with the poisons ability converts its melee damage into poison, dealing only 1 point of direct damage and applying the full amount as poison.
[Combat.c:1160-1163](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1160-L1163) [Combat.c:1213-1214](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1213-L1214)
**BR96828** — Entering terrain that entangles sets stuck to a random 3 to 7 on a creature that is not immune to webs, not invulnerable, and not submerged.
[Time.c:287-308](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L287-L308)
**BR34465** — A stuck defender is always hit, even though stuck does not qualify for triple damage (only surprise, sleep, and paralysis do).
[Combat.c:110-117](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L110-L117) [Combat.c:1096-1157](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1096-L1157)
**BR96404** — A stuck monster cannot leave its web: each attempted move consumes its turn and decreases the counter until it breaks free, at which point the surface web is cleared.
[Monsters.c:3775-3786](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3775-L3786)
**BR27577** — A stuck player struggles: each attempt consumes a turn with the message "you struggle but cannot free yourself", and when the counter reaches 0 the player breaks free and the web is cleared.
[Movement.c:1144-1162](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1144-L1162)
**BR23913** — Being stuck ends immediately for the player who is no longer standing on an entangling tile, and for a monster no longer on an entangling tile.
[Time.c:2054-2056](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2054-L2056) [Monsters.c:1924-1928](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1924-L1928)
**BR65527** — Nausea gas sets the nausea counter to at least 20 on a non-inanimate, non-invulnerable, non-submerged creature.
[Time.c:371-389](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L371-L389)
**BR93338** — A nauseous monster has a 25% chance each move to vomit instead of acting, consuming its turn.
[Monsters.c:3743-3748](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3743-L3748)
**BR37335** — A nauseous player has a 25% chance to vomit instead of moving, consuming their turn.
[Movement.c:1164-1171](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1164-L1171)
**BR48502** — The nausea counter decreases by 1 per periodic update.
[Time.c:2017-2019](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2017-L2019)
**BR18500** — Discord sets the discord counter (30 for the blast; a scaled duration for a staff).
[Items.c:4132-4151](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4132-L4151) [Items.c:4639-4651](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4639-L4651)
**BR49341** — A discordant monster attacks whatever eligible creature is nearest rather than respecting allegiance.
[Monsters.c:3397-3427](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3397-L3427)
**BR44957** — A discordant monster that is wandering will not target the player with bolts, so it casts at other monsters instead.
[Monsters.c:2539-2547](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2539-L2547)
**BR57024** — When discord expires, a fleeing discordant monster whose leader is the player reverts to being an ally and drops its carried item.
[Monsters.c:1929-1941](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1929-L1941)
**BR25391** — A potion or charm of fire immunity sets the fire-immunity counter to its rolled or scaled duration.
[Items.c:6758-6759](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6758-L6759) [Items.c:7392-7393](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7392-L7393)
**BR51327** — Drinking a potion of fire immunity or using a charm of fire immunity extinguishes any burning on the player.
[Items.c:6760-6762](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6760-L6762) [Items.c:7394-7396](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7394-L7396)
**BR56535** — While immune to fire, a creature ignores fiery attacks and cannot be set burning by exposure to fire.
[Combat.c:420-424](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L420-L424) [Time.c:28-51](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L28-L51)
**BR60892** — While immune to fire, a creature is treated as safe from lava.
[Time.c:173-179](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L173-L179)
**BR75439** — The fire-immunity counter decreases by 1 per periodic update, except for creatures that are inherently immune to fire.
[Time.c:2050-2052](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2050-L2052) [Monsters.c:1955-1959](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1955-L1959)
**BR24508** — An explosion deals 15 to 20 damage, or half the creature's maximum health if that is larger.
[Time.c:313-314](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L313-L314)
**BR19893** — An explosion grants the creature 5 turns of explosion immunity.
[Time.c:315](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L315)
**BR31504** — The explosion-immunity counter decreases by 1 per periodic update.
[Time.c:2058-2060](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2058-L2060)
**BR90851** — A submerged creature is excluded from explosion damage entirely.
[Time.c:310-312](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L310-L312)
**BR25686** — Nutrition starts at 2150 for the player (the stomach size).
[Rogue.h:1122-1124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L1122-L1124) [Monsters.c:3925](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3925)
**BR49432** — Nutrition decreases by 1 per periodic update, except that while the player carries the Amulet it decreases only on a 20% chance.
[Time.c:1980-1989](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1980-L1989)
**BR80877** — At 0 nutrition the player loses 1 health per turn and dies of starvation when health reaches 0.
[Time.c:2276-2282](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2276-L2282)
**BR58287** — When nutrition crosses the hunger, weak, and faint thresholds the player receives the corresponding warning message, and at 0 calories the player is told they are starving to death.
[Time.c:824-835](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L824-L835)
**BR98143** — A monster traveling between levels is given a countdown; each periodic update it decreases by 1, and when it reaches exactly 1 the monster enters the current level.
[Time.c:1960-1970](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1960-L1970)
**BR59859** — The Yendor warden is given a countdown of 50 when repositioned just below the player's depth.
[Time.c:1358-1367](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1358-L1367)
**BR49916** — When a monster arrives on a new level its countdown is reset to 0.
[Time.c:1921-1929](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1921-L1929)
**BR98338** — A monster that normally avoids corridors becomes enraged for 4 turns whenever it is attacked in a way that directly deals damage.
[Combat.c:314-333](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L314-L333)
**BR51151** — A creature under magical fear flees; the only enabled way to apply it is to set it to 1 when a fearful creature is attacked, and it is reset to 1 again by any aggressive action against it.
[Combat.c:326-328](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L326-L328) [Combat.c:1052-1055](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1052-L1055)
**BR67694** — When a bolt that deals damage fails to kill a monster, its magical fear is cleared to 0.
[Items.c:4440-4448](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4440-L4448)
**BR15302** — When the fear counter reaches 0 the monster reverts to allied state if the player is its leader, otherwise to tracking the player.
[Monsters.c:1942-1948](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1942-L1948)
**BR60087** — The code path that would set a long magical-fear duration (150 to 225) is disabled, so meaningful fear durations in practice come only from the on-attack setting of 1.
[Items.c:4047-4074](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4047-L4074)
**BR50793** — An entrancement bolt sets the entranced counter on a non-inanimate monster (a scaled staff duration) and wakes it.
[Items.c:4591-4613](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4591-L4613)
**BR64952** — An entranced monster is revealed to the player as though by telepathy.
[Monsters.c:163-177](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L163-L177)
**BR34867** — An entranced monster skips its turns.
[Monsters.c:3340-3349](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3340-L3349) [Time.c:2456-2462](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2456-L2462)
**BR56650** — When the player moves, every entranced monster that is not stuck, paralyzed, or captive is pushed one step in the direction opposite the player's move.
[Movement.c:498-511](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L498-L511)
**BR88931** — Attacking or otherwise dealing damage to an entranced creature clears its entrancement.
[Combat.c:314-329](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L314-L329) [Combat.c:1040-1055](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1040-L1055) [Items.c:6027](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6027)
**BR34544** — When the entrancement bolt hits the player instead, the player becomes confused rather than entranced.
[Items.c:4591-4600](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4591-L4600)
**BR41654** — The darkness counter decreases by 1 per periodic update, producing "the cloak of darkness lifts from your vision" at 0.
[Time.c:1996-2002](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1996-L2002)
**BR31040** — While the darkness counter is positive it shrinks the player's light radius in proportion to how much of the counter remains.
[Light.c:132-142](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L132-L142)
**BR20506** — A potion of darkness sets the darkness counter to a rolled magnitude (keeping the larger of the two) and immediately updates the light radius and vision.
[Items.c:7291-7296](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7291-L7296)
**BR57318** — A creature with a remaining-lifespan counter loses 1 per periodic update, and when it reaches 0 the creature dies and dissipates into thin air.
[Monsters.c:1893-1906](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1893-L1906)
**BR44763** — Shielding sets the shield value to the larger of the current value and a scaled protection amount (from a staff or charm) and records that value as the maximum.
[Items.c:4653-4663](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4653-L4663) [Items.c:6745-6754](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6745-L6754)
**BR38432** — Each point of shield absorbs 10 points of incoming damage: if the shield exceeds the damage times 10 it absorbs everything, otherwise the leftover damage gets through and the shield is depleted.
[Combat.c:1532-1542](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1532-L1542)
**BR68621** — The shield loses one twentieth of its maximum value per periodic update and is cleared completely once it reaches 0.
[Time.c:2070-2075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2070-L2075) [Monsters.c:1949-1954](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1949-L1954)
**BR97727** — Invisibility sets the counter to a scaled duration, and a creature that is inherently invisible keeps it permanently.
[Items.c:4190-4199](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4190-L4199) [Monsters.c:3922-3924](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3922-L3924)
**BR20729** — A creature that is inanimate, invulnerable, or already invisible cannot be made invisible again.
[Items.c:4193](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4193)
**BR27438** — An invisible creature is hidden from observers unless it is standing in gas, though teammates can always see each other.
[Monsters.c:194-219](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L194-L219)
**BR77492** — While the player is invisible the stealth range is fixed at 1 regardless of lighting or armor.
[Time.c:686-691](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L686-L691)
**BR28722** — While the player is invisible their scent fades faster (scent advances 10 per turn instead of 3).
[Time.c:2265-2269](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2265-L2269)
**BR48230** — The invisibility counter decreases by 1 per periodic update, and the creature is redrawn when it becomes visible again.
[Time.c:2077-2079](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2077-L2079) [Monsters.c:1960-1968](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1960-L1968)
**BR80706** — Aggravation sets the counter to its radius when applied at the player's location.
[Items.c:3394-3397](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3394-L3397)
**BR50175** — While the aggravation counter is positive it adds that many points to the player's stealth range.
[Time.c:710-712](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L710-L712)
**BR18867** — The aggravation counter decreases by 1 per periodic update.
[Time.c:2066-2068](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2066-L2068)
**BR91554** — Applying aggravation wakes and alerts every monster within the radius and clears their distance-keeping and corridor-avoiding behaviors.
[Items.c:3372-3382](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3372-L3382)
**BR40440** — Applying aggravation lays a scent trail toward the location.
[Items.c:3385-3392](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3385-L3392)

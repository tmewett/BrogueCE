**BR91629** — Every attack uses the attacker's accuracy rating to determine whether it hits; the effective chance is that accuracy multiplied by a factor that shrinks with the defender's defense, then clamped to the range 0% to 100%. Accuracy above 100 and negative defense are permitted, so the unclamped result can fall outside the normal percentage range.
[Combat.c:29-38](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L29-L38) [Combat.c:127-133](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L127-L133)
**BR93748** — Each point of defense multiplies the incoming chance to hit by about 0.987, so armor makes its wearer exponentially harder to hit as it improves.
[Combat.c:33-38](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L33-L38) [PowerTables.c:184-204](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L184-L204)
**BR97408** — The player's base accuracy is 100, and each point of net weapon enchantment (weapon level adjusted for strength) multiplies accuracy by about 1.065. A cursed or overly heavy weapon can drive net enchantment negative, multiplying accuracy by less than 1.
[Combat.c:53-58](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L53-L58) [Combat.c:118-126](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L118-L126) [PowerTables.c:161-182](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L161-L182)
**BR90053** — When an attack hits, damage is rolled within the attacker's damage range, and a clumping factor shapes the distribution: a clumping factor of 2 rolls two dice, 3 rolls three dice, and so on, concentrating results toward the middle. If the range width is not evenly divisible by the clumping factor, the first few dice have their maximum raised by one so the very top of the range can still be reached.
[Combat.c:43-51](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L43-L51) [Combat.c:1098-1099](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1098-L1099)
**BR87246** — A missed attack deals no damage; when either the attacker or the defender is visible to the player, the game reports that the attacker missed, and when neither is visible the player instead hears combat in the distance.
[Combat.c:1259-1271](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1259-L1271)
**BR52698** — An unseen killing blow is announced only by sound: the player hears something die, or hears something get destroyed when the victim is inanimate.
[Combat.c:1178-1183](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1178-L1183)
**BR76106** — Each point of strength above an item's requirement adds 0.25 to that item's net enchantment, while each point below the requirement subtracts 2.5; net enchantment is then capped between -20 and +50.
[Combat.c:60-67](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L60-L67) [Combat.c:69-76](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L69-L76)
**BR20189** — Damage output is scaled by roughly 1.065 per point of enchantment-like adjustment, which is how a weakened monster's blows grow softer; the same scaling table corresponds to the enchantment values used elsewhere in combat.
[Combat.c:78-85](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L78-L85) [PowerTables.c:138-159](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L138-L159)
**BR65933** — An attack automatically hits a defender that is stuck, paralyzed, held captive, or already seized by the attacker.
[Combat.c:110-117](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L110-L117) [Combat.c:136-146](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L136-L146)
**BR66846** — The player catches a wandering (unaware) monster by surprise, and a defender that is asleep or paralyzed is likewise caught flat-footed; all such attacks automatically hit.
[Combat.c:1061-1069](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1061-L1069) [Combat.c:1096-1099](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1096-L1099)
**BR38655** — A surprise attack deals triple damage; a dagger used for a surprise attack deals five times damage instead.
[Combat.c:1110-1119](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1110-L1119)
**BR88066** — A defender caught surprised, asleep, or paralyzed does not get to retaliate on the spot; its next turn is delayed instead.
[Combat.c:1101-1108](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1101-L1108)
**BR89965** — A non-allied monster struck while surprised, asleep, or paralyzed is woken and begins tracking the player.
[Combat.c:1105-1107](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1105-L1107)
**BR45761** — A lunge attack always hits and deals the same triple damage as a surprise attack (or five times damage with a dagger), and is reported as a vicious lunge.
[Combat.c:1096](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1096) [Combat.c:1110-1119](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1110-L1119) [Combat.c:1145-1146](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1145-L1146)
**BR54926** — A creature able to grab that attacks an adjacent opponent seizes it instead of dealing damage that turn, and while a creature is seized by its attacker every subsequent attack against it automatically hits.
[Combat.c:1074-1094](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1074-L1094) [Combat.c:110-117](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L110-L117)
**BR52487** — If a submerged grabbing creature seizes a player who can see it, the creature immediately surfaces so it can be targeted.
[Combat.c:1082-1087](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1082-L1087)
**BR33297** — A sweeping attack strikes every creature adjacent to the attacker in all eight directions, not just the original target. Each would-be victim must be one the attacker is willing to attack, and cannot be hidden behind impenetrable terrain unless it can be attacked through walls.
[Combat.c:1744-1780](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1744-L1780) [Combat.c:1771-1775](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1771-L1775)
**BR66148** — A slaying weapon always hits and instantly kills creatures of its designated kind; against any other creature its rune does nothing.
[Combat.c:118-124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L118-L124) [Combat.c:613-614](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L613-L614) [Combat.c:655-665](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L655-L665)
**BR44056** — A quietus weapon sometimes instantly kills the creature it strikes, regardless of the creature's remaining health.
[Combat.c:655-665](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L655-L665)
**BR35202** — A speed weapon can freeze time for a moment, granting the player an immediate extra turn.
[Combat.c:645-654](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L645-L654)
**BR89036** — A paralysis weapon can freeze the struck creature in place for a duration that grows with the weapon's enchantment, to a minimum of two turns.
[Combat.c:666-675](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L666-L675) [PowerTables.c:99](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L99)
**BR78777** — A confusion weapon can confuse the struck creature for a duration that grows with enchantment, to a minimum of three turns.
[Combat.c:751-760](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L751-L760) [PowerTables.c:100](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L100)
**BR44146** — A slowing weapon can slow the struck creature for a duration that grows with enchantment, to a minimum of three turns.
[Combat.c:742-750](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L742-L750) [PowerTables.c:102](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L102)
**BR83507** — A multiplicity weapon can summon between one and seven spectral duplicates of the player that fight alongside them for three turns each; greater enchantment raises the number of duplicates.
[Combat.c:676-741](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L676-L741) [PowerTables.c:103-104](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L103-L104)
**BR62468** — A force weapon launches the struck creature backward; if it is stopped short of the full knockback distance it takes impact damage equal to the distance it actually traveled, and the creature or wall it collides with can also be hurt.
[Combat.c:498-589](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L498-L589) [PowerTables.c:101](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L101)
**BR91628** — A mercy weapon heals the creature it strikes instead of harming it.
[Combat.c:764-769](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L764-L769)
**BR80454** — A plenty weapon creates a duplicate of the creature it strikes.
[Combat.c:770-778](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L770-L778)
**BR57646** — Weapon runes other than slaying never trigger against inanimate or invulnerable creatures.
[Combat.c:615-616](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L615-L616)
**BR53275** — A rune's chance to activate rises with the weapon's enchantment but is lower for weapons that already have high base damage. Weapons that strike more slowly get an increased per-hit chance, weapons that strike twice per turn get a reduced per-hit chance, the chance is never lower than the weapon's enchantment level nor higher than 100%, weak or cursed runes activate at a flat 15%, and a slaying rune has no random chance at all.
[PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
**BR98298** — Striking a creature that is surprised, asleep, or paralyzed can trigger a weapon rune at an increased chance, between roughly double and the midpoint between double and certainty.
[Combat.c:619-621](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L619-L621)
**BR23904** — Multiplicity armor has a 33% chance, when the player is struck in melee by a living creature, to spawn one to five short-lived spectral copies of that attacker to fight for the player.
[Combat.c:831-872](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L831-L872) [PowerTables.c:108](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L108)
**BR47523** — Mutuality armor shares an incoming blow with all adjacent enemies: the player and each of those enemies take a reduced, evenly divided portion of the damage.
[Combat.c:874-917](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L874-L917)
**BR27038** — Absorption armor soaks a random amount of each incoming blow, from 1 up to the armor's enchantment (minimum 1); if the blow is fully absorbed, no damage is taken.
[Combat.c:918-927](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L918-L927) [PowerTables.c:107](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L107)
**BR79261** — Reprisal armor reflects part of each melee blow back at the attacker, dealing at least 5% of the damage for each point of enchantment (with a 5% minimum).
[Combat.c:928-944](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L928-L944) [PowerTables.c:106](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L106)
**BR30235** — Immunity armor completely negates all damage from the kind of creature it protects against.
[Combat.c:945-950](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L945-L950)
**BR65158** — The wearer of immunity armor also suffers none of that creature kind's special on-hit effects.
[Combat.c:369-380](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L369-L380) [Combat.c:393-395](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L393-L395)
**BR99068** — Burden armor has a 10% chance each time the player is struck to grow heavier, permanently raising its strength requirement by one.
[Combat.c:951-958](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L951-L958)
**BR65297** — Vulnerability armor doubles all damage the player takes from melee blows.
[Combat.c:959-965](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L959-L965)
**BR94361** — Immolation armor has a 10% chance when the player is struck to erupt in a burst of flame.
[Combat.c:966-973](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L966-L973)
**BR29551** — Armor runes take effect only when the player is the one being struck.
[Combat.c:1121-1123](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1121-L1123)
**BR82064** — Discovering an armor rune's effect automatically identifies that armor.
[Combat.c:978-980](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L978-L980)
**BR33368** — A protected weapon or armor never degrades when striking or being struck by acidic creatures.
[Combat.c:397-401](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L397-L401) [Combat.c:1239-1245](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1239-L1245)
**BR89754** — Attacking an acid-type creature corrodes the player's weapon, lowering its enchantment by one (down to a floor of about -10) and announcing that the weapon weakens; a slaying weapon attacking its designated kind is exempt from this.
[Combat.c:1239-1256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1239-L1256)
**BR91048** — Certain creatures corrode the player's armor when they hit, lowering its enchantment by one unless that would drop it too far.
[Combat.c:397-409](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L397-L409)
**BR78435** — A protection shield absorbs damage at a rate of ten shield points per point of damage; if the shield exceeds the blow, the leftover shield persists, otherwise the shield is used up and any excess damage gets through.
[Combat.c:1532-1542](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1532-L1542)
**BR27052** — A reflective effect has a chance to turn an incoming attack back on its source; that chance rises with enchantment, is about 15% at the first point, and never falls below 1% or rises above 100%.
[PowerTables.c:109-123](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L109-L123)
**BR77279** — Creatures that are immune to weapons take no damage at all from ordinary melee attacks, though other sources such as fire can still hurt them.
[Combat.c:1098-1099](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1098-L1099)
**BR81065** — An invulnerable creature cannot be damaged by any attack.
[Combat.c:1526-1530](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1526-L1530)
**BR36423** — An invulnerable creature also cannot be knocked back or staggered.
[Combat.c:999-1005](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L999-L1005)
**BR58689** — On easy mode, all damage the player takes from an attack is reduced to one-fifth, but never below one point.
[Combat.c:1560-1564](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1560-L1564)
**BR76528** — A blow that reduces a creature to zero health kills it; the creature is marked as dying for later removal, and when the kill is visible the game displays a message naming the attacker and the victim.
[Combat.c:1165-1195](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1165-L1195) [Combat.c:1642-1653](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1642-L1653)
**BR57747** — Killing a dragon with the player as the attacker earns the dragonslayer feat.
[Combat.c:1191-1195](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1191-L1195)
**BR86062** — If the player is killed by an attack, the game ends.
[Combat.c:1188-1190](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1188-L1190)
**BR31952** — A slain monster drops whatever it was carrying.
[Combat.c:1662-1669](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1662-L1669)
**BR96880** — Some monsters leave a death effect such as an explosion or a cloud when they die, accompanied by a message when the death is visible.
[Combat.c:1671-1681](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1671-L1681)
**BR28007** — A monster that was carried inside a slain monster is released at the same spot.
[Combat.c:1711-1732](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1711-L1732)
**BR56010** — Nearby allied monsters may claim a slain monster's corpse to consume and learn one of its abilities, behaviors, or attacks.
[Combat.c:1733](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1733) [Combat.c:1401-1511](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1401-L1511)
**BR95398** — A monster that flees when badly hurt does so once its health drops to a quarter of its maximum or less.
[Combat.c:1600-1605](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1600-L1605)
**BR58833** — Wounded creatures bleed, with larger wounds producing more blood (or, for some creatures, gas).
[Combat.c:1546-1554](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1546-L1554)
**BR11906** — An attacker with transference heals itself for a share of the damage it inflicts: the player heals by the ring's multiplier, allied monsters recover 40%, and enemy monsters recover 90%.
[Combat.c:1566-1588](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1566-L1588)
**BR41331** — Reaping recharges the player's staffs, wands, and charms by a random amount up to the damage dealt multiplied by the reaping bonus, and never more than the victim's remaining health.
[Combat.c:1125-1138](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1125-L1138)
**BR61676** — An exploding, self-destructive attacker dies in the act of attacking and deals no ordinary blow.
[Combat.c:1031-1034](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1031-L1034)
**BR86360** — A weapon heavy enough to stagger can knock its target back one square, unless the target is invulnerable, immobile, inanimate, held captive, backed against a wall, or the destination square is occupied.
[Combat.c:999-1014](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L999-L1014) [Combat.c:1210-1212](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1210-L1212)
**BR40268** — The player's weapon becomes automatically identified once it has been used in enough attacks to become familiar.
[Combat.c:983-997](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L983-L997) [Combat.c:1232-1237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1232-L1237)
**BR62243** — Attacking a monster alerts it to the player's presence.
[Combat.c:344-349](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L344-L349)
**BR25647** — If the player attacks one of their own allies while not confused, that monster stops being an ally.
[Combat.c:335-342](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L335-L342)
**BR41184** — A creature with a poisoning attack deals only a single point of ordinary damage but poisons its victim in proportion to the blow.
[Combat.c:1160-1163](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1160-L1163) [Combat.c:481-486](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L481-L486)
**BR82949** — A creature with a weakening attack weakens its victim whenever its blow deals damage.
[Combat.c:487-492](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L487-L492)
**BR29616** — A creature able to steal and flee takes a random unequipped item from the player's pack when its blow lands (never while it is confused) and then flees; a stack of more than three weapons loses about half, while any other stack loses one item.
[Combat.c:426-479](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L426-L479)
**BR66834** — A creature bound to water or other liquid cannot attack a levitating or flying opponent at all.
[Combat.c:1044-1046](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1044-L1046)
**BR89326** — Attacking a creature that is neither hunting the player nor inanimate forfeits the paladin feat.
[Combat.c:357-367](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L357-L367)
**BR20491** — Attacking a visible monster with a weapon forfeits the pure mage feat.
[Combat.c:1027-1029](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1027-L1029)
**BR74866** — A monster that divides in two when struck splits into a clone, each keeping half the original's health, when it is struck in melee with a free adjacent square available; the split clones do not inherit learned abilities.
[Combat.c:208-312](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L208-L312)
**BR31620** — A sleeping monster wakes as soon as it takes damage from any source.
[Combat.c:1556-1558](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1556-L1558)
**BR53981** — When one of the player's weapon runes takes effect, the weapon is automatically identified.
[Combat.c:783-785](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L783-L785)
**BR40230** — Killing a monster that was leading others demotes it from leadership and re-evaluates the creatures that were following it.
[Combat.c:1737-1741](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1737-L1741)
**BR36640** — A creature with a staggering attack knocks its victim back one square under the same restrictions as a heavy weapon.
[Combat.c:493-495](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L493-L495) [Combat.c:999-1014](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L999-L1014)
**BR17384** — A monster that can grab seizes an adjacent opponent instead of injuring them that turn. [Combat.c:1074-1093](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1074-L1093)
**BR25575** — A creature that is not bound to water can attack a levitating opponent. [Combat.c:1044-1046](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1044-L1046)
**BR37800** — A water-bound monster cannot attack a levitating or flying opponent. [Combat.c:1044-1046](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1044-L1046)

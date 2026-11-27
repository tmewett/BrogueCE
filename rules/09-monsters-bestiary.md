----
BR39474
**Flies.** The creature hovers above the ground, so deep water, chasms and
  lava neither hinder nor harm it: it crosses them as though they were ordinary floor.
  Reference: [Rogue.h 2068](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2068), [Monsters.c 3916-3918](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3916-L3918), [Monsters.c 1389-1391](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1389-L1391), [Time.c 93-98](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L93-L98), [Time.c 174-179](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L174-L179).
----
BR10411
**Moves erratically.** On roughly one turn in three the creature ignores its
  intended path and lurches in a random legal direction, making its approach hard to
  predict. Reference: [Rogue.h 2069](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2069), [Monsters.c 3758-3764](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3758-L3764).
----
BR60148
**Bound to liquid.** The creature cannot venture onto dry land: it moves only
  on tiles that allow submersion. If it is stranded on dry land it cannot move, attack, or
  seize anything and simply writhes helplessly. Reference: [Monsters.c 3770-3773](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3770-L3773), [Monsters.c 1318-1321](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1318-L1321), [Monsters.c 3358-3363](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3358-L3363).
----
BR47597
**Moves at full speed in deep water.** Water neither slows the creature nor deters
  it: it crosses deep water as readily as dry ground. Reference: [Rogue.h 2077](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2077), [Monsters.c 1395-1398](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1395-L1398).
----
BR65633
**Submerges.** While in suitable terrain the creature can sink beneath the surface,
  becoming effectively hidden until it strikes. A submerged creature cannot be targeted
  by bolts; it surfaces when it attacks (and a grabbing creature stays hidden on the turn
  it first seizes its prey). Reference: [Rogue.h 2079](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2079) and [Rogue.h 2152](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2152), [Monsters.c 1977-1991](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1977-L1991).
----
BR99362
**Immobile.** The creature cannot move or make ordinary melee attacks; it can only
  use its special abilities (spells, ranged attacks, and so on) from where it stands.
  Reference: [Rogue.h 2060](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2060), [Monsters.c 3388-3395](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3388-L3395).
----
BR72064
**Keeps its distance.** When an enemy comes within two tiles, the creature backs
  away instead of closing to melee range, even if it has no ranged attack of its own.
  Reference: [Monsters.c 2962-2977](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2962-L2977), [Monsters.c 1771-1773](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1771-L1773), [Monsters.c 1784-1788](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1784-L1788).
----
BR30757
**Will not follow between levels.** The creature will not chase the player up or
  down stairs; it stays on its own dungeon level. Reference: [Rogue.h 2081](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2081), [RogueMain.c 597-608](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L597-L608).
----
BR91557
**Cannot be entangled.** Webs and similar entangling effects pass harmlessly
  through or off the creature. Reference: [Rogue.h 2072](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2072), [Monsters.c 1392](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1392).
----
BR13361
**Attackable while embedded in a wall.** Normally a creature set into solid stone
  cannot be struck; this one can be. Reference: [Rogue.h 2045](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2045), [Monsters.c 297-301](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L297-L301).
----
BR53451
**Carries an item.** The creature may be generated holding a random item; some
  carry one every time, others only about a quarter of the time. Reference: [Monsters.c 132-153](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L132-L153).
----
BR14275
**Drops its item when it dies.** When the creature dies, it drops the item it was
  carrying. Reference: [Combat.c 1662-1667](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1662-L1667), [Monsters.c 4076-4086](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L4076-L4086).
----
BR48675
**Normally left out of the sidebar.** The creature is ordinarily absent from the
  player's list of creatures, even while it is plainly visible in the dungeon.
  Reference: [Rogue.h 2085](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2085), [IO.c 3801](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3801).
----
BR67941
**Never sleeps.** The creature begins awake and hunting rather than asleep when it
  is first placed. Reference: [Monsters.c 111](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L111).
----
BR46423
**Always hunting.** The creature is never asleep or idly wandering; it is
  permanently aware of the player's location and pursues relentlessly. Reference:
  [Rogue.h 2063](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2063), [Monsters.c 1656-1657](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1656-L1657), [Monsters.c 1716-1721](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1716-L1721).
----
BR51688
**Flees near death.** Once reduced below one quarter of its health the creature
  turns and runs; it re-engages once it climbs back above three quarters of its health.
  Reference: [Rogue.h 2064](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2064), [Monsters.c 1774-1814](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1774-L1814), [Combat.c 1600-1605](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1600-L1605).
----
BR48373
**Avoids corridors.** While hunting as part of a group, the creature keeps to
  open ground and will not step from it into a corridor, unless it is enraged and at or
  below half health. Reference: [Monsters.c 1494-1502](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1494-L1502), [Dijkstra.c 237-238](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Dijkstra.c#L237-L238).
----
BR16717
**Acts only when activated.** The creature never takes a turn on its own; it
  springs to life (and acts) only when the magical machine it guards is triggered.
  Reference: [Rogue.h 2086](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2086), [Monsters.c 3333-3338](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3333-L3338).
----
BR85956
**Never fails to use its special ability.** Whenever the creature is eligible to
  use its ability it always does so, with none of the usual random chance of holding
  back. Reference: [Rogue.h 2087](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2087), [Monsters.c 2794-2795](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2794-L2795), [Monsters.c 2809](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2809).
----
BR39641
**Casts spells slowly.** After casting a spell, the creature must wait twice as long
  as after an ordinary attack before acting again, so it can cast only half as often. Reference: [Monsters.c 3130-3131](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3130-L3131), [Monsters.c 3389-3390](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3389-L3390).
----
BR21958
**Inanimate.** The creature is an object rather than a living thing: it is immune
  to many afflictions that affect the living (poison, weakness, hallucination, drowning,
  confusion and the like) and shows only an abbreviated status readout. Reference:
  [Rogue.h 2059](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2059), [Monsters.c 3059](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3059), [Combat.c 1061-1064](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1061-L1064).
----
BR90225
**Invulnerable.** The creature is immune to absolutely everything; it cannot be
  harmed by any means. Reference: [Rogue.h 2076](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2076).
----
BR95842
**Immune to weapons.** Ordinary weapon damage does not hurt it, whether the blow
  comes from a melee weapon or from a weapon thrown at it. Reference: [Rogue.h 2067](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2067), [Combat.c 1098-1100](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1098-L1100), [Items.c 6054-6056](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6054-L6056).
----
BR37585
**Corrodes the weapon that strikes it.** When your weapon strikes the creature in
  melee, it weakens by one enchantment, unless it is protected or is a slaying weapon matched to the creature; it never weakens below -10. Reference: [Rogue.h 2066](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2066), [Combat.c 1239-1256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1239-L1256).
----
BR13523
**Immune to fire.** While its fire immunity holds, flame does not harm the
  creature: it never catches fire, and fiery attacks leave it untouched. Reference:
  [Time.c 28-36](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L28-L36), [Combat.c 420-423](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L420-L423), [Items.c 4406-4416](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4406-L4416).
----
BR57968
**Burns unceasingly.** The creature is permanently ablaze, wrapped in an aura of
  flame. Reference: [Rogue.h 2075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2075), [Monsters.c 3913-3915](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3913-L3915).
----
BR18737
**Reflects about half of magical bolts.** Roughly half of all bolts fired at the
  creature bounce off, as though it wore strongly enchanted armor of reflection.
  Reference: [Rogue.h 2073](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2073).
----
BR12704
**Never results from polymorphism.** No polymorphism spell can transform
  another creature into this one. Reference: [Rogue.h 2088](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2088), [Items.c 3872-3874](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3872-L3874).
----
BR49476
**Dies if negated.** Negation magic destroys the creature outright: a levitating
  one dissipates into thin air, an inanimate one shatters into tiny pieces, and any other falls
  to the ground, lifeless. Reference: [Items.c 3757-3767](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3757-L3767).
----
BR56920
**Invisible.** The creature cannot ordinarily be seen; magic or other senses are
  needed to detect it. Reference: [Rogue.h 2058](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2058), [Monsters.c 3922-3924](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3922-L3924).
----
BR11587
**Described as male or female.** Some creatures are male or female, and messages
  about them refer to the creature with the matching pronoun (he, she, his, her, and so
  on). Reference: [Rogue.h 2083-2084](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2083-L2084), [Monsters.c 402-405](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L402-L405), [Monsters.c 435-457](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L435-L457).
----
BR46956
**Causes hallucinations.** A successful hit leaves the victim hallucinating, so
  it no longer sees the dungeon or its inhabitants correctly. Reference: [Rogue.h 2106](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2106), [Combat.c 410-419](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L410-L419).
----
BR18776
**Steals an item and runs away.** On a hit the creature may snatch a random item
  from the victim's pack and then flee with it.
  Reference: [Rogue.h 2107](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2107), [Combat.c 426-479](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L426-L479).
----
BR32322
**Sets the victim alight.** A successful hit sets the target on fire.
  Reference: [Rogue.h 2108](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2108), [Combat.c 420-424](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L420-L424).
----
BR80252
**Corrodes armor.** A successful hit damages the victim's armor, reducing its
  enchantment. Reference: [Rogue.h 2110](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2110), [Combat.c 397-409](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L397-L409).
----
BR45824
**Deals its damage as poison.** Instead of immediate harm, the attack poisons the
  victim, which then loses health over many turns. Reference: [Rogue.h 2113](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2113), [Combat.c 481-486](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L481-L486), [Combat.c 1160-1163](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1160-L1163).
----
BR32139
**Causes weakness.** A successful hit leaves the victim weak, so its own melee
  blows do less damage. Reference: [Rogue.h 2118](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2118), [Combat.c 487-492](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L487-L492).
----
BR54374
**Grabs and holds its victim.** The first contact seizes the victim's legs, pinning
  it in place so it cannot move away; the creature then attacks the held prey. Reference:
  [Rogue.h 2112](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2112), [Combat.c 1074-1094](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1074-L1094).
----
BR15511
**Explodes rather than attacking.** The creature never lands a melee attack: the
  moment it attacks an enemy it dies in the act instead, dealing no damage. Reference: [Rogue.h 2116](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2116), [Combat.c 1031-1034](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1031-L1034).
----
BR83324
**Recovers health from the damage it deals.** The creature heals itself for a
  large fraction of every point of damage it inflicts — a monster recovers 90% and an
  allied version 40%. Reference: [Rogue.h 2117](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2117), [Combat.c 1566-1588](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1566-L1588).
----
BR97180
**Splits in two when struck.** Every time it is hit (up to a large cap) the
  creature divides into two, each with roughly half the health, spawning the new half in
  an adjacent open space. Reference: [Rogue.h 2115](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2115), [Combat.c 215-216](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L215-L216), [Combat.c 264-307](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L264-L307).
----
BR12637
**Releases a burst on death.** When the creature dies it leaves a dungeon
  effect in the square it occupied (a gas cloud, an explosion, a pool of blood and
  so on). Reference: [Rogue.h 2114](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2114), [Combat.c 1671-1681](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1671-L1681).
----
BR43379
**Strikes up to two enemies in a line.** The attack thrusts through the first
  enemy and can also hit a second one directly behind it, like a spear. Reference:
  [Rogue.h 2119](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2119), [Movement.c 695-753](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L695-L753), [Monsters.c 3819-3824](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3819-L3824).
----
BR97260
**Strikes every adjacent enemy at once.** One swing hits all enemies surrounding
  the creature, like a sweeping axe blow. Reference:
  [Rogue.h 2120](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2120), [Monsters.c 3880-3891](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3880-L3891).
----
BR17980
**Strikes at a distance with a whip.** The creature can attack a target one tile
  away in a cardinal direction, reaching past the space immediately in front of it.
  Reference: [Rogue.h 2121](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2121), [Movement.c 638-688](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L638-L688), [Monsters.c 3819-3824](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3819-L3824).
----
BR88806
**Pushes the victim backward.** A successful hit shoves the victim one tile away
  if there is room, knocking it back like a hammer blow. Reference: [Rogue.h 2122](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2122), [Combat.c 493-495](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L493-L495), [Combat.c 999-1014](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L999-L1014).
----
BR56605
**Reflects all magical bolts back at the caster.** Every bolt aimed at the creature
  is returned directly to whoever fired it. Reference: [Rogue.h 2124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2124), [Monsters.c 4745-4756](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L4745-L4756).
----
BR40544
**Summons allies.** The creature can call up new minions (often in hordes) to fight
  beside it; the chance and frequency depend on how many minions it already has. Reference:
  [Rogue.h 2111](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2111), [Monsters.c 2409-2457](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2409-L2457), [Monsters.c 976-1068](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L976-L1068).
----
BR70715
**Hides inside its summoned minions.** The creature (such as a lich's
  phylactery, a phoenix egg, or a vampire) does not appear bodily: it is carried within one
  of its summoned servants, and when that servant is defeated it reappears in the servant's
  place. Reference:
  [Monsters.c 989-992](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L989-L992), [Monsters.c 1047-1057](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1047-L1057), [Combat.c 1713-1731](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1713-L1731).
----
BR53754
**Casts a shielding spell on allies.** Wraps a friendly creature in a protective
  magical shield that reduces incoming damage. Reference: [Monsters.c 2730-2737](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2730-L2737).
----
BR47059
**Casts a healing spell on allies.** Restores health to a wounded friendly
  creature (never wasted on one already at full health). Reference: [Items.c 4615-4616](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4615-L4616), [Monsters.c 2589-2593](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2589-L2593), [Monsters.c 2738-2743](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2738-L2743).
----
BR73986
**Casts a haste spell on allies.** Makes a friendly creature act more often
  (faster movement and attacks) for a time. Reference: [Items.c 4500-4508](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4500-L4508), [Items.c 3922-3934](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3922-L3934), [Monsters.c 2722-2729](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2722-L2729).
----
BR32475
**Casts a slowing spell on enemies.** Slows a target so it moves and attacks less
  often. Reference: [Items.c 4491-4492](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4491-L4492), [Items.c 3908-3919](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3908-L3919), [Monsters.c 1849-1853](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1849-L1853).
----
BR68026
**Casts discord on enemies.** Turns the victim against its own allies so it
  attacks them for a time. It will not intentionally target the player with this.
  Reference: [Monsters.c 2664-2671](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2664-L2671).
----
BR81389
**Casts negation.** Strips a target's magical effects — haste, levitation,
  shielding and the like — leaving it as it naturally is. Reference:
  [Items.c 3738-3828](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3738-L3828), [Monsters.c 2481-2497](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2481-L2497).
----
BR60584
**Hurls fire bolts.** A searing bolt that damages the target and can set it alight.
  Reference: [Monsters.c 2614-2618](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2614-L2618), [Monsters.c 2639-2645](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2639-L2645).
----
BR78949
**Fires bolts of lightning.** A damaging electrical bolt (the spark turret's whole
  attack). Reference: [Monsters.c 2639-2645](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2639-L2645).
----
BR18935
**Spits entangling webs.** Fires webbing that sticks the victim in place, like a
  spider's web. Reference: [Monsters.c 1608-1617](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1608-L1617), [Monsters.c 2646-2663](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2646-L2663).
----
BR16647
**Blinks (teleports).** Rather than an attack, the creature can teleport short
  distances — blinking toward prey, toward safety when fleeing, or away from danger. This
  is resolved as movement elsewhere, not as a combat bolt. Reference: [Monsters.c 2790-2792](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2790-L2792), [Monsters.c 3437-3443](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3437-L3443).
----
BR71983
**Beckons the target toward it.** A magical tug drags the victim toward the caster,
  but it is never used at point-blank range. Reference: [Items.c 4477-4490](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4477-L4490), [Items.c 4325-4338](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4325-L4338), [Monsters.c 2627-2631](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2627-L2631).
----
BR81211
**Hurls entangling vines.** Lashes the target with grasping vines that hold it
  fast, like webs. Reference: [Monsters.c 2646-2663](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2646-L2663).
----
BR68263
**Attacks at range with a physical projectile.** Fires an arrow or bolt that deals
  ordinary damage to a target in line of sight (does not work against a creature embedded
  in a wall). Reference: [Monsters.c 2632-2638](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2632-L2638).
----
BR88052
**Fires poison darts.** A ranged attack delivered by poison dart (the dart turret's
  whole attack). Reference: [Monsters.c 2632-2645](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2632-L2645).
----
BR77502
**Breathes dragonfire.** The dragon breathes a gout of white-hot flame at an
  enemy — a fiery bolt that deals heavy damage and is never aimed at a creature immune to fire.
  Reference: [Globals.c 1123-1124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1123-L1124), [GlobalsBrogue.c 85](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/variants/GlobalsBrogue.c#L85), [Monsters.c 2755-2769](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2755-L2769), [Items.c 4395-4417](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4395-L4417), [Monsters.c 2608-2618](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2608-L2618).
----
BR83147
**Rat.** Health 6, defense 0, accuracy 80, damage 1–3; regenerates 1 health every 20
turns; normal movement and attack speed. It has no special attacks, but has a 1% chance
each turn to leave a trail of urine. [Globals.c 1030](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1030)
----
BR10466
**Kobold.** Health 7, defense 0, accuracy 80, damage 1–4; regenerates every 20 turns;
normal movement and attack speed. It has no special traits or attacks.
[Globals.c 1031](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1031)
----
BR24264
**Jackal.** Health 8, defense 0, accuracy 70, damage 2–4; regenerates every 20 turns;
**moves twice as fast as normal** (speed 50); normal attack speed. It has a 1% chance
each turn to leave a trail of urine. [Globals.c 1032](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1032)
----
BR89268
**Eel.** Health 18, defense 27, accuracy 100, damage 3–7; regenerates every 5 turns;
moves twice as fast as normal (speed 50); normal attack speed. It is **bound to liquid**
(BR60148), **moves at full speed in deep water** (BR47597), **submerges** (BR65633), **moves
erratically** (BR10411), and **never sleeps** (BR67941).
[Globals.c 1033-1034](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1033-L1034)
----
BR45724
**Monkey.** Health 12, defense 17, accuracy 100, damage 1–3; regenerates every 20
turns; normal speeds. It **steals an item and flees** when it hits (BR18776) and has a 1%
chance per turn to leave a urine trail. [Globals.c 1035-1036](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1035-L1036)
----
BR17917
**Bloat.** Health 4, defense 0, accuracy 100, damage 0 (it never makes a normal
attack); regenerates every 5 turns; normal speeds. It **flies** (BR39474), **moves
erratically** (BR10411), **explodes rather than attacking** (BR15511), and **releases a cloud on
death** (BR12637): a bloated cloud of gas that can ignite. [Globals.c 1037-1038](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1037-L1038)
----
BR37113
**Pit Bloat.** Health 4, defense 0, accuracy 100, damage 0; regenerates every 5 turns;
normal speeds. Like the bloat it **flies** (BR39474), **moves erratically** (BR10411), **explodes
rather than attacking** (BR15511), and **releases a dungeon effect on death** (BR12637) — here
tearing open a pit in the ground rather than blooming into gas.
[Globals.c 1039-1040](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1039-L1040)
----
BR73845
**Goblin.** Health 15, defense 10, accuracy 70, damage 2–5; regenerates every 20
turns; normal speeds. Its attacks **penetrate to a second enemy in a line** (BR43379), and it
**avoids corridors** while hunting (BR48373). [Globals.c 1041-1042](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1041-L1042)
----
BR25964
**Goblin Conjurer.** Health 10, defense 10, accuracy 70, damage 2–4; regenerates every
20 turns; normal movement and attack speed. [Globals.c 1043-1044](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1043-L1044)
----
BR44428
**Goblin Mystic.** Health 10, defense 10, accuracy 70, damage 2–4; regenerates every
20 turns; normal speeds. It **casts a shielding spell on its allies** (BR53754), **keeps its
distance** (BR72064), **avoids corridors** (BR48373), and carries an item a quarter of the time
(BR53451). [Globals.c 1045-1046](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1045-L1046)
----
BR93065
**Goblin Totem.** Health 30, defense 0, accuracy 0 (no normal hits), damage 0;
no regeneration; normal movement but **attacks slowly** (attack speed 300). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) totem that **will not follow between levels** (BR30757)
and **cannot be entangled** (BR91557). It **hastes its allies** (BR73986) and **fires lightning bolts**
(BR78949). It glows. [Globals.c 1047-1048](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1047-L1048)
----
BR51587
**Pink Jelly.** Health 50, defense 0, accuracy 85, damage 1–3; no natural
regeneration; normal speeds. It is a **large** creature, **never sleeps** (BR67941), and
**splits in two whenever it is struck** (BR97180). [Globals.c 1049-1050](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1049-L1050)
----
BR20078
**Toad.** Health 18, defense 0, accuracy 90, damage 1–4; regenerates every 10 turns;
normal speeds. Its bite **causes hallucinations** (BR46956). [Globals.c 1051-1052](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1051-L1052)
----
BR20983
**Vampire Bat.** Health 18, defense 25, accuracy 100, damage 2–6; regenerates every
20 turns; **moves twice as fast as normal** (speed 50); normal attack speed. It **flies**
(BR39474), **moves erratically** (BR10411), and **recovers health from the damage it deals** (BR83324,
at the 90% monster rate). [Globals.c 1053-1054](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1053-L1054)
----
BR79742
**Arrow Turret.** Health 30, defense 0, accuracy 90, damage 2–6; no regeneration;
normal movement but **attacks slowly** (attack speed 250). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) turret that **will not follow between levels** (BR30757).
It **attacks at range with a physical projectile** (BR68263). [Globals.c 1055-1056](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1055-L1056)
----
BR26157
**Acid Mound.** Health 15, defense 10, accuracy 70, damage 1–3; regenerates every 5
turns; normal speeds. Its **touch corrodes armor** (BR80252), and **striking it corrodes the
attacker's weapon** (BR37585). [Globals.c 1057-1058](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1057-L1058)
----
BR72611
**Centipede.** Health 20, defense 20, accuracy 80, damage 4–12; regenerates every 20
turns; normal speeds. Its bite **causes weakness** (BR32139). [Globals.c 1059-1060](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1059-L1060)
----
BR58109
**Ogre.** Health 55, defense 60, accuracy 125, damage 9–13; regenerates every 20
turns; normal movement but **attacks at half rate** (attack speed 200). A **large**
creature that **pushes its victim backward** on a hit (BR88806) and **avoids corridors** while
hunting (BR48373). [Globals.c 1061-1062](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1061-L1062)
----
BR81290
**Bog Monster.** Health 55, defense 60, accuracy 5000 (it essentially never misses),
damage 3–4; regenerates every 3 turns; **moves at half speed** (speed 200); normal attack
speed. A **large** creature **bound to liquid** (BR60148) that **submerges** (BR65633), **moves
erratically** (BR10411), **flees near death** (BR51688), and **grabs and holds its victim** (BR54374).
[Globals.c 1063-1064](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1063-L1064)
----
BR30412
**Ogre Totem.** Health 70, defense 0, accuracy 0, damage 0; no regeneration; normal
movement but **attacks very slowly** (attack speed 400). An **immobile** (BR99362), **never-
sleeping** (BR67941), **inanimate** (BR21958) totem that **will not follow between levels** (BR30757)
and **cannot be entangled** (BR91557). It **heals its allies** (BR47059) and **slows its enemies**
(BR32475). It glows. [Globals.c 1065-1066](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1065-L1066)
----
BR83467
**Spider.** Health 20, defense 70, accuracy 90, damage 3–4; regenerates every 20
turns; normal movement but **attacks at half rate** (attack speed 200). It **spits
entangling webs** (BR18935), its bite **deals poison damage** (BR45824), **cannot be entangled**
itself (BR91557), **casts spells slowly** (BR39641), and **never fails to use its ability** (BR85956).
[Globals.c 1067-1068](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1067-L1068)
----
BR90875
**Spark Turret.** Health 80, defense 0, accuracy 100, damage 0 (no melee attack);
no regeneration; normal movement but **attacks slowly** (attack speed 150). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) turret that **will not follow between levels** (BR30757).
It **fires bolts of lightning** (BR78949). It glows. [Globals.c 1069-1070](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1069-L1070)
----
BR24460
**Wisp.** Health 10, defense 90, accuracy 100, damage 0 (it burns rather than
strikes); regenerates every 5 turns; normal speeds. It **flies** (BR39474), **moves
erratically** (BR10411), **never sleeps** (BR67941), **is immune to fire** (BR13523), **burns
unceasingly** (BR57968), and **dies if negated** (BR49476). Its touch **sets the victim alight**
(BR32322). It glows. [Globals.c 1071-1072](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1071-L1072)
----
BR37547
**Wraith.** Health 50, defense 60, accuracy 120, damage 6–13; regenerates every 5
turns; **moves twice as fast as normal** (speed 50); normal attack speed. A **large**
creature that **flees near death** (BR51688). [Globals.c 1073-1074](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1073-L1074)
----
BR87706
**Zombie.** Health 80, defense 0, accuracy 120, damage 7–12; **no natural
regeneration**; normal speeds. A **large** creature that has a 100% chance each turn to
emit a puff of rot gas. [Globals.c 1075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1075)
----
BR69089
**Troll.** A troll regains 1 health every turn. [Globals.c 1076-1077](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1076-L1077)
----
BR16799
**Ogre Shaman.** Health 45, defense 40, accuracy 100, damage 5–9; regenerates every
20 turns; normal movement but **attacks at half rate** (attack speed 200). A **large**
creature that **summons allies** (BR40544), **keeps its distance** (BR72064), **avoids corridors**
(BR48373), and **casts spells slowly** (BR39641); it **hastes its allies** (BR73986) and **fires
lightning bolts** (BR78949). [Globals.c 1078-1079](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1078-L1079)
----
BR98084
**Naga.** Health 75, defense 70, accuracy 150, damage 7–11; regenerates every 10
turns; normal speeds. A **large** creature that always leaves a puddle where it dies.
[Globals.c 1080-1081](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1080-L1081)
----
BR73698
**Salamander.** Health 60, defense 70, accuracy 150, damage 5–11; regenerates every
10 turns; normal speeds. A **large** creature that **is immune to fire** (BR13523), **submerges**
(BR65633), **never sleeps** (BR67941), and **burns unceasingly** (BR57968). It **strikes up to two
enemies in a line** (BR43379) and always leaves salamander flame where it dies. It glows.
[Globals.c 1082-1083](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1082-L1083)
----
BR98594
**Explosive Bloat.** Health 10, defense 0, accuracy 100, damage 0; regenerates 1 health
every 5 turns; normal speeds. It glows.
[Globals.c 1084-1085](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1084-L1085)
----
BR44680
**Dar Blademaster.** A Dar Blademaster regains 1 health every 20 turns.
[Globals.c 1086-1087](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1086-L1087)
----
BR15683
**Dar Priestess.** Health 20, defense 60, accuracy 100, damage 2–5; regenerates every
20 turns; normal speeds. She **keeps her distance** (BR72064), **avoids corridors** (BR48373), and
carries an item a quarter of the time (BR53451). She **strips magical effects with negation**
(BR81389), **heals allies** (BR47059), **hastes allies** (BR73986), and **fires lightning bolts**
(BR78949). [Globals.c 1088-1089](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1088-L1089)
----
BR88319
**Dar Battlemage.** Health 20, defense 60, accuracy 100, damage 1–3; regenerates
every 20 turns; normal speeds. He **keeps his distance** (BR72064), **avoids corridors** (BR48373),
and carries an item a quarter of the time (BR53451). He **hurls fire bolts** (BR60584), **slows
enemies** (BR32475), and **casts discord** to turn enemies against each other (BR68026).
[Globals.c 1090-1091](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1090-L1091)
----
BR29613
**Acidic Jelly.** Health 60, defense 0, accuracy 115, damage 2–6; no natural
regeneration; normal speeds. A **large** creature whose touch **corrodes armor** (BR80252),
which **corrodes weapons that strike it** (BR37585), and which **splits in two whenever it is
struck** (BR97180). [Globals.c 1092-1093](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1092-L1093)
----
BR68715
**Centaur.** Health 35, defense 50, accuracy 175, damage 4–8; regenerates every 20
turns; **moves twice as fast as normal** (speed 50); normal attack speed. A **large**
creature that **keeps its distance** (BR72064) and **attacks at range with arrows** (BR68263).
[Globals.c 1094-1095](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1094-L1095)
----
BR52291
**Underworm.** Health 80, defense 40, accuracy 160, damage 18–22; regenerates every 3
turns; **moves at two-thirds of normal speed** (speed 150) and **attacks at half rate** (attack
speed 200). A **large** creature that **never sleeps** (BR67941).
[Globals.c 1096-1097](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1096-L1097)
----
BR20321
**Sentinel.** Health 50, defense 0, accuracy 0, damage 0; no regeneration; normal
movement but **attacks slowly** (attack speed 175). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) sentinel that **will not follow between levels** (BR30757),
**casts spells slowly** (BR39641), and **dies if negated** (BR49476). It **casts a healing spell on allies**
(BR47059) and **fires bolts of lightning** (BR78949). It glows. [Globals.c 1098-1099](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1098-L1099)
----
BR53622
**Dart Turret.** Health 20, defense 0, accuracy 140, damage 1–2; no regeneration;
normal movement but **attacks slowly** (attack speed 250). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) turret that **will not follow between levels** (BR30757).
It **fires poison darts** (BR88052) and its sting **causes weakness** (BR32139).
[Globals.c 1100-1101](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1100-L1101)
----
BR88545
**Kraken.** Health 120, defense 0, accuracy 150, damage 15–20; **regenerates rapidly**
(1 health every turn); **moves twice as fast as normal** (speed 50); normal attack speed.
A **large** creature **bound to liquid** (BR60148) that **moves at full speed in water** (BR47597),
**submerges** (BR65633), **moves erratically** (BR10411), **never sleeps** (BR67941), and **flees near
death** (BR51688). It **grabs and holds its victim** (BR54374). [Globals.c 1102-1103](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1102-L1103)
----
BR62417
**Lich.** Health 35, defense 80, accuracy 175, damage 2–6; **no natural
regeneration**; normal speeds. A **large** creature that **keeps its distance** (BR72064), carries an
item a quarter of the time (BR53451), and **never results from polymorphism** (BR12704). It **summons allies**
(BR40544) and **hurls fire bolts** (BR60584). It glows. [Globals.c 1104-1105](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1104-L1105)
----
BR18852
**Phylactery.** Health 30, defense 0, accuracy 0, damage 0; no regeneration; normal
movement but **attacks slowly** (attack speed 150). An **immobile** (BR99362), **never-
sleeping** (BR67941), **inanimate** (BR21958), **always-hunting** (BR46423) receptacle that **will not
follow between levels** (BR30757), cannot be entangled (BR91557), and **dies if negated** (BR49476).
It **summons allies** (BR40544) and **harbours its master inside a minion, freeing the lich
when that minion is destroyed** (BR70715). It glows. [Globals.c 1106-1107](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1106-L1107)
----
BR89674
**Pixie.** Health 10, defense 90, accuracy 100, damage 1–3; regenerates every 20
turns; **moves twice as fast as normal** (speed 50); normal attack speed. It **flies**
(BR39474), **moves erratically** (BR10411), and **keeps its distance** (BR72064). It **strips magical
effects with negation** (BR81389), **slows enemies** (BR32475), **casts discord** (BR68026), and
**fires lightning bolts** (BR78949). It glows. [Globals.c 1108-1109](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1108-L1109)
----
BR13994
**Phantom.** Health 35, defense 70, accuracy 160, damage 12–18; **no natural
regeneration**; **moves twice as fast as normal** (speed 50) and **attacks at half rate**
(attack speed 200). A **large**, **invisible** (BR56920) creature that **flies** (BR39474), **moves
erratically** (BR10411), and **cannot be entangled** (BR91557); it sheds a droplet of ectoplasm 2%
of turns. [Globals.c 1110-1111](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1110-L1111)
----
BR61627
**Flame Turret.** Health 40, defense 0, accuracy 150, damage 1–2; no regeneration;
normal movement but **attacks slowly** (attack speed 250). An **immobile** (BR99362),
**never-sleeping** (BR67941), **inanimate** (BR21958) turret that **will not follow between levels** (BR30757).
It **hurls fire bolts** (BR60584). It glows. [Globals.c 1112-1113](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1112-L1113)
----
BR69414
**Imp.** Health 35, defense 90, accuracy 225, damage 4–9; regenerates 1 health every 10
turns; normal speeds. It glows. [Globals.c 1114-1115](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1114-L1115)
----
BR51639
**Fury.** Health 19, defense 90, accuracy 200, damage 6–11; regenerates every 20
turns; **moves twice as fast as normal** (speed 50); normal attack speed.
[Globals.c 1116-1117](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1116-L1117)
----
BR55764
**Revenant.** Health 30, defense 0, accuracy 200, damage 15–20; **no natural
regeneration**; normal speeds. A **large** creature that is **immune to weapons** (BR95842).
[Globals.c 1118-1119](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1118-L1119), [Combat.c 1098-1100](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1098-L1100), [Items.c 6054-6056](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6054-L6056)
----
BR51040
**Tentacle Horror.** Health 120, defense 95, accuracy 225, damage 25–35; **regenerates
rapidly** (1 health every turn); normal speeds. A **large** brute with no special traits.
[Globals.c 1120](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1120)
----
BR86187
**Golem.** Health 400, defense 70, accuracy 225, damage 4–8; **no natural
regeneration**; normal speeds. A **large** creature that **reflects about half of magical
bolts** (BR18737) and **dies if negated** (BR49476). [Globals.c 1121-1122](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1121-L1122)
----
BR33998
**Dragon.** Health 150, defense 90, accuracy 250, damage 25–50; regenerates every 20
turns; **moves twice as fast as normal** (speed 50) and **attacks at half rate** (attack
speed 200). A **large** creature that **is immune to fire** (BR13523), **always carries an
item** (BR53451), **breathes dragonfire** (BR77502), and **strikes every adjacent enemy at once**
(BR97260). [Globals.c 1123-1124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1123-L1124)
----
BR59785
**Goblin Warlord.** Health 30, defense 17, accuracy 100, damage 3–6; regenerates
every 20 turns; normal speeds. It **keeps its distance** (BR72064), carries an item a quarter
of the time (BR53451), **summons allies** (BR40544), **attacks with penetration to a second enemy
in a line** (BR43379), and **avoids corridors** (BR48373).
[Globals.c 1127-1128](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1127-L1128)
----
BR86600
**Black Jelly.** Health 120, defense 0, accuracy 130, damage 3–8; no natural
regeneration; normal speeds. A **large** creature that **splits in two whenever it is
struck** (BR97180). [Globals.c 1129-1130](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1129-L1130)
----
BR78289
**Vampire.** Health 75, defense 60, accuracy 120, damage 4–15; regenerates every 6
turns; **moves twice as fast as normal** (speed 50); normal attack speed; a **large**
creature. [Globals.c 1131-1132](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1131-L1132)
----
BR56088
**Flamedancer.** Health 65, defense 80, accuracy 120, damage 3–8; **no natural
regeneration**; normal speeds. A **large** creature that **keeps its distance** (BR72064),
**is immune to fire** (BR13523), and **burns unceasingly** (BR57968). Its touch **sets the victim alight**
(BR32322), it **hurls fire bolts** (BR60584), and it always wears a corona of flame where it dies. It glows.
[Globals.c 1133-1134](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1133-L1134)
----
BR18869
**Spectral Blade.** Health 1, defense 0, accuracy 150, damage 1; no regeneration;
**moves twice as fast as normal** (speed 50); normal attack speed. An **inanimate** (BR21958),
**never-sleeping** (BR67941), **flying** (BR39474) blade that **will not follow between levels** (BR30757),
**cannot be entangled** (BR91557), **dies if negated** (BR49476), and is **not listed in the sidebar**
(BR48675). It glows. [Globals.c 1137-1138](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1137-L1138)
----
BR67777
**Spectral Sword.** Health 1, defense 0, accuracy 150, damage 1; no regeneration;
**moves twice as fast as normal** (speed 50); normal attack speed. An **inanimate** (BR21958),
**never-sleeping** (BR67941), **flying** (BR39474) sword that **will not follow between levels** (BR30757),
**cannot be entangled** (BR91557), and **dies if negated** (BR49476). It glows.
[Globals.c 1139-1140](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1139-L1140)
----
BR41375
**Stone Guardian.** Health 1000, defense 0, accuracy 200, damage 12–17; no
regeneration; normal speeds. An **inanimate** (BR21958), **never-sleeping** (BR67941),
**always-hunting** (BR46423) guardian that **will not follow between levels** (BR30757), is **immune to
weapons** (BR95842) and **fire** (BR13523), **dies if negated** (BR49476), **never fails to use its special
ability** (BR85956), and springs to life only when its machine is triggered (BR16717). It **reflects all
magical bolts back at the caster** (BR56605). [Globals.c 1141-1142](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1141-L1142)
----
BR23479
**Winged Guardian.** Health 1000, defense 0, accuracy 200, damage 12–17; no
regeneration; normal speeds. Like the stone guardian it is **inanimate** (BR21958), **never
sleeps** (BR67941), **always hunts** (BR46423), **is immune to weapons** (BR95842) and **fire** (BR13523),
**will not follow between levels** (BR30757), **dies if negated** (BR49476), **never fails to use its special
ability** (BR85956), and springs to life only when its machine is triggered (BR16717). It **reflects all
magical bolts back at the caster** (BR56605) and **blinks** (BR16647).
[Globals.c 1143-1144](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1143-L1144)
----
BR48053
**Guardian Spirit.** Health 1000, defense 0, accuracy 200, damage 5–12; no
regeneration; normal speeds. An **inanimate** (BR21958), **never-sleeping** (BR67941) spirit that is
**immune to weapons** (BR95842) and **fire** (BR13523), **dies if negated** (BR49476), and **never fails to
use its special ability** (BR85956). It **reflects all magical bolts back at the caster** (BR56605). It glows.
[Globals.c 1145-1146](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1145-L1146)
----
BR27618
**Warden of Yendor.** Health 1000, defense 0, accuracy 300, damage 12–17; no
regeneration; **moves half as fast as normal** (speed 200) and **attacks at half rate**
(attack speed 200). A **large**, **never-sleeping** (BR67941), **always-hunting** (BR46423) boss
that is **invulnerable** (BR90225) — it cannot be harmed at all — and **cannot be
polymorphed** (BR12704). It glows. [Globals.c 1147-1148](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1147-L1148)
----
BR17604
**Eldritch Totem.** Health 80, defense 0, accuracy 0, damage 0; no regeneration;
normal movement and attack speed. An **immobile** (BR99362), **never-sleeping** (BR67941),
**inanimate** (BR21958), **always-hunting** (BR46423) totem that **will not follow between levels** (BR30757),
**cannot be entangled** (BR91557), **never fails to use its special ability** (BR85956), and springs to life
only when its machine is triggered (BR16717). It **summons allies** (BR40544).
[Globals.c 1149-1150](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1149-L1150)
----
BR89786
**Mirrored Totem.** Health 80, defense 0, accuracy 0, damage 0; no regeneration;
normal movement and attack speed. An **immobile** (BR99362), **never-sleeping** (BR67941),
**inanimate** (BR21958), **always-hunting** (BR46423) totem that **will not follow between levels** (BR30757),
**cannot be entangled** (BR91557), is **immune to weapons** (BR95842) and **fire** (BR13523), **never fails to
use its special ability** (BR85956), and springs to life only when its machine is triggered (BR16717). It
**reflects all magical bolts back at the caster** (BR56605) and **beckons the target toward it** (BR71983).
[Globals.c 1151-1152](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1151-L1152)
----
BR12594
**Unicorn.** Health 40, defense 60, accuracy 175, damage 2–10; regenerates every 20
turns; **moves twice as fast as normal** (speed 50); normal attack speed. A **large**
creature that **keeps its distance** (BR72064), **casts a shielding spell on allies** (BR53754),
and **casts a healing spell on allies** (BR47059). It always leaves a single dung where it dies. It glows.
[Globals.c 1155-1156](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1155-L1156)
----
BR83759
**Ifrit.** Health 40, defense 75, accuracy 175, damage 5–13; **regenerates rapidly**
(1 health every turn); **moves twice as fast as normal** (speed 50); normal attack speed.
A **large** creature that **is immune to fire** (BR13523) and **flies** (BR39474), and that
**casts discord** (BR68026). It glows. [Globals.c 1157-1158](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1157-L1158)
----
BR64166
**Phoenix.** Health 30, defense 70, accuracy 175, damage 4–10; **no natural
regeneration**; **moves twice as fast as normal** (speed 50); normal attack speed. A **large**
creature that **is immune to fire** (BR13523), **flies** (BR39474), and **never results from polymorphism** (BR12704).
It glows. [Globals.c 1159-1160](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1159-L1160)
----
BR98992
**Phoenix Egg.** A phoenix egg has 50 health.
[Globals.c 1161](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1161)
----
BR57045
**Mangrove Dryad.** Health 70, defense 60, accuracy 175, damage 2–8; regenerates
every 6 turns; normal speeds. A **large** creature.
[Globals.c 1163-1164](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1163-L1164)

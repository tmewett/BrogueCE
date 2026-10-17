- **BR100001** — When a monster's melee attack reduces the player to zero health, the player dies and the game ends, attributed to that monster.
  [Combat.c:1184-1191](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1184-L1191)
- **BR100002** — When a monster's bolt spell reduces the player to zero health, the player dies and the game ends, attributed to that monster.
  [Monsters.c:2768-2773](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2768-L2773)
- **BR100003** — When a reflected bolt reduces the player to zero health, the player dies and the game ends, attributed to the reflected bolt.
  [Items.c:4418-4426](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4418-L4426)
- **BR100004** — When a cursed weapon's life-draining effect reduces the player to zero health, the player dies and the game ends, drained by the cursed weapon.
  [Combat.c:1582-1586](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1582-L1586)
- **BR100005** — When the player's nutrition reaches zero, the player slowly starves, losing one health per turn until death.
  [Time.c:2276-2282](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2276-L2282)
- **BR100006** — When the player is poisoned, the poison deals damage each turn and can kill the player.
  [Time.c:2338-2343](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2338-L2343)
- **BR100007** — When the player is burning, the fire deals damage each turn and can kill the player.
  [Time.c:2328-2332](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2328-L2332)
- **BR100008** — When the player steps into lava without levitation, fire immunity, or invulnerability, the player is consumed instantly.
  [Time.c:170-188](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L170-L188)
- **BR100009** — When the player stands on explosive terrain, the explosion deals heavy damage and can kill the player.
  [Time.c:316-329](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L316-L329)
- **BR100010** — When the player stands on damaging terrain, the terrain deals damage each turn and can kill the player.
  [Time.c:505-526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L505-L526)
- **BR100011** — When the player falls down a pit or shaft and the impact damage is fatal, the player dies from the fall.
  [Time.c:1016-1024](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1016-L1024)

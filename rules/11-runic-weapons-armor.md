- **BR110001** — A weapon's runic effect activates on a hit with a chance that rises with the weapon's enchantment.
  [Combat.c:618-622](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L618-L622) [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110016** — A weapon's runic activation chance is lower for weapons with high base damage.
  [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110017** — A weapon's runic activation chance is adjusted by attack speed: slower weapons get a higher per-hit chance and weapons striking twice per turn get a reduced chance.
  [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110018** — A weapon's runic activation chance is never lower than the weapon's enchantment level nor higher than 100%.
  [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110019** — Weak or cursed runes activate at a flat 15%.
  [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110020** — A slaying rune has no random activation chance at all.
  [PowerTables.c:220-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L220-L345)
- **BR110002** — A speed rune grants the player an immediate extra turn.
  [Combat.c:645-654](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L645-L654)
- **BR110003** — A quietus rune sometimes instantly kills the struck creature.
  [Combat.c:655-665](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L655-L665)
- **BR110004** — A paralysis rune freezes the struck creature for a duration growing with enchantment.
  [Combat.c:666-675](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L666-L675)
- **BR110005** — A multiplicity weapon rune summons spectral duplicates of the player.
  [Combat.c:676-741](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L676-L741)
- **BR110006** — A force rune knocks the struck creature backward, dealing impact damage if stopped short.
  [Combat.c:498-589](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L498-L589)
- **BR110007** — A slaying rune always hits and instantly kills creatures of its designated kind.
  [Combat.c:613-614](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L613-L614) [Combat.c:655-665](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L655-L665)
- **BR110008** — A mercy rune heals the creature it strikes instead of harming it.
  [Combat.c:764-769](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L764-L769)
- **BR110009** — A plenty rune creates a duplicate of the struck creature.
  [Combat.c:770-778](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L770-L778)
- **BR110010** — Weapon runes other than slaying never trigger against inanimate or invulnerable creatures.
  [Combat.c:615-616](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L615-L616)
- **BR110011** — An armor of immunity negates incoming bolts of its designated type.
  [Items.c:4214-4220](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4214-L4220)
- **BR110012** — An armor of reflection bounces bolts back at the caster.
  [Items.c:4221-4228](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4221-L4228)
- **BR110013** — An armor of respiration protects against damaging gases and liquids.
  [Time.c:506-512](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L506-L512)
- **BR110014** — An armor of dampening absorbs explosive damage.
  [Time.c:320-326](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L320-L326)
- **BR110015** — Armor reveals its runic nature after several turns of wear.
  [Items.c:285-296](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L285-L296)
- **BR110021** — A runic weapon or armor identifies its runic nature when its runic effect actually triggers in combat.
  [Items.c:5994-6003](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L5994-L6003)
- **BR110022** — A runic armor's effect can auto-identify the armor when it triggers (for example, reflection).
  [Items.c:4953-4960](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4953-L4960)

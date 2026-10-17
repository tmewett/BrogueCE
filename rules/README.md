# Brogue — Natural-Language Rules

This directory captures Brogue CE's internal game logic as natural-language rules. Each rule
describes an in-game behavior in one or two sentences — without naming data structures, functions,
or modules — and cites the source lines where that behavior is implemented.

## Reference format

References are pinned to a specific commit so they cannot go stale. All links point at commit
`104a20fedbe7d9360d2dfc8ff665571694212ad8` (`master` at the time of writing):

```
https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/<file>#L<start>-L<end>
```

Only files under `src/brogue/` are cited.

## Conventions

- **One or two sentences** per rule, describing observable in-game behavior.
- **No code identifiers**: no function names, struct fields, enum constants, or module names in the
  rule text. (References are allowed to name the file, since that is only the citation.)
- **Exact numbers** are included where the game uses them (e.g. "triple damage", "500 gold",
  "half the usual injury").
- Each rule has a globally unique ID of the form `BR#####` (for example `BR39771`).
- A rule may carry multiple citations when the behavior spans several code sites.
- Citations are written as `[File.c:start-end](permalink)`.

## Files

1191 rules total.

| File | Rules | Subsystem | Source files |
| --- | ---: | --- | --- |
| [01-movement-environment.md](01-movement-environment.md) | 177 | Movement, terrain, hazards, turn/environment updates | `Movement.c`, `Time.c` |
| [02-combat.md](02-combat.md) | 77 | Attacks, damage, accuracy, defense, weapon/armor effects | `Combat.c`, `PowerTables.c` |
| [03-status-effects.md](03-status-effects.md) | 134 | Status effects: application, duration, curing, interactions | `Rogue.h`, `Time.c`, `Monsters.c`, `Combat.c`, `Items.c`, `Movement.c` |
| [04-monsters.md](04-monsters.md) | 190 | Monster AI, behavior states, abilities, allies | `Monsters.c` |
| [05-items.md](05-items.md) | 165 | Items, inventory, targeting, enchanting, identification | `Items.c` |
| [06-progression-score.md](06-progression-score.md) | 66 | Character creation, victory, scoring, difficulty, saving | `RogueMain.c`, `Wizard.c` |
| [07-player-interaction.md](07-player-interaction.md) | 151 | Confirmations, targeting UI, messages, sidebar, recording/playback | `IO.c`, `Buttons.c`, `MainMenu.c`, `Recordings.c` |
| [08-dungeon-generation.md](08-dungeon-generation.md) | 101 | Level generation, rooms, vaults, connectivity, lighting | `Architect.c`, `Grid.c`, `Dijkstra.c`, `Light.c` |
| [09-monsters-bestiary.md](09-monsters-bestiary.md) | 130 | Base-Brogue monsters: stats, innate traits, special attacks, spells | `Globals.c`, `Rogue.h`, `Monsters.c`, `Combat.c`, `Movement.c` |
| [10-player-death.md](10-player-death.md) | 11 | Ways the player can die | `Combat.c`, `Monsters.c`, `Items.c`, `Time.c` |

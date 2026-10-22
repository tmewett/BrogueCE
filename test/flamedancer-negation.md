# Flamedancer negation

Manual check for the flamedancer flame aura. `test/run_regression_tests.py` only replays `.broguerec` files, and this checkout has none. The results below were measured on a wizard-mode depth-1 level of seed 1 by calling `negate()` (the function used by the negation bolt and by `negationBlast()`) and `monsterDetails()` (the function used by the examine box).

## Build

```
make GRAPHICS=NO bin/brogue
make DEBUG=YES TERMINAL=YES bin/brogue
bin/brogue --term --wizard -n -s 1
```

`make GRAPHICS=NO bin/brogue` and `make DEBUG=YES TERMINAL=YES bin/brogue` both completed. Seed 1 enters depth 1 with the player at (38, 26). Wizard mode starts with an identified wand of negation. Depth 1 of this seed has no natural flamedancer. The shared catalog entry stays `DFChance == 100`, `DFType == 37` (`DF_FLAMEDANCER_CORONA`), flags `0x421000` (`MONST_IMMUNE_TO_FIRE | MONST_FIERY | MONST_MAINTAINS_DISTANCE`).

In the terminal build: `C` creates a monster, choose the page that contains `flamedancer`, choose `flamedancer`, choose `No mutation`, pick a visible floor tile, and decline the ally prompt. Enter opens the cursor; move it onto the creature to read the infobox. `a` applies the wand of negation. A debugger breakpoint on `negate` can set `monst->status[STATUS_BURNING] = 0` (`status[9]`) and then finish the call.

## Before negation

A normal flamedancer is born burning (`STATUS_BURNING == 1000`, `STATUS_IMMUNE_TO_FIRE == 1000`) with `DFChance == 100` and `DFType == 37`.

With the wand of negation in the pack, the examine text lists the aura once, inside the removable-traits sentence, and the non-negatable sentence omits it:

```
The flamedancer does not regenerate.
The flamedancer also has special traits that can be removed by a wand of negation: it can hurl gouts of flame, lights enemies on fire when it hits, is immune to fire, burns unceasingly, keeps its distance and radiates an aura of flame.
```

With no negation gear in the pack, the ordinary sentence also lists it once, with a comma between items and "and" before the last:

```
The flamedancer does not regenerate, can hurl gouts of flame, lights enemies on fire when it hits, is immune to fire, burns unceasingly, keeps its distance and radiates an aura of flame.
```

A paralyzed flamedancer at (12, 13) had 0 `FLAMEDANCER_FIRE` tiles within 5 spaces. Four player turns later it was still there and 10 corona tiles were present. `DFChance`, `DFType`, and the fiery and fire-immunity flags were unchanged.

## Negate a burning flamedancer

`negate()` on that creature returned true. After the call:

- `info.DFChance == 0`
- `info.DFType == 0`
- `info.flags == 0` (fiery and fire immunity cleared)
- `STATUS_BURNING == 0` and `STATUS_IMMUNE_TO_FIRE == 0`
- `wasNegated == true`
- the fire bolt was removed

The examine text no longer contains "radiates an aura of flame". It ends with "The flamedancer is stripped of its special traits." and "The flamedancer does not regenerate."

The 10 corona tiles at the old site remained. Moved to clean floor at (35, 4), four further turns created 0 new `FLAMEDANCER_FIRE` tiles there, and burning stayed 0. Negation leaves existing terrain fire in place.

A second, unnegated flamedancer at (12, 3) kept `DFChance == 100`, `DFType == 37`, flags `0x421000`, and `STATUS_BURNING == 1000`. The catalog entry was unchanged.

## Burning status already zero

On a flamedancer at (47, 24), `STATUS_BURNING` was set to 0 and `negate()` still returned true and cleared the generator (`DFChance == 0`, `DFType == 0`, fiery and fire immunity cleared, burning stayed 0). A second `negate()` returned false and left those fields as they were. The aura did not return, and the examine text did not advertise it.

## Other creatures

- Salamander at (56, 25): `negate()` cleared fiery and fire immunity and left `DFChance == 100`, `DFType == 45` (`DF_SALAMANDER_FLAME`).
- Explosive flamedancer at (63, 16): the mutation replaced the feature with `DFChance == 0`, `DFType == 38` (`DF_MUTATION_EXPLOSION`). `negate()` cleared the mutation index and the fiery flags and left that feature in place.
- Will-o-the-wisp at (67, 24): `negate()` killed it (`MB_IS_DYING`) and left `DFChance == 0`, `DFType == 0`.
- Flamedancer with `MONST_INVULNERABLE` set, placed at (12, 13): `negate()` returned true because ability flags are still stripped before the invulnerable branch, and the corona stayed (`DFChance == 100`, `DFType == 37`, fiery flag still set, flags `0x461000`).
- `negate(&player)` returned false, left the player's `DFChance` and `DFType` at 0, and did not end the game.

## Replays

No recording fixtures were in this checkout, and none were fetched, so `test/run_regression_tests.py` was not run. Clearing `DFChance` skips the `rand_percent` draw on later environment ticks. A replay from before this change that negates a flamedancer and continues can diverge there. Replays that never negate a corona-bearing creature still draw while `DFChance` is 100. The recording version was left unchanged.

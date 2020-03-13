Unreleased
==========
-
  Adding the `--server-mode` command line parameter used to run the game from an external launcher
-
  Restored inventory on right click functionality
-
  Water-restricted creatures can no longer jump back into water when on adjacent
  dry land


1.8.1
=====

This release contains some bug fixes and UI improvements. Replays and saves are
fully compatible with 1.8.

Thanks to contributors animal_waves and flend.


Bug fixes
---------
-
  Fixed a playback out-of-sync error when walking into a secret door to
  discover it
-
  Fixed a bug where inspecting an out-of-sight lumenstone would say "you
  remember seeing a lumenstone from depth 0" instead of the depth it was found
  at
-
  Item quantities are now remembered correctly on leaving and revisiting a level
-
  Fixed description of remembered items when hallucinating


Other changes
-------------
-
  Recordings are now sorted by date descending (newest on top) and all dates
  (high scores, saved games, recordings) are now displayed as "YYYY-MM-DD"
-
  Fixed an issue with ring descriptions where an unidentified ring could be
  revealed as negative after reading a scroll of remove curse
-
  Added F12 and Alt+Enter as shortcuts for full-screen mode


1.8
===

This is the first release of *Brogue: Community Edition*! This update focuses on
addressing the most common community complaints with 1.7.5 -- most notably,
the balancing of allies. However, there are many other changes which we hope
will improve the game.


Gameplay changes
----------------

-
  Wands of empowerment are now guaranteed to spawn in early vaults
-
  Wands of empowerment have been strengthened to a middle-ground between
  their 1.7.4 and 1.7.5 versions
-
  Wands of empowerment no longer increase the target's health regeneration rate
-
  The frequencies of various special ally-related rooms have been reverted
  to their 1.7.4 values
-
  Captive allies have been made more common
-
  Wands of plenty now reduce the the maximum health of the target and its
  clone by 50%
-
  Hidden doors and levers are now revealed when attempting to move into
  them. (This addresses an issue where players had to waste turns taking
  damage when trapped up against an obvious secret door by a monster)
-
  Goblin conjurers no longer have the spear attack pattern in contradiction
  with their attack message
-
  Magic-detected cells are now described with "you remember seeing <item> here"
  when the item has been seen
-
  Changed the creatures hit as collateral from a spear attack to be the same
  as those hit by the sweep of an axe attack. (This reincludes hitting hidden
  monsters)


Bug fixes
---------

-
  Allies can no longer learn abilities from the spectral clones created by
  armor of multiplicity
-
  Fixed incorrect percentages in the ring of wisdom tooltip
-
  Excessive input during playback no longer causes long pauses
-
  Fixed incorrect percentage health change info in sidebar
-
  Low health alerts no longer show on every turn when using autopilot or
  watching replays
-
  Fixed an issue causing the damage of staffs of firebolt and lightning to
  always be the average/expected value and have no variation
-
  Fixed many more causes of out-of-sync errors
-
  Fixed a bug which in some cases caused traps to be generated on cells with
  foliage, leading to odd behaviour
-
  Fixed a bug which in some cases caused staffs to appear in treasure vaults
  without their max charges being shown


Other changes
-------------

-
  The Mac version has been remade to be the same as the Windows and Linux
  versions. (Unfortunately, this means it has lost some Mac-specific polish,
  e.g. it is no longer an App. This will be improved in future releases)
-
  Turning on autopilot now requires confirmation
-
  The "blue" player-to-monster combat information is now displayed in ally
  tooltips (so you can more easily assess how much health they have)
-
  The sidebar now displays whether a monster is carrying an item
-
  The maximum seed able to be input in the main menu is now the same on
  all platforms (2^32 - 1 = 4294967295)
-
  Added a `--print-seed-catalog` command line option to recreate the seed
  catalog
-
  Various minor UI tweaks and bug fixes


1.7.5-flend1
============

This patchset to 1.7.5 incorporates all the gameplay-related changes made
by flend in WebBrogue and their fixed Windows build. The changes have been
implemented in the same way for compatibility.


Gameplay changes
----------------

-
  Changed which monsters are hit when spear attacking. Previously, all monsters
  on the attacked cells would be hit, including allies. Now the monsters hit are
  only those that would trigger a spear attack if they were there alone. (This
  excludes hidden monsters)


Bug fixes
---------

-
  Fixed charm durations which were affected by bugs
-
  Monsters now have a 3% chance per turn to forget you, instead of 97%
-
  Reading a scroll of aggravate monsters no longer crashes the game
-
  Reverted some game calculations to the old math system. In particular,
  this fixes the weird potion of darkness FOV and the incorrect damage done
  by speed runics
-
  Staff of healing and ring of wisdom now show correct tooltip information
-
  Knockback from weapons of force now correctly damages creatures
-
  Fixed staff of firebolt/lightning damage and staff of haste duration values
-
  Fixed radii of charms of negation and shattering, and lifespan of guardian charm
-
  Various items no longer malfunction at high enchant levels
-
  Creatures splitting on depth 1 no longer crash the game
-
  Mutated jellies now properly keep mutations when splitting, and do not
  crash in some cases
-
  Throwing items no longer leaks memory

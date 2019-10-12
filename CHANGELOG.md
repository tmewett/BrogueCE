unreleased
==========

-
  Fixed an issue causing the damage of staffs of firebolt and lightning to
  always be the average/expected value and have no variation
-
  Fixed many more causes of out-of-sync errors
-
  The maximum seed able to be input in the main menu is now the same on
  all platforms (2^32 - 1 = 4294967295)
-
  Fixed a bug which in some cases caused traps to be generated on cells with
  foliage, leading to odd behaviour
-
  Fixed a bug which in some cases caused staffs to appear in treasure vaults
  without their max charges being shown
-
  Goblin conjurers no longer have the spear attack pattern in contradiction
  with their attack message
-
  Magic-detected cells are now described with "you remember seeing ITEM here"
  when the item has been seen
-
  Changed the creatures hit as collateral from a spear attack to be the same
  as those hit by the sweep of an axe attack. (This reincludes hitting hidden
  monsters)


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
-
  Tweaked some charm stats to match 1.7.4 behaviour


Bug fixes
---------

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

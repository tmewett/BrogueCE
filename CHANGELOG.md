1.9
===

A new gameplay version! Not compatible with replays and saves from v1.8.x.

In this release, we tweaked or retried some of the changes made in v1.8 around
allies and searching. We believe they are closer in spirit to the original game
and represent a better compromise. There are also some new balance changes, and
a lot of bug fixes.

Thanks to everyone who has contributed code, bug reports, or feedback since
v1.8. Enjoy!

Gameplay changes
----------------
-
  The wand of empowerment is no longer guaranteed in vaults; a random wand is
  spawned instead
-
  Revamped the searching system. Instead of performing a strong search only
  after five consecutive turns of pressing 's', you now perform a weaker,
  single-turn search every time you press 's', with a stronger one on the fifth.
  (Control+s will perform five searches, stopping if interrupted, just like old
  'S'.)
-
  Walking into secret doors and levers no longer reveals them
-
  Buffed staff of protection duration. At /N max charges, the duration is now 13
  x 1.4^(N-2) instead of 5 x 1.53^(N-2)
-
  Nerfed charm of teleportation recharge time. At +1 it starts at the same
  value, but becomes 1 turn at +13 instead of +11
-
  Dar priestesses are now included in the 'Mage' monster class. A weapon of mage
  slaying will instantly kill them, and armor of mage immunity will provide
  invulnerability to their feeble attacks.

Bug fixes
---------
-
  Liches/phoenixes polymorphed into other creatures no longer spawn
  phylacteries/eggs on death
-
  Fixed allies not having a lighter colour tint (this was broken by the addition
  of tiles in 1.8.3)
-
  Fixed a bug where discordant allies did not attack the player diagonally
-
  Fixed a dungeon generation issue which caused the key pedestal in
  falling-torch key rooms to be non-flammable
-
  Fix Ctrl+x (fast explore), Ctrl+A (fast autopilot), and Ctrl+{vi keys}
  movement not working
-
  Fixed an issue where fast-attacking monsters could attack the player before
  falling down a chasm or hole
-
  Fixed more causes of out-of-sync errors. (We think these are now very rare, so
  please let us know if you get one)
-
  Fixed the tile graphics for lumenstones and turrets being swapped
-
  Fixed monsters being shown as letters when hallucinating with graphics enabled

Other changes
-------------
-
  Numpad 5 (center button) no longer confirms when selecting a target
-
  Debug mode, or "wizard mode," is now available by default. Start the game with
  the `-W`/`--wizard` command-line option to play with teleportation,
  immortality and overpowered items


1.8.3
=====

This release contains integration of Oryx's tiles, along with other UI and
quality of life improvements. Gameplay is identical to all 1.8 versions.

Thanks to contributors kzved and flend.


Bug fixes
---------
-
  Fixed a bug where watching a replay to completion would cause a duplicate high
  score to appear

Other changes
-------------
-
  Oryx's graphical tiles are now fully integrated. They can be enabled by
  pressing 'G' or by using the in-game menu. Compared to v1.8.1 tiles
  pre-release, this fixes some incorrect tiles being used for various game
  elements. (Note: unlike original Brogue Tiles, tiles are available at all
  window/font sizes, but due to scaling they only look crisp at a certain size.)
-
  Improved the game's appearance on high-PPI/DPI displays. Also added some
  larger font sizes for better viewing on higher resolution displays in general
-
  The full identified inventory is now shown on the post-victory screen
-
  Pressing the PrintScreen key now saves a screenshot to the save directory
-
  Numpad 5 (center) can be used to confirm targetting


1.8.2
=====

This release contains general improvements and some fixes to regressions caused
by CE. Gameplay is identical to all 1.8 versions.

Thanks to contributors kzved, flend, Vivit-R and zespri.


Bug fixes
---------
-
  Fixed the game screen going black after alt-tabbing while fullscreen on some
  platforms
-
  Fixed the weird scaling when going fullscreen on some platforms
-
  Fixed a rare bug on Linux where some item names would not show

Other changes
-------------
-
  The Mac release is now an app!
-
  Added a re-throw command (T), which throws the last item at the last-chosen
  monster
-
  The game window is now freely resizable, and the best-fitting font size is
  automatically chosen
-
  Restored right-click shortcut to show inventory
-
  (For developers) When building in debug mode, the cheats are now toggled by a
  `--wizard` command line option
-
  The WebBrogue back-end is now built-in, and can be enabled with a build option


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

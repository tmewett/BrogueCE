1.10.1
======

-
  Fixed an out-of-sync error when loading a game with a 64-bit seed.
-
  (For distributors) Added an experimental `--data-dir` option for specifying
  the location of game resources.


1.10
====

A new gameplay version is here! Features some new mechanics, rebalancing, UI
improvements, and bug fixes. Replays and saves are not compatible with 1.9.x.

CE's scope has been changing since we released 1.9. Originally, we weren't
considering making many original or notable changes to the gameplay itself. But
now that the project is more mature, and importantly Pender (Brian) is involved
in development, we are now open to these kind of changes. Other than that, the
project is just the same - community-focused and trying to make a great game
even better. Enjoy!

Thanks to many contributors: Andy Bayer, Antony Boucher, zenzombie, bleakley,
Byron Altice, Jon Knapp, Konstantin Mikhailov, Nathan Fenner, RojjaCebolla,
SanatMishra, anaseto, austen0, flend, pender, and withinwheels.

## Notable changes
-
  It is now possible to fall through chasms into deep water, without damage.
  Falling into shallow water or bog deals half the damage as hitting hard floor.
  Items falling through chasms may now land into deep water, another chasm, or
  lava.
-
  Rebalance Staff of Obstruction builds by (1) preventing crystals from
  persisting on cells occupied by creatures, and (2) allowing hunting monsters
  to remain hunting as long as they are close enough and can see the player,
  even if impassable transparent terrain is between them.
-
  Cautious monsters now enter corridors if they have been attacked recently and
  are below 50% health.
-
  Allies will no longer attack or cast spells at sacrifice targets.
-
  Aiming at a target now automatically aims for or avoids enemies/allies/walls,
  depending on the projectile type. This removes most of the need to aim for a
  cell beyond your intended target to get the best result. It also applies to
  spells cast by enemies!
-
  Increased average damage for flails by 0.5, and flails now have a 2.5% (up
  from 0%) chance to be runic.
-
  Added a sanctuary tile in front of spark turret vaults to prevent allies from
  destroying the turrets.
-
  Scrolls of sanctuary can now be used on brimstone and obsidian.
-
  Added a hybrid graphical mode that uses text for creatures and items but
  graphical tiles for everything else.
-
  The sidebar and monster details now prominently display that a monster has
  been affected by negation.
-
  The message archive is now larger, and can be navigated with the up and down
  arrow keys. Holding shift scrolls one line at a time, holding control jumps to
  the end.
-
  Pressing "w" will hot-swap between recently equipped gear, enabling
  weapon/armor/ring juggling.
-
  "Untempted" feat has been added, for picking up no gold.
-
  "Mystic" feat has been renamed to "Ascetic".

## Bug fixes and other changes
-
  The save game feature is now out of beta!
-
  Fixed a bug causing the game to crash when polymorphing a lich or phoenix.
-
  Seeds are now 64-bits, going up to 18446744073709551615! There are now 2
  billion times more dungeons to explore!
-
  Wands of negation auto-ID if the bolt has an effect. A combat message is shown
  for any monster affected by negation.
-
  When wearing known respiration armor, don't warn when stepping on immune gas
  traps.
-
  Staff descriptions now report the number of turns since their last use.
-
  Fixed a bug where summoned minions could spawn with a carried item which was
  then deleted, making it unavailable for future monster drops and leading to
  differences in available items for players playing the same seed.
-
  When throwing a potion, auto-targeting is now enabled only for potions known
  to be malevolent. When throwing a melee weapon, auto-targeting is now
  disabled.
-
  Fixed a bug with machine rooms where events (such as guardian movement) could
  trigger twice.
-
  Dungeon generation on ARM processors is now the same as with x86.
-
  Fixed doors sometimes being generated adjacent to one another.
-
  Fixed a bug that caused negated weapons and armor to remain illuminated when
  out of line of sight.
-
  Fixed a bug which slightly delayed the reset of stuck status after blinking or
  teleporting in or out of a web/net. Adjacent monsters at the destination were
  afforded a free hit while the player was "dangling helplessly" in error.
-
  Repeated messages now sometimes collapse together with a count.
-
  Use 24-bit colors when the terminal supports it (curses build).
-
  Brogue now starts with a reasonable window size on ultra-wide monitors.


1.9.3
=====

Major graphical improvements on all platforms, some long-awaited bug fixes, and
the usual general QoL.

Thanks to contributors Andy, Anthony, bleakley, SanatMishra, withinwheels, and
zenzombie.

-
  High DPI displays are now supported on all platforms (no more blurriness!)
-
  The window is now freely resizable, to perfectly fit any monitor up to 4K
  size; tiles and fonts auto-scale but stay sharp
-
  Added 6 new tile graphics: vine, net, lichen, glass pipes, and 2 more types of
  altars
-
  Fixed a bug that could cause monkeys with keys to jump into lava
-
  Discordant wandering monsters no longer target the player with bolts/spells
  from beyond stealth range
-
  Closing the window no longer hangs in some cases, and the game is properly
  auto-saved
-
  Fixed a bug that could cause crashes when entranced monsters move
-
  Explosive monsters no longer explode over chasms when they would die by
  falling
-
  Fixed issues with off-level monster pathing that could cause monsters to go
  into a corrupted state that cannot be attacked
-
  Fast/slow monsters now move the correct distance and to the correct spot when
  reloading a level
-
  Added seed and level into default save and recording file names
-
  Expected damage info in monster info boxes now accounts for the strength bonus
  from unidentified weapons
-
  When a mutated monster is negated, the mutation label is removed from the
  sidebar. Excludes agile and juggernaut mutations, since they cannot be negated
-
  Wands of invisibility auto-ID if they turn an monster invisible while the
  player has a telepathic bond
-
  Fixed crashes from pit bloat and gas interactions
-
  Added "go to previous level" key (`<`) to replay viewer
-
  Added new command line options to change settings on launch: `--full-screen`,
  `--stealth`, and `--no-effects`


1.9.2
=====

A small bug fix release, with a few UI improvements!

Thanks to contributors Andy, Antony, and zenzombie.

Bug fixes
---------
-
  Re-rendered the fonts again to fix blur and boldness issues
-
  Fixed a bug from 1.9.1 where dampening armor and slowing weapons had
  "abomination" in their name
-
  Fixed salamanders and other ranged-melee creatures with the grappling mutation
  attempting to seize their target from a distance. Now ranged attacks do damage
  as normal, while adjacent attacks seize
-
  Fixed walls discovered with the scroll of magic mapping not being smoothed out
  in tiles mode
-
  Fixed bolts from unidentified staffs or wands from being referred to by name
  when reflected
-
  Fixed the player's health bar showing a negative percentage when landing a hit
  with a transference ring equipped
-
  Fixed psychic emanations sometimes being described as the wrong size
-
  Wizard-mode replays and saves now work past the first immortal "death"

Other changes
-------------
-
  Added monster mutations to the seed catalog
-
  Recording playback can now be paused with the mouse or the Escape key
-
  Added brogue-cmd.bat file for Windows. brogue.exe does not print output when
  run from a command prompt window, so use this script instead if you want to
  see it, e.g. when printing a seed catalog
-
  A dialog is now shown when trying to play back a wizard game in regular mode
  or vice versa


1.9.1
=====

This patch release contains quite a number of bug fixes and improvements, most
noticably a re-rendering of the font files.

Since we fixed the wand of plenty, replays made in 1.9.1 will not play on 1.9.
However, 1.9.1 will play recordings and load saves from 1.9, preserving the old
behaviour. (We intend to maintain this backwards-compatibility within each 1.X
series.)

This means that 1.9.1 and 1.9 are not completely interchangable, and you should
upgrade if you wish to compare games with players on this newer version.

Thanks to contributors Antony, flend, SanatMishra, and zenzombie.

Bug fixes
---------
-
  Fixed wand of plenty behaviour. Since CE 1.8, it incorrectly set the cloned
  monsters' max health to half the target's _current_ health, instead of half
  its max health. Now the max health is halved properly and the current health
  is unchanged, but capped to the new max.
-
  Fixed the loading bars not showing on screen when loading a save or advancing
  a recording
-
  Fixed some input events being ignored during recording playback (and possibly
  in-game too)
-
  Prevented Alt+Enter fullscreen shortcut from causing an Enter input in-game
-
  The graphical tiles toggle 'G' now works during recording playback
-
  The '6' key now works like the other number keys in playback, instead of
  moving forward a turn
-
  Fixed even more causes of out-of-sync errors
-
  Fixed a rare crash caused by extremely long monster death messages
-
  Fixed some rare crashes during machine generation

Other changes
-------------
-
  Regenerated font files to improve legibility and contrast. This should
  hopefully be more of a return to form for Mac players, and a new improvement
  for Windows and Linux.

  If you want to change back to the old fonts, replace files font-4.png to
  font-15.png in `bin/assets` (or `Resources/assets` in the app contents on
  macOS) with copies from 1.9 or an earlier version.
-
  The seed catalog now contains more information, including items carried by
  monsters, legendary allies, resurrection altars, and commutation altars. Also,
  gold is aggregated per level, shackled vs. caged status is displayed for
  captive allies, and keys show which vault they will open.
-
  Improved perfomance and decreased CPU usage
-
  You can now quit the program while loading a save, and cancel with the escape
  key
-
  You can now enter a turn number to continue looking back through a recording
  when you reach an out-of-sync error
-
  (For developers) You can now take control of recordings in debug mode by
  pressing 'P'
-
  Seed catalog generation now takes seed and depth range parameters at the
  command line (see `brogue --help` for details)
-
  Add a command line option to generate the seed catalog in CSV format


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

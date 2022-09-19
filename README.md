Brogue Lite
=========

Brogue Lite is a variant of the roguelike game Brogue, specifically, of
Brogue Community Edition. Brogue Lite changes the game to make it easier.
Certain systems (like item identification) are streamlined or removed.


About
-----

### Who is Brogue Lite for?

-
  Novice players who want to practice the rules of the game before
  graduating to the classic Brogue experience.
-
  Roguelike players who dislike the item identification game.
-
  Brogue players who occasionally want to switch it up with a more casual experience.


### How is Brogue Lite different from Brogue CE?

Certain tricky systems like item identification are streamlined or removed.
Otherwise, Brogue Lite will remain quite "vanilla" - the goal is not to add
new monsters, etc., though in some cases new items may need to be added
to balance out the ones that are removed.


### List of changed features


Currently implemented:

-
  Removed the identification system. All items are identified upon creation.
-
  Removed negative scrolls that have very little use once identified.
-
  Removed potion of hallucination, which has no use once identified.
-
  Removed cursed equipment, and negative runics, which have no use once identified. Also removed scroll of remove curse.
-
  Added "level feeling" messages when descending, as well as messages when monsters notice you, etc.
-
  Various visual changes to sidebar.


Possible ideas for potential further changes, TBD:

-
  Are there any staffs or wands which have no good use once identified that need to be removed or reworked?
-
  Are wands an unnecessary complication - if so, should wands be removed and replaced with staffs or charms where appropriate?
-
  Guaranteed vaults on dungeon level 1? Probably not needed since amount of items feels quite generous
  at the moment (since you're not wasting any on use-ID).
-
  Replace some of the removed negative items/runics with new ones that make more sense
  with the lack of an identification minigame?
  Or perhaps give the existing ones positive effects? For example: aggravating monsters makes them unable to cast spells.
  Armor of immolation automatically removes webs, nets, and creeping death at your feet.
-
  What should be done with the ally system? Is it complex enough to consider removing it, or an important part
  of the game's appeal to new players? Should there be an alternative for players who don't wish to use allies but with an opportunity cost?
  (For example, instead of freeing a captured enemy, you could sacrifice it for some other reward?)











The original readme for Brogue CE follows.
=========


Brogue CE
=========

> *Countless adventurers before you have descended this torch-lit staircase,
> seeking the promised riches below. As you reach the bottom and step into
> the wide cavern, the doors behind you seal with a powerful magic...*
>
> ***Welcome to the Dungeons of Doom!***

*Brogue* is a single-player strategy game set in the halls of a mysterious
and randomly-generated dungeon. The objective is simple enough -- retrieve the
fabled Amulet of Yendor from the 26th level -- but the dungeon is riddled with
danger. Horrifying creatures and devious, trap-ridden terrain await. Yet it is
also riddled with weapons, potions, and artifacts of forgotten power. Survival
demands strength and cunning in equal measure as you descend, making the most
of what the dungeon gives you. You will make sacrifices, narrow escapes,
and maybe even some friends along the way -- but will you be one of the
lucky few to return alive?

- [Downloads][releases]
- [Wiki](https://brogue.fandom.com/wiki/Brogue_Wiki)
- [Forum (Reddit)](https://www.reddit.com/r/brogueforum/)
- [Roguelikes Discord](https://discord.gg/9pmFGKx) (we have a #brogue channel)
- IRC: [##brogue on Libera Chat](https://kiwiirc.com/nextclient/irc.libera.chat/##brogue)
- [Original website](https://sites.google.com/site/broguegame/)
- [Android port](https://github.com/bilgincoskun/brogue-android-port/releases)


Playing
-------

If you downloaded a [release][releases], you can open the game as follows:

### Windows

Run `brogue.exe`.

### Mac

Run the included app.

As it's an unsigned program, you may have to convince macOS to let you run it.
You can do this by right-clicking the app and choosing *Open* the first time you
run it.

### Linux

Run the `./brogue` script in the same the folder as this file.

Make sure you have SDL2 and SDL2_image installed via your package manager. The
required packages are:

- Debian/Ubuntu: `libsdl2-2.0-0 libsdl2-image-2.0-0`
- Fedora: `SDL2 SDL2_image`
- Arch: `sdl2 sdl2_image`

You can also run `./make-link-for-desktop.sh` to generate a .desktop file to
place on your desktop or applications folder.

### More information

Graphical tiles are included; press 'G' to toggle them.

For some tips on playing the game, see the original website, linked above. Also
check out the wiki -- although keep in mind CE may differ from it.

If you downloaded the source code, you will need to build the game first. For
instructions, see `BUILD.md`.

On Windows, if you want to run from the command prompt, use `brogue-cmd.bat`
instead of `brogue.exe`.


Contributing
------------

Please see our [contribution guide][contrib] for all the ways you can help make
Brogue better!

[contrib]: https://github.com/tmewett/BrogueCE/wiki/Contribution-guide


About
-----

### What is Community Edition?

Brogue was created by Brian Walker. This version, *Brogue: Community Edition*,
is a continuation of its development. It has several main goals:

- fix bugs and crashes
- add useful quality of life and non-gameplay features
- improve the gameplay and keep it exciting
- ease development and maintenance
- be a convenient base for forks and ports to new platforms

### How is CE different from the original Brogue?

Please refer to the changelog or release history for a complete list. There is
also a wiki page: [Changes from original][cfo].

[cfo]: https://github.com/tmewett/BrogueCE/wiki/Changes-from-original

### How is the project run, and how are changes decided?

The project is run with a "benevolent dictator" model, with myself (tmewett) in
charge. There are many other testers and regular contributors; see the
contribution guide to get involved!

We always try to make changes in the same spirit as the rest of the game. Anyone
can propose a change - all discussion occurs in our GitHub issue tracker or on
our development Discord (see CONTRIBUTING.md). We also make sure that things are
well play-tested.


[releases]: https://github.com/tmewett/BrogueCE/releases

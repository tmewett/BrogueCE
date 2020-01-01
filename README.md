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

- [Wiki](https://brogue.fandom.com/wiki/Brogue_Wiki)
- [Forum (Reddit)](https://www.reddit.com/r/brogueforum/)
- [Roguelikes Discord](https://discord.gg/9pmFGKx) (we have a #brogue channel)
- IRC: ##brogue on Freenode
- [Original website](https://sites.google.com/site/broguegame/)


Playing
-------

If you downloaded a release, you can open the game as follows:

- On Windows, go to the `bin` folder and run `brogue.exe`.

- On Mac, run/double-click the `brogue` script in the same the folder as this
  file. As it's an unsigned program, with unsigned libraries, you will have
  to press OK on many prompts asking if you want to run it. I recommend you
  first run `sudo spctl --master-disable` in Terminal to disable the prompts.
  (You can revert this setting in "Security & Privacy" in System Preferences.)

- On Linux, run the `./brogue` script in the same the folder as this file. You
  can also run `./make-link-for-desktop.sh` to generate a .desktop file to
  place on your desktop or applications folder.

If you're running the graphical version (default), the game window can be
resized using the PageDown/PageUp or -/+ keys.

For some tips on playing the game, see the original website, linked above. Also
check out the wiki -- although this version contains some modifications from
what the wiki reports!

If you downloaded the source code, you will need to build the game first. For
instructions, see `BUILD.md`.


About Community Edition
-----------------------

Brogue was created by Brian Walker. This version, *Brogue: Community Edition*,
is a continuation of its development. It has several main goals:

- add community-accepted bugfixes and game changes
- provide a cross-platform build system
- be a convenient base for forks and ports to new platforms

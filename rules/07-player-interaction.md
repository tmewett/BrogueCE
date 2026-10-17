**BR12747** — Risky or irreversible commands pop up a small two-button confirmation box.
[IO.c:2947-2970](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2947-L2970)
**BR56719** — A confirmation is answered **Yes** with `y`, `Y`, or Return.
[IO.c:2948-2952](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2948-L2952)
**BR19813** — A confirmation is answered **No** with `n`, `N`, space, or Escape.
[IO.c:2954-2960](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2954-L2960)
**BR30441** — Canceling a confirmation (pressing Escape with no button focused, or clicking outside the box) counts as choosing No.
[IO.c:2963-2970](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2963-L2970)
[Buttons.c:244-255](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L244-L255)
**BR67301** — While the player is running on autopilot, or while watching a recording,
confirmation prompts are skipped and treated as answered "yes" (unless the prompt
is explicitly marked to also appear during playback).
[IO.c:2940-2942](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2940-L2942)
**BR24236** — Turning on autopilot first asks "Turn on autopilot?".
[IO.c:2602-2605](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2602-L2605)
**BR45412** — Choosing "Save and exit" from the actions menu asks "Save this game and
exit?" before saving.
[IO.c:2624-2631](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2624-L2631)
**BR23707** — Requesting a new game while a game is in progress only prompts "End this
game and begin a new game?" once the player has taken at least 50 turns; before
turn 50 it begins a new game without asking.
[IO.c:2632-2637](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2632-L2637)
**BR79829** — Quitting asks "Quit and abandon this game? (The save will be deleted.)"
and, if confirmed, abandons the run (the save file is deleted).
[IO.c:2638-2644](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2638-L2644)
**BR12362** — Starting a new game while viewing a recording asks "Close recording and
begin a new game?".
[Recordings.c:987-994](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L987-L994)
**BR84067** — When a save file or recording is saved under a name that already exists,
the game asks "File of that name already exists. Overwrite?" before replacing it.
[Recordings.c:1191-1206](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1191-L1206)
[Recordings.c:1235-1256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1235-L1256)
**BR26966** — Resetting the "recent" game statistics asks "Reset recent stats?" first.
[MainMenu.c:1082-1084](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1082-L1084)
**BR50163** — A general alert is shown as a box with a single OK button; it is
dismissed with Return or space.
[MainMenu.c:640-650](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L640-L650)
---
**BR50315** — Pressing Return/Enter turns on the keyboard cursor.
[IO.c:2512-2514](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2512-L2514)
[IO.c:97-107](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L97-L107)
**BR12939** — If the keyboard cursor is not currently placed on the map, turning it on
places it on the player's own location.
[IO.c:99-102](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L99-L102)
**BR95103** — Canceling the cursor (with space or Escape) hides it and clears the
highlighted travel path.
[IO.c:767-769](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L767-L769)
**BR19418** — Canceling the cursor also confirms any pending messages.
[IO.c:767-769](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L767-L769)
**BR11456** — Pressing Tab cycles the cursor through visible monsters, items, and
listed terrain features.
[IO.c:745-750](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L745-L750)
**BR56442** — Holding shift while cycling the cursor with Tab cycles through those
targets in reverse.
[IO.c:747](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L747)
**BR92554** — While examining the map during playback, the game prompts "Examine what?
(&lt;hjklyubn&gt;, mouse, or &lt;tab&gt;)".
[IO.c:583-585](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L583-L585)
**BR15841** — Moving the cursor highlights the travel path from the player to the
cursor.
[IO.c:617-648](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L617-L648)
**BR27320** — Once the player reaches the cursor's destination, cursor mode and the
highlighted path are turned off automatically.
[IO.c:587-595](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L587-L595)
**BR37867** — If the cursor is aimed at an area the player cannot reach, the
highlighted path instead leads to the nearest reachable location.
[IO.c:619-625](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L619-L625)
**BR12275** — Resting the cursor on a visible monster, a visible (or sensed) item, or
a listed terrain feature shows that entity's details in a pop-up box and
highlights it in the sidebar.
[IO.c:651-694](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L651-L694)
**BR21675** — Confirming the cursor while it sits on the player's own location does
nothing.
[IO.c:783-784](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L783-L784)
**BR57297** — Confirming the cursor on an adjacent space (including diagonals, when the
diagonal is not blocked) moves the player there directly.
[IO.c:785-794](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L785-L794)
**BR54835** — Otherwise the player travels along the highlighted path toward the
cursor.
[IO.c:795-796](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L795-L796)
**BR54603** — Control-clicking on a spot moves the player one step along the path
toward that spot, rather than traveling the whole way.
[IO.c:771-778](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L771-L778)
**BR35372** — Pressing the descend or ascend key while the cursor is on the matching
staircase confirms that target and uses the stairs.
[IO.c:752-758](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L752-L758)
**BR37769** — Clicking a location on the map places the keyboard cursor there and
enters cursor mode.
[IO.c:2437-2443](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2437-L2443)
---
**BR95966** — Messages appear in the message area at the top of the screen, newest at
the bottom.
[IO.c:3612-3642](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3612-L3642)
**BR48605** — After messages are confirmed, older lines are dimmed progressively the
further up they are.
[IO.c:3619-3621](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3619-L3621)
**BR54444** — New unconfirmed messages are drawn at full brightness; taking any action
confirms them and dims them.
[IO.c:3619-3621](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3619-L3621)
[IO.c:3653-3656](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3653-L3656)
**BR13184** — Repeats of an identical message are collapsed into one entry with a
count; once the count reaches the maximum it reads "(many)" instead of a number.
[IO.c:3025-3037](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3025-L3037)
**BR87254** — Foldable messages from the same turn are combined onto shared,
semicolon-separated lines in the message display.
[IO.c:3044-3097](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3044-L3097)
**BR26976** — When a message requires acknowledgment, a "--MORE--" marker appears and
play waits for the player to press space or click.
[IO.c:3525-3543](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3525-L3543)
**BR35666** — Pressing any key other than space while a message awaits acknowledgment
flashes a "Press space or click to continue" reminder.
[IO.c:2917-2923](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2917-L2923)
**BR85363** — Clicking in the message area opens the scrollable message archive.
[IO.c:2445-2448](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2445-L2448)
**BR80976** — In the message archive, up/down scroll a page, shift+up/down step one
line, and control+up/down jump to the newest or oldest end.
[IO.c:3313-3335](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3313-L3335)
**BR53455** — The message archive closes on space, Escape, or a click.
[IO.c:3336-3347](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3336-L3347)
**BR99611** — The message archive only opens when there are more messages than fit in
the regular message area.
[IO.c:3356-3364](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3356-L3364)
**BR90343** — The "display old messages" key opens the same message archive.
[IO.c:2607-2608](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2607-L2608)
**BR31995** — Message color reflects importance: harm to the player or an ally is shown
in the "bad" color, harm to an enemy is shown in the "good" color, and neutral
events are white.
[IO.c:3598-3610](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3598-L3610)
**BR58724** — Temporary prompt messages replace the whole message area and are not
archived; the prompt text is capitalized at the start.
[IO.c:3382-3408](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3382-L3408)
---
**BR44457** — The sidebar always lists the player first, then visible monsters, items,
and listed terrain in order of distance from the player (closest first).
[IO.c:3771-3791](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3771-L3791)
[IO.c:3814-3861](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3814-L3861)
**BR63184** — The bottom line of the sidebar is reserved for the current dungeon depth.
[IO.c:3865](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3865)
[IO.c:3904-3905](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3904-L3905)
**BR19271** — An item lying at the player's location is listed in the sidebar
immediately after the player.
[IO.c:3776-3782](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3776-L3782)
**BR18434** — Monsters are listed from closest to farthest in two passes: first
creatures in direct sight, then those seen only indirectly (such as by telepathy).
[IO.c:3793-3814](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3793-L3814)
**BR53458** — Monsters flagged to be omitted from the sidebar are never listed there.
[IO.c:3800-3802](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3800-L3802)
**BR36729** — A focused entity is highlighted with a brightened gradient in the
sidebar.
[IO.c:4760-4767](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4760-L4767)
**BR31299** — If a focused entity would otherwise not fit on screen, it is moved to the
top of the sidebar list.
[IO.c:3784-3791](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3784-L3791)
[IO.c:3906-3909](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3906-L3909)
**BR66697** — Each sidebar row is associated with the map location of the entity shown
there, so rows correspond to places on the map.
[IO.c:3894-3896](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3894-L3896)
**BR85934** — A monster's sidebar entry shows its name and any mutation.
[IO.c:4550-4568](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4550-L4568)
[IO.c:4571-4589](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4571-L4589)
**BR42937** — A monster's sidebar entry shows a health bar if its maximum health is
above 1 and it is not invulnerable.
[IO.c:4591-4611](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4591-L4611)
**BR71934** — A monster's sidebar entry shows a bar for each of its active status
effects.
[IO.c:4629-4661](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4629-L4661)
**BR19836** — A monster's current behavior is labeled in the sidebar, e.g. (Sleeping),
(Ally), (Fleeing), (Wandering), (Hunting), (Captive), (Helpless), (Off balance),
(Guarding), or (Worshiping); a hallucinating player instead sees a random silly
label such as (Dancing) or (Gibbering).
[IO.c:4719-4753](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4719-L4753)
**BR23017** — Poison that would kill the target before it wears off is labeled "Fatal
Poison", and stacked poison is shown with its strength, e.g. "Poisoned (x3)".
[IO.c:4637-4653](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4637-L4653)
**BR90270** — The player's own sidebar entry shows a health bar.
[IO.c:4591-4611](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4591-L4611)
**BR82377** — The player's own sidebar entry shows a nutrition bar whose label changes
through "Nutrition (Hungry)", "Nutrition (Weak)", and "Nutrition (Faint)" down to
"STARVING".
[IO.c:4614-4626](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4614-L4626)
**BR32227** — The player's own sidebar entry shows Strength, Armor, and Gold.
[IO.c:4663-4699](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4663-L4699)
**BR83076** — The player's own sidebar entry shows the stealth range.
[IO.c:4700-4718](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4700-L4718)
**BR27301** — In the player's sidebar entry, armor is shown with a "?" if the worn
armor is unidentifiable.
[IO.c:4676-4689](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4676-L4689)
**BR56096** — The player's entry is tagged "(invisible)", "(dark)", or "(lit)"
depending on invisibility and lighting.
[IO.c:4553-4565](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4553-L4565)
**BR91017** — During playback the sidebar header shows "-- PLAYBACK --" and a turn
progress bar.
[IO.c:3748-3754](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3748-L3754)
**BR60416** — During playback the sidebar header shows either "[PAUSED]" or
"[OUT OF SYNC]" as applicable.
[IO.c:3755-3759](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3755-L3759)
**BR37022** — Selecting an entity to examine shows its full description in a text box
on the map: a monster's details box or an item's details box.
[IO.c:5035-5039](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5035-L5039)
[IO.c:5123-5128](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5123-L5128)
---
**BR96987** — Pressing the inventory key, or right-clicking the map, opens the pack.
[IO.c:2542-2544](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2542-L2544)
[IO.c:2435-2436](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2435-L2436)
**BR83394** — An item's detail box offers an Apply button for food, scrolls, potions,
wands, staves, and charms.
[IO.c:5063-5067](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5063-L5067)
**BR13225** — An item's detail box offers an Equip or Remove button for armor, weapons,
and rings.
[IO.c:5068-5078](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5068-L5078)
**BR68240** — An item's detail box always offers Drop and Throw buttons.
[IO.c:5079-5085](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5079-L5085)
**BR29231** — An item's detail box offers a Call button when the item can be named, and
a Relabel button.
[IO.c:5087-5097](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5087-L5097)
**BR43577** — While an item's detail box is open, the up and down arrows page through
the carried items.
[IO.c:5099-5111](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L5099-L5111)
**BR72215** — The discovered-items screen lists scrolls, rings, potions, staffs, and
wands.
[IO.c:4248-4261](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4248-L4261)
**BR27870** — In the discovered-items screen, identified kinds are shown by name, while
unidentified kinds show a good/bad magic marker and the percentage chance of
generating that kind.
[IO.c:4158-4184](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4158-L4184)
**BR99608** — The discovered-items screen closes on any key or click.
[IO.c:4263-4273](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4263-L4273)
---
**BR77643** — The title screen offers New Game (`N`), Play (`P`), View (`V`), and Quit
(`Q`).
[MainMenu.c:275-287](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L275-L287)
**BR30249** — The Play flyout contains New Seeded Game (`S`), Load Game (`L`), Change
Variant (`A`), and Change Mode (`M`).
[MainMenu.c:366-372](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L366-L372)
**BR82810** — The View flyout contains View Recording (`R`), High Scores (`H`), and
Game Stats (`G`).
[MainMenu.c:374-379](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L374-L379)
**BR30507** — A new seeded game prompts "Generate dungeon with seed number:", accepting
up to 20 digits.
[MainMenu.c:1156-1161](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1156-L1161)
**BR88652** — The previous game's seed is offered as the default in the seed prompt.
[MainMenu.c:1150-1155](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1150-L1155)
**BR43142** — Leaving the seed blank or pressing Escape cancels and does not start a
game.
[MainMenu.c:1167-1170](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1167-L1170)
**BR76508** — A seed too large for the numeric range is clamped to the maximum value,
18446744073709551615 (2^64 − 1).
[MainMenu.c:1163-1166](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1163-L1166)
**BR97169** — The Change Variant dialog describes and lets the player pick Rapid
Brogue, (classic) Brogue, or Bullet Brogue; canceling leaves the choice unchanged.
[MainMenu.c:390-426](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L390-L426)
**BR85257** — The Change Mode dialog offers Wizard, Easy, and Normal.
[MainMenu.c:451-464](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L451-L464)
**BR43510** — The Change Mode dialog explains that Easy mode takes 20% as much damage
but keeps only 10% of the score.
[MainMenu.c:440-443](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L440-L443)
**BR47986** — The Change Mode dialog explains that Wizard mode's score is not saved.
[MainMenu.c:445-449](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L445-L449)
**BR73762** — The Game Stats screen shows All Time and Recent columns for games played,
won, win rate, escaped, mastered, high score, most gold, most lumenstones, deepest
level, average depth, shortest win, and streaks.
[MainMenu.c:983-1047](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L983-L1047)
**BR10299** — The Game Stats screen offers a Reset button when there is recent data.
[MainMenu.c:1068-1084](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1068-L1084)
**BR38190** — File-picker dialogs (for loading games or viewing recordings) list only
files matching the required suffix, sorted newest first.
[MainMenu.c:706-730](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L706-L730)
**BR77834** — In a file picker, each listed file has a letter hotkey and can be selected
by clicking.
[MainMenu.c:737-751](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L737-L751)
**BR84005** — A file picker with more files than fit shows up/down arrow buttons to
page.
[MainMenu.c:779-807](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L779-L807)
**BR26817** — If a file picker finds no matching files, it shows "No applicable files
found.".
[MainMenu.c:858-863](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L858-L863)
**BR58305** — Pressing N on the title screen starts a new game immediately without
opening a flyout.
[MainMenu.c:584-588](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L584-L588)
---
**BR42285** — While playing a recording, the player can speed up playback with `k` or
the up arrow.
[Recordings.c:848-856](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L848-L856)
**BR62572** — While playing a recording, the player can slow playback with `j` or the
down arrow.
[Recordings.c:857-865](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L857-L865)
**BR82494** — While playing a recording, the player can pause or unpause with space.
[Recordings.c:866-872](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L866-L872)
**BR87453** — Tab toggles omniscience during playback and announces "Omniscience
enabled." or "Omniscience disabled.".
[Recordings.c:873-882](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L873-L882)
**BR61186** — `<` seeks to the previous level and `>` to the next level.
[Recordings.c:883-892](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L883-L892)
**BR36523** — Asking to go past the deepest level explored flashes "Already reached
deepest depth explored".
[Recordings.c:887-889](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L887-L889)
**BR15719** — Pressing a digit prompts "Go to turn number:" to fast-forward to that
turn.
[Recordings.c:1070-1075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1070-L1075)
[Recordings.c:779-789](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L779-L789)
**BR59377** — Seeking to the turn the recording is already on reports "Already at turn
N".
[Recordings.c:798-800](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L798-L800)
**BR23216** — If the recording has diverged, seeking forward reports "Out of sync".
[Recordings.c:796-797](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L796-L797)
**BR11882** — The right arrow (or `l`) advances the recording one turn.
[Recordings.c:898-906](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L898-L906)
[Recordings.c:933-936](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L933-L936)
**BR24285** — Holding shift while advancing moves 5 turns.
[Recordings.c:907-909](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L907-L909)
**BR93927** — Holding control while advancing moves 20 turns.
[Recordings.c:910-912](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L910-L912)
**BR80681** — The left arrow (or `h`) rewinds the recording by one turn.
[Recordings.c:900-906](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L900-L906)
[Recordings.c:914-932](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L914-L932)
**BR95854** — Seeking backward prompts "Rewind?" before restarting, unless the player
is still before turn 100, in which case it rewinds without asking.
[Recordings.c:926-932](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L926-L932)
**BR87547** — Rewinding replays the recording from depth 1.
[Recordings.c:723-732](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L723-L732)
**BR32262** — A long seek shows a "[ Loading... ]" progress bar.
[Recordings.c:734-747](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L734-L747)
**BR77639** — `?` during playback opens a playback help screen listing the playback
commands, dismissed with any key.
[Recordings.c:607-661](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L607-L661)
**BR50728** — During playback the pack and discovered-items screens can still be
opened.
[Recordings.c:893-897](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L893-L897)
[Recordings.c:946-950](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L946-L950)
**BR70807** — During playback, "View saved recording" and "Open saved game" let the
player pick a new file to load.
[Recordings.c:956-986](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L956-L986)
**BR75773** — Pressing Escape during playback pauses it.
[Recordings.c:1064-1069](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1064-L1069)
**BR34212** — Clicking in the message area during playback opens the message archive.
[Recordings.c:1079-1087](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1079-L1087)
**BR17154** — Clicking anywhere outside the message area during playback pauses it.
[Recordings.c:1088-1092](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1088-L1092)
**BR19831** — Right-clicking during playback opens the pack.
[Recordings.c:1093-1097](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1093-L1097)
**BR73305** — If the recording diverges from the original game, playback pauses, shows
the message "Playback is out of sync." plus an apology box, marks the sidebar
"[OUT OF SYNC]", and hands control back to the player.
[Recordings.c:305-338](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L305-L338)
**BR48693** — If a recording was made by an incompatible version, the game shows
"This file is from version X and cannot be opened in version Y." and ends the
playback.
[Recordings.c:503-524](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L503-L524)
**BR13473** — Recording files may carry annotations; at a turn with an annotation, the
text is shown in a box with a "--MORE--" acknowledgment before playback resumes.
[Recordings.c:431-450](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L431-L450)
**BR21015** — Starting to view a recording pauses playback and shows "recording
paused. Press space to play."; resuming shows "recording unpaused.".
[Recordings.c:809-825](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L809-L825)
---
**BR77744** — The in-game actions menu labels the save command "Save and exit" (or
"Open saved game"/"View saved recording" during playback).
[IO.c:364-388](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L364-L388)
**BR96804** — The in-game actions menu labels quitting as "Quit and abandon game" (or
"Quit to title screen" during playback).
[IO.c:390-396](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L390-L396)
**BR71276** — Saving asks for a file name with the prompt "Save game as (<esc> to
cancel):".
[Recordings.c:1191-1192](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1191-L1192)
**BR15955** — The default save name includes the version, seed, and current depth.
[Recordings.c:1142-1143](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1142-L1143)
**BR86892** — The save suffix is appended to the chosen name automatically.
[Recordings.c:1193](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1193)
**BR24295** — The default name of a finished run's recording reports the outcome, such
as "Died at depth N", "Mastered the dungeons", or "Escaped the dungeons", and
appends "(wizard)" or "(easy)" for those modes.
[Recordings.c:1144-1157](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1144-L1157)
**BR92309** — Saving a full recording uses the prompt "Save recording as (<esc> to
cancel):".
[Recordings.c:1237-1238](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1237-L1238)
**BR24129** — Declining to name a recording still keeps it as the "last recording"
file.
[Recordings.c:1248-1254](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1248-L1254)
**BR10801** — When the program closes without a prompt, an unfinished game is saved as
a savegame and a finished one is saved as a recording, with no user interaction.
[MainMenu.c:623-638](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L623-L638)
**BR58869** — Filenames may not contain `/`, `\`, or `:`; those characters typed in a
filename are replaced with `-`.
[Recordings.c:1118-1124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1118-L1124)
[IO.c:2791-2795](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2791-L2795)
**BR64035** — When a save file or recording name would collide with an existing file,
a counting number is appended to make the name unique.
[Recordings.c:1102-1116](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1102-L1116)
**BR52091** — While loading a saved game, a "[ Loading... ]" progress bar is shown.
[Recordings.c:1323-1345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1323-L1345)
**BR29933** — Pressing Escape cancels a saved-game load in progress.
[Recordings.c:1346-1351](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1346-L1351)
**BR68875** — Saving displays "Saved." and then ends the game.
[Recordings.c:1200-1202](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1200-L1202)
**BR93420** — Loading a saved game transitions into active play and copies the save
file to a "last game" file.
[Recordings.c:1280-1304](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1280-L1304)
---
**BR91827** — Menus and dialogs are driven by buttons: a button is chosen by clicking
it (on mouse-up) or by pressing one of its hotkeys.
[Buttons.c:240-256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L240-L256)
[Buttons.c:259-298](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L259-L298)
**BR25868** — Escape, space, or clicking outside the menu cancels the prompt.
[Buttons.c:246-252](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L246-L252)
[Buttons.c:302-308](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L302-L308)
**BR31563** — The in-game actions menu lists commands such as rest, autopilot, re-throw,
easy mode, hide color effects, display stealth range, enable graphics, view feats,
view discovered items, view the dungeon seed, and help, in addition to save and
quit.
[IO.c:170-362](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L170-L362)
**BR70026** — In a filename or numeric text prompt, the Delete key removes the last
character.
[IO.c:2783-2787](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2783-L2787)
**BR62975** — In a filename or numeric text prompt, Return accepts the entry.
[IO.c:2825](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2825)
**BR43829** — In a filename or numeric text prompt, Escape cancels it.
[IO.c:2825](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2825)
[IO.c:2833-2835](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2833-L2835)
**BR25882** — The help screen lists that moving the mouse moves the cursor and that
clicking travels.
[IO.c:4073-4075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4073-L4075)
**BR47435** — The help screen lists that Return enables keyboard cursor control and
that space or Escape disables it.
[IO.c:4076-4077](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4076-L4077)
**BR27347** — The help screen lists that hjklyubn, arrow, or numpad keys move or
attack, with control or shift to run.
[IO.c:4078](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4078)
**BR53953** — The dungeon seed is reported in the message area along with the turn
number, game mode (if not normal), and version.
[IO.c:4390-4406](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4390-L4406)
**BR78309** — If the seed is hidden it is reported as "HIDDEN".
[IO.c:4400-4401](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4400-L4401)
**BR39512** — The help screen is dismissed by pressing space or clicking.
[IO.c:4133](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4133)
**BR33110** — The Feats screen lists every feat with its achievement status and closes
on any key or click.
[IO.c:4188-4237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4188-L4237)
**BR93502** — The high scores screen lists each run's rank, score, date, and
description, and closes on space or a click.
[IO.c:4278-4337](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4278-L4337)
**BR22832** — During normal play the bottom row of the screen shows buttons for
Explore, Rest, Search, Menu, and Inventory.
[IO.c:490-518](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L490-L518)
**BR65948** — During playback the bottom row of the screen shows buttons for Unpause,
Omniscience, Next Turn, and Menu.
[IO.c:462-489](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L462-L489)

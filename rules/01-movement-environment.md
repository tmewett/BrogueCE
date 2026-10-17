- **BR31821** — A move is blocked when either the destination square or the square you are leaving obstructs passage.
  [Movement.c:931-933](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L931-L933)
- **BR10130** — A key you hold (or a monster embedded in a wall that you can attack through it) lets you act into an otherwise obstructing square.
  [Movement.c:931-934](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L931-L934)
- **BR10151** — Bumping into a not-yet-discovered obstruction reveals it and shows its description.
  [Movement.c:1230-1243](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1230-L1243)
- **BR29427** — A diagonal move is blocked when either of the two squares flanking the corner blocks diagonal movement, so you cannot cut past a corner.
  [Movement.c:825-839](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L825-L839)
- **BR10367** — A straight cardinal move is never blocked by the corner geometry that blocks diagonals.
  [Movement.c:827-829](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L827-L829)
- **BR75019** — Stepping onto a square containing an item picks it up automatically and interrupts automated movement.
  [Movement.c:1202-1205](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1202-L1205)
- **BR44054** — Stepping onto an ally's square swaps places with it.
  [Movement.c:1189-1200](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1189-L1200)
- **BR11352** — An ally that would refuse to stay on the square you vacate is teleported to the nearest suitable square instead.
  [Movement.c:1193-1195](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1193-L1195)
- **BR72341** — While a hostile creature is holding your legs, moving fails with the message "you cannot move; ... is holding your legs!".
  [Movement.c:1020-1044](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1020-L1044)
- **BR11386** — If the leg-holder is visible and you have not yet committed to the action, the move is cancelled and no turn is spent; otherwise you spend the turn struggling.
  [Movement.c:1030-1042](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1030-L1042)
- **BR54129** — While confused, every move goes in a randomly chosen unblocked direction instead of the one you press.
  [Movement.c:894-907](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L894-L907)
- **BR11541** — A confused move is cancelled if no valid direction exists.
  [Movement.c:894-907](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L894-L907)
- **BR12017** — While confused and standing next to known lava you must first confirm "Risk stumbling into lava?".
  [Movement.c:867-892](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L867-L892)
- **BR96161** — Moving into a hostile monster attacks it instead of moving.
  [Movement.c:973-1017](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L973-L1017)
- **BR12199** — An attack made by moving into a monster costs time equal to your attack speed before you may act again.
  [Movement.c:1013-1014](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1013-L1014)
  [Time.c:2201-2212](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2201-L2212)
- **BR35928** — With a whip you strike a visible enemy up to 5 squares away in a straight line instead of moving, provided the adjacent square in that direction is not corner-blocked.
  [Movement.c:633-688](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L633-L688)
- **BR12436** — With a spear you strike every enemy in the first 2 squares of a straight line in your movement direction, provided the adjacent square is not corner-blocked.
  [Movement.c:690-799](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L690-L799)
- **BR49182** — A flailing weapon hits every enemy adjacent to both your starting and ending square as you step.
  [Movement.c:801-823](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L801-L823)
  [Movement.c:1134-1142](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1134-L1142)
- **BR12763** — A lunging weapon attacks an enemy standing 2 squares away in your movement direction.
  [Movement.c:1112-1133](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1112-L1133)
- **BR13020** — Flail and lunge attacks resolve after you complete the step.
  [Movement.c:1216-1226](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1216-L1226)
- **BR40648** — Attacking a corrosive monster prompts "Degrade your <weapon> by attacking <monster>?" unless the weapon is protected or is a matched slaying runic.
  [Movement.c:558-586](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L558-L586)
- **BR13255** — Attacking a discordant ally prompts "Are you sure you want to attack <ally>?".
  [Movement.c:591-610](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L591-L610)
- **BR13411** — Neither attack confirmation appears while you are confused or hallucinating (but not telepathic).
  [Movement.c:617-631](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L617-L631)
- **BR33062** — Bumping into a shackled captive offers to free it.
  [Movement.c:955-958](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L955-L958)
- **BR13418** — Freeing a caged captive spends a matching key.
  [Movement.c:960-962](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L960-L962)
- **BR13423** — Freeing a captive costs an attack's worth of time.
  [Movement.c:963-965](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L963-L965)
- **BR13526** — A freed captive becomes a grateful, faithful ally ("you free the grateful <name> and gain a faithful ally.").
  [Movement.c:530-537](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L530-L537)
- **BR13676** — Tunnelling through wall terrain automatically frees any captive embedded there.
  [Movement.c:539-553](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L539-L553)
- **BR14273** — A monster converted to your side drops any item it carries, converts any monster it is carrying into an ally too, and becomes your follower.
  [Movement.c:513-528](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L513-L528)
- **BR14498** — Using a key on a square promotes each terrain layer that key opens, choosing "in" or "on" according to how you stand.
  [Movement.c:407-429](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L407-L429)
- **BR14513** — A key that is used is consumed with "you use your <key> in/on <terrain>" and is removed from your pack, the floor, or the carrying monster.
  [Movement.c:431-459](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L431-L459)
- **BR14518** — Moving into terrain that promotes on your entry, such as a lever, shows its flavor text, triggers the promotion, and consumes the turn without moving you.
  [Movement.c:913-929](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L913-L929)
- **BR64538** — While nauseous, moving has a 25% chance to make you vomit instead, wasting the turn.
  [Movement.c:1164-1171](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1164-L1171)
- **BR14717** — While nauseous, attacking has a 25% chance to make you vomit instead, wasting the turn.
  [Movement.c:988-995](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L988-L995)
- **BR15314** — Vomiting leaves a splatter of vomit on the floor and prints "<name> vomits profusely".
  [Movement.c:485-496](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L485-L496)
- **BR60519** — You refuse to willingly step into known lava ("that would be certain death!") unless you are confused, levitating, fire-immune, or the lava is hidden in a secret square.
  [Movement.c:1048-1058](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1048-L1058)
- **BR63035** — Deliberately walking into a known chasm or open trap door asks "Dive into the depths?" and cancels the move if you decline.
  [Movement.c:1060-1070](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1060-L1070)
- **BR95206** — Walking into visible fire asks "Venture into flame?" unless you are already burning or fire-immune.
  [Movement.c:1072-1082](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1072-L1082)
- **BR59445** — Walking into visible confusion or paralysis gas asks "Venture into dangerous gas?" unless you are already burning or wear identified armor of respiration.
  [Movement.c:1084-1093](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1084-L1093)
- **BR83833** — Stepping onto a visible, untriggered pressure plate asks "Step onto the pressure plate?" unless you are levitating (and, for the gas traps, unimpeded by armor of respiration).
  [Movement.c:1095-1110](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1095-L1110)
- **BR15664** — Whenever you take a movement action, every entranced monster that is not stuck, paralyzed, or captive slides one square in the direction opposite your movement.
  [Movement.c:498-511](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L498-L511)
- **BR50847** — Stepping onto a staircase takes you up or down a level.
  [Movement.c:1173-1179](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1173-L1179)
  [Movement.c:2184-2189](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2184-L2189)
- **BR37593** — Descending from the deepest level while carrying the Amulet of Yendor wins the game.
  [Movement.c:2195-2196](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2195-L2196)
- **BR37646** — Descending from the deepest level without the Amulet is repelled ("the crystal archway repels you").
  [Movement.c:2197-2201](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2197-L2201)
- **BR38076** — Ascending off the top level while carrying the Amulet of Yendor wins the game.
  [Movement.c:2203-2214](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2203-L2214)
- **BR38077** — Ascending off the top level without the Amulet is refused ("The dungeon exit is magically sealed!").
  [Movement.c:2215-2219](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2215-L2219)
- **BR85417** — Running in a direction continues until something disturbs you.
  [Movement.c:37-59](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L37-L59)
- **BR15954** — A run stops as soon as a step fails.
  [Movement.c:38-41](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L38-L41)
- **BR15995** — A run stops if the next square is off the map or is a square you would avoid stepping onto.
  [Movement.c:43-46](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L43-L46)
- **BR16118** — A run stops if an adjacent square holds an item or a visible hostile creature.
  [Movement.c:47-48](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L47-L48)
  [Movement.c:2091-2108](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2091-L2108)
- **BR16305** — During a cardinal run, a change in whether any cardinal neighbor except the one directly behind you is a square you would avoid stops the run.
  [Movement.c:49-58](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L49-L58)
- **BR16355** — Starting a run while confused does nothing; the run is disabled.
  [Movement.c:31-31](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L31-L31)
- **BR35869** — Auto-travel only works to already-explored squares ("You have not explored that location.").
  [Movement.c:1678-1681](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1678-L1681)
- **BR17177** — Auto-travel shows the proposed route and asks "Travel this route? (y/n)" before following it.
  [Movement.c:1691-1719](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1691-L1719)
- **BR17201** — Auto-travel reports "No path is available." when the destination cannot be reached.
  [Movement.c:1726-1729](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1726-L1729)
- **BR17332** — Auto-travelling to an adjacent square simply moves you there in one step.
  [Movement.c:1667-1676](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1667-L1676)
- **BR17834** — Choosing a travel target: your own square answers "you are already there.", an explored square moves the cursor, and an unexplored square prints the failure message.
  [Movement.c:2164-2178](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2164-L2178)
- **BR40929** — A recorded route is re-checked as you walk it and travel aborts the moment any step of the remaining route becomes corner-blocked or becomes a square you would avoid.
  [Movement.c:1584-1605](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1584-L1605)
- **BR18298** — Auto-travel walks you step by step toward the target and stops when a step fails or you interrupt with a key.
  [Movement.c:1611-1647](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1611-L1647)
- **BR30141** — Auto-exploration refuses to start while you are confused ("Not while you're confused.").
  [Movement.c:1950-1953](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1950-L1953)
- **BR18464** — Auto-exploration refuses to start while you are embedded in terrain ("Not while you're trapped.").
  [Movement.c:1954-1957](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1954-L1957)
- **BR45596** — During auto-explore you first fight any adjacent enemy.
  [Movement.c:1968-1974](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1968-L1974)
  [Movement.c:1990-1998](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1990-L1998)
- **BR18527** — After fighting, auto-explore steps toward unexplored ground.
  [Movement.c:2003-2033](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2003-L2033)
  [Movement.c:1901-1913](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1901-L1913)
- **BR18555** — Auto-explore favors squares holding items as goals.
  [Movement.c:1914-1921](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1914-L1921)
- **BR19310** — Auto-explore steers around terrain and items it would rather avoid.
  [Movement.c:1916-1917](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1916-L1917)
  [Movement.c:1780-1870](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1780-L1870)
- **BR19564** — Auto-explore stops the moment you are disturbed.
  [Movement.c:2020-2033](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2020-L2033)
- **BR19609** — During auto-explore the message "Exploring... press any key to stop." is shown.
  [Movement.c:1976-1984](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1976-L1984)
- **BR33720** — The exploration penalty for unfamiliar squares rises with depth, from 10 on the upper levels to 30 on deep ones.
  [Movement.c:1786-1786](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1786-L1786)
  [Movement.c:1846-1851](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1846-L1851)
- **BR20029** — Squares already known to be trap-free always cost a flat 10 to explore.
  [Movement.c:1842-1845](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1842-L1845)
- **BR65504** — Once auto-explore has mapped out the level it heads for the stairs.
  [Movement.c:2013-2016](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2013-L2016)
  [Movement.c:1926-1931](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1926-L1931)
- **BR20276** — Full auto-play descends the stairs and keeps exploring the next level.
  [Movement.c:2041-2063](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2041-L2063)
- **BR20805** — Full auto-play prints "Playing... press any key to stop.".
  [Movement.c:2046-2047](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2046-L2047)
- **BR20839** — Auto-fight keeps attacking an adjacent enemy only while your hit points exceed the damage that enemy is expected to deal, unless you told it to fight to the death.
  [Movement.c:2065-2089](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2065-L2089)
- **BR53892** — Every time you step on a square you perform a weak automatic search for secrets there, once per square.
  [Time.c:2300-2304](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2300-L2304)
- **BR81487** — Searching on five consecutive turns ends in a much stronger final sweep ("you finish your detailed search of the area.").
  [Time.c:2179-2186](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2179-L2186)
- **BR21216** — A search's chance of revealing a secret falls off with distance from you and is lower for walls than for open floor.
  [Time.c:2156-2193](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2156-L2193)
  [Movement.c:2131-2162](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2131-L2162)
- **BR21546** — A thorough search also marks nearby squares as known to be trap-free.
  [Movement.c:2131-2162](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2131-L2162)
- **BR39524** — Standing on, rather than levitating over, any secret square you can see reveals its secret.
  [Time.c:123-130](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L123-L130)
- **BR21571** — Revealing a secret replaces each secret layer with the feature it conceals and refreshes the square.
  [Movement.c:2110-2128](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2110-L2128)
- **BR82459** — If you see a monster pass through a secret door, the secret is revealed.
  [Time.c:218-223](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L218-L223)
- **BR21760** — If you spot a monster embedded in secretly wall-ified terrain, that secret is revealed.
  [Time.c:2517-2523](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2517-L2523)
- **BR39970** — A creature that steps on a pressure plate (unless levitating, submerged, or a submerging creature on submergible ground) depresses it once, revealing and triggering the trap if it was secret.
  [Time.c:225-251](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L225-L251)
- **BR22348** — A pressure plate resets when no creature or item remains on the square.
  [Time.c:1493-1501](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1493-L1501)
- **BR22580** — A visible monster on a plate shows "a pressure plate clicks underneath <monster>!"; an unseen creature on a visible plate shows "a pressure plate clicks!".
  [Time.c:237-244](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L237-L244)
- **BR22654** — A key lying on the floor or carried by a monster that matches a key-locked, key-promoting square is used automatically when a creature stands there.
  [Time.c:461-464](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L461-L464)
- **BR24837** — Any creature that ends up in lava without levitation, fire immunity, or invulnerability is killed instantly.
  [Time.c:173-181](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L173-L181)
- **BR22950** — A player killed by lava sees "you plunge into <lava>!" and "Killed by <lava>".
  [Time.c:182-188](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L182-L188)
- **BR23703** — A monster killed by lava that you can see is announced as consumed and leaves a burst of flame.
  [Time.c:189-205](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L189-L205)
- **BR51947** — Standing in deep water without levitation submerges you and dims your vision, unless the square is entangling or obstructed; shallow water never submerges you.
  [Time.c:81-91](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L81-L91)
- **BR23859** — Leaving deep water, levitating, or standing on entangling or obstructing terrain surfaces you.
  [Time.c:72-80](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L72-L80)
- **BR54253** — For every 100 ticks spent standing in deep water, there is a 50% chance an unequipped item in your pack is swept away by the current.
  [Time.c:474-501](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L474-L501)
- **BR23914** — The same current can sweep away a monster's carried item.
  [Time.c:502-504](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L502-L504)
- **BR94905** — Standing in water, unless levitating above it, puts out any fire burning on you.
  [Time.c:209-216](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L209-L216)
- **BR24013** — When fire on you goes out, your light is restored and you see "you are no longer on fire.".
  [Time.c:1868-1878](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1868-L1878)
- **BR24083** — Fire cannot spread onto a tile that puts out fire, because its extinguishing nature outranks any flammable material there.
  [Time.c:1182-1196](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1182-L1196)
- **BR91575** — Stepping into webbing traps a creature for 3-7 turns; creatures immune to webs or submerged in water are never caught.
  [Time.c:287-308](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L287-L308)
- **BR24700** — Each turn you try to move while stuck in webbing the counter ticks down ("you struggle but cannot free yourself") until you break free and the web is destroyed ("you break free!").
  [Movement.c:1144-1162](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1144-L1162)
- **BR24824** — Leaving entangling terrain frees you from webbing immediately.
  [Time.c:2054-2056](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2054-L2056)
- **BR54331** — Standing on explosive ground deals 15-20 damage, but at least half your maximum hit points.
  [Time.c:311-315](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L311-L315)
- **BR24904** — Surviving explosive ground grants 5 turns of immunity to further explosions.
  [Time.c:311-315](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L311-L315)
- **BR25832** — Dampening armor absorbs an explosion ("Your <armor> pulses and absorbs the damage.") and may become identified.
  [Time.c:316-331](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L316-L331)
- **BR80482** — Ground or gas that harms you drains hit points equal to a fifteenth of your maximum, minimum 1, per 100 ticks.
  [Time.c:507-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L507-L545)
- **BR25953** — Mending ground heals you at the same rate, up to your maximum ("you feel much better.").
  [Time.c:547-559](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L547-L559)
- **BR99758** — Any creature standing over a chasm, open trap door, or similar hole, not levitating and not held fast by webs or crystal, is marked to fall.
  [Time.c:93-98](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L93-L98)
  [Time.c:160-171](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L160-L171)
- **BR26045** — The fall executes at the end of the turn, so fast monsters cannot act while suspended in mid-air.
  [Time.c:1369-1371](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1369-L1371)
  [Time.c:2249-2253](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2249-L2253)
- **BR80059** — When you fall, every monster and item still on the level falls with you to the level below rather than being left hanging above.
  [Time.c:997-999](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L997-L999)
- **BR70260** — Monsters that fall take 6-12 damage and reappear on the level below.
  [Time.c:1393-1411](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1393-L1411)
- **BR26194** — A falling monster you can see shows "<monster> plunges out of sight!".
  [Time.c:1384-1388](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1384-L1388)
- **BR26324** — Falling dungeon guardians and mirrored totems are destroyed outright instead, so they cannot block the level beneath.
  [Time.c:1390-1392](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1390-L1392)
- **BR97271** — Landing in deep water after a fall is unharmed.
  [Time.c:1015-1016](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1015-L1016)
- **BR26844** — Landing in other liquid halves the fall damage.
  [Time.c:1018-1019](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1018-L1019)
- **BR26994** — Landing on hard ground deals fall damage ("You are injured by the fall.") and can kill you ("Killed by a fall").
  [Time.c:1013-1013](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1013-L1013)
  [Time.c:1020-1026](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1020-L1026)
- **BR27132** — Falling on the deepest level instead teleports you away with "A strange force seizes you as you fall.".
  [Time.c:1031-1034](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1031-L1034)
- **BR18832** — Catching fire sets you ablaze for at least 7 turns.
  [Time.c:28-51](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L28-L51)
- **BR28134** — While burning you take 1-3 fire damage each turn and can die "Burned to death" until the flames burn out on their own.
  [Time.c:2328-2337](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2328-L2337)
- **BR79486** — Standing in fire ignites you.
  [Time.c:448-449](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L448-L449)
- **BR28360** — A burning creature standing on flammable terrain, unless submerged or falling, sets that terrain alight.
  [Time.c:450-459](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L450-L459)
- **BR95213** — Every 100 ticks, each burning square tries to ignite itself and its four orthogonal neighbours.
  [Time.c:1512-1526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1512-L1526)
- **BR28642** — Flammable material that catches fire is consumed and replaced by fire, and flammable gas burns its volume away entirely.
  [Time.c:1198-1229](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1198-L1229)
- **BR64999** — Whether a flammable square catches is a per-material chance rolled on each attempt.
  [Time.c:1181-1198](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1181-L1198)
- **BR28829** — A square that resists 12 exposures within one environment update will not be tried again until the counter resets at the next update.
  [Time.c:1175-1180](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1175-L1180)
  [Time.c:1431-1436](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1431-L1436)
- **BR21531** — Explosive terrain erupts once at least 8 of its neighbouring squares are on fire, block gas, or are themselves explosive.
  [Time.c:1202-1215](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1202-L1215)
- **BR77717** — Items lying on ground that catches fire are burned up and destroyed ("<item> burns up!"), leaving a burst of flame in their place.
  [Time.c:856-875](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L856-L875)
- **BR32647** — Each environment update, gas evens itself out, with every tile taking the average volume of itself and its eight neighbouring gas-permeable squares; a remainder unit is distributed randomly.
  [Time.c:1248-1275](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1248-L1275)
- **BR10960** — Chasms and open trap doors vent gas off the level entirely, counting as an extra neighbour with no gas.
  [Time.c:1269-1271](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1269-L1271)
- **BR29862** — Gases that dissipate quickly or slowly lose one unit of volume, with a 50% or 20% chance per update.
  [Time.c:1285-1291](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1285-L1291)
- **BR30087** — Any tile whose gas falls below one unit of volume becomes clear.
  [Time.c:1281-1284](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1281-L1284)
- **BR28946** — Whenever any gas is present, the gas simulation runs twice per environment update.
  [Time.c:1438-1449](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1438-L1449)
- **BR30543** — When two different gases meet, the incoming gas is capped at 3 units of volume on the mixed tile.
  [Time.c:1276-1280](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1276-L1280)
- **BR30707** — Gas on a square that cannot hold gas disperses instantly into its gas-permeable neighbours and clears.
  [Time.c:1292-1320](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1292-L1320)
- **BR37619** — After your action, time advances in ticks and every creature, including you, acts whenever its accumulated ticks run out, so faster creatures act more often.
  [Time.c:2386-2398](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2386-L2398)
  [Time.c:2449-2477](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2449-L2477)
- **BR30801** — Monsters that are activation-only, paralyzed, entranced, or captive do not act when their ticks run out; their timer is reset.
  [Time.c:2456-2462](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2456-L2462)
- **BR29293** — Moving costs time equal to your movement speed.
  [Time.c:2350-2354](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2350-L2354)
- **BR30898** — Attacking costs time equal to your attack speed.
  [Time.c:2208-2210](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2208-L2210)
- **BR30914** — A staggering weapon doubles the attack's time cost on a successful blow.
  [Time.c:2204-2205](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2204-L2205)
- **BR30996** — A quick weapon halves the attack's time cost.
  [Time.c:2206-2207](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2206-L2207)
- **BR91235** — Every 100 ticks staffs recharge and charms count down.
  [Time.c:2400-2403](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2400-L2403)
- **BR31024** — Every 100 ticks worn gear becomes better understood, gradually identifying itself.
  [Time.c:2404-2404](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2404-L2404)
- **BR31183** — Every 100 ticks the monster-spawn timer decrements.
  [Time.c:2405-2405](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2405-L2405)
- **BR31244** — Every 100 ticks every monster feels instantaneous tile effects.
  [Time.c:2407-2410](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2407-L2410)
- **BR32089** — Every 100 ticks every monster's status timers decrement.
  [Time.c:2412-2415](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2412-L2415)
- **BR32390** — Every 100 ticks the environment updates: fire spreads, gas diffuses, items float, and monsters fall.
  [Time.c:2429-2429](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2429-L2429)
- **BR32465** — Every 100 ticks your status timers decrement, draining hunger and wearing off timed effects.
  [Time.c:2430-2430](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2430-L2430)
- **BR32507** — Every 100 ticks you feel instantaneous tile effects, which can end the game.
  [Time.c:2431-2434](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2431-L2434)
  [Time.c:2574-2577](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2574-L2577)
- **BR32837** — Every 100 ticks monsters approach the stairs to enter your level.
  [Time.c:2435-2435](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2435-L2435)
- **BR77317** — While paralyzed you take no actions, but turns keep passing with monsters acting and the environment updating until the paralysis wears off.
  [Time.c:2560-2589](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2560-L2589)
- **BR18632** — New packs of monsters arrive on the level every 125 to 175 turns.
  [Time.c:2081-2084](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2081-L2084)
- **BR71925** — Your scent trail advances 3 units per turn, or 10 while you are invisible.
  [Time.c:2265-2269](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2265-L2269)
- **BR35018** — Scent is deposited on each square of your field of view, stronger near you, unless the square blocks scent or passage.
  [Movement.c:2481-2487](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2481-L2487)
  [Time.c:659-675](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L659-L675)
- **BR34974** — Nutrition drops by 1 each turn.
  [Time.c:1983-1987](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1983-L1987)
- **BR32966** — Nutrition does not drain while you are paralyzed.
  [Time.c:1981-1982](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1981-L1982)
- **BR32984** — Carrying the Amulet of Yendor slows the nutrition drain to one turn in five.
  [Time.c:1984-1984](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1984-L1984)
- **BR32368** — As nutrition falls you are told "you are hungry", "you feel weak with hunger", "you feel faint with hunger", and finally "you are starving to death!".
  [Time.c:814-836](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L814-L836)
- **BR32992** — Except for the first, the hunger warnings demand acknowledgment.
  [Time.c:826-836](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L826-L836)
- **BR33395** — The hunger warnings add " and have no food" when your pack contains no food.
  [Time.c:818-822](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L818-L822)
- **BR95832** — At zero nutrition you lose 1 hit point per turn and die when they run out ("Starved to death").
  [Time.c:2277-2282](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2277-L2282)
- **BR33602** — At nutrition 1 or less you are forced to gobble any food in your pack, which consumes a turn.
  [Time.c:839-854](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L839-L854)
  [Time.c:2632-2634](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2632-L2634)
- **BR60046** — You do not regenerate hit points while starving or poisoned.
  [Time.c:2283-2284](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2283-L2284)
- **BR33646** — Otherwise you slowly heal back toward your maximum as turns pass.
  [Time.c:2283-2298](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2283-L2298)
- **BR58362** — When a temporary effect such as levitation or fire immunity over lava is all that keeps you alive on your current square, you get one warning, "better head back to solid ground!", when the effect is about to run out with just enough time to reach safety.
  [Time.c:2599-2611](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2599-L2611)
- **BR33875** — Once you no longer have time to make it back to safety the warning becomes "you're past the point of no return!".
  [Time.c:2612-2616](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2612-L2616)
- **BR34157** — Discovering a new square that can be pathed through adds to the current turn's discovery count.
  [Time.c:742-750](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L742-L750)
- **BR34686** — The turn's discovery count is granted as experience to your allies on your level and on the levels immediately above and below.
  [Time.c:966-985](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L966-L985)
- **BR34818** — When you fall, a secret square you are standing on and can see is revealed.
  [Time.c:991-995](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L991-L995)
- **BR35136** — Your stealth range is 14 by default; invisibility sets it to 1, darkness and shadow each halve it, resting halves it rounding up, aggravation adds to it, and stealth rings subtract from it, with a floor of 2, or 1 when rested.
  [Time.c:686-726](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L686-L726)
- **BR35365** — Armor whose unenchanted strength requirement exceeds 12 increases your stealth range by the excess.
  [Time.c:677-684](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L677-L684)
- **BR35857** — Activating a machine powers and promotes every wired square belonging to it, and then each creature of that machine immediately takes a turn.
  [Time.c:1042-1095](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1042-L1095)
- **BR35874** — A machine containing a circuit breaker cannot be powered or activated.
  [Time.c:1097-1109](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1097-L1109)
- **BR35927** — Terrain with a random promotion chance is promoted in two passes each environment update.
  [Time.c:1451-1491](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1451-L1491)
- **BR36245** — Terrain that promotes on creature contact promotes every such layer while any creature, including a flying one, stands on it.
  [Time.c:253-265](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L253-L265)
- **BR36250** — Terrain that promotes on player entry promotes every such layer when you stand on it.
  [Time.c:267-274](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L267-L274)
- **BR36447** — Terrain that promotes on sacrifice entry promotes when a creature marked for sacrifice belonging to that machine stands on it.
  [Time.c:276-285](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L276-L285)
- **BR36633** — Electricity promotes any tile flagged to promote on electricity.
  [Time.c:1152-1166](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1152-L1166)
- **BR36750** — The warden of Yendor follows you between levels, appearing near the stairs after a delay.
  [Time.c:1334-1367](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1334-L1367)
- **BR36935** — A monster crossing a level is placed near the appropriate stairs, or the pit it fell through, and displaces any creature already there.
  [Time.c:1881-1932](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1881-L1932)
- **BR37097** — A monster entering through a pit takes 6-12 damage, announced as plummeting or falling from above.
  [Time.c:1934-1953](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1934-L1953)
- **BR37131** — Monsters on the level above or below approach the stairs and enter your level after a countdown.
  [Time.c:1956-1977](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1956-L1977)

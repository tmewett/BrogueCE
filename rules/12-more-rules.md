
----
BR27245
"Pressing `?` during play opens the help screen."
Citation: [IO.c:2610-2611](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2610-L2611)
Note: IO.c:2610-2611 handles BROGUE_HELP_KEY by calling printHelpScreen (IO.c:4066-4137). Existing help-screen rules BR25882, BR47435, and BR27347 describe three listed commands, BR39512 covers dismissal, and the NewRules entries BR40113, BR41110, and BR34710 describe how the overlay is drawn and restored, but no rule in rules/* states that the help key opens the help screen. ID BR27245 is unique across rules/.

----
BR37741
"Pressing `F` during play opens the Feats screen, which lists every feat with its achievement status."
Citation: [IO.c:2613-2614](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2613-L2614) [IO.c:4188-4237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4188-L4237)
Note: IO.c:2613-2614 handles FEATS_KEY by calling displayFeatsScreen (IO.c:4188-4237). BR33110 describes the screen's contents and dismissal but not the key binding that opens it, so the in-game behavior at this key press was uncaptured. ID BR37741 is unique across rules/.

----
BR12234
"Pressing `D` during play opens the discoveries screen, which lists every kind of scroll, ring, potion, staff, and wand and shows which ones you have identified."
Citation: [IO.c:2616-2617](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2616-L2617) [IO.c:4240-4276](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4240-L4276)
Note: IO.c:2616-2617 handles DISCOVERIES_KEY by calling printDiscoveriesScreen, whose implementation at IO.c:4240-4276 prints the SCROLLS, RINGS, POTIONS, STAFFS, and WANDS sections. No rule in rules/* describes this screen or its key binding; BR31563 only lists "view discovered items" as an actions-menu entry. ID BR12234 is unique across rules/.

----
BR27794
"Each frame of the animation advances every light flash that is still running and redraws the dungeon with the flash's current brightness; a flash that has finished fading is discarded so it never appears again, and the animation continues until no flash remains."
Citation: [Light.c:379-402](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L379-L402)
Note: Light.c:379-402 is the frame loop that keeps every still-active flare (Light.c:384-385), draws its current frame (Light.c:386-388, whose radius and colour scaling is at Light.c:359-363), frees and clears the flare once it is no longer active (Light.c:389-392), demotes visibility and refreshes the field-of-view display each frame (Light.c:395-396), and repeats while at least one flare remains (Light.c:402). BR14003 cites Light.c:289-348 and covers only the creation, fade rate, and expiry condition of a flare; no rule in rules/* describes the animation loop that advances and redraws active flashes and drops expired ones. ID BR27794 is unique across rules/ and test/.

----
BR19558
"While some part of a light flash is within your view, each animation frame is held for about 10 milliseconds so the flash can be seen; if no part of the flash is in view, the frames are not held and the animation completes immediately. While watching a recording with full knowledge of the level, the hold applies even when the flash is outside your own view."
Citation: [Light.c:394-399](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L394-L399)
Note: Light.c:397-399 pauses 10 milliseconds per frame only while not fast-forwarding, at least one flare is still active, and either some flare frame was in view or playback omniscience is on; otherwise the loop runs with no delay. The in-view result is set by Light.c:386-388. BR14003 describes the fade itself but no rule in rules/* mentions that animation frames are paced only when the flash is visible or that the pacing is 10 milliseconds. ID BR19558 is unique across rules/ and test/.

----
BR30142
"While fast-forwarding a recording, or while the game's true-colour display mode is on, the animation is never paused between frames, so light flashes play through without delay."
Citation: [Light.c:376-377](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L376-L377) [Light.c:397-398](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L397-L398)
Note: Light.c:377 marks the animation fast-forwarded when true-colour mode or recording fast-forward is on, and the pause at Light.c:397-398 is skipped whenever that flag is set. No rule in rules/* describes this suppression of flare-animation pauses; BR19558 covers only the ordinary in-view pacing. ID BR30142 is unique across rules/ and test/.

----
BR24247
"A light flash is drawn on top of the lighting that existed before the flash, and that earlier lighting is restored after every frame, so the flash leaves no permanent glow behind and the dungeon returns to its normal illumination when the animation ends."
Citation: [Light.c:376](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L376) [Light.c:400-403](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L400-L403)
Note: Light.c:376 backs up the dungeon's lighting before the animation begins; each frame records the old light values (Light.c:400) and restores the saved lighting (Light.c:401) after drawing the flash, and the field-of-view display is refreshed once more after the loop (Light.c:403). BR54362 covers the saving of old light values during ordinary lighting recomputation, but no rule in rules/* describes the flash being drawn over and then removed from the underlying lighting. ID BR24247 is unique across rules/ and test/.

----
BR17894
"Whenever the mouse button is pressed or released, or the pointer moves into a new cell, the game first removes the highlight from whichever menu button was highlighted before, so moving the pointer off a button clears its highlight instead of leaving it lit."
Citation: [Buttons.c:207-217](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L207-L217)
Note: Buttons.c:207-209 handles only mouse-down, mouse-up, and pointer-entered-cell events, and Buttons.c:214-217 clears the previously focused button before the new focus is located at Buttons.c:219-237, so a menu never keeps a stale highlight after the pointer leaves a button (and if the pointer is over no button, none stays highlighted). BR11267 describes the tint of the button currently under the pointer, and BR35576/BR91827 describe the pressed look and choosing on mouse-up, but no rule in rules/* states that the previous highlight is cleared on each mouse event. ID BR17894 is unique across rules/.

----
BR37749
"A menu control that is hidden or disabled cannot be highlighted or clicked with the mouse, even when the pointer is over the place where it would appear; only controls that are currently drawn on screen and enabled respond to the pointer."
Citation: [Buttons.c:221-223](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L221-L223)
Note: The pointer focus scan at Buttons.c:221-223 accepts a button only when both its drawn and enabled flags are set, so hidden controls (such as the invisible previous/next controls of BR23709) and disabled menu entries, including the drawn "---" separator rows, are skipped by the mouse. BR23709 says only that invisible controls remain reachable by hotkey, and BR11267's note mentions enabled buttons only in passing; no rule in rules/* describes that hidden or disabled buttons are excluded from pointer focus and clicks. ID BR37749 is unique across rules/.

----
BR36733
"When a guardian charm is used, its guardian appears on the nearest reachable square to you that no creature or staircase occupies and whose terrain the guardian does not avoid; among equally near squares one is chosen at random, and it never appears on your own square."
Citation: [Items.c:6665-6668](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6665-L6668)
Note: Items.c:6665-6668 creates the guardian and places it with the qualifying-location search: the destination may not be the player's square (blocking map flag HAS_PLAYER), may not hold another monster or a staircase (forbidden flags HAS_PLAYER | HAS_MONSTER | HAS_STAIRS), and may not be terrain the guardian avoids (the forbidden terrain flags come from avoidedFlagsForMonster, minus spontaneously igniting terrain). The search runs a distance scan outward from the player and picks a random one of the equally nearest qualifying squares, so the guardian materialises beside you. BR96573 says only that the charm summons a guardian, BR26583 covers its lifespan, BR84903 its flash and first-turn delay, and BR65190 its bond to you; no rule in rules/* describes where the guardian appears. ID BR36733 is unique across rules/.

----
BR14078
"The guardian summoned by a guardian charm is your ally but does not follow you around: when it has no enemy to fight, it mills about near where it appeared instead of trailing you."
Citation: [Items.c:6669](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6669), [Monsters.c:3153-3154](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3153-L3154), [Monsters.c:3223-3227](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3223-L3227)
Note: Items.c:6669 marks the guardian as a follower bound to the player that does not track its leader. Monsters.c:3223-3227 then makes such a monster mill about instead of pathing to its leader whenever it has no enemy or corpse to deal with, and Monsters.c:3153-3154 exempts it from the 4-square leash that otherwise limits which enemies an ordinary ally will pursue. BR63005 describes ordinary followers staying within 2 squares of their leader and BR96573 only says the guardian fights for you, so no rule in rules/* states that the guardian does not follow you. ID BR14078 is unique across rules/.

----
BR11468
"Each time you discharge a wand, the game records one more use of it, and a wand whose charges you have not yet learned reports that count in its description as "used once", "used twice", or "used N times"."
Citation: [Items.c:6654](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6654), [Items.c:1587-1598](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L1587-L1598)
Note: Items.c:6654 increments a wand's discharge counter on every successful discharge, not only when the wand runs out of charges. While a wand's charges are unknown (it is not identified and its maximum charges have not been learned), Items.c:1587-1598 prints the counter in the item description as "(used once)", "(used twice)", or "(used N times)"; once the charges are known the description shows the remaining charge count instead. BR46697 mentions the counter only while describing an empty wand and does not state what the player sees, so no rule in rules/* describes this use count. ID BR11468 is unique across rules/.

----
BR36580
"Eating a food ration prints "That food tasted delicious!", while eating a mango prints "My, what a yummy mango!"."
Citation: [Items.c:6711-6715](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6711-L6715)
Note: Items.c:6711-6715 is the message branch at the end of eating: a ration prints "That food tasted delicious!" and every other food kind (the mango) prints "My, what a yummy mango!". The same block sits at Items.c:6677-6681 in the working tree because later commits shifted this file, but the pinned commit cited here is authoritative. BR39781 covers the nutrition gain and BR42934 the not-hungry-enough confirmation, but no rule in rules/* describes these distinct messages. ID BR36580 is unique across rules/.

----
BR33715
"When a blow lands but deals no damage, the combat message adds that the attacker does no damage; when the player made that attack, it also counts as being disturbed, which interrupts running or auto-exploration."
Citation: [Combat.c:1140-1144](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1140-L1144)
Note: Combat.c:1140-1144 selects the zero-damage explanation clause ("but do/does no damage") and, when the attacker is the player, sets the player's disturbed flag. The message appears both for ordinary hits and for kills because the same clause is appended at Combat.c:1159-1170. BR27038 and BR77279/BR81065 describe sources of zero damage (absorption, weapon immunity, invulnerability) and BR10761 notes that the player's harmless attacks are allowed, but no rule in rules/* describes this zero-damage combat message or the player disturbance it sets. ID BR33715 is unique across rules/ and test/.

----
BR40200
"When a blow lands on a paralyzed defender, the combat message explains that the defender is struck while paralyzed."
Citation: [Combat.c:1147-1148](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1147-L1148)
Note: Combat.c:1147-1148 appends " while $HESHE is/are paralyzed" (with pronouns resolved for the defender) to the attack message when the defender was paralyzed at the start of the attack; the clause is also carried into the kill message at Combat.c:1166-1170. BR36538 covers the mechanics of attacking a paralyzed creature (auto-hit, triple damage, no retaliation) but says nothing about the reported explanation, and no other rule in rules/* mentions this paralyzed combat message. ID BR40200 is unique across rules/ and test/.

----
BR18636
"When a blow lands on a sleeping defender, the combat message explains that the defender was struck in its sleep."
Citation: [Combat.c:1149-1150](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1149-L1150)
Note: Combat.c:1149-1150 appends " in $HISHER sleep" (with pronouns resolved for the defender) to the attack message when the defender was asleep at the start of the attack, and the clause also appears in the kill message at Combat.c:1166-1170. BR66846, BR88066, and BR89965 cover the mechanics of attacking a sleeping creature (auto-hit, delayed retaliation, waking), but no rule in rules/* describes this sleep combat message. ID BR18636 is unique across rules/ and test/.

----
BR23006
"When an ally that can blink has an enemy inside the range it is allowed to pursue, it blinks toward that enemy only about 30% of the time, or every time if it is a kind that never holds back its ability; on other turns it simply walks toward the enemy."
Citation: [Monsters.c:3158-3161](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3158-L3161), [Monsters.c:3207-3208](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3207-L3208)
Note: Monsters.c:3159-3160 gates the ally's blink toward an enemy on a 30% random roll, bypassed when the monster never holds back its abilities, and Monsters.c:3207-3208 is the fallback walk toward the closest enemy when the blink does not happen. BR61728 states only that an allied blinker teleports toward the nearest enemy and omits the chance, and BR80604 covers the 30% chance only for a fleeing blink; no rule in rules/* describes this attack-side chance. ID BR23006 is unique across rules/.

----
BR12479
"When an ally blinks toward an enemy, it can choose only a foe that it is willing and able to attack, that it can reach, and whose square it does not refuse to occupy unless the foe can be struck through walls; it notices an invisible foe only about one time in three, always if it never holds back its ability."
Citation: [Monsters.c:3180-3194](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3180-L3194)
Note: Monsters.c:3182-3189 is the target filter that decides which enemies are planted as destinations in the blink preference map: it excludes creatures the ally will not attack, unreachable ones, avoided squares unless the creature is attackable through walls, and futile attacks, and it includes an invisible creature only on a 33% roll (automatically for a monster that never holds back its ability). Squares the ally avoids are also forbidden as landing spots at Monsters.c:3170-3172. BR61728 does not state these selection constraints, and BR69774 describes the 33% invisibility notice only for the ordinary target scan, not the blink map. ID BR12479 is unique across rules/.

----
BR32375
"A successful blink ends the creature's turn as though it had attacked, so its next action comes after its attack speed, doubled for a creature that casts spells slowly."
Citation: [Monsters.c:3199-3202](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3199-L3202), [Monsters.c:2355-2358](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2355-L2358)
Note: Monsters.c:3199-3202 makes the ally's attack-side blink both consume the turn at attack speed (doubled for slow casters) and return from the ally's action, and Monsters.c:2355-2358 applies the same delay inside the shared blink routine after every successful blink. BR56857 gives the attack-speed turn cost only to casting, and BR80604 says a fleeing blink ends its turn without the timing detail; no rule in rules/* states the turn cost of blinking itself. ID BR32375 is unique across rules/.

----
BR16275
"A square can hold more than one kind of terrain at once — floor, liquid, surface covering and gas — so a gas cloud drifting over lava makes the square count as both. When the game checks a square against a required terrain, such as choosing where a horde may spawn, where a spreading feature may grow, or a random matching location, it accepts the square if any of those layers is that terrain."
Citation: [Architect.c:39-46](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L39-L46)
Note: Architect.c:39-46 returns true when any of the four terrain layers — dungeon, liquid, surface or gas — equals the requested type. That any-layer match is what Monsters.c:725 and Monsters.c:810-815 use to test horde spawn terrain, what Architect.c:3340-3341 and Architect.c:3365 use to require propagation terrain for a spreading feature, and what Architect.c:3866 uses for random matching locations. BR48210 covers horde re-rolling and BR67454 covers terrain-matched random placement, but no rule in rules/* states that a square carries several terrains at once or that matching checks every layer. ID BR16275 is unique across rules/ and test/NewRules.txt.

----
BR38319
"When the game judges the level's open space — for example to tell a hallway from a room, to check whether a lake, machine or feature would cut off part of the level, or to rebuild a vault's interior — ordinary walkable floor counts as open, while walls, lava, deep water, chasms, traps and fire count as solid. Secret doors, doors opened by a key, and door-like barriers such as a portcullis, barricade or statue instead count as open even though they block walking."
Citation: [Architect.c:48-55](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L48-L55)
Note: Architect.c:48-55 is the passability test: a tile without a pathing blocker passes, and a pathing blocker passes only if it obstructs passability and is secret, key-promoted or level-connecting. Its callers decide hallway-versus-room shape (Architect.c:187-188 via passableArcCount, used at Architect.c:512 and Architect.c:3668 and by Items.c:632,733, Monsters.c:829,1500-1501, Grid.c:188,301), whether blocking terrain would disconnect the level (Architect.c:3156,3185-3200, gating Architect.c:728-731,1219-1222,1473,1860,3436), and vault interior rebuilding (Architect.c:754). BR59986, BR86387, BR44816, BR82515 and BR68008 describe those analyses but none states which terrain counts as open or that these door types are openings. ID BR38319 is unique across rules/ and test/NewRules.txt.

----
BR32708
"To shape a cave or lake, the game measures the largest connected region left after smoothing and regenerates the whole pattern until that region's bounding box is at least the required minimum width and height. It then keeps only that largest region and discards every smaller one, so the finished feature is a single connected body of at least the required size."
Citation: [Grid.c:468-507](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L468-L507), [Grid.c:527-540](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L527-L540)
Note: Grid.c:468-483 marks each connected region and records the largest one, Grid.c:490-507 scans its columns (and the rows that follow) to find its minimum and maximum coordinates, and Grid.c:524-529 computes the width and height from that bounding box, repeating the entire noise-and-smoothing attempt while the largest region is smaller than the requested minimum or absent. Grid.c:531-540 then rewrites the grid so only the largest region survives. BR55910 says a lake blob is at least 4 x 4 but attributes that only to the call site, BR72185 and BR51980 give cave and cavern sizes from the call site, and BR10340 describes contiguous-region measurement; none states that the largest region is selected, that its bounding box controls the retry, or that all other regions are discarded. ID BR32708 is unique across rules/, and new_rule_id.sh generated it.

----
BR15799
"When you switch between text, graphical, and hybrid display modes, or pause, resume, or toggle omniscience during a recording, the notice appears in teal."
Citation: [GlobalsBase.c:88](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L88), [IO.c:2651-2662](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L2651-L2662), [Recordings.c:813-820](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L813-L820), [Recordings.c:877-881](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L877-L881)
Note: GlobalsBase.c:88 fixes teal's shade; IO.c:2652, 2657 and 2662 pass it to the three display-mode notices, and Recordings.c:814, 820, 878 and 880 pass it to the pause, resume, omniscience-on and omniscience-off notices. BR31995 covers only bad/good/neutral event coloring, and no rule in rules/* mentions these teal status notices. ID BR15799 is unique across rules/.

----
BR39384
"A dar blademaster is drawn in purple."
Citation: [GlobalsBase.c:89](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L89), [Globals.c:1086](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1086)
Note: GlobalsBase.c:89 defines purple, and the dar blademaster's catalog row at Globals.c:1086 uses it as the creature's foreground color. BR35726 covers the drawn colors of six other monsters but not the dar blademaster, and no rule in rules/* states this creature's purple. ID BR39384 is unique across rules/.

----
BR15111
"The progress bar shown while a recording is playing back or a game is loading is dark purple."
Citation: [GlobalsBase.c:90](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L90), [IO.c:3750-3754](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L3750-L3754), [Recordings.c:745-749](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L745-L749), [Recordings.c:1344-1347](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L1344-L1347)
Note: GlobalsBase.c:90 defines dark purple; IO.c:3753 fills the playback turn counter with it, and Recordings.c:747 and 1345 fill the "[ Loading... ]" bar with it during seeking and saved-game loading. BR32262 and BR52091 describe the progress bars' presence but not their color, and no rule in rules/* states that they are dark purple. ID BR15111 is unique across rules/.

----
BR37020
"The dense tangle of netting dropped by a netting trap is drawn brown."
Citation: [GlobalsBase.c:91](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L91), [Globals.c:471](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L471), [Globals.c:772](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L772)
Note: GlobalsBase.c:91 defines brown; the netting terrain row at Globals.c:471 uses it as the net's foreground color, and Globals.c:772 is the "a net falls from the ceiling!" feature that creates that netting. Existing netting rules cover entanglement and burning only, and no rule in rules/* describes the net's brown color. ID BR37020 is unique across rules/.

----
BR11437
"Recurring poison damage flashes the victim green."
Citation: [GlobalsBase.c:92](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L92), [Time.c:2339-2342](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2339-L2342), [Monsters.c:1908-1911](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1908-L1911)
Note: GlobalsBase.c:92 defines green; Time.c:2341 passes it as the flash color for the player's poison tick, and Monsters.c:1910 does the same for a poisoned monster's tick. The initial poisoning uses a different color at Combat.c:1632, so the rule deliberately covers only recurring poison damage. No rule in rules/* describes this green flash. ID BR11437 is unique across rules/.

----
BR10160
"When your poisoning crosses a new severity threshold, a warning such as >25% poisoned or Fatally poisoned appears over your figure in yellow on a dark green background."
Citation: [GlobalsBase.c:93](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L93), [Time.c:917-929](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L917-L929)
Note: GlobalsBase.c:93 defines dark green; Time.c:923-928 builds the threshold warning text and passes yellow as the foreground and dark green as the background to the alert that flashes over the player. Existing poison rules cover damage and duration but not this alert's appearance, and no rule in rules/* mentions the dark green background. ID BR10160 is unique across rules/.

----
BR21569
"While you are on fire, each turn's burning damage flashes your figure orange."
Citation: [GlobalsBase.c:94](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L94), [Time.c:2328-2333](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2328-L2333)
Note: GlobalsBase.c:94 defines orange; Time.c:2330 passes it to the damage routine for each burning tick on the player. Burning rules describe ignition, damage and extinguishing but not this flash, and no rule in rules/* states that burning damage flashes orange. ID BR21569 is unique across rules/.

----
BR10759
"A flood trap is drawn blue."
Citation: [GlobalsBase.c:96](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L96), [Globals.c:390](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L390)
Note: GlobalsBase.c:96 defines blue, and the revealed flood trap's terrain row at Globals.c:390 uses it as the trap's foreground color; while the trap is still secret the hidden row at Globals.c:389 shows ordinary floor instead. Trap rules cover triggering and resetting but not appearance, and no rule in rules/* describes the flood trap's blue. ID BR10759 is unique across rules/.

----
BR13937
"Drinking a potion of descent flashes your view dark blue."
Citation: [GlobalsBase.c:97](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L97), [Items.c:7298-7302](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7298-L7302)
Note: GlobalsBase.c:97 defines dark blue; Items.c:7299 flashes the field of view in it as the potion opens the floor and drops the player. Existing descent-potion rules cover the fall and the hole but not this flash, and no rule in rules/* describes it. ID BR13937 is unique across rules/.

----
BR28391
"The gilded doors leading out of the dungeon are drawn light blue."
Citation: [GlobalsBase.c:99](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L99), [Globals.c:335](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L335)
Note: GlobalsBase.c:99 defines light blue, and the dungeon-exit terrain row at Globals.c:335 uses it as the gilded doorway's foreground color. BR87046 identifies the exit and BR37646/BR38077 cover its refusal messages, but no rule in rules/* describes the doors' light blue color. ID BR28391 is unique across rules/.

----
BR31847
"In a monster's sidebar entry, its mutation is named in a color matching the mutation: explosive orange, agile light blue, juggernaut brown, vampiric red, toxic green, and reflective dark turquoise."
Citation: [GlobalsBase.c:88-103](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L88-L103), [Globals.c:1398-1412](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1398-L1412), [IO.c:4576-4588](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4576-L4588)
Note: GlobalsBase.c:88-103 fixes the shades; the mutation rows at Globals.c:1398, 1402, 1404, 1408, 1410 and 1412 assign them as each mutation's text color, and IO.c:4578-4582 colors the parenthesized mutation title in the sidebar with that color. BR85934 notes that the sidebar shows a mutation but not its color, and no rule in rules/* describes these mutation label colors. ID BR31847 is unique across rules/.

----
BR20863
"A monster whose powers have been fully negated is marked Negated in the sidebar in pink, or dark pink when its entry is dimmed because another creature is focused."
Citation: [GlobalsBase.c:100-101](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L100-L101), [IO.c:4720-4722](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4720-L4722)
Note: GlobalsBase.c:100-101 define pink and dark pink; IO.c:4722 prints the "Negated" label with pink normally and dark pink when the entry is dim, and IO.c:3870-3873 sets dim for every listed creature other than the focused one. Negation rules cover the loss of powers but not this label or its colors, and no rule in rules/* mentions it. ID BR20863 is unique across rules/.

----
BR18307
"A fire trap is drawn red."
Citation: [GlobalsBase.c:102](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L102), [Globals.c:388](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L388)
Note: GlobalsBase.c:102 defines red, and the revealed fire trap's terrain row at Globals.c:388 uses it as the trap's foreground color; the hidden row at Globals.c:387 shows ordinary floor until it is discovered. Trap rules cover triggering, and BR21569 covers burning damage, but no rule in rules/* describes the fire trap's red appearance. ID BR18307 is unique across rules/.

----
BR18146
"The cloud of healing spores released by bloodwort is drawn dark red."
Citation: [GlobalsBase.c:103](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L103), [Globals.c:510](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L510)
Note: GlobalsBase.c:103 defines dark red, and the healing-cloud gas row at Globals.c:510 uses it as the cloud's background color. Existing bloodwort and healing rules cover the spores' healing effect but not the cloud's appearance, and no rule in rules/* describes its dark red color. GlobalsBase.c:100 (pink) is already covered for paralyzing gas by BR29835, and GlobalsBase.c:95 (dark orange) appears only in a commented-out flash at Items.c:7286, so it has no observable behavior and no rule is needed for it. ID BR18146 is unique across rules/.

----
BR33997
"Whenever the game draws a number from a range with a clumping factor greater than one, it adds together that many separate uniform rolls, so values cluster toward the middle of the range instead of being equally likely; when the range's width is not evenly divisible by the clumping factor, the first few rolls may each reach one higher so the top of the range can still come up."
Citation: [Math.c:40-59](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Math.c#L40-L59)
Note: Math.c:48 computes the base number of sides, and Math.c:50-56 add `(upperBound - lowerBound) % clumpFactor` rolls with one extra side before filling the remaining `clumpFactor` rolls with the base sides; Math.c:58 adds the lower bound. This single clumped draw supplies attack damage (Combat.c:1093, 1098-1099), generated weapon/armor/wand/ring/charm stats (Items.c:283, 345, 352, 369), thrown-weapon damage (Items.c:6022), potion magnitude (Items.c:7234), light radius (Light.c:64), horde size (Monsters.c:711), fall damage (Time.c:1022), and staff damage (PowerTables.c:51). BR90053 describes the same dice-clumping effect only for an attack's damage and cites Combat.c, so no rule in rules/* states it as the shared range roll or cites Math.c for it. ID BR33997 is unique across rules/.

----
BR19740
"A percentage roll succeeds when a random whole number from 0 to 99 is below the requested chance, so a 0% chance never succeeds and a 100% or greater chance always succeeds; chances outside 0 to 100 are clamped to that range."
Citation: [Math.c:61-64](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Math.c#L61-L64)
Note: Math.c:63 draws uniformly from 0 through 99 and compares it with the requested percent after clamping that percent to 0-100, which is the shared test behind the many per-event percentage rules (e.g. nausea, stealth notices, rune activation). Those rules state their individual chances but no rule in rules/* describes this common percentage roll or cites Math.c:61-64. ID BR19740 is unique across rules/.

----
BR17360
"When the game randomizes the order of a set of candidate tiles, directions, or other list entries, it produces a fair shuffle in which every possible ordering is equally likely, so no entry is systematically favored."
Citation: [Math.c:66-81](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Math.c#L66-L81)
Note: Math.c:71-80 is a Fisher-Yates shuffle: for each position it swaps with a uniformly chosen position from that index through the end (Math.c:74), so every permutation is equally likely. It orders the tiles scanned while adding dungeon loops (Architect.c:351), growing rooms (Architect.c:705-707, 1196-1198), placing a room's doorways (Architect.c:2404), building bridges (Architect.c:2828-2830), and laying waypoints (Architect.c:3081); it also chooses a hallway direction (Architect.c:2242), directions for a monster's movement (Monsters.c:2167-2168), and the order in which a machine's tiles are processed (Time.c:1055-1057). BR15561 mentions item flavors being shuffled anew but describes a different hand-rolled swap loop in Items.c rather than this shared list shuffle; no rule in rules/* describes the fair shuffle or cites Math.c:66-81. ID BR17360 is unique across rules/.

----
BR35620
"When gas is stranded on a square that cannot hold it, its volume is split as evenly as possible among the neighbouring squares that can hold gas, each of which takes on that same kind of gas, and any volume that does not divide evenly is lost."
Citation: [Time.c:1311-1319](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1311-L1319)
Note: Time.c:1311 gives each gas-permeable neighbour `pmap[i][j].volume / numSpaces`, an integer division that discards the remainder; Time.c:1312-1313 copies the gas type only when that share is at least 1; Time.c:1318-1319 zeroes the source square and clears its gas. BR30707 states that gas on a square that cannot hold gas disperses instantly into its gas-permeable neighbours and clears, but not how the volume is shared out or that the share is truncated, and BR32647 describes the averaging diffusion for squares that can hold gas rather than this instant dispersal. ID BR35620 is unique across rules/.

----
BR37725
"The ambient light that fills the dungeon is neutral grey near the surface and shifts gradually toward a darker blue-grey as you descend, reaching its deepest shade at the depth of the amulet."
Citation: [Globals.c:120-121](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L120-L121), [Globals.c:293-296](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L293-L296), [RogueMain.c:541-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L541-L545)
Note: Globals.c:120-121 define the ambient light's surface shade as light grey and its deepest shade as blue-grey, Globals.c:295 registers the pair as the first dynamic colour bound, and RogueMain.c:541-545 blends the light from the first shade toward the second by the fraction of the amulet's depth reached, so the tint changes level by level. Light.c:283-287 compares a tile's light with this ambient light to decide darkness, and IO.c:1525-1535 performs the blend. Existing rules mention the miner's light only as a darkness threshold (BR70413) or a minimum under darkness (BR23366); none describes its colour or depth progression. Globals.c:118 defines a sacred glyph light colour that is referenced nowhere in src/, so it has no observable effect and no rule was added for it. ID BR37725 is unique across rules/.

----
BR29950
"Deep water is drawn as a dark blue that shimmers with every refresh, and the shade behind it grows darker as you descend toward the amulet."
Citation: [Globals.c:124-126](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L124-L126), [Globals.c:297](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L297), [Globals.c:413](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L413), [RogueMain.c:541-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L541-L545)
Note: Globals.c:124 defines the dark blue foreground with random components and colorDances set, so it ripples on every refresh, and Globals.c:125-126 bound the background from a lighter blue at the surface to a darker blue at the amulet depth; Globals.c:297 registers that pair as a dynamic colour blended by RogueMain.c:541-545, and Globals.c:413 is the deep-water terrain row that uses both. BR51947, BR23859, BR54253, BR97271, BR17312 and BR78750 cover submersion, surfacing, vision loss, item loss and fall damage in deep water, but no rule in rules/* describes its appearance. ID BR29950 is unique across rules/.

----
BR38901
"Shallow water is drawn in a lighter blue that shimmers with every refresh, and the shade behind it darkens as you descend toward the amulet."
Citation: [Globals.c:127-129](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L127-L129), [Globals.c:298](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L298), [Globals.c:414](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L414), [RogueMain.c:541-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L541-L545)
Note: Globals.c:127 defines the blue foreground with random components and colorDances set, Globals.c:128-129 bound the background from a brighter blue at the surface to a darker blue at the amulet depth, Globals.c:298 registers that pair as a dynamic colour, and Globals.c:414 is the shallow-water terrain row that uses them; RogueMain.c:541-545 applies the depth blend. Flooding machines reuse the same colours at Globals.c:432-433 and Globals.c:445-446. Existing rules mention shallow water only as a landing surface that halves fall damage (BR16982) and as not submerging the player (BR51947); no rule in rules/* describes its appearance. ID BR38901 is unique across rules/.

----
BR17041
"A wall-mounted torch is drawn as a warm orange flame that flickers with every refresh."
Citation: [Globals.c:122](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L122), [Globals.c:337](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L337), [Globals.c:344-345](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L344-L345)
Note: Globals.c:122 defines torch orange with colorDances set, and the ordinary wall torch's terrain row at Globals.c:337 uses it as the foreground colour; the dormant and transitioning haunted torches at Globals.c:344-345 use the same colour until they transform. BR34933 describes the torch and candle light cast into the room (and the dormant haunted torch flame) but no rule in rules/* states that an ordinary wall-mounted torch is itself drawn as a flickering orange flame. ID BR17041 is unique across rules/.

----
BR21724
"A creature that splits when struck leaves its clone on an unoccupied square in one of the four cardinal directions from the connected mass of the creature and its adjacent friendly creatures, chosen at random, and never on terrain the creature would avoid."
Citation: [Combat.c:226-256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L226-L256), [Combat.c:264-274](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L264-L274)
Note: Combat.c:226-256 marks the connected mass of the splitter and all cardinally adjacent friendly creatures, then treats every in-map cardinal neighbour of that mass as a candidate unless it is already marked, holds the player or a monster, or is avoided by the creature (monsterAvoids at 248); Combat.c:264-274 picks one candidate at random and places the clone there. Existing BR97180 only says an adjacent open space is used and BR74866 only says a free adjacent square is available, so neither states the contiguous-group perimeter, the cardinal restriction, the random choice, or the avoidance and occupancy filters. ID BR21724 is unique across rules/.

----
BR30875
"If the creature's attacker is directly adjacent when the blow lands, the attacker's square counts as part of the connected mass, so the clone can appear on the far side of the attacker — for example behind you when you fight a jelly in a corridor."
Citation: [Combat.c:210-232](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L210-L232)
Note: Combat.c:210-223 admits the split only for a living creature with the clone-on-defend ability under the clone cap and, when the attacker is within one square (line 219), records the attacker's square as loc; Combat.c:229-232 adds that square to the contiguous-monster group before eligible edges are found, which is exactly what lets a jelly clone appear behind the player in a hallway. Neither BR97180 nor BR74866 describes this attacker-square supplementation. ID BR30875 is unique across rules/.

----
BR13651
"When a scroll of recharging or a recharging charm restores your staffs, each staff also starts a fresh recharge delay: a staff of blinking or obstruction waits twice as long to regain each charge as any other staff, and the delay is divided by the staff's enchantment level, so a more enchanted staff regains its charges faster."
Citation: [Items.c:3997-4001](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3997-L4001)
Note: Items.c:3998-3999 counts each carried staff and Items.c:4000 refills its charges to its enchantment level, while Items.c:4001 resets its recharge countdown to 10,000 for a staff of blinking or obstruction and 5,000 for any other staff, divided by the staff's enchantment level (the same formula used for the ordinary per-tick recharge in Time.c:1827-1830). BR99307 and BR75807 say a scroll of recharging refills staffs and readies charms but give no recharge delay, BR55510 says enchanting a staff speeds its recharge without the formula, and BR91235 only says staffs recharge every 100 ticks. The WAND branch at Items.c:4003-4005 (add one charge to each wand) is unreachable in the current game because the only callers pass STAFF (Items.c:6755) or STAFF | CHARM (Items.c:7079), so it has no observable behavior and no rule. ID BR13651 is unique across rules/.

----
BR41287
"When recharging affects at least one staff, wand, or charm in your pack, a message says a surge of energy courses through your pack recharging them, naming only the kinds actually recharged; if you carry nothing it can recharge, the same surge is announced but 'nothing happens.'"
Citation: [Items.c:4013-4043](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L4013-L4043)
Note: Items.c:4013 counts how many of the three kinds were found, Items.c:4015-4040 builds "a surge of energy courses through your pack, recharging your …" listing staffs, wands and charms in that order with "and"/commas and the correct singular or plural, and Items.c:4041-4042 falls back to "a surge of energy courses through your pack, but nothing happens." when no qualifying item is carried. This player-facing text belongs to the recharging routine but is not described by BR99307 or BR75807, which only state that staffs refill and charms ready. ID BR41287 is unique across rules/.

----
BR26601
"An unidentified ring is named after a gemstone, such as a ruby ring or an opal ring, and no two kinds of ring share the same gemstone in a given game. The gemstone names are shuffled anew each game."
Citation: [Globals.c:1548-1569](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1548-L1569), [Globals.c:1656-1664](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1656-L1664), [Items.c:1628-1643](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L1628-L1643), [Items.c:8006-8017](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L8006-L8017)
Note: Globals.c:1548-1569 defines the 18-name gemstone pool, Globals.c:1656-1664 builds the ring table from it, and Items.c:1638-1642 prints the gemstone before "ring" for an unidentified ring (an identified ring instead reads "ring of <effect>", Items.c:1629-1630). Items.c:8006-8017, within the flavor shuffle at Items.c:7963-8040, reshuffles the gemstone names each game, implementing the per-game reshuffle clause (BR15561 covers item flavors generally). No other rule in rules/* says rings are named with gemstones or that each ring kind gets a distinct one.

----
BR23444
"An unidentified wand is named after a metal, such as a nickel wand or a copper wand, and no two kinds of wand share the same metal in a given game; the metal names are shuffled anew each game."
Citation: [Globals.c:1531-1546](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1531-L1546), [Items.c:1562-1579](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L1562-L1579)
Note: Globals.c:1531-1546 defines the 12-name metal pool, and Items.c:1573-1578 prints the metal before "wand" for an unidentified wand (an identified wand instead reads "wand of <effect>", Items.c:1563-1566). The default wand table draws its flavors from these metal names at src/variants/GlobalsBrogue.c:699-708, which lies outside src/brogue/ and so is not linked. BR15561 covers the per-game reshuffling of item flavors generally, but no rule in rules/* says wands are named with metals or that each wand kind gets a distinct one.

----
BR32252
"When you look at a square that holds an item but no creature, the item is described as 'lying' there, as 'drifting' if the terrain moves items such as deep water, or as 'enclosed' if the terrain blocks passage."
Citation: [Movement.c:379-383](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L379-L383)
Note: Movement.c:370 enters this branch only when the square has no creature; Movement.c:379-380 appends " enclosed" when the terrain obstructs passage, and Movement.c:381-383 otherwise appends " drifting" when the terrain moves items or " lying" when it does not. BR12275 cites Movement.c:401-405 only to say that terrain under the cursor prints its description in the message line, and no rule in rules/* states this wording for an item's position. ID BR32252 is unique across rules/ and test/.

----
BR13208
"A looked-at item on the map is described with 'is' when it is a single item and with 'are' when it is a stack of more than one or a pile of gold; while you are hallucinating, it is always described with 'is' regardless of quantity."
Citation: [Movement.c:374-377](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L374-L377)
Note: Movement.c:374-375 forces the singular verb whenever the player is hallucinating outside omniscience playback; otherwise Movement.c:377 chooses the plural for a quantity above one or for gold. BR13847 covers hallucinated item names but not this verb agreement, and no rule in rules/* describes singular/plural agreement in the location description. ID BR13208 is unique across rules/ and test/.

----
BR25045
"When an item is described on the map, it is joined to its terrain with 'in' or 'through' if the terrain is one creatures stand in, and with 'on' or 'across' otherwise; the 'through' and 'across' forms are used when the item is drifting."
Citation: [Movement.c:384-385](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L384-L385)
Note: Movement.c:373 decides whether the item is drifting from whether the terrain moves items, and Movement.c:384-385 then chooses "through" or "in" for terrain creatures stand in and "across" or "on" otherwise, taking the moving form in each pair. BR57874 describes an item actually drifting across the floor but not this wording, and no rule in rules/* covers the preposition chosen for a looked-at item. ID BR25045 is unique across rules/ and test/.

----
BR27806
"Looking at a square that holds neither a creature nor an item reports 'you see' that terrain when the square is directly visible and 'you sense' it when the square is only perceived indirectly."
Citation: [Movement.c:390-393](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L390-L393)
Note: Movement.c:390-393 is the no-creature, no-item outcome: it prints "you see <terrain>" when the square is directly visible and "you sense <terrain>" when it is not. BR44457 covers only the sidebar's ordering of directly seen before indirectly sensed entries, and BR12275 only that a looked-at terrain description is printed; no rule in rules/* describes this see/sense wording. ID BR27806 is unique across rules/ and test/.

----
BR13948
"Unless you are hallucinating, a looked-at item whose full name would overflow the space left on the description line is named more tersely instead, though the finished sentence can still be wider than the map."
Citation: [Movement.c:387-388](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L387-L388), [Movement.c:130-139](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L130-L139)
Note: Movement.c:387-388 builds the sentence without the item name and then asks for the item's name with whatever width remains on a dungeon-wide line; Movement.c:133-135 retries with the terse form when the detailed name is too long, and the comment at Movement.c:122-126 notes the result may still exceed the width. BR13847 covers only hallucinated item names, and no rule in rules/* describes this short-name fallback for a looked-at item. ID BR13948 is unique across rules/ and test/.

----
BR17651
"In wizard mode, backing out of the monster list cancels monster creation entirely: no monster is created, and the prompt asking where to place it never appears."
Citation: [Wizard.c:358-361](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L358-L361)
Note: Wizard.c:358 shows the list of monsters on the chosen page, and Wizard.c:359-361 returns immediately when the chooser reports no selection (cancelled with Escape, or the list closed without a pick), so generateMonster at Wizard.c:365 is never reached and the mutation, placement-prompt, and targeting steps at Wizard.c:367-382 do not run. BR27330 covers creating a monster from the catalog, BR79970 covers the paging, and BR16066/BR10231/BR24559 cover the mutation offer and placement, so no rule in rules/* describes this cancel path. ID BR17651 is unique across rules/ and test/.

----
BR12805
"When the burden armor's rune triggers, the game announces 'your <armor> suddenly feels heavier!', raises the armor's strength requirement by one, and immediately re-equips the armor so any change in its weight takes effect at once."
Citation: [Combat.c:952-957](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L952-L957)
Note: Combat.c:952-957 is the burden branch of the armor rune handler: on a 10% roll it increments the armor's strength requirement (953), prints "your <armor> suddenly feels heavier!" (954), and calls the equip routine on the already-worn armor (955), which re-evaluates its weight at Items.c:7747-7753. BR99068 cites the same range but describes only the 10% chance and the permanent strength-requirement increase, while no rule in rules/* describes the "suddenly feels heavier" announcement or the immediate re-equipping that applies the new weight right away. ID BR12805 is unique across rules/ and test/.

----
BR33558
"Vulnerability armor doubles the damage its wearer takes from a thrown weapon, not just from melee blows."
Citation: [Combat.c:959-960](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L959-L960), [Items.c:6054-6059](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6054-L6059)
Note: Items.c:6054-6059 routes a thrown weapon that hits the player through the same armor-rune handler with the melee flag false, and the vulnerability branch at Combat.c:959-960 doubles the damage unconditionally (the melee flag is consulted only by the reprisal branch). BR65297 describes vulnerability as doubling only melee damage, so no rule in rules/* describes the doubled damage from thrown weapons. ID BR33558 is unique across rules/ and test/.

----
BR41718
"Pressing the up or down arrow while an item's detail box is open steps to the previous or next carried item in a loop, so going down past the last item returns to the first and going up past the first returns to the last."
Citation: [Items.c:3101-3112](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3101-L3112), [Items.c:3035-3043](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3035-L3043)
Note: Items.c:3101-3106 decrements the highlighted line on an up press and wraps a negative line to the last item, and Items.c:3107-3112 increments it on a down press and wraps a line at or past the item count back to 0; the same wrap is applied when the arrow keys are pressed on the inventory list itself at Items.c:3035-3043, which then opens the detail box for the wrapped item. BR43577 states only that the up and down arrows page through the carried items and BR23709 only that the invisible paging controls exist, so no rule in rules/* describes the wrap-around at either end of the list. ID BR41718 is unique across rules/ and test/.

----
BR38402
"Choosing Apply, Equip, Remove, Drop, Throw, Relabel, or Call in an item's detail box performs that action at once and closes the pack, so the game does not go on to treat that item as the one chosen for the command that opened the pack."
Citation: [Items.c:3120-3124](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3120-L3124), [Items.c:7574-7577](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7574-L7577)
Note: Items.c:3120-3124 is the branch taken after any action key other than up or down: it restores the previous screen and returns 0 from the inventory instead of the chosen item's letter, and the caller at Items.c:7574-7577 interprets that 0 as "the player acted directly from the inventory screen via a button" and reports no item, so the pending command does nothing further. BR83394, BR13225, BR68240, and BR29231 describe only which buttons the detail box offers, and BR54814 covers only opening the pack to pick an item for a command, so no rule in rules/* describes the action being performed from the detail box or the pack closing as a result. ID BR38402 is unique across rules/ and test/.

----
BR36336
"A fleeing monster moves toward whichever neighbouring square is farthest from you by walking distance, so it retreats along corridors and around walls rather than in a straight line."
Citation: [Time.c:1687-1730](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1687-L1730), [Monsters.c:3505](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3505), [Movement.c:1523-1526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1523-L1526)
Note: Time.c:1687 seeds your square at zero, Time.c:1721-1723 converts walking distances into safety values that fall off with distance, and Time.c:1730 propagates them outward; Monsters.c:3505 asks for the neighbouring square with the lowest safety value and Movement.c:1523-1526 accepts it only when it improves on the current square, is not corner-blocked, and is known passable. BR65017 covers the blink-or-step choice and the refusal to enter a square no safer than its own, but no rule states that a fleeing monster ranks neighbouring squares by walking distance from you. ID BR36336 is unique across rules/ and test/.

----
BR24649
"A fleeing monster will not flee onto lava, sacred ground, a chasm, a trapdoor, or a square held by a sleeping, long-stationary, activation-only, or allied creature, unless that creature is itself fleeing."
Citation: [Time.c:1628-1659](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1628-L1659)
Note: Time.c:1628-1632 forbids sacred ground and lava to the monster, Time.c:1639-1650 forbids squares held by sleeping, long-stationary, activation-only, or allied creatures (except another fleeing creature), and Time.c:1653-1659 forbids auto-descent and trapdoor tiles. BR65017 covers the step itself and BR36336 covers how far a square is from you, but no rule lists the terrain and occupied squares a fleeing monster refuses. ID BR24649 is unique across rules/ and test/.

----
BR38375
"A fleeing monster treats deep water and ground that spontaneously ignites as long detours rather than as impassable, so it avoids them unless every alternative is worse."
Citation: [Time.c:1667-1673](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1667-L1673)
Note: Time.c:1667-1673 assigns deep water and spontaneously igniting terrain a movement cost of 5 for the monster's safety map while leaving it passable, so the route is allowed but heavily penalised. BR65017 covers the resulting step and BR24649 covers outright-forbidden squares; no rule describes this detour cost. ID BR38375 is unique across rules/ and test/.

----
BR35598
"A fleeing monster steps only onto ground you have already found passable or a secret door you have discovered."
Citation: [Movement.c:1523-1526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1523-L1526)
Note: Movement.c:1525 requires a candidate square to be known to the player as passable or as a secret door before a creature will step there, which is the discovered-ground limit on fleeing. BR65017 covers the step itself and BR24649 and BR38375 cover the terrain preferences; no rule states this knowledge requirement. ID BR35598 is unique across rules/ and test/.

----
BR32744
"A fleeing ally moves toward whichever neighbouring square is farthest by walking distance from the nearest hostile, not from you, so it runs away from enemies. It shuns squares occupied by you or by hostiles and sacred ground, and it will not move at all unless that square is strictly safer than the one it is leaving."
Citation: [GlobalsBase.c:36](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L36), [Time.c:1545-1605](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1545-L1605), [Monsters.c:3092-3108](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3092-L3108)
Note: GlobalsBase.c:36 declares the ally safety map. Time.c:1545-1605 seeds it at zero on squares held by your enemies (1569-1572), forbids your square and hostile-held squares for the ally's own movement cost (1566-1578), forbids sacred ground (1566-1568) and impassable or pathing-blocking terrain (1560-1565), then curves the remaining distances and negates them (1592-1594) so that the safest square is the one farthest from any enemy. Monsters.c:3095 takes the neighbouring square with the lowest resulting value, and 3102-3104 abandons the flight attempt when no neighbour is strictly lower than the ally's current square. BR66892 describes the blink-or-step and the no-safer-square guard, but no rule in rules/* states that an ally's danger is measured from the nearest hostile rather than from you, or which squares are excluded from that measurement. ID BR32744 is unique across rules/ and test/.

----
BR35671
"Once a fleeing monster can no longer see you, it keeps the last safety picture it took instead of recomputing it from your new position, so it does not follow your movements through walls or darkness. It takes a fresh picture only while you are in its field of view, or directly beside it while you are invisible."
Citation: [GlobalsBase.c:35](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L35), [Monsters.c:2348-2379](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2348-L2379), [Monsters.c:3492](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L3492), [Rogue.h:2268](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L2268)
Note: Monsters.c:2348-2358 considers a fleeing monster aware of you when your square is in its field of view, or when you are invisible and within one square. When aware, Monsters.c:2361-2369 discards any stored copy and returns the current shared safety map; when unaware, 2370-2378 allocates a copy once and reuses it forever, so later moves of yours do not change its escape route. Rogue.h:2268 documents this per-monster stored map as the anti-omniscience measure. BR65017 covers the resulting retreat step and BR48497 covers a thief leader being kept from fleeing before it has seen you, but no rule in rules/* describes the stale-picture behavior itself. ID BR35671 is unique across rules/ and test/.

----
BR20866
"Every square on the dungeon grid has eight neighbours — the four straight directions and the four diagonals — and creatures can move, strike, and be caught by area effects in any of those directions."
Citation: [GlobalsBase.c:38-39](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L38-L39), [Movement.c:1489-1532](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L1489-L1532)
Note: GlobalsBase.c:38 defines the eight neighbour offsets (north, south, east, west and the four diagonals) and GlobalsBase.c:39 lists the same eight in circular order for scans around a square. Movement.c:1489-1532 walks all eight directions when choosing a step, with the flag at 1500-1502 changing only the order in which diagonals and straight directions are examined. Existing rules cover specific consequences — BR29427 and BR10367 the diagonal corner-blocking rule, BR33297 an attack that sweeps all eight directions, and BR32647 gas spreading to eight neighbours — but no rule in rules/* states the eight-way adjacency itself, which the declarations at GlobalsBase.c:38-39 define. ID BR20866 is unique across rules/ and test/.

----
BR31036
"Every dungeon square is given its own random tint, so otherwise identical terrain is shaded slightly differently from one square to the next. With colour effects hidden, that per-square variation is replaced by one neutral set of values, so identical terrain looks uniform."
Citation: [GlobalsBase.c:34](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L34), [IO.c:928-953](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L928-L953), [RogueMain.c:296-303](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L296-L303)
Note: GlobalsBase.c:34 declares the per-square random colour values. RogueMain.c:296-303 pre-shuffles them once per game to values from 0 to 1000 using the cosmetic random stream. IO.c:928-953 (bakeTerrainColors) adds each square's stored values to the terrain's random colour components, and switches to a uniform neutral set when colour effects are hidden. Existing rules describe individual terrain hues and dancing or flickering colours, but none describes this per-square variation or the neutral shading used when colour effects are hidden. ID BR31036 is unique across rules/ and test/.

----
BR24387
"A discovered square you can no longer see is drawn from memory — the terrain, item, and gas last shown there — under a dim blue-grey tint, and keeps that remembered picture until you see it again. Stairs, the dungeon exit, and locked iron doors are exempt from the foreground fade, so their symbols stay bright on the map."
Citation: [GlobalsBase.c:31](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L31), [IO.c:1126-1136](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L1126-L1136), [IO.c:1333-1344](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L1333-L1344), [IO.c:1394-1411](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L1394-L1411), [Globals.c:331-336](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L331-L336)
Note: GlobalsBase.c:31 declares the permanent map cells. When a square stops being visible, Movement.c:2276-2282 records its terrain and flags, and IO.c:1333-1344 stores the character and colours last drawn there. On later redraws IO.c:1126-1136 restores that remembered appearance for squares that are discovered but out of sight, and IO.c:1394-1411 applies the blue-grey memory tint, skipping the foreground fade for tiles flagged to keep a bright foreground — stairs, the dungeon exit and portal, and locked iron doors (Globals.c:331-336). BR14627 and BR24943 cover the visibility flags and permanent discovery, but no rule in rules/* describes the remembered appearance or the memory tint. ID BR24387 is unique across rules/ and test/.

----
BR21051
"Looking at a discovered square you cannot currently see reports what you remember there — the item or terrain feature you last saw — while a square known only through magic mapping instead says you expect that terrain to be there."
Citation: [GlobalsBase.c:31](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/GlobalsBase.c#L31), [Movement.c:246-265](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L246-L265)
Note: Movement.c:246-265 is the remembered-square branch of the look description: for a discovered but unseen square it names the remembered item (or the remembered terrain when no item was there) and prints "you remember seeing <object> here."; for a square known only by magic mapping it prints "you expect <terrain> to be here." The remembered item category, kind, quantity, and origin depth are stored at IO.c:1288-1292 and the remembered terrain at Movement.c:2229-2233, all in the permanent map cell declared at GlobalsBase.c:31. BR12275 covers examining visible monsters and items and BR14627/BR24943 cover map memory, but no rule describes these remembered-square messages. ID BR21051 is unique across rules/ and test/.

----
BR18471
"A range drawn with a clumping factor of one or less is rolled as a plain uniform draw, so every value from the lower bound to the upper bound is equally likely; only a factor above one clumps results toward the middle."
Citation: [Math.c:44-46](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Math.c#L44-L46)
Note: Math.c:44-46 is the no-clump branch of the shared range draw: when the factor is one or less it returns an ordinary uniform draw over the inclusive bounds (Math.c:45), and only a factor above one continues to the multi-roll sum at Math.c:48-58. Player damage ranges use a clumping factor of one (Items.c:7798), so this branch is what the player rolls for those. BR33997 describes only factors greater than one, and BR22852 describes the underlying uniform draw from Math.c:160-170 without stating that a clumped range with a factor of one or less degrades to it, so no rule in rules/* describes this fallback. ID BR18471 is unique across rules/ and test/.

----
BR34530
"A staff of blinking's maximum targeting distance is two spaces plus twice its net enchantment, and that cap applies once the staff is identified or its maximum charges are known; until then the targeting line is not range-limited."
Citation: [Items.c:6577-6583](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6577-L6583), [Items.c:6588](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6588), [Items.c:5681-5684](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L5681-L5684), [PowerTables.c:52](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L52)
Note: Items.c:6577-6583 sets maxDistance to staffBlinkDistance(netEnchant(theItem)) for a staff of blinking that is ITEM_IDENTIFIED or has ITEM_MAX_CHARGES_KNOWN, and to -1 otherwise; PowerTables.c:52 defines staffBlinkDistance as 2 + 2 x enchant. The value is passed to chooseTarget at Items.c:6588, and chooseTarget truncates the highlighted trajectory to that distance (Items.c:5681-5684). BR19585 states only that an identified staff of blinking has a fixed maximum distance; it omits the enchantment scaling, the maximum-charges-known condition, and the unlimited default, so those observable aspects were uncited. ID BR34530 is unique across rules/ and test/.

----
BR18753
"If you confirm a zap from a staff of blinking whose range you know and the square you would land on is lava you have discovered, you refuse to blink with "that would be certain death!" and the zap is canceled without consuming a charge; levitating or being immune to fire skips this refusal."
Citation: [Items.c:6589-6595](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6589-L6595), [Items.c:6495-6505](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6495-L6505), [Items.c:6530-6533](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6530-L6533), [Items.c:6490-6493](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6490-L6493), [Items.c:6641-6643](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6641-L6643)
Note: playerCancelsBlinking is called at Items.c:6589-6595 only when a target was confirmed, the staff kind is identified (boltKnown, Items.c:6585), and the bolt effect is BE_BLINKING. With a known range (maxDistance > 0) it computes the impact square via getImpactLoc and, when that discovered square is instantly lethal lava, sets certainDeath and prints "that would be certain death!" (Items.c:6495-6505, 6530-6533), so confirmedTarget becomes false. Levitation or fire immunity returns false early (Items.c:6490-6493). The canceled path returns before any charge is decremented (Items.c:6641-6643 versus 6651-6656). Rules BR60519 and BR45966 cover refusing to step into lava, but no rule in rules/* covers refusing a blinking zap that would land in known lava. ID BR18753 is unique across rules/ and test/.

----
BR14508
"If the bolt is a known blink but your staff's range is not yet known and the possible blink path crosses discovered lava before a guaranteed safe landing spot, the game asks "Blink across lava with unknown range?" and the zap is canceled if you decline."
Citation: [Items.c:6589-6595](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6589-L6595), [Items.c:6506-6528](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6506-L6528), [Items.c:6534-6537](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6534-L6537), [Items.c:6641-6643](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L6641-L6643)
Note: When the staff's range is unknown (maxDistance <= 0) playerCancelsBlinking walks the line toward the target, marking possibleDeath on each discovered instantly lethal lava square and clearing certainDeath once a discovered safe square is at least staffBlinkDistance(2) - 1 cells along (Items.c:6506-6528). A possible-but-not-certain lava crossing then prompts "Blink across lava with unknown range?" and declining returns true (Items.c:6534-6537), which clears confirmedTarget at Items.c:6589-6595 so the zap returns before spending a charge (Items.c:6641-6643). Levitation or fire immunity skips the check (Items.c:6490-6493). This unknown-range confirmation is described by no rule in rules/*. ID BR14508 is unique across rules/ and test/.

----
BR36312
"A positive total enchantment from your worn rings of light multiplies your own light radius, while a total of zero or below instead divides it, halving it at zero, thirding it at minus one, quartering it at minus two, and so on, so cursed rings of light sharply shrink your light."
Citation: [Light.c:123-130](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L123-L130), [Items.c:7864](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7864), [Items.c:7881-7883](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7881-L7883), [Items.c:7897-7898](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7897-L7898)
Note: Light.c:123-130 sets the miner's light radius from the 100-times base radius: when the light multiplier is negative it divides by one more than its magnitude (Light.c:125-126), otherwise it multiplies by the multiplier and applies a small floor (Light.c:128-129). Items.c:7864 starts the multiplier at 1, Items.c:7881-7883 adds each equipped ring of light's effective enchantment, and Items.c:7897-7898 pushes a total of zero or less one step lower, so a net zero divides the radius by 2, net minus one by 3, net minus two by 4, and so on. BR17370 covers only that a ring of light increases the radius by its enchantment, and BR23156 covers growth from light-boosting effects plus shrinkage from darkness and water; neither describes this reciprocal scaling of the radius for non-positive totals. ID BR36312 is unique across rules/ and test/.

----
BR40052
"When the game's master seed fits in its lower 32 bits, each level's seed is drawn from the backward-compatible small range — a value from 0 to 9999 plus 10,000 times another value from 0 to 9999 — so level seeds stay within the old 32-bit seed scheme. A level seed of zero is never kept; it is replaced by the level's depth plus one."
Citation: [RogueMain.c:263-269](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L263-L269)
Note: RogueMain.c:264-266 draws the backward-compatible seed when the master seed's upper 32 bits are zero: two rand_range(0, 9999) calls, the second scaled by 10,000, give a small 32-bit value. RogueMain.c:267-269 then rejects a zero seed by setting it to the level index plus one. BR37990 describes how a seed larger than 32 bits changes the dungeons; no rule in rules/* describes this backward-compatible draw or the zero-seed replacement. ID BR40052 is unique across all rule files.

----
BR21780
"A newly created level starts with empty monster and dormant-monster lists, no items, no scent map, and its visited flag cleared; the player's exit position for the level is reset to (0, 0)."
Citation: [RogueMain.c:270-275](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L270-L275)
Note: RogueMain.c:270-275 initializes each level's creature lists as empty lists (createCreatureList), clears the item list and scent map to NULL, sets visited to false, and resets playerExitedVia to (0, 0). This is the starting state before the level is populated and explored. No rule in rules/* describes this initial empty state; BR16675 and BR92702 concern monsters that are later placed on levels. ID BR21780 is unique across all rule files.

----
BR24594
"The down stairs are placed at a random position within the map's interior, and the placement is repeated until the down stairs are at least one third of the map's width away from the up stairs."
Citation: [RogueMain.c:276-279](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L276-L279)
Note: RogueMain.c:276-279 is the do-while that picks the down-stairs location: x is drawn from 1 to DCOLS-2 and y from 1 to DROWS-2, and the loop repeats while the distance from the up-stairs location is less than DCOLS/3. BR35287 covers only that the next level's up stairs reuse this down-stairs location; no rule in rules/* describes the random placement or the one-third separation constraint. ID BR24594 is unique across all rule files.

----
BR22809
"When a distance calculation starts from squares that already carry a distance, those squares are taken up in order of increasing distance, so the calculation spreads outward from the nearest starting square first and every square ends up with its shortest distance to the closest starting square."
Citation: [Dijkstra.c:166-184](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Dijkstra.c#L166-L184)
Note: Dijkstra.c:166-184 is the seeding half of the batch input used by the distance-map scan: for each square whose entry cost is positive and whose distance is below the scan's maximum, Dijkstra.c:169-177 walk the distance-ordered waiting list and splice the square in at its sorted position (restarting from the front when the cached insertion point has already passed it, 166-167), and 179 remembers it as the next insertion point. Because the scan is fed an input distance map (127-143, called at 205), several pre-marked sources enter the waiting list at once and are worked nearest-first, which gives the scent, shore, waypoint, safety, and player-pathing maps their multi-source nearest distances. BR14684 covers the seeding values at 127-132, and BR14654 covers the later relaxation and shortest-route behavior of the scan; no rule in rules/* cites 166-184 or describes this sorted seeding. ID BR22809 is unique across all rule files.

----
BR42661
"Squares that cannot be entered, and squares already sitting at the maximum distance the scan tracks, are never added to the waiting order, so no distance spreads outward from an impassable or unreachable square."
Citation: [Dijkstra.c:158-187](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Dijkstra.c#L158-L187)
Note: Inside the batch input's seeding loop only squares with a positive entry cost and a distance below the scan's maximum are linked into the waiting order (Dijkstra.c:158-159); a square at or beyond that maximum is dropped at 180-183, and a square whose entry cost is not positive — an obstruction or a square the traveller must avoid — is dropped at 184-187 (the non-positive values are -1 for forbidden and -2 for obstruction, Rogue.h:2765-2766), so neither kind becomes a source that spreads distances. BR37348 covers only that an obstructed square cannot be entered while expanding, and BR43009/BR33256 cover two specific kinds of avoided square; no rule in rules/* describes these squares being kept out of the scan's starting set. ID BR42661 is unique across all rule files.

----
BR33355
"When you first become stuck in entangling terrain you see "you are stuck fast in <terrain>!", and when a monster you can directly see first becomes stuck you see "<monster> is stuck fast in <terrain>!"; neither message appears while automation such as auto-exploration is running."
Citation: [Time.c:292-308](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L292-L308)
Note: Time.c:296-298 builds the player's message from the description of the entangling terrain layer, and Time.c:302-305 does the same for a directly visible monster; both message calls sit inside !rogue.automationActive guards (Time.c:294, Time.c:301), so auto-exploration and other automation do not interrupt play with them. No rule in rules/* quotes the "stuck fast" wording or describes its suppression during automation; BR96828 and BR91575 cover only the duration and exemptions of becoming stuck. ID BR33355 is unique across all rule files.

----
BR12820
"A creature that is already stuck in entangling terrain is not stuck again, so its remaining 3-7 turns of entanglement are not refreshed by standing in further entangling terrain."
Citation: [Time.c:288-292](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L288-L292)
Note: Time.c:288-290 requires that the creature is not already stuck before Time.c:292 rolls a fresh random 3-7 duration; without this guard the duration would reset on every application of the tile effect while the creature remained on the entangling square. BR96828/BR91575 describe the initial catch and BR96404/BR27577 the struggle, but no rule in rules/* states that an already-stuck creature keeps its existing countdown. ID BR12820 is unique across all rule files.

----
BR27525
"When a distance map is computed, each square is assigned a cost to enter it and a route's distance is the sum of those entry costs, so planning prefers cheaper squares and will take a longer way around an expensive one."
Citation: [Dijkstra.c:149-156](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Dijkstra.c#L149-L156), [Dijkstra.c:61-62](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Dijkstra.c#L61-L62)
Note: Dijkstra.c:149-156 chooses the per-square cost (the supplied cost map's value at 152-153, stored at 156; the border and terrain-only branches pick impassable negative values at 147-151), and Dijkstra.c:61-62 adds that cost on every relaxation, so distances accumulate entry costs and routes detour around expensive squares. BR42661 covers only which squares enter the waiting order and BR54368 only the shore-distance map's own costs; no rule in rules/* states that per-square entry costs are additive route weights. ID BR27525 is unique across all rule files.

----
BR26960
"A placed wizard-created monster is asked about as "Make the <name> your ally?" unless it is inanimate or invulnerable, although a phoenix egg or a phylactery is still asked about despite being invulnerable. A monster that is never asked is simply created and announced as "<name> created.""
Citation: [Wizard.c:426-427](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L426-L427), [Wizard.c:434-437](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L434-L437)
Note: Wizard.c:426 negates the inanimate and invulnerable flags to decide whether the ally question is asked, and its two extra conditions admit a phoenix egg and a phylactery even though both are invulnerable; Wizard.c:427 builds the "Make the <name> your ally?" wording. For every monster excluded by the test, Wizard.c:435 builds "<name> created." with no ally offer. BR99923 says only that the player is prompted after placement, so no rule in rules/* states which created monsters are never offered as allies or what those monsters report. ID BR26960 is unique across rules/ and test/.

----
BR33614
"Accepting the ally offer makes the created monster your ally and announces "Allied <name> created.", while declining leaves it as an ordinary monster and announces "<name> created.""
Citation: [Wizard.c:428-433](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L428-L433), [Wizard.c:437](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L437)
Note: Wizard.c:428 asks for confirmation; on acceptance Wizard.c:429 turns the monster into an ally and Wizard.c:430 builds "Allied <name> created.", otherwise Wizard.c:432 builds "<name> created.", and Wizard.c:437 displays whichever message was built. BR99923 covers only the prompt itself and no rule in rules/* states the two outcomes or their messages. ID BR33614 is unique across rules/ and test/.

----
BR15599
"If no location is chosen for a wizard-created monster, it is killed and removed from the level, leaving nothing behind."
Citation: [Wizard.c:438-442](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L438-L442)
Note: Wizard.c:438-442 is the branch taken when the location chooser is cancelled: it clears pending messages, kills the just-created monster outright, and removes dead monsters, so the creation is undone. BR24559 covers a chosen but invalid location and no rule in rules/* describes this cancellation path. ID BR15599 is unique across rules/ and test/.

----
BR39133
"When the seed catalog is written in CSV, each item is broken into separate fields for its category, its kind, and its rune; an item that can be enchanted — armor, a charm, a ring, a staff, a wand, or a weapon — also gets an enchantment field, which for a wand is its remaining charge count and otherwise its enchantment level."
Citation: [SeedCatalog.c:57-67](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L57-L67)
Note: SeedCatalog.c:57-67 is the CSV branch of the item-reporting routine: it fills the category from the item category names (58), the kind name (59), and the runic name (60), then fills the enchantment field only for the enchantable categories armor, charm, ring, staff, wand, and weapon (61), using the wand's charges (62-63) or the item's enchantment (64-66). Existing seed-catalog rules BR23144, BR20949, and BR39812 cover floor gold, floor-item listing/amulet omission, and the vault-opening key number, but none describes how an individual item is split into category/kind/rune/enchantment fields or that wands are reported by charges. ID BR39133 is unique across rules/ and test/NewRules.txt.

----
BR13158
"When the seed catalog is written in plain text, each item is listed by its ordinary in-game name rather than by separate category, kind, and rune fields."
Citation: [SeedCatalog.c:68-69](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L68-L69)
Note: SeedCatalog.c:68-69 is the non-CSV branch of the item-reporting routine: it builds the item's display text with the game's normal item-naming routine, so the plain-text catalog shows the same name a player would see. BR20949 covers that each non-gold floor item is listed and BR23144 covers gold entries, but no rule in rules/* states that the plain-text listing uses the in-game item name. ID BR13158 is unique across rules/ and test/NewRules.txt.

----
BR37653
"When several terrain features share a square, the one-line description and flavor text shown for it describe the most visually prominent one — the feature drawn in front of the others — so a gas cloud lying over a floor is described as the cloud rather than the floor."
Citation: [Movement.c:63-77](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L63-L77), [Movement.c:101-109](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L101-L109)
Note: Movement.c:63-77 scans the terrain present on a square and keeps the one drawn in front of the rest (the lowest draw priority, with ties going to the first found); the gas layer is skipped only for callers that ask for it. The flavor text and description of a square are then read from that frontmost feature (Movement.c:101-109), and the description of your own square uses them both when standing and in the "you are hovering above ..." form while levitating (Movement.c:161-166). This same frontmost feature is also what map memory records (Movement.c:2233). BR98623 covers only which text is used while levitating and BR24387 the remembered appearance, but no rule in rules/* states that a square's description comes from its frontmost terrain feature. ID BR37653 is unique across rules/ and test/.

----
BR39120
"When a run comes to an end, the line beneath the map is refreshed so that it describes your surroundings where you stopped."
Citation: [Movement.c:60](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L60), [Time.c:53-69](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L53-L69)
Note: Once the run's loop finishes, Movement.c:60 refreshes the flavor line; Time.c:53-69 reprints, when the game has been disturbed and has not ended, the cool-air message for respiration armor on respiration-immune terrain, the levitating location description, or the flavor text of the square you stand on. BR16305 covers the passability change that can stop such a run and BR34710 the refresh after the help screen, but no rule in rules/* describes the refresh performed when a run ends. ID BR39120 is unique across rules/ and test/.

----
BR18640
"Wearing a ring of regeneration sets the time to recover all of your health to 300 × 0.75^enchantment + 2 turns: a ring at +0 takes 302 turns, each additional enchantment level multiplies the 300-turn share by 0.75, and the total bottoms out at about 2 turns however high the enchantment. Examining the ring reports the resulting turn count and contrasts it with the ordinary 300 turns."
Citation: [PowerTables.c:125-135](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/PowerTables.c#L125-L135), [Items.c:2610-2615](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L2610-L2615), [Rogue.h:1122](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Rogue.h#L1122)
Note: PowerTables.c:133 clamps the ring's enchantment into the 0.75^b table (b from -10 to +50) and PowerTables.c:134 computes 300 × 0.75^b turns plus a flat 2 turns; the comment at PowerTables.c:131-132 notes this bottoms out near two turns even at a very high bonus. Items.c:2610-2615 prints that total in the ring's examine text and contrasts it with the ordinary 300 turns defined at Rogue.h:1122. BR19386 only says a regeneration ring "increases your regeneration by its enchantment", and BR20189 covers the unrelated damage fraction table at PowerTables.c:138-159, so no rule in rules/* gives the regeneration time formula, its 25%-per-level scaling, or the 2-turn floor. ID BR18640 is unique across rules/ and test/.

----
BR28703
"When the game searches for the nearest available place from a reference square, only squares at a positive walking distance are eligible; unreachable squares and the reference square itself are never chosen, and if no square has a positive distance the search reports that no place was found."
Citation: [Grid.c:210-220](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L210-L220), [Grid.c:249-254](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L249-L254)
Note: leastPositiveValueInGrid (Grid.c:210-220) starts its running minimum at zero and records only grid values greater than zero, so a zero distance (the measuring point itself) and the negative values used for unreachable tiles are excluded; randomLeastPositiveLocationInGrid (Grid.c:249-254) turns a result of zero into the invalid-position sentinel, which getQualifyingPathLocNear (Grid.c:345-358) answers with its pathing-agnostic fallback. BR68830 states only that things are placed "at the nearest reachable valid tile", and BR37307/BR23280 cover how ties among the nearest tiles are broken, but none states that only positive distances qualify or that an all-non-positive map yields no nearest place. ID BR28703 is unique across rules/ and test/.

----
BR36984
"When the game needs a random spot from a set of qualifying squares — one door site per compass direction, a horde's distant spawn point, or where a teleporting monster lands — every qualifying square is equally likely to be chosen; if no square qualifies, the request reports that no spot is available instead of choosing one."
Citation: [Grid.c:222-245](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L222-L245)
Note: randomLocationInGrid counts the tiles matching the requested mark via validLocationCount (Grid.c:225, body at 197-208), returns (-1, -1) when that count is zero (Grid.c:228-231), and otherwise draws a uniform index from 0 through count - 1 (Grid.c:232) and scans the map to the tile at that index (Grid.c:233-243), so no tile is favored by its position. Call sites include one door candidate per compass direction (Architect.c:2230-2233), distant horde summons (Monsters.c:1017-1023), monster spawn points (Monsters.c:1088-1103) and teleport destinations (Monsters.c:1179). Rule 08-dungeon-generation.md:69 says only that one door site is chosen per direction, and no rule in rules/* describes this uniform random choice or the no-candidate sentinel. ID BR36984 is unique across rules/ and test/.

----
BR28624
"The first time you are poisoned while not already poisoned, the message "scalding poison fills your veins" is added to the message log."
Citation: [Combat.c:1618-1620](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1618-L1620)
Note: Combat.c:1618-1620 is the guard in addPoison that fires only when the victim is the player and the player's poison counter is zero, emitting the combat message "scalding poison fills your veins" through the bad-message color. Later poison increments are silent. BR64646 and BR75638 cover poison concentration, duration and per-turn damage, but no rule in rules/* describes this first-poisoning message. ID BR28624 is unique across rules/ and test/.

----
BR40948
"Each time poison takes effect on a visible creature, its figure flashes at full strength in the poison color, a flickering green."
Citation: [Combat.c:1631-1632](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1631-L1632), [Globals.c:69](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L69)
Note: Combat.c:1631-1632 flashes any creature the player can see with the flash strength set to 100 whenever addPoison applies a positive poison duration; canSeeMonster treats the player as always visible, so the player's own figure flashes too. Globals.c:69 defines that poison color as black plus randomly varying green and a little blue. BR11437 deliberately covers only the green flash of recurring poison damage and notes that the initial poisoning uses another color, so no rule in rules/* describes this initial poison flash. ID BR40948 is unique across rules/ and test/.

----
BR40773
"Adding poison that would add no turns of duration has no effect: the victim is not messaged, flashed, or made any more poisoned, so poisonous lichen stops lengthening poison once 5 or more turns remain."
Citation: [Combat.c:1617](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1617), [Time.c:452-453](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L452-L453)
Note: Combat.c:1617 is the guard that wraps the entire addPoison body, so an application with a non-positive duration increment changes nothing: no first-poison message, no flash, and no increase in concentration or duration. Time.c:452-453 shows the only such application, lichen's max(0, 5 - remaining poison) duration, which makes the poison from standing on lichen saturate at 5 turns. BR64646 and BR81488 cover concentration and the lichen concentration cap, but no rule in rules/* describes this no-op guard or lichen's 5-turn duration cap. ID BR40773 is unique across rules/ and test/.

----
BR22521
"When playback diverges from the original game, the apology box explains that the recording has diverged from the originally recorded game, that a modified version of Brogue or a bug could be the cause, and that on a different computer the recording might still succeed on the original computer."
Citation: [Recordings.c:299-303](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L299-L303)
Note: Recordings.c:299-303 defines OOS_APOLOGY, the exact text playbackPanic prints into the apology box (Recordings.c:305-338); BR73305 describes only that an apology box appears, without its contents. ID BR22521 is unique across rules/ and test/.

----
BR27784
"When a rebuilt vault ends up with a room sealed off from the rest of the level, the game tunnels that orphaned room back to the rest of the level along the shortest orthogonal route, carving every rock tile on the way into floor so the room becomes reachable again."
Citation: [Architect.c:804-822](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L804-L822)
Note: Architect.c:804 runs a distance scan across the rebuilt interior in which every interior tile costs 1 to cross and interior tiles that are already open start at distance 0 (791-797), while tiles outside the interior are impassable (798-800). Starting at the orphaned tile (806-807), the walk at 808-821 follows any cardinal neighbour with a strictly lower distance and turns the current tile into floor (816) until it reaches a distance-0 tile, so the carved tunnel is the shortest orthogonal route from the orphaned room to existing open space. Each orphaned room gets its own connection (the per-orphan loop at 777-830). BR68008 only says a rebuilt vault's interior is "reconnecting any rooms that were orphaned in the process" (Architect.c:734-854, a whole-function citation) and states neither the shortest-route guarantee nor that the route becomes floor; no other rule in rules/* mentions orphaned rooms or this connection work. ID BR27784 is unique across rules/ and test/.

----
BR12331
"An orphaned room's connecting tunnel is cut only through solid rock that lies inside the vault's rebuilt interior, and it ends as soon as it reaches a tile there that is already open, so it never breaks through walls of the surrounding level or opens new ground outside the vault."
Citation: [Architect.c:790-822](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L790-L822)
Note: Architect.c:790-800 marks only interior tiles as passable to the scan (uniform cost 1, distance 0 for open ones) and gives every non-interior tile the obstruction cost, so the downhill walk at 808-821 can never step outside the interior; the walk also terminates at the first interior tile whose distance is 0, i.e. already-open floor or the vault's origin, and only interior tiles are set to floor at 816. This confines the reconnection to the rebuilt vault area: no outer-level wall is carved and no tunnel exits it. BR27784 describes that a shortest tunnel reconnects each orphaned room, and BR68008 notes that rebuilding a vault reconnects orphans, but no rule in rules/* states the carve area restriction. ID BR12331 is unique across rules/ and test/.

----
BR28969
"In wizard mode's list dialogs — the item-or-monster menu and the lists of item categories, item kinds, runics, vorpal enemies, monster ranges, monsters, and mutations — each choice is assigned the next letter of the alphabet as its shortcut, usable in either lowercase or uppercase, and with keyboard labels on the choice is displayed with that lowercase shortcut letter, a closing parenthesis, and a space before its name."
Citation: [Wizard.c:47-64](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L47-L64)
Note: Wizard.c:55-56 give the i-th choice a lowercase and uppercase letter hotkey starting at "a"/"A", and Wizard.c:59-61 prefix the displayed text with the lowercase hotkey and ") " when KEYBOARD_LABELS is true (defined true at Rogue.h:79, so the labels are always shown in the released game). This shared list chooser is used for the item-or-monster menu at Wizard.c:516, categories at Wizard.c:458, item kinds at Wizard.c:193, runics at Wizard.c:152, vorpal enemies at Wizard.c:101, mutations at Wizard.c:298, and the monster range and monster lists at Wizard.c:343 and Wizard.c:358. BR89701 covers wizard item creation only as a category-then-kind flow, BR91827 covers generic button hotkeys, and BR77834 covers file-picker letter hotkeys; no rule in rules/* describes these wizard list shortcuts or their labels. ID BR28969 is unique across rules/ and test/.

----
BR18356
"Every choice in a wizard mode list dialog occupies its own row starting in the same column one line below the title, and the choices are drawn in the dialog's own background color without a gradient, so they read as plain text on the panel rather than as raised buttons; a mouse click selects a choice only when it is on that choice's own row."
Citation: [Wizard.c:52-58](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L52-L58), [Buttons.c:224-233](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L224-L233)
Note: Wizard.c:57 places every choice at the dialog's left column and Wizard.c:58 puts the i-th choice one row below the title row plus i, producing a single-column list. Wizard.c:53 clears the gradient and wide-click flags that initializeButton sets (B_GRADIENT at Buttons.c:135) and Wizard.c:54 changes the row color from the normal button color to interfaceBoxColor, the same color as the panel drawn at Wizard.c:78; drawButton applies no gradient when that flag is absent (Buttons.c:91). Buttons.c:232 normally accepts a click one row above or below a wide-click button, so clearing that flag at Wizard.c:53 confines each choice's click target to its own row. BR91827 covers only that buttons are clicked or hotkeyed, and no rule in rules/* describes this list layout or flat appearance. ID BR18356 is unique across rules/ and test/.

----
BR36062
"The dialog around a wizard mode list is sized to its contents, growing wider for the longest title or shortcut-labeled choice and taller for each additional choice."
Citation: [Wizard.c:50-70](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L50-L70)
Note: Wizard.c:50 starts the measured width at the title's length, Wizard.c:62-64 lengthen it to any shortcut-labeled choice that is longer, and Wizard.c:69-70 turn that maximum into a width one column wider and a height of the choice count plus two, with the shaded panel drawn one cell beyond those bounds at Wizard.c:78. The same measured width is passed to the input loop at Wizard.c:83, so clicks outside it cancel. No rule in rules/* describes the size of these wizard list dialogs. ID BR36062 is unique across rules/ and test/.

----
BR26972
"When the game prepares a set of squares for routing or placement, every square whose terrain has any of the selected blocking properties, or whose occupancy, visibility, or machine state has any of the selected disqualifying conditions, is marked off-limits; all other squares keep whatever value they already had."
Citation: [Grid.c:159-170](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L159-L170)
Note: Grid.c:159-170 is the shared filter that scans every map square and writes the supplied value only when the square's terrain matches any of the given terrain flags or its map flags match the given map flags, leaving non-matching squares untouched. Its callers use it to block pathing obstacles and forbidden targets inside the distance-map builder (Grid.c:315-317 and 327, reached through Grid.c:287-360), and to reject harmful terrain plus occupied, stair, visible, or machine squares when choosing summon, periodic-spawn, and teleport destinations (Monsters.c:1006, 1094, 1101, 1168-1170). Existing rules describe those caller-specific outcomes (BR39283 distant summoning, BR82926 periodic spawns, BR53348 teleport landing, BR68830 distance-map uses), but no rule in rules/* describes the common flag-based square filtering itself. ID BR26972 is unique across rules/ and test/.

----
BR30488
"Watching a recording replays the run automatically, carrying out the recorded actions one turn at a time at the current playback speed, and keeps going until the recorded game ends or playback is stopped."
Citation: [MainMenu.c:1240-1258](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1240-L1258)
Note: MainMenu.c:1240-1258 is the playback loop: it repeats while the recorded game has not ended and playback is still active (1240), marks the game as between turns so the playback delay is honored (1253), obtains the next recorded event (1254), and executes that event immediately (1257). Existing rules cover the playback controls — speed (BR42285, BR62572), pause (BR82494), and single-turn stepping (BR11882, BR80681) — but none states that viewing a recording automatically advances through the run turn by turn until it ends. ID BR30488 is unique across rules/ and test/.

----
BR22582
"During playback, randomness that only affects appearance is kept separate from the randomness that determines the game's outcome, so cosmetic variation such as dancing terrain colors cannot make the recording diverge from the game that was recorded."
Citation: [MainMenu.c:1252-1257](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1252-L1257)
Note: MainMenu.c:1252 switches to cosmetic randomness around the wait for the next recorded event, and MainMenu.c:1255 restores gameplay randomness before that event is executed at 1257, so random draws consumed by visual-only effects while waiting between turns cannot alter the replayed game. Existing divergence rules (BR73305, BR22521) describe only the out-of-sync apology and its text, not this isolation of cosmetic randomness. ID BR22582 is unique across rules/ and test/.

----
BR39983
"When a recording is finished — whether the recorded run ended or the player stopped playback — the game discards the replayed dungeon and everything on it and clears its playback status, returning the player to the title screen."
Citation: [MainMenu.c:1258-1265](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1258-L1265)
Note: MainMenu.c:1260 frees every level of the replayed game along with its creatures, items, scent maps, and pathing maps; MainMenu.c:1264-1265 turn off playback mode and clear the out-of-sync state. The menu junction then finds no pending command and displays the title screen (MainMenu.c:1124-1132). Existing rules cover messages shown while playback ends or diverges (BR48693, BR73305) but none describes tearing down the replayed game and returning to the title screen. ID BR39983 is unique across rules/ and test/.

----
BR13270
"In server mode or when a recording is played non-interactively, the program quits as soon as playback finishes instead of returning to the title screen."
Citation: [MainMenu.c:1267-1269](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1267-L1269)
Note: MainMenu.c:1267-1269 sets the next command to quit when server mode or non-interactive playback is active, after the playback flags have been cleared, so the menu loop ends at MainMenu.c:1285 rather than showing the title screen again. No rule in rules/* mentions server mode, non-interactive playback, or the program terminating when such playback finishes. ID BR13270 is unique across rules/ and test/.

----
BR39094
"When a monster that was carried inside a slain monster is released where you can see it, the game announces the arrival as '<name> appears'."
Citation: [Combat.c:1725-1729](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1725-L1729)
Note: Combat.c:1725 tests canSeeMonster for the carried monster, and Combat.c:1727-1728 format "<name> appears" and post it as a combat message. BR28007 states only that the carried monster is released at the same spot, and BR70715 only that it reappears in the servant's place; neither describes the announcement. ID BR39094 is unique across rules/ and test/.

----
BR31838
"A monster released from a slain monster's body immediately suffers the effects of the terrain it appears on, so it can plunge into a chasm, be consumed instantly by lava, or trigger a pressure plate the moment it emerges."
Citation: [Combat.c:1731](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Combat.c#L1731) [Time.c:101-205](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L101-L205)
Note: Combat.c:1731 calls applyInstantTileEffectsToCreature(carriedMonster) immediately after placing the released monster, before any turn passes; Time.c:101-205 is that function, whose chasm branch marks the monster as falling (160-168) and whose lava branch kills it at once (173-205). Existing rules BR28007, BR70715 and BR72651 describe where a carried monster goes but not that it is exposed to the hazards of its arrival square. ID BR31838 is unique across rules/ and test/.

----
BR31624
"An attack bolt can still be fired at a creature embedded in a wall when that creature can be attacked through walls, such as a turret built into stone."
Citation: [Monsters.c:2632-2637](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2632-L2637)
Note: Monsters.c:2632-2637 rejects an attack bolt when the target's square obstructs passability only if the target also lacks the ability to be attacked through walls, so wall-mounted turrets remain valid targets. BR58883 states only that an attack bolt is not fired at an embedded creature, omitting this exception, and BR13361 describes the species trait for being attacked while embedded but not that a ranged attacker may still target it. ID BR31624 is unique across rules/ and test/.

----
BR39702
"In the Discoveries screen, an unidentified kind is marked as bad magic if it is a scroll of aggravate monsters or summon monsters, a potion of hallucination, incineration, descent, poison, paralysis, confusion, lichen, or darkness, or a wand or staff whose bolt is aimed at allies; other scrolls and potions and all charms are marked as good magic, while rings carry no good or bad mark because they could go either way."
Citation: [Items.c:7416-7465](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7416-L7465) [IO.c:4162-4169](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/IO.c#L4162-L4169)
Note: Items.c:7417-7464 (cited range starts at the function comment) assigns the good/bad/neutral suffix per item category and kind; IO.c:4162-4169 draws the good- or bad-magic symbol for unidentified kinds and draws nothing when the suffix is neutral. Existing BR27870 (07-player-interaction.md) and BR59433 (05-items.md) say an unidentified kind shows a good/bad marker and that detected inventory items show their polarity, but no rule in rules/* states which scrolls, potions, wands, staffs, rings, or charms count as good or bad. ID BR39702 is unique across rules/ and test/.

----
BR27364
"Drinking a potion you know to be of bad magic, or one whose detected magic marks it bad, first asks 'Really drink a potion of <name>?' or, if its kind is still unidentified, 'Really drink a cursed potion?', and the potion is only drunk if you confirm."
Citation: [Items.c:7254-7265](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L7254-L7265)
Note: Items.c:7254-7255 gates the prompt on the bad-magic suffix for the potion kind, combined with the kind being identified or the item's magic having been detected; Items.c:7257-7261 choose the named or "cursed potion" wording and Items.c:7262-7264 cancel the drink unless confirmed. BR50174 covers the matching confirmation for reading a scroll but no rule in rules/* covers the potion confirmation. ID BR27364 is unique across rules/ and test/.

----
BR31197
"Every menu or dialog has a fixed rectangular area on the screen, and releasing the mouse outside that area cancels the menu, while releasing anywhere inside it — even on blank space between its buttons — does not."
Citation: [Buttons.c:185-188](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L185-L188), [Buttons.c:246-247](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L246-L247)
Note: Buttons.c:185-188 records the menu's window origin and size when its button state is initialized, and Buttons.c:246-247 cancels on a mouse release only when that release lies outside the recorded rectangle and no button is currently held down. BR25868 states only that clicking outside the menu cancels; no rule in rules/* states that the cancel region is the menu's whole rectangular area, so a release on blank space inside the dialog, between its buttons, does not cancel. ID BR31197 is unique across rules/ and test/.

----
BR11417
"On each display update, a square you can see or sense is redrawn if any of its terrain has animated colours or if you are hallucinating, so dancing terrain keeps sparkling and hallucination makes the visible map shimmer; during automatic play this refresh happens only every fifth turn."
Citation: [Movement.c:2313-2330](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2313-L2330)
Note: Movement.c:2313-2324 selects a visible or sensed cell whose dungeon, liquid, surface, or gas layer has a dancing colour, or any cell the player sees while hallucinating; Movement.c:2326-2329 then clears the cell's stable memory and redraws it. The same check is throttled to every fifth turn while automation is active (Movement.c:2315), and hallucination also randomizes the colours of visible cells at IO.c:1428-1430. BR37436 describes only the potion of hallucination's message, not the recurring redraw that makes animated terrain sparkle or the whole visible map shimmer while hallucinating. ID BR11417 is unique across rules/ and test/NewRules.txt.

----
BR23647
"Your field of view is mirror-symmetric around you: the same pattern of walls and obstructions blocks sight identically in all eight directions, so the visible area is never skewed toward one direction."
Citation: [Movement.c:2343-2375](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2343-L2375), [Movement.c:2384-2391](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Movement.c#L2384-L2391)
Note: Movement.c:2343-2375 maps a coordinate in the first octant into each of the other seven octants by reflection, and Movement.c:2388-2391 runs the same sight scan once per octant. No rule in rules/* describes this symmetry of the field of view; existing sight rules cover the scan's range and obstacle branches (BR77568, BR71426, BR33509, BR89014) but not the octant mirroring. ID BR23647 is unique across rules/ and test/NewRules.txt.

----
BR18393
"A ring with no enchantment or a negative enchantment is automatically identified in full as soon as its kind is identified, so its exact zero or negative enchantment is revealed along with its type."
Citation: [Items.c:5960-5964](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L5960-L5964)
Note: Items.c:5960-5964, inside the routine that identifies an item's kind, sets the fully-identified flag for any ring whose enchantment is zero or less, so such a ring's enchantment is known the moment its type is. BR42824 covers a ring identifying itself after being worn for a time, and BR70142 covers identifying a kind generally, but no rule in rules/* describes this zero-or-negative-enchantment shortcut. ID BR18393 is unique across rules/.

----
BR41169
"A wand whose kind always comes with the same fixed number of charges is automatically identified in full as soon as its type is known, so its remaining charges are displayed."
Citation: [Items.c:5966-5970](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L5966-L5970), [Items.c:859-864](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L859-L864)
Note: Items.c:5966-5970 marks a wand fully identified when its kind's charge range has equal lower and upper bounds, so the kind's identification also reveals that wand's charges; Items.c:859-864 does the same when such a wand is picked up after its kind is already known. In the standard item set the wand of empowerment is the only wand with a fixed starting charge count (always one). BR40205 covers only that a wand starts with a random number of charges, and no rule in rules/* describes this fixed-charge identification. ID BR41169 is unique across rules/.

----
BR36269
"When a machine is activated, each creature guarding that machine that moves only when activated — provided it was spawned on the current dungeon level and is not already dying — immediately takes a turn, so guardians and totems spring into action the moment the machine triggers."
Citation: [Time.c:1071-1089](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L1071-L1089)
Note: Time.c:1071-1085 walks the level's creatures and collects only those whose machineHome matches the activated machine, whose spawnDepth equals the current depth, and whose species has MONST_GETS_TURN_ON_ACTIVATION (Rogue.h:2066, "moves only when activated"); Time.c:1086-1089 then calls monstersTurn for each collected creature that lacks MB_IS_DYING, bypassing the normal per-turn loop. BR35857 says activation makes "each creature of that machine" take a turn, which overstates the behavior: creatures of the machine that do not move only when activated, and activation monsters created on another depth, get no such turn. BR16717 covers only that such a creature never takes a turn on its own. No rule in rules/* states this activation-turn gating. ID BR36269 is unique across rules/ and test/NewRules.txt.

----
BR35064
"When the runic chooser appears for armor created in wizard mode, plate mail is offered only the bad runics vulnerability and immolation, while any other armor is offered every armor runic."
Citation: [Wizard.c:134-142](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L134-L142), [Globals.c:1601-1613](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1601-L1613)
Note: Wizard.c:135-140 is the PLATE_MAIL branch of the armor runic chooser: it builds buttons only for armorRunicNames beyond NUMBER_GOOD_ARMOR_ENCHANT_KINDS and sets runicOffset so the selection maps back to those entries; Rogue.h:863 makes that offset A_BURDEN, so the two offered names (Globals.c:1611-1612) are vulnerability and immolation. Wizard.c:141-146 offers every armor runic for any other armor kind. BR36714 covers the chooser only generically, BR24625 covers the analogous heavy-weapon restriction and explicitly the "any other weapon" branch (Wizard.c:126-130), BR24399 covers throwing weapons, and BR26686 covers which categories reach the chooser, so no rule in rules/* describes the armor-kind restriction. ID BR35064 is unique across rules/ and test/NewRules.txt.

----
BR40494
"Watching a recording never adds an entry to the run history, because the history is written only when the player actually dies, quits, or wins in a normal game — never when the same events are merely replayed."
Citation: [RogueMain.c:1232-1234](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1232-L1234), [RogueMain.c:1398-1400](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1398-L1400)
Note: The run-history write on both the game-over path (RogueMain.c:1232-1234) and the victory path (RogueMain.c:1398-1400) is guarded by both "not playing back" and "normal game mode". BR22377 states only the normal-mode condition, so no rule in rules/* says that a replayed death, quit, or victory leaves the run history untouched. ID BR40494 is unique across rules/ and test/.

----
BR32395
"If a recording reaches a victory while it is being fast-forwarded, the fast-forwarding stops and the current level is redrawn so the ending is shown from the final dungeon view."
Citation: [RogueMain.c:1253-1256](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1253-L1256)
Note: RogueMain.c:1253-1256, at the start of the victory sequence, clears the fast-forward flag and redraws the level when that flag was set. Fast-forwarding is enabled while seeking through a recording (Recordings.c:740) and while loading a saved game (Recordings.c:1316), so a seek that runs past the recorded ending arrives at victory in this state. BR15719 covers only the go-to-turn prompt that begins the fast-forward; no rule in rules/* describes its cancellation at victory. ID BR32395 is unique across rules/ and test/.

----
BR36290
"In the seed catalog's plain-text listing, a resurrection altar is reported with the number of the vault it belongs to, reading "A resurrection altar (vault N)"."
Citation: [SeedCatalog.c:230-232](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L230-L232)
Note: SeedCatalog.c:230-232 is the plain-text branch for a resurrection altar: it prints "A resurrection altar (vault %s)" using the vault number stored on the altar tile (SeedCatalog.c:225). Existing seed-catalog rules BR23144, BR20949, BR39812, BR39133, and BR13158 cover floor gold, floor items, vault-opening keys, CSV item fields, and in-game item names, but none describes altar entries, so plain-text resurrection-altar output is uncovered. ID BR36290 is unique across rules/ and test/NewRules.txt.

----
BR33780
"When the seed catalog is written as CSV, a resurrection altar is reported as an altar of the resurrection kind, with its vault number in the vault field and no enchantment or rune."
Citation: [SeedCatalog.c:226-232](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L226-L232)
Note: SeedCatalog.c:226-229 is the CSV branch for a resurrection altar: it emits category "altar", kind "resurrection altar", empty enchantment and rune fields, and the altar tile's vault number (supplied at SeedCatalog.c:225) in the vault field; SeedCatalog.c:230-232 is the corresponding plain-text branch. No rule in rules/* describes resurrection-altar catalog entries or their CSV fields. ID BR33780 is unique across rules/ and test/NewRules.txt.

----
BR19523
"Commutation altars are created in pairs belonging to the same vault, but the seed catalog lists each such pair only once, reporting a single commutation altar with that vault's number; in CSV it appears as an altar of the commutation kind, and in plain text it reads "A commutation altar (vault N)"."
Citation: [SeedCatalog.c:234-237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L234-L237), [SeedCatalog.c:240-249](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/SeedCatalog.c#L240-L249)
Note: SeedCatalog.c:234-237 scans for commutation altar tiles and records their vault numbers rather than printing each tile immediately (the comment states that commutation altars come in pairs and only one should be printed); SeedCatalog.c:240-249 then prints one entry per recorded vault, CSV as category "altar"/kind "commutation altar" (240-246) or plain text as "A commutation altar (vault %s)" (247-249). Because both altars of a pair share the vault number, the catalog collapses them to one line. No rule in rules/* describes commutation-altar catalog entries. ID BR19523 is unique across rules/ and test/NewRules.txt.

----
BR18460
"Distance between two squares is the larger of their horizontal and vertical separations, so a diagonal step counts as one square and a creature three squares east and three squares north is three squares away rather than six."
Citation: [Monsters.c:1578-1580](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1578-L1580)
Note: Monsters.c:1578-1580 returns max(abs(x1-x2), abs(y1-y2)) — the Chebyshev distance used by range checks such as the fear thresholds of 2 and 3 squares (BR17223, BR29646), the ally leash, blink range, and knockback distance. No rule in rules/* defines the distance measure: rules that speak of "squares away" leave it implicit, and none cites these lines. ID BR18460 is unique across rules/ and test/.

----
BR24107
"When a monster is woken, its next action is set a full standard turn away, so it waits that long before acting even if it is faster or slower than normal."
Citation: [Monsters.c:1587-1591](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1587-L1591)
Note: Monsters.c:1588-1590 alerts the woken monster unless it is already an ally, and 1591 sets its next-action countdown to exactly 100 ticks — one standard turn — regardless of its own movement speed, so a fast monster waits longer than one of its turns and a slow one waits less. BR53535 covers the turn a sleeping monster loses at the start of its own turn (Monsters.c:3370-3375), and BR34006/BR54165 cover the alerting, but no rule states this flat delay applied at the moment of waking. ID BR24107 is unique across rules/ and test/.

----
BR21625
"When a monster wakes, its sleeping or wandering teammates are roused too but are held back for at least a full standard turn before they can act."
Citation: [Monsters.c:1592-1597](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1592-L1597)
Note: Monsters.c:1594 restricts the rousing to the woken monster's teammates in normal mode, and 1595-1597 raise a sleeping or wandering teammate's next-action countdown to at least 100 ticks — one standard turn. BR54035 says such teammates wake and track or flee but not that they lose a turn, and the woken monster's own delay is BR24107. ID BR21625 is unique across rules/ and test/.

----
BR17859
"Every fresh distance, cost, or terrain calculation begins from a clean working area in which all squares hold the same starting value, so no result can inherit a value left over from an earlier calculation."
Citation: [Grid.c:56-64](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L56-L64)
Note: Grid.c:56-64 writes the supplied starting value to every square of a working grid and is the shared reset used before the game builds distance maps, cost maps, safety maps, scent maps, and blob generations (for example Grid.c:311-312, Architect.c:391/742/1251, IO.c:605/631, Monsters.c:883/1002/1092, and RogueMain.c:357-360). BR39926 covers only the wipe before each blob-generation attempt, BR16761 only the route scan's 30000 reset, and BR14684 only the scan input seeding, so no rule states that a new calculation always starts from a uniformly filled working area; in particular the map-cache clears that discard a previous turn's safety, ally-safety, or scent data rely on it. ID BR17859 is unique across rules/ and test/.

----
BR19624
"When cave or lake terrain is smoothed, every round judges each tile from a snapshot of the layout taken at the start of that round, so a tile that changes early in the round does not affect its neighbours' fate until the next round."
Citation: [Grid.c:46-54](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L46-L54), [Grid.c:367-379](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Grid.c#L367-L379)
Note: Grid.c:46-54 copies a whole grid value by value, and Grid.c:367-368 uses it to back up the blob layout immediately before a smoothing round; the round's neighbour counts at Grid.c:373-379 read only that backup, while births and deaths are written to the live layout at Grid.c:382-388. This makes each generation synchronous. Existing cellular-automata rules BR13087, BR21409, BR93785 and BR36680 describe the neighbour count and the birth/survival/death outcomes but not that all of them are judged from the pre-round snapshot, so the in-place-update alternative is not excluded. The other copyGrid uses (debug inspection copies at Architect.c:368/844 and the stored monster safety map at Monsters.c:2376) are covered by BR35671 or are not player-visible. ID BR19624 is unique across rules/ and test/.

----
BR13294
"On the title screen, the Play and View entries each carry a left-pointing triangle beside their name to mark that they open a further menu, while New Game and Quit carry no such marker."
Citation: [MainMenu.c:277-285](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L277-L285), [Buttons.c:105-111](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L105-L111)
Note: MainMenu.c:284 sets the first symbol of the first three main-menu buttons to a left-facing triangle, and MainMenu.c:283 limits that loop to the buttons other than Quit. Only the Play and View labels contain an asterisk placeholder at MainMenu.c:278-279 (New Game at 277 and Quit at 280 do not), and the button-drawing loop at Buttons.c:106-111 replaces an asterisk with the next configured symbol, so the triangle is actually drawn only beside Play and View; the triangle assigned to New Game never appears. BR77643 lists the four title-screen entries but not their markers, BR35067 describes the asterisk-to-symbol substitution generically (naming left triangles without saying which entries show one), and BR13130 describes only the entries' shared column. ID BR13294 is unique across rules/ and test/NewRules.txt.

----
BR25377
"A monster whose current movement or attack speed differs from its species' natural speed counts as a target for negation, and negating it restores both speeds to normal."
Citation: [Monsters.c:2515-2516](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2515-L2516), [Items.c:3778-3785](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3778-L3785)
Note: Monsters.c:2515-2516 are the two guards in monsterIsNegatable that answer true when a monster's live movement or attack speed differs from its species baseline (haste and slow set those fields at Items.c:3929-3932 and 3915-3918), so a merely hasted or slowed monster is treated as negatable; Items.c:3778-3785 resets both fields to the baseline inside negate(). BR29664 covers negation of player status values from the fixed status table and BR22853/BR71884 cover speed returning to normal when slow or haste expires, but no rule in rules/* states that negation restores a monster's altered speed. ID BR25377 is unique across rules/ and test/.

----
BR40997
"A monster that carries a mutation which can be negated counts as a target for negation, and negating it removes the mutation."
Citation: [Monsters.c:2517](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2517), [Items.c:3787-3792](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3787-L3792)
Note: Monsters.c:2517 is the mutation guard inside monsterIsNegatable: it answers true when mutationIndex is set and the mutation's canBeNegated flag is set (Globals.c:1396-1423 marks agile and juggernaut false, so those two speed/hardening mutations are never stripped, while explosive, infested, grappling, vampiric, toxic and reflective are). Items.c:3787-3792 clears mutationIndex and marks the monster negated. BR46598 covers only what a mutation does when applied, and the sidebar/description rules name a mutation without saying negation can remove it, so no rule in rules/* describes this negation behavior. ID BR40997 is unique across rules/ and test/.

----
BR29525
"A monster that knows at least one bolt negation can erase counts as a target for negation, and negating it strips those bolts while leaving any bolt that cannot be negated."
Citation: [Monsters.c:2521-2526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2521-L2526), [Items.c:3803-3816](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L3803-L3816)
Note: Monsters.c:2521-2526 scans the monster's 20 bolt slots and answers true for any bolt lacking BF_NOT_NEGATABLE (defined at Rogue.h:1856), making a monster whose only special feature is a spell a valid negation target; Items.c:3803-3816 blanks each such bolt and then compacts the surviving BF_NOT_NEGATABLE bolts into the front of the list. BR80726 says only that a scroll of negation "strips special abilities", and BR59057 is about a monster's decision to cast negation, so no rule in rules/* states that negation removes a monster's removable spells and spares the rest. ID BR29525 is unique across rules/ and test/.

----
BR42632
"When you examine a monster, your carried negation charm, scroll of negation, or wand of negation is named as something that will affect it only if the monster has a trait that negation can remove; a wand is omitted this way for a monster that reflects all bolts."
Citation: [Monsters.c:4693-4702](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L4693-L4702), [Monsters.c:2515-2526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2515-L2526)
Note: monsterIsNegatable's only caller is monsterDetails at Monsters.c:4693: it fills the negation-method list with whichever of the negation charm, scroll, and wand you carry, so when it returns false the list stays empty and the description never says the monster's traits can be removed or that your item will kill it (Monsters.c:4714-4721 and 4806-4833). Monsters.c:4700 additionally withholds the wand for a monster with MA_REFLECT_100. BR37022 covers only that examining opens a details box and BR62908 covers a reflector ignoring a negation bolt, so no rule in rules/* describes this examine-text behavior. ID BR42632 is unique across rules/ and test/.

----
BR42457
"A new game started without entering a seed number uses a seed derived from the current clock, so two such games begun at different times generate different dungeons."
Citation: [MainMenu.c:1172-1177](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1172-L1177), [RogueMain.c:234-237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L234-L237), [Math.c:184-186](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Math.c#L184-L186)
Note: For a new game that is not the seeded-game command, MainMenu.c:1172-1173 sets the requested seed to 0, which is passed straight to game initialization at MainMenu.c:1177; RogueMain.c:234-236 hands that seed to the seeding routine, and Math.c:185-186 replaces a seed of 0 with a value derived from the wall clock. Existing rules BR30507, BR88652, BR43142, and BR76508 cover only the seed prompt, its default, cancellation, and oversized-seed clamping, so no rule in rules/* states that an unseeded new game is seeded from the clock. (NewRules candidate BR14678 describes the same clock seeding but cites MainMenu.c:1194-1196, which is the saved-game branch, not this new-game branch.) ID BR42457 is unique across rules/ and test/.

----
BR38103
"In server mode, once the game that was started ends, the program quits instead of returning to the title screen."
Citation: [MainMenu.c:1180-1183](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/MainMenu.c#L1180-L1183)
Note: MainMenu.c:1180 enters the main play loop for the new game, so it returns when that game ends; MainMenu.c:1181-1183 then sets the next command to quit whenever server mode is active, bypassing the title screen. NewRules candidates BR13270 and BR42758 cover only quitting after finishing a playback/server-mode recording (MainMenu.c:1267-1269), and no rule in rules/* mentions server mode after an ordinary game. ID BR38103 is unique across rules/ and test/.

----
BR17271
"A button's background is shaded with a smooth left-to-right gradient: it is darkened by half at the left and right ends and reaches the button's full color in the middle, so the control reads as raised rather than flat."
Citation: [Buttons.c:32-34](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L32-L34), [Buttons.c:60-62](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L60-L62), [Buttons.c:87-91](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L87-L91)
Note: Buttons.c:32-34 returns a 0-to-100% sine curve across the label, Buttons.c:60-62 darkens the edge color by half toward black while keeping the button's full color for the middle, and Buttons.c:87-91 blends each character cell between those edge and middle colors when the button carries its default gradient flag. The comment at Buttons.c:37 describes the same edge-to-middle fade. Buttons that clear the gradient flag are drawn flat (MainMenu.c:740, Wizard.c:53), which BR18356 notes for wizard list rows, but no rule in rules/* describes the gradient itself. ID BR17271 is unique across rules/ and test/.

----
BR19796
"A button's label starts from white when the button is enabled and gray when it is disabled, with any color changes embedded in the label overriding that starting color."
Citation: [Buttons.c:51](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L51)
Note: Buttons.c:51 chooses the label's base color from the button's enabled flag: white when enabled and gray when disabled. Embedded color escapes override that base from the point they appear (BR41304), so the gray shows in the unescaped portions of a disabled label, including the leading spaces of the "---" separator rows that remain drawn while disabled at IO.c:208, IO.c:230, IO.c:255, IO.c:293, IO.c:326, IO.c:365, and IO.c:403 (their dashes use an explicit dark gray escape). BR37749 covers only that disabled controls cannot be clicked, so no rule in rules/* describes the gray base color of a disabled label. ID BR19796 is unique across rules/ and test/.

----
BR41304
"A button's label is white by default but may contain embedded color changes, so part of the text, such as a hotkey letter, is drawn in a different color."
Citation: [Buttons.c:38](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L38), [Buttons.c:80-83](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L80-L83)
Note: Buttons.c:38 states that label text is white but may use color escapes, and Buttons.c:80-83 skips over each escape in the label and adopts its encoded color for the characters that follow, so the white and yellow escapes spliced into labels (for example the Yes/No labels at IO.c:2944-2945 and IO.c:2948/2971) render as differently colored portions of a single button. BR41276 covers only the white and yellow colors of confirmation Yes/No labels, and no rule in rules/* describes color escapes in button labels generally. ID BR41304 is unique across rules/ and test/.

----
BR29092
"When a button is pressed, its background is darkened toward black, but if that darkening would barely change the button's already very dark background, the button instead brightens halfway toward the hover color."
Citation: [Buttons.c:40](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L40), [Buttons.c:64-70](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Buttons.c#L64-L70)
Note: Buttons.c:40 is the comment describing the pressed style, including the exception for a black button; Buttons.c:64-70 implements it by darkening the middle background 75% toward black and, when the result differs from the original color by less than a small threshold, discarding that darkened color and blending the original 50% toward the hover tint. Black buttons are used in the inventory list (Items.c:2874), so pressing one visibly brightens rather than darkens it. BR35576 covers the ordinary darkening and the dimmed label but not this near-black exception. ID BR29092 is unique across rules/ and test/.

----
BR38230
"A hidden pressure plate that belongs to a caustic gas trap releases a cloud of caustic gas when it is triggered."
Citation: [Globals.c:377-378](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L377-L378)
Note: Globals.c:377 is the hidden floor row whose discovery effect reveals the trap, and Globals.c:378 is the revealed caustic gas trap whose fire effect is the poison gas cloud. The generic pressure-plate rules (BR20087, BR39970) say a plate triggers the trap it controls but never say that this trap releases caustic gas. ID BR38230 is unique across rules/ and test/.

----
BR11139
"A hidden trap door drops whoever is standing on it through the floor into a hole when its pressure plate is triggered, and the hole remains as an open pit."
Citation: [Globals.c:379-380](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L379-L380)
Note: Globals.c:379 is the ordinary-floor appearance of the undiscovered trap door with the trap-door discovery effect, and Globals.c:380 is the revealed hole with automatic descent. BR99758 and BR63035 say a creature over a trap door is marked to fall and that diving must be confirmed, but no rule says that triggering the hidden plate opens the trap door and drops the creature through. ID BR11139 is unique across rules/ and test/.

----
BR11361
"A hidden pressure plate that belongs to a paralysis trap releases a cloud of paralytic gas when it is triggered."
Citation: [Globals.c:381-382](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L381-L382)
Note: Globals.c:381 is the hidden floor row with the paralysis-trap discovery effect, and Globals.c:382 is the revealed paralysis trigger. BR264 says paralysis gas paralyzes and BR20087 says a plate triggers its trap, but neither states that this trap releases paralytic gas. ID BR11361 is unique across rules/ and test/.

----
BR11928
"A hidden pressure plate that belongs to a confusion trap releases a cloud of psychotropic gas when it is triggered."
Citation: [Globals.c:385-386](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L385-L386)
Note: Globals.c:385 is the hidden floor row with the confusion-trap discovery effect, and Globals.c:386 is the revealed confusion trap whose fire effect is the confusion gas cloud. BR208 says confusion gas confuses, but no rule says that this trap produces it. ID BR11928 is unique across rules/ and test/.

----
BR12349
"A hidden pressure plate that belongs to a fire trap shoots a jet of flame when it is triggered, setting the victim and nearby flammable ground alight."
Citation: [Globals.c:387-388](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L387-L388)
Note: Globals.c:387 is the hidden floor row with the flamethrower-trap discovery effect, and Globals.c:388 is the revealed fire trap whose fire effect is the flamethrower. Existing fire rules cover ignition and burning (BR47057, BR480) but no rule describes the fire trap's own flaming jet. ID BR12349 is unique across rules/ and test/.

----
BR12448
"A hidden pressure plate that belongs to a flood trap floods the surrounding area with water when it is triggered."
Citation: [Globals.c:389-390](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L389-L390)
Note: Globals.c:389 is the hidden floor row with the flood-trap discovery effect, and Globals.c:390 is the revealed flood trap whose fire effect is the flood. Deep-water and shallow-water rules (BR51947, BR29950) describe the resulting water, but no rule says the trap floods the room. ID BR12448 is unique across rules/ and test/.

----
BR12506
"A hidden pressure plate that belongs to a net trap drops a net from the ceiling onto the square when it is triggered."
Citation: [Globals.c:391-392](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L391-L392)
Note: Globals.c:391 is the hidden floor row with the net-trap discovery effect, and Globals.c:392 is the revealed net trap whose fire effect is the falling net. BR91575 and BR96828 describe what webbing does to a creature, but no rule says that this trap creates the netting. ID BR12506 is unique across rules/ and test/.

----
BR13054
"A hidden pressure plate that belongs to an alarm trap sets off a piercing shriek that aggravates nearby monsters when it is triggered."
Citation: [Globals.c:393-394](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L393-L394)
Note: Globals.c:393 is the hidden floor row with the alarm-trap discovery effect, and Globals.c:394 is the revealed alarm trap whose fire effect is the aggravating shriek. Aggravation rules describe the status once applied (BR524), but no rule says the alarm trap applies it. ID BR13054 is unique across rules/ and test/.

----
BR13096
"Stepping into a closed wooden door opens it without stopping your move; the door blocks sight and gas, burns readily, and an open door swings shut again by itself."
Citation: [Globals.c:328-329](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L328-L329)
Note: Globals.c:328 defines the closed wooden door: vision- and gas-blocking, flammable, opened on step. Globals.c:329 defines the open door as flammable with a guaranteed per-turn promotion back to the closed door. Architect.c:2898-2906 decides which doorways become doors, but no rule in rules/* describes a door's own opening, closing, sight-blocking or burning behavior. ID BR13096 is unique across rules/ and test/.

----
BR13131
"A secret door looks exactly like an ordinary stone wall and blocks movement, sight, and gas until it is discovered, after which it becomes a wooden door."
Citation: [Globals.c:330](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L330)
Note: Globals.c:330 is the secret door row: it draws as a wall, obstructs everything, is flammable and secret, and its discovery effect is an ordinary door. BR21571 says revealing a secret replaces the tile with the feature it hid and BR82459 says seeing a monster pass through reveals it, but no rule describes the disguised wall's appearance and obstruction before discovery. ID BR13131 is unique across rules/ and test/.

----
BR13147
"A locked iron door blocks movement and sight until the matching key is used on it, after which it remains open."
Citation: [Globals.c:331-332](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L331-L332)
Note: Globals.c:331 is the locked iron door, which obstructs everything and is opened only by a key on its square; Globals.c:332 is the permanent open iron door that no longer blocks movement or sight. BR14498 and BR14513 cover the act of using a key on terrain, but no rule says locked doors block until keyed or that keying one leaves it open. ID BR13147 is unique across rules/ and test/.

----
BR13258
"A closed portcullis blocks movement and items, and when its mechanism is powered the iron bars rise and vanish."
Citation: [Globals.c:339-340](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L339-L340)
Note: Globals.c:339 is the closed portcullis, passability- and item-blocking and wired to a mechanism, with the open-portcullis feature as its promotion. Globals.c:340 is the dormant floor that activates the portcullis. BR14518 covers generic promotion on entry, but no rule describes a portcullis blocking a passage or being raised by its mechanism. ID BR13258 is unique across rules/ and test/.

----
BR13797
"A dry wooden barricade blocks movement and items and burns fiercely, so setting it alight clears the passage."
Citation: [Globals.c:341](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L341)
Note: Globals.c:341 is the wooden barricade: passability- and item-blocking, flammable, with a dedicated burning feature. BR488 says flammable material that catches fire is consumed and replaced by fire, but no rule describes the barricade as an obstruction that can be burned away. ID BR13797 is unique across rules/ and test/.

----
BR15015
"A marble statue blocks movement, items, gas, and surface effects, and a statue rigged to a mechanism cracks and then shatters, while an instant-cracking statue shatters as soon as its mechanism is powered."
Citation: [Globals.c:351-354](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L351-L354)
Note: Globals.c:351 is the inert statue with all four obstruction flags; Globals.c:352 and Globals.c:354 are wired statues that become the cracking statue or shatter immediately; Globals.c:353 is the cracking statue with a 35% per-turn chance to shatter. No rule in rules/* describes statues as obstructions or their cracking and shattering. ID BR15015 is unique across rules/ and test/.

----
BR15126
"A stone archway portal is inert until its mechanism is powered, when it flashes and activates."
Citation: [Globals.c:355](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L355)
Note: Globals.c:355 is the portal archway, wired to a mechanism and promoting to the activated portal. BR38379 describes the light released when a portal activates, but no rule says that the archway is inert beforehand or what activation does. ID BR15126 is unique across rules/ and test/.

----
BR16151
"A turret can lie hidden inside a wall and emerge when its mechanism is powered."
Citation: [Globals.c:356](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L356)
Note: Globals.c:356 is the dormant turret disguised as an ordinary stone wall, wired to a mechanism, and promoting to the turret-emergence feature. Turret combat and bestiary rules exist (BR31624, BR52291), but no rule says a turret can start hidden in a wall and emerge. ID BR16151 is unique across rules/ and test/.

----
BR16668
"A monster can be sealed inside a wall and burst out when its mechanism is powered."
Citation: [Globals.c:357](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L357)
Note: Globals.c:357 is the dormant wall monster disguised as an ordinary stone wall, wired to a mechanism, with the wall-shatter feature as its promotion. Horde and monster-generation rules (BR114, BR171) describe monsters arriving by other means, but no rule says a monster can begin hidden inside a wall and break out. ID BR16668 is unique across rules/ and test/.

----
BR17211
"A mechanism can darken the ground, plunging the surrounding area into supernatural darkness."
Citation: [Globals.c:358-360](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L358-L360)
Note: Globals.c:358 is the dormant floor wired to a mechanism; Globals.c:359 is the darkening floor that promotes after a delay; Globals.c:360 is the finished dark floor, which casts the supernatural-darkness light. BR41963 describes the darkness cloud's shading, but no rule says a mechanism can turn ordinary ground into a patch of darkness. ID BR17211 is unique across rules/ and test/.

----
BR17238
"A hidden trigger on the ground activates its mechanism when you step on it and then disappears."
Citation: [Globals.c:361](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L361)
Note: Globals.c:361 is the featureless ground tile that is wired to a mechanism, promotes when the player enters, and vanishes on promotion, so the trigger is used once. BR14518 covers terrain that promotes on entry generally, but no rule describes an invisible one-use floor trigger. ID BR17238 is unique across rules/ and test/.

----
BR17243
"A repeating floor trigger reactivates its mechanism each time you step onto it."
Citation: [Globals.c:535](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L535)
Note: Globals.c:535 is the featureless repeating trigger, wired to a mechanism and promoting on player entry without vanishing, so it can fire on every entry. BR17238 covers the one-use trigger; no rule describes a trigger that can be reused. ID BR17243 is unique across rules/ and test/.

----
BR17784
"A pressure plate used by a mechanism becomes an inactive plate once it is stepped on and no longer triggers anything."
Citation: [Globals.c:402-403](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L402-L403)
Note: Globals.c:402 is the exposed pressure plate that promotes on step to Globals.c:403, the used plate, which has no trap flag and no promotion. BR20087 and BR22348 describe a plate depressing and resetting when clear, which is the behavior of reusable trap plates, not this one-use mechanism plate. ID BR17784 is unique across rules/ and test/.

----
BR18595
"A magical glyph on the floor activates its mechanism when you step on it, then becomes an inactive glyph."
Citation: [Globals.c:404-405](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L404-L405)
Note: Globals.c:404 is the magical glyph, wired to a mechanism and promoting on player entry to the inactive glyph at Globals.c:405. BR11577 and BR16893 describe the two glyphs' light radii, but no rule says that stepping on the active glyph triggers its mechanism and changes it. ID BR18595 is unique across rules/ and test/.

----
BR18840
"A dormant gas vent hidden in the floor begins spewing paralytic gas when its mechanism is powered."
Citation: [Globals.c:383-384](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L383-L384)
Note: Globals.c:383 is the hidden vent with the paralysis-vent discovery effect, and Globals.c:384 is the revealed inactive vent connected to a reserve of paralytic gas; both are wired and promote to the spewing feature. BR596 describes the paralysis gas effect, but no rule says a vent can release it. ID BR18840 is unique across rules/ and test/.

----
BR19757
"A gas vent hidden in the floor opens and begins spewing caustic gas when its mechanism is powered."
Citation: [Globals.c:395-397](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L395-L397)
Note: Globals.c:395 is the hidden vent, Globals.c:396 is the dormant vent it becomes, and Globals.c:397 is the active vent that spews poison gas on a 100% per-turn chance. BR745 says standing in caustic gas injures, but no rule describes the vent that produces it. ID BR19757 is unique across rules/ and test/.

----
BR20022
"A gas vent hidden in the floor begins spewing explosive methane when its mechanism is powered."
Citation: [Globals.c:398-400](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L398-L400)
Note: Globals.c:398 is the hidden vent, Globals.c:399 is the dormant vent it becomes, and Globals.c:400 is the active vent that spews methane on a 50% per-turn chance and is itself flammable. BR21531 and BR501 describe methane igniting and exploding once it exists, but no rule describes the vent. ID BR20022 is unique across rules/ and test/.

----
BR20239
"A steam vent periodically blasts scalding steam from a natural crevice in the floor."
Citation: [Globals.c:401](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L401)
Note: Globals.c:401 is the steam vent, which blocks items and promotes to a steam puff on a 2.5% per-turn chance. BR745 covers the damage of standing in damaging gas, but no rule describes a vent that produces scalding steam on its own. ID BR20239 is unique across rules/ and test/.

----
BR20583
"A glass dewar blocks movement and gas and is flammable; stepping into one shatters it, releasing a pressurised cloud of its gas and leaving broken glass on the floor."
Citation: [Globals.c:406-409](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L406-L409)
Note: Globals.c:406-409 are the caustic, confusion, paralysis, and methane dewars; each obstructs movement and gas, is flammable, and promotes when the player enters to a gas-release feature that also leaves broken glass (Globals.c:704-707). BR18889 mentions a dewar only as a travel destination, and no rule describes breaking one. ID BR20583 is unique across rules/ and test/.

----
BR20612
"A bog is knee-deep mud that lets a creature submerge, and it occasionally releases a puff of methane."
Citation: [Globals.c:415](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L415)
Note: Globals.c:415 is the mud tile: it allows submerging and promotes to a methane puff on a 1% per-turn chance. BR16982 mentions a bog only as a surface that halves fall damage, and no rule describes its muddy, submerging, methane-releasing behavior. ID BR20612 is unique across rules/ and test/.

----
BR21736
"Brimstone ignites spontaneously; burning active brimstone leaves inert brimstone, which can later reignite."
Citation: [Globals.c:425-426](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L425-L426)
Note: Globals.c:425 is active brimstone, which is flammable and spontaneously ignites, burns out to inert brimstone, and can also cool to it by chance; Globals.c:426 is inert brimstone, which spontaneously ignites and promotes back to active brimstone on an 8% per-turn chance. BR99 and BR180 mention brimstone only as terrain that lava placement or monster pathing avoids. ID BR21736 is unique across rules/ and test/.

----
BR21906
"A rope bridge is flammable, and when it burns it snaps and plunges into the chasm below, leaving a gap."
Citation: [Globals.c:428-430](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L428-L430)
Note: Globals.c:428 is the rickety rope bridge, flammable and vanishing on promotion; Globals.c:429 is the plummeting bridge; Globals.c:430 is the bridge edge staked to the chasm rim. Globals.c:745 gives the snap-and-plunge message and the falling-bridge feature. BR117 and BR120 cover bridge generation only. ID BR21906 is unique across rules/ and test/.

----
BR21921
"A flooding mechanism fills the surrounding area with water, which later drains away again."
Citation: [Globals.c:432-433](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L432-L433)
Note: Globals.c:432 is the dormant flood water and Globals.c:433 is the spreading flood water, which advances on a 25% per-turn chance; the deep and shallow flood water rows at Globals.c:445-446 have negative promotion chances that drain them away. BR51947 and BR29950 describe ordinary deep and shallow water, not a machine-driven flood that later recedes. ID BR21921 is unique across rules/ and test/.

----
BR22507
"The translucent ground around the rim of a hole crumbles away over time, while the hole itself drops anything standing over it to the level below."
Citation: [Globals.c:442-444](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L442-L444)
Note: Globals.c:442 is the hole, with automatic descent; Globals.c:443 is its glowing variant; Globals.c:444 is the hole edge, which has a negative promotion chance and vanishes on promotion, so the safe rim retreats. BR16982 and BR99758 say a creature over a chasm, hole, or trap door is marked to fall, but neither describes the crumbling rim. ID BR22507 is unique across rules/ and test/.

----
BR22596
"A patch of poisonous lichen grows outward onto adjacent open ground each turn."
Citation: [Globals.c:451](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L451)
Note: Globals.c:451 is the lichen tile, which poisons creatures standing on it and promotes to the lichen-growth feature on a 100% per-turn chance. BR81488 and BR55229 describe the poison and levitation immunity, and BR44267 describes the level generator scattering lichen while the dungeon is built; none describes a lichen patch continuing to spread during play. ID BR22596 is unique across rules/ and test/.

----
BR23007
"Dense foliage and luminescent fungal forest block sight and burn readily; walking through either tramples it, and trampled growth slowly regrows."
Citation: [Globals.c:472-476](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L472-L476)
Note: Globals.c:472 and 475 are the sight-blocking, flammable foliage and fungal forest that promote to trampled versions on step; Globals.c:473 is dead foliage; Globals.c:474 and 476 are the trampled versions, which regrow on a 1% per-turn chance. BR28360 and BR480 describe burning terrain generally, but no rule describes the sight-blocking, trampling, and regrowth behavior. ID BR23007 is unique across rules/ and test/.

----
BR23867
"A green forcefield blocks movement, gas, and diagonal movement, and it melts away over time and when a creature touches it."
Citation: [Globals.c:477-478](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L477-L478)
Note: Globals.c:477 is the forcefield with passability, gas, and diagonal obstruction and a negative promotion chance that accelerates where more open ground borders it; Globals.c:478 is the dissolving crystal it becomes, which also promotes on creature contact. BR33630 describes only its light. ID BR23867 is unique across rules/ and test/.

----
BR25180
"An iron cage over a candle-lit altar blocks access to the items until a matching key is used on the altar, which lifts the cage."
Citation: [Globals.c:363-365](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L363-L365)
Note: Globals.c:363 is the keyhole altar, which is wired and opens with a key; Globals.c:364 is the altar with its cage open and a promotion that closes it when no key remains; Globals.c:365 is the closed iron cage, passability- and surface-blocking, which opens with a key. Existing key rules (BR14498, BR14513) describe using a key but not the caged-altar mechanism. ID BR25180 is unique across rules/ and test/.

----
BR27214
"Taking an item from a candle-lit altar triggers its mechanism and leaves the altar inert."
Citation: [Globals.c:366-367](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L366-L367)
Note: Globals.c:366 and 367 are the altar switches, wired to a mechanism and promoting when an item is picked up, to the inert altar row. No rule in rules/* describes an altar that reacts to an item being taken. ID BR27214 is unique across rules/ and test/.

----
BR28571
"A locked iron cage blocks movement and releases its captive when a matching key is used on it."
Citation: [Globals.c:370-371](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L370-L371)
Note: Globals.c:370 is the open cage; Globals.c:371 is the locked iron cage, which obstructs passability, surface effects, and gas and promotes to the cage-opening feature when keyed. BR13418 and BR14498 cover freeing a captive with a key, but no rule describes the cage itself as an obstruction that opens. ID BR28571 is unique across rules/ and test/.

----
BR29906
"A sealed coffin is flammable; when it burns it bursts open and a dark figure rises from it."
Citation: [Globals.c:372-373](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L372-L373)
Note: Globals.c:372 is the sealed coffin, flammable and wired to a mechanism, whose fire effect is the burning coffin and whose promotion is the burst-open coffin; Globals.c:373 is the open coffin. Globals.c:807 gives the dark-figure message and wakes the dormant monster. No rule in rules/* describes a coffin opening or releasing its occupant. ID BR29906 is unique across rules/ and test/.

----
BR30221
"A bloodwort stalk grows seed pods, and stepping on a pod bursts it into a cloud of healing spores."
Citation: [Globals.c:513-514](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L513-L514)
Note: Globals.c:513 is the bloodwort stalk, which grows pods on a 1% per-turn chance; Globals.c:514 is the pod, which blocks movement and items and promotes when the player enters to the pod-burst feature that releases healing spores. BR18146 describes only the spore cloud's color. ID BR30221 is unique across rules/ and test/.

----
BR31859
"A cloud of healing spores restores a portion of the health of each living creature standing in it."
Citation: [Globals.c:510](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L510)
Note: Globals.c:510 is the healing cloud, flagged as causing healing; Time.c:547-559 restores one fifteenth of the creature's maximum health per 100 ticks to non-inanimate, non-submerged creatures. BR18146 covers only the cloud's dark red color. ID BR31859 is unique across rules/ and test/.

----
BR31870
"Luminescent algae spreads through deep water and later recedes, glowing blue and then green."
Citation: [Globals.c:520-522](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L520-L522)
Note: Globals.c:520 is the algae well; Globals.c:521 is the blue-glowing algae, which promotes to spread at a 5% per-turn chance; Globals.c:522 is the green-glowing algae, which reverts at a 3% per-turn chance. BR41031 describes only the light radius. ID BR31870 is unique across rules/ and test/.

----
BR31949
"Thorned vines entangle and injure creatures, and when you step onto them they wither into ordinary grass."
Citation: [Globals.c:525-526](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L525-L526)
Note: Globals.c:525 is the ancient-spirit vine tile, entangling, damaging, flammable, and promoting when the player enters to Globals.c:526, a tuft of grass. BR91575 and BR96828 describe entangling terrain generally, but no rule describes these vines or their withering. ID BR31949 is unique across rules/ and test/.

----
BR32952
"A patch of sunlight lights its square, while a patch of shadows darkens it."
Citation: [Globals.c:423-424](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L423-L424)
Note: Globals.c:423 is the sunlight pool, which casts the sun light; Globals.c:424 is the darkness patch, which casts the darkness-patch light that darkens the square. BR41963 covers only the supernatural-darkness cloud, and no rule describes these terrain patches. ID BR32952 is unique across rules/ and test/.

----
BR33124
"Taking the Amulet of Yendor from its pedestal triggers the mechanism around it."
Citation: [Globals.c:529](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L529)
Note: Globals.c:529 is the featureless floor switch beneath the Amulet, wired to a mechanism and promoting when an item is picked up. BR37593 and BR38076 describe winning with the Amulet, but no rule says that lifting it triggers machinery. ID BR33124 is unique across rules/ and test/.

----
BR33186
"When two enchantable items rest on twin commutation altars, their enchantment levels are swapped and the altars are scorched."
Citation: [Globals.c:532-533](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L532-L533), [Items.c:1160-1189](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Items.c#L1160-L1189)
Note: Globals.c:532 is the commutation altar, wired and flagged to swap enchantments; Globals.c:533 is the scorched altar it becomes. Items.c:1160-1189 finds two swappable items on the same mechanism and exchanges their effective enchantment levels only when the levels differ. BR19523 covers only the seed catalog's listing of these altars. ID BR33186 is unique across rules/ and test/.

----
BR34188
"When its mechanism is powered, glowing glass pipes char and become inert."
Citation: [Globals.c:534-535](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L534-L535)
Note: Globals.c:534 is the glowing glass pipe, wired to a mechanism and promoting to the inert pipe at Globals.c:535, which no longer glows. No rule in rules/* describes the pipes changing state with their mechanism. ID BR34188 is unique across rules/ and test/.

----
BR35147
"When a resurrection altar's mechanism is powered, it brings back your most powerful fallen ally and becomes a scorched altar."
Citation: [Globals.c:538-539](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L538-L539), [Monsters.c:2889-2904](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L2889-L2904)
Note: Globals.c:538 is the resurrection altar, wired to a mechanism and promoting to the inert altar at Globals.c:539; the resurrection feature calls resurrectAlly (Architect.c:3370-3371), which prefers the fallen ally with the greatest total power (Monsters.c:2889-2904). BR70163 says dead allies are kept for resurrection, but no rule describes the altar performing it. ID BR35147 is unique across rules/ and test/.

----
BR36150
"A sacrificial altar demands the marked sacrifice, and when that creature steps onto the altar the sacrifice is completed."
Citation: [Globals.c:543-544](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L543-L544)
Note: Globals.c:543 is the dormant sacrifice altar; Globals.c:544 is the active sacrifice altar, which promotes when a creature marked for sacrifice enters and is wired to its mechanism. BR36447 covers the generic sacrifice-entry promotion, and BR126 covers marked hordes, but no rule describes the altar's demand and completion. ID BR36150 is unique across rules/ and test/.

----
BR37178
"A chasm can conceal a stone bridge that extends across it when its mechanism is powered."
Citation: [Globals.c:554-556](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L554-L556)
Note: Globals.c:554 is the chasm with a hidden bridge, indistinguishable from an ordinary chasm; Globals.c:555 is the active bridge section that extends across it; Globals.c:556 is the chasm edge wired to announce the bridge. BR117 and BR120 cover bridge generation over open gaps, not a machine-built bridge hidden in a chasm. ID BR37178 is unique across rules/ and test/.

----
BR37431
"A wall rigged to a rat trap cracks and then shatters, releasing the rats hidden behind it."
Citation: [Globals.c:559-560](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L559-L560)
Note: Globals.c:559 is the dormant rat-trap wall; Globals.c:560 is the cracking wall, which promotes to the wall-shatter feature on a 5% per-turn chance. BR16668 covers a single monster bursting from a wall, but no rule describes this cracking wall that releases a group of rats. ID BR37431 is unique across rules/ and test/.

----
BR37960
"A darkened crystal globe keeps its mechanism from activating until an electric shock lights it, and the shining globe no longer breaks the circuit."
Citation: [Globals.c:563-564](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L563-L564)
Note: Globals.c:563 is the darkened globe, which promotes when exposed to electricity and is a circuit breaker; Globals.c:564 is the shining globe, which promotes no further and is not a circuit breaker. Time.c:1097-1108 refuses to activate any mechanism that still contains a circuit breaker. BR36633 covers electricity promoting terrain, but no rule describes the globe's blocking role. ID BR37960 is unique across rules/ and test/.

----
BR38405
"The passage left by an underworm crumbles through solid granite, opening a tunnel through the rock."
Citation: [Globals.c:568-570](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L568-L570)
Note: Globals.c:568 is a dormant worm-tunnel marker; Globals.c:569 is the active marker, which crumbles the granite on a per-turn chance; Globals.c:570 is the outer wall, which shatters when the tunnel is complete. BR52291 describes the underworm itself but not the tunnel it leaves. ID BR38405 is unique across rules/ and test/.

----
BR38875
"A ceremonial brazier is a burning fixture that blocks movement and items."
Citation: [Globals.c:573](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L573)
Note: Globals.c:573 is the brazier, flagged as fire and as obstructing passability and items, and listed in the sidebar. No rule in rules/* describes a brazier or its obstruction. ID BR38875 is unique across rules/ and test/.

----
BR39115
"In the goblin warren, mud walls block movement, the mud floor is flammable, and the hanging animal-skin doorway blocks sight and gas and burns readily."
Citation: [Globals.c:576-578](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L576-L578)
Note: Globals.c:576 is the flammable mud floor; Globals.c:577 is the obstruction mud wall; Globals.c:578 is the hanging animal-skin doorway, which blocks vision and gas and is flammable. BR114 mentions the goblin encampment structure but no rule describes its mud-and-skin construction. ID BR39115 is unique across rules/ and test/.

----
BR39634
"Sputtering embers cool into ash over time."
Citation: [Globals.c:469](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L469)
Note: Globals.c:469 is the embers tile, which promotes to ash on a 3% per-turn chance. BR27751 describes only their light. ID BR39634 is unique across rules/ and test/.

----
BR40221
"Ice is frozen water that gradually melts back into deep or shallow water, faster where more open ground borders it."
Citation: [Globals.c:435-438](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L435-L438)
Note: Globals.c:435-438 are deep and shallow ice and their melting variants; the unmelted rows have negative promotion chances, so melting accelerates with the number of passable neighbours, and they become the corresponding water rows. BR51947 and BR29950 describe the resulting water but not ice. ID BR40221 is unique across rules/ and test/.

----
BR40612
"A lever that has been pulled becomes an inactive lever that cannot be pulled again."
Citation: [Globals.c:348-349](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L348-L349)
Note: Globals.c:348 is the lever, which promotes when the player enters it; Globals.c:349 is the pulled lever, which has no promotion and whose description says it will not budge. BR14518 covers the initial pull, but no rule says the lever is then spent. ID BR40612 is unique across rules/ and test/.

----
BR41553
"A wall-mounted torch wired to a mechanism can drop from its bracket and burn on the floor when the mechanism is powered."
Citation: [Globals.c:342-343](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L342-L343)
Note: Globals.c:342 is the dormant wall-mounted torch, wired to a mechanism and promoting to Globals.c:343, the fallen torch burning on the floor. BR34933 and BR17041 describe ordinary torch light, but no rule says a torch can be made to fall. ID BR41553 is unique across rules/ and test/.

----
BR35318
"Stone walls are tinted by a depth-dependent shade that shifts from a lighter grey near the surface to a darker tone at the amulet's depth."
Citation: [Globals.c:80-81](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L80-L81), [Globals.c:296](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L296), [Globals.c:327](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L327), [RogueMain.c:541-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L541-L545)
Note: Globals.c:80-81 define the wall shade at the surface and at the amulet depth, Globals.c:296 registers that pair as a dynamic colour, and Globals.c:327 is the plain stone wall that uses it. RogueMain.c:541-545 blends the two by the fraction of the amulet's depth reached. BR37725 describes only the ambient light's similar depth blend, and no rule describes the wall tint. ID BR35318 is unique across rules/ and test/.

----
BR21153
"The ground is tinted by a depth-dependent shade that shifts from a blue-black near the surface to a neutral dark grey at the amulet's depth."
Citation: [Globals.c:90-91](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L90-L91), [Globals.c:299](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L299), [Globals.c:323-324](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L323-L324), [RogueMain.c:541-545](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L541-L545)
Note: Globals.c:90-91 define the floor shade at the surface and at the amulet depth, Globals.c:299 registers that pair as a dynamic colour, and Globals.c:323-324 are the ordinary and floodable floor that use it. RogueMain.c:541-545 blends the two by depth. BR29950 and BR38901 describe the water tints, and BR37725 the ambient light, but no rule describes the floor's own depth tint. ID BR21153 is unique across rules/ and test/.

----
BR16712
"Periodic hordes appear only when both a legal spawn square and an eligible horde type exist; if either is unavailable, no horde is created."
Citation: [Monsters.c:1097-1112](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1097-L1112), [Monsters.c:798-806](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L798-L806), [Monsters.c:1123-1125](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L1123-L1125)
Note: getRandomMonsterSpawnLocation reports failure when no square survives the visible, occupied, stair, harmful-terrain and machine filters (Monsters.c:1097-1111), and spawnPeriodicHorde only attempts a spawn when that call succeeds (Monsters.c:1123); when the random horde type lookup finds no eligible type it returns null (Monsters.c:800-806), so no creature is created. BR82926 and BR49453 describe where periodic hordes are placed and that they wander, but no rule states that an impossible placement or unavailable horde type yields no horde. ID BR16712 is unique across rules/ and test/.

----
BR12490
"A recording file is created the first time it is written and its fixed header is then rewritten in place at the start of the file, so each later save updates that same file rather than appending a second header."
Citation: [Recordings.c:202-216](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L202-L216)
Note: Recordings.c:202-207 creates an empty file when the recording's path does not yet exist, then Recordings.c:209-216 reopens the file for update, rewinds to the start, and writes all header bytes over the existing contents. Because this runs on every buffer flush (Recordings.c:232-233), the opening metadata is refreshed in place instead of duplicate headers being appended. No rule in rules/* cites Recordings.c:177-221 or otherwise states that a recording file is created on first write and its header updated in place. ID BR12490 is unique across rules/ and test/.

----
BR19228
"The recording header stores the recording's total length, and a recording is never shorter than its own header, so even a recording with no played turns remains a complete file that playback can finish."
Citation: [Recordings.c:199-200](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L199-L200), [Recordings.c:218-220](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L218-L220)
Note: Recordings.c:199-200 writes the running recording length into the last 4-byte field of the 36-byte header; playback reads that field back at Recordings.c:529, and recallChar reports the end of the recording once the read position passes it (Recordings.c:268-270). Recordings.c:218-219 then raises the length to the header size when it would otherwise be shorter, so an empty recording still has a valid length. Existing playback rules cover seeking, pausing, and out-of-sync handling, but no rule in rules/* describes the stored recording length or the header-size floor. ID BR19228 is unique across rules/ and test/.

----
BR26537
"A creature carrying the explosive mutation is wreathed in a glowing orange aura that lights the tiles within 1.5 to 2 tiles of it, fading to nothing at the edge."
Citation: [Light.c:246-248](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L246-L248), [Globals.c:1372](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L1372), [Globals.c:956](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L956)
Note: Light.c:246-248 paints a mutation's light at the creature's location whenever its mutation has one; Globals.c:1372 is the only mutation row with a light, granting the explosive mutation the explosive bloat light, whose Globals.c:956 entry is the orange explosive aura colour over radius 150-200 (1.5-2 tiles) with zero edge brightness. The bestiary glow rules (e.g. BR44680) describe the explosive bloat monster kind, and BR83937 covers telepathy light, but no rule in rules/* describes the glow granted by the explosive mutation. ID BR26537 is unique across rules/ and test/.

----
BR31554
"A creature that is on fire, but is not one that burns unceasingly by nature, casts firelight that lights the tiles within 3 to 4 tiles of it, fading to nothing at the edge."
Citation: [Light.c:250-252](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Light.c#L250-L252), [Globals.c:940](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L940), [Globals.c:72](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Globals.c#L72)
Note: Light.c:250-252 paints the burning-creature light at any burning creature's location unless its kind is naturally fiery; Globals.c:940 defines that light as the red-orange firebolt colour over radius 300-400 (3-4 tiles) with zero edge brightness, and Globals.c:72 defines the colour. BR30955 covers the player's carried light turning fiery while burning, but no rule in rules/* describes the firelight cast by a burning monster. ID BR31554 is unique across rules/ and test/.

----
BR37255
"If an open-area vault is given a starting tile, it grows from that tile instead of picking a random floor tile, so a horde's accompanying encampment rises where the horde appeared."
Citation: [Architect.c:1157-1163](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L1157-L1163), [Monsters.c:852-855](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Monsters.c#L852-L855)
Note: Architect.c:1157-1163 replaces the origin with a random floor tile only when no location was supplied; otherwise the supplied tile is used as the growth origin. Monsters.c:852-855 builds a horde's accompanying machine at the horde's chosen location, which is the in-game path that supplies an origin. BR36818 describes only the random-tile case and BR94740 covers entrance-style vaults, so no rule in rules/* states that a supplied starting tile is used. ID BR37255 is unique across rules/ and test/.

----
BR40805
"When an open-area vault picks its own starting tile, that tile is a floor tile with no creature, item, staircase, your character, or existing vault on it."
Citation: [Architect.c:1159-1163](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L1159-L1163), [Architect.c:3827-3842](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L3827-L3842)
Note: Architect.c:1160 asks for a random floor tile, and the search at Architect.c:3827-3842 rejects any square flagged as holding the player, a monster, stairs, an item, or a machine, as well as item-blocking terrain. BR36818 says only that the vault starts on a random floor tile, not that the tile must be otherwise empty. ID BR40805 is unique across rules/ and test/.

----
BR38283
"If an open-area vault's attempted interior is rejected, the next attempt starts from an empty interior, so no tiles claimed by the failed attempt are carried over."
Citation: [Architect.c:1152-1155](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Architect.c#L1152-L1155)
Note: Architect.c:1154 clears the mapped interior at the top of every retry, before the starting tile is chosen and distances are recalculated. BR45230 covers abandoning a vault without changing the level and BR41023 covers restoring the level after a failed build, but no rule in rules/* states that each retry begins with a clean interior. ID BR38283 is unique across rules/ and test/.

----
BR22324
"When hunger forces you to eat at the end of a turn, the game announces 'unable to control your hunger, you eat a mango.' for fruit or 'unable to control your hunger, you eat a ration of food.' otherwise, and consumes only the first food item in your pack."
Citation: [Time.c:843-851](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L843-L851), [Time.c:2632-2634](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Time.c#L2632-L2634)
Note: Time.c:843-851 walks the pack and stops at the first food item, naming it a mango when it is fruit and a ration of food otherwise (845-847), then eats it (849). The trigger at Time.c:2632-2634 (working tree 2665-2667) calls this only when nutrition is 1 or less at the end of the player's turn. BR33602 covers the forced meal and its turn cost but not the announcement or that only one item is eaten, and no rule in rules/* quotes this message. ID BR22324 is unique across rules/ and test/.

----
BR13458
"Typing a turn beyond the recording's total length jumps only as far as the recording's final turn, so the destination never exceeds the number of turns available."
Citation: [Recordings.c:801-803](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L801-L803)
Note: Recordings.c:802 clamps the requested turn with min(destinationFrame, rogue.howManyTurns) before seeking, and howManyTurns is the recording's stored total turn count (read at Recordings.c:527). BR15719 covers the go-to-turn prompt, BR59377 the already-at-turn alert, and BR23216 the out-of-sync alert, but no rule in rules/* states that an oversized destination is capped to the recording's last turn. ID BR13458 is unique across rules/ and test/.

----
BR13906
"After the recording jumps to a chosen turn, playback remains paused at that turn until the player presses space to resume."
Citation: [Recordings.c:801-804](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Recordings.c#L801-L804)
Note: Recordings.c:804 sets the paused state true immediately after the seek in the branch that actually jumps (Recordings.c:801-803); the already-at-turn and out-of-sync branches above it return without seeking. BR15719 covers only the prompt that starts the jump, and BR59377/BR23216 cover the two alerts, so no rule in rules/* describes that the recording ends up paused after a seek. ID BR13906 is unique across rules/ and test/.

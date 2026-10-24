**BR47860** — The game distinguishes three modes: normal, easy, and wizard.
[RogueMain.c:1232-1234](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1232-L1234)
[RogueMain.c:1364-1368](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1364-L1368)
[RogueMain.c:1407-1415](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1407-L1415)
**BR22377** — A run is recorded into the run history only when played in normal mode; easy and wizard runs are not.
[RogueMain.c:1232-1234](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1232-L1234)
[RogueMain.c:1398-1400](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1398-L1400)
**BR39597** — At game start the game chooses among three gameplay variants: the standard game, a "rapid" variant, and a "bullet" variant.
[RogueMain.c:173-185](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L173-L185)
**BR19079** — A new game begins on depth 1 and records the deepest depth reached as 1.
[RogueMain.c:376-377](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L376-L377)
**BR89567** — A new game resets the accumulated gold and the total gold generated so far to 0.
[RogueMain.c:383-384](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L383-L384)
**BR59025** — A new game resets the player turn count and the absolute turn count to 0.
[RogueMain.c:379-380](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L379-L380)
**BR74104** — The player starts with 12 strength.
[RogueMain.c:396](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L396)
**BR84421** — The player starts with a food ration.
[RogueMain.c:420-421](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L420-L421)
**BR49147** — The player starts with an uncursed non-runic dagger that is immediately equipped.
[RogueMain.c:423-428](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L423-L428)
**BR81759** — The player starts with 15 darts.
[RogueMain.c:430-435](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L430-L435)
**BR27339** — The player starts with uncursed non-runic leather armor that is immediately equipped.
[RogueMain.c:437-442](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L437-L442)
**BR75391** — In a debug build the player additionally starts with a clairvoyance ring, an awareness ring, a powerful runic dagger and runic leather armor, five high-charge staves, and four wands.
[RogueMain.c:451-531](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L451-L531)
**BR78012** — The welcome message tells the player to retrieve the Amulet of Yendor from a specific floor of the dungeon and escape with it, naming that floor with an ordinal number.
[RogueMain.c:157-171](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L157-L171)
**BR69169** — Before the welcome message, the message log and archive are cleared and the screen is blacked out.
[RogueMain.c:319-323](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L319-L323)
[RogueMain.c:532-534](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L532-L534)
**BR26483** — In a debug build with immortality active, dying instead fully heals the player, refills hunger if it is very low, and resumes the run rather than ending it.
[RogueMain.c:1134-1143](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1134-L1143)
**BR35287** — The stair connecting two depths is shared: the down-stair of one depth and the up-stair of the next depth are placed at the same location.
[RogueMain.c:276-284](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L276-L284)
**BR92887** — Descent is capped at the dungeon's deepest level: while there, any attempt to move to a deeper level (whether by descending stairs or by falling) does nothing, while going back up is allowed.
[RogueMain.c:558-560](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L558-L560)
**BR11461** — Monsters near the player that are able to use stairs will follow the player to the next level, approaching the stairs after a delay based on their distance.
[RogueMain.c:590-628](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L590-L628)
**BR71115** — When the player leaves a level, its map, items, and monsters are saved; returning to it restores that saved state.
[RogueMain.c:630-661](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L630-L661)
[RogueMain.c:775-814](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L775-L814)
**BR58181** — The player arrives beside the stairs when using them, or near the point of impact when falling.
[RogueMain.c:847-892](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L847-L892)
**BR40245** — When the player arrives on a square containing an item, the game announces the item lying below.
[RogueMain.c:896-905](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L896-L905)
**BR68635** — The first time the player arrives on the amulet's floor a special flavor message is shown, and the first time the player arrives on the deepest floor a different special message is shown.
[RogueMain.c:838-845](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L838-L845)
**BR73625** — If level generation fails to place the amulet altar on the amulet's floor, the game places an amulet somewhere on that floor as a fallback.
[RogueMain.c:741-765](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L741-L765)
**BR63668** — On a greater victory the amulet is displayed as "The Birthright of Yendor".
[RogueMain.c:1300-1305](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1300-L1305)
**BR10314** — There are two victory flavors: a normal escape and a greater "transcendence" victory.
[RogueMain.c:1240](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1240)
[RogueMain.c:1261-1280](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1261-L1280)
**BR50899** — On the normal victory the ending text says the player escaped from the Dungeons of Doom, while on the greater victory it says the player transcended them.
[RogueMain.c:1261-1280](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1261-L1280)
**BR99222** — A victory screen tallies the carried gold plus the value of every carried item to produce a total treasure value, and the amulet's value is doubled on the greater victory.
[RogueMain.c:1290-1327](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1290-L1327)
[RogueMain.c:1300-1306](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1300-L1306)
**BR44153** — The victory high-score description records whether the player escaped or mastered the dungeon and reports lumenstones carried as none, one, or a count (e.g. "with 3 lumenstones").
[RogueMain.c:1349-1356](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1349-L1356)
**BR87073** — A victory saves a high score only when not in wizard mode and not during playback.
[RogueMain.c:1364-1368](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1364-L1368)
**BR71839** — After a victory the game displays the high-score list.
[RogueMain.c:1379-1381](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1379-L1381)
**BR36967** — When the player dies, the run ends.
[RogueMain.c:1077-1079](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1077-L1079)
[RogueMain.c:1236](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1236)
**BR39547** — On death the player's displayed health is emptied.
[RogueMain.c:1103-1105](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1103-L1105)
**BR90833** — The death message states the killer and depth as "Killed by a/an X on depth N" (or a custom phrase followed by "on depth N").
[RogueMain.c:1157-1162](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1157-L1162)
**BR76612** — On death the game shows "You die..." and waits for acknowledgment; pressing the inventory key identifies every carried item first.
[RogueMain.c:1098-1127](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1098-L1127)
**BR54571** — A second death call for a player already marked as dying is ignored.
[RogueMain.c:1072-1075](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1072-L1075)
**BR56397** — If the player quits, the game ends as a game over but is not treated as a death; the ending screen is blanked instead of fading out from the player's body.
[RogueMain.c:1087-1092](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1087-L1092)
[RogueMain.c:1150-1151](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1150-L1151)
**BR85743** — The game-over screen appends "amulet in hand" to the death description when the amulet is carried.
[RogueMain.c:1182-1185](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1182-L1185)
**BR32560** — Achievements earned during the run are listed on both the death screen (only those that differ from their default) and the victory achievements screen.
[RogueMain.c:1194-1202](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1194-L1202)
[RogueMain.c:1337-1347](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1337-L1347)
**BR70708** — On death, every gem in the pack counts as 500 gold, and that amount is added to the player's accumulated gold to form the score.
[RogueMain.c:1164-1172](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1164-L1172)
**BR83040** — The death high-score text reports the score as gold, phrased "with treasure worth N gold" when gems were held and "with N gold" otherwise, only when the score is greater than zero.
[RogueMain.c:1177-1181](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1177-L1181)
**BR74433** — The high-score entry saved on death describes only the killer and depth, omitting the treasure amount.
[RogueMain.c:1186-1188](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1186-L1188)
**BR86443** — In easy mode the final score is divided by ten, both on death and on victory.
[RogueMain.c:1174-1176](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1174-L1176)
[RogueMain.c:1360-1362](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1360-L1362)
**BR44882** — When the player dies and the run is not a playback, the score is saved to the high-score list and the high scores are displayed.
[RogueMain.c:1211-1213](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1211-L1213)
**BR32177** — The run history records a death as "Died" with the killer's name, a quit as "Quit", and a victory with the victory verb.
[RogueMain.c:1232-1234](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1232-L1234)
[RogueMain.c:1398-1400](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1398-L1400)
**BR48729** — Both endings mark the game as having ended with a success exit status.
[RogueMain.c:1236-1237](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1236-L1237)
[RogueMain.c:1402-1403](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1402-L1403)
**BR80910** — Easy mode is enabled from an in-game confirmation prompt.
[RogueMain.c:1411-1415](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1411-L1415)
**BR88010** — While easy mode is active the player takes only 20% as much damage.
[RogueMain.c:1420](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1420)
**BR90722** — Enabling easy mode announces that the player will suffer a 90% income tax.
[RogueMain.c:1421](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1421)
**BR39962** — Entering easy mode changes the player's displayed character to an ampersand and refreshes the display.
[RogueMain.c:1415-1419](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1415-L1419)
**BR99070** — If easy mode is already active, attempting to enable it again only prints a message and changes nothing.
[RogueMain.c:1407-1410](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/RogueMain.c#L1407-L1410)
**BR89701** — Wizard mode allows the player to create an arbitrary item by choosing its category and kind, and then adding it to the pack.
[Wizard.c:439-507](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L439-L507)
**BR11963** — Creating gold in wizard mode adds its quantity directly to the player's gold total instead of creating a pack item.
[Wizard.c:479-486](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L479-L486)
**BR36714** — Created weapons and armor can be given a runic; slaying and immunity runics additionally require choosing a target monster class, and choosing no vorpal target removes the runic.
[Wizard.c:106-173](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L106-L173)
**BR79883** — Created item enchantment levels are bounded by category: wands default to their charge count, charms at least 1, staves at least 2, rings and non-runic equipment from -3, bad runics only from -3 up to -1, and good runics at least 1.
[Wizard.c:198-263](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L198-L263)
**BR88672** — An item created in wizard mode is not cursed unless its enchantment is negative.
[Wizard.c:468-470](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L468-L470)
[Wizard.c:260-262](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L260-L262)
**BR64450** — A wizard-created item can be identified immediately on request.
[Wizard.c:496-500](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L496-L500)
**BR51895** — The wizard creation menu first offers a choice between creating an item and creating a monster.
[Wizard.c:509-523](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L509-L523)
**BR22777** — Wizard item creation refuses to start when the pack is full.
[Wizard.c:448-451](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L448-L451)
**BR29653** — A wizard-created item is added to the pack and announced by name.
[Wizard.c:502-506](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L502-L506)
**BR34341** — A wizard-created gem records the current depth as its origin.
[Wizard.c:475-477](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L475-L477)
**BR27330** — Wizard mode allows the player to create an arbitrary monster from the catalog, excluding the lich and the phoenix.
[Wizard.c:342-365](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L342-L365)
[Wizard.c:322-328](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L322-L328)
**BR79970** — The wizard monster catalog is sorted alphabetically and presented in pages of 24.
[Wizard.c:319-340](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L319-L340)
**BR16066** — A wizard-created monster is offered a mutation unless it can never be mutated.
[Wizard.c:367-370](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L367-L370)
**BR24559** — A wizard-created monster must be placed on a visible, valid location (an open spot, or a wall for a turret), and an invalid choice discards it.
[Wizard.c:371-400](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L371-L400)
**BR99923** — After a wizard-created monster is placed, the player is prompted to make it an ally.
[Wizard.c:416-426](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L416-L426)
**BR26114** — Placing a wizard-created monster where one already stands destroys the existing monster.
[Wizard.c:402-407](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/Wizard.c#L402-L407)

Fix wand of plenty behaviour. Since CE 1.8, it incorrectly set the cloned
monsters' max health to half the target's _current_ health, instead of half its
max health. Now the max health is halved properly and the current health is
unchanged, but capped to the new max.

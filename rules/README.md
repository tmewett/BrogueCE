# Brogue — Natural-Language Rules

This directory captures Brogue CE's internal game logic as natural-language rules. Each rule
describes an in-game behavior in one or two sentences — without naming data structures, functions,
or modules — and cites the source lines where that behavior is implemented.

## Reference format

References are pinned to a specific commit so they cannot go stale. All links point at commit
`104a20fedbe7d9360d2dfc8ff665571694212ad8` (`master` at the time of writing):

```
https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8/src/brogue/<file>#L<start>-L<end>
```

Only files under `src/brogue/` are cited.

## Conventions

- **One or two sentences** per rule, describing observable in-game behavior.
- **No code identifiers**: no function names, struct fields, enum constants, or module names in the
  rule text. (References are allowed to name the file, since that is only the citation.)
- **Exact numbers** are included where the game uses them (e.g. "triple damage", "500 gold",
  "half the usual injury").
- Each rule has a globally unique ID of the form `BR#####` (for example `BR39771`).
- A rule may carry multiple citations when the behavior spans several code sites.
- Citations are written as `[File.c:start-end](permalink)`.

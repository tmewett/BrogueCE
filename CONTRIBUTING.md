Contributing
============

For general help on using GitHub to contribute to a project, check out the
[GitHub.com Help][1].

We have a Discord server for discussion; feel free to join it here:
https://discord.gg/8pxE4j8.

## Testing

The *master* branch represents the latest development snapshot, and should be
considered ready for play-testing at all times. If you're a Brogue player, you
can help out by playing the latest *master* and letting us know of any bugs you
run into! Just follow the build instructions (BUILD.md).

## Code

When submitting patches or opening pull requests to Brogue CE, please
attempt to meet the follow guidelines. To avoid wasted work, I recommend
first discussing with us the proposed changes on the Discord or by opening
an issue report.

### Branches and versions

Brogue CE version numbers follow 1.MINOR.PATCH. Essentially, patch-level
releases don't change the gameplay experience in any way--this is to avoid
breaking saves and replays. Minor-point releases may do so.

The are two long-term branches which you should base PRs on:

* *master* is for gameplay changes for the next minor-point release
* *release* is for bugfixes and other non-gameplay changes, for the next patch
  release. It is merged into *master* periodically.

Any other public branches may be rebased and force-pushed at any time, so please
be careful when branching from them.

### Style

- Use 4 space of indentation
- Be consistent with formatting (pay attention to whitespace between brackets,
  commas, etc.)
- Try to follow the style of existing code
- Use braces for control structures on multiple lines. I.e. instead of

  ```c
  if (cond)
      action();
  ```

  do

  ```c
  if (cond) {
      action();
  }
  ```

  or if you really want,

  ```c
  if (cond) action();
  ```

### Commits

I find a clear history to be very beneficial to work with, so I have some
guidelines for managing the commits in your branch:

- Try to make each commit represent a single, easily-describable change
- Follow [the seven rules of a great commit message][3]
- Keep your commit history clean of errors, revertions and fixes to your own
  changes
  - If your change works as a single commit, one way to do this is to
    continually amend your commit as you work on it
  - Alternatively, you can make separate commits and then squash them down to
    one using a soft reset or an interactive rebase
  - If you want multiple commits, then look into fixups and auto-squashing

### Change files

When making user-facing changes, please add a non-technical description of each
change to a Markdown (.md) file in `changes/`. These files are collated to
create the release notes.

If the change is from one commit, include this file in it. For a branch of
multiple commits, add it in a separate commit.

[1]: https://help.github.com/en/github
[3]: https://chris.beams.io/posts/git-commit/#seven-rules

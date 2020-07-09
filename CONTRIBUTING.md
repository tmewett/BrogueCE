Contributing
============

For general help on using GitHub to contribute to a project, check out the
[GitHub.com Help][1].

We have a Discord server for discussion -- you can join it with invite code
`8pxE4j8`.

## Testing

The *master* branch represents the latest development snapshot, and should be
considered ready for play-testing at all times. If you're a Brogue player, you
can help out by playing the latest *master* and letting us know of any bugs you
run into!

## Code

When submitting patches or opening pull requests to Brogue CE, please
attempt to meet the follow guidelines. To avoid wasted work, I recommend
first discussing with us the proposed changes on the Discord or by opening
an issue report.

### Branches and versions

Brogue CE version numbers follow 1.MINOR.PATCH. Essentially, patch-level
releases don't change the gameplay experience in any way--this is to avoid
breaking saves and replays. Minor-point releases may do so.

The are two long-term branches:

* *master* is for gameplay changes for the next minor-point release
* *release* is for bugfixes and other non-gameplay changes, for the next patch
  release. It is merged into *master* periodically.

Any other public branches may be rebased and force-pushed at any time, so please
be careful when branching from them.

### Style

I am mostly subjective here but not particularly fussy. There is no official
code style (besides 4 spaces indentation), but:

- Be consistent with formatting
- Try to follow the style of existing code

### Commits

I am quite picky about commits, as I find a good history to be very beneficial.

- Try to make each commit represent a single, easily-describable change
- Keep your commit history clean of errors and fixups (e.g. by [rewriting
history][2])
- Follow [the seven rules of a great commit message][3]

When making user-facing changes, please add a non-technical description of each
change to a Markdown (.md) file in `changes/`. These files are collated to
create the release notes. If the change is from one commit, include this file in
it. For a branch of multiple commits, add it in a separate commit.

[1]: https://help.github.com/en/github
[2]: https://git-scm.com/book/en/v2/Git-Tools-Rewriting-History
[3]: https://chris.beams.io/posts/git-commit/#seven-rules

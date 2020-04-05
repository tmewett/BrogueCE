Contributing
============

For general help on using GitHub to contribute to a project, check out the
[GitHub.com Help][1].

We have a Discord server for discussion -- you can join it with invite code
`8pxE4j8`.

When submitting patches or opening pull requests to Brogue CE, please
attempt to meet the follow guidelines. To avoid wasted work, I recommend
first discussing with us the proposed changes on the Discord or by opening
an issue report.

## Code

I am mostly subjective here but not particularly fussy. There is no official
code style (besides 4 spaces indentation), but:

- Be consistent with formatting
- Try to follow the style of existing code

## Commits

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

Contributing
============

For general help on using GitHub to contribute to a project, check out the
[GitHub.com help].

We have a Discord server for discussion; feel free to join it here:
https://discord.gg/8pxE4j8.

If you are having an issue with online brogue, that is a separate repository,
found here: https://github.com/flend/web-brogue

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

Brogue CE version numbers follow 1.MINOR.PATCH. Patch-level releases don't
change the gameplay experience very much, but minor-point releases may do.

There are two long-term branches which you should base PRs on:

* *master* is for gameplay changes for the next minor-point release
* *release* is for bug fixes and other non-gameplay changes, for the next patch
  release. It is merged into *master* periodically.

### Commits

I find a clear Git history very beneficial to work with, so I care quite a bit
about how commits are presented in PRs.

Please read my [tips for using Git effectively][Git guidance], which can be
considered guidelines for contributing to this project.

### Change files

When making user-facing changes, please add a non-technical description of each
change to a Markdown (.md) file in `changes/`. These files are collated to
create the release notes.

If the change is from one commit, include this file in it. For a branch of
multiple commits, add it in a separate commit.

### Style

- Use 4 spaces of indentation
- Be consistent with formatting (pay attention to whitespace between brackets,
  commas, etc.) and try to follow how existing code looks
- Declare functions and variables local to a file as `static`
- Prefer `int` over `short` in new integer declarations
- Use braces for control structures on multiple lines

  ```c
  // no
  if (cond)
      action();

  // yes
  if (cond) {
      action();
  }

  // acceptable
  if (cond) action();
  ```

- When writing bracketed lists over multiple lines, wrap it naturally and don't
  align to the open bracket (this includes declarations)

  ```c
  // yes
  some_function(
      a, b,
      c,
      d
  );

  // no
  some_function(a, b,
                c,
                d);
  ```

- When writing multi-line if/while conditions, use at least the same indentation
  as the body. It can be clearer to over-indent to separate it

  ```c
  // same indent is ok, but...
  while ((A && B)
      || C) {
      ...
  }

  // over-indent can be clearer
  while ((A && B)
          || C) {
      ...
  }

  // a gap works too
  while ((A && B)
      || C) {

      ...
  }
  ```

[GitHub.com help]: https://docs.github.com/en/free-pro-team@latest/github
[Git guidance]: http://www.collider.in/tom/git-guidance.html

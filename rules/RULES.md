# Brogue — Natural-Language Rules

The complete rule set lives in the [`rules/`](rules/README.md) directory. Start with
[`rules/README.md`](rules/README.md) for the reference format, conventions, and an index of all
1191 rules by subsystem.

Each rule describes an in-game behavior in one or two sentences, without naming data structures,
functions, or modules, and cites the source lines where that behavior is implemented. All references
are pinned to commit
[`104a20fedbe7d9360d2dfc8ff665571694212ad8`](https://github.com/tmewett/BrogueCE/blob/104a20fedbe7d9360d2dfc8ff665571694212ad8)
so they cannot go stale.

The following are prompts that can be used to refine the existing rules and test set using AI tools:

----------------------------------
--- VERIFY A RULE MEETS REQUIREMENTS ---
----------------------------------

OBJECTIVE

Verify that rule IDs <<LIST OF A FEW RULE IDs>> meet the following requirements.


REQUIREMENTS

Rules for the game Brogue are collated into files in the directory "rules/*".

Rules MUST describe in-game behavior in one or two natural-language sentences.

Rules MUST NOT name data structures, functions, or modules from the source code, but rather use in-game terminology for their descriptions.

Rules MUST cite source code lines where that behavior is implemented in the game code.

Rules MUST describe a single, simple, observable behavior.

Rules MUST be assigned an ID beginning with "BR" followed by 5 random digits (such as "BR39812").

Rule IDs MUST be unique across all rule files.


----------------------------------
--- VERIFY A RULE HAS TESTS ---
----------------------------------

OBJECTIVE

Verify that rule IDs <<LIST OF A FEW RULE IDs>> meet the following requirements.


REQUIREMENTS

Rules MUST have one or more associated tests in test/*

Rule IDs without tests MUST be documented in test/RuleMissingTest.txt

Tests for a rule MUST include the rule ID in the test name.


----------------------------------
--- VERIFY A TEST MEETS REQUIREMENTS ---
----------------------------------

OBJECTIVE

Verify that tests for rule IDs <<LIST OF A FEW RULE IDs>> meet the following requirements.


REQUIREMENTS

Tests MUST be associated with a rule found in rules/*

Tests without associated rules MUST be documented in test/TestMissingRule.txt

If a rule has more than one test, they MUST include a positive and negative test for the rule.

Tests SHOULD NOT reference game engine components or data structures.  Use TestFramework functions which hide engine details instead.

Additional functions SHOULD be added to the TestFramework when tests need to interact with game engine components.

Tests MUST be falsifiable.  If a test succeeds when its test assertion is negated, it is not a valid test.


----------------------------------
--- FIND MISSING RULES ---
----------------------------------

OBJECTIVE

Verify that the in-game behaviors described by source code at src/brogue/<<FILENAME>>.c lines <<LINE-NUMBERS>> is captured by one or more rules in rules/*


REQUIREMENTS

Each observable in-game behavior MUST have a rule describing it in rules/*

Rules MUST describe in-game behavior in one or two natural-language sentences.

Each rule MUST cite the source code where the in-game behavior is implemented.

Rules MUST be assigned an ID beginning with "BR" followed by 5 random digits (such as "BR39812").

Rule IDs MUST be unique across all rule files.


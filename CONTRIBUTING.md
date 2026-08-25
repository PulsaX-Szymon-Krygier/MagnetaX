# Contributing to MagnetaX

If you found a bug, have an idea, or want to improve something, feel free to get involved.

**Please note that** MagnetaX is still in an early and active stage of development. The engine already has a growing set of working systems, but many parts of the API, tooling, architecture, and workflow are still being shaped as the project grows.

Because of that, some conventions and implementation details may change over time. Contributions should follow the current direction of the project and avoid treating unfinished parts as final contracts.

The main thing is to keep changes clear, focused, and easy to review.

You do not need to open an issue for every tiny fix. If you are planning something bigger, though, talk about it first. It is better to agree on the direction before writing a lot of code.

## Before you start

Please read:

- the [MagnetaX Coding Style](CODING_STYLE.md)
- the [AI Usage Policy](AI_POLICY.md)

If your change adds a new dependency or third-party code, check the license first.

## Small changes

Small and obvious fixes can go straight to a pull request.

For example:

- simple bug fixes
- warnings
- small CMake fixes
- typos
- small documentation fixes
- obvious platform fixes
- tests for existing behavior

If the bug is not obvious or the fix could affect more than one part of the engine, opening an issue first is usually a better idea.

## Bigger changes

Open an issue first if you want to work on something like:

- a new feature
- a public API change
- an architecture change
- a new subsystem
- a new dependency
- a larger refactor
- serialization or file format changes
- something that affects multiple engine systems

This does not need to become a huge formal proposal.

Just explain:

- what you want to change
- what problem it solves
- why you think it makes sense
- roughly how you would approach it

Once the direction looks good, you can start working on it.

If you are not sure whether your change needs an issue first, open one before writing a lot of code.

## Issues

MagnetaX currently uses a small set of issue types:

- `bug` for something that does not work correctly
- `enhancement` for improving something that already exists
- `feature` for new functionality
- `proposal` for bigger ideas that should be discussed before implementation

Keep issues focused on the actual problem or idea.

For bugs, include enough information to reproduce the problem if possible.

Useful details can include:

- what happened
- what you expected instead
- reproduction steps
- platform and compiler
- GPU and driver for graphics issues
- validation layer, sanitizer, or error output
- screenshots or videos for visual problems

## Pull requests

Try to keep one clear purpose per PR.

Good:

```text
Fix dangling reference in Entity::GetChildren
```

Not so good:

```text
Fix GetChildren
Rename Scene methods
Reformat ComponentPool
Change CMake
Add serialization helper
```

If two changes can be reviewed separately, they probably should be separate PRs.

Before opening a PR:

- review your own diff
- remove unrelated changes
- follow `CODING_STYLE.md`
- build the affected targets
- test what you changed
- make sure you understand everything you are submitting
- follow `AI_POLICY.md` if AI directly helped create part of the contribution

## PR description

Small changes do not need an essay.

For bigger changes, try to cover four things:

### What

What changed?

### Why

Why was the change needed?

### How

How does the solution work?

### Testing

How did you check it?

For rendering or editor changes, screenshots or short videos are useful when they actually help show the result.

For performance changes, include some numbers if possible.

## Draft pull requests

Draft PRs are useful for bigger work.

If the general idea was already discussed but you want feedback before finishing everything, open a draft PR.

It is much easier to fix a bad direction early than after a few thousand lines of code.

## Code review

Review is part of the process.

A change can work perfectly and still not fit MagnetaX.

For example, it may:

- go against the current architecture
- add complexity we do not need
- introduce an abstraction without a real use case
- change public API in a direction we do not want
- mix too many unrelated things into one PR

If a maintainer asks for changes, keep the discussion focused on the code and the problem being solved.

If something is unclear, ask.

## AI-assisted contributions

AI can be used as a development tool, but contributions must follow the [AI Usage Policy](AI_POLICY.md).

You are still responsible for understanding, reviewing, testing, and explaining everything you submit.

Prompt-to-PR and autonomous vibe-coded contributions are not accepted.

## Third-party dependencies

Do not add a dependency just because it saves a few lines of code.

Before adding one, think about:

- why we need it
- whether the engine can reasonably do without it
- runtime cost
- maintenance cost
- platform support
- license compatibility
- whether it leaks into public engine APIs

New dependencies should be discussed in an issue first.

Keep their original license and copyright notices.

## Keep changes focused

Do not mix unrelated cleanup into a feature or bug fix.

If the code you are touching needs a small cleanup to make the actual change better, that is fine.

Just do not turn every PR into a reason to rename, reformat, or redesign half of the engine.

## In short

Small obvious fix? Open a PR.

Bigger change? Open an issue first.

Keep PRs focused, follow the project style, test what you change, and make sure you actually understand the code you submit.
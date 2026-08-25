# AI Usage Policy

Responsible AI usage is welcome in MagnetaX development.

AI can be a very useful tool. You can use it for research, learning, debugging, code review, exploring ideas, generating boilerplate, writing tests, helping with documentation, or assisting with implementation.

The rule is simple:

**AI should help you do the work, not do the contribution for you.**

If you open a Pull Request, you are responsible for everything in it.

## Human ownership

You should understand the code you submit.

That means knowing:

- what it does
- why it works
- why you chose that solution
- where it belongs in the engine
- what other parts of the project it may affect

You should also be able to explain your decisions during review.

"The AI generated it" is not an explanation.

If a contribution is clearly submitted without real human understanding, it will be closed instead of being fixed or rewritten by maintainers.

## What is fine

Using AI as a development tool is completely fine.

For example:

- researching APIs, algorithms, standards, or possible solutions
- learning how existing MagnetaX code works
- debugging
- looking for possible bugs
- discussing architecture
- comparing different solutions
- generating repetitive boilerplate
- suggesting small implementation parts
- drafting tests
- helping with documentation
- helping with CMake, scripts, or tooling

Generated code is not automatically a problem.

If AI generates something useful and you understand it, review it, adapt it to MagnetaX, test it, and take responsibility for it, that's fine.

Treat generated output as something to work with, not as a finished patch ready to push.

## What is not fine

MagnetaX does not accept prompt-to-PR or vibe-coded contributions.

Don't give an issue to an AI agent, let it change a lot of files, and then submit the result without being deeply involved yourself.

Don't submit code when:

- you don't fully understand what it does
- an AI agent made the important design or architecture decisions for you
- you accepted the result mainly because it compiles or passes tests
- large generated changes were not carefully reviewed
- an AI-generated refactor has no clear reason behind it
- the PR contains unnecessary helpers, abstractions, formatting changes, or unrelated code
- you can't explain why the implementation belongs where it does
- AI was used instead of actually reading and understanding the codebase

Autonomous agents must not be the real author of a MagnetaX contribution.

You stay in control from the idea, through implementation, to testing and review.

## Project standards always apply

AI-assisted code follows exactly the same rules as everything else.

It **must** follow:

- the [MagnetaX Coding Style](CODING_STYLE.md)
- existing naming and formatting rules
- current architecture and subsystem boundaries
- ownership and lifetime rules
- build and testing requirements
- licensing and contribution rules

AI is not an excuse for strange formatting, unnecessary abstractions, overcomplicated code, or generated noise.

Before opening a PR, clean the code up and make sure it actually looks like MagnetaX code.

## Testing and verification

Test what you submit.

A successful build is good. Passing tests are good. Neither one proves that the implementation is correct or makes sense.

Depending on the change, you may need to:

- build the affected targets
- run the affected code
- check failure cases
- test platform-specific behavior
- use validation layers or sanitizers
- check ownership and lifetime behavior
- make sure the change still fits the current architecture

You decide when your contribution is ready for review. The AI tool doesn't.

## Code review

Take part in your own review.

AI can help you investigate a maintainer's question or understand something you missed. That's fine.

What we don't want is a loop where review comments are copied into an AI tool and the generated answers or patches are sent back without much thought.

If a maintainer asks why you made a decision, you should be able to explain it yourself.

If you can't reasonably explain the contribution you submitted, it will be rejected.

## Disclosure

You don't need to disclose normal AI-assisted research, learning, debugging, code review, architecture discussion, or general technical help.

If AI directly generated or heavily rewrote code, tests, documentation, or other content that became an important part of a contribution to MagnetaX, that assistance **must be disclosed** in the PR description.

A short note is enough:

> AI assistance: ChatGPT/Claude/... was used to help draft boilerplate and parts of the implementation. The submitted changes were reviewed, adapted, understood, and tested by the contributor.

This is not a penalty.

Responsible AI-assisted contributions are welcome.

## Licensing

You are responsible for making sure you have the right to contribute everything you submit.

Don't submit generated material that copies third-party code, documentation, assets, or other copyrighted content under incompatible terms.

Using AI does not move copyright or licensing responsibility away from you.

## Scope

This policy only applies to contributions submitted to the official MagnetaX project.

What you do in your own game, application, private fork, experiment, or other project built with MagnetaX is your business.

Use AI as much or as little as you want there.

These rules only define what we expect when something is contributed back to MagnetaX.

## In short

AI is welcome as a tool.

Use it to learn faster, debug problems, explore ideas, automate repetitive work, generate boilerplate, or help you write better code.

Just don't hand your contribution over to an AI agent and send the result to us.

**AI may assist the work. A human must own the contribution.**

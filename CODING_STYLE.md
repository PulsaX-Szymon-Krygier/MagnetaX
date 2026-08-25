# MagnetaX Coding Style

**Last updated:** 25.08.2026

This document describes the coding style and standards used in MagnetaX code.

The goal is simple: keep the code consistent, readable, and easy to work with as the engine grows.

Use common sense. This is not meant to turn development into a formatting exercise. If a rule makes a piece of code harder to read, prefer the clearer version.

This document may change as MagnetaX grows and the codebase gives us better reasons to adjust a rule.

Third-party and generated code are not covered by this style.

## Basics

### Indentation

C++ and Objective-C++ code use 4 spaces.

Do not use tabs.

CMake files use 2 spaces to match the existing build scripts.

### Braces

MagnetaX uses Allman-style braces.

```cpp
if (condition)
{
    Foo();
}

class Foo
{
};

void Foo()
{
}
```

A short `if` may stay on one line when it contains one simple statement and the whole thing is easy to read.

This is mainly useful for guard clauses and simple control flow.

```cpp
if (!value) return false;
if (!item) continue;
if (count == 0) return;
```

If the body contains more than one statement, use braces.

```cpp
if (!Foo())
{
    Bar();
    return false;
}
```

If the body needs its own line, use braces as well.

Avoid:

```cpp
if (condition)
    Foo();
```

The simple rule is: either keep the complete `if` on one clear line, or use a normal braced body.

### Line length and wrapping

Try to keep lines below roughly **120 to 140 characters at most**.

This is an approximate maximum range, not a target and not a hard limit.

Nobody should be counting characters while writing normal code. Slightly longer lines are fine when breaking them would make the code harder to read.

At the same time, avoid huge 200 or 300 character lines.

When a line becomes too long, wrap it naturally.

Do not automatically put every argument on a separate line.

Prefer:

```cpp
bool Foo(FooDevice* device, FooAllocator* allocator, const FooCreateInfo& createInfo, const FooConfig& config,
    uint32 flags);
```

over:

```cpp
bool Foo(
    FooDevice* device,
    FooAllocator* allocator,
    const FooCreateInfo& createInfo,
    const FooConfig& config,
    uint32 flags);
```

For continuation lines, use one normal indentation level.

```cpp
Foo(arg1, arg2, arg3, arg4,
    arg5, arg6);
```

Do not align arguments with large amounts of spaces just to match the opening parenthesis.

The same rule applies to API calls, constructors, initializer lists, and long expressions.

### Pointer and reference style

Keep `*` and `&` next to the type.

```cpp
Foo* foo;
const Bar& bar;
```

Not:

```cpp
Foo *foo;
Bar &bar;
```

## Naming

### Types

Classes, structs, enums, aliases, and other named types use `PascalCase`.

```cpp
Foo
FooConfig
FooEnvironment
FooRenderData
FooBuilder
```

Do not use styles like:

```cpp
fooConfig
FOO_CONFIG
Fooconfig
```

### Functions and methods

Functions and methods also use `PascalCase`.

```cpp
Create()
Destroy()
GetValue()
FindSomething()
CalculateSomething()
```

### Variables and members

Local variables and class members use `camelCase`.

```cpp
device
imageView
physicalDevice
cameraPosition
viewProj
```

Do not add prefixes such as `m_` to members.

### Parameters

Parameters use `camelCase`.

If a parameter has the same name as a member, a leading underscore may be used to make the difference clear.

```cpp
class Foo
{
private:
    FooDevice* device = nullptr;

public:
    bool Create(FooDevice* _device);
};
```

Do not use a leading underscore when there is no naming conflict.

```cpp
void SetCount(uint32 count);
```

The main special case is PImpl, where `_impl` is the standard member name.

```cpp
struct FooImpl;
std::unique_ptr<FooImpl> _impl;
```

### Enum values

Enum values use `UPPER_SNAKE_CASE`.

```cpp
enum class FooState
{
    INVALID,
    LOADING,
    READY,
    FAILED
};
```

```cpp
FooState::READY
FooMode::READ_ONLY
```

### Acronyms and abbreviations

Well-known technical acronyms keep their normal form.

```cpp
UIRenderData
FXAAConfig
WinAPIWindow
X11Window
IOUtils
GetEntityByID
```

For variables, keep the normal `camelCase` structure.

```cpp
entityID
typeID
apiVersion
uiRenderData
```

Common and obvious abbreviations are fine when they improve readability.

```cpp
graphicsDevice
gfxDevice
```

Avoid inventing short names that make the code harder to understand.

```cpp
graDev
```

When in doubt, prefer the clearer name.

### Common type suffixes

Use the existing MagnetaX naming meaning consistently.

```text
*CreateInfo     input used when creating something
*Config         configuration
*State          current runtime state
*Properties     properties of an object or resource
*Capabilities   supported features or limits
*Info           general read-only information
```

Do not use these suffixes only because they sound appropriate. The name should match what the type actually represents.

## Files and includes

### File names

A file should normally match the main type it contains.

```text
Foo.h
Foo.cpp

Bar.h
Bar.cpp
```

Headers use:

```cpp
#pragma once
```

Do not replace this with manual include guards unless there is a real reason.

### Include order

Keep includes in a predictable order so files are easier to scan and dependencies are easier to manage.

The exact order is a MagnetaX project convention. It is mainly about keeping files readable and making dependency direction easier to see.

Use the following general order:

1. MagnetaX Public/Internal headers using `<MX/...>`
2. Cross-subsystem source/internal headers such as `<Graphics/...>`
3. Same-folder headers using `"Foo.h"`
4. Close relative headers such as `"../Foo.h"`
5. Platform, API, and third-party headers
6. Standard library headers

Example:

```cpp
#include <MX/Core/CoreMinimal.h>
#include <MX/Foo/FooConfig.h>
#include <Graphics/FooRenderData.h>
#include "Foo.h"
#include "../Bar.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
```

Do not add empty lines between normal include groups.

Conditional platform includes may naturally form their own block.

```cpp
#if MX_PLATFORM_WINDOWS
#include <Windows.h>
#endif
```

### CoreMinimal

The lowest self-contained MagnetaX header should include the appropriate `CoreMinimal.h` according to the current project convention.

Do not rely on accidental transitive includes from unrelated headers.

## MagnetaX prefixes

### Macros

MagnetaX macros use the `MX_` prefix.

When a macro belongs to a specific module or area, include that module in the name.

```cpp
MX_PLATFORM_WINDOWS
MX_PLATFORM_APPLE

MX_GRAPHICS_VULKAN
MX_GRAPHICS_VULKAN_DEBUG
MX_GRAPHICS_VULKAN_MIN_VERSION
```

Prefer:

```text
MX_<MODULE>_<NAME>
```

for globally visible MagnetaX macros.

### Global constants

Globally visible MagnetaX constants also use an `MX_` prefix.

Use a module prefix when it helps make ownership clear.

```cpp
constexpr float32 MX_MATH_PI = 3.14f;
constexpr uint32 MX_GRAPHICS_SOMETHING = 1024;
```

The prefix matters when the symbol can become visible outside a single implementation file.

### File-local constants

File-local constants do not need an `MX_` prefix.

Keep them in an anonymous namespace inside the `.cpp` file.

```cpp
namespace
{
    constexpr uint32 MAX_COUNT = 64;
}
```

The same rule applies to small helpers that only belong to one implementation file.

## Namespaces

MagnetaX does not use named C++ namespaces as the default way to organize the engine.

The folder structure, module boundaries, and type names already provide the main organization.

Named namespaces may still be used for specific internal implementation details where they solve a real problem, for example entry-point or detail-level code.

Do not introduce namespaces only to group unrelated APIs.

### Anonymous namespaces

Anonymous namespaces are fine in `.cpp` files for file-local helpers, constants, and implementation details.

```cpp
namespace
{
    constexpr uint32 DEFAULT_COUNT = 4;

    bool IsValidFoo(...)
    {
        ...
    }
}
```

Prefer this for file-local implementation details instead of exposing helpers outside the file.

## Platform and API naming

If a type is tied to a specific platform or API, the name should make that clear.

Use full, readable prefixes.

```cpp
VulkanDevice
VulkanImage
VulkanPipeline

WinAPIWindow
WinAPIInput

X11Window
X11Input

CocoaWindow
MetalVulkanSurface
```

Avoid shortening important platform or API names just to save a few characters.

Raw API types keep their original names.

```cpp
VkDevice
VkImage
VkSampler

HWND
HINSTANCE
```

MagnetaX wrappers use MagnetaX names.

```cpp
VulkanDevice
VulkanImage
VulkanTexture
```

This should make the boundary obvious when reading the code.

## Casts

Normal C++ casts are welcome where they make the intent clearer.

```cpp
static_cast<uint32>(value)
reinterpret_cast<void*>(ptr)
```

C-style casts are also allowed when they are natural and clear in the surrounding code.

```cpp
(uint32)value
```

Do not perform mechanical cast cleanup only to replace one valid style with another.

Platform code and Objective-C++ may naturally use casts required by their APIs or language integration.

## Comments and documentation

### Comments

Comments are welcome when they help.

Use them for things such as:

- explaining why something is done
- non-obvious limitations
- important ownership or lifetime rules
- platform or API workarounds
- unusual behavior that would otherwise be easy to misunderstand
- useful development notes

Comments can be casual when that fits the codebase.

Do not comment every obvious line or write an essay around simple code.

Bad:

```cpp
// Increment the counter by one
counter++;
```

Better:

```cpp
// Keep one slot free because index 0 is reserved for the invalid handle
```

Educational or temporary comments are fine during development, but clean up excessive notes before merging them into the public codebase.

### Public API documentation

Not every public method needs a Doxygen block.

Document public APIs when there is something useful to explain, such as ownership, lifetime, valid ranges, side effects, or behavior that is not obvious from the API itself.

Do not add documentation only to repeat the function name in sentence form.

## CMake

CMake-specific MagnetaX build variables use the `MXB_` prefix.

```cmake
MXB_DIR_ROOT
MXB_DIR_ENGINE
MXB_PLATFORM_WINDOWS
MXB_SOURCE_ENGINE_PUBLIC
```

CMake helper functions follow the existing `MXB...` naming style.

```cmake
MXBAddShaders(...)
```

CMake uses 2-space indentation.

Keep target and dependency naming consistent with the existing project.

```text
MXEngine
MXGame
MXEditor

MX_ThirdParty_<name>
```

## Third-party and generated code

Do not reformat third-party code to match MagnetaX.

This includes dependencies under `ThirdParty/` and any code generated by external tools.

Their own style and license rules take priority.

The same applies to generated shader headers and other generated build artifacts.

## Formatting tools

`CODING_STYLE.md` is the source of truth for MagnetaX style.

A `.clang-format` file may be added as a helper, but it should support the project style rather than define a different one.

Do not run broad automatic formatting passes that create unrelated changes across the codebase.

Formatting changes should stay focused and intentional.

## Keep changes focused

Do not mix unrelated formatting, renaming, cleanup, or refactoring into a feature or bug fix.

If a piece of code needs cleanup because the current change touches it, keep the cleanup local and useful.

Avoid large style-only diffs unless the purpose of the change is specifically to bring existing code into the agreed MagnetaX standard.

## In short

Keep the code simple, consistent, and easy to read.

Use the existing style when it already works. Do not invent new conventions locally.

Prefer clear names over clever abbreviations, avoid unnecessary abstractions, and keep platform or API-specific code obvious from its names.

The style exists to make MagnetaX easier to work on, not to make developers fight the formatter.
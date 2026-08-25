# Third-Party Notices

MagnetaX uses some third-party open source libraries.

Each dependency keeps its original license and copyright notices. The license files included with the dependencies are the authoritative source for their terms.

## Dear ImGui

**Project:** Dear ImGui  
**Author:** Omar Cornut and contributors  
**License:** MIT License  
**Location:** `MagnetaX/ThirdParty/imgui`

Dear ImGui is currently used for MagnetaX editor tooling.

See the original license included with the submodule:

```text
MagnetaX/ThirdParty/imgui/LICENSE.txt
```

Dear ImGui also includes or embeds some third-party components and assets under their own permissive terms. Their original notices remain part of the ImGui submodule.

## stb

**Project:** stb  
**Author:** Sean Barrett and contributors  
**License:** MIT License or Public Domain  
**Location:** `MagnetaX/ThirdParty/stb`

MagnetaX uses stb under the MIT license option.

See:

```text
MagnetaX/ThirdParty/stb/LICENSE
```

## tinyobjloader

**Project:** tinyobjloader  
**Author:** Syoyo Fujita and contributors  
**License:** MIT License  
**Location:** `MagnetaX/ThirdParty/tinyobjloader`

See:

```text
MagnetaX/ThirdParty/tinyobjloader/LICENSE
```

The tinyobjloader repository also contains some bundled third-party components with their own licenses, including `mapbox/earcut.hpp` under the ISC License. Their original license files remain included in the submodule.

## Adding new dependencies

When a new third-party dependency is added to MagnetaX:

- check that its license is compatible with the project
- keep its original license and copyright notices
- add it to this file
- keep the dependency separated from MagnetaX code where practical

This file only lists dependencies that are actually used by the project.

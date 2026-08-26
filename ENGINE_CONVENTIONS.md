# MagnetaX Engine Conventions

**Last updated:** 26.08.2026

This document describes the main technical conventions used by the engine.

The goal is simple: keep one predictable internal representation instead of letting every file format, graphics API, or platform bring its own rules into the whole codebase.

External formats and backends may use different conventions. When they do, conversion should normally happen at the boundary.

Only conventions that already exist in the project are documented here. More can be added when new systems actually need them.

## Coordinate system

The world uses a right-handed coordinate system.

```text
+X = right
+Y = up
-Z = forward
+Z = backward
```

The default forward direction is:

```text
(0, 0, -1)
```

Cameras and directional or spot lights use the same local forward direction.

The default up and right directions are:

```text
up = (0, 1, 0)
right = (1, 0, 0)
```

Positive axis-angle rotations follow the right-hand rule.

## Transforms

A transform contains:

```text
position
rotation
scale
```

The default transform is:

```text
position = (0, 0, 0)
rotation = identity
scale    = (1, 1, 1)
```

Local transform matrices are built as:

```text
Translation * Rotation * Scale
```

With the matrix/vector convention used by the engine, a point is therefore transformed in this order:

```text
scale
rotation
translation
```

Entity hierarchy is composed as:

```text
parentWorld * childLocal
```

so a child inherits the complete transform chain of its parents.

Negative or mirrored scale can reverse triangle winding.

Explicit handling for mirrored transforms is planned for a later transform/rendering pass. Until then, mirrored transforms should be used carefully.

## Rotations and angles

Transforms store rotations as quaternions.

Quaternion components use:

```text
x, y, z, w
```

The identity quaternion is:

```text
(0, 0, 0, 1)
```

Yaw, pitch and roll use:

```text
yaw   = rotation around Y
pitch = rotation around X
roll  = rotation around Z
```

`Quaternion::FromYawPitchRoll...()` composes them as:

```text
yaw * pitch * roll
```

Explicit degree and radian math functions are available where both forms are useful.

Scene-facing values that are easier to author in degrees use degrees.

Examples include:

```text
CameraComponent::fieldOfView
LightComponent::innerConeAngle
LightComponent::outerConeAngle
```

Math code converts them to radians where needed.

## Matrices

`Matrix4f` uses row-major storage in C++ memory.

The layout is:

```text
m00 m01 m02 m03
m10 m11 m12 m13
m20 m21 m22 m23
m30 m31 m32 m33
```

Mathematically, column vectors are used:

```text
result = matrix * vector
```

Translation is stored in:

```text
m03
m13
m23
```

Model matrices follow:

```text
Model = Translation * Rotation * Scale
```

Rendering follows:

```text
MVP = Projection * View * Model
```

Matrix storage order and mathematical multiplication convention are separate things. Row-major storage does not imply row-vector math.

GLSL matrices use their normal column-major representation.

The Vulkan backend transposes CPU `Matrix4f` values before uploading them to GLSL push constants or uniform buffers.

That conversion belongs to the graphics backend. CPU-side math should not change its representation only to match one graphics API.

## Camera and view space

Camera view space is right-handed and looks toward:

```text
-Z
```

The view matrix is the inverse of the camera world transform.

`CameraComponent::fieldOfView` is a vertical field of view expressed in degrees.

Current defaults are:

```text
field of view = 60 degrees
near plane = 0.1
far plane = 1000.0
```

These are defaults, not fixed engine limits.

## Projection and depth

Perspective and orthographic projection use Vulkan-compatible clip-space depth.

After projection and perspective division:

```text
X = -1 .. 1
Y = -1 .. 1
Z =  0 .. 1
```

Depth uses the normal, non-reversed convention:

```text
near plane = 0
far plane  = 1
```

Depth buffers are cleared to:

```text
1.0
```

and normal geometry uses:

```text
LESS
```

depth testing.

Reverse-Z is planned for a future depth-precision pass.

The projection matrices contain the Y conversion needed by the Vulkan rendering path while world space keeps:

```text
+Y = up
```

Backend-specific clip-space details should stay at the graphics boundary rather than leaking into the world-space convention.

## Mesh geometry

Scene meshes are rendered as indexed triangle lists.

The internal mesh vertex contains:

```text
position
normal
uv
```

Mesh indices use:

```text
uint32
```

Canonical mesh geometry follows:

```text
front face = counter-clockwise (CCW)
normals    = outward
```

when looking at the front of a surface.

Normal opaque geometry uses back-face culling.

The Vulkan geometry path therefore uses:

```text
cull mode = BACK
front face = COUNTER_CLOCKWISE
```

Geometry passes should agree on what the engine considers a front face.

If a pass intentionally needs front-face culling, it should select `FRONT` culling explicitly instead of changing the winding convention.

Fullscreen passes and UI may disable culling where it is not useful.

## Mesh import

External mesh files do not have to use the same conventions as the engine.

The importer is responsible for converting source data before the resulting `MeshAsset` reaches the renderer.

The OBJ importer:

```text
triangulates geometry
preserves authored positions and axes
loads normals and UVs when present
can generate normals when the OBJ contains none
```

Automatic OBJ handedness, up-axis, and forward-axis conversion is planned as the mesh import pipeline grows.

For now, OBJ geometry should be exported using axes compatible with the project coordinate system.

`MeshAsset::flipWinding` is an import option for source meshes whose triangle winding is reversed.

Conceptually:

```text
0, 1, 2

becomes

0, 2, 1
```

The option converts source geometry into the canonical winding.

It does not introduce a second internal winding convention and does not automatically invert normals supplied by the source file.

When normals are generated by the OBJ loader, they are calculated after the optional winding conversion so they match the final triangle orientation.

If more mesh import options become necessary later, they can be grouped into dedicated import settings without changing this ownership rule.

## UV coordinates

Texture coordinates use a top-left origin.

```text
(0, 0) --------> +U
  |
  |
  |
  v
 +V
```

The usual texture range is:

```text
U = 0 .. 1
V = 0 .. 1
```

UV values outside that range are allowed. Sampler addressing decides how they behave.

Material `uvScale` multiplies mesh UVs before texture sampling.

External formats should be converted to this UV convention by their importer.

OBJ texture coordinates use the opposite vertical direction, so the OBJ importer normalizes them as:

```text
V = 1 - V
```

This conversion belongs to the OBJ importer.

It should not be implemented as a global image flip in the texture loader or as a special case in the renderer.

Intentional UV mirroring authored into a model should otherwise remain unchanged.

## Image orientation

Image files are loaded in their normal file orientation.

The texture loader does not globally flip images.

An author should be able to open a PNG or another supported image normally and expect the same orientation when it is used in the engine.

A texture should not need a special rotated or mirrored version just because it is used by the project.

Format-specific differences belong at the appropriate import boundary.

## Screen and UI coordinates

2D screen-space coordinates intentionally use a different convention from 3D world space.

Mouse coordinates use:

```text
origin = top-left
+X = right
+Y = down
```

UI positions follow the same general screen-space direction:

```text
origin = top-left
+X = right
+Y = down
```

UI positions are expressed in pixels.

So:

```text
3D world = +Y up
2D UI    = +Y down
```

This difference is intentional.

## Colors

Lighting calculations are performed in linear color space.

Numeric colors supplied directly through engine data are treated as linear values.

Examples include:

```text
MaterialAsset::baseColor
LightComponent::color
SceneEnvironment::backgroundColor
SceneEnvironment::ambientLightColor
UI colors
```

`TextureAsset` stores raw RGBA8 pixel data. A general color-space semantic for texture assets is planned as the material and texture system grows.

When a texture is used as a material base-color texture, the Vulkan renderer uploads it using an sRGB image format.

Sampling therefore converts stored sRGB color values to linear values before lighting.

Future data textures such as roughness, metallic, AO, or normal maps should use formats appropriate for non-color data instead of being treated as sRGB.

## Deferred rendering data

The GBuffer stores:

```text
Albedo   = RGBA8_UNORM
Normal   = RGBA16_FLOAT
Material = RGBA8_UNORM
Depth    = D32_FLOAT
```

Albedo values stored in the GBuffer are linear.

Normals stored in the GBuffer are world-space normals.

Vertex normals are transformed using the inverse-transpose model normal matrix before being written.

The material buffer uses:

```text
R = roughness
G = metallic
B = ambient occlusion
A = unused
```

PBR material ranges are:

```text
roughness = 0 .. 1
metallic = 0 .. 1
ambient occlusion = 0 .. 1
```

The lighting shader may clamp values such as roughness internally where needed for BRDF stability.

## HDR and presentation

Scene lighting is rendered into:

```text
RGBA16_FLOAT
```

so internal scene color is HDR-capable and linear.

Exposure and tone mapping are planned as the next HDR presentation step.

The Vulkan swapchain prefers:

```text
BGRA8_SRGB
SRGB_NONLINEAR
```

when that surface format is available and falls back to another supported format when necessary.

Once exposure and tone mapping are added, this section should also define the final HDR-to-display transform.

## Lights

Point lights use their world-space transform position.

Directional and spot lights use the entity's local:

```text
-Z
```

axis as their forward direction.

That direction is transformed into world space before rendering.

Light colors are treated as linear values.

`LightComponent::intensity` is an engine intensity multiplier rather than a physically defined unit such as lumens or candela.

`range` is expressed in world-space units.

Point and spot lights use inverse-square-style attenuation with a smooth range cutoff.

Spotlight cone angles are expressed in degrees.

`innerConeAngle` and `outerConeAngle` are measured from the center forward axis rather than representing the complete cone diameter.

The full outer cone angle is therefore conceptually:

```text
outerConeAngle * 2
```

This is also how the spot shadow projection is derived.

## Units

A physical world-unit scale is planned to be standardized when systems such as physics, audio, and character movement begin to depend on it.

Until then, there is no fixed rule such as:

```text
1 unit = 1 meter
```

Positions, light ranges, camera planes, and other distances use the same generic world-space units.

The scale should be chosen from real requirements of those systems rather than being fixed ahead of time.

## Boundaries

The general rule is:

```text
source format
-> importer
-> engine convention
-> CPU asset / engine data
-> graphics or platform backend
```

Importers handle differences in source asset formats.

Graphics backends handle API-specific representation differences.

Platform code handles operating-system-specific differences.

Entities, gameplay code, and high-level systems should not need to care whether an OBJ, Vulkan, Windows, or another external system used a different convention before the data entered the engine.

## Future conventions

This document will grow together with the engine.

Conventions for new systems should be added when their corresponding features become real and the project has an actual reason to standardize them.

## In short

The project should keep one internal convention for each kind of data.

When an external format, graphics API, operating system, or tool uses something different:

```text
convert at the boundary
```

Do not spread multiple coordinate systems, matrix conventions, UV orientations, winding rules, or color-space assumptions throughout the engine.

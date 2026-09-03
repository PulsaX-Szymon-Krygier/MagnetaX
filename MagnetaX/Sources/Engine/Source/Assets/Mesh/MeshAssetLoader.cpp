// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "MeshAssetLoader.h"
#include <tiny_obj_loader.h>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <utility>

namespace
{
    struct OBJVertexKey
    {
        int32 vertexIndex;
        int32 normalIndex;
        int32 texcoordIndex;
        uint32 smoothingGroup;
        usize flatFaceID;

        bool operator==(const OBJVertexKey&) const = default;
    };

    struct OBJNormalKey
    {
        int32 vertexIndex;
        uint32 smoothingGroup;
        usize flatFaceID;

        bool operator==(const OBJNormalKey&) const = default;
    };

    struct OBJVertexKeyHash
    {
        usize operator()(const OBJVertexKey& key) const
        {
            usize hash = std::hash<int32>{}(key.vertexIndex);
            hash ^= std::hash<int32>{}(key.normalIndex) << 1;
            hash ^= std::hash<int32>{}(key.texcoordIndex) << 2;
            hash ^= std::hash<uint32>{}(key.smoothingGroup) << 3;
            hash ^= std::hash<usize>{}(key.flatFaceID) << 4;

            return hash;
        }
    };

    struct OBJNormalKeyHash
    {
        usize operator()(const OBJNormalKey& key) const
        {
            usize hash = std::hash<int32>{}(key.vertexIndex);
            hash ^= std::hash<uint32>{}(key.smoothingGroup) << 1;
            hash ^= std::hash<usize>{}(key.flatFaceID) << 2;

            return hash;
        }
    };

    bool LoadFromOBJ(const AssetSource& source, std::vector<MeshVertex>& vertices,
        std::vector<uint32>& indices, bool flipWinding = false)
    {
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true; // Keep triangles only for now

        if (!reader.ParseFromFile(source.GetPath(), config)) return false;

        const tinyobj::attrib_t& attributes = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

        usize indexCount = 0;

        for (const tinyobj::shape_t& shape : shapes) indexCount += shape.mesh.indices.size();

        vertices.clear();
        vertices.reserve(indexCount);

        indices.clear();
        indices.reserve(indexCount);

        std::unordered_map<OBJVertexKey, uint32, OBJVertexKeyHash> vertexMap;
        vertexMap.reserve(indexCount);

        std::vector<uint8> normalPresent;
        normalPresent.reserve(indexCount);

        std::vector<OBJNormalKey> generatedNormalKeys;
        generatedNormalKeys.reserve(indexCount);

        bool hasMissingNormals = false;
        usize flatFaceID = 1;

        for (const tinyobj::shape_t& shape : shapes)
        {
            usize indexOffset = 0;

            for (usize faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); ++faceIndex)
            {
                const uint32 faceVertexCount = shape.mesh.num_face_vertices[faceIndex];

                if (faceVertexCount != 3) return false;
                if (indexOffset + faceVertexCount > shape.mesh.indices.size()) return false;

                const uint32 smoothingGroup = faceIndex < shape.mesh.smoothing_group_ids.size() ?
                    shape.mesh.smoothing_group_ids[faceIndex] : 0;

                for (uint32 vertexInFace = 0; vertexInFace < faceVertexCount; ++vertexInFace)
                {
                    const tinyobj::index_t& objIndex = shape.mesh.indices[indexOffset + vertexInFace];

                    if (objIndex.vertex_index < 0) return false;

                    bool hasNormal = false;
                    int32 normalIndex = -1;
                    Vector3f importedNormal{};

                    if (objIndex.normal_index >= 0)
                    {
                        const usize normalOffset = static_cast<usize>(objIndex.normal_index) * 3;

                        if (normalOffset + 2 < attributes.normals.size())
                        {
                            importedNormal = Vector3f(attributes.normals[normalOffset], attributes.normals[normalOffset + 1],
                                attributes.normals[normalOffset + 2]);

                            normalIndex = objIndex.normal_index;
                            hasNormal = true;
                        }
                    }

                    const uint32 keySmoothingGroup = hasNormal ? 0 : smoothingGroup;
                    const usize keyFlatFaceID = !hasNormal && smoothingGroup == 0 ? flatFaceID : 0;

                    const OBJVertexKey key{ objIndex.vertex_index, normalIndex, objIndex.texcoord_index, keySmoothingGroup, keyFlatFaceID };
                    auto existing = vertexMap.find(key);

                    if (existing != vertexMap.end())
                    {
                        indices.push_back(existing->second);
                        continue;
                    }

                    MeshVertex vertex{};

                    const usize positionOffset = static_cast<usize>(objIndex.vertex_index) * 3;

                    if (positionOffset + 2 >= attributes.vertices.size()) return false;

                    vertex.position = Vector3f(attributes.vertices[positionOffset], attributes.vertices[positionOffset + 1],
                        attributes.vertices[positionOffset + 2]);

                    if (hasNormal) vertex.normal = importedNormal;

                    if (objIndex.texcoord_index >= 0)
                    {
                        const usize texcoordOffset = static_cast<usize>(objIndex.texcoord_index) * 2;

                        if (texcoordOffset + 1 < attributes.texcoords.size())
                        {
                            vertex.uv = Vector2f(attributes.texcoords[texcoordOffset], 1.0f - attributes.texcoords[texcoordOffset + 1]);
                        }
                    }

                    const uint32 vertexIndex = static_cast<uint32>(vertices.size());
                    const OBJNormalKey normalKey{ objIndex.vertex_index, keySmoothingGroup, keyFlatFaceID };

                    vertices.push_back(vertex);
                    normalPresent.push_back(hasNormal);
                    generatedNormalKeys.push_back(normalKey);

                    if (!hasNormal) hasMissingNormals = true;

                    indices.push_back(vertexIndex);
                    vertexMap.emplace(key, vertexIndex);
                }

                indexOffset += faceVertexCount;
                ++flatFaceID;
            }

            if (indexOffset != shape.mesh.indices.size()) return false;
        }

        // Flip winding if required
        if (flipWinding)
        {
            for (usize i = 0; i + 2 < indices.size(); i += 3)
            {
                std::swap(indices[i + 1], indices[i + 2]);
            }
        }

        // Generate missing normals
        if (hasMissingNormals)
        {
            std::unordered_map<OBJNormalKey, Vector3f, OBJNormalKeyHash> generatedNormals;
            generatedNormals.reserve(vertices.size());

            for (usize i = 0; i + 2 < indices.size(); i += 3)
            {
                const uint32 index0 = indices[i];
                const uint32 index1 = indices[i + 1];
                const uint32 index2 = indices[i + 2];

                const MeshVertex& vert0 = vertices[index0];
                const MeshVertex& vert1 = vertices[index1];
                const MeshVertex& vert2 = vertices[index2];

                const Vector3f edge1 = vert1.position - vert0.position;
                const Vector3f edge2 = vert2.position - vert0.position;
                const Vector3f faceNormal = Vector3f::Cross(edge1, edge2);

                if (!normalPresent[index0]) generatedNormals[generatedNormalKeys[index0]] += faceNormal;
                if (!normalPresent[index1]) generatedNormals[generatedNormalKeys[index1]] += faceNormal;
                if (!normalPresent[index2]) generatedNormals[generatedNormalKeys[index2]] += faceNormal;
            }

            for (usize i = 0; i < vertices.size(); ++i)
            {
                if (normalPresent[i]) continue;

                auto normal = generatedNormals.find(generatedNormalKeys[i]);
                if (normal == generatedNormals.end()) continue;

                vertices[i].normal = normal->second;
                vertices[i].normal.Normalize();
            }
        }

        return true;
    }
}

bool MeshAssetLoader::LoadFromFile(const AssetSource& source, std::vector<MeshVertex>& vertices,
    std::vector<uint32>& indices, bool flipWinding)
{
    const std::filesystem::path path(source.GetPath());
    const std::string extension = path.extension().string();

    if (extension == ".obj") return LoadFromOBJ(source, vertices, indices, flipWinding);

    return false;
}

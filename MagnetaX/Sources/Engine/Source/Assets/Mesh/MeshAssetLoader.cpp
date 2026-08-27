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

        bool operator==(const OBJVertexKey&) const = default;
    };

    struct OBJVertexKeyHash
    {
        usize operator()(const OBJVertexKey& key) const
        {
            usize hash = std::hash<int32>{}(key.vertexIndex);
            hash ^= std::hash<int32>{}(key.normalIndex) << 1;
            hash ^= std::hash<int32>{}(key.texcoordIndex) << 2;

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

        for (const tinyobj::shape_t& shape : shapes)
        {
            for (const tinyobj::index_t& objIndex : shape.mesh.indices)
            {
                if (objIndex.vertex_index < 0) return false;

                const OBJVertexKey key{ objIndex.vertex_index, objIndex.normal_index, objIndex.texcoord_index };
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

                if (objIndex.normal_index >= 0)
                {
                    const usize normalOffset = static_cast<usize>(objIndex.normal_index) * 3;

                    if (normalOffset + 2 < attributes.normals.size())
                    {
                        vertex.normal = Vector3f(attributes.normals[normalOffset], attributes.normals[normalOffset + 1],
                            attributes.normals[normalOffset + 2]);
                    }
                }

                if (objIndex.texcoord_index >= 0)
                {
                    const usize texcoordOffset = static_cast<usize>(objIndex.texcoord_index) * 2;

                    if (texcoordOffset + 1 < attributes.texcoords.size())
                    {
                        //vertex.uv = Vector2f(attributes.texcoords[texcoordOffset], attributes.texcoords[texcoordOffset + 1]);
                        vertex.uv = Vector2f(attributes.texcoords[texcoordOffset], 1.0f - attributes.texcoords[texcoordOffset + 1]);
                    }
                }

                const uint32 vertexIndex = static_cast<uint32>(vertices.size());

                vertices.push_back(vertex);
                indices.push_back(vertexIndex);

                vertexMap.emplace(key, vertexIndex);
            }
        }

        // Flip winding if required
        if (flipWinding)
        {
            for (usize i = 0; i + 2 < indices.size(); i += 3)
            {
                std::swap(indices[i + 1], indices[i + 2]);
            }
        }

        // Generate normals if .obj does not have them
        // But verify this method later
        if (attributes.normals.empty())
        {
            for (usize i = 0; i + 2 < indices.size(); i += 3)
            {
                MeshVertex& vert0 = vertices[indices[i]];
                MeshVertex& vert1 = vertices[indices[i + 1]];
                MeshVertex& vert2 = vertices[indices[i + 2]];

                const Vector3f edge1 = vert1.position - vert0.position;
                const Vector3f edge2 = vert2.position - vert0.position;
                const Vector3f faceNormal = Vector3f::Cross(edge1, edge2);

                vert0.normal += faceNormal;
                vert1.normal += faceNormal;
                vert2.normal += faceNormal;
            }

            for (MeshVertex& vertex : vertices) vertex.normal.Normalize();
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

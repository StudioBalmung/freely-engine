#pragma once

#include <string>
#include <vector>

// Forward declaration of Assimp structures to avoid pulling heavy headers in the public API.
namespace Assimp {
    class Importer;
    struct aiScene;
    struct aiMesh;
}

namespace Freely {
    namespace Renderer {
        struct Mesh {
            std::vector<float> vertices; // Interleaved position, normal, texcoord etc.
            std::vector<unsigned int> indices;
        };

        class ModelImporter {
        public:
            /**
             * Load a 3D model from the given file path using Assimp.
             * Supported formats: obj, fbx, gltf, glb, etc.
             * @param filePath Absolute or relative path to the model file.
             * @return Vector of meshes extracted from the model. Empty vector indicates failure.
             */
            static std::vector<Mesh> LoadModel(const std::string& filePath);
        };
    }
}

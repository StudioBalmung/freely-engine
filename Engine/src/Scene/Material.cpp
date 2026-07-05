#include "Freely/Scene/Material.h"
#include "Freely/Renderer/Shader.h"
#include "Freely/Renderer/Texture.h"

namespace Freely {

void Material::Bind() const {
    if (ShaderProgram) {
        ShaderProgram->Bind();
        ShaderProgram->SetVec3("u_Material.albedo", Albedo);
        ShaderProgram->SetFloat("u_Material.metallic", Metallic);
        ShaderProgram->SetFloat("u_Material.roughness", Roughness);
        ShaderProgram->SetFloat("u_Material.ao", AO);

        if (AlbedoMap) {
            AlbedoMap->Bind(0);
            ShaderProgram->SetInt("u_AlbedoMap", 0);
        }
        if (NormalMap) {
            NormalMap->Bind(1);
            ShaderProgram->SetInt("u_NormalMap", 1);
        }
        if (MetallicRoughnessMap) {
            MetallicRoughnessMap->Bind(2);
            ShaderProgram->SetInt("u_MetallicRoughnessMap", 2);
        }
    }
}

void Material::Unbind() const {
    if (ShaderProgram) {
        ShaderProgram->Unbind();
    }
}

std::shared_ptr<Material> Material::Create(std::shared_ptr<Shader> shader) {
    auto mat = std::make_shared<Material>();
    mat->ShaderProgram = shader;
    return mat;
}

} // namespace Freely

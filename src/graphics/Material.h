#ifndef GUI_MATERIAL_H
#define GUI_MATERIAL_H

#include <memory>
#include <utility>
#include "Texture.h"
#include "Shader.h"


struct MaterialData {
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specColor;
    float shininess;
    float refractionIndex;
    float alpha;
};


/**
 * Stores a shader w/ uniform values
 */
class Material {
private:
    std::unordered_map<std::string, UniformMapping> uniforms;
public:
    MaterialData matData{};

    std::string name;
    std::shared_ptr<Texture> ambientTex;
    std::shared_ptr<Texture> diffuseTex;
    std::shared_ptr<Texture> specTex;
    std::shared_ptr<Texture> shininessTex;
    std::shared_ptr<Texture> bumpTex;

    explicit Material(std::string name);


    void SetUniform(const std::string &uName, UniformType type, void *data);

    void Bind(Shader &shader);

    void Unbind();

private:
    static int
    setTexture(Shader &shader, const std::string &texName, const std::shared_ptr<Texture> &texture, int slot);

    static void setMaterialData(Shader &shader, MaterialData data);
};

#endif //GUI_MATERIAL_H

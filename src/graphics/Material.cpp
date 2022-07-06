#include <iostream>
#include "Material.h"

void Material::SetUniform(const std::string &name, UniformType type, void *data) {
  uniforms[name] = {type, data};
}

void Material::Bind() {
  shader.Bind();
  // Bitmap for textures present
  int texturesPresent = 0;
  texturesPresent |= setTexture("u_ambientTex", ambientTex, 0);
  texturesPresent |= setTexture("u_diffuseTex", diffuseTex, 1) << 1;
  texturesPresent |= setTexture("u_specTex", specTex, 2) << 2;
  texturesPresent |= setTexture("u_shinyTex", shininessTex, 3) << 3;
  glUniform1i(GetUniformLocation("u_texturesPresent"), texturesPresent);

  setMaterialData(matData);

  for (const auto &uMapping: uniforms) {
    auto name = uMapping.first;
    auto u = uMapping.second;
    auto type = u.type;
    auto data = u.data;
    int location = GetUniformLocation(name);
    switch (type) {
      case U4f: {
        glUniform4fv(location, 1, (float *) data);
        break;
      }
      case U3f: {
        auto floats = (float *) data;
        glUniform3fv(location, 1, (float *) data);
        break;
      }
      case U4i: {
        glUniform1iv(location, 1, (int *) data);
        break;
      }
      case UM4f: {
        auto *mat = (glm::mat4 *) data;
        glUniformMatrix4fv(GetUniformLocation(name), 1, TRANSPOSE, &((*mat)[0][0]));
        break;
      }
      default:
        std::cerr << "Unsupported uniform type." << std::endl;
    }

  }
}

int Material::setTexture(const std::string &name, const std::shared_ptr<Texture> &texture, int slot) {
  if (texture) {
    texture->Bind(slot);
    glUniform1i(GetUniformLocation(name), slot);
    return 1;
  }
  return 0;
}

void Material::setMaterialData(MaterialData data) {
  glUniform3fv(GetUniformLocation("u_ambientColor"), 1, &(data.ambientColor.x));
  glUniform3fv(GetUniformLocation("u_diffuseColor"), 1, &(data.diffuseColor.x));
  glUniform3fv(GetUniformLocation("u_specColor"), 1, &(data.specColor.x));
  glUniform1f(GetUniformLocation("u_shininess"), data.shininess);
  glUniform1f(GetUniformLocation("u_refractionIndex"), data.refractionIndex);
  glUniform1f(GetUniformLocation("u_alpha"), data.alpha);
}


void Material::Unbind() {

}

int Material::GetUniformLocation(const std::string &name) {
  if (uniformCache.contains(name)) return uniformCache[name];

  int location = glGetUniformLocation(shader.rendererId, name.c_str());

  if (location == -1) {
    printf("WARN: Uniform %s not found\n", name.c_str());
  }

  uniformCache[name] = location;
  return location;
}
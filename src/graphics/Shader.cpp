#include "Shader.h"
#include "../log.h"
#include <glm/gtc/type_ptr.hpp>

#include <utility>

const std::string extensions[] = {"vert", "geom", "frag"};
const GLenum types[] = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER};

// For vertex / frag shaders
Shader::Shader(const std::string &baseName, bool compute) {
  name = baseName;
  id = glCreateProgram();
  uint32_t idsToDelete[3] = {0, 0, 0};
  if (!compute) {
    for (int i = 0; i < 3; i++) {

      char *source = readFile(baseName + "." + extensions[i]);

      if (!source) {
        continue;
      }

      uint32_t shaderId = CompileShader(source, types[i]);
      glAttachShader(id, shaderId);
      idsToDelete[i] = shaderId;
      free(source);
    }
  } else {
    char *source = readFile(baseName + ".comp");

    if (!source) {
      Log::logf("ERR: Sources are null");
      exit(1);
    }

    uint32_t shaderId = CompileShader(source, GL_COMPUTE_SHADER);
    glAttachShader(id, shaderId);
    idsToDelete[0] = shaderId;
    free(source);
  }
  int linkStatus;
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
  if(linkStatus != GL_TRUE) {
    int log_length;
    char message[1024];
    glGetProgramInfoLog(id, 1024, &log_length, message);
    Log::fatalf("%s link error: %s", name.c_str(), message);
  }

  int validateStatus;
  glValidateProgram(id);
  glGetProgramiv(id, GL_VALIDATE_STATUS, &validateStatus);
  if(validateStatus != GL_TRUE) {
    int log_length;
    char message[1024];
    glGetProgramInfoLog(id, 1024, &log_length, message);
    Log::fatalf("%s validation error: %s", name.c_str(), message);
  }

  for (unsigned int i: idsToDelete) {
    // Delete intermediates
    if (i)
      glDeleteShader(i);
  }
}

Shader::~Shader() {
  glDeleteProgram(id);
}

void Shader::Bind() const {
  glUseProgram(id);
}

void Shader::Unbind() {
  glUseProgram(0);
}

unsigned int Shader::CompileShader(char *source, GLenum type) {
  unsigned int shaderId = glCreateShader(type);
  glShaderSource(shaderId, 1, &source, nullptr);
  glCompileShader(shaderId);
  int result;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
  if(result != GL_TRUE) {
      int log_length;
      char message[1024];
      glGetShaderInfoLog(shaderId, 1024, &log_length, message);
      Log::fatalf("%s shader could not be compiled: %s", name.c_str(), message);
#ifdef DEBUG
      __debugbreak();
#endif
  }
  return shaderId;
}

Shader *Shader::LoadShader(const std::string &name) {
  return shaders[name];
}

Shader *Shader::CreateShader(const string &name) {
  auto s = new Shader(SHADER_PATH + name);
  shaders[name] = s;
  return s;
}

Shader *Shader::CreateComputeShader(const string &name) {
  auto s = new Shader(SHADER_PATH + name, true);
  shaders[name] = s;
  return s;
}

uint32_t getPadding(uint32_t idx, uint32_t alignment) {
  uint32_t offset = idx - (idx / alignment) * alignment;
  if (offset > 0)
    return alignment - offset;
  return 0;
}

unsigned int Shader::CreateBuffer(GLenum type, const string &name, BufferLayout layout, int idx) {
  uint32_t id;
  glGenBuffers(1, &id);
  glBindBuffer(type, id);

  uint32_t size = 0;
  for (const auto &element: layout.GetElements()) {
    for (int i = 0; i < element.count; i++) {
      size += getPadding(size, element.alignment) + element.size;
    }
    // Account for padding at end of struct
    if (element.type == LAYOUT_TYPE)
      size += getPadding(size, element.alignment);
  }

  glBufferData(type, size, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(type, idx, id);
  globalUniforms[name] = {id, size, std::move(layout)};
  return id;
}

unsigned int Shader::CreateGlobalUniform(const string &name, BufferLayout layout, int idx) {
  return CreateBuffer(GL_UNIFORM_BUFFER, name, std::move(layout), idx);
}

unsigned int Shader::CreateShaderStorageBuffer(const string &name, BufferLayout layout, int idx) {
  return CreateBuffer(GL_SHADER_STORAGE_BUFFER, name, std::move(layout), idx);
}

void Shader::SetBuffer(GLenum type, const string &name, char *data, uint32_t bufSize) {
  auto uniformData = globalUniforms[name];
  auto layout = uniformData.layout;
  uint32_t inOffset = 0;
  uint32_t outOffset = 0;
  glBindBuffer(type, uniformData.id);
  for (const auto &element: layout.GetElements()) {
    for (int i = 0; i < element.count; i++) {
      outOffset += getPadding(outOffset, element.alignment);

      // If element is a struct/array
      // Only support one level ATM
      if (element.type == LAYOUT_TYPE) {
        for (const auto &subElement: element.subElements) {
          for (int j = 0; j < subElement.count; j++) {
            outOffset += getPadding(outOffset, subElement.alignment);

            if (inOffset > bufSize) return;
            glBufferSubData(type, outOffset, subElement.size, (void *) (data + inOffset));
            outOffset += subElement.size;
            inOffset += subElement.size;
          }
        }
        uint32_t padSize = getPadding(outOffset, element.alignment);
        // Just write garbage from the stack
        char padding[16];
        glBufferSubData(type, outOffset, padSize, (void *) &padding);
        outOffset += padSize;

      } else {
        if (inOffset > bufSize) return;
        glBufferSubData(type, outOffset, element.size, (void *) (data + inOffset));
        outOffset += element.size;
        inOffset += element.size;
      }
    }
  }
}

void Shader::SetGlobalUniform(const string &name, char *data, uint32_t bufSize) {
  SetBuffer(GL_UNIFORM_BUFFER, name, data, bufSize);
}

void Shader::SetShaderStorageBuffer(const string &name, char *data, uint32_t bufSize) {
  SetBuffer(GL_SHADER_STORAGE_BUFFER, name, data, bufSize);
}

void Shader::SetUniform(const std::string &uniName, UniformType type, void *data, int count /* = 1 */) {
  Bind();
  int location = GetUniformLocation(uniName);
  switch (type) {
    case U1f: {
      glUniform1fv(location, count, (float *) data);
      break;
    }
    case U2f: {
      glUniform2fv(location, count, (float *) data);
      break;
    }
    case U3f: {
      glUniform3fv(location, count, (float *) data);
      break;
    }
    case U4f: {
      glUniform4fv(location, count, (float *) data);
      break;
    }
    case U1i: {
      glUniform1iv(location, count, (int *) data);
      break;
    }
    case U4i: {
      glUniform4iv(location, count, (int *) data);
      break;
    }
    case UM4f: {
      auto *mat = (glm::mat4 *) data;
      glUniformMatrix4fv(location, count, TRANSPOSE, glm::value_ptr(*mat));
      break;
    }
    case UHandle: {
      glUniformHandleui64vARB(location, count, (GLuint64 *) data);
      break;
    }
    default:
      std::cerr << "Unsupported uniform type." << std::endl;
  }
}

int Shader::GetUniformLocation(const std::string &uniName) {
  if (uniformCache.contains(uniName)) return uniformCache[uniName];

  int location = glGetUniformLocation(id, uniName.c_str());

  if (location == -1) {
    Log::logf("WARN: Uniform %s not found", uniName.c_str());
  }

  uniformCache[uniName] = location;
  return location;
}
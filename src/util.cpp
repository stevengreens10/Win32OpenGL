#include "util.h"

char *readFile(const string &path) {
  FILE *file = fopen(path.c_str(), "r");
  if (!file) {
    return nullptr;
  }
  // Get file size
  fseek(file, 0L, SEEK_END);
  long sz = ftell(file);
  rewind(file);

  char *contents = (char *) malloc(sz + 1);
  int idx = 0;

  int ch;
  while ((ch = fgetc(file)) != EOF) {
    contents[idx++] = (char) ch;
  }
  contents[idx] = '\0';


  return contents;
}

vector<string> split(const string &s, const string &delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  string token;
  vector<string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

glm::vec4 color(int r, int g, int b, int a) {
  return color(float(r), float(g), float(b), float(a));
}

glm::vec4 color(float r, float g, float b, float a) {
  return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

glm::vec3 sToVec3(vector<string> s, int startIdx) {
  glm::vec3 vec;
  float *f = &vec.x;
  for (int i = 0; i < 3; i++) {
    try {
      f[i] = std::stof(s[i + startIdx]);
    } catch (std::exception &e) {
      f[i] = 0.0f;
    }
  }
  return vec;
}

glm::vec2 sToVec2(vector<string> s, int startIdx) {
  glm::vec2 vec;
  float *f = &vec.x;
  for (int i = 0; i < 2; i++) {
    try {
      f[i] = std::stof(s[i + startIdx]);
    } catch (int e) {
      f[i] = 0.0f;
    }
  }
  return vec;
}

void calculateTangents(vector<Vertex> &vertices, const vector<uint32_t> &indices) {
  for (int i = 0; i < indices.size(); i += 3) {
    glm::vec3 edge1 = vertices[indices[i + 1]].pos - vertices[indices[i]].pos;
    glm::vec3 edge2 = vertices[indices[i + 2]].pos - vertices[indices[i]].pos;
    glm::vec2 deltaUV1 = vertices[indices[i + 1]].uv - vertices[indices[i]].uv;
    glm::vec2 deltaUV2 = vertices[indices[i + 2]].uv - vertices[indices[i]].uv;

    glm::vec3 tangent;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    vertices[indices[i]].tangent = tangent;
    vertices[indices[i + 1]].tangent = tangent;
    vertices[indices[i + 2]].tangent = tangent;
  }
}
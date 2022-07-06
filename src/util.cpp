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
      f[i] = stof(s[i + startIdx]);
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
      f[i] = stof(s[i + startIdx]);
    } catch (int e) {
      f[i] = 0.0f;
    }
  }
  return vec;
}

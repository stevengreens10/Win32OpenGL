#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#ifndef GUI_CUBE_H
#define GUI_CUBE_H

#include <memory>
#include "../Object.h"
#include "Primitive.h"

class Cube : public Primitive {
private:
    inline static std::shared_ptr<VertexArray> vao;
    inline static std::shared_ptr<IndexBuffer> ibo;
public:
    explicit Cube(std::reference_wrapper<Material> m) : Primitive(m) {}

    static void InitBuffers();

private:
    static std::shared_ptr<VertexArray> CreateVertexBuffer();

    static std::shared_ptr<IndexBuffer> CreateIndexBuffer();

    inline VertexArray *getVertexBuffer() override {
      return vao.get();
    }

    IndexBuffer *getIndexBuffer() override {
      return ibo.get();
    }
};


#endif //GUI_CUBE_H

#pragma clang diagnostic pop
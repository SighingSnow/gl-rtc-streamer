#ifndef OBJECT_H
#define OBJECT_H

#include "Shader.h"
#include <glm/glm.hpp>

namespace zrender{

class ZObject {
public:
    virtual void init() = 0;
protected:
    virtual void initShader() = 0;
    virtual void initBuffer() = 0;
    Shader * shader_;
    unsigned int vao_,vbo_;
};


};



#endif // Object
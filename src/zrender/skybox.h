#ifndef SKYBOX_H
#define SKYBOX_H

#include "Object.h"
#include "Camera.h"
#include "../common.h"

#include <vector>
#include <glm/glm.hpp>

namespace zrender{

class SkyBox : public ZObject
{
public:
    SkyBox(Camera & camera): camera_(camera){
        loadCubeMap(skybox_faces);
    };
    ~SkyBox();
    void init() override;
    void draw() const; // automatically manage OpenGL context
protected:
    void initShader() override;
    void initBuffer() override;
private:
    Camera & camera_;
    unsigned int skyboxTexture;
    void loadCubeMap(std::vector<std::string> faces);
};


};


#endif
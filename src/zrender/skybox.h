#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>

namespace zrender{

class SkyBox
{
public:
    void draw(); // automatically manage OpenGL context
private:
    unsigned int VAO,VBO;
    int loadCubeMap(std::vector<std::string> faces);
};


};


#endif
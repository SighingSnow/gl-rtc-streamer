//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef SCENE_H
#define SCENE_H

#include "common.h"
#include "zrender/Shader.h"
#include "zrender/Camera.h"
#include "zrender/Model.h"
#include "zrender/skybox.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <vector>
#include FT_FREETYPE_H
#include "streamer.h"

#include<chrono>
#include<random>

using namespace zrender;

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

class Scene {
public:
    Scene();
    void SetUpEnv();
    void SetObjs(); // Set objs for drawing
    void SetCallback();
    void DrawScene();
    void Terminate();
    void AttachStreamer(Streamer* streamer);
    Camera* get_camera();
    ~Scene(){
    }
private:
    int frame_count;
    float prev_time;
    float delta_t;
    void Notify(uint8_t* buf);
    void loadFonts();
    void renderText(std::string text,GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
    Streamer* streamer_; // This can be changed to a list
private:
    GLFWwindow* window;
    unsigned int VBO,tVBO;
    unsigned int VAO,tVAO;
    std::vector<Model*> models_; // TODO: delete 
    Shader* textShader;
    Camera* camera;
    std::map<GLchar, Character> Characters;
public:
    void clientKeyboardCallback(char c);
    void clientMouseMoveCallback(double x,double y);
// draw my scene.
private:
    int mWidth_,mHeight_;
    SkyBox * skybox_;
    Shader * cShader;
    Shader * modelShader_;
    void initScene();
    void initModel();
    void renderScene();
    void renderTime();
    void renderModel();
};


#endif

#include "skybox.h"

#include <iostream>
#include <memory>
#include "zimage.h"
using namespace zrender;

const float skyboxVerts[] =  {
    -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
};

void SkyBox::init() {
    initShader();
    initBuffer();
}

SkyBox::~SkyBox() {
    glDeleteVertexArrays(1,&vao_);
    glDeleteBuffers(1,&vbo_);
}

void SkyBox::draw() const {
    if(shader_ == nullptr) {
        std::cerr<<"please init skybox shader first\n";
        return;
    }
    //
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    shader_->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera_.Zoom), (float)screen_width / (float)screen_height, 0.1f, 100.0f);
    glm::mat4 view = glm::mat4(glm::mat3(camera_.GetViewMatrix()));
    shader_->setMat4("view",view);
    shader_->setMat4("projection",projection);
    glBindVertexArray(vao_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP,skyboxTexture);
    glDrawArrays(GL_TRIANGLES,0,36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void SkyBox::initShader() {
    shader_ = new Shader("../src/Shaders/skybox.vs","../src/Shaders/skybox.fs");
    shader_->use();
    shader_->setInt("skybox",0);
    return;
}

void SkyBox::initBuffer() {
    glGenVertexArrays(1,&vao_);
    glGenBuffers(1,&vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER,vbo_);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVerts),&skyboxVerts,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    return;
}

void SkyBox::loadCubeMap(std::vector<std::string> faces) {
    glGenTextures(1,&skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP,skyboxTexture);
    std::unique_ptr<ZImage> image(new ZImage());
    int width,height,nrChannels;
    for(unsigned int i = 0; i < faces.size(); i++){
        image->load(faces[i].c_str());
        if(image->data())
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image->width(), image->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, image->data());
        else 
            std::cerr<<"load failed\n";
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return;
}
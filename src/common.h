//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef COMMON_H
#define COMMON_H
#include <thread>
#include <mutex>
#include <vector>
// For debug
const std::string out_h264 = "out.h264";
static char* RTMP_LIVESTEAM = "rtmp://localhost/live/livestream";
static bool dump_video_option = false;
static bool rtmp_publish_option = false;
static bool rtc_publish_option = true;
static bool docker_option = false;
// This is for screen saving
#ifdef __APPLE__
const int scale = 2;
#else
const int scale = 1;
#endif

// global
const unsigned int screen_width = 800;
const unsigned int screen_height = 600;
const unsigned int SCR_WIDTH = screen_width * scale;
const unsigned int SCR_HEIGHT = screen_height * scale;
// webrtc
const uint16_t defaultPort = 8000;
// FFMPEG parameters
const int inlinesize[2] = {SCR_WIDTH*3,0}; // For sws_scale convert function

static const char* rtmp_fout_name = "rtsp://localhost/livestream/live";

// OpenGL parameters
const std::string cube_vs = "../src/Shaders/cube.vs";
const std::string cube_fs = "../src/Shaders/cube.fs";
const std::string mcube_vs = "../src/Shaders/mcube.vs";
const std::string mcube_fs = "../src/Shaders/mcube.fs";
const std::string text_vs = "../src/Shaders/text.vs";
const std::string text_fs = "../src/Shaders/text.fs";
const std::string model_vs = "../src/Shaders/model.vs";
const std::string model_fs = "../src/Shaders/model.fs";

const std::vector<std::string> gpu_str = {"cpu","nvenc","vaapi","qsv","videotoolbox"};

const std::vector<std::string> skybox_faces = {
"../resource/texture/skybox/right.jpg",
"../resource/texture/skybox/left.jpg",
"../resource/texture/skybox/top.jpg",
"../resource/texture/skybox/bottom.jpg",
"../resource/texture/skybox/front.jpg",
"../resource/texture/skybox/back.jpg"
};

// const std::vector<std::string> skybox_faces = {
// "../resource/texture/skybox/posx.jpg",
// "../resource/texture/skybox/negx.jpg",
// "../resource/texture/skybox/posy.jpg",
// "../resource/texture/skybox/negy.jpg",
// "../resource/texture/skybox/posz.jpg",
// "../resource/texture/skybox/negz.jpg"
// };


enum HW_ACCEL_TYPE {
        CPU,
        NVENC,
        VAAPI,
        QSV,
        VIDEOTOOLBOX
};

const float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

#endif

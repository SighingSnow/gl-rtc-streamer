//
// Created by 宋庭聿 on 2021/10/22.
//

#include "streamer.h"

#include <nlohmann/json.hpp>
#include "scene.h"
using namespace rtc;

Streamer::Streamer(Scene & scene):scene_(scene)
{
    encoder = nullptr;
    rtmp_publisher = nullptr;
}

void Streamer::initRtmp() {
    rtmp_publisher = new RtmpPublisher();
    rtmp_publisher->setUp();
}

void Streamer::beginStream(bool dump_video_opt,int ors_gpu_id,std::string ip_addr)
{
    encoder = new Encoder();
    encoder->dump_video_option = dump_video_opt;
    encoder->setHWId(ors_gpu_id);
    encoder->Init();
    if(rtmp_publish_option){
        initRtmp();
    }
    else if(rtc_publish_option){
        initRtc(ip_addr);
    }
}

void Streamer::encode(uint8_t *buffer)
{
    int ret_buf_size = 0;
    uint8_t* ret_buf = nullptr;
    encoder->genOnePkt(buffer,&ret_buf,ret_buf_size);
    if(rtmp_publish_option)
        rtmpPublish(ret_buf,ret_buf_size);
    if(rtc_publish_option)
        rtcPublish(ret_buf,ret_buf_size);
    free(ret_buf); // malloced in encoder
}

void Streamer::setEncodeHWId(int id)
{
    encoder->setHWId(id);
}

void Streamer::rtmpPublish(uint8_t *buf, int size) {
    rtmp_publisher->publish(buf,size);
}

void Streamer::rtcPublish(uint8_t *buf, int size) {
    rtc_publisher->publish(buf,size);
}

void Streamer::endStream()
{
    encoder->endEncode();
}

void Streamer::initRtc(std::string ip_addr)
{
    rtc_publisher = new RtcPublisher();
    rtc_publisher->setPubAddr(ip_addr);
    rtc_publisher->setUp();
    // bind callback
    func_keyboard = std::bind(&Scene::clientKeyboardCallback,scene_,std::placeholders::_1);
    func_mousemove = std::bind(&Scene::clientMouseMoveCallback,scene_,std::placeholders::_1,std::placeholders::_2);
    rtc_publisher->setInputCallBack(&func_keyboard,&func_mousemove);
}

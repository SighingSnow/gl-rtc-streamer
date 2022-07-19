//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef STREAMER_H
#define STREAMER_H

#include <rtc/rtc.hpp>
#include <rtc/websocket.hpp>
#include <nlohmann/json.hpp>
#include "encoder.h"
#include "rtmp_publisher.h"
#include "rtc_publisher.h"

class Scene;

using json = nlohmann::json;

// TODO: Refact Streamer to Ors
class Streamer {
public:
    Streamer(Scene & scene);
    void beginStream(bool dump_video_opt = false,int ors_gpu_id = 0,string ip_addr = "");
    void endStream();
    void encode(uint8_t* buffer);
    void setDumpVideoOpt(bool val);
    void setEncodeHWId(int id);
private:
    Scene & scene_;
    Encoder* encoder;
    RtmpPublisher* rtmp_publisher;
    RtcPublisher* rtc_publisher;
private:
    void initRtc(std::string ip_addr);
    void initRtmp();
    void rtmpPublish(uint8_t* buf,int size);
    void rtcPublish(uint8_t* buf,int size);
private:
    std::function<void(char)> func_keyboard;
    std::function<void(double,double)> func_mousemove;
};


#endif //STREAMER_H

//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef ENCODER_H
#define ENCODER_H

extern "C"{
#include<libavdevice/avdevice.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include <mutex>
#include <thread>

const int AV_CODEC_ID = AV_CODEC_ID_H264;

#include<iostream>
#include<cstdio>

#include "rtmp_publisher.h"

class Encoder {
public:
    Encoder();
    void Init();
    void genOnePkt(uint8_t* buffer,uint8_t** ret_buf,int& ret_buf_size);
    void dumpLocalVideo();
    void flushEncoder(int streamIndex);
    void endEncode();
    void setHWId(int id);
    bool dump_video_option = false;
private:
    int frame_count;
    uint8_t * in_buf[2];
    std::string out_filename;
    FILE* fout;
private:
    AVCodec* codec;
    AVCodecContext* codecCtx;
    AVFormatContext* ofctx;
    AVStream* stream; // dump local video
    AVOutputFormat* outputFormat;
    AVPacket* pkt;
    AVFrame* frameYUV;
    SwsContext* swsContext;
private:
    AVBufferRef *hw_device_ctx = nullptr;
    AVBufferRef *hw_frames_ref = nullptr;
    AVHWFramesContext * frames_ctx;
    AVFrame * hw_frame;
    // gpu_id 0: cpu 1: nvidia gpu  2: amd gpu card  
    // 3: intel : quick sync video 4: macos : video tool box
    int ors_gpu_id = 0; 
    void rgb24ToYuvframe(uint8_t* buffer);
    void initFFmpegEnv();
    void initLocalStream();
    void setCodec();
    void allocBuffer();
    void allocInBuf();
    void allocAVFrames();
    void allocPkt();
    void setSwsCtx();
    void setHwCtx(); // Hardware context
    void initCodecCtx();
// For debug
private:
    void rgb24toppm(uint8_t* buf,int width,int height);
    void yuv420toppm(AVFrame* frame);
    void write_ppm_header(FILE* fp);
    void flip(uint8_t** buf);
};


#endif

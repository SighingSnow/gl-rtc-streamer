//
// Created by 宋庭聿 on 2021/10/22.
//

#include "encoder.h"

#include "common.h"

Encoder::Encoder(){
    codec = nullptr;
    codecCtx = nullptr;
    pkt = nullptr;
    frameYUV = nullptr;
    swsContext = nullptr;
    write_to_file_flag = false;
    out_filename = nullptr;
    frame_count = 0;
}

Encoder::Encoder(bool write_flag,char* filename){
    ofctx = nullptr;
    stream = nullptr;
    outputFormat = nullptr;
    codec = nullptr;
    codecCtx = nullptr;
    pkt = nullptr;
    frameYUV = nullptr;
    swsContext = nullptr;
    write_to_file_flag = write_flag;
    out_filename = filename;
    frame_count = 0;
}

void Encoder::Init()
{
    in_buf[0] = (uint8_t*)malloc(sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH*3);
    in_buf[1] = nullptr;

    // Output file context
    avformat_alloc_output_context2(&ofctx,NULL,NULL,out_filename);
    outputFormat = ofctx->oformat;

    if(avio_open(&ofctx->pb,out_filename,AVIO_FLAG_READ_WRITE) < 0)
    {
        std::cout << "open outfile error" << std::endl;
        return;
    }
    stream = avformat_new_stream(ofctx,NULL);
    if(stream == nullptr){
        std::cout<<"Create new stream fails"<<std::endl;
        return;
    }

    // AV_CODEC_ID is declared in common.h
    //codec = avcodec_find_encoder((AVCodecID) AV_CODEC_ID);
    codecCtx = stream->codec;
    codecCtx->codec_id = outputFormat->video_codec;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    codecCtx->bit_rate = 400000;
    codecCtx->width = SCR_WIDTH;
    codecCtx->height = SCR_HEIGHT;
    codecCtx->time_base.num = 1;
    codecCtx->time_base.den = 25;
    codecCtx->gop_size = 250;
    //codecCtx->framerate = (AVRational){25,1};
    //codecCtx->bit_rate = SCR_HEIGHT * SCR_WIDTH * 3;
    codecCtx->qmin = 10;
    codecCtx->qmax = 51;
    codecCtx->max_b_frames = 1;
    AVDictionary* para = nullptr;
    if(codecCtx->codec_id == AV_CODEC_ID_H264){
        av_dict_set(&para,"preset","slow",0);
        av_dict_set(&para,"tune","zerolatency",0);
    }

    codec = avcodec_find_encoder(codecCtx->codec_id);
    if(!codec){
        std::cout<<"can not find encoder"<<std::endl;
        return;
    }
    int ret = avcodec_open2(codecCtx,codec,&para);
    if(ret < 0){
        std::cout<<"avcodec_open2 failed"<<std::endl;
        return;
    }
    avformat_write_header(ofctx,NULL);
    pkt = av_packet_alloc();
    if(!pkt)
        exit(1);

    frameYUV = av_frame_alloc();
    if(!frameYUV){
        std::cerr<<"Could not allocate video frame\n";
        exit(1);
    }
    frameYUV->format = AV_PIX_FMT_YUV420P;
    frameYUV->width = SCR_WIDTH;
    frameYUV->height = SCR_HEIGHT;
    ret = av_frame_get_buffer(frameYUV,0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    fout = fopen(out_filename,"wb");

    swsContext = sws_getContext(SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_RGB24,
                                SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC,NULL,NULL,NULL);
}

void Encoder::FlushEncoder(int streamIndex)
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if(!(ofctx->streams[0]->codec->codec->capabilities & AV_CODEC_CAP_DELAY))
        return;
    while(1){
        enc_pkt.data = nullptr;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(ofctx->streams[streamIndex]->codec,&enc_pkt, nullptr,&got_frame);
        av_frame_free(NULL);
        if(ret < 0)
            break;
        if(!got_frame){
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeedd to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
        ret = av_write_frame(ofctx,&enc_pkt);
        if(ret < 0)
            break;
    }
    return;
}

void Encoder::EndEncode()
{
    FlushEncoder(0);
    av_write_trailer(ofctx);
    if(stream){
        avcodec_close(stream->codec);
    }
    avio_close(ofctx->pb);
    av_packet_unref(pkt);
    avformat_free_context(ofctx);
    avcodec_free_context(&codecCtx);
    av_frame_free(&frameYUV);
    av_packet_free(&pkt);
    sws_freeContext(swsContext);
    fclose(fout);
}

void Encoder::GenOnePkt(uint8_t* buffer)
{
    // TODO: memory leak may happen here
    memcpy(in_buf[0],buffer,sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH*3);
    in_buf[1] = nullptr;
    int height = sws_scale(swsContext,(const uint8_t* const*)in_buf,inlinesize,0,SCR_HEIGHT,
                           frameYUV->data,frameYUV->linesize);
    if(height <= 0) exit(1);
    // TODO: whether pts info needed should be further discuss
    frameYUV->pts = frame_count++ * (stream->time_base.den / codecCtx->time_base.num * 25);
    int ret = avcodec_send_frame(codecCtx,frameYUV);
    if(ret < 0){
        printf("Error sending a frame for encoding");
        exit(1);
    }
    while(ret >= 0){
        ret = avcodec_receive_packet(codecCtx,pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }
        else if(ret < 0){
            std::cerr<<"error during encoding"<<std::endl;
            exit(1);
        }
        if(write_to_file_flag){
            PktToX264();
        }
        // TODO: send this pkt data to rtsp out file.
        av_packet_unref(pkt);
    }
}

void Encoder::PktToX264()
{
    pkt->stream_index = stream->index;
    int er = av_write_frame(ofctx,pkt);
    if(er < 0){
        std::cout<<"write frame error"<<std::endl;
    }
    fwrite(pkt->data,1,pkt->size,fout);
}
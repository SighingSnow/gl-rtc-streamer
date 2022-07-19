//
// Created by 宋庭聿 on 2021/10/22.
//

#include "encoder.h"

#include "common.h"

Encoder::Encoder(){
    ofctx = nullptr;
    stream = nullptr;
    outputFormat = nullptr;
    codec = nullptr;
    codecCtx = nullptr;
    pkt = nullptr;
    frameYUV = nullptr;
    swsContext = nullptr;
    out_filename = "out.h264";
    frame_count = 0;
}

// Init ffmpeg encoder
void Encoder::Init()
{
    initFFmpegEnv();
}

void Encoder::genOnePkt(uint8_t* buffer,uint8_t** ret_buf,int& ret_buf_size)
{
    int ret = 0;
    rgb24ToYuvframe(buffer);
    if(ors_gpu_id && ors_gpu_id != HW_ACCEL_TYPE::VIDEOTOOLBOX) {
        av_hwframe_transfer_data(hw_frame, frameYUV, 0);
        ret = avcodec_send_frame(codecCtx,hw_frame);
    } else {
        ret = avcodec_send_frame(codecCtx,frameYUV);
    }
    
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
        if(dump_video_option){
            dumpLocalVideo();
        }
        ret_buf_size = pkt->size;
        //printf("%d\n",ret_buf_size);
        *ret_buf = (uint8_t*)malloc(ret_buf_size*sizeof(uint8_t));
        memcpy(*ret_buf,pkt->data,sizeof(uint8_t)*ret_buf_size);
        // TODO: send this pkt data to rtsp out file.
        av_packet_unref(pkt);
    }
}

void Encoder::dumpLocalVideo()
{
    static int count = 2;
    pkt->stream_index = stream->index;
    int er = av_write_frame(ofctx,pkt);
    if(er < 0){
        std::cout<<"write frame error"<<std::endl;
    }
    if(count == 0) fclose(fout);
    fwrite(pkt->data,1,pkt->size,fout);
    //count--;
}

void Encoder::flushEncoder(int streamIndex)
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if(ofctx == nullptr) return;
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
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
        ret = av_write_frame(ofctx,&enc_pkt);
        if(ret < 0)
            break;
    }
    return;
}

void Encoder::endEncode()
{
    if(dump_video_option) {
        flushEncoder(0);
        av_write_trailer(ofctx);
        if(stream){
            avcodec_close(stream->codec);
        }
        avio_close(ofctx->pb);
        avformat_free_context(ofctx);
        fclose(fout);
    }
    av_packet_unref(pkt);
    //avcodec_free_context(&codecCtx);
    av_frame_free(&frameYUV);
    if(ors_gpu_id){
        av_frame_free(&hw_frame);
        av_buffer_unref(&hw_device_ctx);
    } 
    //sav_packet_free(&pkt);
    sws_freeContext(swsContext);
}

void Encoder::initFFmpegEnv()
{
    int ret = 0;
    
    avcodec_register_all();
    avdevice_register_all();
    av_register_all();
    avformat_network_init();
    setCodec();
    initCodecCtx();
    if(ors_gpu_id != HW_ACCEL_TYPE::CPU && ors_gpu_id != HW_ACCEL_TYPE::VIDEOTOOLBOX) {
        setHwCtx(); // using gpu or hw accel
        codecCtx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
        if(codecCtx->hw_frames_ctx == nullptr) {
            std::cout<<"[Encoder] create new reference to an av buffer fails"<<std::endl;
        }
    }
    // for cpu only
    AVDictionary* para = nullptr;
    if(codecCtx->codec_id == AV_CODEC_ID_H264){
        av_dict_set(&para,"preset","slow",0);
        av_dict_set(&para,"tune","zerolatency",0);
    }
    ret = avcodec_open2(codecCtx,codec,&para);
    allocBuffer();
    setSwsCtx();// SwsScale
    if(dump_video_option){
        initLocalStream();
    }
}
void Encoder::initLocalStream()
{
    int ret;
    avformat_alloc_output_context2(&ofctx,NULL,NULL,out_filename.c_str());
    outputFormat = ofctx->oformat;
    if(avio_open2(&ofctx->pb,out_filename.c_str(),AVIO_FLAG_WRITE, nullptr,nullptr) < 0)
    {
        std::cout << "open outfile error" << std::endl;
        return;
    }
    if(ofctx->oformat->flags & AVFMT_GLOBALHEADER){
        codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    stream = avformat_new_stream(ofctx,codec);
    if(stream == nullptr){
        std::cout<<"Create new stream fails"<<std::endl;
        return;
    }
    
    ret = avcodec_parameters_from_context(stream->codecpar,codecCtx);
    if(ret < 0){
        std::cout<<"Could not initialize stream codec parameters"<<std::endl;
        exit(1);
    }
    stream->codecpar->extradata_size = codecCtx->extradata_size;
    stream->codecpar->extradata = codecCtx->extradata;
    avformat_write_header(ofctx,NULL);
    fout = fopen(out_filename.c_str(),"wb");
}

void Encoder::setCodec()
{
    switch(ors_gpu_id)
    {
        case 0: codec = avcodec_find_encoder(AV_CODEC_ID_H264); break;
        case 1: codec = avcodec_find_encoder_by_name("h264_nvenc"); break;
        case 2: codec = avcodec_find_encoder_by_name("h264_vaapi"); break;
        case 3: codec = avcodec_find_encoder_by_name("h264_qsv"); break;
        case 4: codec = avcodec_find_encoder_by_name("h264_videotoolbox"); break;
        default:
            codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            break;
    }
    if(codec == nullptr) {
        ors_gpu_id = 0;
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        std::cout<<"Specific accelerate hardware not found, using cpu encoding"<<std::endl;
    }
    return;
}

void Encoder::allocBuffer()
{
    allocInBuf();
    allocAVFrames();
    allocPkt();
}
void Encoder::allocInBuf(){
    in_buf[0] = (uint8_t*)malloc(sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH*3);
    in_buf[1] = nullptr;
}
void Encoder::allocAVFrames() {
    int ret = 0;
    // init frameYUV
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
    // Videotoolbox will malloc frame automatically
    if(ors_gpu_id && ors_gpu_id != HW_ACCEL_TYPE::VIDEOTOOLBOX){
        // init hw_frame
        hw_frame = av_frame_alloc();
        if(!hw_frame){
            std::cerr<<"Could not allocate video frame\n";
            exit(1);
        }
        // allocate buffer for hw_frame
        ret = av_hwframe_get_buffer(codecCtx->hw_frames_ctx, hw_frame, 0);
        if(ret < 0)
            std::cout<<"[Encoder] Allocate frame fails for codecCtx->hw_frames_ctx"<<std::endl;
    }
}
void Encoder::allocPkt()
{
    pkt = av_packet_alloc();
    if(!pkt)
        exit(1);
}

void Encoder::setHwCtx()
{
    int ret = 0;
    switch(ors_gpu_id){
        case 1: ret = av_hwdevice_ctx_create(&hw_device_ctx,AV_HWDEVICE_TYPE_CUDA, NULL, NULL, 0); break;
        case 2: ret = av_hwdevice_ctx_create(&hw_device_ctx,AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);break;
        case 3: ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_QSV,NULL, NULL, 0);  break;
        case 4: ret = av_hwdevice_ctx_create(&hw_device_ctx,AV_HWDEVICE_TYPE_VIDEOTOOLBOX,NULL,NULL,0); break;
    }
    if(ret < 0){
        std::cout<<"[Encoder] create hwdevice context fails"<<std::endl;
        exit(-1);
    }
    
    hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx);
    
    if(hw_frames_ref == nullptr){
        std::cout<<"[Encoder] Failed to create frame context."<<std::endl;
    }
    
    frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
    switch(ors_gpu_id){
        case 1: frames_ctx->format = AV_PIX_FMT_CUDA;break; // I don't know if cuda is correct
        case 2: frames_ctx->format = AV_PIX_FMT_VAAPI;break;
        case 3: frames_ctx->format = AV_PIX_FMT_QSV; break;
        case 4: frames_ctx->format = AV_PIX_FMT_VIDEOTOOLBOX;break;
    }
	frames_ctx->sw_format = AV_PIX_FMT_YUV420P;
	frames_ctx->width     = SCR_WIDTH;
	frames_ctx->height    = SCR_HEIGHT;
    //frames_ctx->initial_pool_size = 20;
	ret = av_hwframe_ctx_init(hw_frames_ref);
    if(ret < 0)
        std::cout<<"[Encoder] Finalizing to initialize hw frame context."<<std::endl;
    
}
void Encoder::setSwsCtx()
{
    swsContext = sws_getContext(SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_RGB24,
                                SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC,NULL,NULL,NULL);
}
void Encoder::initCodecCtx()
{
    codecCtx = avcodec_alloc_context3(codec);
    const AVRational dst_fps = {25,1};
    codecCtx->codec_tag = 0;
    //codecCtx->codec_id = AV_CODEC_ID_H264;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    switch(ors_gpu_id){
        case 0: codecCtx->pix_fmt = AV_PIX_FMT_YUV420P; break;
        case 1: codecCtx->pix_fmt = AV_PIX_FMT_CUDA; break;
        case 2: codecCtx->pix_fmt = AV_PIX_FMT_VAAPI; break;
        case 3: codecCtx->pix_fmt = AV_PIX_FMT_QSV; break;
        case 4: codecCtx->pix_fmt = AV_PIX_FMT_YUV420P; break;
    }
    codecCtx->width = SCR_WIDTH;
    codecCtx->height = SCR_HEIGHT;
    codecCtx->gop_size = 25;
    codecCtx->framerate = dst_fps;
    codecCtx->time_base = av_inv_q(dst_fps);
    //codecCtx->bit_rate = SCR_HEIGHT * SCR_WIDTH * 3;
    codecCtx->max_b_frames = 0;
}

void Encoder::rgb24ToYuvframe(uint8_t* buffer)
{
    // TODO: please reverse the picture upside down
    flip(&buffer);
    memcpy(in_buf[0],buffer,sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH*3);
    in_buf[1] = nullptr;
    // Dump
    //rgb24toppm(buffer,SCR_WIDTH,SCR_HEIGHT);
    //rgb24toppm(in_buf[0],SCR_WIDTH,SCR_HEIGHT);
    int height = sws_scale(swsContext,(const uint8_t* const*)in_buf,inlinesize,0,SCR_HEIGHT,
                           frameYUV->data,frameYUV->linesize);
    if(height <= 0) exit(1);
    // TODO: whether pts info needed should be further discuss
    //frameYUV->pts = (frame_count++)*(stream->time_base.den)/((stream->time_base.num)*25);
    frameYUV->pts = AV_NOPTS_VALUE;
    return;
}

void Encoder::setHWId(int id)
{
    ors_gpu_id = id;
}

// Below are funcs help to debug, not necessary
// the buf is up-down reversed
void Encoder::rgb24toppm(uint8_t *buf, int width, int height)
{
    FILE* fp = fopen("rgb24to.ppm","wb+");
    write_ppm_header(fp);
//    for(int j = height -1 ;j >=0 ;j--){
//        fwrite(buf+j*width*3,1,width*3,fp);
//    }
    for(int j = 0; j < height;j++){
        fwrite(buf+j*width*3,1,width*3,fp);
    }
    fclose(fp);
    return;
}
void Encoder::write_ppm_header(FILE *fp)
{
    fprintf(fp,"P6\n");
    fprintf(fp,"%d %d\n",SCR_WIDTH,SCR_HEIGHT);
    fprintf(fp,"%d\n",255);
}

void Encoder::flip(uint8_t** buf)
{
    int totalLength = SCR_HEIGHT*SCR_WIDTH*3;
    int oneLineLength = SCR_WIDTH*3;
    uint8_t* tmp = (uint8_t*)malloc(SCR_HEIGHT*SCR_WIDTH*3);
    memcpy(tmp,*buf,SCR_WIDTH*SCR_HEIGHT*3);
    memset(*buf,0,sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH*3);
    for(int i = 0; i < SCR_HEIGHT;i++){
        memcpy(*buf+oneLineLength*i,tmp+totalLength-oneLineLength*(i+1),oneLineLength);
    }
    // rgb24toppm(*buf,SCR_WIDTH,SCR_HEIGHT);
}

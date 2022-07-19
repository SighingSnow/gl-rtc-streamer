#include <ctime>
#include <cstdio>

#include "scene.h"
#include "streamer.h"

void SetScene(Scene* scene){
    scene->SetUpEnv();
    scene->SetObjs();
    scene->SetCallback();
}

void doMain(int argc,char* argv[])
{
    Scene* scene = new Scene();
    Streamer* streamer = new Streamer(*scene);
    bool dump_video_opt = false;
    int ors_gpu_id = 0;
    // -h help
    // -d dump local video
    // -gpu number e.g. -gpu 0 is using cpu
    auto parseArgv = [&](){
        int index = 0;
        if(argc <= 1) return;
        while(index < argc){
            if(std::string(argv[index]) == "-d"){
                dump_video_opt = true;
                std::cout<<"[MAIN] Dump video enable"<<std::endl;
            } 
            else if(std::string(argv[index]) == "-gpu" ) {
                if(index+1 >= argc || !(argv[index+1][0]>='0' && argv[index+1][1]<='3') ) {
                    std::cout << "gpu param wrong, using cpu to encode" << std::endl;
                }
                ors_gpu_id = (argv[index+1][0]-'0');
                std::cout<<"[MAIN] Trying to access "<<gpu_str[ors_gpu_id]<<" for encoding"<<std::endl;
                index++;
            }
            index++;
        }
        return;
    };
    parseArgv();

    SetScene(scene);
    scene->AttachStreamer(streamer);
    streamer->beginStream(dump_video_opt,ors_gpu_id);
    scene->DrawScene();
    streamer->endStream();
    scene->Terminate();

    return;
}

int main(int argc,char* argv[])
{
    doMain(argc,argv);
    return 0;
}


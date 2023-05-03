#ifndef IMAGE_H
#define IMAGE_H

namespace zrender 
{

class ZImage
{
public:
    ZImage() {
        data_ = nullptr;
        width_ = 0;
        height_ = 0;
        channels_ = 0;
    }
    ~ZImage();
    void load(const char * path);
    unsigned char * data() const;
    int width() const ;
    int height() const ;
    int channels() const;
private:
    unsigned char * data_;
    int width_;
    int height_;
    int channels_;
};


};

#endif
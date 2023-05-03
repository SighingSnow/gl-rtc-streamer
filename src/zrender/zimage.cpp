#include "zimage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace zrender;

ZImage::~ZImage(){
    if(data_) stbi_image_free(data_);
}

void ZImage::load(const char * path)
{
    data_ = stbi_load(path, &width_, &height_, &channels_, 0);
}

unsigned char * ZImage::data() const {
    return data_;
}

int ZImage::width() const {
    return width_;
}

int ZImage::height() const {
    return height_;
}

int ZImage::channels() const {
    return channels_;
}
#pragma once

#include <mutex>
#include <string>
#include "rwlock.h"
#include "shared_memory.h"
#include "common/types.hpp"

class CSharedImage {

public:

    static CSharedImage& Instance();

    // Access last frame
    void GetCompressedFrame(byte_ptr jpegImg);
    void GetBase64Frame(std::string& base64Img);
    void GetRawFrame(byte_ptr bmpImg);

    // TODO: TBD
    // Access nth frame
    void GetNthCompressedFrame(byte_ptr jpegImg, unsigned idx);
    void GetNthBase64Frame(std::string& base64Img, unsigned idx);
    void GetNthRawFrame(byte_ptr bmpImg, unsigned idx);

private:

    CSharedImage() {};
	~CSharedImage() {};

    CSharedImage(CSharedImage const&) = delete;
	CSharedImage& operator= (CSharedImage const&) = delete;

    void ImageReaderThread();
    void CompressRawImage();
    void JpegToBase64();

    byte_ptr raw_;

    std::string base64Img_;
    
    
    mutable std::mutex mutex_;
};



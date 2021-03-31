#pragma once

#include <mutex>
#include <string>
#include "rwlock.h"
#include "shared_memory.h"
#include "common/types.hpp"
#include "common/cmn.h"

#include <jpeglib.h>

class CSharedImage {

public:

    static CSharedImage& Instance();
    CSharedImage* GetPointer();

    void ImageReaderThread();

    // Access last frame
    void GetCompressedFrame(byte_ptr jpegImg) const;
    void GetBase64Frame(std::string& base64Img) const;
    void GetRawFrame(byte_ptr bmpImg) const;

    // TODO: TBD
    // Access nth frame
    void GetNthCompressedFrame(byte_ptr jpegImg, unsigned idx) const;
    void GetNthBase64Frame(std::string& base64Img, unsigned idx) const;
    void GetNthRawFrame(byte_ptr bmpImg, unsigned idx) const;

private:

    CSharedImage();
	~CSharedImage();

    CSharedImage(CSharedImage const&) = delete;
	CSharedImage& operator= (CSharedImage const&) = delete;

    void CompressRawImage();
    void JpegToBase64();
    void ReadSharedMem();

    void JpegInit();
    int JpegProc(unsigned char *outbuf, int allSize, unsigned char *image_buffer);
    int ConvertToBase64(unsigned char* buf_out, unsigned char* buf_in, int data_length);

    shm_mutex_array *img_buffer_ = nullptr;
    shm_ids_mutex   *ids_buffer_ = nullptr;

    pthread_mutex_t ipc_mutex_[SALMON_SIZE] = {0};
    pthread_mutex_t ids_mutex_ {0};

    std::string base64Img_;

    // Last frame attributes
    uint64_t id_ = 0;
    uint64_t time_ = 0;
    double lat_ = 0;
    double lon_ = 0;

    uint64_t lastTime_ = 0;
    uint64_t last_index_ = 0;

    void *imgsum_ = nullptr;
    struct jpeg_compress_struct cinfo_ {0};
    struct jpeg_error_mgr jerr_ {0};

    unsigned char left_b64_[1024*700];
    unsigned char right_b64_[1024*700];
    int left_size_ = 0;

    void *dst_jpg_mem1_ = nullptr;
    void *dst_jpg_mem2_ = nullptr;

    unsigned counter_ {};

    mutable std::mutex mutex_;
};

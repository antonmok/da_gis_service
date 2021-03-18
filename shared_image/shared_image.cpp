
#include <thread>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>

#include "shared_image.hpp"
#include "common/logger.h"
#include "base64.h"

void savePassage(char* left_fact, char* right_fact, uint64_t ttime, const char* number, cv::Mat* retLeft, cv::Mat* retRight);

CSharedImage& CSharedImage::Instance()
{
	static CSharedImage singletonInstance;
	return singletonInstance;
}

CSharedImage* CSharedImage::GetPointer()
{
    return this;
}

void CSharedImage::JpegInit()
{
	memset(&cinfo_, 0, sizeof(jpeg_compress_struct));
	memset(&jerr_, 0, sizeof(jpeg_error_mgr));
}

CSharedImage::CSharedImage()
{
    JpegInit();

    imgsum_ = malloc(IMAGE_HEIGHT * 2 * IMAGE_WIDTH);
    dst_jpg_mem1_ = malloc(IMAGE_HEIGHT * IMAGE_WIDTH);
    dst_jpg_mem2_ = malloc(IMAGE_HEIGHT * IMAGE_WIDTH);

    // frames
    int shm_id = shm_open(SALMON_PATH, O_RDONLY, S_IRUSR | S_IWUSR);

    if (shm_id < 0) {
        LOG(ERROR) << "mutexFramesInit: shm_open error: " << errno;
    }

    img_buffer_ = (shm_mutex_array*)mmap(nullptr, sizeof(shm_mutex) * SALMON_SIZE, PROT_READ, MAP_SHARED, shm_id, 0);
    if (img_buffer_ == MAP_FAILED) {
        LOG(ERROR) << "mutexFramesInit: mmap failed";
    }

    for (int i = 0; i < SALMON_SIZE; i++) {
        ipc_mutex_[i] = img_buffer_->frame[i].ipc_mutex;
    }

    // ids
    shm_id = shm_open(SALMON_IDS_PATH, O_RDONLY, S_IRUSR | S_IWUSR);

    if (shm_id < 0) {
        LOG(ERROR) << "mutexIdsInit: shm_open error: " << errno;
    }

    ids_buffer_ = (shm_ids_mutex*)mmap(nullptr, sizeof(shm_ids_mutex), PROT_READ, MAP_SHARED, shm_id, 0);
    if (ids_buffer_ == MAP_FAILED) {
        LOG(ERROR) << "mutexIdsInit: mmap failed";
    }

    ids_mutex_ = ids_buffer_->ipc_mutex;

    DLOG(INFO) << "shared frames initialized";
}

CSharedImage::~CSharedImage()
{
    jpeg_finish_compress(&cinfo_);
	jpeg_destroy_compress(&cinfo_);

    delete (byte_ptr)imgsum_;
    delete (byte_ptr)dst_jpg_mem1_;
    delete (byte_ptr)dst_jpg_mem2_;
}

int CSharedImage::JpegProc(unsigned char *outbuf, int allSize, unsigned char *image_buffer)
{
	unsigned long outsize = allSize;

	//compress
	cinfo_.err = jpeg_std_error(&jerr_);
	jpeg_create_compress(&cinfo_);

	jpeg_mem_dest(&cinfo_, &outbuf, &outsize);

	cinfo_.image_width = IMAGE_WIDTH;      /* image width and height, in pixels */
	cinfo_.image_height = IMAGE_HEIGHT;
	cinfo_.input_components = 1;     /* # of color components per pixel */
	cinfo_.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
	jpeg_set_defaults(&cinfo_);

	jpeg_start_compress(&cinfo_, true);

	JSAMPROW row_pointer[1];        /* pointer to a single row */
	int row_stride;                 /* physical row width in buffer */
	row_stride = IMAGE_WIDTH * 1;   /* JSAMPLEs per row in image_buffer */
	while ((cinfo_.next_scanline < cinfo_.image_height)) {
		row_pointer[0] = &image_buffer[cinfo_.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo_, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo_);

	jpeg_destroy_compress(&cinfo_);

	return outsize;
}

int CSharedImage::ConvertToBase64(unsigned char* buf_out, unsigned char* buf_in, int data_length)
{
    int size = Base64encode((char*)buf_out, (char*)buf_in, data_length);

    return size;
}

void CSharedImage::ImageReaderThread()
{
    while (true) {
        usleep(50 * 1000); // ~20 fps

        ReadSharedMem();

        const char* num_str = "9999";
        cv::Mat retLeft;
        cv::Mat retRight;

        savePassage((char*)imgsum_, (char*)((char*)imgsum_ + IMAGE_HEIGHT * IMAGE_WIDTH), time_, num_str, &retLeft, &retRight);

        auto t1 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds t1ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1.time_since_epoch());

        int nJpegSize1 = JpegProc((unsigned char*)dst_jpg_mem1_, IMAGE_HEIGHT * IMAGE_WIDTH, (unsigned char*)retLeft.data);
        int nJpegSize2 = JpegProc((unsigned char*)dst_jpg_mem2_, IMAGE_HEIGHT * IMAGE_WIDTH, (unsigned char*)retRight.data);

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds t2ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2.time_since_epoch());

        //DLOG_EVERY_N(INFO, 50) << "jpeg compression time: " << (t2ms - t1ms).count() << std::endl;

        int leftRes = ConvertToBase64(left_b64_, (unsigned char*)dst_jpg_mem1_, nJpegSize1);
        int rightRes = ConvertToBase64(right_b64_, (unsigned char*)dst_jpg_mem2_, nJpegSize2);

        left_size_ = leftRes;

        left_b64_[leftRes] = 0;
        right_b64_[rightRes] = 0;

        counter_++;
    }
}

void savePassage(char* left_fact, char* right_fact, uint64_t ttime, const char* number, cv::Mat* retLeft, cv::Mat* retRight)
{
  int hblackString = 200;

  cv::Mat mFullLeft(IMAGE_HEIGHT + hblackString,IMAGE_WIDTH,CV_8UC1);
  cv::Mat mFullRight(IMAGE_HEIGHT + hblackString,IMAGE_WIDTH,CV_8UC1);

  cv::Mat mLeft(IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1, (void*)left_fact);
  cv::Mat mRight(IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1, (void*)right_fact);

  mLeft.copyTo(mFullLeft(cv::Rect(0,0,IMAGE_WIDTH,IMAGE_HEIGHT)));
  mRight.copyTo(mFullRight(cv::Rect(0,0,IMAGE_WIDTH,IMAGE_HEIGHT)));

  cv::Point p0(0,IMAGE_HEIGHT), p1(IMAGE_WIDTH, IMAGE_HEIGHT + hblackString);
  cv::rectangle(mFullLeft, p0,p1, cv::Scalar(0, 0, 0),cv::FILLED);
  cv::rectangle(mFullRight, p0,p1, cv::Scalar(0, 0, 0),cv::FILLED);

  char str_left[1024] = {0};
  char str_right[1024] = {0};

  time_t tt = ttime / 1000000;
  int tt_ms = ttime - tt*1000000;
  std::tm *ptm = std::localtime(&tt);

  sprintf(str_left,"TANDEM LEFT %s %d-%02d-%02d %02d:%02d:%02d.%06d",number,ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tt_ms);

  cv::putText(mFullLeft, 
            str_left,
            cv::Point(15,IMAGE_HEIGHT + 45), // Coordinates
            cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
            2.0, // Scale. 2.0 = 2x bigger
            cv::Scalar(0, 0, 0), // BGR Color
            3); // Line Thickness (Optional)
            //cv::CV_AA); // Anti-alias (Optional)

  sprintf(str_right,"TANDEM RIGHT %s %d-%02d-%02d %02d:%02d:%02d.%06d",number,ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tt_ms);

  cv::putText(mFullRight, 
            str_right,
            cv::Point(15,IMAGE_HEIGHT + 45), // Coordinates
            cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
            2.0, // Scale. 2.0 = 2x bigger
            cv::Scalar(0, 0, 0), // BGR Color
            3); // Line Thickness (Optional)
            //cv::CV_AA); // Anti-alias (Optional)

  //cv::imwrite("/mnt/m2/blabber/hjh.jpg",mFullLeft);
  //cv::imwrite("/mnt/m2/blabber/hjh2.jpg",mFullRight);

  *retLeft = mFullLeft.clone();
  *retRight = mFullRight.clone();

  int jj = retLeft->cols;
  int jj2 = retLeft->rows;

  return;
}

void CSharedImage::CompressRawImage()
{
    std::lock_guard<std::mutex> guard(mutex_);


}

void CSharedImage::JpegToBase64()
{
    std::lock_guard<std::mutex> guard(mutex_);
}

void CSharedImage::GetCompressedFrame(byte_ptr jpegImg) const
{
    std::lock_guard<std::mutex> guard(mutex_);

    memcpy(jpegImg, dst_jpg_mem1_, left_size_);
}

void CSharedImage::GetBase64Frame(std::string& base64Img) const
{
    std::lock_guard<std::mutex> guard(mutex_);

    base64Img.assign((char*)left_b64_);

    //DLOG_EVERY_N(INFO, 50) << "Get base64, size: " << base64Img.size();
}

void CSharedImage::GetRawFrame(byte_ptr bmpImg) const
{
    std::lock_guard<std::mutex> guard(mutex_);
}

void CSharedImage::ReadSharedMem()
{
    std::lock_guard<std::mutex> guard(mutex_);

    bool res = false;
    uint64_t last_time = 0; //mcsec
    uint64_t last_index = 0;

        if (pthread_mutex_lock(&ids_mutex_) == 0) {
 
            res = true;

            last_time = ids_buffer_->last_time; //mcsec
            last_index = ids_buffer_->last_index;

            if (last_index_ != last_index) {
                last_index_ = last_index;
            } else {
                res = false;
            }

            pthread_mutex_unlock(&ids_mutex_);
        }

    if (res) {
        if (pthread_mutex_lock(&ipc_mutex_[last_index]) == 0) {

            memcpy(imgsum_, img_buffer_->frame[last_index].buf, 2 * IMAGE_HEIGHT * IMAGE_WIDTH);
            lastTime_ = img_buffer_->frame[last_index].time;

            id_     = img_buffer_->frame[last_index].id;
            time_   = img_buffer_->frame[last_index].time_gns;
            lat_    = img_buffer_->frame[last_index].lat;
            lon_    = img_buffer_->frame[last_index].lon;

            pthread_mutex_unlock(&ipc_mutex_[last_index]);
            //DLOG_EVERY_N(INFO, 50) << "get " << counter_ << " frames,  last id: " << id_ << std::endl;

        } else {
            LOG(ERROR) << "writeFrame failed to lock mutex!";
            LOG(INFO) << "last_id = " << last_index;
        }
    }
}

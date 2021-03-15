
#include <thread>
#include "shared_image.hpp"
#include "common/logger.h"


CSharedImage& CSharedImage::Instance()
{
	static CSharedImage singletonInstance;
	return singletonInstance;
}

void CSharedImage::ImageReaderThread()
{

}

void CSharedImage::CompressRawImage()
{

}

void JpegToBase64()
{

}

void GetCompressedFrame(byte_ptr jpegImg)
{

}

void GetBase64Frame(std::string& base64Img)
{

}

void GetRawFrame(byte_ptr )
{

}

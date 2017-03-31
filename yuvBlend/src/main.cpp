#include <opencv2/opencv.hpp>

#include "blender_input.hpp"
#include "edge_image_scale_blender.h"
#include "yuneec_cv_utilities.hpp"
#include <chrono>

using namespace cv;

#define TEST_MODUL 0

//parameter for camera calibration,if use different image size,
//need to change src_width src_height for thermal image,
//dst_width dst height for visible image.
const float scaleFactor = 0.86;
const int shift_x = 662;
const int shift_y = 322;

const int src_width = 640;
const int src_height = 512;
const int resizeThermalWidth = src_width * scaleFactor;
const int resizeThermalHeight  = src_height * scaleFactor;
const int thermalFrameSize = src_width * src_height * 3 / 2;

const int dst_width = 1920;
const int dst_height = 1080;
const int visibleFrameSize = dst_width * dst_height * 3 / 2;

void runFuse()
{
    //data prepare
    FILE* thermalFile = fopen("CGOT00067-1.yuv","rb+");
    FILE* visibleFile=fopen("CGOT00067.yuv","rb+");

    unsigned char* pThermalYuvBuf = new unsigned char[thermalFrameSize];
    unsigned char* pVisibleYuvBuf = new unsigned char[visibleFrameSize];

    size_t thermalSize = fread(pThermalYuvBuf,thermalFrameSize * sizeof(unsigned char),1,thermalFile );
    size_t visibleSize = fread(pVisibleYuvBuf,visibleFrameSize * sizeof(unsigned char),1,visibleFile);

    if(! thermalSize && ! visibleSize)
    {
        std::cout<<"Read image data failed !"<<std::endl;
        return;
    }
    unsigned char* dst_yuv_data[3];
    dst_yuv_data[0] = pVisibleYuvBuf;
    dst_yuv_data[1] = pVisibleYuvBuf+dst_width*dst_height;
    dst_yuv_data[2] = pVisibleYuvBuf + dst_width*dst_height + dst_width*dst_height/4;

    unsigned char* src_yuv_data[3];
    src_yuv_data[0] = pThermalYuvBuf;
    src_yuv_data[1] = pThermalYuvBuf + src_width*src_height;
    src_yuv_data[2] = pThermalYuvBuf + src_width*src_height + src_width*src_height / 4;

    std::cout<<"image size check:"<<resizeThermalWidth<<std::endl;
    //interface
    void* scale_blender_ctx = create_scale_blender_ctx(src_width, src_height, \
                                                       resizeThermalWidth, resizeThermalHeight, dst_width, dst_height, \
                                                       dst_width, dst_height, YUV420P_I420, \
                                                       shift_x, shift_y, "dog");

    scale_blend_edge_image(src_yuv_data, dst_yuv_data, scale_blender_ctx);

    release_scale_blender_ctx(scale_blender_ctx);

    std::cout<<"Run fuse image over."<<std::endl;
    std::cout<<"The processed result is parameter:"<<std::endl;
    std::cout<<"dst_yuv_data"<<std::endl;
    //---------------------------------------------
    //For test the fuse result
#if TEST_MODUL
    Mat dst_img,rgbImage;

    rgbImage.create(dst_height * 3 / 2,dst_width,CV_8UC1);
    memcpy(rgbImage.data,pVisibleYuvBuf,visibleFrameSize * sizeof(unsigned char));
    cvtColor(rgbImage, dst_img, CV_YUV2BGR_I420);
    imshow("blend", dst_img);
    waitKey();
#endif

}

int main(int argc, char** argv)
{
    auto start = std::chrono::system_clock::now();
    runFuse();

    auto end =  std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<chrono::duration<double,std::milli>>(end - start);
    std::cout<<"Cost time:"<<double(duration.count())<<std::endl;
    return 0;

}

//Test camera calibratiob lib
#include "computeScale.hpp"
#include <memory>

//void runComputeShift(cv::Mat thermal,cv::Mat visible)
//void runComputeShift()
//{

//}

int main(int argc, char** argv)
{
    //parameter num chaeck
    if(argc < 3)
    {
            std::cout<<"parameter num ERROR"<<std::endl;
            std::cout<<"Usage: ./main  ./thermalImage ./visibleImage"<<std::endl;
    }

//    //read the yuv image data
//    FILE* thermalFile = fopen("thermal.yuv","rb+");
//    FILE* visibleFile = fopen("visible.yuv","rb+");

//    Mat thermalImage = imageConvert(thermalFile,thermalWidth,thermalHeight);
//    Mat visibleImage = imageConvert(visibleFile,visibleWidth,visibleHeight);
    //runComputeShift();
    return 0;
}

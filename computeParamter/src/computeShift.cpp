//Test camera calibratiob lib
#include "computeShift.hpp"
#include <memory>

int main(int argc, char** argv)
{
    //parameter num chaeck
    if(argc < 3)
    {
            std::cout<<"parameter num ERROR"<<std::endl;
            std::cout<<"Usage: ./main  ./thermalImage ./visibleImage"<<std::endl;
    }

    return 0;
}

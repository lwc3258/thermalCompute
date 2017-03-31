/*
 * @brief   :   image process module
 * @creater :   hw.zhu
 * @data    :   2016/01/25
 * @version :   0.1
*/

#ifndef YUNEEC_CV_IMGPROC_H
#define YUNEEC_CV_IMGPROC_H

#include <stdio.h>
#include <string>
using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;


#ifdef __cplusplus    
extern "C" {
#endif


enum enhancer_method{ EE_CANNY=0, EE_LAPLACIAN, EE_SOBEL};

class EdgeEnhancer
{
public:
        int method_;
public:
        EdgeEnhancer();
        EdgeEnhancer(int method);
        void edge_enhance(unsigned char *yuv_data, int width, int height, int yuv_format,
                const Rect& roi, int channel_num = 1, bool display=false);

        static int method_type(const string& method_name);

        void edge_enhance_canny(Mat& yuv_img, const Rect& roi, unsigned char edge_thresh, int channel_num = 1);
        void edge_enhance_laplacian(Mat& yuv_img, const Rect& roi, int channel = 1);
        void edge_enhance_sobel(Mat& yuv_img, const Rect& roi, int channel_num);
};


#ifdef __cplusplus
}
#endif

#endif // YUNEEC_CV_IMGPROC_H

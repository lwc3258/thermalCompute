/*
 * @brief   :   utility module
 * @creater :   hw.zhu
 * @data    :   2016/01/25
 * @version :   0.1
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

#ifdef __cplusplus    
extern "C" {
#endif

//YUV420P format
enum
{
         YUV420P_I420 = 0, YUV420SP
};

/*
 * function :   convert yuv to mat
 * input    :   yuv_data -- yuv data
 *          :   width -- resolution width
 *          :   height -- resolution height
 *          :   yuv_format -- yuv format
 * output   :   Mat -- opencv Mat
*/
cv::Mat yuneec_cv_yuv2mat(uchar *yuv_data, int width, int height, int yuv_format);

cv::Rect get_center_rect(const Size& input_size, const Size& range);

int yuv_image_format(const string& format_name);

void image_minus(const Mat& src1, const Mat& src2, Mat& dst);

void SaveShortIntMat(const Mat& mat, const string& sv_path);

void SaveUCharMat(const Mat& mat, const string& sv_path);


#ifdef __cplusplus
}
#endif


#endif // UTILITIES_H

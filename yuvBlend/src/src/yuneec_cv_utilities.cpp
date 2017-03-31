/*
 * @brief   :   utility module
 * @creater :   hw.zhu
 * @data    :   2016/01/25
 * @version :   0.1
*/

#include <iostream>
#include <fstream>
#include "yuneec_cv_utilities.hpp"
#include "edge_enhancer.hpp"


#ifdef ANDROID
#include <android/log.h>
#define TAG "ffplay"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif


using namespace std;
using namespace cv;

cv::Mat yuneec_cv_yuv2mat(uchar *yuv_data, int width, int height, int yuv_format)
{
    CV_Assert(yuv_data != NULL && width > 0 && height > 0);

    Mat yuv_img;
    switch(yuv_format)
    {
    case YUV420P_I420:
        {
            //yuv_img.create(height * 3 / 2, width, CV_8UC1);
            //yuv_img.data = yuv_data;
            yuv_img = Mat(height * 3 / 2, width, CV_8UC1, yuv_data);
            break;
        }
    default:
        {
            cout << "[yuneec_cv_yuv2mat] yuv420p:this format not supported yet!" << endl;
            break;
        }
    }

    return yuv_img;

}



Rect get_center_rect(const Size& input_size, const Size& range)
{
        Rect center_rect;
        center_rect.width = input_size.width <= range.width ? input_size.width : range.width;
        center_rect.height = input_size.height <= range.height ? input_size.height : range.height;

        center_rect.x = range.width / 2 - center_rect.width / 2;
        center_rect.y = range.height / 2 - center_rect.height / 2;

        return center_rect;
}



int yuv_image_format(const string& format_name)
{
        if (format_name == "YUV420P_I420")
        {
                return YUV420P_I420;
        }
        else
        {
                return -1;
        }
}



void image_minus(const Mat& src1, const Mat& src2, Mat& dst)
{
	//LOGV("image_minus\n");

	assert(src1.data&&src2.data&&dst.data);
	assert((src1.cols == src2.cols) && (src1.rows == src2.rows) && (src1.rows == dst.rows) && (src1.cols == dst.cols));
	
	//LOGV("src_width: %d\t src_height: %d\n", src1.cols, src1.rows);
	//if (!dst.data)
	//{
	//	dst.create(src1.rows, src1.cols, CV_16SC1);
	//}

	unsigned char* src1_data = src1.data;
	unsigned char* src2_data = src2.data;
	short int* dst_data = (short int*)(dst.data);

	for (int r = 0; r < src1.rows; r++)
	{
		for (int c = 0; c < src1.cols; c++)
		{
			//LOGV("%d == %d\n", src1_data[r*src1.cols + c], src1.at<uchar>(r,c));
			//LOGV("%d == %d\n", src2_data[r*src2.cols + c], src2.at<uchar>(r,c));
			//dst_data[r*dst.cols + c] = src1_data[r*src1.cols + c] - src2_data[r*src2.cols + c];
			dst_data[r*dst.cols + c] = src1.at<uchar>(r, c) - src2.at<uchar>(r, c);
			//LOGV("%d - %d = %d", src1_data[r*src1.cols + c], src2_data[r*src2.cols + c], dst_data[r*dst.cols + c]);
		}
	}

	return;
}


void SaveShortIntMat(const Mat& mat, const string& sv_path)
{
        fstream out_file(sv_path.c_str(), ios::out);
        for (int i = 0; i < mat.rows; i++)
        {
                for (int j = 0; j < mat.cols; j++)
                {
                        out_file << mat.at<short int>(i, j) << "\t";
                }
                out_file << endl;
        }
        out_file.close();

        return;
}


void SaveUCharMat(const Mat& mat, const string& sv_path)
{
        fstream out_file(sv_path.c_str(), ios::out);
        for (int i = 0; i < mat.rows; i++)
        {
                for (int j = 0; j < mat.cols; j++)
                {
                        out_file << (int)(mat.at<uchar>(i, j)) << "\t";
                }
                out_file << endl;
        }
        out_file.close();

        return;
}

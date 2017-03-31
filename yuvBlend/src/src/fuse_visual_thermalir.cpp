
#include <stdio.h>
#include <stdlib.h>
#include "canny_edge_extractor.h"
#include "laplacian_edge_extractor.h"
#include "sobel_edge_extractor.h"
#include "dog_edge_extractor.h"
#include "yuneec_cv_utilities.hpp"
#include "fuse_visual_thermalir.h"

#ifdef ANDROID
#include <android/log.h>
#define TAG "ffplay"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif

//assumption: edge is the overlap of src and dst
static void copy_yuv420(unsigned char** src_yuv, int src_width, int src_height,
        unsigned char** dst_yuv, int dst_width, int dst_height, const int shift_x, const int shift_y,
        const Mat& edge_y)
{
        unsigned char* src_y = src_yuv[0];
        unsigned char* src_u = src_yuv[1];
        unsigned char* src_v = src_yuv[2];

        unsigned char* dst_y = dst_yuv[0];
        unsigned char* dst_u = dst_yuv[1];
        unsigned char* dst_v = dst_yuv[2];

        short int* edge_data = (short int*)(edge_y.data);

        assert(src_y&&src_u&&src_v);
        assert(dst_y&&dst_u&&dst_v);
        assert((src_width > 0) && (src_height > 0) && (dst_width > 0) && (dst_height > 0));
	
	LOGV("blend y channel\n");
        //Y
	int blend_width = edge_y.cols;
	int blend_height = edge_y.rows; 
        for (int src_r = 0; src_r < blend_height; src_r++)
        {
                int dst_r = shift_y + src_r;
                if (dst_r >= dst_height)
                {
                        break;
                }

                for (int src_c = 0; src_c < blend_width; src_c++)
                {
                        int dst_c = shift_x + src_c;
                        if (dst_c >= dst_width)
                        {
                                break;
                        }

                        int dst_value = src_y[src_r*src_width + src_c] + edge_data[src_r*edge_y.cols + src_c];
                        
                        if (dst_value < 0)
                        {
                                dst_y[dst_r*dst_width + dst_c] = 0;
                        }
                        else if (dst_value > 255)
                        {
                                dst_y[dst_r*dst_width + dst_c] = 255;
                        }
                        else
                        {
                                dst_y[dst_r*dst_width + dst_c] = dst_value;
                        }

                        //dst_u[(dst_r / 2)*(dst_width / 2) + dst_c / 2] = src_u[(src_r / 2)*(src_width / 2) + src_c / 2];
                        //dst_v[(dst_r / 2)*(dst_width / 2) + dst_c / 2] = src_v[(src_r / 2)*(src_width / 2) + src_c / 2];
                }

        }

        //U&V
	LOGV("blend u&v channel\n");
        int src_uv_width = src_width / 2;
        int src_uv_height = src_height / 2;
        int dst_uv_width = dst_width / 2;
        int dst_uv_height = dst_height / 2;
        int shift_uv_x = shift_x / 2;
        int shift_uv_y = shift_y / 2;

	int blend_uv_width = blend_width/2;
	int blend_uv_height = blend_height/2;

	int row_copy_len = (blend_uv_width <= (dst_uv_width - shift_uv_x)) ? blend_uv_width : (dst_uv_width - shift_uv_x);

        for (int src_r = 0; src_r < blend_uv_height; src_r++)
        {
                int dst_r = shift_uv_y + src_r;
                if (dst_r >= dst_uv_height)
                {
                        break;
                }

                memcpy(dst_u + dst_r*dst_uv_width + shift_uv_x, src_u + src_r*src_uv_width, row_copy_len);
                memcpy(dst_v + dst_r*dst_uv_width + shift_uv_x, src_v + src_r*src_uv_width, row_copy_len);
        }
        return;
}


static void copy_yuv(unsigned char** src_yuv, int src_width, int src_height, int src_format,
        unsigned char** dst_yuv, int dst_width, int dst_height, int dst_format, const int shift_x, const int shift_y, 
        const Mat& edge_y)
{
        assert(src_format == dst_format);
        switch (src_format)
        {
        case YUV420P_I420:
                copy_yuv420(src_yuv, src_width, src_height, dst_yuv, dst_width, dst_height, shift_x, shift_y,
                        edge_y);
                break;
        default:
		LOGV("invalid foramt!\n");
                break;
        }

        return;
}


extern "C"
{

void fuse_visual_thermalir(unsigned char** visual_yuv, int visual_width, int visual_height, int visual_format,
        unsigned char** ir_yuv, int ir_width, int ir_height, int ir_format, int shift_x, int shift_y)
{
	int64 begin_time = cv::getTickCount();

	LOGV("start extracting edge\n");
	DoGEdgeExtractor dee;

        //extract edge on y channel
	int roi_y_width = ir_width<=(visual_width-shift_x) ? ir_width : (visual_width-shift_x);
	int roi_y_height = ir_height<=(visual_height-shift_y) ? ir_height : (visual_height-shift_y);
        Rect roi_y(shift_x, shift_y, roi_y_width, roi_y_height);

        Mat visual_y = Mat(visual_height, visual_width, CV_8UC1, visual_yuv[0]);
        Mat edge_y;
        Mat visual_y_roi = visual_y(roi_y);
	//GaussianBlur(visual_y_roi, visual_y_roi, Size(3, 3), 0, 0);
        dee.extract_edge(visual_y_roi, edge_y);
	LOGV("extracted edge!\n");

        copy_yuv(ir_yuv, ir_width, ir_height, ir_format, visual_yuv, visual_width, visual_height, visual_format, shift_x, shift_y, edge_y);

	int64 end_time = cv::getTickCount();
	double time = (double)(end_time-begin_time)*1000/getTickFrequency();
	LOGV("blending, time elapsed(ms): %lf\n", time);

        return;
}

}

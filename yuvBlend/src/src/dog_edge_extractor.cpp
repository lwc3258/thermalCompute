
#include "dog_edge_extractor.h"
#include "yuneec_cv_utilities.hpp"


#ifdef ANDROID
#include <android/log.h>
#define TAG "ffplay"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif



DoGEdgeExtractor::DoGEdgeExtractor() :EdgeExtractor(EEDGE_DOG)
{
}

DoGEdgeExtractor::DoGEdgeExtractor(const int width , const int height) :EdgeExtractor(EEDGE_DOG)
{
	blur_.create(height, width, CV_8UC1);
}


void DoGEdgeExtractor::extract_edge(const Mat& img, Mat& edge)
{
	//int64 begin_time_gb3 = cv::getTickCount();

        //Mat small_blur;
	//GaussianBlur(img, small_blur, Size(3, 3), 0, 0);

	//int64 end_time_gb3 = cv::getTickCount();
	//double time_gb3 = (double)(end_time_gb3-begin_time_gb3)*1000/getTickFrequency();
	//LOGV("gaussian blur 3*3, time elapsed(ms): %lf\n", time_gb3);

	//int64 begin_time_gb19 = cv::getTickCount();
	//Mat big_blur;
        //GaussianBlur(img, big_blur, Size(19, 19), 0, 0);

	//int64 end_time_gb19 = cv::getTickCount();
	//double time_gb19 = (double)(end_time_gb19-begin_time_gb19)*1000/getTickFrequency();
	//LOGV("gaussian blur 19*19, time elapsed(ms): %lf\n", time_gb19);

	int64 begin_time_gb15 = cv::getTickCount();
	//Mat blur;
        GaussianBlur(img, blur_, Size(15, 15), 0, 0);

	int64 end_time_gb15 = cv::getTickCount();
	double time_gb15 = (double)(end_time_gb15-begin_time_gb15)*1000/getTickFrequency();
	LOGV("gaussian blur 15*15, time elapsed(ms): %lf\n", time_gb15);

	
	//int64 begin_time_gb11 = cv::getTickCount();
	//Mat big_blur;
        //GaussianBlur(img, big_blur, Size(11, 11), 0, 0);

	//int64 end_time_gb11 = cv::getTickCount();
	//double time_gb11 = (double)(end_time_gb11-begin_time_gb11)*1000/getTickFrequency();
	//LOGV("gaussian blur 11*11, time elapsed(ms): %lf\n", time_gb11);

	
	//int64 begin_time_add = cv::getTickCount();

        //addWeighted(img, 1, big_blur, -1, 0, edge, CV_16SC1);
	//SaveShortIntMat(edge2, "./sdcard/edge2.txt");
	
	image_minus(img, blur_, edge);
	//SaveUCharMat(img, "./sdcard/img.txt");
	//SaveUCharMat(big_blur, "./sdcard/blur.txt");
	//SaveShortIntMat(edge, "./sdcard/edge.txt");

	//int64 end_time_add = cv::getTickCount();
	//double time_add = (double)(end_time_add-begin_time_add)*1000/getTickFrequency();
	//LOGV("add mat, time elapsed(ms): %lf\n", time_add);
}


DoGEdgeExtractor::~DoGEdgeExtractor()
{
	if(blur_.data)
	{
		blur_.release();
	}
}


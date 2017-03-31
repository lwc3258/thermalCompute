

#include "yuneec_cv_utilities.hpp"
#include "edge_image_blender.h"
#include "edge_extractor.h"
#include "canny_edge_extractor.h"
#include "laplacian_edge_extractor.h"
#include "sobel_edge_extractor.h"
#include "dog_edge_extractor.h"


#ifdef ANDROID
#include <android/log.h>
#define TAG "ffplay"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif


EdgeImageBlender::EdgeImageBlender(const int src_width, const int src_height, const int dst_width, const int dst_height, 
	int image_format, const int shift_x, const int shift_y, const string& edge_extractor_method) 
	:src_width_(src_width), src_height_(src_height), dst_width_(dst_width), dst_height_(dst_height), image_format_(image_format),
	shift_x_(shift_x), shift_y_(shift_y)
{
	//LOGV("1----------");
	assert(src_width_ > 0 && src_height_ > 0 && dst_width_ > 0 && dst_height > 0);
	// case of 'shift_x < 0' is not concerned 
	assert(shift_x_ >= 0 && shift_y_ >= 0);

	src_uv_width_ = src_width_ / 2;
	src_uv_height_ = src_height_ / 2;

	dst_uv_width_ = dst_width_ / 2;
	dst_uv_height_ = dst_height_ / 2;

	shift_uv_x_ = shift_x_ / 2;
	shift_uv_y_ = shift_y_ / 2;
	
	int roi_y_width = src_width <= (dst_width - shift_x) ? src_width : (dst_width - shift_x);
	int roi_y_height = src_height <= (dst_height - shift_y) ? src_height : (dst_height - shift_y);

	//LOGV("2----------");
	assert(roi_y_width > 0 && roi_y_height > 0);

	overlap_roi_ = Rect(shift_x, shift_y, roi_y_width, roi_y_height);
	overlap_roi_x_ = shift_x_;
	overlap_roi_y_ = shift_y_;
	overlap_roi_width_ = roi_y_width;
	overlap_roi_height_ = roi_y_height;

	//LOGV("3----------shift_x:%d shift_y:%d roi_y_width: %d  roi_y_height: %d",shift_x_, shift_y_, roi_y_width, roi_y_height);
	//edge_y_.create(roi_y_height, roi_y_width, CV_16SC1);
	//blur_.resize(overlap_roi_width_*overlap_roi_height_);
	//LOGV("3---------1-");
	blur_img_.create(overlap_roi_height_, overlap_roi_width_, CV_8UC1);
	//LOGV("3---------2-%s", edge_extractor_method.c_str());
	//gaussian_blur_ptr_ = new yuneec_cv::GaussianBlur(dst_width_, dst_height_, overlap_roi_x_, overlap_roi_y_, \
		overlap_roi_width_, overlap_roi_height_,7, 7, 5, 5);
	
	//CreateEdgeExtractor(edge_extractor_method);
	pedge_extractor_ = NULL;
	//LOGV("3----------");
}


void EdgeImageBlender::CreateEdgeExtractor(const string& edge_extractor_method)
{
	int method_type = EdgeExtractor::method_type(edge_extractor_method);
	switch (method_type)
	{
	case EEDGE_CANNY:
		pedge_extractor_ = new CannyEdgeExtractor(50, 100);
		break;
	case EEDGE_LAPLACIAN:
		pedge_extractor_ = new LaplacianEdgeExtractor();
		break;
	case EEDGE_SOBEL:
		pedge_extractor_ = new SobelEdgeExtrator();
		break;
	case EEDGE_DOG:
		pedge_extractor_ = new DoGEdgeExtractor();
		break;
	default:
		pedge_extractor_ = NULL;
	}

	return;
}


void EdgeImageBlender::ReleaseEdgeExtractor(void)
{

	//release edge
	//if (!edge_y_.data)
	//{
	//	edge_y_.release();
	//}

	if (blur_img_.data)
	{
		blur_img_.release();
	}

	//release edge extractor
	if (pedge_extractor_)
	{
		delete pedge_extractor_;
	}
}


//input: pointer of image data
void EdgeImageBlender::blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data)
{
        switch (image_format_)
        {
        case YUV420P_I420:
                blend_edge_image_yuv(src_img_data, dst_img_data);
                break;
        default:
	case YUV420SP:
		blend_edge_image_yuv420sp(src_img_data, dst_img_data);
                break;
        }
}


void EdgeImageBlender::blend_edge_image_yuv(unsigned char** src_img_data, unsigned char** dst_img_data)
{
	//extract edge on y channel
	Mat visual_y = Mat(dst_height_, dst_width_, CV_8UC1, dst_img_data[0]);
	//Mat edge_y;
	Mat visual_y_roi = visual_y(overlap_roi_);
	//pedge_extractor_->extract_edge(visual_y_roi, edge_y_);
	//cv::filter2D(visual_y_roi, edge_y_, edge_y_.depth(), kernel_);


	// blur roi image patch
	//unsigned char* blur_data_ptr = &(blur_[0]);
	//gaussian_blur_ptr_->blur(dst_img_data[0], blur_data_ptr);

	GaussianBlur(visual_y_roi, blur_img_, Size(7, 7), 5, 5);
	unsigned char* blur_data_ptr = blur_img_.data;
	

	//blend on channel y
	for (int r = 0; r < overlap_roi_height_; r++)
	{
		int dst_r = r + shift_y_;
		for (int c = 0; c < overlap_roi_width_; c++)
		{
			int dst_value = src_img_data[0][r*src_width_ + c] + dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] 
									- blur_data_ptr[r*overlap_roi_width_+c];
			 if (dst_value < 0)
			 {
				 dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = 0;
			 }
			 else if (dst_value > 255)
			 {
				 dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = 255;
			 }
			 else
			 {
				 dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = dst_value;
			 }	 
		}
	}

	// blend on uv channel
	for (int r = 0; r < overlap_roi_height_/2; r++)
	{
		int dst_r = shift_uv_y_ + r;

		memcpy(dst_img_data[1] + dst_r*dst_uv_width_ + shift_uv_x_, src_img_data[1] + r*src_uv_width_, overlap_roi_width_ / 2);

		memcpy(dst_img_data[2] + dst_r*dst_uv_width_ + shift_uv_x_, src_img_data[2] + r*src_uv_width_, overlap_roi_width_ / 2);
	}

	return;
}


void EdgeImageBlender::blend_edge_image_yuv420sp(unsigned char** src_img_data, unsigned char** dst_img_data)
{
	Mat visual_y = Mat(dst_height_, dst_width_, CV_8UC1, dst_img_data[0]);
	Mat visual_y_roi = visual_y(overlap_roi_);

	// blur roi image patch
	GaussianBlur(visual_y_roi, blur_img_, Size(9, 9), 5, 5);
	unsigned char* blur_data_ptr = blur_img_.data;
	
	//unsigned char* blur_data_ptr = &(blur_[0]);
	//gaussian_blur_ptr_->blur(dst_img_data[0], blur_data_ptr);

	//blend on channel y
	for (int r = 0; r < overlap_roi_height_; r++)
	{
		int dst_r = r + shift_y_;
		for (int c = 0; c < overlap_roi_width_; c++)
		{
			int dst_value = src_img_data[0][r*src_width_ + c] + dst_img_data[0][dst_r*dst_width_ + shift_x_ + c]
				- blur_[r*overlap_roi_width_ + c];
			if (dst_value < 0)
			{
				dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = 0;
			}
			else if (dst_value > 255)
			{
				dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = 255;
			}
			else
			{
				dst_img_data[0][dst_r*dst_width_ + shift_x_ + c] = dst_value;
			}
		}
	}

	// blend on uv channel size(width, height/2)
	for (int r = 0; r < overlap_roi_height_/2; r++)
	{
		int dst_r = shift_uv_y_ + r;

		memcpy(dst_img_data[1] + dst_r*dst_width_ + shift_x_, src_img_data[1] + r*src_width_, overlap_roi_width_);
	}

}


void EdgeImageBlender::blend_edge_ir(unsigned char* src, unsigned char* dst)
{
	//unsigned char* blur_data_ptr = &(blur_[0]);
	//gaussian_blur_ptr_->blur(dst, blur_data_ptr);

    //auto start = std::chrono::system_clock::now();
	//Mat blur;
	Mat dst_img(dst_height_, dst_width_, CV_8UC1, dst);
	GaussianBlur(dst_img(Rect(overlap_roi_x_, overlap_roi_y_, overlap_roi_width_, overlap_roi_height_)), blur_img_, Size(5, 5), 2, 2);
	unsigned char* blur_data_ptr = blur_img_.data;
    //imwrite("/sdcard/Movies/dst_roi.jpg", dst_img(Rect(overlap_roi_x_, overlap_roi_y_, overlap_roi_width_, overlap_roi_height_)));
    //imwrite("/sdcard/Movies/dst_roi_blur.jpg", blur_img_);
	//LOGV("test\n");
	//while(1);
	//blend on channel y
	/*
	for (int r = 0; r < overlap_roi_height_; r++)
	{
		int dst_r = r + shift_y_;
		for (int c = 0; c < overlap_roi_width_; c++)
		{
			int dst_value = src[r*src_width_ + c] + 2*(dst[dst_r*dst_width_ + shift_x_ + c]
				- blur_data_ptr[r*overlap_roi_width_ + c]);
			if (dst_value < 0)
			{
				src[r*src_width_ + c] = 0;
			}
			else if (dst_value > 255)
			{
				src[r*src_width_ + c] = 255;
			}
			else
			{
				src[r*src_width_ + c] = dst_value;
			}
		}
	}
	//*/
	//*
	for (int r = 0; r < overlap_roi_height_; r++)
	{
		int dst_r = r + shift_y_;
		int c = 0;
		for (; c <= overlap_roi_width_-4; c+=4)
		{
			int c0 = c, c1 = c + 1, c2 = c + 2, c3 = c + 3;
			int dst_value0 = src[r*src_width_ + c0] + 2 * (dst[dst_r*dst_width_ + shift_x_ + c0]
				- blur_data_ptr[r*overlap_roi_width_ + c0]);
			int dst_value1 = src[r*src_width_ + c1] + 2 * (dst[dst_r*dst_width_ + shift_x_ + c1]
				- blur_data_ptr[r*overlap_roi_width_ + c1]);
			int dst_value2 = src[r*src_width_ + c2] + 2 * (dst[dst_r*dst_width_ + shift_x_ + c2]
				- blur_data_ptr[r*overlap_roi_width_ + c2]);
			int dst_value3 = src[r*src_width_ + c3] + 2 * (dst[dst_r*dst_width_ + shift_x_ + c3]
				- blur_data_ptr[r*overlap_roi_width_ + c3]);
			src[r*src_width_ + c0] = saturate_uchar(dst_value0);
			src[r*src_width_ + c1] = saturate_uchar(dst_value1);
			src[r*src_width_ + c2] = saturate_uchar(dst_value2);
			src[r*src_width_ + c3] = saturate_uchar(dst_value3);
		}

		for (; c < overlap_roi_width_; c++)
		{
			int dst_value = src[r*src_width_ + c] + 2 * (dst[dst_r*dst_width_ + shift_x_ + c]
				- blur_data_ptr[r*overlap_roi_width_ + c]);

			src[r*src_width_ + c] = saturate_uchar(dst_value);
		}
	}
	//*/
	
}



EdgeImageBlender::~EdgeImageBlender()
{
	//if (edge_y_.data)
	//{
	//	edge_y_.release();
	//}

	//if (kernel_.data)
	//{
	//	kernel_.release();
	//}

	//if (gaussian_blur_ptr_)
	//{
	//	delete gaussian_blur_ptr_;
	//}
	ReleaseEdgeExtractor();
}

extern "C"
{
	//for using in c code
	typedef struct blender_ctx
	{
		EdgeImageBlender* pblender;
	} BLENDER_CTX;


	void* create_blender_ctx(const int src_width, const int src_height, const int dst_width, const int dst_height,
		int image_format, const int shift_x, const int shift_y, const string& edge_extractor_method)
	{
		//LOGV("create_blender_ctx\n");
		BLENDER_CTX* ctx = new BLENDER_CTX;
		assert(ctx);

		ctx->pblender = new EdgeImageBlender(src_width, src_height, dst_width, dst_height, image_format, shift_x, shift_y, edge_extractor_method);
		//LOGV(" end create_blender_ctx\n");
		return ctx;
	}


	void release_blender_ctx(void* ctx)
	{
		if (!ctx)
		{
			return;
		}

		BLENDER_CTX* blender_ctx = (BLENDER_CTX*)ctx;

		if (blender_ctx->pblender)
		{
			delete blender_ctx->pblender;
			blender_ctx->pblender = NULL;
		}

		//release ctx
		delete blender_ctx;

		return;
	}


	Rect get_overlap_roi(void* ctx)
	{
		assert(ctx);
		BLENDER_CTX* blender_ctx = (BLENDER_CTX*)ctx;

		return Rect(blender_ctx->pblender->overlap_roi_x_,
			blender_ctx->pblender->overlap_roi_y_,
			blender_ctx->pblender->overlap_roi_width_,
			blender_ctx->pblender->overlap_roi_height_);
	}


	void blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data, void* ctx)
	{
		assert(ctx);
		BLENDER_CTX* blender_ctx = (BLENDER_CTX*)ctx;

		assert(blender_ctx->pblender);
		blender_ctx->pblender->blend_edge_image(src_img_data, dst_img_data);

		return;
	}


	void blend_edge_ir(unsigned char* src_img_data, unsigned char* dst_img_data, void* ctx)
	{
		assert(ctx);
		BLENDER_CTX* blender_ctx = (BLENDER_CTX*)ctx;

		assert(blender_ctx->pblender);
		blender_ctx->pblender->blend_edge_ir(src_img_data, dst_img_data);

		return;
	}
}


#include "edge_extractor.h"
//#include "gaussian_blur.h"

class EdgeImageBlender
{
public:
	// input image data is always same in video
	int src_width_;
	int src_height_;
	int src_uv_width_;
	int src_uv_height_;

	int dst_width_;
	int dst_height_;
	int dst_uv_width_;
	int dst_uv_height_;

	int image_format_;

	// distance between two image
	int shift_x_;
	int shift_y_;

	int shift_uv_x_;
	int shift_uv_y_;

	// overlap region between two image
	Rect overlap_roi_;
	int overlap_roi_x_;
	int overlap_roi_y_;
	int overlap_roi_width_;
	int overlap_roi_height_;


	vector<unsigned char> blur_;

	Mat blur_img_;

	//yuneec_cv::GaussianBlur* gaussian_blur_ptr_;

	// edge extractor
        EdgeExtractor* pedge_extractor_;
public:
	EdgeImageBlender(const int src_width, const int src_height, const int dst_width, const int dst_height, int image_format,
		const int shift_x, const int shift_y, const string& edge_extractor_method);

	void CreateEdgeExtractor(const string& edge_extractor_method);

	void blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data);

	void blend_edge_image_yuv(unsigned char** src_img_data, unsigned char** dst_img_data);

	void blend_edge_image_yuv420sp(unsigned char** src_img_data, unsigned char** dst_img_data);

	void blend_edge_ir(unsigned char* src, unsigned char* dst);

	unsigned char saturate_uchar(int value)
	{
		return (unsigned char)((unsigned)value <= UCHAR_MAX ? value : value > 0 ? UCHAR_MAX : 0);
	}

	void ReleaseEdgeExtractor(void);

	~EdgeImageBlender();
};

extern "C" void* create_blender_ctx(const int src_width, const int src_height, const int dst_width, const int dst_height,
	int image_format, const int shift_x, const int shift_y, const string& edge_extractor_method);

extern "C" void release_blender_ctx(void* ctx);

extern "C" void blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data, void* ctx);

extern "C" Rect get_overlap_roi(void* ctx);

extern "C" void blend_edge_ir(unsigned char* src_img_data, unsigned char* dst_img_data, void* ctx);

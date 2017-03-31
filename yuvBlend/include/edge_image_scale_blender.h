
#include <string>

using namespace std;

extern "C" void* create_scale_blender_ctx( int src_width, int src_height, int scale_width, int scale_height, int dst_width,
	 int dst_height, int scale_dst_width, int scale_dst_height, int image_format, int shift_x, int shift_y, const string& edge_extractor_method);

extern "C" void release_scale_blender_ctx(void* ctx);

extern "C" void scale_blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data, void* ctx);

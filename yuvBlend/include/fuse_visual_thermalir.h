
extern "C"
{
void fuse_visual_thermalir(unsigned char** visual_yuv, int visual_width, int visual_height, int visual_format,
        unsigned char** ir_yuv, int ir_width, int ir_height, int ir_format, int shift_x, int shift_y);
}


#include "edge_image_blender.h"
#include "opencv_utilities.h"
#include "yuneec_cv_utilities.hpp"


#ifdef ANDROID
#include <android/log.h>
#define TAG "ffplay"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#else
#define LOGV(...)
#endif

extern "C"
{
//for using in c code
typedef struct scale_blender_ctx
{
    int image_format_;

    int src_width_;
    int src_height_;

    int resized_src_width_;
    int resized_src_height_;

    int dst_width_;
    int dst_height_;

    int resized_dst_width_;
    int resized_dst_height_;

    int blend_src_width_;
    int blend_src_height_;

    //yuv
    int yuv_channel_num_;
    unsigned char** resized_src_;

    int shift_x_;
    int shift_y_;

    Rect overlap_roi_;

    unsigned char* resized_dst_;
    unsigned char* blend_src_;
    unsigned char* resized_blend_src_;
    void* blender_ctx_;

} SCALE_BLENDER_CTX;


unsigned char** create_resized_ir_yuv420(int width, int height)
{
    unsigned char** ir = new unsigned char*[3];

    ir[0] = new unsigned char[width*height];
    ir[1] = new unsigned char[width / 2 * height / 2];
    ir[2] = new unsigned char[width / 2 * height / 2];

    return ir;
}


void release_resized_ir_yuv420(unsigned char** ir)
{
    if (!ir)
    {
        return;
    }

    delete ir[0];
    delete ir[1];
    delete ir[2];

    delete ir;
}


unsigned char** create_resized_ir_yuv420sp(int width, int height)
{
    unsigned char** ir = new unsigned char*[2];

    ir[0] = new unsigned char[width*height];
    ir[1] = new unsigned char[width*height/2];

    return ir;
}


void release_resized_ir_yuv420sp(unsigned char** ir)
{
    if (!ir)
    {
        return;
    }

    delete ir[0];
    delete ir[1];

    delete ir;
}


unsigned char** create_resized_ir(int width, int height, int image_format)
{
    assert(width >= 0 && height >= 0);
    switch (image_format)
    {
    case YUV420P_I420:
        return create_resized_ir_yuv420(width, height);
        break;
    case YUV420SP:
        return create_resized_ir_yuv420sp(width, height);
        break;
    default:
        return NULL;
        break;
    }

    return NULL;
}


void release_resized_ir(unsigned char** ir, int image_format)
{
    switch (image_format)
    {
    case YUV420P_I420:
        release_resized_ir_yuv420(ir);
        break;
    case YUV420SP:
        release_resized_ir_yuv420sp(ir);
        break;
    default:
        break;
    }

    return;
}


void* create_scale_blender_ctx(int src_width, int src_height, int scale_width, int scale_height,
                               int dst_width, int dst_height, int scale_dst_width, int scale_dst_height, int image_format, int shift_x, int shift_y,
                               const string& edge_extractor_method)
{
    assert(src_width > 0 && src_height > 0 && scale_width > 0 && scale_height > 0);
    assert(dst_width > 0 && dst_height > 0 && scale_dst_width > 0 && scale_dst_height > 0);

    SCALE_BLENDER_CTX* ctx = new SCALE_BLENDER_CTX;
    assert(ctx);

    ctx->image_format_ = image_format;
    ctx->src_width_ = src_width;
    ctx->src_height_ = src_height;

    ctx->resized_src_width_ = scale_width;
    ctx->resized_src_height_ = scale_height;

    ctx->dst_width_ = dst_width;
    ctx->dst_height_ = dst_height;

    ctx->resized_dst_width_ = scale_dst_width;
    ctx->resized_dst_height_ = scale_dst_height;

    // visual image
    float dst_scale_rate = 1.0f;
    ctx->resized_dst_ = NULL;
    bool dst_resized = (dst_width != scale_dst_width || dst_height != scale_dst_height);
    if (dst_resized)
    {
        float width_scale_rate = (float)scale_dst_width / dst_width;
        float height_scale_rate = (float)scale_dst_height / dst_height;        
        assert(fabs(width_scale_rate - height_scale_rate) < FLT_EPSILON);
        dst_scale_rate = width_scale_rate;

        ctx->resized_dst_ = new unsigned char[scale_dst_width*scale_dst_height];
    }

    float src_scale_rate = 1.0f;
    ctx->resized_src_ = NULL;
    bool src_resized = (src_width != scale_width || src_height != scale_height);
    if (src_resized)
    {
        float width_scale_rate = (float)scale_width / src_width;
        float height_scale_rate = (float)scale_height / src_height;
        //std::cout<<"Scale rate check:"<<width_scale_rate<<" "<<height_scale_rate<<std::endl;
        //assert(fabs(width_scale_rate - height_scale_rate) < FLT_EPSILON);
        src_scale_rate = width_scale_rate;

        ctx->resized_src_ = create_resized_ir(scale_width, scale_height, image_format);
    }

    // need to generate intermediate ir
    ctx->blend_src_width_ = round_opencv(scale_width*dst_scale_rate);
    ctx->blend_src_height_ = round_opencv(scale_height*dst_scale_rate);
    // set NULL to delete easily
    // ctx->blend_src_ = ctx->resized_src_[0];
    ctx->blend_src_ = NULL;
    if (dst_resized||!src_resized)
    {
        ctx->blend_src_ = new unsigned char[ctx->blend_src_width_*ctx->blend_src_height_];
    }

    //ctx->resized_blend_src_ = ctx->resized_src_[0];
    ctx->resized_blend_src_ = NULL;
    if (!src_resized && dst_resized)
    {
        ctx->resized_blend_src_ = new unsigned char[scale_width*scale_height];
    }

    ctx->shift_x_ = shift_x;
    ctx->shift_y_ = shift_y;

    int blend_shift_x = round_opencv(dst_scale_rate*shift_x);
    int blend_shift_y = round_opencv(dst_scale_rate*shift_y);

    ctx->blender_ctx_ = create_blender_ctx(ctx->blend_src_width_, ctx->blend_src_height_, scale_dst_width, scale_dst_height,
                                           image_format, blend_shift_x, blend_shift_y, edge_extractor_method);

    Rect overlap_roi = get_overlap_roi(ctx->blender_ctx_);
    ctx->overlap_roi_.x = round_opencv(overlap_roi.x / dst_scale_rate);
    ctx->overlap_roi_.y = round_opencv(overlap_roi.y / dst_scale_rate);
    ctx->overlap_roi_.width = round_opencv(overlap_roi.width / dst_scale_rate);
    ctx->overlap_roi_.height = round_opencv(overlap_roi.height / dst_scale_rate);

    return ctx;
}


void release_scale_blender_ctx(void* ctx)
{
    if (!ctx)
    {
        return;
    }

    SCALE_BLENDER_CTX* scale_blender_ctx = (SCALE_BLENDER_CTX*)ctx;

    // release resized src
    release_resized_ir(scale_blender_ctx->resized_src_, scale_blender_ctx->image_format_);
    scale_blender_ctx->resized_src_ = NULL;

    if (scale_blender_ctx->blend_src_)
    {
        delete scale_blender_ctx->blend_src_;
        scale_blender_ctx->blend_src_ = NULL;
    }

    if (scale_blender_ctx->resized_blend_src_)
    {
        delete scale_blender_ctx->resized_blend_src_;
        scale_blender_ctx->resized_blend_src_ = NULL;
    }

    if (scale_blender_ctx->resized_dst_)
    {
        delete scale_blender_ctx->resized_dst_;
        scale_blender_ctx->resized_dst_ = NULL;
    }

    release_blender_ctx(scale_blender_ctx->blender_ctx_);
    scale_blender_ctx->blender_ctx_ = NULL;

    //release ctx
    delete scale_blender_ctx;

    return;
}


void copy_src2dst_yuv420(unsigned char** src, int src_width, int src_height, unsigned char** dst, int dst_width, int dst_height,
                         const Rect& overlap_roi)
{
    //LOGV("channel y\n");
    //channel y
    for (int r = 0; r < overlap_roi.height; r++)
    {
        int dst_r = overlap_roi.y + r;
        memcpy(dst[0] + dst_r*dst_width + overlap_roi.x, src[0] + r*src_width, overlap_roi.width);
    }

    //channel uv
    //LOGV("channel uv\n");
    for (int r = 0; r < overlap_roi.height / 2; r++)
    {
        int dst_r = overlap_roi.y / 2 + r;
        memcpy(dst[1] + dst_r*dst_width / 2 + overlap_roi.x / 2, src[1] + r*src_width / 2, overlap_roi.width / 2);
        memcpy(dst[2] + dst_r*dst_width / 2 + overlap_roi.x / 2, src[2] + r*src_width / 2, overlap_roi.width / 2);
    }
    //LOGV("end copy\n");
}


void copy_src2dst_yuv420sp(unsigned char** src, int src_width, int src_height, unsigned char** dst, int dst_width, int dst_height,
                           const Rect& overlap_roi)
{
    //channel y
    for (int r = 0; r < overlap_roi.height; r++)
    {
        int dst_r = overlap_roi.y + r;
        memcpy(dst[0] + dst_r*dst_width + overlap_roi.x, src[0] + r*src_width, overlap_roi.width);
    }

    //channel uv
    for (int r = 0; r < overlap_roi.height / 2; r++)
    {
        int dst_r = overlap_roi.y / 2 + r;
        memcpy(dst[1] + dst_r*dst_width + overlap_roi.x, src[1] + r*src_width, overlap_roi.width);
    }
}

void copy_src2dst_yuv(unsigned char** src, int src_width, int src_height, unsigned char** dst, int dst_width, int dst_height,
                      const Rect& overlap_roi, int image_format)
{
    switch (image_format)
    {
    case YUV420P_I420:
        copy_src2dst_yuv420(src, src_width, src_height, dst, dst_width, dst_height, overlap_roi);
        break;
    case YUV420SP:
        copy_src2dst_yuv420sp(src, src_width, src_height, dst, dst_width, dst_height, overlap_roi);
        break;
    default:
        break;
    }

    return;
}


void resize_ir_yuv420(unsigned char** src_img_data, const Mat& blend_src_y, SCALE_BLENDER_CTX* ctx, unsigned char** resized_src)
{
    Mat src_u(ctx->src_height_ / 2, ctx->src_width_ / 2, CV_8UC1, src_img_data[1]);
    Mat src_v(ctx->src_height_ / 2, ctx->src_width_ / 2, CV_8UC1, src_img_data[2]);

    Mat resized_src_y = blend_src_y;
    Mat resized_src_u = src_u;
    Mat resized_src_v = src_v;

    if (ctx->src_width_ != ctx->resized_src_width_
            || ctx->src_height_ != ctx->resized_src_height_)
    {
        // resize src
        resized_src_u = Mat(ctx->resized_src_height_ / 2, ctx->resized_src_width_ / 2, CV_8UC1, ctx->resized_src_[1]);
        resized_src_v = Mat(ctx->resized_src_height_ / 2, ctx->resized_src_width_ / 2, CV_8UC1, ctx->resized_src_[2]);

        resize(src_u, resized_src_u, resized_src_u.size());
        resize(src_v, resized_src_v, resized_src_v.size());
    }

    if (ctx->blend_src_width_ != ctx->resized_src_width_
            || ctx->blend_src_height_ != ctx->resized_src_height_)
    {
        if (ctx->resized_blend_src_)
        {
            resized_src_y = Mat(ctx->resized_src_height_, ctx->resized_src_width_, CV_8UC1, ctx->resized_blend_src_);
        }
        else
        {
            resized_src_y = Mat(ctx->resized_src_height_, ctx->resized_src_width_, CV_8UC1, ctx->resized_src_[0]);
        }
        resize(blend_src_y, resized_src_y, resized_src_y.size());
    }

    resized_src[0] = resized_src_y.data;
    resized_src[1] = resized_src_u.data;
    resized_src[2] = resized_src_v.data;
}


void resize_ir_yuv420sp(unsigned char** src_img_data, const Mat& blend_src_y, SCALE_BLENDER_CTX* ctx, unsigned char** resized_src)
{
    Mat src_uv(ctx->src_height_ / 2, ctx->src_width_ / 2, CV_8UC2, src_img_data[1]);

    Mat resized_src_y = blend_src_y;
    Mat resized_src_uv = src_uv;

    if (ctx->src_width_ != ctx->resized_src_width_
            || ctx->src_height_ != ctx->resized_src_height_)
    {
        // resize src
        //resized_src_y = Mat(ctx->resized_src_height_, ctx->resized_src_width_, CV_8UC1, ctx->resized_src_[0]);
        resized_src_uv = Mat(ctx->resized_src_height_ / 2, ctx->resized_src_width_ / 2, CV_8UC2, ctx->resized_src_[1]);

        //resize(blend_src_y, resized_src_y, resized_src_y.size());
        resize(src_uv, resized_src_uv, resized_src_uv.size());
    }

    if (ctx->blend_src_width_ != ctx->resized_src_width_
            || ctx->blend_src_height_ != ctx->resized_src_height_)
    {
        if (ctx->resized_blend_src_)
        {
            resized_src_y = Mat(ctx->resized_src_height_, ctx->resized_src_width_, CV_8UC1, ctx->resized_blend_src_);
        }
        else
        {
            resized_src_y = Mat(ctx->resized_src_height_, ctx->resized_src_width_, CV_8UC1, ctx->resized_src_[0]);
        }
        resize(blend_src_y, resized_src_y, resized_src_y.size());
    }

    resized_src[0] = resized_src_y.data;
    resized_src[1] = resized_src_uv.data;
}


void resize_ir(unsigned char** src, const Mat& blend_y, SCALE_BLENDER_CTX* ctx, unsigned char* resized_data[])
{
    //LOGV("------------resize image_format: %d\n", ctx->image_format_);

    switch (ctx->image_format_)
    {
    case YUV420P_I420:
        resize_ir_yuv420(src, blend_y, ctx, resized_data);
        break;
    case YUV420SP:
        resize_ir_yuv420sp(src, blend_y, ctx, resized_data);
        break;
    default:
        break;
    }

    return;
}


void scale_blend_edge_image(unsigned char** src_img_data, unsigned char** dst_img_data, void* ctx)
{
    //LOGV("---------scale blend edge image\n");
    int64 begin_time = cv::getTickCount();
    assert(ctx);
    SCALE_BLENDER_CTX* sb_ctx = (SCALE_BLENDER_CTX*)ctx;

    Mat dst_y = Mat(sb_ctx->dst_height_, sb_ctx->dst_width_, CV_8UC1, dst_img_data[0]);
    Mat resized_dst_y = dst_y;
    if (sb_ctx->resized_dst_width_ != sb_ctx->dst_width_
            || sb_ctx->resized_dst_height_ != sb_ctx->dst_height_)
    {
        resized_dst_y = Mat(sb_ctx->resized_dst_height_, sb_ctx->resized_dst_width_, CV_8UC1, sb_ctx->resized_dst_);
        resize(dst_y, resized_dst_y, resized_dst_y.size());
    }
    Mat src_y(sb_ctx->src_height_, sb_ctx->src_width_, CV_8UC1, src_img_data[0]);
    int64 begin_time_others = cv::getTickCount();
    Mat blend_src_y;
    assert(sb_ctx->blend_src_ || sb_ctx->resized_src_);
    if (sb_ctx->blend_src_)
    {
        blend_src_y = Mat(sb_ctx->blend_src_height_, sb_ctx->blend_src_width_, CV_8UC1, sb_ctx->blend_src_);
    }
    else
    {
        blend_src_y = Mat(sb_ctx->blend_src_height_, sb_ctx->blend_src_width_, CV_8UC1, sb_ctx->resized_src_[0]);
    }
    if (sb_ctx->blend_src_width_ != sb_ctx->src_width_
            || sb_ctx->blend_src_height_ != sb_ctx->src_height_)
    {
        resize(src_y, blend_src_y, blend_src_y.size());
    }
    else
    {
        src_y.copyTo(blend_src_y);
    }
    int64 end_time_others = cv::getTickCount();
    double time_others = (double)(end_time_others-begin_time_others)*1000/getTickFrequency();
    LOGV("others, time elapsed(ms): %lf\n", time_others);

    //note: no scale, src_y will enhance by edge
    int64 begin_time_blend = cv::getTickCount();
    //imwrite("./src.jpg", blend_src_y);
    blend_edge_ir(blend_src_y.data, resized_dst_y.data, sb_ctx->blender_ctx_);
    //imwrite("./blend.jpg", blend_src_y);
    LOGV("save blend\n");
    //while(1);
    int64 end_time_blend = cv::getTickCount();
    double time_blend = (double)(end_time_blend-begin_time_blend)*1000/getTickFrequency();
    LOGV("blending, time elapsed(ms): %lf\n", time_blend);

    int64 begin_time_resize = cv::getTickCount();
    unsigned char* resized_src[3];
    resize_ir(src_img_data, blend_src_y, sb_ctx, resized_src);
    Mat resized_img = Mat(sb_ctx->resized_src_height_, sb_ctx->resized_src_width_, CV_8UC1, resized_src[0]);
    //imwrite("/sdcard/Movies/resize.jpg", resized_img);
    int64 end_time_resize = cv::getTickCount();
    double time_resize = (double)(end_time_resize-begin_time_resize)*1000/getTickFrequency();
    LOGV("resize, time elapsed(ms): %lf\n", time_resize);

    int64 begin_time_copy = cv::getTickCount();
    copy_src2dst_yuv(resized_src, sb_ctx->resized_src_width_, sb_ctx->resized_src_height_,
                     dst_img_data, sb_ctx->dst_width_, sb_ctx->dst_height_, sb_ctx->overlap_roi_, sb_ctx->image_format_);
    Mat dst_img = Mat(sb_ctx->dst_height_, sb_ctx->dst_width_, CV_8UC1, dst_img_data[0]);
    //imwrite("dst_img.png",dst_img);

    int64 end_time_copy = cv::getTickCount();
    double time_copy = (double)(end_time_copy-begin_time_copy)*1000/getTickFrequency();
    LOGV("copy, time elapsed(ms): %lf\n", time_copy);

    int64 end_time = cv::getTickCount();
    double time = (double)(end_time-begin_time)*1000/getTickFrequency();
    LOGV("all time elapsed(ms): %lf\n", time);
}

}

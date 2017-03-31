/*
 * @brief   :   cv demo of enhancing edge for yuv
 * @author  :   hw.zhu
 * @data    :   2016/01/26
 * @version :   0.1
*/

#include <stdio.h>
#include <iostream>
#include <core/core.hpp>
#include <imgproc/imgproc.hpp>
#include <fstream>
#include "yuneec_cv_utilities.hpp"
#include "edge_enhancer.hpp"
#include "fuse_visual_thermalir.h"
#include "sobel_edge_extractor.h"
#include "laplacian_edge_extractor.h"
#include "canny_edge_extractor.h"

using namespace std;
using namespace cv;


void parse_parameter(int argc, char* argv[], int& method, string& src_path, string& dst_path, 
        int& src_width, int& src_height, int& dst_width, int& dst_height, int& src_yuv_format, 
        int& dst_yuv_format, int& channel_num, int& shift_x, int& shift_y, bool& display)
{
        int i = 0;
        while (i < argc)
        {
                if (string(argv[i]) == "-src")
                {
                        src_path = string(argv[++i]);
                }
                else if (string(argv[i]) == "-dst")
                {
                        dst_path = string(argv[++i]);
                }
                else if (string(argv[i]) == "-method")
                {
                        method = EdgeEnhancer::method_type(string(argv[++i]));
                }
                else if (string((argv[i])) == "-src_width")
                {
                        src_width = atoi(argv[++i]);
                }
                else if (string(argv[i]) == "-src_height")
                {
                        src_height = atoi(argv[++i]);
                }
                else if (string((argv[i])) == "-dst_width")
                {
                        dst_width = atoi(argv[++i]);
                }
                else if (string((argv[i])) == "-dst_height")
                {
                        dst_height = atoi(argv[++i]);
                }
                else if (string(argv[i]) == "-channel_num")
                {
                        channel_num = atoi(argv[++i]);
                }
                else if (string(argv[i]) == "-src_yuv_format")
                {
                        src_yuv_format = yuv_image_format(string(argv[++i]));
                }
                else if (string(argv[i]) == "-dst_yuv_format")
                {
                        dst_yuv_format = yuv_image_format(string(argv[++i]));
                }
                else if (string(argv[i]) == "-shift_x")
                {
                        shift_x = atoi(argv[++i]);
                }
                else if (string(argv[i]) == "-shift_y")
                {
                        shift_y = atoi(argv[++i]);
                }
                else if (string(argv[i]) == "-display")
                {
                        display = true;
                }
                else
                {
                }

                i++;
        }
}



int main_edge_enhance(int argc, char **argv)
{
        //default parameter
        int method = 0;         //canny         
        string src_path("");
        string dst_path("");
        int width = 0;
        int height = 0;
        int yuv_format = 0;     //YUV420P_I420
        //too many parameter
        Size process_size(640, 480);
        int channel_num = 1;
        bool display = false;
        //parse_parameter(argc - 1, argv + 1, method, src_path, dst_path, width, height, yuv_format, channel_num, display);

        //read yuv video from file
        /*
        fstream src_file(src_path.c_str(), ios::in | ios::binary);
        fstream dst_file(dst_path.c_str(), ios::out | ios::binary);
        assert(src_file&&dst_file);

        Rect center_rect = get_center_rect(Size(640, 480), Size(width, height));
        int buf_len = width*height * 3 / 2;
        unsigned char* pyuv = new unsigned char[buf_len];

        EdgeEnhancer edge_enhancer(method);
        int frame_index = 0;
        for (;;)
        {
                src_file.read((char*)pyuv, buf_len*sizeof(unsigned char));
                int64 begin_time = cv::getTickCount();
                edge_enhancer.edge_enhance(pyuv, width, height, yuv_format, center_rect, channel_num, display);
                int64 end_time = cv::getTickCount();
                double cost_time = (double)(end_time - begin_time) * 1000 / getTickFrequency();
                cout << "cost_time(ms): " << cost_time << endl;
                dst_file.write((char*)pyuv, buf_len*sizeof(unsigned char));

                cout << "frame: " << frame_index++ << endl;
                if (src_file.eof())
                {
                        cout << "done!" << endl;
                        break;
                }

        }

        src_file.close();
        dst_file.close();
        //*/

        //read video
        //*
        cv::VideoCapture src_video(src_path);
        assert(src_video.isOpened());
        Mat frame;
        src_video >> frame;
        assert(frame.data);
        cout << "frame.size(): " << frame.size() << endl;
        Rect center_rect = get_center_rect(Size(640, 480), frame.size());
        cv::VideoWriter dst_video(dst_path, CV_FOURCC('X', 'V', 'I', 'D'), 25, frame.size());


        EdgeEnhancer edge_enhancer(method);
        int frame_index = 0;
        while (1)
        {
                cout << "frame_index: " << frame_index << endl;
                imshow("src frame", frame);
                Mat yuv;
                cvtColor(frame, yuv, CV_BGR2YUV_I420);
                //edge_enhancer.edge_enhance_sobel(yuv, center_rect, channel_num);
                edge_enhancer.edge_enhance_canny(yuv, center_rect, 50, channel_num);
                cvtColor(yuv, frame, CV_YUV2BGR_I420);
                imshow("enhanced frame", frame);
                waitKey();

                src_video >> frame;
                frame_index++;
        }
        //*/


        /*read image from file
        int len = height * 3 / 2 * width;
        string base_dir("F:/workspace/cv/yuneec/YUVProc/YUVProc");
        int index = 130;
        uchar* yuv_data = new uchar[len];
        while (1)
        {
                char frame_index_buf[32];
                sprintf(frame_index_buf, "src_%d.yuv", index);
                string src_file_path = base_dir + "/" + string(frame_index_buf);
                fstream src_file(src_file_path.c_str(), ios::in | ios::binary);
                assert(src_file);
                
                src_file.read((char*)yuv_data, len*sizeof(uchar));
                src_file.close();

                Mat src_yuv_img = yuneec_cv_yuv2mat(yuv_data, width, height, yuv_format);
                imshow("src_yuv_img", src_yuv_img);
                Mat src_bgr_img;
                cvtColor(src_yuv_img, src_bgr_img, CV_YUV2BGR_I420);
                imshow("src_bgr_img", src_bgr_img);
                waitKey();

                sprintf(frame_index_buf, "dst_%d.yuv", index);
                string dst_file_path = base_dir + "/" + string(frame_index_buf);
                fstream dst_file(dst_file_path.c_str(), ios::in | ios::binary);
                assert(dst_file);
                dst_file.read((char*)yuv_data, len*sizeof(char));
                dst_file.close();

                Mat dst_yuv_img = yuneec_cv_yuv2mat(yuv_data, width, height, yuv_format);
                imshow("dst_yuv_img", dst_yuv_img);
                Mat dst_bgr_img;
                cvtColor(dst_yuv_img, dst_bgr_img, CV_YUV2BGR_I420);
                imshow("dst_bgr_img", dst_bgr_img);

                waitKey();

                index++;
        }
        delete yuv_data;
        //*/
        getchar();
        return EXIT_SUCCESS;
}


int main_fusion(int argc, char** argv)
{
        //default parameter
        int method = 0;         //canny         
        string src_path("");
        string dst_path("");
        int src_width = 0;
        int src_height = 0;
        int dst_width = 0;
        int dst_height = 0;

        int src_yuv_format = 0;     //YUV420P_I420
        int dst_yuv_format = 0;
        int channel_num = 1;
        bool display = false;
        int shift_x = 0;
        int shift_y = 0;
        parse_parameter(argc - 1, argv + 1, method, src_path, dst_path, src_width, src_height, dst_width, dst_height, 
                src_yuv_format, dst_yuv_format, channel_num, shift_x, shift_y, display);

        //read yuv video from file
        //*
        fstream src_file(src_path.c_str(), ios::in | ios::binary);
        fstream dst_file(dst_path.c_str(), ios::in | ios::binary);
        assert(src_file&&dst_file);

        int src_buf_len = src_width*src_height * 3 / 2;
        int dst_buf_len = dst_width*dst_height * 3 / 2;
        unsigned char* psrc_yuv = new unsigned char[src_buf_len];
        unsigned char* pdst_yuv = new unsigned char[dst_buf_len];
        assert(psrc_yuv&&pdst_yuv);
        unsigned char* psrc_yuv_array[3];
        psrc_yuv_array[0] = psrc_yuv;
        psrc_yuv_array[1] = psrc_yuv + src_width*src_height;
        psrc_yuv_array[2] = psrc_yuv + src_width*src_height + src_width*src_height/4;

        unsigned char* pdst_yuv_array[3];
        pdst_yuv_array[0] = pdst_yuv;
        pdst_yuv_array[1] = pdst_yuv + dst_width*dst_height;
        pdst_yuv_array[2] = pdst_yuv + dst_width*dst_height + dst_width*dst_height/4;

        int frame_index = 0;
        for (;;)
        {
                src_file.read((char*)psrc_yuv, src_buf_len*sizeof(unsigned char));
                dst_file.read((char*)pdst_yuv, dst_buf_len*sizeof(unsigned char));
                //memset(psrc_yuv, 255, src_width*src_height);
                //memset(psrc_yuv + src_width*src_height, 128, src_width*src_height / 4);
                //memset(psrc_yuv + src_width*src_height + src_width*src_height / 4, 128, src_width*src_height / 4);
                if (display)
                {
                        Mat src_yuv = yuneec_cv_yuv2mat(psrc_yuv, src_width, src_height, src_yuv_format);
                        Mat src_rgb;
                        cvtColor(src_yuv, src_rgb, CV_YUV2BGR_I420);
                        cv::namedWindow("IR", WINDOW_AUTOSIZE);
                        imshow("IR", src_rgb);

                        Mat dst_yuv = yuneec_cv_yuv2mat(pdst_yuv, dst_width, dst_height, dst_yuv_format);
                        Mat dst_rgb;
                        cvtColor(dst_yuv, dst_rgb, CV_YUV2BGR_I420);
                        imshow("Visual", dst_rgb);
                }

                int64 begin_time = cv::getTickCount();
                fuse_visual_thermalir(pdst_yuv_array, dst_width, dst_height, dst_yuv_format, psrc_yuv_array, src_width, src_height,
                        src_yuv_format, shift_x, shift_y);
                int64 end_time = cv::getTickCount();
                double cost_time = (double)(end_time - begin_time) * 1000 / getTickFrequency();
                cout << "cost_time(ms): " << cost_time << endl;
                
                if (display)
                {
                        Mat dst_yuv = yuneec_cv_yuv2mat(pdst_yuv, dst_width, dst_height, dst_yuv_format);
                        Mat dst_rgb;
                        cvtColor(dst_yuv, dst_rgb, CV_YUV2BGR_I420);
                        imshow("Fusion", dst_rgb);
                        waitKey();
                }
                cout << "frame: " << frame_index++ << endl;
                if (src_file.eof()||dst_file.eof())
                {
                        cout << "done!" << endl;
                        break;
                }

        }

        src_file.close();
        dst_file.close();
        //*/

        return 0;
}

int main_extract_edge(int argc, char** argv)
{
        string img_path("./test2.jpg");
        Mat img = imread(img_path,0);

        CannyEdgeExtractor cee(50, 100);
        SobelEdgeExtrator see;
        LaplacianEdgeExtractor lee;

        Mat edge;
        see.extract_edge(img, edge);
        //lee.extract_edge(img, edge);
        //cee.extract_edge(img, edge);

        imshow("edge", edge);
        waitKey();

        return 0;
}
int main(int argc, char** argv)
{
        return main_fusion(argc, argv);
        //return main_extract_edge(argc, argv);
}
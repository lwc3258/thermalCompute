/*
 * @brief   :   image process module
 * @creater :   hw.zhu
 * @data    :   2016/01/25
 * @version :   0.1
*/


#include <iostream>
#include "edge_enhancer.hpp"
#include "yuneec_cv_utilities.hpp"

#include <opencv2/opencv.hpp>
 using namespace std;
 using namespace cv;


 EdgeEnhancer::EdgeEnhancer()
 {
         method_ = EE_CANNY;
 }


 EdgeEnhancer::EdgeEnhancer(int method) :method_(method)
 {        
 }


 void  EdgeEnhancer::edge_enhance(unsigned char *yuv_data, int width, int height, int yuv_format,
         const Rect& roi, int channel_num, bool display)
 {
         CV_Assert(yuv_data != NULL && width > 0 && height > 0);
         Mat yuv_img = yuneec_cv_yuv2mat(yuv_data, width, height, yuv_format);
         
         if (display)
         {
                 Mat rgb;
                 //cvtColor(yuv_img, rgb, CV_YUV2BGR_I420);
                 imshow("yuv_img", yuv_img);
                 imshow("yuv", rgb);
         }
         
         if (yuv_img.empty())
         {
                 cout << "[yuneec_cv_edges_enhanced]sorry, no data, exit!" << endl;
                 return;
         }

         switch (method_)
         {
         case EE_CANNY:
                 edge_enhance_canny(yuv_img, roi, 50, channel_num);
         	break;
         case EE_LAPLACIAN:
                 edge_enhance_laplacian(yuv_img, roi, channel_num);
                break;
         case EE_SOBEL:
                 edge_enhance_sobel(yuv_img, roi, channel_num);
                 break;
         default:
                 cout << "invalid method" << endl;
                 return;
         }

         if (display)
         {
                 Mat rgb;
                 cvtColor(yuv_img, rgb, CV_YUV2BGR_I420);
                 imshow("enhanced yuv_img", yuv_img);
                 imshow("enhanced yuv", rgb);
                 waitKey();
         }
         return;
 }


 void  EdgeEnhancer::edge_enhance_canny(Mat& yuv_img, const Rect& roi, unsigned char edge_thresh, int channel_num)
 {
         CV_Assert((roi.width <= yuv_img.cols) && (roi.height <= yuv_img.rows / 2 * 3));
         if (channel_num == 3)
         {
                 Mat bgr_img;
                 cvtColor(yuv_img, bgr_img, CV_YUV2BGR_I420);
                 imshow("before enhance", bgr_img);
                 vector<Mat> bgr_vec;
                 split(bgr_img, bgr_vec);
                 for (size_t i = 0; i < bgr_vec.size(); i++)
                 {
                         Mat edge;
                         Mat roi_bgr = bgr_vec[i](roi);
                         GaussianBlur(roi_bgr, roi_bgr, Size(3, 3), 0, 0);
                         cv::Canny(roi_bgr, edge, edge_thresh, edge_thresh * 2, 3);
                         add(roi_bgr, edge, roi_bgr);
                 }
                 merge(bgr_vec, bgr_img);
                 cvtColor(bgr_img, yuv_img, CV_BGR2YUV_I420);
                 imshow("bgr_enchance", bgr_img);
         }
         else
         {
                 Mat gray_img, edges;
                 cvtColor(yuv_img, gray_img, COLOR_YUV2GRAY_I420);

                 gray_img = gray_img(roi);
                 Mat yuv_roi_img = yuv_img(roi);
                 GaussianBlur(gray_img, gray_img, Size(3, 3), 0, 0);
                 Canny(gray_img, edges, edge_thresh, edge_thresh * 2, 3);

                 //1. yuv_img is allocated automatically as having different image size with input image
                 //add(gray_img, edges, yuv_img);
                 add(gray_img, edges, yuv_roi_img);
         }
    return;
 }


 void EdgeEnhancer::edge_enhance_laplacian(Mat& yuv_img, const Rect& roi, int channel)
 {
         CV_Assert((roi.width <= yuv_img.cols) && (roi.height <= yuv_img.rows / 2 * 3));
         if (channel == 3)
         {
                 Mat bgr_img;
                 cvtColor(yuv_img, bgr_img, CV_YUV2BGR_I420);
                 //imshow("before enhance", bgr_img);
                 vector<Mat> bgr_vec;
                 split(bgr_img, bgr_vec);

                 for (size_t i = 0; i < bgr_vec.size(); i++)
                 {
                         Mat edge;
                         Mat roi_bgr = bgr_vec[i](roi);
                         GaussianBlur(roi_bgr, roi_bgr, Size(3, 3), 0, 0);
                         cv::Laplacian(roi_bgr, edge, CV_16S, 3);
                         convertScaleAbs(edge, edge);
                         add(roi_bgr, edge, roi_bgr);
                 }
                 merge(bgr_vec, bgr_img);
                 cvtColor(bgr_img, yuv_img, CV_BGR2YUV_I420);
         }
         else
         {
                 Mat gray_img;
                 cvtColor(yuv_img, gray_img, COLOR_YUV2GRAY_I420);

                 Mat yuv_roi_img = yuv_img(roi);
                 gray_img = gray_img(roi);

                 GaussianBlur(gray_img, gray_img, Size(3, 3), 0, 0);
                 Mat edge;
                 cv::Laplacian(gray_img, edge, CV_16S, 3);
                 convertScaleAbs(edge, edge);

                 //1. yuv_img is allocated automatically as having different image size with input image
                 //add(gray_img, edges, yuv_img);
                 add(gray_img, edge, yuv_roi_img);
         }
 }


 void EdgeEnhancer::edge_enhance_sobel(Mat& yuv_img, const Rect& roi, int channel_num)
 {
         if (channel_num == 3)
         {
                 Mat bgr_img;
                 cvtColor(yuv_img, bgr_img, CV_YUV2BGR_I420);
                 GaussianBlur(bgr_img, bgr_img, Size(3, 3), 0, 0);
                 //imshow("before enhance", bgr_img);

                 vector<Mat> bgr_vec;
                 split(bgr_img, bgr_vec);

                 for (size_t i = 0; i < bgr_vec.size(); i++)
                 {
                         Mat roi_bgr = bgr_vec[i](roi);
                         Mat gradx, grady, mag;
                         Sobel(roi_bgr, gradx, CV_16S, 1, 0);
                         convertScaleAbs(gradx, gradx);
                         Sobel(roi_bgr, grady, CV_16S, 0, 1);
                         convertScaleAbs(grady, grady);
                         cv::addWeighted(gradx, 0.5, grady, 0.5, 0, mag);
                         //imshow("mag", mag);
                         //waitKey();
                         add(roi_bgr, mag, roi_bgr);
                 }
                 merge(bgr_vec, bgr_img);
                 cvtColor(bgr_img, yuv_img, CV_BGR2YUV_I420);
                 //imshow("after enhance", bgr_img);
                 //waitKey();

                 return;
         }
         else
         {
                 Mat gray_img;
                 cvtColor(yuv_img, gray_img, COLOR_YUV2GRAY_I420);

                 Mat yuv_roi_img = yuv_img(roi);
                 gray_img = gray_img(roi);

                 GaussianBlur(gray_img, gray_img, Size(3, 3), 0, 0);

                 Mat gradx, grady, mag;
                 Sobel(gray_img, gradx, CV_16S, 1, 0);
                 convertScaleAbs(gradx, gradx);
                 Sobel(gray_img, grady, CV_16S, 0, 1);
                 convertScaleAbs(grady, grady);
                 cv::addWeighted(gradx, 0.5, grady, 0.5, 0, mag);

                 //1. yuv_img is allocated automatically as having different image size with input image
                 //add(gray_img, edges, yuv_img);
                 add(gray_img, mag, yuv_roi_img);
         }
 }



 int EdgeEnhancer::method_type(const string& method_name)
 {
         if (method_name == "canny")
         {
                 return EE_CANNY;
         }
         else if (method_name == "laplacian")
         {
                 return EE_LAPLACIAN;
         }
         else if (method_name == "sobel")
         {
                 return EE_SOBEL;
         }

         return -1;
 }

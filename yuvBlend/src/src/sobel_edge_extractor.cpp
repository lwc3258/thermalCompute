
#include "sobel_edge_extractor.h"

SobelEdgeExtrator::SobelEdgeExtrator() :EdgeExtractor(EEDGE_SOBEL)
{
}

void SobelEdgeExtrator::extract_edge(const Mat& img, Mat& edge)
{
        if (img.channels() > 1)
        {
                vector<Mat> split_img_vec;
                cv::split(img, split_img_vec);
                edge = Mat(img.rows, img.cols, CV_8UC1, Scalar(0));
                for (size_t i = 0; i < split_img_vec.size(); i++)
                {
                        //GaussianBlur(split_img_vec[i], split_img_vec[i], Size(3, 3), 0, 0);
                        Mat edge_i;
                        Mat grad_x, grad_y;
                        Sobel(split_img_vec[i], grad_x, CV_16SC1, 1, 0);
                        convertScaleAbs(grad_x, grad_x);
                        Sobel(split_img_vec[i], grad_y, CV_16SC1, 0, 1);
                        convertScaleAbs(grad_y, grad_y);
                        addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edge_i);
                        max(edge, edge_i, edge);
                }

                return;
        }

        // gaussian blur
        Mat blur = img;
        //GaussianBlur(img, blur, Size(3, 3), 0, 0);
        Mat grad_x, grad_y;
        Sobel(blur, grad_x, CV_16SC1, 1, 0);
        convertScaleAbs(grad_x, grad_x);
        Sobel(blur, grad_y, CV_16SC1, 0, 1);
        convertScaleAbs(grad_y, grad_y);
        addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edge);

        return;
}

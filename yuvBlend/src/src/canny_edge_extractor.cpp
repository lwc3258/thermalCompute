
#include "canny_edge_extractor.h"

CannyEdgeExtractor::CannyEdgeExtractor(double thresh1, double thresh2) :EdgeExtractor(EEDGE_CANNY),thresh1_(thresh1), thresh2_(thresh2)
{
}


void CannyEdgeExtractor::extract_edge(const Mat& img, Mat& edge)
{
        if (img.channels() > 1)
        {
                vector<Mat> split_img_vec;
                cv::split(img, split_img_vec);                                                                                        
                edge = Mat(img.rows, img.cols, CV_8UC1, Scalar(0));
                for (size_t i = 0; i < split_img_vec.size(); i++)
                {
                        GaussianBlur(split_img_vec[i], split_img_vec[i], Size(3, 3), 0, 0);
                        Mat edge_i;
                        Canny(split_img_vec[i], edge_i, thresh1_, thresh2_);
                        edge = edge + edge_i;
                }

                return;
        }

        // gaussian blur
        Mat blur;
        GaussianBlur(img, blur, Size(3, 3), 0, 0);
        
        //test
        //imshow("gray", gray);
        
        //extract edge
        Canny(blur, edge, thresh1_, thresh2_);
        
        //test
        //imshow("edge", edge);
        //waitKey();
        //SaveUCharMat(edge, "./edge.txt");

        return;
}

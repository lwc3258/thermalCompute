
#include "laplacian_edge_extractor.h"

LaplacianEdgeExtractor::LaplacianEdgeExtractor() :EdgeExtractor(EEDGE_LAPLACIAN)
{
}

void LaplacianEdgeExtractor::extract_edge(const Mat& img, Mat& edge)
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
                        cv::Laplacian(split_img_vec[i], edge_i, CV_16S, 19);
                        convertScaleAbs(edge_i, edge_i);
                        //edge = edge + edge_i;
                        cv::max(edge, edge_i, edge);
                }

                return;
        }

        // gaussian blur
        Mat blur;
        GaussianBlur(img, blur, Size(3, 3), 0, 0);
        cv::Laplacian(blur, edge, CV_16S, 3);
        edge.convertTo(edge, CV_8U);
        //convertScaleAbs(edge, edge);

        return;
}
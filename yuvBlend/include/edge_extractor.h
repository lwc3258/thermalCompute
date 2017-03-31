#ifndef EDGE_EXTRACTOR_
#define EDGE_EXTRACTOR_

#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

enum edge_extractor_method{ EEDGE_CANNY = 0, EEDGE_LAPLACIAN, EEDGE_SOBEL, EEDGE_DOG };

class EdgeExtractor
{
public:
        int method_;
public:
        EdgeExtractor(int method);
        virtual void extract_edge(const Mat& gray, Mat& edge);
	static int method_type(const string& method_name);
};

#endif

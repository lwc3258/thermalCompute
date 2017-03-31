
#include "edge_extractor.h"

class CannyEdgeExtractor :public EdgeExtractor
{
public:
        double thresh1_;
        double thresh2_;
public:
        CannyEdgeExtractor(double thresh1, double thresh2);
        void extract_edge(const Mat& gray, Mat& edge);
};
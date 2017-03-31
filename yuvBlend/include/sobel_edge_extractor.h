
#include "edge_extractor.h"

class SobelEdgeExtrator :public EdgeExtractor
{
public:
        SobelEdgeExtrator();
        void extract_edge(const Mat& gray, Mat& edge);
};
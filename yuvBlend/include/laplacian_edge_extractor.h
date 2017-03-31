
#include "edge_extractor.h"

class LaplacianEdgeExtractor :public EdgeExtractor
{

public:
        LaplacianEdgeExtractor();
        void extract_edge(const Mat& img, Mat& edge);
};
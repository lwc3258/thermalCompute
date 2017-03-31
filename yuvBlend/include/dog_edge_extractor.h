
#include "edge_extractor.h"

class DoGEdgeExtractor:public EdgeExtractor
{
public:
	Mat blur_;
public:
        DoGEdgeExtractor();
	DoGEdgeExtractor(const int width , const int height);
        void extract_edge(const Mat& img, Mat& edge);
	~DoGEdgeExtractor();
};

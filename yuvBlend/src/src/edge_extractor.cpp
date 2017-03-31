
#include <assert.h>
#include "edge_extractor.h"

EdgeExtractor::EdgeExtractor(int method) :method_(method)
{
}

void EdgeExtractor::extract_edge(const Mat& gray, Mat& edge)
{
}

int EdgeExtractor::method_type(const string& method_name)
{
        if (method_name == "canny")
        {
                return EEDGE_CANNY;
        }
        else if (method_name == "laplacian")
        {
                return EEDGE_LAPLACIAN;
        }
        else if (method_name == "sobel")
        {
                return EEDGE_SOBEL;
        }
        else if (method_name == "dog")
        {
                return EEDGE_DOG;
        }
        else
        {
                return -1;
        }  
}

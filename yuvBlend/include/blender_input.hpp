
#include <string>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

class BlenderInput
{
public:
    //ferared image
	string src_img_path_ = "";
	string dst_img_path_ = "";
	Rect dst_roi_ = Rect(0,0,0,0);
	Point shift_ = Point(0,0);
	string tracker_config_path_ = "";
	bool display_ = false;

public:
	BlenderInput(int argc, char* argv[]);
private:
	void parse_param(int argc, char* argv[]);
};

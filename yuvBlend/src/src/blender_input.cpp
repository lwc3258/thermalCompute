
#include "blender_input.hpp"
#include "utils.hpp"

BlenderInput::BlenderInput(int argc, char* argv[])
{
    parse_param(argc, argv);
}


void BlenderInput::parse_param(int argc, char* argv[])
{
    int i = 0;
	while (i<argc)
	{
		if (string(argv[i]) == "-src")
		{
			src_img_path_ = string(argv[++i]);
		}
		else if (string(argv[i]) == "-dst")
		{
			dst_img_path_ = string(argv[++i]);
		}
		else if (string(argv[i]) == "-roi")
		{
			dst_roi_.x = str2int(string(argv[++i]));
			dst_roi_.y = str2int(string(argv[++i]));
			dst_roi_.width = str2int(string(argv[++i]));
			dst_roi_.height = str2int(string(argv[++i]));
		}
		else if (string(argv[i]) == "-shift")
		{
		      shift_.x = str2int(string(argv[++i]));
		      shift_.y = str2int(string(argv[++i]));
		}
		else if (string(argv[i]) == "-display")
		{
		    display_ = true;
		}
		else
		{
			cout << "parse tracker input parameter error: "
				<< string(argv[i]) << endl;
			return;
		}

		i++;
	}

	return;
}

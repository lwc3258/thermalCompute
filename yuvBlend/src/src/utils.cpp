
#include "utils.hpp"

int str2int(const string& str)
{
	stringstream ss;
	ss << str;
	int integer;
	ss >> integer;
	return integer;
}

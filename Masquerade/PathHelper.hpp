#pragma once
#include <string>
#include <algorithm>

using std::string;

class PathHelper {
public:
	static void ToUnix(string& path) {
		std::replace(path.begin(), path.end(), '\\', '/');
	}
};
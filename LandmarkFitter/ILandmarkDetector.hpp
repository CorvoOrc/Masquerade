#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::ofstream;

class ILandmarkDetector
{
public:
	ILandmarkDetector() {}
	virtual ~ILandmarkDetector() {}

	virtual void Init(string predictorPath) = 0;
	virtual void Detect(string imagePath) = 0;
	virtual void Serialize(string path, string extention) = 0;
};


#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

struct RenderParams {
	RenderParams(string modelfile, string isomapfile, string mappingsfile, string contourfile, string edgetopologyfile, string blendshapesfile, string outputbasename) {
		_modelfile = modelfile;
		_isomapfile = isomapfile;
		_mappingsfile = mappingsfile;
		_contourfile = contourfile;
		_edgetopologyfile = edgetopologyfile;
		_blendshapesfile = blendshapesfile;
		_outputbasename = outputbasename;
	}

	string _modelfile;
	string _isomapfile;
	string _mappingsfile;
	string _contourfile;
	string _edgetopologyfile;
	string _blendshapesfile;
	string _outputbasename;
};

struct LandmarkParams
{
	LandmarkParams(string shape_predictor_file, string computed_landmark_file) {
		_shape_predictor_file = shape_predictor_file;
		_computed_landmark_file = _computed_landmark_file;
	}

	string _shape_predictor_file;
	string _computed_landmark_file;
};
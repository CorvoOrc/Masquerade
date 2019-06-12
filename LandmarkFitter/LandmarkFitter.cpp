#include <iostream>
#include "DlibLandmarkDetector.hpp"
#include "DlibCNNLandmarkDetector.hpp"
#include "LandmarkSerializer.hpp"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int argc, char* argv[]) {
	string modelfile, isomapfile, imagefile, landmarksfile, mappingsfile, contourfile, edgetopologyfile,
		blendshapesfile, outputbasename, shape_predictor_file, computed_landmark_file;
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
			("image,i", po::value<string>(&imagefile)->required()->default_value("../Masquerade/data/IntelSummer.jpg"),
				"an input image")
			("shape_predictor,sp", po::value<string>(&shape_predictor_file)->required()->default_value("../eos/share/shape_predictor_68_face_landmarks.dat"),
				"an input image")
			("computed_landmark,cl", po::value<string>(&computed_landmark_file)->required()->default_value("landmark"),
				"an input image");
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);
	}
	catch (const po::error & e) {
		return EXIT_FAILURE;
	}

	ILandmarkDetector* detector = new DlibLandmarkDetector();
	detector->Init(shape_predictor_file);
	detector->Detect(imagefile);
	detector->Serialize(computed_landmark_file, ".pts");

	std::cin.get();
}
#include "SurreyFaceModelBuilder.hpp"
#include "IglViewer.hpp"


int main(int argc, char* argv[]) {
	string modelfile, isomapfile, imagefile, landmarksfile, mappingsfile, contourfile, edgetopologyfile,
		blendshapesfile, outputbasename, shape_predictor_file, computed_landmark_file, animation_settings_file;
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
			("model,m", po::value<string>(&modelfile)->required()->default_value("../eos/share/sfm_shape_3448.bin"),
				"a Morphable Model stored as cereal BinaryArchive")
			("image,i", po::value<string>(&imagefile)->required()->default_value("../eos/examples/data/indoor_039.png"),
				"an input image")
			("landmarks,l", po::value<string>(&landmarksfile)->required()->default_value("../eos/examples/data/indoor_039.pts"),
				"2D landmarks for the image, in ibug .pts format")
			("mapping,p", po::value<string>(&mappingsfile)->required()->default_value("../eos/share/ibug_to_sfm.txt"),
				"landmark identifier to model vertex number mapping")
			("model-contour,c", po::value<string>(&contourfile)->required()->default_value("../eos/share/sfm_model_contours.json"),
				"file with model contour indices")
			("edge-topology,e", po::value<string>(&edgetopologyfile)->required()->default_value("../eos/share/sfm_3448_edge_topology.json"),
				"file with model's precomputed edge topology")
			("blendshapes,b", po::value<string>(&blendshapesfile)->required()->default_value("../eos/share/expression_blendshapes_3448.bin"),
				"file with blendshapes")
			("output,o", po::value<string>(&outputbasename)->required()->default_value("out"),
				"basename for the output rendering and obj files")
			("animation_settings,s", po::value<string>(&animation_settings_file)->required()->default_value("../eos/share/animation_settings.json"),
				"file with animation settings");
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);
	}
	catch (const po::error & e) {
		return EXIT_FAILURE;
	}

	shared_ptr<Settings> settings = make_shared<Settings>();
	shared_ptr<IChrono> chrono = make_shared<Chrono>();
	shared_ptr<Timeline> timeline = make_shared<Timeline>();
	IViewer* viewer = new IglViewer(settings, chrono, timeline);
	viewer->LoadSettings(animation_settings_file);

	RenderParams params(modelfile, isomapfile, mappingsfile, contourfile, edgetopologyfile, blendshapesfile, outputbasename);

	viewer->Show(params);
}
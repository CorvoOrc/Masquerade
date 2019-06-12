#include "IFaceModelBuilder.hpp"

#include "eos/core/Image.hpp"
#include "eos/core/image/opencv_interop.hpp"
#include "eos/core/Landmark.hpp"
#include "eos/core/LandmarkMapper.hpp"
#include "eos/core/read_pts_landmarks.hpp"
#include "eos/core/write_obj.hpp"
#include "eos/fitting/fitting.hpp"
#include "eos/fitting/contour_correspondence.hpp"
#include "eos/fitting/closest_edge_fitting.hpp"
#include "eos/fitting/RenderingParameters.hpp"
#include "eos/morphablemodel/Blendshape.hpp"
#include "eos/morphablemodel/MorphableModel.hpp"
#include "eos/render/draw_utils.hpp"
#include "eos/render/texture_extraction.hpp"
#include "eos/core/image/opencv_interop.hpp"
#include "eos/render/render.hpp"
#include "eos/cpp17/optional.hpp"

#include "Eigen/Core"

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

using namespace eos;

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using eos::core::Landmark;
using eos::core::LandmarkCollection;

class SurreyFaceModelBuilder : public IFaceModelBuilder
{
public:
	SurreyFaceModelBuilder() : IFaceModelBuilder() {
		_result = new int;
		*_result = 0;
	}
	~SurreyFaceModelBuilder() {}

	IFaceModelBuilder* LoadModel(string imagefile, string landmarksfile, string modelfile, string mappingsfile, 
		string blendshapesfile, string contourfile, string edgetopologyfile) {
		_image = cv::imread(imagefile);
		try
		{
			_landmarks = core::read_pts_landmarks(landmarksfile);
		}
		catch (const std::runtime_error & e)
		{
			std::cout << "Error reading the landmarks: " << e.what() << std::endl;
			*_result = 1;
		}
		morphablemodel::MorphableModel temp_morphable_model;
		try
		{
			temp_morphable_model = morphablemodel::load_model(modelfile);
		}
		catch (const std::runtime_error & e)
		{
			std::cout << "Error loading the Morphable Model: " << e.what() << std::endl;
			*_result = 2;
		}
		try
		{
			_landmark_mapper = core::LandmarkMapper(mappingsfile);
		}
		catch (const std::exception & e)
		{
			std::cout << "Error loading the landmark mappings: " << e.what() << std::endl;
			*_result = 3;
		}
		_model_contour = contourfile.empty() ? fitting::ModelContour() : fitting::ModelContour::load(contourfile);
		_ibug_contour = fitting::ContourLandmarks::load(mappingsfile);
		_edge_topology = morphablemodel::load_edge_topology(edgetopologyfile);
		const vector<morphablemodel::Blendshape> blendshapes = morphablemodel::load_blendshapes(blendshapesfile);

		_morphable_model = new morphablemodel::MorphableModel(
			temp_morphable_model.get_shape_model(), blendshapes,
			temp_morphable_model.get_color_model(), cpp17::nullopt,
			temp_morphable_model.get_texture_coordinates()
		);

		return this;
	}
	IFaceModelBuilder* FitShape(vector<float>& pca_coeffs, vector<float>& blendshape_coeffs) {
		vector<Eigen::Vector2f> fitted_image_points;
		std::tie(_mesh, _rendering_params) = fitting::fit_shape_and_pose(*_morphable_model, _landmarks, _landmark_mapper, _image.cols, _image.rows,
			_edge_topology, _ibug_contour, _model_contour, 5, cpp17::nullopt, 50.0f, cpp17::nullopt, cpp17::nullopt, pca_coeffs, blendshape_coeffs, fitted_image_points);
		return this;
	}
	IFaceModelBuilder* FitTexture() {
		const Eigen::Matrix<float, 3, 4> affine_from_ortho = fitting::get_3x4_affine_camera_matrix(_rendering_params, _image.cols, _image.rows);
		_isomap = core::to_mat(render::extract_texture(_mesh, affine_from_ortho, core::from_mat(_image), true));

		return this;
	}
	void Serialize(string outputbasename) {
		Mat outimg = _image.clone();
		for (auto&& lm : _landmarks)
		{
			cv::rectangle(outimg, cv::Point2f(lm.coordinates[0] - 2.0f, lm.coordinates[1] - 2.0f),
				cv::Point2f(lm.coordinates[0] + 2.0f, lm.coordinates[1] + 2.0f), { 255, 0, 0 });
		}

		render::draw_wireframe(outimg, _mesh, _rendering_params.get_modelview(), _rendering_params.get_projection(),
			fitting::get_opencv_viewport(_image.cols, _image.rows));

		fs::path outputfile = outputbasename + ".png";
		cv::imwrite(outputfile.string(), outimg);

		outputfile.replace_extension(".obj");
		core::write_textured_obj(_mesh, outputfile.string());

		outputfile.replace_extension(".isomap.png");
		cv::imwrite(outputfile.string(), _isomap);
	}
	eos::core::Mesh GetMesh() {
		return _mesh;
	}
	eos::morphablemodel::MorphableModel* GetMorphableModel() {
		return _morphable_model;
	}
	Mat& GetImage() {
		return _image;
	}
private:
	Mat _image;

	LandmarkCollection<Eigen::Vector2f> _landmarks;
	morphablemodel::MorphableModel* _morphable_model;
	core::LandmarkMapper _landmark_mapper;
	fitting::ModelContour _model_contour;
	fitting::ContourLandmarks _ibug_contour;
	morphablemodel::EdgeTopology _edge_topology;

	core::Mesh _mesh;
	fitting::RenderingParameters _rendering_params;
	Mat _isomap;

	int* _result;
};
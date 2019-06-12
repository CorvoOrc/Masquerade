#pragma once
#include "ILandmarkDetector.hpp"

#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>

#include <dlib/dnn.h>
#include <dlib/data_io.h>
using namespace dlib;
// ----------------------------------------------------------------------------------------

template <long num_filters, typename SUBNET> using con5d = con<num_filters, 5, 5, 2, 2, SUBNET>;
template <long num_filters, typename SUBNET> using con5  = con<num_filters, 5, 5, 1, 1, SUBNET>;

template <typename SUBNET> using downsampler  = relu<affine<con5d<32, relu<affine<con5d<32, relu<affine<con5d<16, SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5  = relu<affine<con5<45, SUBNET>>>;

using net_type = loss_mmod<con<1, 9, 9, 1, 1, rcon5<rcon5<rcon5<downsampler<input_rgb_image_pyramid<pyramid_down<6>>>>>>>>;

// ----------------------------------------------------------------------------------------


class DlibCNNLandmarkDetector : public ILandmarkDetector
{
public:
	DlibCNNLandmarkDetector() {}
	~DlibCNNLandmarkDetector() {}

public:
	void Init(string predictorPath)
	{
		using namespace dlib;

		string netPath = "../eos/share/mmod_human_face_detector.dat";
		deserialize(netPath) >> _net;
		deserialize(predictorPath) >> _predictor;
	}
	void DlibCNNLandmarkDetector::Detect(std::string imagePath)
	{
		using namespace dlib;

		matrix<rgb_pixel> img;
		load_image(img, imagePath);
		pyramid_up(img);

		auto dets = _net(img);
		std::cout << "Number of faces detected: " << dets.size() << std::endl;

		for (int i = 0; i < dets.size(); ++i) {
			rectangle rect = dets[i].rect;
			full_object_detection shape = _predictor(img, rect);
			_shapes.push_back(shape);
			std::cout << "number of parts: " << shape.num_parts() << std::endl;
		}

		win.set_image(img);
		win.add_overlay(render_face_detections(_shapes));
		dlib::array<array2d<rgb_pixel> > face_chips;
		extract_image_chips(img, get_face_chip_details(_shapes), face_chips);
		win_faces.set_image(tile_images(face_chips));
	}

	void DlibCNNLandmarkDetector::Serialize(std::string path, std::string extention)
	{
		for (int i = 0; i < _shapes.size(); i++)
		{
			std::ofstream file;
			string fullPath = path;
			fullPath.append("_").append(std::to_string(i)).append(extention);
			file.open(fullPath);
			WriteOpenBlock(file, _shapes[i]);
			WritePayload(file, _shapes[i]);
			WriteCloseBlock(file, _shapes[i]);
			file.close();
		}
	}

	int DlibCNNLandmarkDetector::GetPointSize()
	{
		return 68;
	}
private:
	void DlibCNNLandmarkDetector::WriteOpenBlock(std::ofstream &file, dlib::full_object_detection shape)
	{
		file << "version: 1" << std::endl 
			<< "n_points: " << shape.num_parts() << std::endl
			<< "{" << std::endl;
	}

	void DlibCNNLandmarkDetector::WritePayload(std::ofstream& file, dlib::full_object_detection shape)
	{
		double coefSyncToEos = 0.5f;
		for (int i = 0; i < shape.num_parts(); i++)
			file << coefSyncToEos * shape.part(i).x() << " " << coefSyncToEos * shape.part(i).y() << std::endl;
	}

	void DlibCNNLandmarkDetector::WriteCloseBlock(std::ofstream& file, dlib::full_object_detection shape)
	{
		file << "}";
	}
private:
	dlib::shape_predictor _predictor;
	std::vector<dlib::full_object_detection> _shapes;

	net_type _net;

	image_window win;
	image_window win_faces;
};
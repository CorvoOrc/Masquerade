#pragma once
#include "ILandmarkDetector.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>

using namespace dlib;

class DlibLandmarkDetector : public ILandmarkDetector {
public:
	DlibLandmarkDetector() {}
	~DlibLandmarkDetector() {}

public:
	void Init(string predictorPath) {
		using namespace dlib;

		_detector = get_frontal_face_detector();
		deserialize(predictorPath) >> _predictor;
	}
	void DlibLandmarkDetector::Detect(std::string imagePath) {
		using namespace dlib;

		array2d<rgb_pixel> img;
		load_image(img, imagePath);
		pyramid_up(img);

		std::vector<rectangle> dets = _detector(img);
		std::cout << "Number of faces detected: " << dets.size() << std::endl;

		for (unsigned long j = 0; j < dets.size(); ++j) {
			full_object_detection shape = _predictor(img, dets[j]);
			_shapes.push_back(shape);
			std::cout << "number of parts: " << shape.num_parts() << std::endl;
		}

		for (int j = 0; j < _shapes.size(); j++) {
			win.set_image(img);
			win.add_overlay(render_face_detections(_shapes));
		}
		
		dlib::array<array2d<rgb_pixel> > face_chips;
		extract_image_chips(img, get_face_chip_details(_shapes), face_chips);
		win_faces.set_image(tile_images(face_chips));
	}

	void DlibLandmarkDetector::Serialize(std::string path, std::string extention) {
		for (int i = 0; i < _shapes.size(); i++) {
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

	int DlibLandmarkDetector::GetPointSize() {
		return 68;
	}
private:
	void DlibLandmarkDetector::WriteOpenBlock(std::ofstream& file, dlib::full_object_detection shape) {
		file << "version: 1" << std::endl
			<< "n_points: " << shape.num_parts() << std::endl
			<< "{" << std::endl;
	}

	void DlibLandmarkDetector::WritePayload(std::ofstream& file, dlib::full_object_detection shape) {
		double coefSyncToEos = 0.5f;
		for (int i = 0; i < shape.num_parts(); i++)
			file << coefSyncToEos * shape.part(i).x() << " " << coefSyncToEos * shape.part(i).y() << std::endl;
	}

	void DlibLandmarkDetector::WriteCloseBlock(std::ofstream & file, dlib::full_object_detection shape) {
		file << "}";
	}
private:
	dlib::frontal_face_detector _detector;
	dlib::shape_predictor _predictor;
	std::vector<dlib::full_object_detection> _shapes;

	image_window win;
	image_window win_faces;
};
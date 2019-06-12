#pragma once

#include <string>

#include "Eigen/Core"
#include "igl/opengl/glfw/Viewer.h"
#include <igl/png/writePNG.h>

using std::string;

class ScreenCapture {
public:
	ScreenCapture() {}
	void Capture(string path, string filename, string extention, int cols, int rows, igl::opengl::glfw::Viewer& viewer) {
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> G(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> B(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> A(rows, cols);

		viewer.core.draw_buffer(viewer.data(), false, R, G, B, A);

		igl::png::writePNG(R, G, B, A, path + filename + extention);
	}
};
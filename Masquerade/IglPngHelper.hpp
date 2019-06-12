#pragma once

#include <string>
#include "Eigen/Core"
#include <igl_stb_image.h>

using std::string;

inline bool readDirectPNG(
	const string png_file,
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& R,
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& G,
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& B,
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic>& A
)
{
	int cols, rows, n;
	unsigned char* data = igl::stbi_load(png_file.c_str(), &cols, &rows, &n, 4);
	if (data == NULL) {
		return false;
	}

	R.resize(cols, rows);
	G.resize(cols, rows);
	B.resize(cols, rows);
	A.resize(cols, rows);

	for (unsigned i = 0; i < rows; ++i) {
		for (unsigned j = 0; j < cols; ++j) {
			R(j, i) = data[4 * (j + cols * i) + 0];
			G(j, i) = data[4 * (j + cols * i) + 1];
			B(j, i) = data[4 * (j + cols * i) + 2];
			A(j, i) = data[4 * (j + cols * i) + 3];
		}
	}

	igl::stbi_image_free(data);

	return true;
}
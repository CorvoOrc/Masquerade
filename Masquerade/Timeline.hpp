#pragma once
#include <ctime>
#include <vector>
#include "Eigen/Core"

using std::vector;
using Eigen::VectorXf;

struct Timeline {
	vector<Eigen::VectorXf> shapes;
	vector<vector<float>> coeffs;
	vector<int> num_vertices;
	vector<time_t> base_times;

	void Clear() {
		shapes.clear();
		coeffs.clear();
		num_vertices.clear();
		base_times.clear();
	}
};
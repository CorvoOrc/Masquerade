#pragma once

#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

using std::string; 
using std::ofstream;

class LandmarkSerializer {
public:
	LandmarkSerializer() { }
	void Serialize() {}
	void Edit(string inpath, string outpath) {
		std::ifstream file;
		std::ofstream outfile;
		file.open(inpath);
		outfile.open(outpath);
		string line;
		using std::getline;
		using std::string;
		using std::vector;
		getline(file, line); // 'version: 1'
		getline(file, line); // 'n_points : 68'
		getline(file, line); // '{'
		vector<vector<float>> _data;
		int i = 0;
		while (getline(file, line)) {
			if (line == "}")
				break;
			std::stringstream lineStream(line);
			_data.push_back(vector<float>());
			_data[i].push_back(0);
			_data[i].push_back(0);
			if (!(lineStream >> _data[i][0] >> _data[i][1]))
				throw std::runtime_error(string("Landmark format error while parsing the line: " + line));
			++i;
		}

		outfile << "version: 1" << std::endl
			<< "n_points: " << 68 << std::endl
			<< "{" << std::endl;
		for (int i = 0; i < 68; i++)
			outfile << _data[i][0] << " " << _data[i][1] << std::endl;
		outfile << "}";
		outfile.close();
		file.close();
	}
};
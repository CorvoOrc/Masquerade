#pragma once
#include "cereal/archives/binary.hpp"
#include "cereal/types/string.hpp"

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

struct Settings {
	int emotion_size;

	float animation_speed;
	float min_speed;
	float max_speed;
	float speed_offset;

	bool is_looped_animation;
	float interpolation_shape_step;
	long long interpolation_time_step;

	bool is_screen_capture;
	std::string screen_path;
	std::string screen_ex;

	friend class cereal::access;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(CEREAL_NVP(emotion_size), CEREAL_NVP(animation_speed), CEREAL_NVP(min_speed),
			CEREAL_NVP(max_speed), CEREAL_NVP(speed_offset), CEREAL_NVP(is_looped_animation),
			CEREAL_NVP(interpolation_shape_step), CEREAL_NVP(interpolation_time_step),
			CEREAL_NVP(is_screen_capture), CEREAL_NVP(screen_path), CEREAL_NVP(screen_ex));
	};
};
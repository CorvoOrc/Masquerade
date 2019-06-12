#pragma once

inline float Lerp(float a, float b, float t, float step) {
	return a + (b - a) * t / step;
}
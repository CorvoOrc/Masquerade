#pragma once
#include "Params.hpp"

class IViewer {
public:
	virtual void LoadSettings(string& filename) = 0;
	virtual void Show(RenderParams params) = 0;
};
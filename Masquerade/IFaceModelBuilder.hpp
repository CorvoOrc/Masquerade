#include <iostream>
#include <string>
#include <vector>

#include "eos/core/Mesh.hpp"
#include "eos/morphablemodel/MorphableModel.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using std::string;
using std::vector;

using std::cout;
using std::cin;
using std::endl;

using namespace eos;
using cv::Mat;

typedef int FACE_MODEL_BUILD_RESULT;

class IFaceModelBuilder
{
public:
	IFaceModelBuilder() {}
	virtual ~IFaceModelBuilder() {}

	virtual IFaceModelBuilder* LoadModel(string imagefile, string landmarksfile, string modelfile, string mappingsfile,
		string blendshapesfile, string contourfile, string edgetopologyfile) = 0;
	virtual IFaceModelBuilder* FitShape(vector<float>& pca_coeffs, vector<float>& blendshape_coeffs) = 0;
	virtual IFaceModelBuilder* FitTexture() = 0;

	virtual void Serialize(string outputbasename) = 0;

	virtual eos::core::Mesh GetMesh() = 0;
	virtual eos::morphablemodel::MorphableModel* GetMorphableModel() = 0;

	virtual Mat& GetImage() = 0;
};
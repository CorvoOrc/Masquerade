### Masquerade: header-only library for simple a animation of emotion

Contains two project - LandmarkDetector and Masquerade.

LandmarkDetector project provides the following functionality based on dlib:
- detection of face (presents HOF and CNN detectors)
- detection of face landmarks (ibug standard);
- landmarks serialization;

Masquerade project provides:
- face reconstruction by RGB image and face landmarks (used [Surrey Face Model](http://https://www.cvssp.org/faceweb/3dmm/facemodel/ "Surrey Face Model"));
- texturing of reconstuction;
- viewer for result observation based on libigl and imgui;
- animation of emotion used blendshapes for base point on timeline which processed by linear interpolation;
- auto screenshoting of base point on timeline;

## To be added
- full head 3D model;
- physics for model;
- color PCA model (included in paid version SFM only)
- target and actor animation (animation from an actor transferring to target face)
- face reconstruction with CNN

# Getting started
**Prerequisite**
- C++ 17;
- dlib (face landmark detection);
- eos (3D Morphable Face Model fitting framework);
- opencv2 (image loading only);
- libigl (geometry processing and visualisation)
- imgui (gui)
- cereal (serialization)
- boost (write params from console, system path operations)

**Usage**

Cmake will soon (but now all configuration is manual...I`m sorry)
- You need install all dependency (prerequisite);
- for LandmarkDetector set preprocessor definitions DLIB_HAVE_SSE2;DLIB_JPEG_SUPPORT;DLIB_PNG_SUPPORT;
- generate .pts file from LandmarkDetector (in case using own images) or take from ibug database (in case using ibug images)
- set configuration for Masquerade in animation_settings.json
- run Masquerade->Build Model->Set emotion coeffs and addition params if desired->Animate

**Usage as library**

Entry point is Masquerate.cpp. For using in you own project u need define IViewer interface:

`shared_ptr<Settings> settings = make_shared<Settings>();`
`shared_ptr<IChrono> chrono = make_shared<Chrono>();`
`shared_ptr<Timeline> timeline = make_shared<Timeline>();`
  
`IViewer* viewer = new IglViewer(settings, chrono, timeline);`
  
Then load settings:

`viewer->LoadSettings(animation_settings_file);`

Final part:

`viewer->Show(params);`

# Brief theory
Surray Face Model is 3D Morphable Model of human face as a linear combination of orthogonal basis vectors obtained by PCA of some samples.

Limitations: cannot represent all possible faces and extract facial details - wrinkles, folds. For break limitation you can see on methods with Convolution Neural Networks. 

# Examples
**Face detection**

>Input:

![Input](https://github.com/CorvoOrc/Masquerade/blob/master/images/IntelSummer.jpg)

>Faces:

![Faces](https://github.com/CorvoOrc/Masquerade/blob/master/images/intelFaces.png "Faces")

>Landmarks:

![Landmarks](https://github.com/CorvoOrc/Masquerade/blob/master/images/intelContours.png "Contours")

**Animation**

>Input:

![Amanda](https://github.com/CorvoOrc/Masquerade/blob/master/images/image_0010.png "Amanda")

>Mask:

![Mask](https://github.com/CorvoOrc/Masquerade/blob/master/images/image_0010Mask.png "Mask")

>Reconstruction:

![Reconstruction](https://github.com/CorvoOrc/Masquerade/blob/master/images/amanda_full.png "Reconstruction")

>Surprized animation:

![Animation cropped](https://github.com/CorvoOrc/Masquerade/blob/master/images/amanda_combined.png "Animation cropped")

>Input:

![Steshenko](https://github.com/CorvoOrc/Masquerade/blob/master/images/steshFace.png "Steshenko")

>Mask:

![Mask](https://github.com/CorvoOrc/Masquerade/blob/master/images/steshMask.png "Mask")

>Reconstruction:

![Reconstruction](https://github.com/CorvoOrc/Masquerade/blob/master/images/editor_common.png "Reconstruction")

>Angry animation:

![Angry animation](https://github.com/CorvoOrc/Masquerade/blob/master/images/corvo_combined.png "Angry animation")

>Angry animation wired:

![Angry animation wired](https://github.com/CorvoOrc/Masquerade/blob/master/images/corvo_combined_wired.png "Angry animation wired")

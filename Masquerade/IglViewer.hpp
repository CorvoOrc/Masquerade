#include "IViewer.hpp"

#ifdef _WIN32
	#include <windows.h>
	#undef max
	#undef min
#endif

#include <algorithm>

#include "eos/core/Mesh.hpp"
#include "eos/morphablemodel/Blendshape.hpp"
#include "eos/morphablemodel/MorphableModel.hpp"

#include "igl/opengl/glfw/Viewer.h"
#include "igl/opengl/glfw/imgui/ImGuiMenu.h"
#include <igl/png/writePNG.h>
#include <igl/png/readPNG.h>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

#include "IglPngHelper.hpp"

#include "Settings.hpp"
#include "InterpolationHelper.hpp"
#include "Chrono.hpp"
#include "Timeline.hpp"
#include "ImGuiReactiveProp.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;

#define BUFSIZE MAX_PATH

class IglViewer : public IViewer {
public:
	IglViewer(shared_ptr<Settings>& settings, shared_ptr<IChrono>& chrono, shared_ptr<Timeline>& timeline) {
		_is_model_builded = false;
		_is_animated = false;

		_settings = settings;

		_animation_speed = make_unique<ReactiveFloat>();
		_speed_change_callback = make_unique< MemberCallback<IglViewer, RectiveEventArgs<float>> >(&IglViewer::OnAnimationSpeedChanged, this);

		_chrono = chrono;
		_struck_callback = make_unique<MemberCallback<IglViewer, ChronoEventArgs>>(&IglViewer::OnStrucked, this);
		_chrono->GetStruck()->Add(_struck_callback.get());
		
		_timeline = timeline;
	}
	~IglViewer() {
		_chrono->GetStruck()->Remove(_struck_callback.get());
		_animation_speed->GetChanged()->Remove(_speed_change_callback.get());
	}
public:
	void LoadSettings(string& filename) {
		std::ifstream file(filename);
		if (!file)
			throw std::runtime_error("Error opening file for reading: " + filename);
		cereal::JSONInputArchive output_archive(file);
		output_archive(cereal::make_nvp("animation_settings", *_settings));

		_current_coeffs.clear();
		_finish_coeffs.clear();
		_base_expression_coeffs.clear();

		for (int i = 0; i < _settings->emotion_size; i++) {
			_current_coeffs.push_back(0);
			_finish_coeffs.push_back(0);
			_base_expression_coeffs.push_back(0);
		}

		_animation_speed->Set(_settings->animation_speed);
	}
	void Show(RenderParams params) {
		using namespace eos;
		using Eigen::VectorXf;

		igl::opengl::glfw::imgui::ImGuiMenu menu;
		viewer.plugins.push_back(&menu);

		menu.callback_draw_custom_window = [&]() {
			ImGui::SetNextWindowPos(ImVec2(0.f * menu.menu_scaling(), 585), ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(240, 280), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Morphable Model", nullptr, ImGuiWindowFlags_NoSavedSettings);
			if (ImGui::Button("Build model", ImVec2(-1, 0))) {
#ifdef _WIN32
				TCHAR Buffer[BUFSIZE];
				DWORD dwRet;
				dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
#endif
				string imagefile = igl::file_dialog_open();
#ifdef _WIN32
				if (!SetCurrentDirectory(Buffer)) {
					cout << "SetCurrentDirectory failed " << GetLastError() << endl;
					return;
				}
#endif
				cout << "Loading Image " << imagefile << "..." << endl;
				_imagefile = imagefile;
				fs::path imagePath = imagefile;
				imagePath.replace_extension(".pts");
				string landmarksfile = imagePath.generic_string();
				_landmarksfile = landmarksfile;
				BuildModel(viewer, imagefile, landmarksfile, params);
				RenderNeutralExpression(params);
			}
			if (ImGui::Button("Serialize", ImVec2(-1, 0))) {
				Serialize(params._outputbasename);
			}
			ImGui::End();
			if (_is_model_builded) {
				ImGui::SetNextWindowPos(ImVec2(180.f * menu.menu_scaling(), 0), ImGuiSetCond_FirstUseEver);
				ImGui::SetNextWindowSize(ImVec2(240, 583), ImGuiSetCond_FirstUseEver);
				ImGui::Begin("Emotion Animator", nullptr, ImGuiWindowFlags_NoSavedSettings);
				ImGui::Text("Current coeffs:");
				ImGui::BeginGroup();
				ImGui::LabelText("anger", std::to_string(_current_coeffs[0]).c_str());
				ImGui::LabelText("disgust", std::to_string(_current_coeffs[1]).c_str());
				ImGui::LabelText("fear", std::to_string(_current_coeffs[2]).c_str());
				ImGui::LabelText("happiness", std::to_string(_current_coeffs[3]).c_str());
				ImGui::LabelText("sadness", std::to_string(_current_coeffs[4]).c_str());
				ImGui::LabelText("surprise", std::to_string(_current_coeffs[5]).c_str());
				ImGui::EndGroup();
				ImGui::Text("Finish coeffs:");
				ImGui::BeginGroup();
				ImGui::SliderFloat("anger", &_finish_coeffs[0], -3.0, 3.0);
				ImGui::SliderFloat("disgust", &_finish_coeffs[1], -3.0, 3.0);
				ImGui::SliderFloat("fear", &_finish_coeffs[2], -3.0, 3.0);
				ImGui::SliderFloat("happiness", &_finish_coeffs[3], -3.0, 3.0);
				ImGui::SliderFloat("sadness", &_finish_coeffs[4], -3.0, 3.0);
				ImGui::SliderFloat("surprise", &_finish_coeffs[5], -3.0, 3.0);
				ImGui::EndGroup();

				if (!_is_animated && ImGui::Button("Animate", ImVec2(-1, 0)))
					Animate(_chrono.get(), _imagefile, _landmarksfile, params, _current_coeffs, _finish_coeffs);
				if (_is_animated && ImGui::Button("Stop", ImVec2(-1, 0)))
					Stop();

				if (ImGui::SliderFloat("Speed", &_animation_speed->GetRef(), _settings->min_speed, _settings->max_speed))
					_animation_speed->ForceCheck();
				ImGui::Checkbox("Looped", &_settings->is_looped_animation);
				ImGui::Checkbox("Screen Capture", &_settings->is_screen_capture);
				ImGui::End();
			}
			
			_chrono->Tick();
		};

		viewer.launch();
	}
	void ScreenCapture(string path, string filename, string extention) {
		auto cols = _model_builder->GetImage().cols;
		auto rows = _model_builder->GetImage().rows;
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> G(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> B(rows, cols);
		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> A(rows, cols);

		viewer.core.draw_buffer(viewer.data(), false, R, G, B, A);

		igl::png::writePNG(R, G, B, A, path + filename + extention);
	}
private:
	void OnStrucked(ChronoEventArgs* args) {
		if (_is_animated)
			UpdateAnimation(viewer, args->_chrono);
	}
	void OnAnimationSpeedChanged(RectiveEventArgs<float>* args) {
		auto ts = _chrono->GetStamp();
		UpdateBaseTime(ts);
	}
	void BuildModel(igl::opengl::glfw::Viewer& viewer, const string& imagefile, const string& landmarksfile, RenderParams params) {
		_model_builder = make_shared<SurreyFaceModelBuilder>();
		_model_builder
			->LoadModel(imagefile, landmarksfile, params._modelfile, params._mappingsfile, params._blendshapesfile, params._contourfile, params._edgetopologyfile)
			->FitShape(_base_shape_coeffs, _base_expression_coeffs)
			->FitTexture()
			->Serialize(params._outputbasename);

		_current_coeffs.clear();
		for (int i = 0; i < _base_expression_coeffs.size(); i++)
			_current_coeffs.push_back(_base_expression_coeffs[i]);

		_is_model_builded = true;
	}
	void RenderNeutralExpression(RenderParams params) {
		using Eigen::VectorXf;

		auto get_V = [](const core::Mesh & mesh) {
			Eigen::MatrixXd V(mesh.vertices.size(), 3);
			for (int i = 0; i < mesh.vertices.size(); ++i) {
				V(i, 0) = mesh.vertices[i](0);
				V(i, 1) = mesh.vertices[i](1);
				V(i, 2) = mesh.vertices[i](2);
			}
			return V;
		};
		auto get_F = [](const core::Mesh & mesh) {
			Eigen::MatrixXi F(mesh.tvi.size(), 3);
			for (int i = 0; i < mesh.tvi.size(); ++i) {
				F(i, 0) = mesh.tvi[i][0];
				F(i, 1) = mesh.tvi[i][1];
				F(i, 2) = mesh.tvi[i][2];
			}
			return F;
		};
		auto get_UV = [](const core::Mesh & mesh) {
			Eigen::MatrixXd V(mesh.texcoords.size(), 2);
			for (int i = 0; i < mesh.texcoords.size(); ++i) {
				V(i, 0) = mesh.texcoords[i][0];
				V(i, 1) = mesh.texcoords[i][1];
			}
			return V;
		};

		VectorXf shape = _model_builder->GetMorphableModel()->get_shape_model().draw_sample(_base_shape_coeffs);
		const auto num_vertices = _model_builder->GetMorphableModel()->get_shape_model().get_data_dimension() / 3;

		for (int i = 0; i < _base_expression_coeffs.size(); i++)
			_current_coeffs[i] = 0.0f;

		auto mesh = _model_builder->GetMesh();
		viewer.data().set_mesh(get_V(mesh), get_F(mesh));

		Eigen::Map<Eigen::MatrixXf> shape_reshaped(shape.data(), 3, num_vertices);
		viewer.data().set_vertices(shape_reshaped.transpose().cast<double>());

		viewer.core.align_camera_center(viewer.data().V, viewer.data().F);

		Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;
		readDirectPNG(params._outputbasename + ".isomap.png", R, G, B, A);

		viewer.data().show_texture = true;
		viewer.data().set_uv(get_UV(_model_builder->GetMesh()));
		viewer.data().set_texture(R, G, B);
		viewer.core.lighting_factor = 0;
		viewer.data().set_colors(Eigen::RowVector3d(1, 1, 1));
	}
	void Serialize(string& outputbasename) {
		_model_builder->Serialize(outputbasename);
	}
	void Animate(IChrono* chrono, const string& imagefile, const string& landmarksfile, RenderParams params, vector<float>& start_coefs, vector<float>& finish_coefs) {
		_timeline->Clear();
		if (_settings->is_screen_capture)
			ScreenCapture(_settings->screen_path, "screen", _settings->screen_ex);

		ComputeTimeline(start_coefs, finish_coefs);
		FillTimeline(chrono);
		
		PrepareAnimation();
	}
	inline void ComputeTimeline(vector<float>& start_coefs, vector<float>& finish_coefs) {
		vector<float> current_coeffs(start_coefs);
		vector<float> temp(current_coeffs);
		_timeline->coeffs.push_back(current_coeffs);
		bool is_value_step = false;
		do {
			is_value_step = false;
			for (int i = 0; i < temp.size(); i++)
				temp[i] = 0.0f;

			for (int i = 0; i < start_coefs.size(); i++) {
				if (abs(finish_coefs[i] - current_coeffs[i]) > 0.00001) {
					temp[i] = std::min(abs(finish_coefs[i] - current_coeffs[i]), _settings->interpolation_shape_step);
					temp[i] *= finish_coefs[i] < current_coeffs[i] ? -1 : 1;
					is_value_step = true;
				}
			}
			if (is_value_step) {
				for (int i = 0; i < current_coeffs.size(); i++)
					current_coeffs[i] += temp[i];
				_timeline->coeffs.push_back(current_coeffs);
			}
		} while (is_value_step);
		cout << "Animate base size " << _timeline->coeffs.size() << endl;
	}
	inline void FillTimeline(IChrono * chrono) {
		using Eigen::VectorXf;

		vector<float> pca_coeffs;
		auto ts = chrono->GetStamp();
		for (int i = 0; i < _timeline->coeffs.size(); i++) {
			VectorXf shape = _model_builder->GetMorphableModel()->get_shape_model().draw_sample(_base_shape_coeffs);
			const auto num_vertices = _model_builder->GetMorphableModel()->get_shape_model().get_data_dimension() / 3;
			const auto& blendshapes = eos::cpp17::get<morphablemodel::Blendshapes>(_model_builder->GetMorphableModel()->get_expression_model().value());
			for (int j = 0; j < blendshapes.size(); ++j)
				shape += blendshapes[j].deformation * _timeline->coeffs[i][j];

			_timeline->shapes.push_back(shape);
			_timeline->num_vertices.push_back(num_vertices);

			_timeline->base_times.push_back(ComputeBaseTime(ts, i));
		}
	}
	inline long long ComputeBaseTime(long long& ts, int& i) {
		auto speed = Clamp(_animation_speed->Get(), _settings->max_speed + _settings->speed_offset);
		return ts + (long long)(i * _settings->interpolation_time_step * speed);
	}
	inline void PrepareAnimation() {
		_is_animated = true;
		_current_frame = 0;
		_captured_frame = 0;
		viewer.core.is_animating = true;
		_animation_speed->GetChanged()->Add(_speed_change_callback.get());
	}
	void Stop() {
		_is_animated = false;
		_current_frame = 0;
		for_each(begin(_current_coeffs), end(_current_coeffs), [](auto & coeff) { coeff = 0.0f; });
		_animation_speed->GetChanged()->Remove(_speed_change_callback.get());
	}
	inline bool CheckUpdateAnimation(IChrono* chrono, long long& current_ts, long long& delta_time) {
		if (_current_frame == -1 || _current_frame == _timeline->shapes.size() - 1) {
			if (!_settings->is_looped_animation) {
				Stop();
				cout << "Animation finished" << endl;
				return false;
			}
			else {
				UpdateBaseTime(current_ts);
				_current_frame = FindTimelineFrame(chrono);
				delta_time = current_ts - _timeline->base_times[_current_frame - 1];
				cout << "Animation refreshed" << endl;
			}
		}
		else 
			delta_time = current_ts - _timeline->base_times[_current_frame - 1];
		return true;
	}
	inline void UpdateBaseTime(long long& ts) {
		for (int i = 0; i < _timeline->base_times.size(); i++)
			_timeline->base_times[i] = ComputeBaseTime(ts, i);
	}
	void UpdateAnimation(igl::opengl::glfw::Viewer& viewer, IChrono* chrono) {
		auto current_ts = chrono->GetStamp();
		_current_frame = FindTimelineFrame(chrono);
		long long delta_time = 0;

		if (!CheckUpdateAnimation(chrono, current_ts, delta_time))
			return;

		auto shape = _timeline->shapes[_current_frame];
		InterpolateShape(delta_time, shape);
		DrawFaceVertices(shape);
		UpdateAnimationCoeffs();
		if (_current_frame > _captured_frame && _settings->is_screen_capture) {
			ScreenCapture(_settings->screen_path, "screen" + std::to_string(_captured_frame), _settings->screen_ex);
			_captured_frame++;
		}
	}
	inline int FindTimelineFrame(IChrono* chrono) {
		auto now = chrono->GetStamp();
		for (int i = 0; i < _timeline->base_times.size(); i++)
			if (now < _timeline->base_times[i])
				return i;
		return -1;
	}
	inline void InterpolateShape(long long& delta_time, Eigen::VectorXf& shape) {
		auto rows_count = _timeline->shapes[_current_frame - 1].rows();
		auto cols_count = _timeline->shapes[_current_frame - 1].cols();
		auto a = _timeline->shapes[_current_frame - 1];
		auto b = _timeline->shapes[_current_frame];
		auto speed = Clamp(_animation_speed->Get(), _settings->max_speed + _settings->speed_offset);
		for (int i = 0; i < rows_count; i++)
			for (int j = 0; j < cols_count; j++)
				shape(i, j) = Lerp(a(i, j), b(i, j), delta_time, (float)_settings->interpolation_time_step * speed);
	}
	inline float Clamp(float value, float base, float separator=10.0) {
		return (((base - value) * separator) / separator);
	}
	inline void DrawFaceVertices(Eigen::VectorXf& shape) {
		auto num_vertices = _timeline->num_vertices[_current_frame];
		Eigen::Map<Eigen::MatrixXf> shape_instance_reshaped(shape.data(), 3, num_vertices);
		viewer.data().set_vertices(shape_instance_reshaped.transpose().cast<double>());
	}
	inline void UpdateAnimationCoeffs() {
		cout << "currentAnimationFrame: " << _current_frame << endl;
		for (int i = 0; i < _settings->emotion_size; i++)
			_current_coeffs[i] = _timeline->coeffs[_current_frame][i];
	}
private:
	shared_ptr<Settings> _settings;

	unique_ptr<ReactiveFloat> _animation_speed;
	unique_ptr<ICallback> _speed_change_callback;

	shared_ptr<IFaceModelBuilder> _model_builder;

	string _imagefile;
	string _landmarksfile;

	bool _is_model_builded;
	bool _is_animated;

	vector<float> _base_shape_coeffs;
	vector<float> _base_expression_coeffs;
	vector<float> _current_coeffs;
	vector<float> _finish_coeffs;
	int _current_frame;
	int _captured_frame;

	shared_ptr<Timeline> _timeline;
	shared_ptr<IChrono> _chrono;
	unique_ptr<ICallback> _struck_callback;

	igl::opengl::glfw::Viewer viewer;
};
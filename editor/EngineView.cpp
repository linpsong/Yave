/*******************************
Copyright (c) 2016-2018 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "EngineView.h"

#include <editor/context/EditorContext.h>

#include <yave/scene/SceneView.h>
#include <yave/renderers/ToneMapper.h>
#include <yave/window/EventHandler.h>

#include <yave/framegraph/renderers.h>

#include <y/math/Volume.h>
#include <y/io/File.h>

#include <imgui/imgui.h>

namespace editor {

EngineView::EngineView(ContextPtr cptr) :
		Widget(ICON_FA_DESKTOP " Engine View"),
		ContextLinked(cptr),
		_ibl_data(std::make_shared<IBLData>(device())),
		_resources(std::make_shared<FrameGraphResourcePool>(device())),
		_scene_view(context()->scene().scene()),
		_gizmo(context()) {
	;
}

EngineView::~EngineView() {
	context()->scene().reset_scene_view(&_scene_view);
}

void EngineView::paint_ui(CmdBufferRecorder& recorder, const FrameToken& token) {
	update();

	TextureView* output = nullptr;

	if(_resources) {
		FrameGraph graph(_resources);
		auto gbuffer = render_gbuffer(graph, &_scene_view, content_size());
		auto lighting = render_lighting(graph, gbuffer, _ibl_data);
		auto tone_mapping = render_tone_mapping(graph, lighting);

		FrameGraphImageId output_image = tone_mapping.tone_mapped;
		{
			FrameGraphPassBuilder builder = graph.add_pass("ImGui texture pass");
			builder.add_texture_input(output_image, PipelineStage::FragmentBit);
			builder.set_render_func([&output, output_image](CmdBufferRecorder& rec, const FrameGraphPass* pass) {
					auto out = std::make_unique<TextureView>(pass->resources()->get_image<ImageUsage::TextureBit>(output_image));
					output = out.get();
					rec.keep_alive(std::move(out));
				});
		}

		std::move(graph).render(recorder);
		recorder.keep_alive(std::pair(_ibl_data, _resources));
	}

	// ImGui
	{
		if(output) {
			ImGui::GetWindowDrawList()->AddImage(output,
				position() + math::Vec2(ImGui::GetWindowContentRegionMin()),
				position() + math::Vec2(ImGui::GetWindowContentRegionMax()));

		}

		_gizmo.paint(recorder, token);
		if(!_gizmo.is_dragging()) {
			update_selection();
		}
	}
}

void EngineView::update() {
	if(ImGui::IsWindowHovered()) {
		if(ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(2)) {
			ImGui::SetWindowFocus();
		}
	}

	if(ImGui::IsWindowFocused()) {
		context()->scene().set_scene_view(&_scene_view);
	}

	// process inputs
	update_camera();
}




void EngineView::update_camera() {
	auto size = content_size();
	auto& camera = _scene_view.camera();

	math::Vec3 cam_pos = camera.position();
	math::Vec3 cam_fwd = camera.forward();
	math::Vec3 cam_lft = camera.left();

	if(ImGui::IsWindowFocused()) {

		float cam_speed = 500.0f;
		float dt = cam_speed / ImGui::GetIO().Framerate;

		if(ImGui::IsKeyDown(int(context()->settings().camera().move_forward))) {
			cam_pos += cam_fwd * dt;
		}
		if(ImGui::IsKeyDown(int(context()->settings().camera().move_backward))) {
			cam_pos -= cam_fwd * dt;
		}
		if(ImGui::IsKeyDown(int(context()->settings().camera().move_left))) {
			cam_pos += cam_lft * dt;
		}
		if(ImGui::IsKeyDown(int(context()->settings().camera().move_right))) {
			cam_pos -= cam_lft * dt;
		}


		if(ImGui::IsMouseDown(1)) {
			auto delta = math::Vec2(ImGui::GetIO().MouseDelta) / math::Vec2(ImGui::GetWindowSize());
			delta *= context()->settings().camera().sensitivity;

			{
				auto pitch = math::Quaternion<>::from_axis_angle(cam_lft, delta.y());
				cam_fwd = pitch(cam_fwd);
			}
			{
				auto yaw = math::Quaternion<>::from_axis_angle(cam_fwd.cross(cam_lft), -delta.x());
				cam_fwd = yaw(cam_fwd);
				cam_lft = yaw(cam_lft);
			}

			auto euler = math::Quaternion<>::from_base(cam_fwd, cam_lft, cam_fwd.cross(cam_lft)).to_euler();
			bool upside_down = cam_fwd.cross(cam_lft).z() < 0.0f;
			euler[math::Quaternion<>::RollIndex] = upside_down ? -math::pi<float> : 0.0f;
			auto rotation = math::Quaternion<>::from_euler(euler);
			cam_fwd = rotation({1.0f, 0.0f, 0.0f});
			cam_lft = rotation({0.0f, 1.0f, 0.0f});
		}


		if(ImGui::IsMouseDown(2)) {
			auto delta = ImGui::GetIO().MouseDelta;
			cam_pos -= (delta.y * cam_fwd.cross(cam_lft) + delta.x * cam_lft);
		}
	}

	float fov = math::to_rad(60.0f);
	auto proj = math::perspective(fov, float(size.x()) / float(size.y()), 1.0f);
	auto view = math::look_at(cam_pos, cam_pos + cam_fwd, cam_fwd.cross(cam_lft));
	camera.set_proj(proj);
	camera.set_view(view);
}

void EngineView::update_selection() {
	if(!ImGui::IsWindowHovered() || !ImGui::IsMouseClicked(0)) {
		return;
	}

	math::Vec2 viewport = ImGui::GetWindowSize();
	math::Vec2 offset = ImGui::GetWindowPos();

	auto inv_matrix = _scene_view.camera().inverse_matrix();
	auto cam_pos = _scene_view.camera().position();

	math::Vec2 ndc = ((math::Vec2(ImGui::GetIO().MousePos) - offset) / viewport) * 2.0f - 1.0f;
	math::Vec4 h_world = inv_matrix * math::Vec4(ndc, 0.5f, 1.0f);
	math::Vec3 world = h_world.to<3>() / h_world.w();

	math::Ray<> ray(cam_pos, world - cam_pos);

	float distance = std::numeric_limits<float>::max();
	for(const auto& tr : context()->scene().scene().static_meshes()) {
		auto [pos, rot, sc] = tr->transform().decompose();
		unused(rot);

		float scale = std::max({sc.x(), sc.y(), sc.z()});
		float dist = (pos - cam_pos).length();

		if(ray.intersects(pos, tr->radius() * scale) && dist < distance) {
			context()->selection().set_selected(tr.get());
			distance = dist;
		}
	}

	if(distance == std::numeric_limits<float>::max()) {
		context()->selection().set_selected(nullptr);
	}
}

}

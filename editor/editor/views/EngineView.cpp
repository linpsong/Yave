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

#include <editor/EditorContext.h>

#include <yave/renderers/ToneMapper.h>
#include <yave/window/EventHandler.h>

#include <y/io/File.h>

#include <imgui/imgui.h>

namespace editor {

EngineView::EngineView(ContextPtr cptr) :
		Dock("Engine view"),
		ContextLinked(cptr),
		_ibl_data(new IBLData(device())),
		_uniform_buffer(device(), 1),
		_gizmo(context()) {
}

math::Vec2ui EngineView::render_size() const {
	return _renderer ? _renderer->output().size() : math::Vec2ui();
}

void EngineView::create_renderer(const math::Vec2ui& size) {
	auto scene		= Node::Ptr<SceneRenderer>(new SceneRenderer(device(), *context()->scene_view()));
	auto gbuffer	= Node::Ptr<GBufferRenderer>(new GBufferRenderer(scene, size));
	auto deferred	= Node::Ptr<TiledDeferredRenderer>(new TiledDeferredRenderer(gbuffer, _ibl_data));
	auto tonemap	= Node::Ptr<SecondaryRenderer>(new ToneMapper(deferred));

	_renderer		= Node::Ptr<FramebufferRenderer>(new FramebufferRenderer(tonemap, size));

	_ui_material = new Material(device(), MaterialData()
			.set_frag_data(SpirVData::from_file(io::File::open("copy.frag.spv").expected("Unable to load spirv file.")))
			.set_vert_data(SpirVData::from_file(io::File::open("screen.vert.spv").expected("Unable to load spirv file.")))
			.set_bindings({Binding(_renderer->output()), Binding(_uniform_buffer)})
			.set_depth_tested(false)
		);
}

void EngineView::draw_callback(RenderPassRecorder& recorder, void* user_data) {
	reinterpret_cast<EngineView*>(user_data)->render_ui(recorder);
}

// called via draw_callback ONLY
void EngineView::render_ui(RenderPassRecorder& recorder) {
	if(!_renderer) {
		y_fatal("Internal error.");
	}

	auto region = recorder.region("EngineView::render_ui");

	recorder.bind_material(*_ui_material);
	recorder.draw(vk::DrawIndirectCommand(6, 1));
}

void EngineView::paint_ui(CmdBufferRecorder<>& recorder, const FrameToken& token) {
	math::Vec2 win_size = ImGui::GetWindowSize();
	math::Vec2 win_pos = ImGui::GetWindowPos();

	if(!_renderer || win_size != render_size()) {
		create_renderer(win_size);
	}

	if(_renderer) {
		// process inputs
		update_camera();

		// render engine
		{
			RenderingPipeline pipeline(_renderer);
			pipeline.render(recorder, token);

			// so we don't have to wait when resizing
			recorder.keep_alive(_renderer);
			recorder.keep_alive(_ui_material);
		}

		// ui stuff
		auto map = TypedMapping(_uniform_buffer);
		map[0] = ViewData{math::Vec2i(win_size), math::Vec2i(win_pos), math::Vec2i(win_size)};

		ImGui::GetWindowDrawList()->AddCallback(reinterpret_cast<ImDrawCallback>(&draw_callback), this);
	}

	_gizmo.paint(recorder, token);

}

void EngineView::update_camera() {
	if(!ImGui::IsWindowFocused()) {
		return;
	}

	auto size = render_size();
	auto& camera = context()->scene_view()->camera();
	math::Vec3 cam_pos = camera.position();
	math::Vec3 cam_fwd = camera.forward();
	math::Vec3 cam_lft = camera.left();


	float cam_speed = 500.0f;
	float dt = cam_speed / ImGui::GetIO().Framerate;

	if(ImGui::IsKeyDown(int(context()->key_settings.move_forward))) {
		cam_pos += cam_fwd * dt;
	}
	if(ImGui::IsKeyDown(int(context()->key_settings.move_backward))) {
		cam_pos -= cam_fwd * dt;
	}
	if(ImGui::IsKeyDown(int(context()->key_settings.move_left))) {
		cam_pos += cam_lft * dt;
	}
	if(ImGui::IsKeyDown(int(context()->key_settings.move_right))) {
		cam_pos -= cam_lft * dt;
	}



	auto proj = math::perspective(math::to_rad(60.0f), float(size.x()) / float(size.y()), 1.0f);
	auto view = math::look_at(cam_pos, cam_pos + cam_fwd, cam_fwd.cross(cam_lft));
	camera.set_proj(proj);
	camera.set_view(view);
}

}

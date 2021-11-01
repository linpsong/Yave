/*******************************
Copyright (c) 2016-2021 Grégoire Angerand

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

#include <editor/Widget.h>
#include <editor/EditorWorld.h>

#include <yave/scene/SceneView.h>
#include <yave/systems/OctreeSystem.h>

#include <external/imgui/yave_imgui.h>

namespace editor {

class CameraDebug : public Widget {
    editor_widget(CameraDebug, "View", "Debug")

    public:
        CameraDebug() : Widget(ICON_FA_VIDEO " Camera debug", ImGuiWindowFlags_AlwaysAutoResize) {
        }

    protected:
        void on_gui() override {
            const Camera& camera = scene_view().camera();
            const math::Vec3 pos = camera.position();
            const math::Vec3 fwd = camera.forward();
            const math::Vec3 rht = camera.right();
            const math::Vec3 up = fwd.cross(rht);

            const math::Quaternion<> rot = math::Quaternion<>::from_base(fwd, rht, up);

            ImGui::Text("FoV: %.1f", camera.field_of_view());
            ImGui::Text("Aspect ratio: %.2f", camera.aspect_ratio());

            ImGui::Separator();

            ImGui::Text("position: %.1f, %.1f, %.1f", pos.x(), pos.y(), pos.z());
            ImGui::Text("forward : %.1f, %.1f, %.1f", fwd.x(), fwd.y(), fwd.z());
            ImGui::Text("right   : %.1f, %.1f, %.1f", rht.x(), rht.y(), rht.z());
            ImGui::Text("up      : %.1f, %.1f, %.1f", up.x(), up.y(), up.z());

            ImGui::Text("rotation: %.1f, %.1f, %.1f, %.1f", rot.x(), rot.y(), rot.z(), rot.w());

            if(ImGui::CollapsingHeader("Rotation")) {
                const math::Vec3 x = rot({1.0f, 0.0f, 0.0f});
                const math::Vec3 y = rot({0.0f, 1.0f, 0.0f});
                const math::Vec3 z = rot({0.0f, 0.0f, 1.0f});
                ImGui::Text("X axis: %.1f, %.1f, %.1f", x.x(), x.y(), x.z());
                ImGui::Text("Y axis: %.1f, %.1f, %.1f", y.x(), y.y(), y.z());
                ImGui::Text("Z axis: %.1f, %.1f, %.1f", z.x(), z.y(), z.z());

                ImGui::Separator();

                auto euler = rot.to_euler();
                ImGui::Text("pitch: %.1f°", math::to_deg(euler[math::Quaternion<>::PitchIndex]));
                ImGui::Text("yaw  : %.1f°", math::to_deg(euler[math::Quaternion<>::YawIndex]));
                ImGui::Text("roll : %.1f°", math::to_deg(euler[math::Quaternion<>::RollIndex]));
            }
        }
};



class CullingDebug : public Widget {
    editor_widget(CullingDebug, "View", "Debug")

    public:
        CullingDebug() : Widget("Culling debug", ImGuiWindowFlags_AlwaysAutoResize) {
        }

    protected:
        void on_gui() override {
            const EditorWorld& world = current_world();
            const Camera& camera = scene_view().camera();

            core::Vector<ecs::EntityId> visible;
            const OctreeSystem* octree_system = world.find_system<OctreeSystem>();
            if(octree_system) {
                visible = octree_system->octree().find_entities(camera.frustum());
            }

            const usize in_frustum = visible.size();
            const usize total = world.component_ids<TransformableComponent>().size();

            ImGui::Text("%u entities in octree", u32(total));
            ImGui::Text("%u entities in frustum", u32(in_frustum));
            ImGui::Text("%u%% culled", u32(float(total - in_frustum) / float(total) * 100.0f));

        }
};



}
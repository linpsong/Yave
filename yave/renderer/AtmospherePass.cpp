/*******************************
Copyright (c) 2016-2023 Grégoire Angerand

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

#include "AtmospherePass.h"

#include <yave/framegraph/FrameGraph.h>
#include <yave/framegraph/FrameGraphPass.h>
#include <yave/framegraph/FrameGraphFrameResources.h>
#include <yave/graphics/device/DeviceResources.h>
#include <yave/graphics/commands/CmdBufferRecorder.h>

#include <yave/components/DirectionalLightComponent.h>
#include <yave/components/AtmosphereComponent.h>
#include <yave/meshes/StaticMesh.h>
#include <yave/ecs/EntityWorld.h>

#include <y/core/Chrono.h>

namespace yave {

static const DirectionalLightComponent* find_sun(const SceneView& scene) {
    for(const auto& [id, comp] : scene.world().query<DirectionalLightComponent>(ecs::tags::not_hidden)) {
        const auto& [sun] = comp;
        return &sun;
    }

    return nullptr;
}

static const AtmosphereComponent* find_atmosphere_component(const SceneView& scene) {
    for(const auto& [id, comp] : scene.world().query<AtmosphereComponent>(ecs::tags::not_hidden)) {
        const auto& [atmo] = comp;
        return &atmo;
    }

    return nullptr;
}


static FrameGraphImageId integrate_optical_depth(FrameGraph& framegraph, const uniform::AtmosphereParams& params) {
    const math::Vec2ui size = math::Vec2ui(128, 128);
    const ImageFormat format = VK_FORMAT_R32_SFLOAT;


    FrameGraphPassBuilder builder = framegraph.add_pass("Atmosphere integration pass");

    const auto integrated = builder.declare_image(format, size);

    builder.add_inline_input(InlineDescriptor(params));
    builder.add_color_output(integrated);
    builder.set_render_func([=](RenderPassRecorder& render_pass, const FrameGraphPass* self) {
        const auto* material = device_resources()[DeviceResources::AtmosphereIntegrationMaterialTemplate];
        render_pass.bind_material_template(material, self->descriptor_sets()[0]);
        render_pass.draw_array(3);
    });

    return integrated;
}

AtmospherePass AtmospherePass::create(FrameGraph& framegraph, const GBufferPass& gbuffer, FrameGraphImageId lit) {
    const AtmosphereComponent* atmosphere = find_atmosphere_component(gbuffer.scene_pass.scene_view);
    const DirectionalLightComponent* sun = find_sun(gbuffer.scene_pass.scene_view);

    if(!atmosphere || !sun) {
        AtmospherePass pass;
        pass.lit = lit;
        return pass;
    }

    const auto region = framegraph.region("Atmosphere");

    static constexpr math::Vec3 wavelengths(700.0f, 530.0f, 440.0f);
    auto rayleigh = [](float scattering_strength) {
        math::Vec3 v = wavelengths;
        for(auto& x : v) {
            x = std::pow(400.0f / x, 4.0f) * scattering_strength;
        }
        return v;
    };

    const uniform::AtmosphereParams params {
        math::Vec3(0.0f, 0.0f, -atmosphere->planet_radius),
        atmosphere->planet_radius,

        rayleigh(atmosphere->scattering_strength),
        atmosphere->atmosphere_height,

        -sun->direction().normalized(),
        atmosphere->planet_radius + atmosphere->atmosphere_height,

        sun->color() * sun->intensity(),
        atmosphere->density_falloff,
    };


    const FrameGraphImageId optical_depth_lut = integrate_optical_depth(framegraph, params);


    FrameGraphPassBuilder builder = framegraph.add_pass("Atmosphere pass");

    const auto atmo = builder.declare_copy(lit);

    builder.add_uniform_input(gbuffer.depth);
    builder.add_uniform_input(lit);
    builder.add_uniform_input(optical_depth_lut);
    builder.add_uniform_input(gbuffer.scene_pass.camera_buffer);
    builder.add_inline_input(InlineDescriptor(params), 0);
    builder.add_color_output(atmo);
    builder.set_render_func([=](RenderPassRecorder& render_pass, const FrameGraphPass* self) {
        const auto* material = device_resources()[DeviceResources::AtmosphereMaterialTemplate];
        render_pass.bind_material_template(material, self->descriptor_sets()[0]);
        render_pass.draw_array(3);
    });


    AtmospherePass pass;
    pass.lit = atmo;
    return pass;
}

}


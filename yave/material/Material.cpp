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
#include "Material.h"
#include "MaterialTemplate.h"

#include <yave/graphics/graphics.h>
#include <yave/graphics/device/DeviceResources.h>
#include <yave/graphics/images/TextureLibrary.h>

namespace yave {

static auto material_texture_views(const SimpleMaterialData& data) {
    std::array<TextureView, SimpleMaterialData::texture_count> textures = {
        *device_resources()[DeviceResources::GreyTexture],          // Diffuse
        *device_resources()[DeviceResources::FlatNormalTexture],    // Normal
        *device_resources()[DeviceResources::WhiteTexture],         // Roughness
        *device_resources()[DeviceResources::WhiteTexture],         // Metallic
        *device_resources()[DeviceResources::WhiteTexture],         // Emissive
    };

    for(usize i = 0; i != SimpleMaterialData::texture_count; ++i) {
        y_debug_assert(!data.textures()[i].is_loading());
        if(const auto* tex = data.textures()[i].get()) {
            textures[i] = *tex;
        }
    }

    return textures;
}

static DescriptorSet create_descriptor_set(const SimpleMaterialData& data) {
    const auto textures = material_texture_views(data);

    std::array<u32, textures.size()> texture_indices = {};
    for(usize i = 0; i != textures.size(); ++i) {
        texture_indices[i] = texture_library().add_texture(textures[i]);
    }

    std::array<u8, sizeof(texture_indices) + sizeof(data.constants())> bytes;
    std::memcpy(bytes.data(), &data.constants(), sizeof(data.constants()));
    std::memcpy(bytes.data()+ sizeof(data.constants()), texture_indices.data(), sizeof(texture_indices));

    return DescriptorSet({InlineDescriptor(bytes)});
}

static DeviceResources::MaterialTemplates material_template_for_data(const SimpleMaterialData& data) {
    if(data.alpha_tested()) {
       return DeviceResources::TexturedAlphaMaterialTemplate;
    }
    return DeviceResources::TexturedMaterialTemplate;
}



Material::Material(SimpleMaterialData&& data) :
        _template(device_resources()[material_template_for_data(data)]),
        _set(create_descriptor_set(data)),
        _data(std::move(data)) {
}

Material::Material(const MaterialTemplate* tmp, SimpleMaterialData&& data) :
        _template(tmp),
        _set(create_descriptor_set(data)),
        _data(std::move(data)) {
}

Material::~Material() {
    if(!is_null()) {
        // Todo, textures might still be used by command buffer
        for(const TextureView& tex : material_texture_views(_data)) {
            texture_library().remove_texture(tex);
        }
    }
}

const SimpleMaterialData& Material::data() const {
    return _data;
}

DescriptorSetBase Material::descriptor_set() const {
    return _set;
}

const MaterialTemplate* Material::material_template() const {
    return _template;
}

bool Material::is_null() const {
    return _set.is_null();
}

}


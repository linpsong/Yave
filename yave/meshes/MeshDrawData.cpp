/*******************************
Copyright (c) 2016-2022 Grégoire Angerand

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

#include "MeshDrawData.h"

#include <yave/graphics/device/MeshAllocator.h>

namespace yave {

std::array<AttribSubBuffer, 3> MeshBufferData::untyped_attrib_buffers() const {
    y_debug_assert(_attrib_buffers.positions.size() == _attrib_buffers.normals_tangents.size());
    y_debug_assert(_attrib_buffers.positions.size() == _attrib_buffers.uvs.size());

    return std::array<AttribSubBuffer, 3>{
        _attrib_buffers.positions,
        _attrib_buffers.normals_tangents,
        _attrib_buffers.uvs,
    };
}

u64 MeshBufferData::attrib_buffer_elem_count() const {
    return _attrib_buffers.positions.size();
}

TriangleSubBuffer MeshBufferData::triangle_buffer() const {
    return _triangle_buffer;
}

const TypedAttribSubBuffer<math::Vec3>& MeshBufferData::position_buffer() const {
    return _attrib_buffers.positions;
}

MeshAllocator* MeshBufferData::parent() const {
    return _parent;
}



MeshDrawData::MeshDrawData(MeshDrawData&& other) {
    swap(other);
}

MeshDrawData& MeshDrawData::operator=(MeshDrawData&& other) {
    swap(other);
    return *this;
}

MeshDrawData::~MeshDrawData() {
    y_debug_assert(is_null());
}

void MeshDrawData::recycle() {
    y_debug_assert(_buffer_data);
    _buffer_data->parent()->recycle(this);
}

bool MeshDrawData::is_null() const {
    return !_buffer_data;
}

TriangleSubBuffer MeshDrawData::triangle_buffer() const {
    y_debug_assert(_buffer_data);
    return _buffer_data->triangle_buffer();
}

const TypedAttribSubBuffer<math::Vec3>& MeshDrawData::position_buffer() const {
    y_debug_assert(_buffer_data);
    return _buffer_data->position_buffer();
}

const MeshBufferData& MeshDrawData::mesh_buffers() const {
    y_debug_assert(_buffer_data);
    return *_buffer_data;
}

const VkDrawIndexedIndirectCommand& MeshDrawData::indirect_data() const {
    return _indirect_data;
}

void MeshDrawData::swap(MeshDrawData& other) {
    std::swap(_indirect_data, other._indirect_data);
    std::swap(_buffer_data, other._buffer_data);
    std::swap(_vertex_count, other._vertex_count);
}

}


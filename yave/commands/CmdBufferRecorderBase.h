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
#ifndef YAVE_COMMANDS_CMDBUFFERRECORDERBASE_H
#define YAVE_COMMANDS_CMDBUFFERRECORDERBASE_H

#include <yave/yave.h>
#include <yave/framebuffer/Framebuffer.h>
#include <yave/barriers/Barrier.h>
#include "CmdBuffer.h"

#include <yave/material/GraphicPipeline.h>
#include <yave/shaders/ComputeProgram.h>
#include <yave/framebuffer/Viewport.h>


namespace yave {

class DescriptorSetBase;

namespace detail {
using DescriptorSetList = std::initializer_list<std::reference_wrapper<const DescriptorSetBase>>;
}


class PushConstant : NonCopyable {
		template<typename... Args>
		static std::true_type is_tuple(const std::tuple<Args...>&);

		template<typename T>
		static std::false_type is_tuple(const T&);

	public:
		constexpr PushConstant() = default;

		template<typename T>
		constexpr PushConstant(const T& data) : _data(&data), _size(sizeof(T)) {
			static_assert(sizeof(T) % 4 == 0, "PushConstant's size must be a multiple of 4");
			static_assert(!decltype(is_tuple(data))::value, "std::tuple is not standard layout");
		}

		template<typename T>
		constexpr PushConstant(const core::ArrayView<T>& arr) : _data(arr.data()), _size(arr.size(), sizeof(T)) {
			static_assert(sizeof(T) % 4 == 0, "PushConstant's size must be a multiple of 4");
			static_assert(!decltype(is_tuple(*arr.data()))::value, "std::tuple is not standard layout");
		}

		PushConstant(PushConstant&&) = delete;
		PushConstant& operator=(PushConstant&&) = delete;

		const void* data() const {
			return _data;
		}

		usize size() const {
			return _size;
		}

		bool is_empty() const {
			return !_size;
		}

	private:
		const void* _data = nullptr;
		usize _size = 0;
};

struct CmdBufferRegion : public DeviceLinked, NonCopyable {
	public:
		CmdBufferRegion() = default;

		~CmdBufferRegion();

		CmdBufferRegion(CmdBufferRegion&&) = delete;
		CmdBufferRegion& operator=(CmdBufferRegion&&) = delete;

	private:
		friend class CmdBufferRecorderBase;

		CmdBufferRegion(const CmdBufferRecorderBase& cmd_buffer, const char* name, const math::Vec4& color);

		vk::CommandBuffer _buffer;
};

class RenderPassRecorder : NonCopyable {
	public:
		using DescriptorSetList = detail::DescriptorSetList;

		~RenderPassRecorder();

		// specific
		void bind_material(const Material& material, DescriptorSetList descriptor_sets = {});
		void bind_pipeline(const GraphicPipeline& pipeline, DescriptorSetList descriptor_sets);

		void draw(const vk::DrawIndexedIndirectCommand& indirect);

		void bind_buffers(const SubBuffer<BufferUsage::IndexBit>& indices, const core::ArrayView<SubBuffer<BufferUsage::AttributeBit>>& attribs);
		void bind_index_buffer(const SubBuffer<BufferUsage::IndexBit>& indices);
		void bind_attrib_buffers(const core::ArrayView<SubBuffer<BufferUsage::AttributeBit>>& attribs);

		const Viewport& viewport() const;

		// proxies from _cmd_buffer
		CmdBufferRegion region(const char* name, const math::Vec4& color = math::Vec4());
		DevicePtr device() const;
		vk::CommandBuffer vk_cmd_buffer() const;

	private:
		friend class CmdBufferRecorderBase;

		RenderPassRecorder(CmdBufferRecorderBase& cmd_buffer, const Viewport& viewport);

		CmdBufferRecorderBase& _cmd_buffer;
		Viewport _viewport;
};

class CmdBufferRecorderBase : public CmdBufferBase {
	public:
		using DescriptorSetList = detail::DescriptorSetList;

		~CmdBufferRecorderBase();

		CmdBufferRegion region(const char* name, const math::Vec4& color = math::Vec4());

		RenderPassRecorder bind_framebuffer(const Framebuffer& framebuffer);

		void dispatch(const ComputeProgram& program, const math::Vec3ui& size, DescriptorSetList descriptor_sets, const PushConstant& push_constants = PushConstant());

		void dispatch_size(const ComputeProgram& program, const math::Vec3ui& size, DescriptorSetList descriptor_sets, const PushConstant& push_constants = PushConstant());
		void dispatch_size(const ComputeProgram& program, const math::Vec2ui& size, DescriptorSetList descriptor_sets, const PushConstant& push_constants = PushConstant());

		void barriers(const core::ArrayView<BufferBarrier>& buffers, const core::ArrayView<ImageBarrier>& images, PipelineStage src, PipelineStage dst);
		void barriers(const core::ArrayView<BufferBarrier>& buffers, PipelineStage src, PipelineStage dst);
		void barriers(const core::ArrayView<ImageBarrier>& images, PipelineStage src, PipelineStage dst);

		// never use directly, needed for internal work and image loading
		void transition_image(ImageBase& image, vk::ImageLayout src, vk::ImageLayout dst);

		template<typename T>
		void keep_alive(T&& t) {
			CmdBufferBase::keep_alive(std::forward<T>(t));
		}

	protected:
		CmdBufferRecorderBase() = default;
		CmdBufferRecorderBase(CmdBufferBase&& base, CmdBufferUsage usage);

		void swap(CmdBufferRecorderBase& other);

	private:
		friend class RenderPassRecorder;

		void end_renderpass();
		void check_no_renderpass() const;
		void bind_framebuffer(const Framebuffer& framebuffer, vk::SubpassContents subpass);

		// this could be in RenderPassRecorder, but putting it here makes erroring easier
		const RenderPass* _render_pass = nullptr;
};

static_assert(is_safe_base<CmdBufferRecorderBase>::value);

}

#endif // YAVE_COMMANDS_CMDBUFFERRECORDERBASE_H

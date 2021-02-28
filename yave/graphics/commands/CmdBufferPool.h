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
#ifndef YAVE_GRAPHICS_COMMANDS_CMDBUFFERPOOL_H
#define YAVE_GRAPHICS_COMMANDS_CMDBUFFERPOOL_H

#include <yave/yave.h>
#include <yave/graphics/device/DeviceLinked.h>
#include <yave/graphics/commands/CmdBufferData.h>

#include <yave/utils/traits.h>

#include <mutex>

namespace yave {

class CmdBufferPool : NonMovable, public DeviceLinked {

    public:
        CmdBufferPool(DevicePtr dptr);

        ~CmdBufferPool();

        VkCommandPool vk_pool() const;

        CmdBuffer create_buffer();

    private:
        friend class LifetimeManager;
        friend class CmdBuffer;

        void release(CmdBufferData* data);
        void recycle(CmdBufferData* data);


        static void prepare_for_recycling(CmdBufferData* data);

        CmdBufferData* alloc();

        void join_all();

    private:
        CmdBufferData* create_data();

        std::mutex _pool_lock;
        std::mutex _pending_lock;
        std::mutex _recycle_lock;

        VkHandle<VkCommandPool> _pool;

        core::Vector<std::unique_ptr<CmdBufferData>> _cmd_buffers;

        core::Vector<CmdBufferData*> _pending;
        core::Vector<CmdBufferData*> _recycled;

        core::Vector<VkFence> _fences;

        const u32 _thread_id;
};

static_assert(is_safe_base<CmdBufferPool>::value);

}

#endif // YAVE_GRAPHICS_COMMANDS_CMDBUFFERPOOL_H


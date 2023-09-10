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
#ifndef YAVE_GRAPHICS_COMMANDS_CMDQUEUE_H
#define YAVE_GRAPHICS_COMMANDS_CMDQUEUE_H

#include "CmdBufferRecorder.h"

#include <yave/graphics/swapchain/Swapchain.h>
#include <y/concurrent/Mutexed.h>

#include <memory>
#include <thread>

namespace yave {

class CmdQueue final : NonMovable {
    public:
        CmdQueue(u32 family_index, VkQueue queue);
        ~CmdQueue();

        u32 family_index() const;
        const Timeline& timeline() const;

        void wait();
        void clear_all_cmd_pools();

        CmdBufferPool& cmd_pool_for_thread();

        VkResult present(CmdBufferRecorder&& recorder, const FrameToken& token, const Swapchain::FrameSyncObjects& swaphain_sync);

    private:
        friend class CmdBufferRecorderBase;

        // Does not wait for the completion of previous commands before starting
        void submit_async_delayed_start(CmdBufferData* data);
        TimelineFence submit(CmdBufferData* data);

        TimelineFence submit_internal(CmdBufferData* data, VkSemaphore wait = {}, VkSemaphore signal = {}, VkFence fence = {});

        void clear_thread(u32 thread_id);

        concurrent::Mutexed<VkQueue> _queue = {};
        concurrent::Mutexed<core::Vector<CmdBufferData*>> _delayed_start;

        Timeline _timeline;

        concurrent::Mutexed<core::Vector<std::pair<u32, std::unique_ptr<CmdBufferPool>>>> _cmd_pools;

        const u32 _family_index = u32(-1);

        static concurrent::Mutexed<core::Vector<CmdQueue*>> _all_queues;
};

}

#endif // YAVE_GRAPHICS_COMMANDS_CMDQUEUE_H


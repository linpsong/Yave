/*******************************
Copyright (c) 2016-2020 Grégoire Angerand

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
#ifndef EDITOR_SYSTEMS_UISYSTEM_H
#define EDITOR_SYSTEMS_UISYSTEM_H

#include <editor/editor.h>

#include <yave/ecs/System.h>

#include <y/core/Chrono.h>

#include <memory>

namespace editor {

class UiWidgetBase;
class UiComponent;

class UiSystem : public ecs::System {
    public:
        UiSystem(ContextPtr ctx, MainWindow& window);
        ~UiSystem();

        void tick(ecs::EntityWorld& world) override;

    private:
        bool should_delete(const ecs::EntityWorld& world, const UiComponent* component) const;

        void paint_menu(ecs::EntityWorld& world);
        void paint_widgets(ecs::EntityWorld& world, CmdBufferRecorder& recorder);
        void paint_widget(ecs::EntityWorld& world, CmdBufferRecorder& recorder, UiWidgetBase* widget);

        MainWindow* _window = nullptr;
        std::unique_ptr<ImGuiRenderer> _renderer;

        core::Chrono _frame_timer;
};

}

#endif // EDITOR_SYSTEMS_UISYSTEM_H

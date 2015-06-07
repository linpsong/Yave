/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_SCREENSHADERRENDERER
#define N_GRAPHICS_SCREENSHADERRENDERER

#include "BufferedRenderer.h"
#include "ShaderCombinaison.h"

namespace n {
namespace graphics {

class ScreenShaderRenderer : public BufferedRenderer
{
	public:
		ScreenShaderRenderer(ShaderCombinaison *sh, BufferedRenderer *c = 0, const core::String &name = "tex", uint slt = 0, const math::Vec2ui &s = math::Vec2ui(0));

		virtual ~ScreenShaderRenderer() {
		}

		virtual void *prepare() override;
		virtual void render(void *ptr) override;

	private:
		BufferedRenderer *child;
		ShaderCombinaison *shader;
		uint slot;
		core::String uName;

		static const Material<float> &getMaterial();
};

}
}


#endif //  N_GRAPHICS_SCREENSHADERRENDERER


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

#ifndef N_GRAPHICS_CONTEXT_H
#define N_GRAPHICS_CONTEXT_H

#include <n/concurent/SynchronizedQueue.h>
#include <n/core/Functor.h>
#include <n/math/Matrix.h>

namespace n {
namespace graphics {

class ShaderCombinaison;

class GLContext
{
	public:
		static GLContext *getContext();

		void addGLTask(const core::Functor<void()> &f);

		void setProjectionMatrix(const math::Matrix4<> &m);
		void setViewMatrix(const math::Matrix4<> &m);
		void setModelMatrix(const math::Matrix4<> &m);

		bool processTasks();

		static bool checkGLError();

		const ShaderCombinaison *getShader() const {
			return shader;
		}

	private:
		friend class ShaderCombinaison;

		GLContext();
		~GLContext();

		concurent::SynchronizedQueue<core::Functor<void()>> tasks;

		math::Matrix4<> projection;
		math::Matrix4<> view;
		math::Matrix4<> model;

		ShaderCombinaison *shader;
};

}
}


#endif // N_GRAPHICS_CONTEXT_H

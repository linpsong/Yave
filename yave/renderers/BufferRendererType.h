/*******************************
Copyright (c) 2016-2017 Gr�goire Angerand

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
#ifndef YAVE_RENDERERS_BUFFERRENDERERTYPE_H
#define YAVE_RENDERERS_BUFFERRENDERERTYPE_H

#include <yave/yave.h>

#include <yave/images/ImageView.h>

namespace yave {

/*enum class BufferRendererType : u64 {
	Depth = 0x01,

	AlbedoMetallic = 0x02,
	NormalRoughness = 0x04,

	Lighting = 0x08,

	DepthVariance = 0x10
};

constexpr BufferRendererType operator|(BufferRendererType a, BufferRendererType b) {
	return BufferRendererType(uenum(a) | uenum(b));
}

constexpr BufferRendererType operator&(BufferRendererType a, BufferRendererType b) {
	return BufferRendererType(uenum(a) & uenum(b));
}*/

/*#define YAVE_CREATE_RENDERER_BASE(name, member)		\
struct name : NonCopyable {							\
	virtual ~name() {}								\
	virtual TextureView member() const = 0;			\
}*/

}

#endif // YAVE_RENDERERS_BUFFERRENDERERTYPE_H
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

#include "Animation.h"

namespace yave {

Animation::Animation(float duration, core::Vector<AnimationChannel>&& channels) : _duration(duration), _channels(std::move(channels)) {
}

const core::Vector<AnimationChannel>& Animation::channels() const {
	return _channels;
}

float Animation::duration() const {
	return _duration;
}

std::optional<math::Transform<>> Animation::bone_transform(const core::String& name, float time) const {
	auto channel = std::find_if(_channels.begin(), _channels.end(), [&](const auto& ch) { return ch.name() == name; });

	if(channel == _channels.end()) {
		return std::optional<math::Transform<>>();
	}

	return std::optional(channel->bone_transform(time));
}

}
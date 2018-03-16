/*******************************
Copyright (c) 2016-2018 Grégoire Angerand

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
#ifndef Y_CORE_RC_H
#define Y_CORE_RC_H

#include <y/utils.h>

namespace y {
namespace core {

namespace detail {

template<typename T>
class Ptr : NonCopyable {
	public:
		using value_type = std::remove_reference_t<T>;
		using pointer = std::remove_extent_t<T>*;
		using const_pointer = const std::remove_extent_t<T>*;

		T& operator*() const {
			return *_ptr;
		}

		pointer operator->() const {
			return _ptr;
		}

		operator std::conditional_t<std::is_const_v<T>, const void*, void*>() const {
			return _ptr;
		}

		bool operator<(const_pointer t) const {
			return _ptr < t;
		}

		bool operator>(const_pointer t) const {
			return _ptr > t;
		}

		bool operator!() const {
			return is_null();
		}

		bool is_null() const {
			return !_ptr;
		}

		pointer as_ptr() const {
			return _ptr;
		}

		template<typename = void>
		auto& operator[](usize i) const {
			static_assert(std::is_array_v<T>, "T is not an array");
			return _ptr[i];
		}

	protected:
		Ptr() = default;

		template<typename Y>
		explicit Ptr(Y*&& p) : _ptr(p) {
		}

		Owner<pointer> _ptr = nullptr;

		void destroy() {
			if constexpr(std::is_array_v<T>) {
				delete[] _ptr;
			} else {
				delete _ptr;
			}
		}
};

}

template<typename T>
class Unique : public detail::Ptr<T> {

	using Base = detail::Ptr<T>;
	using Base::_ptr;

	public:
		using pointer = typename Base::pointer;
		using const_pointer = typename Base::const_pointer;

		Unique() = default;

		Unique(pointer&& p) : Base(std::move(p)) {
		}

		template<typename Y>
		Unique(Y*&& p) : Base(std::move(p)) {
		}

		Unique(std::remove_const_t<T>&& p) : Unique(new T(std::move(p))) {
		}

		Unique(std::nullptr_t) {
		}

		Unique(Unique&& p) {
			swap(p);
		}

		template<typename Y>
		Unique(Unique<Y>&& p) {
			swap(p);
		}

		~Unique() {
			Base::destroy();
		}

		Unique& operator=(Unique&& p) {
			swap(p);
			return *this;
		}


		template<typename Y>
		Unique& operator=(Unique<Y>&& p) {
			swap(p);
			return *this;
		}

		template<typename Y>
		Unique& operator=(Y*&& p) {
			Unique ptr(std::move(p));
			swap(ptr);
			return *this;
		}

		Unique& operator=(std::nullptr_t p) {
			Unique ptr(p);
			swap(ptr);
			return *this;
		}

	private:
		template<typename Y>
		void swap(Unique<Y>& p) {
			std::swap(_ptr, p._ptr);
		}

};

template<typename T, typename C = u32>
class Rc : public detail::Ptr<T> {

	using Base = detail::Ptr<T>;
	using Base::_ptr;

	public:
		using pointer = typename Base::pointer;
		using const_pointer = typename Base::const_pointer;

		Rc() = default;

		Rc(std::nullptr_t) {
		}

		Rc(pointer&& p) : Base(std::move(p)), _count(new C(1)) {
		}

		template<typename Y>
		Rc(Y*&& p) : Base(std::move(p)), _count(new C(1)) {
		}

		Rc(std::remove_const_t<T>&& p) : Rc(new T(std::move(p))) {
		}

		Rc(const Rc& p) : Rc(p._ptr, p._count) {
		}

		template<typename Y>
		Rc(const Rc<Y, C>& p) : Rc(p._ptr, p._count) {
		}

		Rc(Rc&& p) {
			swap(p);
		}

		template<typename Y>
		Rc(Rc<Y, C>&& p) {
			swap(p);
		}

		~Rc() {
			unref();
		}

		template<typename Y>
		void swap(Rc<Y, C>& p) {
			std::swap(_ptr, p._ptr);
			std::swap(_count, p._count);
		}

		const C& ref_count() const {
			return *_count;
		}

		Rc& operator=(const Rc& p) {
			if(p._count != _count) {
				unref();
				ref(p);
			}
			return *this;
		}

		Rc& operator=(Rc&& p) {
			swap(p);
			return *this;
		}

		Rc& operator=(std::nullptr_t) {
			unref();
			return *this;
		}

		template<typename Y>
		Rc& operator=(const Rc<Y, C>& p) {
			if(p._count != _count) {
				unref();
				ref(p);
			}
			return *this;
		}

		template<typename Y>
		Rc& operator=(Y*&& p) {
			return operator=(Rc(p));
		}

		template<typename Y>
		Rc& operator=(Rc<Y, C>&& p) {
			swap(p);
			return *this;
		}

	private:
		template<typename U, typename D>
		friend class Rc;

		// only called by other Rc's so we take ownership as well
		Rc(T* p, C* c) : Base(std::move(p)), _count(c) {
			if(_count) {
				++(*_count);
			}
		}

		template<typename Y>
		void ref(const Rc<Y, C>& p) {
			if(p._count && (_count = p._count)) {
				++(*_count);
			}
			_ptr = p._ptr;
		}

		void unref() {
			if(_count && !--(*_count)) {
				Base::destroy();
				delete _count;
			}
			_ptr = nullptr;
			_count = nullptr;
		}

		C* _count = nullptr;
};


template<typename T>
Unique(T*&&) -> Unique<T>;

template<typename T>
Rc(T*&&) -> Rc<T>;

}
}

namespace std {
template<typename T>
struct hash<y::core::Unique<T>> : hash<T*> {
	auto operator()(const y::core::Unique<T>& p) const {
		return hash<T*>::operator()(p.as_ptr());
	}
};
template<typename T, typename C>
struct hash<y::core::Rc<T, C>> : hash<T*> {
	auto operator()(const y::core::Rc<T, C>& p) const {
		return hash<T*>::operator()(p.as_ptr());
	}
};
}


#endif // Y_CORE_RC_H

// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
	template <typename T, auto D> class unique_resource;
}

template <typename T, auto D>
class gm::unique_resource {
public:
	using value_type = T;
	using reference = T&;
	using const_reference = T const&;
	using rvalue_reference = T&&;

	unique_resource() = default;
	~unique_resource() { reset(); }

	explicit unique_resource(rvalue_reference obj) : _object(obj) {}

	unique_resource(unique_resource&& src) : _object(std::move(src.get())) {}
	inline unique_resource& operator=(unique_resource&& src);

	inline unique_resource& operator=(rvalue_reference obj);

	const_reference get() const { return _object; }
	reference get() { return _object; }

	inline void reset(rvalue_reference obj);
	inline void reset();

private:
	T _object = {};
};

template <typename T, auto D>
gm::unique_resource<T, D>& gm::unique_resource<T, D>::operator=(unique_resource&& src) {
	if (this != &src) {
		reset(std::move(src.get()));
	}
	return *this;
}

template <typename T, auto D>
gm::unique_resource<T, D>& gm::unique_resource<T, D>::operator=(rvalue_reference obj) {
	if (std::addressof(obj) != std::addressof(_object)) {
		D(_object);
		_object = std::move(obj);
	}
	return *this;
}

template <typename T, auto D>
void gm::unique_resource<T, D>::reset(rvalue_reference obj) {
	D(_object);
	_object = std::forward<T>(obj);
}

template <typename T, auto D>
void gm::unique_resource<T, D>::reset() {
	D(_object);
	_object = T{};
}
// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {

template <typename T>
class com_ptr
{
public:
	using pointer = T*;
	using reference = T&;

	com_ptr() = default;
	explicit com_ptr(pointer ptr) : _ptr(ptr) { _addref(); }
	com_ptr(std::nullptr_t) {}
	~com_ptr() { _release(); }

	com_ptr(com_ptr const& rhs) : _ptr(rhs._ptr) { _addref(); }
	com_ptr(com_ptr&& rhs) : _ptr(rhs._ptr) { rhs._ptr = nullptr; }

	com_ptr& operator=(com_ptr const& rhs) { if (this != std::addressof(rhs)) { _release(); _ptr = rhs._ptr; _addref(); } return *this; }
	com_ptr& operator=(com_ptr&& rhs) { if (this != std::addressof(rhs)) { _release(); _ptr = rhs._ptr; rhs._ptr = nullptr;; } return *this; }
	com_ptr& operator=(std::nullptr_t) { _release(); _ptr = nullptr; return *this; }

	explicit operator bool() const { return _ptr != nullptr; }
	bool empty() const { return _ptr == nullptr; }

	void reset(pointer ptr) { _release(); _ptr = ptr; _addref(); }

	pointer release() { pointer tmp = _ptr; _ptr = nullptr; return tmp; }
	void attach(pointer ptr) { _release(); _ptr = ptr; }

	pointer get() const { return _ptr; }
	pointer operator->() const { return _ptr; }
	reference operator*() const { return _ptr; }

	friend bool operator==(com_ptr const& lhs, com_ptr const& rhs) { return lhs.get() == rhs.get(); }
	friend bool operator!=(com_ptr const& lhs, com_ptr const& rhs) { return lhs.get() != rhs.get(); }
	friend bool operator==(com_ptr const& lhs, std::nullptr_t) { return lhs.get() == nullptr; }
	friend bool operator!=(com_ptr const& lhs, std::nullptr_t) { return lhs.get() != nullptr; }
	friend bool operator==(std::nullptr_t, com_ptr const& rhs) { return nullptr == rhs.get(); }
	friend bool operator!=(std::nullptr_t, com_ptr const& rhs) { return nullptr != rhs.get(); }

private:
	void _addref() { if (_ptr != nullptr) _ptr->AddRef(); }
	void _release() { if (_ptr != nullptr) _ptr->Release(); }

	pointer _ptr = nullptr;
};

} // namespace gm

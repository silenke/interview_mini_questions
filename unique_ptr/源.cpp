#include <utility>

template <typename T, typename U>
using enable_if_convertible_t = std::enable_if_t<std::is_convertible_v<T*, U*>, int>;

template <typename T>
class unique_ptr {
public:
	unique_ptr(const unique_ptr&) = delete;
	unique_ptr& operator=(const unique_ptr&) = delete;

	constexpr unique_ptr() noexcept = default;
	constexpr unique_ptr(nullptr_t) noexcept : unique_ptr() {}

	//explicit unique_ptr(T* ptr) noexcept : ptr_{ ptr } {}

	template <typename U, enable_if_convertible_t<U, T> = 0>
	explicit unique_ptr(U* ptr) noexcept : ptr_{ ptr } {}

	//unique_ptr(unique_ptr&& rhs) noexcept : ptr_{ rhs.release() } {}

	template <typename U, enable_if_convertible_t<U, T> = 0>
	unique_ptr(unique_ptr<U>&& rhs) noexcept : ptr_{ rhs.release() } {}

	~unique_ptr() noexcept {
		if (ptr_) delete ptr_;
	}

	//unique_ptr& operator=(unique_ptr&& rhs) noexcept {
	//	reset(rhs.release());
	//	return *this;
	//}

	template <typename U, enable_if_convertible_t<U, T> = 0>
	unique_ptr& operator=(unique_ptr<U>&& rhs) noexcept {
		reset(rhs.release());
		return *this;
	}

	unique_ptr& operator=(nullptr_t) noexcept {
		reset();
		return *this;
	}

	T* get() const noexcept { return ptr_; }
	explicit operator bool() const noexcept {
		return static_cast<bool>(ptr_);
	}
	T* release() noexcept {
		return std::exchange(ptr_, nullptr);
	}
	void reset(T* ptr = nullptr) noexcept {
		auto old = std::exchange(ptr_, ptr);
		if (old) delete old;
	}
	void swap(unique_ptr& rhs) noexcept {
		std::swap(ptr_, rhs.ptr_);
	}

	T& operator*() const { return *ptr_; }
	T* operator->() const noexcept { return ptr_; }

private:
	T* ptr_{ nullptr };
};

template <typename T, typename... Args>
auto make_unique(Args&&... args) {
	return unique_ptr<T>{ new T(std::forward<Args>(args)...) };
}

#include <string>
#include <iostream>
int main() {

	auto p = make_unique<std::string>("123");
	std::cout << *p;
}

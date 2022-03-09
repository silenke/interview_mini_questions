#include <utility>

class ref_count {
public:
	int use_count() const noexcept { return count_; }
	void inc_ref() noexcept { ++count_; }
	int dec_ref() noexcept { return --count_; }

private:
	int count_{ 1 };
};

template <class T>
class shared_ptr {
public:
	constexpr shared_ptr() noexcept = default;

	constexpr shared_ptr(nullptr_t) noexcept : shared_ptr{} {}

	explicit shared_ptr(T* ptr) : ptr_{ ptr } {
		if (ptr) {
			rep_ = new ref_count{};
		}
	}

	shared_ptr(const shared_ptr& rhs) noexcept : ptr_{ rhs.ptr_ }, rep_{ rep_ } {
		if (rep_) {
			rep_->inc_ref();
		}
	}

	shared_ptr(shared_ptr&& rhs) noexcept : ptr_{ rhs.ptr_ }, rep_{ rep_ } {
		rhs.ptr_ = nullptr;
		rhs.rep_ = nullptr;
	}

	~shared_ptr() noexcept {
		if (rep_ && rep_->dec_ref()) {
			delete ptr_;
			delete rep_;
		}
	}

	shared_ptr& operator=(const shared_ptr& rhs) {
		shared_ptr{ rhs }.swap(*this);
		return *this;
	}

	shared_ptr& operator=(shared_ptr&& rhs) {
		shared_ptr{ std::move(rhs) }.swap(*this);
		return *this;
	}

	void swap(shared_ptr& rhs) noexcept {
		std::swap(ptr_, rhs.ptr_);
		std::swap(rep_, rhs.rep_);
	}

	void reset() noexcept {
		shared_ptr{}.swap(*this);
	}

	void reset(nullptr_t) noexcept {
		reset();
	}

	void reset(T* ptr) {
		shared_ptr{ ptr }.swap(*this);
	}

	T* get() const noexcept { return ptr_; }
	T& operator*() const noexcept { return *ptr_; }
	T* operator->() const noexcept { return ptr_; }

	int use_count() const noexcept { return rep_ ? rep_->use_count() : 0; }
	bool unique() const noexcept { return use_count() == 1; }
	explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

private:
	T* ptr_{ nullptr };
	ref_count* rep_{ nullptr };
};

template <class T, class... Args>
auto make_shared(Args&&... args) {
	return shared_ptr<T>{ new T(std::forward<Args>(args)...) };
}

#include <string>
#include <iostream>
int main() {

	auto p = make_shared<std::string>("123");
	auto p2 = p;
	std::cout << *p << *p2;
}

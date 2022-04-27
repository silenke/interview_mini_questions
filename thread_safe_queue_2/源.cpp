#include <boost/circular_buffer.hpp>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <optional>

template <class T>
class Queue {	// 有界队列
public:
	Queue(size_t capacity) : q_{capacity} {}

	template <class T>
	void push(T&& val) {	// 阻塞
		
		std::unique_lock lk{ m_ };
		not_full_.wait(lk, [this] { return !q_.full(); });

		assert(!q_.full());

		q_.push_back(std::forward<T>(val));
		not_empty_.notify_one();
	}

	T pop() {	// 阻塞

		std::unique_lock lk{ m_ };
		not_empty_.wait(lk, [this] { return !q_.empty(); });

		assert(!q_.empty());

		T ret{ std::move_if_noexcept(q_.front()) };
		q_.pop_front();
		not_full_.notify_one();
		return ret;
	}
	
	template <class T>
	bool try_push(T&& val) {	// 非阻塞

		std::lock_guard lk{ m_ };
		if (q_.full()) return false;

		q_.push_back(std::forward<T>(val));
		not_empty_.notify_one();
		return true;
	}

	std::optional<T> try_pop() {	// 非阻塞

		std::lock_guard lk{ m_ };
		if (q_.empty()) return {};
		
		std::optional<T> ret{ std::move_if_noexcept(q_.front()) };
		q_.pop_front();
		not_full_.notify_one();
		return ret;
	}

private:
	boost::circular_buffer<T> q_;
	std::mutex m_;
	std::condition_variable not_full_;
	std::condition_variable not_empty_;
};

#include <thread>
#include <iostream>
int main() {

	Queue<int> q{ 10 };
	std::thread t1(
		[&] {
			//using namespace std::literals;
			//std::this_thread::sleep_for(1s);
			for (int i = 0; i < 100; ) {
				if (q.try_push(i)) {
					i++;
				}
			}
		}
	);
	std::thread t2(
		[&] {
			for (int i = 0; i < 100; ) {
				if (auto ret = q.try_pop()) {
					std::cout << *ret << " ";
					i++;
				}
			}
		}
	);
	t1.join();
	t2.join();
}

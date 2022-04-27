#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <optional>

template <class T>
class Queue {	// 无界队列
public:
	//Queue(const Queue&) = delete;
	//Queue& operator=(Queue&) = delete;

	void push(const T& val) {
		emplace(val);
	}

	void push(T&& val) {
		emplace(std::move(val));
	}

	template <class... Args>
	void emplace(Args&&... args) {

		std::lock_guard lk{ m_ };
		q_.emplace(std::forward<Args>(args)...);
		cv_.notify_one();
	}

	T pop() {

		std::unique_lock lk{ m_ };
		cv_.wait(lk, [this] { return !q_.empty(); });

		assert(!q_.empty());

		T ret{ std::move_if_noexcept(q_.front()) };
		q_.pop();
		return ret;
	}

	std::optional<T> try_pop() {

		std::lock_guard lk{ m_ };
		if (q_.empty()) return {};

		std::optional<T> ret{ std::move_if_noexcept(q_.front()) };
		q_.pop();
		return ret;
	}

private:
	std::queue<T> q_;
	std::mutex m_;
	std::condition_variable cv_;
};

#include <thread>
#include <iostream>
int main() {

	Queue<int> q;
	std::thread t1(
		[&] {
			//using namespace std::literals;
			//std::this_thread::sleep_for(1s);
			for (int i = 0; i < 100; i++) {
				q.push(i);
			}
		}
	);
	std::thread t2(
		[&] {
			for (int i = 0; i < 100; i++) {
				//std::cout << q.pop() << " ";
				if (auto ret = q.try_pop()) {
					std::cout << *ret << " ";
				}
			}
		}
	);
	t1.join();
	t2.join();
}

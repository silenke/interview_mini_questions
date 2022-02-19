#include <vector>
#include <thread>
#include <boost/circular_buffer.hpp>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class thread_pool {
public:
	explicit thread_pool(size_t n) : q_{ n }, running_{ false } {}

	~thread_pool() { stop(); }

	void start(size_t n) {

		if (running_) return;

		running_ = true;
		threads_.reserve(n);
		while (n--) {
			threads_.emplace_back(&thread_pool::worker, this);
		}
	}

	void stop() {

		if (!running_) return;

		{
			std::lock_guard lk{ m_ };
			running_ = false;
			not_full_.notify_all();
			not_empty_.notify_all();
		}

		for (auto& t : threads_) {
			if (t.joinable()) t.join();
		}
	}

	template <class Fun>
	void submit(Fun f) {

		std::unique_lock lk{ m_ };
		not_full_.wait(lk, [this] { return !running_ || !q_.full(); });

		assert(!running_ || !q_.full());

		if (!running_) return;

		q_.push_back(std::move(f));
		not_empty_.notify_one();
	}

private:
	void worker() {

		while (true) {

			task t;
			{
				std::unique_lock lk{ m_ };
				not_empty_.wait(lk, [this] { return !running_ || !q_.empty(); });

				assert(!running_ || !q_.empty());

				if (!running_) return;

				t = std::move(q_.front());
				q_.pop_front();
				not_full_.notify_one();
			}
			t();
		}
	}

	using task = std::function<void()>;

	std::vector<std::thread> threads_;
	boost::circular_buffer<task> q_;
	std::mutex m_;
	std::condition_variable not_full_;
	std::condition_variable not_empty_;
	bool running_;
};

#include <iostream>
int main() {

	thread_pool pool{ 10 };
	pool.start(3);
	for (int i = 0; i < 100; i++) {
		pool.submit(
			[i] {
				std::cout << i << " " << std::this_thread::get_id() << "\n";
			}
		);
	}
}

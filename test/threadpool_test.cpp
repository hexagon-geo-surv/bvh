#include <bvh/v2/bvh.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/thread_pool.h>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
  std::cout << "threadpool_test" << std::endl;

  bvh::v2::ThreadPool threadpool;
  std::vector<std::thread> threads;

  size_t num_threads = 200;
  size_t num_tasks = 10;
  size_t num_millis = 45;

  auto estimated_time = std::chrono::milliseconds(
      num_tasks * num_millis * num_threads / threadpool.get_thread_count());

  std::cout << "total estimated time:"
            << std::chrono::duration_cast<std::chrono::seconds>(estimated_time)
            << std::endl;

  // monitoring thread, simply print time so that is is obvious so when that we
  // can
  threads.emplace_back([&]() {
    auto delta = std::max(std::chrono::milliseconds(1), estimated_time / 10);
    auto elasped = std::chrono::milliseconds(0);
    auto start = std::chrono::system_clock::now();

    while (std::chrono::system_clock::now() < start + estimated_time) {
      std::this_thread::sleep_for(delta);
      elasped += delta;
      std::cout << "[estimated remaining time: " << estimated_time - elasped
                << "]" << std::endl;
    }

    std::cout << "[test should be done by now]" << std::endl;
  });

  // submit lots of task concurrently
  for (size_t i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
      bvh::v2::ThreadPool::TaskGroup tgroup;
      for (int j = 0; j < num_tasks; ++j) {
        threadpool.push(tgroup, [&](size_t) {
          std::this_thread::sleep_for(std::chrono::milliseconds(num_millis));
        });
      }

      threadpool.wait(tgroup);
    });
  }

  std::cout << "wait for " << threads.size() << " threads" << std::endl;
  for (auto &th : threads) {
    if (th.joinable()) {
      th.join();
    }
  }

  threadpool.wait_all();
  std::cout << "done" << std::endl;
  return 0;
}
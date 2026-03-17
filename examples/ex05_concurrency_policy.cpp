#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 5: Use atomic concurrency policy and verify concurrent consistency.
  using atomic_t =
      primitive<int, policy::value::checked, policy::concurrency::atomic,
                policy::error::expected>;

  auto const lhs = atomic_t{12};
  auto const rhs = atomic_t{30};

  std::atomic<int> mismatch_count{0};
  std::vector<std::thread> workers;
  workers.reserve(4);

  // Run many concurrent dispatches. Any error or wrong value is counted.
  for (int i = 0; i < 4; ++i) {
    workers.emplace_back([&]() {
      for (int n = 0; n < 10000; ++n) {
        auto const out = operations::add(lhs, rhs);
        if (!out.has_value() || out->value() != 42) {
          mismatch_count.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  for (auto &worker : workers) {
    worker.join();
  }

  // A non-zero mismatch count indicates unexpected behavior under concurrency.
  if (mismatch_count.load(std::memory_order_relaxed) != 0) {
    std::cerr << "atomic policy path mismatch\n";
    return 1;
  }

  std::cout << "concurrency_policy demo passed\n";
  return 0;
}

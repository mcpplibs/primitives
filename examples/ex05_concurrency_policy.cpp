/*
 * Example: ex05_concurrency_policy
 *
 * Purpose:
 * Demonstrate a more representative read/write mixed workload under fenced
 * concurrency policy.
 *
 * Expected results:
 * - Writer threads continuously update shared operands via store().
 * - Reader threads concurrently run add/sub dispatch and write snapshots.
 * - No dispatch error and no range violation are observed.
 * - Primitive load/store/CAS APIs all work under fenced policy.
 * - Program prints a success message and exits with code 0.
 */

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 5: Use fenced concurrency policy in a read/write mixed scenario.
  using fenced_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  constexpr int kWriterThreads = 4;
  constexpr int kReaderThreads = 6;
  constexpr int kIterationsPerThread = 20000;
  constexpr int kMaxOperand = 100000;

  auto lhs = fenced_t{0};
  auto rhs = fenced_t{0};
  auto sink = fenced_t{0};

  std::atomic<int> add_error_count{0};
  std::atomic<int> sub_error_count{0};
  std::atomic<int> range_violation_count{0};
  std::atomic<bool> start{false};

  std::vector<std::thread> workers;
  workers.reserve(kWriterThreads + kReaderThreads);

  // Writers continuously publish operand updates.
  for (int writer = 0; writer < kWriterThreads; ++writer) {
    workers.emplace_back([&, writer]() {
      while (!start.load(std::memory_order_acquire)) {
      }

      for (int n = 0; n < kIterationsPerThread; ++n) {
        auto const next_lhs = (writer + n) % (kMaxOperand + 1);
        auto const next_rhs = (writer * 3 + n * 7) % (kMaxOperand + 1);
        lhs.store(next_lhs);
        rhs.store(next_rhs);
      }
    });
  }

  // Readers mix load + dispatch + store paths.
  for (int reader = 0; reader < kReaderThreads; ++reader) {
    workers.emplace_back([&, reader]() {
      while (!start.load(std::memory_order_acquire)) {
      }

      for (int n = 0; n < kIterationsPerThread; ++n) {
        if (((reader + n) & 1) == 0) {
          auto const out = operations::add(lhs, rhs);
          if (!out.has_value()) {
            add_error_count.fetch_add(1, std::memory_order_relaxed);
            continue;
          }

          auto const value = out->load();
          if (value < 0 || value > (kMaxOperand * 2)) {
            range_violation_count.fetch_add(1, std::memory_order_relaxed);
          }

          sink.store(value);
          continue;
        }

        auto const out = operations::sub(lhs, rhs);
        if (!out.has_value()) {
          sub_error_count.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        auto const value = out->load();
        if (value < -kMaxOperand || value > kMaxOperand) {
          range_violation_count.fetch_add(1, std::memory_order_relaxed);
        }

        sink.store(value);
      }
    });
  }

  start.store(true, std::memory_order_release);

  for (auto &worker : workers) {
    worker.join();
  }

  auto expected = sink.load();
  auto const cas_ok = sink.compare_exchange(expected, expected + 1);
  if (!cas_ok || sink.load() != expected + 1) {
    std::cerr << "fenced CAS failed after mixed workload\n";
    return 1;
  }

  if (add_error_count.load(std::memory_order_relaxed) != 0 ||
      sub_error_count.load(std::memory_order_relaxed) != 0 ||
      range_violation_count.load(std::memory_order_relaxed) != 0) {
    std::cerr << "concurrency workload mismatch: add_err="
              << add_error_count.load(std::memory_order_relaxed)
              << ", sub_err="
              << sub_error_count.load(std::memory_order_relaxed)
              << ", range_violation="
              << range_violation_count.load(std::memory_order_relaxed)
              << '\n';
    return 1;
  }

  std::cout << "concurrency_policy demo passed, final sink=" << sink.load()
            << '\n';
  return 0;
}

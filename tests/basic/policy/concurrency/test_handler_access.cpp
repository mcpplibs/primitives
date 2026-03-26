#include <atomic>
#include <cstdint>
#include <gtest/gtest.h>

import mcpplibs.primitives.policy;
import mcpplibs.primitives.underlying;
import mcpplibs.primitives.operations.impl;

using namespace mcpplibs::primitives;

namespace {

struct NonTriviallyCopyableRep {
  int value{0};
  ~NonTriviallyCopyableRep() {}
};

struct LowAlignmentRep {
  unsigned char bytes[sizeof(std::uint64_t)]{};
};

} // namespace

TEST(PolicyConcurrencyTest, FencedInjectsFences) {
  using fenced_handler =
      policy::concurrency::handler<policy::concurrency::fenced,
                                   operations::Addition, int,
                                   policy::error::kind>;
  using single_handler = policy::concurrency::handler<policy::concurrency::none,
                                                      operations::Addition, int,
                                                      policy::error::kind>;

  auto const fenced_injection = fenced_handler::inject();
  auto const single_injection = single_handler::inject();

  EXPECT_TRUE(fenced_injection.fence_before);
  EXPECT_TRUE(fenced_injection.fence_after);
  EXPECT_EQ(fenced_injection.order_before, std::memory_order_seq_cst);
  EXPECT_EQ(fenced_injection.order_after, std::memory_order_seq_cst);
  EXPECT_FALSE(single_injection.fence_before);
  EXPECT_FALSE(single_injection.fence_after);
}

TEST(PolicyConcurrencyTest, FencedVariantsUseExpectedMemoryOrders) {
  using relaxed_handler =
      policy::concurrency::handler<policy::concurrency::fenced_relaxed,
                                   operations::Addition, int,
                                   policy::error::kind>;
  using acq_rel_handler =
      policy::concurrency::handler<policy::concurrency::fenced_acq_rel,
                                   operations::Addition, int,
                                   policy::error::kind>;

  auto const relaxed = relaxed_handler::inject();
  auto const acq_rel = acq_rel_handler::inject();

  EXPECT_EQ(relaxed.order_before, std::memory_order_relaxed);
  EXPECT_EQ(relaxed.order_after, std::memory_order_relaxed);
  EXPECT_EQ(acq_rel.order_before, std::memory_order_acquire);
  EXPECT_EQ(acq_rel.order_after, std::memory_order_release);
}

TEST(PolicyConcurrencyTest, PrimitiveAccessHandlerProtocolByPolicy) {
  EXPECT_TRUE(
      (policy::concurrency::handler_access_available<policy::concurrency::none,
                                                     int>));
  EXPECT_TRUE((policy::concurrency::handler_access_available<
               policy::concurrency::fenced, int>));
  EXPECT_TRUE((policy::concurrency::handler_access_available<
               policy::concurrency::fenced_relaxed, int>));
  EXPECT_TRUE((policy::concurrency::handler_access_available<
               policy::concurrency::fenced_acq_rel, int>));
  EXPECT_TRUE((policy::concurrency::handler_access_available<
               policy::concurrency::fenced_seq_cst, int>));
}

TEST(PolicyConcurrencyTest, PrimitiveAccessRejectsNonTriviallyCopyableRep) {
  EXPECT_FALSE((policy::concurrency::handler_access_available<
                policy::concurrency::fenced, NonTriviallyCopyableRep>));
  EXPECT_FALSE((policy::concurrency::handler_access_available<
                policy::concurrency::fenced_relaxed, NonTriviallyCopyableRep>));
  EXPECT_FALSE((policy::concurrency::handler_access_available<
                policy::concurrency::fenced_acq_rel, NonTriviallyCopyableRep>));
  EXPECT_FALSE((policy::concurrency::handler_access_available<
                policy::concurrency::fenced_seq_cst, NonTriviallyCopyableRep>));
}

TEST(PolicyConcurrencyTest, PrimitiveAccessRespectsAtomicRefAlignmentGate) {
  EXPECT_FALSE((policy::concurrency::handler_access_available<
                policy::concurrency::none, LowAlignmentRep>));

  constexpr bool requires_stronger_alignment =
      std::atomic_ref<LowAlignmentRep>::required_alignment >
      alignof(LowAlignmentRep);

  if constexpr (requires_stronger_alignment) {
    EXPECT_FALSE((policy::concurrency::handler_access_available<
                  policy::concurrency::fenced, LowAlignmentRep>));
    EXPECT_FALSE((policy::concurrency::handler_access_available<
                  policy::concurrency::fenced_relaxed, LowAlignmentRep>));
    EXPECT_FALSE((policy::concurrency::handler_access_available<
                  policy::concurrency::fenced_acq_rel, LowAlignmentRep>));
    EXPECT_FALSE((policy::concurrency::handler_access_available<
                  policy::concurrency::fenced_seq_cst, LowAlignmentRep>));
  } else {
    GTEST_SKIP() << "platform atomic_ref required_alignment does not exceed "
                    "alignof(LowAlignmentRep)";
  }
}

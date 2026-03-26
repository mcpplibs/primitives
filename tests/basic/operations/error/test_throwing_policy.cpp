#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.operations.operators;

using namespace mcpplibs::primitives;

TEST(OperationsTest, ThrowErrorPolicyThrowsException) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::throwing>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  EXPECT_THROW((void)operations::div(lhs, rhs), std::runtime_error);
}

TEST(OperationsTest, ThrowErrorPolicyExceptionHasReasonMessage) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::throwing>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  try {
    (void)operations::div(lhs, rhs);
    FAIL() << "Expected std::runtime_error to be thrown";
  } catch (std::runtime_error const &e) {
    EXPECT_NE(std::string(e.what()).find("division by zero"),
              std::string::npos);
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}

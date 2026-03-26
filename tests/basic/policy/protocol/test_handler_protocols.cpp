#include <gtest/gtest.h>

import mcpplibs.primitives.policy;
import mcpplibs.primitives.operations.impl;

using namespace mcpplibs::primitives;

TEST(PolicyProtocolTest, BuiltinHandlersSatisfyProtocolConcepts) {
  static_assert(policy::type::handler_protocol<policy::type::strict,
                                               operations::Addition, int, int>);
  static_assert(policy::concurrency::handler_protocol<policy::concurrency::none,
                                                      operations::Addition, int,
                                                      policy::error::kind>);
  static_assert(policy::value::handler_protocol<policy::value::checked,
                                                operations::Addition, int,
                                                policy::error::kind>);
  static_assert(policy::error::handler_protocol<policy::error::expected,
                                                operations::Addition, int,
                                                policy::error::kind>);

  EXPECT_TRUE((policy::type::handler_protocol<policy::type::strict,
                                              operations::Addition, int, int>));
  EXPECT_TRUE((policy::concurrency::handler_protocol<policy::concurrency::none,
                                                     operations::Addition, int,
                                                     policy::error::kind>));
  EXPECT_TRUE((policy::value::handler_protocol<policy::value::checked,
                                               operations::Addition, int,
                                               policy::error::kind>));
  EXPECT_TRUE((policy::error::handler_protocol<policy::error::expected,
                                               operations::Addition, int,
                                               policy::error::kind>));
}

#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

namespace {

struct SignedBox {
  long long value;
};

struct UnsignedBox {
  std::uint16_t value;
};

struct FloatBox {
  double value;
};

} // namespace

template <> struct underlying::traits<SignedBox> {
  using value_type = SignedBox;
  using rep_type = long long;

  static constexpr bool enabled = true;
  static constexpr auto kind = underlying::category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return SignedBox{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <> struct underlying::traits<UnsignedBox> {
  using value_type = UnsignedBox;
  using rep_type = std::uint16_t;

  static constexpr bool enabled = true;
  static constexpr auto kind = underlying::category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return UnsignedBox{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <> struct underlying::traits<FloatBox> {
  using value_type = FloatBox;
  using rep_type = double;

  static constexpr bool enabled = true;
  static constexpr auto kind = underlying::category::floating;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return FloatBox{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

TEST(ConversionRiskTest, NumericRiskDetectsOverflowAndUnderflow) {
  auto const overflow = conversion::numeric_risk<std::uint16_t>(70000);
  ASSERT_TRUE(overflow.has_value());
  EXPECT_EQ(*overflow, conversion::risk::kind::overflow);

  auto const underflow = conversion::numeric_risk<std::uint16_t>(-1);
  ASSERT_TRUE(underflow.has_value());
  EXPECT_EQ(*underflow, conversion::risk::kind::underflow);
}

TEST(ConversionRiskTest, NumericRiskDetectsDomainAndPrecisionLoss) {
  auto const domain =
      conversion::numeric_risk<int>(std::numeric_limits<double>::quiet_NaN());
  ASSERT_TRUE(domain.has_value());
  EXPECT_EQ(*domain, conversion::risk::kind::domain_error);

  auto const precision =
      conversion::numeric_risk<float>(std::numeric_limits<std::int64_t>::max());
  ASSERT_TRUE(precision.has_value());
  EXPECT_EQ(*precision, conversion::risk::kind::precision_loss);
}

TEST(ConversionCastTest, CheckedCastReportsErrorForInvalidInput) {
  auto const ok = conversion::checked_cast<int>(42u);
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(*ok, 42);

  auto const bad = conversion::checked_cast<std::uint16_t>(-7);
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}

TEST(ConversionCastTest, SaturatingCastClampsAndHandlesNaN) {
  EXPECT_EQ(conversion::saturating_cast<std::int16_t>(100000),
            std::numeric_limits<std::int16_t>::max());
  EXPECT_EQ(conversion::saturating_cast<std::int16_t>(-100000),
            std::numeric_limits<std::int16_t>::lowest());
  EXPECT_EQ(conversion::saturating_cast<int>(
                std::numeric_limits<double>::quiet_NaN()),
            0);
}

TEST(ConversionCastTest, TruncatingCastHandlesFloatingInputs) {
  EXPECT_EQ(conversion::truncating_cast<int>(42.9), 42);
  EXPECT_EQ(conversion::truncating_cast<int>(
                std::numeric_limits<double>::infinity()),
            std::numeric_limits<int>::max());
  EXPECT_EQ(conversion::truncating_cast<int>(
                -std::numeric_limits<double>::infinity()),
            std::numeric_limits<int>::lowest());
}

TEST(ConversionCastTest, ExactCastRejectsPrecisionLoss) {
  auto const ok = conversion::exact_cast<int>(42);
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(*ok, 42);

  auto const precision =
      conversion::exact_cast<float>(std::numeric_limits<std::int64_t>::max());
  ASSERT_FALSE(precision.has_value());
  EXPECT_EQ(precision.error(), conversion::risk::kind::precision_loss);
}

TEST(ConversionUnderlyingTest, CheckedCastUsesRepBridge) {
  auto const ok = conversion::checked_cast<UnsignedBox>(SignedBox{42});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->value, 42);

  auto const bad = conversion::checked_cast<UnsignedBox>(SignedBox{-1});
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}

TEST(ConversionUnderlyingTest, SaturatingCastClampsByUnderlyingRep) {
  auto const saturated = conversion::saturating_cast<UnsignedBox>(SignedBox{
      std::numeric_limits<long long>::max()});
  EXPECT_EQ(saturated.value, std::numeric_limits<std::uint16_t>::max());
}

TEST(ConversionUnderlyingTest, TruncatingAndExactCastSupportFloatingSources) {
  auto const truncated = conversion::truncating_cast<SignedBox>(FloatBox{12.75});
  EXPECT_EQ(truncated.value, 12);

  auto const exact = conversion::exact_cast<SignedBox>(FloatBox{12.0});
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->value, 12);
}

TEST(ConversionTypeTest, ResultTypesMatchContracts) {
  static_assert(std::same_as<decltype(conversion::checked_cast<int>(1)),
                             conversion::cast_result<int>>);
  static_assert(std::same_as<decltype(conversion::exact_cast<UnsignedBox>(
                                 SignedBox{1})),
                             conversion::cast_result<UnsignedBox>>);
}

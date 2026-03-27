#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives.literals;

using namespace mcpplibs::primitives::literals;

namespace {

template <auto Value> struct literal_value_tag;

template <typename Probe>
concept literal_available = requires { typename literal_value_tag<Probe::value()>; };

struct U8OverflowProbe {
  static consteval auto value() { return 256_u8; }
};

struct I8OverflowProbe {
  static consteval auto value() { return 128_i8; }
};

struct F32ExactPrecisionLossProbe {
  static consteval auto value() { return 16777217_f32e; }
};

struct F64ExactPrecisionLossProbe {
  static consteval auto value() { return 9007199254740993_f64e; }
};

struct F32OverflowProbe {
  static consteval auto value() { return 1.0e39_f32; }
};

struct F32ExactOverflowProbe {
  static consteval auto value() { return 1.0e39_f32e; }
};

struct F32ExactUnderflowProbe {
  static consteval auto value() { return 1.0e-50_f32e; }
};

static_assert(!literal_available<U8OverflowProbe>);
static_assert(!literal_available<I8OverflowProbe>);
static_assert(!literal_available<F32ExactPrecisionLossProbe>);
static_assert(!literal_available<F64ExactPrecisionLossProbe>);
static_assert(!literal_available<F32OverflowProbe>);
static_assert(!literal_available<F32ExactOverflowProbe>);
static_assert(!literal_available<F32ExactUnderflowProbe>);

} // namespace

TEST(LiteralsTest, IntegerLiteralsReturnExpectedUnderlyingTypes) {
  static_assert(std::same_as<decltype(42_u8), std::uint8_t>);
  static_assert(std::same_as<decltype(42_u16), std::uint16_t>);
  static_assert(std::same_as<decltype(42_u32), std::uint32_t>);
  static_assert(std::same_as<decltype(42_u64), std::uint64_t>);
  static_assert(std::same_as<decltype(42_size), std::size_t>);
  static_assert(std::same_as<decltype(42_diff), std::ptrdiff_t>);
  static_assert(std::same_as<decltype(42_i8), std::int8_t>);
  static_assert(std::same_as<decltype(42_i16), std::int16_t>);
  static_assert(std::same_as<decltype(42_i32), std::int32_t>);
  static_assert(std::same_as<decltype(42_i64), std::int64_t>);

  EXPECT_EQ(42_u8, static_cast<std::uint8_t>(42));
  EXPECT_EQ(42_u16, static_cast<std::uint16_t>(42));
  EXPECT_EQ(42_u32, static_cast<std::uint32_t>(42));
  EXPECT_EQ(42_u64, static_cast<std::uint64_t>(42));
  EXPECT_EQ(42_size, static_cast<std::size_t>(42));
  EXPECT_EQ(42_diff, static_cast<std::ptrdiff_t>(42));
  EXPECT_EQ(42_i8, static_cast<std::int8_t>(42));
  EXPECT_EQ(42_i16, static_cast<std::int16_t>(42));
  EXPECT_EQ(42_i32, static_cast<std::int32_t>(42));
  EXPECT_EQ(42_i64, static_cast<std::int64_t>(42));
}

TEST(LiteralsTest, FloatingLiteralsReturnExpectedUnderlyingTypes) {
  static_assert(std::same_as<decltype(1.25_f32), float>);
  static_assert(std::same_as<decltype(1.25_f32e), float>);
  static_assert(std::same_as<decltype(1.25_f64), double>);
  static_assert(std::same_as<decltype(1.25_f64e), double>);
  static_assert(std::same_as<decltype(1.25_f80), long double>);
  static_assert(std::same_as<decltype(1.25_f80e), long double>);
  static_assert(std::same_as<decltype(16777216_f32), float>);
  static_assert(std::same_as<decltype(16777216_f32e), float>);
  static_assert(std::same_as<decltype(16777217_f32), float>);
  static_assert(std::same_as<decltype(9007199254740992_f64), double>);
  static_assert(std::same_as<decltype(9007199254740992_f64e), double>);
  static_assert(std::same_as<decltype(9007199254740993_f64), double>);
  static_assert(std::same_as<decltype(0.1_f32), float>);
  static_assert(std::same_as<decltype(0.1_f64), double>);

  EXPECT_FLOAT_EQ(1.25_f32, 1.25f);
  EXPECT_FLOAT_EQ(1.25_f32e, 1.25f);
  EXPECT_DOUBLE_EQ(1.25_f64, 1.25);
  EXPECT_DOUBLE_EQ(1.25_f64e, 1.25);
  EXPECT_EQ(1.25_f80, static_cast<long double>(1.25));
  EXPECT_EQ(1.25_f80e, static_cast<long double>(1.25));
  EXPECT_FLOAT_EQ(16777216_f32, 16777216.0f);
  EXPECT_FLOAT_EQ(16777216_f32e, 16777216.0f);
  EXPECT_FLOAT_EQ(16777217_f32, static_cast<float>(16777217.0L));
  EXPECT_DOUBLE_EQ(9007199254740992_f64, 9007199254740992.0);
  EXPECT_DOUBLE_EQ(9007199254740992_f64e, 9007199254740992.0);
  EXPECT_DOUBLE_EQ(9007199254740993_f64, static_cast<double>(9007199254740993.0L));
  EXPECT_FLOAT_EQ(0.1_f32, static_cast<float>(0.1L));
  EXPECT_FLOAT_EQ(2_f32, 2.0f);
  EXPECT_FLOAT_EQ(2_f32e, 2.0f);
  EXPECT_DOUBLE_EQ(0.1_f64, static_cast<double>(0.1L));
  EXPECT_DOUBLE_EQ(2_f64, 2.0);
  EXPECT_DOUBLE_EQ(2_f64e, 2.0);
  EXPECT_EQ(2_f80, static_cast<long double>(2.0));
  EXPECT_EQ(2_f80e, static_cast<long double>(2.0));
}

TEST(LiteralsTest, CharacterLiteralsReturnExpectedUnderlyingTypes) {
  static_assert(std::same_as<decltype('A'_uchar), unsigned char>);
  static_assert(std::same_as<decltype(u8'A'_char8), char8_t>);
  static_assert(std::same_as<decltype(u'A'_char16), char16_t>);
  static_assert(std::same_as<decltype(U'A'_char32), char32_t>);
  static_assert(std::same_as<decltype(L'A'_wchar), wchar_t>);

  EXPECT_EQ('A'_uchar, static_cast<unsigned char>('A'));
  EXPECT_EQ(u8'A'_char8, u8'A');
  EXPECT_EQ(u'A'_char16, u'A');
  EXPECT_EQ(U'A'_char32, U'A');
  EXPECT_EQ(L'A'_wchar, L'A');
}

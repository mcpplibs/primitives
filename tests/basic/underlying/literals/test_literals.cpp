#include <cstdint>
#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives.underlying;

using namespace mcpplibs::primitives::literals;

TEST(UnderlyingLiteralsTest, IntegerLiteralsReturnExpectedUnderlyingTypes) {
  static_assert(std::same_as<decltype(42_u8), std::uint8_t>);
  static_assert(std::same_as<decltype(42_u16), std::uint16_t>);
  static_assert(std::same_as<decltype(42_u32), std::uint32_t>);
  static_assert(std::same_as<decltype(42_u64), std::uint64_t>);
  static_assert(std::same_as<decltype(42_i8), std::int8_t>);
  static_assert(std::same_as<decltype(42_i16), std::int16_t>);
  static_assert(std::same_as<decltype(42_i32), std::int32_t>);
  static_assert(std::same_as<decltype(42_i64), std::int64_t>);

  EXPECT_EQ(42_u8, static_cast<std::uint8_t>(42));
  EXPECT_EQ(42_u16, static_cast<std::uint16_t>(42));
  EXPECT_EQ(42_u32, static_cast<std::uint32_t>(42));
  EXPECT_EQ(42_u64, static_cast<std::uint64_t>(42));
  EXPECT_EQ(42_i8, static_cast<std::int8_t>(42));
  EXPECT_EQ(42_i16, static_cast<std::int16_t>(42));
  EXPECT_EQ(42_i32, static_cast<std::int32_t>(42));
  EXPECT_EQ(42_i64, static_cast<std::int64_t>(42));
}

TEST(UnderlyingLiteralsTest, FloatingLiteralsReturnExpectedUnderlyingTypes) {
  static_assert(std::same_as<decltype(1.25_f32), float>);
  static_assert(std::same_as<decltype(1.25_f64), double>);
  static_assert(std::same_as<decltype(1.25_f80), long double>);

  EXPECT_FLOAT_EQ(1.25_f32, 1.25f);
  EXPECT_DOUBLE_EQ(1.25_f64, 1.25);
  EXPECT_EQ(1.25_f80, static_cast<long double>(1.25));
  EXPECT_FLOAT_EQ(2_f32, 2.0f);
  EXPECT_DOUBLE_EQ(2_f64, 2.0);
  EXPECT_EQ(2_f80, static_cast<long double>(2.0));
}

TEST(UnderlyingLiteralsTest, CharacterLiteralsReturnExpectedUnderlyingTypes) {
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

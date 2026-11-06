#include <gtest/gtest.h>
#include <print>

#include "scan.hpp"

template <typename... Ts>
bool compareScanResult(const std::tuple<Ts...> &result, const std::tuple<Ts...> &expected) {
    return result == expected;
}

TEST(ScanTest, ParseIntegerArgsTest) {
    auto result = stdx::scan<int, short, long>("-1, -2, 3", "{d}, {d}, {d}");
    ASSERT_TRUE(result);
    ASSERT_TRUE(compareScanResult(result.value().result, {-1, -2, 3}));
}

TEST(ScanTest, ParseUnsignedArgsTest) {
    auto result = stdx::scan<unsigned, uint32_t, size_t>("1, 2, 3", "{u}, {u}, {u}");
    ASSERT_TRUE(result);
    ASSERT_TRUE(compareScanResult(result.value().result, {1, 2, 3}));
}

TEST(ScanTest, ParseFloatArgsTest) {
    auto result = stdx::scan<float, double, float>("1.1, 2.0, 0.3", "{f}, {f}, {f}");
    ASSERT_TRUE(result);
    ASSERT_TRUE(compareScanResult(result.value().result, {1.1, 2.0, 0.3}));
}

TEST(ScanTest, ParseStringArgsTest) {
    auto result = stdx::scan<std::string, std::string_view, std::string>("hello, world, !", "{s}, {s}, {s}");
    ASSERT_TRUE(result);
    ASSERT_TRUE(compareScanResult(result.value().result, {"hello", "world", "!"}));
}

TEST(ScanTest, CvQualifiedTypes) {
    auto result = stdx::scan<const int, volatile int>("1, 1", "{d}, {d}");
    ASSERT_TRUE(result);
    ASSERT_TRUE(compareScanResult(result.value().result, {1, 1}));
}

TEST(WrongScanTest, MissedSpecifier) {
    auto result = stdx::scan<int>("1", "{}");
    ASSERT_FALSE(result);
}

TEST(WrongScanTest, UknownSpecifier) {
    auto result = stdx::scan<int>("1", "{kdfsfds}");
    ASSERT_FALSE(result);
}

TEST(WrongScanTest, TypeDoesNotMatchSpecifier) {
    auto result = stdx::scan<int>("1", "{f}");
    ASSERT_FALSE(result);
}

TEST(WrongScanTest, MoreTypesThanValues) {
    auto result = stdx::scan<int, int, int>("1", "{d}");
    ASSERT_FALSE(result);
}

TEST(WrongScanTest, MoreValuesThanTypes) {
    auto result = stdx::scan<int>("1, 1", "{d}, {d}");
    ASSERT_FALSE(result);
}

TEST(WrongScanTest, DifferentNumOfInputArgsAndFormatSpec) {
    auto result = stdx::scan<int, int>("1, 1", "{d}");
    ASSERT_FALSE(result);
}
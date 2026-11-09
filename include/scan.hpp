#pragma once
#include "parse.hpp"
#include "types.hpp"
#include <expected>
#include <functional>
#include <utility>

namespace {

template <typename T, typename... Ts>
concept OneOf = (std::same_as<T, Ts> || ...);

template <typename T>
concept AllowedType = OneOf<std::remove_cv_t<T>, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t,
                            uint64_t, float, double, std::string_view, std::string>;

}  // namespace

namespace stdx {

template <typename... Ts>
    requires(AllowedType<Ts> && ...)
constexpr std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input,
                                                                               std::string_view format) {
    auto res = details::parse_sources(input, format);
    if (res) {
        const auto &[format_parts, input_parts] = res.value();

        if (format_parts.size() != input_parts.size()) {
            return std::unexpected(details::scan_error{"input and format parts must have the same size"});
        }

        constexpr auto valuesCount = sizeof...(Ts);

        if (valuesCount != input_parts.size()) {
            return std::unexpected(details::scan_error{"input parts and provided types must have the same size"});
        }

        auto checkIsSuccessOrGetError = [](const auto &expectedRes, details::scan_error &err) {
            if (!expectedRes) {
                err = expectedRes.error();
                return false;
            }
            return true;
        };

        auto aggregateConversionsResults =
            [&checkIsSuccessOrGetError](
                auto &&...vals) -> std::expected<details::scan_result<Ts...>, details::scan_error> {
            details::scan_error err;
            auto check = std::bind_back(checkIsSuccessOrGetError, err);
            if ((check(vals) && ...)) {
                return details::scan_result<Ts...>{{std::forward<decltype(vals)>(vals).value()...}};
            }
            return std::unexpected(err);
        };

        auto makeScanResult = [... fs = details::parse_value_with_format<Ts>, input_parts, format_parts,
                               &aggregateConversionsResults]<size_t... Ids>(std::index_sequence<Ids...>) {
            return aggregateConversionsResults(fs(input_parts[Ids], format_parts[Ids])...);
        };

        return makeScanResult(std::make_index_sequence<valuesCount>{});
    } else {
        return std::unexpected(res.error());
    }
}

}  // namespace stdx

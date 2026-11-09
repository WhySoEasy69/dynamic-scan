#pragma once

#include <charconv>
#include <expected>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {

template <typename T>
constexpr std::expected<T, scan_error> convertTo(std::string_view input) {
    typename std::remove_cv_t<T> value;
    auto [ptr, ec] = std::from_chars(input.begin(), input.end(), value);
    if (ec != std::errc{} || ptr != input.end()) {
        return std::unexpected(scan_error{"failed conversion"});
    }
    return value;
}

template <>
constexpr std::expected<std::string, scan_error> convertTo<std::string>(std::string_view input) {
    return std::string(input);
}

template <>
constexpr std::expected<std::string_view, scan_error> convertTo<std::string_view>(std::string_view input) {
    return input;
}

template <typename T>
constexpr std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    if (fmt.empty())
        return std::unexpected(details::scan_error{"format specifier is missed"});

    if ((fmt == "d" && std::is_integral_v<T>) ||
        (fmt == "s" && (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)) ||
        (fmt == "u" && std::is_unsigned_v<T>) || (fmt == "f" && std::is_floating_point_v<T>)) {
        return convertTo<T>(input);
    }
    return std::unexpected(details::scan_error{"unexpected format specifier"});
}

// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
constexpr std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        std::string_view remaining_format = format.substr(start);
        auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

}  // namespace stdx::details
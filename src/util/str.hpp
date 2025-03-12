#pragma once
#include <type_traits>
#include <string_view>
#include <cctype>

namespace util {
    template <class T>
    static int strcmpi(std::basic_string_view<T> lhs, std::basic_string_view<T> rhs) {
        using uT = std::make_unsigned_t<T>;
        for (size_t i = 0; i < lhs.length(); ++i) {
            int lhs_char = std::tolower((uT)lhs[i]);
            int rhs_char = 0;
            if (i < rhs.length())
                rhs_char = std::tolower((uT)rhs[i]);
            if (lhs_char-rhs_char != 0)
                return lhs_char-rhs_char;
        }
        if (rhs.length() > lhs.length()) {
            return -std::tolower((uT)rhs[lhs.length()]);
        }
        return 0;
    }
}
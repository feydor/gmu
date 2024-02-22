#ifndef GMU_UTILS_H
#define GMU_UTILS_H

#include <type_traits>
#include <string>
#include <stdexcept>

namespace Utils {

    /** Throws an exception when the pointer param is null */
    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, void>::type
    require_nonnull(T t, const std::string &msg) {
        if (t == nullptr || t == NULL)
            throw std::runtime_error(msg);
    }

    std::string format_min_sec(float time_s);
};

#endif

//
// Created by willzhang on 2020/9/23.
//

#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include "native_debug.h"

class TimeUtil {
public:
    static long now() {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
};

#endif //TIME_UTIL_H

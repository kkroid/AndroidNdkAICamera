#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>
#include "native_debug.h"

class Frame {
public:
    uint8_t *data;
    int width = 0;
    int height = 0;
    int len = 0;

    Frame(uint8_t *data, int width, int height, int len)
            : data(data), width(width), height(height), len(len) {
    }

    ~Frame() {
        // LOGI("~Frame");
        delete data;
    }

    Frame *copy() {
        auto *copy = new uint8_t[len];
        memcpy(copy, data, len);
        return new Frame(copy, width, height, len);
    }
};

#endif //FRAME_H

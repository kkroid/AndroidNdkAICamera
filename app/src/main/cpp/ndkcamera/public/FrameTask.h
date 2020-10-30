#ifndef __FRAME_TASK_H__
#define __FRAME_TASK_H__

#include <stdint.h>
#include "native_debug.h"
#include "time_util.h"
#include "Frame.h"

class FrameTask {
public:
    int32_t fps = 2;
    std::string name;
    long lastFrameTime;
    int frameCount;

    FrameTask(std::string name, int32_t fps) : fps(fps), name(name) {
        frameCount = 0;
        lastFrameTime = -1;
    }

    virtual ~FrameTask() {
        LOGI("~FrameTask");
    }

    /**
     * 过滤多余的帧
     */
    bool ignoreFrame() {
        long now = TimeUtil::now();
//        if (lastFrameTime == -1) {
//            lastFrameTime = now;
//        }
        if (now - lastFrameTime < 1000) {
            if (frameCount >= fps) {
                // LOGI("%s ignored, frameCount = %d, fps = %d", name.c_str(), frameCount, fps);
                return true;
            }
        } else {
            frameCount = 0;
            lastFrameTime = now;
        }
        frameCount++;
        return false;
    }

    /**
     * Feed raw data
     * @return if handled return true, else return false;
     */
    virtual void doTask(Frame *frame) = 0;
};
#endif

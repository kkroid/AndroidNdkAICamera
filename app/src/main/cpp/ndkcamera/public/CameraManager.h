
/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __CAMERA_MANAGER_H__
#define __CAMERA_MANAGER_H__

#include <stdint.h>
#include <functional>
#include <thread>

#include "FrameProcesser.h"
#include <native_debug.h>
#include "ICameraServer.h"
#include <media/NdkImage.h>

/**
 * basic CameraAppEngine
 */
class CameraManager : public PreviewCallback {
public:
    int32_t frameWidth = 240;
    int32_t frameHeight = 320;
    int32_t frameRotation = 90;
    int32_t frameFormat = AIMAGE_FORMAT_YUV_420_888;
    int32_t cameraId = 0;

    static CameraManager *getInstance() {
        static CameraManager instance;
        return &instance;
    }

    ~CameraManager();

    void startPreview();

    void stopPreview();

    void addFrameTask(FrameTask *_frameTask) {
        frameTaskMap[_frameTask->name] = _frameTask;
        LOGI("task %s added", _frameTask->name.c_str());
        if (!cameraReady) {
            startPreview();
        }
    }

    void removeFrameTask(std::string taskName) {
        if (frameTaskMap.erase(taskName)) {
            LOGI("task %s removed", taskName.c_str());
        }
        if (frameTaskMap.empty()) {
            stopPreview();
        }
    }

    FrameTask *getFrameTask(std::string name) {
        return frameTaskMap[name];
    }

    void onFrameAvailable(Frame *frame) override;

private:
    std::map<std::string, FrameTask *> frameTaskMap;
    FrameProcessor *frameProcessor = new FrameProcessor(&frameTaskMap);

    volatile bool cameraReady;
    CameraServer *cameraServer;

    CameraManager() :
            cameraReady(false),
            cameraServer(nullptr) {
    }
};

#endif  // __CAMERA_MANAGER_H__
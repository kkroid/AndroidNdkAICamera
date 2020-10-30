
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
#include "CameraServerConfig.h"
#include <native_debug.h>
#include "ICameraServer.h"

/**
 * basic CameraAppEngine
 */
class CameraManager : public PreviewCallback {
public:
    static CameraManager *getInstance() {
        static CameraManager instance;
        return &instance;
    }

    void init(CameraServerConfig *configuration) {
        setFrameSize(configuration->getWidth(), configuration->getHeight());
        setFrameFormat(configuration->getFormat());
        setFrameFps(configuration->getFps());
        setFrameRotation(configuration->getRotation());
        setFrameTasks(configuration->getFrameTasks());
        frameProcessor = new FrameProcessor(&frameTaskMap);
        refreshFps();
    }

    ~CameraManager();

    void startPreview();

    void stopPreview();

    void setFrameSize(int32_t _frameWidth, int32_t _frameHeight) {
        frameWidth = _frameWidth;
        frameHeight = _frameHeight;
        if (cameraReady) {
            cameraServer->setFrameSize(_frameWidth, _frameHeight);
        }
    }

    void setFrameRotation(int32_t _frameRotation) {
        frameRotation = _frameRotation;
        if (cameraReady) {
            cameraServer->setFrameRotation(_frameRotation);
        }
    }

    void setFrameFormat(int32_t _frameFormat) {
        frameFormat = _frameFormat;
//        cameraServer->setFrameFormat(_frameFormat);
    }

    void setFrameFps(int32_t _frameFps) {
        frameFps = _frameFps;
        if (cameraReady) {
            cameraServer->setFrameFps(_frameFps);
        }
    }

    void setFrameTasks(const std::map<std::string, FrameTask *> &_frameTaskMap) {
        frameTaskMap = _frameTaskMap;
        LOGI("Added %ld tasks", (long) frameTaskMap.size());
    }

    void addFrameTask(FrameTask *_frameTask) {
        frameTaskMap[_frameTask->name] = _frameTask;
        LOGI("task %s added", _frameTask->name.c_str());
        if (!cameraReady) {
            startPreview();
        } else {
            refreshFps();
        }
    }

    void removeFrameTask(std::string taskName) {
        if (frameTaskMap.erase(taskName)) {
            LOGI("task %s removed", taskName.c_str());
        }
        if (frameTaskMap.empty()) {
            stopPreview();
        } else {
            refreshFps();
        }
    }

    void onFrameAvailable(Frame *frame) override;

private:
    int32_t frameWidth = 480;
    int32_t frameHeight = 640;
    int32_t frameRotation = 270;
    int32_t frameFormat = 0;
    int32_t frameFps = 5;
    std::map<std::string, FrameTask *> frameTaskMap;
    FrameProcessor *frameProcessor;

    volatile bool cameraReady;
    CameraServer *cameraServer;

    CameraManager() :
            cameraReady(false),
            cameraServer(nullptr) {
    }

    void refreshFps() {
        int tmpFps = frameFps;
        if (frameFps == 0) {
            std::map<std::string, FrameTask *>::iterator taskIterator;
            for (taskIterator = frameTaskMap.begin(); taskIterator != frameTaskMap.end(); taskIterator++) {
                auto *frameTask = taskIterator->second;
                tmpFps = frameTask->fps > tmpFps ? frameTask->fps : tmpFps;
            }
        }
        LOGI("refreshFps to %d", tmpFps);
        setFrameFps(tmpFps);
    }
};

#endif  // __CAMERA_MANAGER_H__
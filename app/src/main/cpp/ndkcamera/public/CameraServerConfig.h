#ifndef __CAMERA_ENGINE_CONFIGURATION_H__
#define __CAMERA_ENGINE_CONFIGURATION_H__

#include <stdint.h>
#include "FrameTask.h"
#include "CameraManager.h"

class CameraServerConfig {
public:
    int getWidth() const {
        return width;
    }

    void setWidth(int _width) {
        width = _width;
    }

    int getHeight() const {
        return height;
    }

    void setHeight(int _height) {
        height = _height;
    }

    int32_t getRotation() const {
        return rotation;
    }

    void setRotation(int32_t _rotation) {
        rotation = _rotation;
    }

    int32_t getFormat() const {
        return format;
    }

    void setFormat(int32_t _format) {
        format = _format;
    }

    void setCameraId(int32_t _cameraId) {
        cameraId = _cameraId;
    }

    int getCameraId() {
        return cameraId;
    }

    const std::map<std::string, FrameTask *> &getFrameTasks() const {
        return frameTaskMap;
    }

    void setFrameTasks(const std::map<std::string, FrameTask *> &_frameTaskMap) {
        frameTaskMap = _frameTaskMap;
    }

    ~CameraServerConfig() {
        LOGI("~CameraServerConfig");
        frameTaskMap.clear();
    }

private:
    int width = 0;
    int height = 0;
    int32_t rotation = 0;
    int32_t format = 0;
    int32_t cameraId = 0;
    std::map<std::string, FrameTask *> frameTaskMap;
};


class CameraServerConfigBuilder {
public:
    CameraServerConfigBuilder() {}

    ~CameraServerConfigBuilder() {
        LOGI("~CameraServerConfigBuilder");
        frameTaskMap.clear();
    }

    static CameraServerConfigBuilder *newBuilder() {
        return new CameraServerConfigBuilder();
    }

    CameraServerConfig *build() {
        CameraServerConfig *config = new CameraServerConfig();
        if (width > 0) {
            config->setWidth(width);
        }
        if (height > 0) {
            config->setHeight(height);
        }
        config->setCameraId(cameraId);
        config->setRotation(rotation);
        config->setFormat(format);
        config->setFrameTasks(frameTaskMap);
        return config;
    }

    CameraServerConfigBuilder *frameWidth(int _width) {
        width = _width;
        return this;
    }

    CameraServerConfigBuilder *frameHeight(int _height) {
        height = _height;
        return this;
    }

    CameraServerConfigBuilder *frameRotation(int _orientation) {
        rotation = _orientation;
        return this;
    }

    CameraServerConfigBuilder *setCameraId(int _cameraId) {
        cameraId = _cameraId;
        return this;
    }

    CameraServerConfigBuilder *frameFormat(int32_t frameFormat) {
        format = frameFormat;
        return this;
    }

    CameraServerConfigBuilder *addFrameTask(FrameTask *frameTask) {
        frameTaskMap[frameTask->name] = frameTask;
        return this;
    }

private:
    int width = 0;
    int height = 0;
    int32_t rotation = 0;
    int32_t format = 0;
    int32_t cameraId = 0;
    std::map<std::string, FrameTask *> frameTaskMap;
};

#endif

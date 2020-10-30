#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include "PreviewCallback.h"
#include <stdint.h>

class CameraServer {
public:
    int32_t frameWidth = 480;
    int32_t frameHeight = 640;
    int32_t frameRotation = 270;
    int32_t frameFormat = 0;

    PreviewCallback *previewCallback = nullptr;
    std::map<std::string, FrameTask *> frameTaskMap;

    virtual ~CameraServer() {
    };

    /**
     * 开启预览
     */
    virtual void startPreview() = 0;

    /**
     * 关闭预览
     */
    virtual void stopPreview() = 0;

    /**
     * 设置帧监听器
     */
    virtual void setPreviewCallback(PreviewCallback *callback) {
        previewCallback = callback;
    };

    /**
     * 设置预览帧大小
     */
    virtual void setFrameSize(int32_t _frameWidth, int32_t _frameHeight) = 0;

    /**
     * 设置摄像头角度
     */
    virtual void setFrameRotation(int32_t _frameRotation) = 0;

    virtual void setFrameTask(std::map<std::string, FrameTask *> _frameTaskMap) {
        frameTaskMap = _frameTaskMap;
    };
};

#endif //CAMERASERVER_H
#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include "PreviewCallback.h"
#include <stdint.h>

class CameraServer {
public:
    int32_t frameFormat = 0;
    int32_t frameWidth = 240;
    int32_t frameHeight = 320;
    int32_t frameRotation = 90;

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

    virtual void setFrameTask(std::map<std::string, FrameTask *> _frameTaskMap) {
        frameTaskMap = _frameTaskMap;
    };
};

#endif //CAMERASERVER_H
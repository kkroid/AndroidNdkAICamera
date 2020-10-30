#include "CameraManager.h"
#include <cstdio>
#include <utility>
#include <ctpl_stl.h>
#include <blockingconcurrentqueue.h>
#include <Frame.h>
#include "native_debug.h"
#include <map>

#include <AndroidCameraServer.h>


CameraManager::~CameraManager() {
    LOGI("~CameraManager");
    cameraReady = false;
    delete frameProcessor;
    frameProcessor = nullptr;
}

/**
 * Create a camera object for onboard FRONT_FACING camera
 */
void CameraManager::startPreview() {
#if PLATFORM_ANDROID
    cameraServer = new AndroidCameraServer(frameWidth, frameHeight, frameRotation, frameFormat, cameraId);
    LOGI("startPreview, task size = %ld", (long) frameTaskMap.size());
    cameraServer->setPreviewCallback(this);
    cameraServer->setFrameTask(frameTaskMap);
    cameraServer->startPreview();
#endif
}

void CameraManager::stopPreview() {
    if (cameraServer) {
        cameraServer->stopPreview();
        delete cameraServer;
        cameraServer = nullptr;
    }
    cameraReady = false;
    LOGI("Camera Closed");
}

void CameraManager::onFrameAvailable(Frame *frame) {
    if (nullptr != frameProcessor) {
        frameProcessor->push(frame);
    }
}

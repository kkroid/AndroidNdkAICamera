#include "AndroidCameraServer.h"
#include "AndroidCameraHandler.h"
#include <camera/NdkCameraManager.h>
#include <utility>
#include <unistd.h>
#include <cinttypes>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <media/NdkImage.h>
#include <UIPreview.h>
#include "native_debug.h"

AndroidCameraServer::AndroidCameraServer(int32_t _frameWidth,
                                         int32_t _frameHeight,
                                         int32_t _frameRotation,
                                         int32_t _frameFormat,
                                         int32_t _cameraId) :
        cameraMgr(nullptr),
        activeCameraId(""),
        cameraOrientation(0),
        outputContainer(nullptr),
        captureSessionState(CaptureSessionState::MAX_STATE) {

    frameWidth = _frameWidth;
    frameHeight = _frameHeight;
    frameRotation = _frameRotation;
    frameFormat = _frameFormat;
    imageDataReader = nullptr;

    captureSession = nullptr;

    requests.resize(CAPTURE_REQUEST_COUNT);
    memset(requests.data(), 0, requests.size() * sizeof(requests[0]));
    cameraMgr = ACameraManager_create();
    ASSERT(cameraMgr, "Failed to create cameraManager")
    // Pick up a front-facing camera to preview
    selectCameraId(_cameraId);
    LOGI("selected camera id:%s", activeCameraId.id.c_str());
    // Create back facing camera device
    CALL_MGR(openCamera(cameraMgr, activeCameraId.id.c_str(), getDeviceListener(),
                        &activeCameraId.device))
    CALL_MGR(registerAvailabilityCallback(cameraMgr, getManagerListener()))
}

AndroidCameraServer::~AndroidCameraServer() {
    LOGI("~AndroidCameraServer");
    // stop session if it is on:
    if (captureSessionState == CaptureSessionState::ACTIVE) {
        LOGI("close session:%p", captureSession);
        ACameraCaptureSession_stopRepeating(captureSession);
    }
    ACameraCaptureSession_close(captureSession);

    for (auto &req: requests) {
        CALL_REQUEST(removeTarget(req.aRequest, req.outputTarget))
        ACaptureRequest_free(req.aRequest);
        ACameraOutputTarget_free(req.outputTarget);
        CALL_CONTAINER(remove(outputContainer, req.sessionOutput))
        ACaptureSessionOutput_free(req.sessionOutput);
        ANativeWindow_release(req.outputNativeWindow);
    }
//    delete captureRequests;
    requests.resize(0);

    ACaptureSessionOutputContainer_free(outputContainer);
    CALL_DEV(close(activeCameraId.device))
    if (cameraMgr) {
        CALL_MGR(unregisterAvailabilityCallback(cameraMgr, getManagerListener()))
        ACameraManager_delete(cameraMgr);
        cameraMgr = nullptr;
    }

    delete imageDataReader;
    imageDataReader = nullptr;

    delete imageStreamCallbackImpl;
    imageStreamCallbackImpl = nullptr;
    LOGI("release android camera server");
}

void AndroidCameraServer::startPreview() {
    if (nullptr != previewCallback) {
        ImageFormat view = {frameWidth, frameHeight, frameFormat};
        imageStreamCallbackImpl = new ImageStreamCallbackImpl(&frameTaskMap, previewCallback, frameRotation);
        imageDataReader = new AndroidImageReader(view, imageStreamCallbackImpl);
        LOGI("startPreview:width = %d, height = %d, format = %d, rotation = %d",
             frameWidth,
             frameHeight,
             frameFormat,
             frameRotation);
        setUIPreviewEnable(true);
        createSession(imageDataReader->getNativeWindow(), frameRotation);
    }
    reTriggerRepeatingRequest();
}

void AndroidCameraServer::stopPreview() {
    if (captureSessionState == CaptureSessionState::ACTIVE) {
        LOGI("stopPreview");
        ACameraCaptureSession_stopRepeating(captureSession);
    }
}

void AndroidCameraServer::selectCameraId(int32_t facing) {
    ACameraIdList *cameraIds = nullptr;
    CALL_MGR(getCameraIdList(cameraMgr, &cameraIds))
    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];
        ACameraMetadata *metadataObj;
        CALL_MGR(getCameraCharacteristics(cameraMgr, id, &metadataObj))
        int32_t count = 0;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx = 0; tagIdx < count; ++tagIdx) {
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {
                        0,
                };
                CALL_METADATA(getConstEntry(metadataObj, tags[tagIdx], &lensInfo))
                int cid = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                        lensInfo.data.u8[0]);
                if (cid == facing) {
                    CameraId cam(id);
                    cam.facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                            lensInfo.data.u8[0]);
                    cam.owner = false;
                    cam.device = nullptr;
                    activeCameraId = cam;
                    break;
                }
            }
        }
        ACameraMetadata_free(metadataObj);
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

void AndroidCameraServer::createSession(ANativeWindow *imageStreamWindow, int32_t imageRotation) {
    LOGI("Create session");
    CALL_CONTAINER(create(&outputContainer))
    ANativeWindow *uiPreviewWindow = nullptr;
    if (uiPreviewEnable) {
        LOGI("uiPreviewEnable:%d", uiPreviewEnable);
        uiPreviewWindow = UIPreview::getInstance()->getNativeWindow();
    }
    requests[UI_PREVIEW_IDX].outputNativeWindow = uiPreviewWindow;
    requests[UI_PREVIEW_IDX].requestTemplate = TEMPLATE_PREVIEW;
    requests[IMAGE_STREAM_IDX].outputNativeWindow = imageStreamWindow;
    requests[IMAGE_STREAM_IDX].requestTemplate = TEMPLATE_PREVIEW;

    uint8_t aeMode = ACAMERA_CONTROL_AE_MODE_ON;
    uint8_t afMode = ACAMERA_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
    uint8_t awbMode = ACAMERA_CONTROL_AWB_MODE_AUTO;
    for (auto &req : requests) {
        if (!req.outputNativeWindow) {
            continue;
        }
        ANativeWindow_acquire(req.outputNativeWindow);
        CALL_OUTPUT(create(req.outputNativeWindow, &(req.sessionOutput)))
        CALL_CONTAINER(add(outputContainer, req.sessionOutput))
        CALL_TARGET(create(req.outputNativeWindow, &(req.outputTarget)))
        CALL_DEV(createCaptureRequest(activeCameraId.device, req.requestTemplate, &(req.aRequest)))
        CALL_REQUEST(addTarget(req.aRequest, req.outputTarget))

        ACaptureRequest_setEntry_i32(req.aRequest, ACAMERA_JPEG_ORIENTATION, 1, &imageRotation);
        ACaptureRequest_setEntry_u8(req.aRequest, ACAMERA_CONTROL_AE_MODE, 1, &aeMode);
        ACaptureRequest_setEntry_u8(req.aRequest, ACAMERA_CONTROL_AF_MODE, 1, &afMode);
        ACaptureRequest_setEntry_u8(req.aRequest, ACAMERA_CONTROL_AWB_MODE, 1, &awbMode);
    }

//    captureRequests[UI_PREVIEW_IDX] = requests[UI_PREVIEW_IDX].aRequest;
//    captureRequests[IMAGE_STREAM_IDX] = requests[IMAGE_STREAM_IDX].aRequest;

    // Create a capture session for the given preview request
    captureSessionState = CaptureSessionState::READY;

    CALL_DEV(createCaptureSession(activeCameraId.device, outputContainer, getSessionListener(), &captureSession))

//    ACaptureRequest_setEntry_i32(requests[IMAGE_STREAM_IDX].aRequest,
//                                 ACAMERA_CONTROL_AE_TARGET_FPS_RANGE,
//                                 2,
//                                 new int32_t[]{30, 30});
//    if (nullptr != uiPreviewWindow) {
//        ACaptureRequest_setEntry_i32(requests[UI_PREVIEW_IDX].aRequest,
//                                     ACAMERA_CONTROL_AE_TARGET_FPS_RANGE,
//                                     2,
//                                     new int32_t[]{30, 30});
//    }
}

void AndroidCameraServer::reTriggerRepeatingRequest() {
    ACaptureRequest *captureRequests[] = {requests[UI_PREVIEW_IDX].aRequest, requests[IMAGE_STREAM_IDX].aRequest};
    CALL_SESSION(setRepeatingRequest(captureSession,
                                     nullptr,
                                     CAPTURE_REQUEST_COUNT,
//                                      &(requests[UI_PREVIEW_IDX].aRequest),
                                     captureRequests,
                                     nullptr))
}

void AndroidCameraServer::captureRequestSetEntryI32(uint32_t tag, uint32_t count, const int32_t *data) {
     CALL_REQUEST(setEntry_i32(requests[UI_PREVIEW_IDX].aRequest, tag, count, data))
     CALL_REQUEST(setEntry_i32(requests[IMAGE_STREAM_IDX].aRequest, tag, count, data))
//    for (int i = 0; i < CAPTURE_REQUEST_COUNT; ++i) {
//        ACaptureRequest *req = captureRequests[i];
//        CALL_REQUEST(setEntry_i32(req, tag, count, data))
//    }
    reTriggerRepeatingRequest();
}

void AndroidCameraServer::captureRequestSetEntryU8(uint32_t tag, uint32_t count, const uint8_t *data) {
     CALL_REQUEST(setEntry_u8(requests[UI_PREVIEW_IDX].aRequest, tag, count, data))
     CALL_REQUEST(setEntry_u8(requests[IMAGE_STREAM_IDX].aRequest, tag, count, data))
//    for (int i = 0; i < CAPTURE_REQUEST_COUNT; ++i) {
//        ACaptureRequest *req = captureRequests[i];
//        CALL_REQUEST(setEntry_u8(req, tag, count, data))
//    }
    reTriggerRepeatingRequest();
}

void AndroidCameraServer::captureRequestSetEntryI64(uint32_t tag, uint32_t count, const int64_t *data) {
     CALL_REQUEST(setEntry_i64(requests[UI_PREVIEW_IDX].aRequest, tag, count, data))
     CALL_REQUEST(setEntry_i64(requests[IMAGE_STREAM_IDX].aRequest, tag, count, data))
//    for (int i = 0; i < CAPTURE_REQUEST_COUNT; ++i) {
//        ACaptureRequest *req = captureRequests[i];
//        CALL_REQUEST(setEntry_i64(req, tag, count, data))
//    }
    reTriggerRepeatingRequest();
}

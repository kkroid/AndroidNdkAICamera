#ifndef __ANDROID_CAMERA_H__
#define __ANDROID_CAMERA_H__

//#pragma clang diagnostic push
#pragma pack(push)
#pragma pack(8)

#include <string>
#include <stdint.h>
#include <map>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraMetadataTags.h>
#include <FrameProcesser.h>
#include "AndroidImageReader.h"
#include "ICameraServer.h"
#include <ImageStreamCallbackImpl.h>

enum class CaptureSessionState : int32_t {
    READY = 0,  // session is ready
    ACTIVE,     // session is busy
    CLOSED,     // session is closed(by itself or a new session evicts)
    MAX_STATE
};

struct CaptureRequestInfo {
    ANativeWindow *outputNativeWindow;
    ACaptureSessionOutput *sessionOutput;
    ACameraOutputTarget *outputTarget;
    ACaptureRequest *aRequest;
    ACameraDevice_request_template requestTemplate;
    int sessionSequenceId;
};

enum PREVIEW_INDICES {
    IMAGE_STREAM_IDX = 0,
    UI_PREVIEW_IDX,
    CAPTURE_REQUEST_COUNT,
};

class CameraId {
public:
    ACameraDevice *device;
    std::string id;
    acamera_metadata_enum_android_lens_facing_t facing;
    bool available;  // free to use ( no other apps are using
    bool owner;      // we are the owner of the camera
    explicit CameraId(const char *_id)
            : device(nullptr),
              facing(ACAMERA_LENS_FACING_FRONT),
              available(false),
              owner(false) {
        id = _id;
    }

    explicit CameraId(void) { CameraId(""); }

    ~CameraId() {
        // LOGI("~CameraId");
    }
};

class AndroidCameraServer : public CameraServer {
private:
    ACameraManager *cameraMgr;
    CameraId activeCameraId;
    uint32_t cameraOrientation;
    AndroidImageReader *imageDataReader;

    std::vector<CaptureRequestInfo> requests;
//    ACaptureRequest **captureRequests;

    ACaptureSessionOutputContainer *outputContainer;
    ACameraCaptureSession *captureSession;
    CaptureSessionState captureSessionState;

    ImageStreamCallbackImpl *imageStreamCallbackImpl = nullptr;

    bool uiPreviewEnable = false;
    struct android_app *app;

public:
    AndroidCameraServer(int32_t frameWidth,
                        int32_t frameHeight,
                        int32_t frameRotation,
                        int32_t frameFormat,
                        int32_t cameraId);

    ~AndroidCameraServer();

    void startPreview() override;

    void stopPreview() override;

    void setFrameSize(int32_t _frameWidth, int32_t _frameHeight) override;

    void setFrameRotation(int32_t _frameRotation) override;

    void setUIPreviewEnable(bool enable) {
        uiPreviewEnable = enable;
    }

    void onCameraStatusChanged(const char *id, bool available);

    void onDeviceState(ACameraDevice *dev);

    void onDeviceError(ACameraDevice *dev, int err);

    void onSessionState(ACameraCaptureSession *ses, CaptureSessionState state);

    void onCaptureSequenceEnd(int sequenceId);

    void onCaptureFailed(ACaptureRequest *request, ACameraCaptureFailure *failure);

private:
    ACameraManager_AvailabilityCallbacks *getManagerListener();

    ACameraDevice_stateCallbacks *getDeviceListener();

    ACameraCaptureSession_stateCallbacks *getSessionListener();

    /**
     * 将设置推到camera
     */
    void reTriggerRepeatingRequest();

    /**
     * 选摄像头
     */
    void selectCameraId(int32_t facing);

    void createSession(ANativeWindow *imageStreamWindow, int32_t imageRotation);

    /**
     * 设置int32类型参数
     */
    void captureRequestSetEntryI32(uint32_t tag, uint32_t count, const int32_t *data);

    /**
     * 设置uint8类型参数
     */
    void captureRequestSetEntryU8(uint32_t tag, uint32_t count, const uint8_t *data);

    /**
     * 设置int64类型参数
     */
    void captureRequestSetEntryI64(uint32_t tag, uint32_t count, const int64_t *data);
};

#pragma pack(pop)
//#pragma clang diagnostic pop
#endif
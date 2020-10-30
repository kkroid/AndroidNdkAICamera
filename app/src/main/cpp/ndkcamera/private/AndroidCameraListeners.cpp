//
// Created by Will Zhang on 2020/9/22.
//
#ifndef __LISTENERS_H__
#define __LISTENERS_H__

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraDevice.h>
#include "AndroidCameraServer.h"
#include "AndroidCameraHandler.h"

/*
 * Camera Manager Listener object
 */
void onCameraAvailable(void *ctx, const char *id) {
    reinterpret_cast<AndroidCameraServer *>(ctx)->onCameraStatusChanged(id, true);
}

void onCameraUnavailable(void *ctx, const char *id) {
    reinterpret_cast<AndroidCameraServer *>(ctx)->onCameraStatusChanged(id, false);
}

void onDeviceStateChanges(void *ctx, ACameraDevice *dev) {
    reinterpret_cast<AndroidCameraServer *>(ctx)->onDeviceState(dev);
}

void onDeviceErrorChanges(void *ctx, ACameraDevice *dev, int err) {
    reinterpret_cast<AndroidCameraServer *>(ctx)->onDeviceError(dev, err);
}

/**
 * onCameraStatusChanged()
 *  handles Callback from ACameraManager
 */
void AndroidCameraServer::onCameraStatusChanged(const char *id, bool available) {
    if (activeCameraId.id == id) {
        activeCameraId.available = available;
        LOGI("activeCameraId %s available:%d", id, available);
    }
    LOGI("NDK Camera %s available:%d", id, available);
}

void AndroidCameraServer::onDeviceState(ACameraDevice *dev) {
    std::string id(ACameraDevice_getId(dev));
    LOGW("device %s is disconnected", id.c_str());
    if (activeCameraId.id == id) {
        activeCameraId.available = false;
        ACameraDevice_close(activeCameraId.device);
    }
}

void AndroidCameraServer::onDeviceError(ACameraDevice *dev, int err) {
    std::string id(ACameraDevice_getId(dev));

    if (activeCameraId.id == id) {
        LOGI("CameraDevice %s is in error %#x", id.c_str(), err);
        PrintCameraDeviceError(err);
        CameraId &cam = activeCameraId;
        switch (err) {
            case ERROR_CAMERA_IN_USE: // NOLINT(bugprone-branch-clone)
                cam.available = false;
                cam.owner = false;
                break;
            case ERROR_CAMERA_SERVICE:
            case ERROR_CAMERA_DEVICE:
            case ERROR_CAMERA_DISABLED:
            case ERROR_MAX_CAMERAS_IN_USE:
                cam.available = false;
                cam.owner = false;
                break;
            default:
                LOGI("Unknown Camera Device Error: %#x", err);
        }
    }
}

void AndroidCameraServer::onSessionState(ACameraCaptureSession *ses, CaptureSessionState state) {
    if (!ses || ses != captureSession) {
        LOGW("CaptureSession is %s, state:%d", (ses ? "NOT our session" : "NULL"), state);
        return;
    }
    ASSERT(state < CaptureSessionState::MAX_STATE, "Wrong state %d", state)
    captureSessionState = state;
}

void AndroidCameraServer::onCaptureSequenceEnd(int sequenceId) {
    if (sequenceId != requests[UI_PREVIEW_IDX].sessionSequenceId)
        return;
    LOGI("onCaptureSequenceEnd");
    // resume preview
    CALL_SESSION(setRepeatingRequest(captureSession,
                                     nullptr, 1,
                                     &requests[UI_PREVIEW_IDX].aRequest,
                                     nullptr))
}

void AndroidCameraServer::onCaptureFailed(ACaptureRequest *_request, ACameraCaptureFailure *failure) {
    ASSERT(failure->sequenceId == requests[UI_PREVIEW_IDX].sessionSequenceId, "Error jpg sequence id")
    // startPreview();
}

ACameraManager_AvailabilityCallbacks *AndroidCameraServer::getManagerListener() {
    static ACameraManager_AvailabilityCallbacks cameraMgrListener = {
            .context = this,
            .onCameraAvailable = ::onCameraAvailable,
            .onCameraUnavailable = ::onCameraUnavailable,
    };
    return &cameraMgrListener;
}

ACameraDevice_stateCallbacks *AndroidCameraServer::getDeviceListener() {
    static ACameraDevice_stateCallbacks cameraDeviceListener = {
            .context = this,
            .onDisconnected = ::onDeviceStateChanges,
            .onError = ::onDeviceErrorChanges,
    };
    return &cameraDeviceListener;
}

// CaptureSession state callbacks
void onSessionClosed(void *ctx, ACameraCaptureSession *ses) {
    LOGI("session %p closed", ses);
    reinterpret_cast<AndroidCameraServer *>(ctx)
            ->onSessionState(ses, CaptureSessionState::CLOSED);
}

void onSessionReady(void *ctx, ACameraCaptureSession *ses) {
    LOGI("session %p ready", ses);
    reinterpret_cast<AndroidCameraServer *>(ctx)
            ->onSessionState(ses, CaptureSessionState::READY);
}

void onSessionActive(void *ctx, ACameraCaptureSession *ses) {
    LOGI("session %p active", ses);
    reinterpret_cast<AndroidCameraServer *>(ctx)
            ->onSessionState(ses, CaptureSessionState::ACTIVE);
}

ACameraCaptureSession_stateCallbacks *AndroidCameraServer::getSessionListener() {
    static ACameraCaptureSession_stateCallbacks sessionListener = {
            .context = this,
            .onActive = ::onSessionActive,
            .onReady = ::onSessionReady,
            .onClosed = ::onSessionClosed,
    };
    return &sessionListener;
}

#endif

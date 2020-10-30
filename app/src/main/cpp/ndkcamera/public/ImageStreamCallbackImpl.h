#ifndef IMAGESTREAMCALLBACKIMPL_H
#define IMAGESTREAMCALLBACKIMPL_H

#include "AndroidImageReader.h"

class ImageStreamCallbackImpl : public ImageStreamCallback {
public:
    explicit ImageStreamCallbackImpl(std::map<std::string, FrameTask *> *_frameTaskMap,
                                     PreviewCallback *callback,
                                     int rotation) :
            frameTaskMap(_frameTaskMap),
            callback(callback),
            frameRotation(rotation) {
    }

    ~ImageStreamCallbackImpl() {
        LOGI("ImageStreamCallbackImpl");
    }

private:
    std::map<std::string, FrameTask *> *frameTaskMap;
    PreviewCallback *callback;
    int frameRotation = 0;

    static uint8_t *getI420Data(AImage *image, int &width, int &height, int &len, int rotate) {
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);

        int32_t yPixelStride = 0, srcStrideY = 0, cbPixelStride = 0, srcPixelStrideUv = 0, srcStrideU = 0, srcStrideV = 0;
        uint8_t *srcY, *srcU, *srcV;
        int yLength, cbLength, crLength;
        AImage_getPlanePixelStride(image, 0, &yPixelStride);
        AImage_getPlaneRowStride(image, 0, &srcStrideY);
        AImage_getPlaneData(image, 0, &srcY, &yLength);
        AImage_getPlanePixelStride(image, 1, &cbPixelStride);
        AImage_getPlaneRowStride(image, 1, &srcStrideU);
        AImage_getPlaneData(image, 1, &srcU, &cbLength);
        AImage_getPlanePixelStride(image, 2, &srcPixelStrideUv);
        AImage_getPlaneRowStride(image, 2, &srcStrideV);
        AImage_getPlaneData(image, 2, &srcV, &crLength);

        const int y_plane_length = width * height;
        const int uv_plane_length = y_plane_length / 4;
        int buffer_length = y_plane_length + uv_plane_length * 2;
        len = buffer_length;
        uint8_t *buffer = new uint8_t[buffer_length];
        libyuv::Android420ToI420(srcY,
                                 srcStrideY,
                                 srcU,
                                 srcStrideU,
                                 srcV,
                                 srcStrideV,
                                 srcPixelStrideUv,
                                 buffer,
                                 width,
                                 buffer + y_plane_length,
                                 width / 2,
                                 buffer + y_plane_length + uv_plane_length,
                                 width / 2,
                                 width,
                                 height);
        if (rotate % 90 == 0) {
            const int rotate_y_plane_length = width * height;
            const int rotate_uv_plane_length = rotate_y_plane_length / 4;
            const int rotate_buffer_length = rotate_y_plane_length + rotate_uv_plane_length * 2;
            uint8_t *rotatedBuffer = new uint8_t[rotate_buffer_length];
            libyuv::I420Rotate(
                    buffer,
                    width,
                    buffer + rotate_y_plane_length,
                    width / 2,
                    buffer + rotate_y_plane_length + rotate_uv_plane_length,
                    width / 2,
                    rotatedBuffer,
                    height,
                    rotatedBuffer + rotate_y_plane_length,
                    height / 2,
                    rotatedBuffer + rotate_y_plane_length + rotate_uv_plane_length,
                    height / 2,
                    width,
                    height,
                    (libyuv::RotationMode) rotate
            );
            int tmp = width;
            width = height;
            height = tmp;
            delete[] buffer;
            return rotatedBuffer;
        }
        return buffer;
    }

    void onImageAvailable(AImage *image) {
        if (!frameTaskMap->empty()) {
            int len, width, height;
            uint8_t *data = getI420Data(image, width, height, len, frameRotation);
            if (nullptr != callback) {
                Frame *frame = new Frame(data, width, height, len);
                callback->onFrameAvailable(frame);
            } else {
                delete data;
            }
        }
        AImage_delete(image);
    }
};

#endif //IMAGESTREAMCALLBACKIMPL_H

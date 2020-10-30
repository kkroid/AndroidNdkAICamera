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
//#pragma clang diagnostic push

#include "AndroidImageReader.h"
#include <string>
#include <functional>
#include <thread>
#include <cstdlib>
#include <dirent.h>
#include <utility>
#include "native_debug.h"
#include <sstream>
#include <libyuv.h>

/**
 * MAX_BUF_COUNT:
 *   Max buffers in this AndroidImageReader.
 */
#define MAX_BUF_COUNT 4

/**
 * AndroidImageReader listener: called by AImageReader for every frame captured
 * We pass the event to AndroidImageReader class, so it could do some housekeeping
 * about
 * the loaded queue. For example, we could keep a counter to track how many
 * buffers are full and idle in the queue. If camera almost has no buffer to
 * capture
 * we could release ( skip ) some frames by AImageReader_getNextImage() and
 * AImageReader_delete().
 */
void onImageCallback(void *ctx, AImageReader *reader) {
    reinterpret_cast<AndroidImageReader *>(ctx)->imageDataCallback(reader);
}

/**
 * Constructor
 */
AndroidImageReader::AndroidImageReader(ImageFormat view, ImageStreamCallback *callback)
        : reader(nullptr) {
    imageStreamCallback = callback;
    media_status_t status = AImageReader_new(view.width, view.height, view.format, MAX_BUF_COUNT, &reader);
    ASSERT(reader && status == AMEDIA_OK, "Failed to create AImageReader")

    AImageReader_ImageListener listener{
            .context = this, .onImageAvailable = onImageCallback,
    };
    AImageReader_setImageListener(reader, &listener);
}

AndroidImageReader::~AndroidImageReader() {
    ASSERT(reader, "NULL Pointer to %s", __FUNCTION__)
    LOGI("AndroidImageReaderder:reader closed");
    AImageReader_delete(reader);
}

void AndroidImageReader::imageDataCallback(AImageReader *_reader) {
    if (!_reader) {
        LOGW("Ignore invalid image");
        return;
    }
    AImage *image = getNextImage();
    if (imageStreamCallback && image) {
        imageStreamCallback->onImageAvailable(image);
    } else {
        AImage_delete(image);
    }
}

ANativeWindow *AndroidImageReader::getNativeWindow() {
    if (!reader) return nullptr;
    ANativeWindow *nativeWindow;
    media_status_t status = AImageReader_getWindow(reader, &nativeWindow);
    ASSERT(status == AMEDIA_OK, "Could not get ANativeWindow")
    return nativeWindow;
}

/**
 * getNextImage()
 *   Retrieve the next image in AndroidImageReader's bufferQueue, NOT the last image so
 * no image is skipped. Recommended for batch/background processing.
 */
AImage *AndroidImageReader::getNextImage() {
    AImage *image;
    media_status_t status = AImageReader_acquireNextImage(reader, &image);
    if (status != AMEDIA_OK) {
        return nullptr;
    }
    return image;
}

/**
 * getLatestImage()
 *   Retrieve the last image in AndroidImageReader's bufferQueue, deleting images in
 * in front of it on the queue. Recommended for real-time processing.
 */
AImage *AndroidImageReader::getLatestImage() {
    AImage *image;
    media_status_t status = AImageReader_acquireLatestImage(reader, &image);
    if (status != AMEDIA_OK) {
        return nullptr;
    }
    return image;
}

//#pragma clang diagnostic pop
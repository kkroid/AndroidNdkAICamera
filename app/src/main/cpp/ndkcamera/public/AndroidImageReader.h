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
#ifndef CAMERA_IMAGE_READER_H
#define CAMERA_IMAGE_READER_H

#include <stdint.h>
#include <media/NdkImage.h>
#include <android/native_window.h>
#include <native_debug.h>
#include "media/NdkImageReader.h"

class ImageStreamCallback {

public:
    /**
     * Feed raw data
     * @return if handled return true, else return false;
     */
    virtual void onImageAvailable(AImage *image) = 0;

    virtual ~ImageStreamCallback() {
        LOGI("~ImageStreamCallback");
    }
};

/*
 * ImageFormat:
 *     A Data Structure to communicate resolution between camera and AndroidImageReader
 */
struct ImageFormat {
    ImageFormat(int32_t width, int32_t height, int32_t format) : width(width), height(height), format(format) {
    }

    int32_t width = 0;
    int32_t height = 0;

    int32_t format = 0;  // Through out this demo, the format is fixed to YUV_420 format
};

class AndroidImageReader {
public:
    /**
     * Ctor and Dtor()
     */
    explicit AndroidImageReader(ImageFormat view, ImageStreamCallback *imageStreamCallback);

    ~AndroidImageReader();

    /**
     * Report cached ANativeWindow, which was used to create camera's capture
     * session output.
     */
    ANativeWindow *getNativeWindow(void);

    /**
     * Retrieve Image on the top of Reader's queue
     */
    AImage *getNextImage(void);

    /**
    * Retrieve Image on the back of Reader's queue, dropping older images
    */
    AImage *getLatestImage(void);

    /**
     * AImageReader callback handler. Called by AImageReader when a frame is
     * captured
     * (Internal function, not to be called by clients)
     */
    void imageDataCallback(AImageReader *reader);

private:
    AImageReader *reader = nullptr;
    ImageStreamCallback *imageStreamCallback = nullptr;

};

#endif  // CAMERA_IMAGE_READER_H

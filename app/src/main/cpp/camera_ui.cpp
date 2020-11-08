
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

#include "CameraManager.h"
#include <jni.h>
#include <native_debug.h>
#include <dirent.h>
#include <media/NdkImage.h>
#include <string>
#include <ctpl_stl.h>
#include "UltraFaceTask.h"
#include "GenderAgeDemo.h"
#include <UIPreview.h>
#include <CVTask.h>

using namespace std;

CameraManager *cameraManager;

class DemoTask : public FrameTask {
public:
    explicit DemoTask(string name, int fps) : FrameTask(move(name), fps) {}

    void doTask(Frame *frame) override {
        long start = TimeUtil::now();
        if (fps < 3) {
            LOGI("%s is working, frameCount = %d", name.c_str(), frameCount);
        }
    }
};

char *jstringToChar(JNIEnv *env, jstring jstr) {
    char *rtn = nullptr;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("UTF-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    auto barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_openCamera(JNIEnv *env, jobject thiz, jint w, jint h, jint r, jint fps, jboolean
mirror, jstring path) {
    if (!cameraManager) {
        cameraManager = CameraManager::getInstance();
        cameraManager->frameWidth = w;
        cameraManager->frameHeight = h;
        cameraManager->frameRotation = r;
        cameraManager->cameraId = mirror ? 1 : 0;
//        FrameTask *ultraFaceTask = new UltraFaceTask(fps,
//                                                     Config::previewWidth,
//                                                     Config::previewHeight,
//                                                     2,
//                                                     0.7f);
//        cameraManager->addFrameTask(ultraFaceTask);
        FrameTask *cvTask = new CVTask(fps, jstringToChar(env, path));
        cameraManager->addFrameTask(cvTask);
    }
    cameraManager->startPreview();
}

extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    UIPreview::getInstance()->setSurface(env, surface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_setFaceListener(JNIEnv *env, jobject thiz, jobject faceListener) {
    UIPreview::getInstance()->setFaceListener(faceListener);
}

extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_addTask(JNIEnv *env, jobject thiz, jstring name, jint fps) {
    if (cameraManager) {
        char *taskName = jstringToChar(env, name);
        cameraManager->addFrameTask(new DemoTask(taskName, fps));
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_removeTask(JNIEnv *env, jobject thiz, jstring name) {
    if (cameraManager) {
        char *taskName = jstringToChar(env, name);
        cameraManager->removeFrameTask(taskName);
    }
}


extern "C" JNIEXPORT void JNICALL
Java_com_kk_afdd_MainActivity_closeCamera(JNIEnv *env, jobject thiz) {
    if (cameraManager) {
        cameraManager->stopPreview();
    }
    cameraManager = nullptr;
}

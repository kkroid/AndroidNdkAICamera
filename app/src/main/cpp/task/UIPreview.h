#ifndef UIPREVIEW_H
#define UIPREVIEW_H

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <UltraFace.hpp>

class UIPreview {
public:
    static UIPreview *getInstance() {
        static UIPreview instance;
        return &instance;
    }

    ~UIPreview() {
        env->DeleteGlobalRef(surface);
        env->DeleteGlobalRef(faceListener);
        surface = nullptr;
        faceListener = nullptr;
        delete previewJvm;
        previewJvm = nullptr;

    }

    ANativeWindow *getNativeWindow() {
        if (nullptr != surface) {
            return ANativeWindow_fromSurface(env, surface);
        }
        return nullptr;
    }

    void setSurface(JNIEnv *_env, jobject _surface) {
        env = _env;
        surface = env->NewGlobalRef(_surface);
        // 保存全局JVM以便在子线程中使用
        env->GetJavaVM(&previewJvm);
    }

    void onFaceDetected(std::string faceJson) {
        const std::lock_guard<std::mutex> lock(g_i_mutex);
        previewJvm->AttachCurrentThread(&env, nullptr);
        if (nullptr == onFaceClass || nullptr == setFaceInfoListId) {
            onFaceClass = env->GetObjectClass(faceListener);
            setFaceInfoListId = env->GetMethodID(onFaceClass, "setFaceInfoList", "(Ljava/lang/String;)V");
        }
        jstring jmsg = env->NewStringUTF(faceJson.c_str());
        env->CallVoidMethod(faceListener, setFaceInfoListId, jmsg);
        env->DeleteLocalRef(jmsg);
    }

    void setFaceListener(jobject listener) {
        faceListener = env->NewGlobalRef(listener);
    }

private:
    JNIEnv *env;
    jobject surface;
    jobject faceListener;
    JavaVM *previewJvm = nullptr;
    jclass onFaceClass;
    jmethodID setFaceInfoListId;

    std::mutex g_i_mutex;

    UIPreview() = default;
};

#endif

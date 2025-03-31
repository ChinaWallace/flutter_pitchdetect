#include <jni.h>
#include <string>
#include "PitchDetect.h"

static PitchDetect* pitchDetect = nullptr;
// 全局的 JNI 环境和回调对象
JavaVM* gJavaVM;
jobject gCallbackObj;
jmethodID gCallbackMethod;

extern "C" {

// 初始化回调
JNIEXPORT void JNICALL
Java_com_cai_pitchdetect_pitchdetect_1flutter_MainActivityKt_initCallback(JNIEnv *env, jobject thiz, jobject callback) {
    env->GetJavaVM(&gJavaVM);
    gCallbackObj = env->NewGlobalRef(callback);
    jclass callbackClass = env->GetObjectClass(gCallbackObj);
    gCallbackMethod = env->GetMethodID(callbackClass, "onStringReceived", "(Ljava/lang/String;)V");
}

JNIEXPORT void JNICALL
Java_com_cai_pitchdetect_pitchdetect_1flutter_MainActivityKt_processAudioData(JNIEnv *env, jobject thiz, jbyteArray data, jint size) {

    if (!pitchDetect) {
        pitchDetect = new PitchDetect();
        pitchDetect->startRecord();
    }

    jbyte* buffer = env->GetByteArrayElements(data, nullptr);
    pitchDetect->recvData(0, (unsigned char*)buffer, size);
    env->ReleaseByteArrayElements(data, buffer, JNI_ABORT);
}

// 发送字符串到 Kotlin
void sendStringToKotlin(const std::string& message) {
    JNIEnv* newEnv;
    bool isAttached = false;
    jint result = gJavaVM->GetEnv((void**)&newEnv, JNI_VERSION_1_6);
    if (result == JNI_EDETACHED) {
        if (gJavaVM->AttachCurrentThread(&newEnv, nullptr) == JNI_OK) {
            isAttached = true;
        }
    }

    if (newEnv && gCallbackObj && gCallbackMethod) {
        jstring jMessage = newEnv->NewStringUTF(message.c_str());
        newEnv->CallVoidMethod(gCallbackObj, gCallbackMethod, jMessage);
        newEnv->DeleteLocalRef(jMessage);
    }

    if (isAttached) {
        gJavaVM->DetachCurrentThread();
    }
}

// 释放全局引用
JNIEXPORT void JNICALL
Java_com_cai_pitchdetect_pitchdetect_1flutter_MainActivity_releaseCallback(JNIEnv *env, jobject thiz) {
    if (gCallbackObj) {
        env->DeleteGlobalRef(gCallbackObj);
        gCallbackObj = nullptr;
    }
}

}

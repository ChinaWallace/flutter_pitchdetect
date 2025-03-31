#ifndef PITCH_WRAPPER_H
#define PITCH_WRAPPER_H

#include <jni.h>
#include <string>
#include <functional>

#ifdef __cplusplus
extern "C" {
#endif

// 声明供外部 C++ 代码调用的函数
void sendStringToKotlin(const std::string& message);

JNIEXPORT void JNICALL
Java_com_cai_pitchdetect_pitchdetect_1flutter_MainActivityKt_initCallback(JNIEnv *env, jobject thiz, jobject callbackObj);

JNIEXPORT void JNICALL
Java_com_cai_pitchdetect_pitchdetect_1flutter_MainActivityKt_processAudioData(JNIEnv *env, jobject thiz, jbyteArray data, jint size);

#ifdef __cplusplus
}
#endif
#endif
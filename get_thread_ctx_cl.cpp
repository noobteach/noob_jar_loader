#pragma once

#include "jni.h"

namespace get_thread_ctx_cl {
    __forceinline inline jobject get(JNIEnv *env, jstring name) {
        env->PushLocalFrame(4);

        jclass Thread_cls = env->FindClass("java/lang/Thread");
        jmethodID getThreads_mid = env->GetStaticMethodID(Thread_cls, "getThreads", "()[Ljava/lang/Thread;");
        jfieldID name_fid = env->GetFieldID(Thread_cls, "name", "Ljava/lang/String;");
        jfieldID contextClassLoader_fid = env->GetFieldID(Thread_cls, "contextClassLoader", "Ljava/lang/ClassLoader;");

        jclass String_cls = env->FindClass("java/lang/String");
        jmethodID equals_mid = env->GetMethodID(String_cls, "equals", "(Ljava/lang/Object;)Z");

        jobjectArray threads_array = reinterpret_cast<jobjectArray>(env->CallStaticObjectMethod(Thread_cls, getThreads_mid));
        jsize thread_count = env->GetArrayLength(threads_array);

        for (jsize i = 0; i < thread_count; i++) {
            env->PushLocalFrame(4);

            jobject thread = env->GetObjectArrayElement(threads_array, i);
            jstring thread_name = reinterpret_cast<jstring>(env->GetObjectField(thread, name_fid));
            if (env->CallBooleanMethod(name, equals_mid, thread_name)) {
                jobject result = env->GetObjectField(thread, contextClassLoader_fid);
                return env->PopLocalFrame(env->PopLocalFrame(result));
            }

            env->PopLocalFrame(nullptr);
        }

        env->PopLocalFrame(nullptr);
        return nullptr;
    }
}

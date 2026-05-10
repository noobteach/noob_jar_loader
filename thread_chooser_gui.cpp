#pragma once

#include "jni.h"

#include "jni_show_io_gui.cpp"

namespace thread_chooser_gui {
    __forceinline inline jobject pop(JNIEnv *env) {
        env->PushLocalFrame(16);

        jclass Thread_cls = env->FindClass("java/lang/Thread");
        jmethodID getThreads_mid = env->GetStaticMethodID(Thread_cls, "getThreads", "()[Ljava/lang/Thread;");
        jfieldID tid_fid = env->GetFieldID(Thread_cls, "tid", "J");
        jfieldID name_fid = env->GetFieldID(Thread_cls, "name", "Ljava/lang/String;");
        jfieldID contextClassLoader_fid = env->GetFieldID(Thread_cls, "contextClassLoader", "Ljava/lang/ClassLoader;");

        jobjectArray threads_array = reinterpret_cast<jobjectArray>(env->CallStaticObjectMethod(Thread_cls, getThreads_mid));
        jsize thread_count = env->GetArrayLength(threads_array);

        jclass String_cls = env->FindClass("java/lang/String");

        jclass Class_cls = env->FindClass("java/lang/Class");
        jmethodID getName_mid = env->GetMethodID(Class_cls, "getName", "()Ljava/lang/String;");

        jclass StringBuilder_cls = env->FindClass("java/lang/StringBuilder");
        jmethodID StringBuilder_StringBuilder_mid = env->GetMethodID(StringBuilder_cls, "<init>", "()V");
        jmethodID append_String_mid = env->GetMethodID(StringBuilder_cls, "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;");
        jmethodID append_Long_mid = env->GetMethodID(StringBuilder_cls, "append", "(J)Ljava/lang/StringBuilder;");
        jmethodID toString_mid = env->GetMethodID(StringBuilder_cls, "toString", "()Ljava/lang/String;");

        jstring separator = env->NewStringUTF("----");
        jobjectArray options = env->NewObjectArray(thread_count, String_cls, nullptr);

        for (jsize i = 0; i < thread_count; i++) {
            env->PushLocalFrame(8);

            jobject thread = env->GetObjectArrayElement(threads_array, i);
            jobject sb = env->NewObject(StringBuilder_cls, StringBuilder_StringBuilder_mid);

            jlong tid = env->GetLongField(thread, tid_fid);
            env->CallObjectMethod(sb, append_Long_mid, tid);
            env->CallObjectMethod(sb, append_String_mid, separator);

            jstring thread_name = reinterpret_cast<jstring>(env->GetObjectField(thread, name_fid));
            env->CallObjectMethod(sb, append_String_mid, thread_name);
            env->CallObjectMethod(sb, append_String_mid, separator);

            jobject tccl = env->GetObjectField(thread, contextClassLoader_fid);
            if (tccl != nullptr) {
                jclass tccl_class = env->GetObjectClass(tccl);
                jstring class_name = reinterpret_cast<jstring>(env->CallObjectMethod(tccl_class, getName_mid));
                env->CallObjectMethod(sb, append_String_mid, class_name);
            } else {
                env->CallObjectMethod(sb, append_String_mid, env->NewStringUTF("null"));
            }

            jstring option_str = reinterpret_cast<jstring>(env->CallObjectMethod(sb, toString_mid));
            env->SetObjectArrayElement(options, i, option_str);

            env->PopLocalFrame(nullptr);
        }

        jstring prompt = env->NewStringUTF("choose a thread");
        jobject selected = jni_show_io_gui::choicebox(env, prompt, options);

        if (!selected) {
            env->PopLocalFrame(nullptr);
            return nullptr;
        }

        for (jsize i = 0; i < thread_count; i++) {
            jstring option_str = reinterpret_cast<jstring>(env->GetObjectArrayElement(options, i));
            if (env->IsSameObject(selected, option_str)) {
                jobject thread = env->GetObjectArrayElement(threads_array, i);
                jobject result = env->GetObjectField(thread, contextClassLoader_fid);
                return env->PopLocalFrame(result);
            }
        }

        env->PopLocalFrame(nullptr);
        return nullptr;
    }
}

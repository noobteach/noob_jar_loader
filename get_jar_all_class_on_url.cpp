#pragma once

#include <cstddef>

#include "jni.h"

#include "config.hpp"
#include "jni_show_io_gui.cpp"

namespace get_jar_all_class_on_url {
    inline std::byte pool[MAX_JAR_SIZE_128MB]{};
    inline size_t start[MAX_CLASS_COUNT_4096]{};
    inline size_t end[MAX_CLASS_COUNT_4096]{};

    __forceinline inline bool get(JNIEnv *env, jstring jar_url_str) {
        env->PushLocalFrame(16);

        jclass URL_cls = env->FindClass("java/net/URL");
        jclass ZipInputStream_cls = env->FindClass("java/util/zip/ZipInputStream");
        jclass ZipEntry_cls = env->FindClass("java/util/zip/ZipEntry");
        jclass String_cls = env->FindClass("java/lang/String");

        jmethodID URL_init_mid = env->GetMethodID(URL_cls, "<init>", "(Ljava/lang/String;)V");
        jmethodID openStream_mid = env->GetMethodID(URL_cls, "openStream", "()Ljava/io/InputStream;");
        jmethodID ZipInputStream_init_mid = env->GetMethodID(ZipInputStream_cls, "<init>", "(Ljava/io/InputStream;)V");
        jmethodID getNextEntry_mid = env->GetMethodID(ZipInputStream_cls, "getNextEntry", "()Ljava/util/zip/ZipEntry;");
        jmethodID getName_mid = env->GetMethodID(ZipEntry_cls, "getName", "()Ljava/lang/String;");
        jmethodID read_mid = env->GetMethodID(ZipInputStream_cls, "read", "()I");
        jmethodID endsWith_mid = env->GetMethodID(String_cls, "endsWith", "(Ljava/lang/String;)Z");

        jstring str_dot_class = env->NewStringUTF(".class");
        jstring str_module_info_dot_class = env->NewStringUTF("module-info.class");

        jobject url_obj = env->NewObject(URL_cls, URL_init_mid, jar_url_str);
        if (jni_show_io_gui::err_check_show_drop(env)) {
            env->PopLocalFrame(nullptr);
            return false;
        }

        jobject is_obj = env->CallObjectMethod(url_obj, openStream_mid);
        if (jni_show_io_gui::err_check_show_drop(env)) {
            env->PopLocalFrame(nullptr);
            return false;
        }

        jobject zis_obj = env->NewObject(ZipInputStream_cls, ZipInputStream_init_mid, is_obj);
        if (jni_show_io_gui::err_check_show_drop(env)) {
            env->PopLocalFrame(nullptr);
            return false;
        }

        size_t data_in_pool = 0, index_in_pool = 0;
        while (true) {
            env->PushLocalFrame(16);

            jobject entry_obj = env->CallObjectMethod(zis_obj, getNextEntry_mid);
            if (jni_show_io_gui::err_check_show_drop(env)) {
                env->PopLocalFrame(nullptr);
                env->PopLocalFrame(nullptr);
                return false;
            }
            if (!entry_obj) {
                env->PopLocalFrame(nullptr);
                break;
            }

            jstring entry_name_str = reinterpret_cast<jstring>(env->CallObjectMethod(entry_obj, getName_mid));
            if (jni_show_io_gui::err_check_show_drop(env)) {
                env->PopLocalFrame(nullptr);
                env->PopLocalFrame(nullptr);
                return false;
            }
            if (!entry_name_str) {
                env->PopLocalFrame(nullptr);
                break;
            }

            if (env->CallBooleanMethod(entry_name_str, endsWith_mid, str_dot_class) && !env->CallBooleanMethod(entry_name_str, endsWith_mid, str_module_info_dot_class)) {
                start[index_in_pool] = data_in_pool;

                while (true) {
                    jint b = env->CallIntMethod(zis_obj, read_mid);
                    if (jni_show_io_gui::err_check_show_drop(env)) {
                        env->PopLocalFrame(nullptr);
                        env->PopLocalFrame(nullptr);
                        return false;
                    }
                    if (b < 0) break;

                    if (data_in_pool >= MAX_JAR_SIZE_128MB) {
                        env->PopLocalFrame(nullptr);
                        env->PopLocalFrame(nullptr);
                        return false;
                    }
                    pool[data_in_pool] = static_cast<std::byte>(b);
                    data_in_pool++;
                }

                if (index_in_pool >= MAX_CLASS_COUNT_4096) {
                    env->PopLocalFrame(nullptr);
                    env->PopLocalFrame(nullptr);
                    return false;
                }
                end[index_in_pool] = data_in_pool;
                index_in_pool++;
            }
            env->PopLocalFrame(nullptr);
        }
        env->PopLocalFrame(nullptr);
        return true;
    }
}

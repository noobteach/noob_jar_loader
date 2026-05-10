#pragma once

#include <cstring>

#include "jni.h"

#include "config.hpp"
#include "get_jar_all_class_on_url.cpp"

namespace resolve_class_depends {
    struct class_depends {
        jstring class_name;
        jstring super_name;
        size_t iface_start_idx;
        size_t iface_end_idx;
    };

    inline jstring iface_pool[MAX_IFACE_COUNT_8192]{};
    inline size_t iface_pool_start_idx = 0;
    inline size_t iface_pool_end_idx = 0;

    __forceinline inline bool is_superclass_ready(JNIEnv *env, class_depends *deps_cache, size_t i, bool *ready, size_t loaded_count, jmethodID equals_mid) {
        for (size_t j = 0; j < loaded_count; j++) {
            if (i == j) continue;
            if (env->CallBooleanMethod(deps_cache[j].class_name, equals_mid, deps_cache[i].super_name)) {
                return ready[j];
            }
        }
        return true;
    }

    __forceinline inline bool is_interfaces_ready(JNIEnv *env, class_depends *deps_cache, size_t i, bool *ready, size_t loaded_count, jmethodID equals_mid) {
        for (size_t k = deps_cache[i].iface_start_idx; k < deps_cache[i].iface_end_idx; k++) {
            for (size_t j = 0; j < loaded_count; j++) {
                if (i == j) continue;
                if (env->CallBooleanMethod(deps_cache[j].class_name, equals_mid, iface_pool[k])) {
                    if (ready[j]) break;
                    return false;
                }
            }
        }
        return true;
    }

    __forceinline inline void sort_deps(JNIEnv *env, class_depends *deps_cache, size_t *loading_order, size_t loaded_count, jmethodID equals_mid) {
        size_t loading_order_cursor = 0;
        bool ready[MAX_CLASS_COUNT_4096]{};

        for (size_t k = 0; k < loaded_count; k++) {
            for (size_t i = 0; i < loaded_count; i++) {
                if (ready[i]) continue;
                if (is_superclass_ready(env, deps_cache, i, ready, loaded_count, equals_mid) && is_interfaces_ready(env, deps_cache, i, ready, loaded_count, equals_mid)) {
                    loading_order[loading_order_cursor++] = i;
                    ready[i] = true;
                    break;
                }
            }
        }
        for (size_t i = 0; i < loaded_count; i++)
            if (!ready[i]) loading_order[loading_order_cursor++] = i;
    }

    __forceinline inline bool super_sorter_8to24(JNIEnv *env, size_t *loading_order, size_t loaded_count) {
        env->PushLocalFrame(LOCREF_CAP_FOR_DEPS_SORTING);

        jclass ClassReader_cls = env->FindClass("jdk/internal/org/objectweb/asm/ClassReader");
        jmethodID ClassReader_init_mid = env->GetMethodID(ClassReader_cls, "<init>", "([B)V");
        jmethodID getClassName_mid = env->GetMethodID(ClassReader_cls, "getClassName", "()Ljava/lang/String;");
        jmethodID getSuperName_mid = env->GetMethodID(ClassReader_cls, "getSuperName", "()Ljava/lang/String;");
        jmethodID getInterfaces_mid = env->GetMethodID(ClassReader_cls, "getInterfaces", "()[Ljava/lang/String;");

        jclass String_cls = env->FindClass("java/lang/String");
        jmethodID equals_mid = env->GetMethodID(String_cls, "equals", "(Ljava/lang/Object;)Z");

        class_depends deps_cache[MAX_CLASS_COUNT_4096]{};

        for (size_t i = 0; i < loaded_count; i++) {
            jsize class_len = static_cast<jsize>(get_jar_all_class_on_url::end[i] - get_jar_all_class_on_url::start[i]);
            jbyteArray class_bytes = env->NewByteArray(class_len);
            env->SetByteArrayRegion(class_bytes, 0, class_len, reinterpret_cast<jbyte *>(&get_jar_all_class_on_url::pool[get_jar_all_class_on_url::start[i]]));

            jobject asm_class_reader = env->NewObject(ClassReader_cls, ClassReader_init_mid, class_bytes);
            env->DeleteLocalRef(class_bytes);

            deps_cache[i].class_name = reinterpret_cast<jstring>(env->CallObjectMethod(asm_class_reader, getClassName_mid));
            deps_cache[i].super_name = reinterpret_cast<jstring>(env->CallObjectMethod(asm_class_reader, getSuperName_mid));

            jobjectArray interfaces_array = reinterpret_cast<jobjectArray>(env->CallObjectMethod(asm_class_reader, getInterfaces_mid));
            env->DeleteLocalRef(asm_class_reader);

            jsize interfaces_count = interfaces_array ? env->GetArrayLength(interfaces_array) : 0;

            deps_cache[i].iface_start_idx = iface_pool_end_idx;
            for (jsize k = 0; k < interfaces_count; k++) {
                if (iface_pool_end_idx >= iface_pool_start_idx + MAX_IFACE_COUNT_8192) {
                    env->PopLocalFrame(nullptr);
                    return false;
                }
                iface_pool[iface_pool_end_idx] = reinterpret_cast<jstring>(env->GetObjectArrayElement(interfaces_array, k));
                iface_pool_end_idx++;
            }
            deps_cache[i].iface_end_idx = iface_pool_end_idx;

            env->DeleteLocalRef(interfaces_array);
        }

        sort_deps(env, deps_cache, loading_order, loaded_count, equals_mid);

        env->PopLocalFrame(nullptr);
        return true;
    }

    __forceinline inline bool super_sorter_25(JNIEnv *env, size_t *loading_order, size_t loaded_count) {
        env->PushLocalFrame(LOCREF_CAP_FOR_DEPS_SORTING);

        jclass ClassFile_cls = env->FindClass("java/lang/classfile/ClassFile");
        jmethodID of_mid = env->GetStaticMethodID(ClassFile_cls, "of", "()Ljava/lang/classfile/ClassFile;");

        jclass ClassModel_cls = env->FindClass("java/lang/classfile/ClassModel");
        jmethodID parse_mid = env->GetMethodID(ClassFile_cls, "parse", "([B)Ljava/lang/classfile/ClassModel;");
        jmethodID thisClass_mid = env->GetMethodID(ClassModel_cls, "thisClass", "()Ljava/lang/classfile/constantpool/ClassEntry;");
        jmethodID superclass_mid = env->GetMethodID(ClassModel_cls, "superclass", "()Ljava/util/Optional;");
        jmethodID interfaces_mid = env->GetMethodID(ClassModel_cls, "interfaces", "()Ljava/util/List;");

        jclass ClassEntry_cls = env->FindClass("java/lang/classfile/constantpool/ClassEntry");
        jmethodID asInternalName_mid = env->GetMethodID(ClassEntry_cls, "asInternalName", "()Ljava/lang/String;");

        jclass Optional_cls = env->FindClass("java/util/Optional");
        jmethodID isPresent_mid = env->GetMethodID(Optional_cls, "isPresent", "()Z");
        jmethodID Optional_get_mid = env->GetMethodID(Optional_cls, "get", "()Ljava/lang/Object;");

        jclass List_cls = env->FindClass("java/util/List");
        jmethodID List_size_mid = env->GetMethodID(List_cls, "size", "()I");
        jmethodID List_get_mid = env->GetMethodID(List_cls, "get", "(I)Ljava/lang/Object;");

        jclass String_cls = env->FindClass("java/lang/String");
        jmethodID equals_mid = env->GetMethodID(String_cls, "equals", "(Ljava/lang/Object;)Z");

        jobject asm_classFile_obj = env->CallStaticObjectMethod(ClassFile_cls, of_mid);

        class_depends deps_cache[MAX_CLASS_COUNT_4096]{};

        for (size_t i = 0; i < loaded_count; i++) {
            jsize class_len = static_cast<jsize>(get_jar_all_class_on_url::end[i] - get_jar_all_class_on_url::start[i]);
            jbyteArray class_bytes = env->NewByteArray(class_len);
            env->SetByteArrayRegion(class_bytes, 0, class_len, reinterpret_cast<jbyte *>(&get_jar_all_class_on_url::pool[get_jar_all_class_on_url::start[i]]));

            jobject asm_classModel = env->CallObjectMethod(asm_classFile_obj, parse_mid, class_bytes);
            env->DeleteLocalRef(class_bytes);

            jobject class_name_entry = env->CallObjectMethod(asm_classModel, thisClass_mid);
            deps_cache[i].class_name = reinterpret_cast<jstring>(env->CallObjectMethod(class_name_entry, asInternalName_mid));
            env->DeleteLocalRef(class_name_entry);

            jobject super_name_optional = env->CallObjectMethod(asm_classModel, superclass_mid);
            if (env->CallBooleanMethod(super_name_optional, isPresent_mid)) {
                jobject super_name_entry = env->CallObjectMethod(super_name_optional, Optional_get_mid);
                env->DeleteLocalRef(super_name_optional);
                deps_cache[i].super_name = reinterpret_cast<jstring>(env->CallObjectMethod(super_name_entry, asInternalName_mid));
                env->DeleteLocalRef(super_name_entry);
            } else {
                env->DeleteLocalRef(super_name_optional);
            }

            jobject interfaces_list = env->CallObjectMethod(asm_classModel, interfaces_mid);
            env->DeleteLocalRef(asm_classModel);

            jint interfaces_count = env->CallIntMethod(interfaces_list, List_size_mid);

            deps_cache[i].iface_start_idx = iface_pool_end_idx;
            for (jsize k = 0; k < interfaces_count; k++) {
                jobject interface_name_entry = env->CallObjectMethod(interfaces_list, List_get_mid, k);

                if (iface_pool_end_idx >= iface_pool_start_idx + MAX_IFACE_COUNT_8192) {
                    env->PopLocalFrame(nullptr);
                    return false;
                }
                iface_pool[iface_pool_end_idx] = reinterpret_cast<jstring>(env->CallObjectMethod(interface_name_entry, asInternalName_mid));
                iface_pool_end_idx++;

                env->DeleteLocalRef(interface_name_entry);
            }
            deps_cache[i].iface_end_idx = iface_pool_end_idx;

            env->DeleteLocalRef(interfaces_list);
        }

        sort_deps(env, deps_cache, loading_order, loaded_count, equals_mid);

        env->PopLocalFrame(nullptr);
        return true;
    }
}

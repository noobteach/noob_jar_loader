#pragma once

#include <cstdint>

#include "jni.h"

#include "config.hpp"
#include "get_jar_all_class_on_url.cpp"
#include "get_thread_ctx_cl.cpp"
#include "jni_show_io_gui.cpp"
#include "resolve_class_depends.cpp"
#include "thread_chooser_gui.cpp"

extern "C" void special_entry_point() {
    JavaVM *jvm{};
    jsize vm_count{};
    JNI_GetCreatedJavaVMs(&jvm, 1, &vm_count);

    JNIEnv *env{};
    jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr);

    env->PushLocalFrame(32);

    jclass Long_cls = env->FindClass("java/lang/Long");
    jmethodID Long_toString_mid = env->GetStaticMethodID(Long_cls, "toString", "(J)Ljava/lang/String;");

    jclass String_cls = env->FindClass("java/lang/String");
    jmethodID concat_mid = env->GetMethodID(String_cls, "concat", "(Ljava/lang/String;)Ljava/lang/String;");

    jclass Class_cls = env->FindClass("java/lang/Class");
    jmethodID forName_mid = env->GetStaticMethodID(Class_cls, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");

    jni_show_io_gui::headless(env, false);

    jstring jar_url_str = jni_show_io_gui::inputbox(env, env->NewStringUTF(
                                                        "type any legal url path string to load jar\n"
                                                        "such as http://127.0.0.1:12345/1.jar\n"
                                                        "or https://some.web.com/1.jar\n"
                                                        "or file:///C:/some_folder/1.jar\n"));

    if (!get_jar_all_class_on_url::get(env, jar_url_str)) {
        jni_show_io_gui::msgbox(env, env->NewStringUTF("get_jar_all_class_on_url FAIL!"));
        env->PopLocalFrame(nullptr);
        jvm->DetachCurrentThread();
        return;
    }

    size_t loaded_count = 0;
    for (size_t i = 0; i < MAX_CLASS_COUNT_4096; i++) {
        if (get_jar_all_class_on_url::end[i] - get_jar_all_class_on_url::start[i] > 0)
            loaded_count++;
    }

    jstring counter_str = reinterpret_cast<jstring>(env->CallStaticObjectMethod(Long_cls, Long_toString_mid, loaded_count));
    jstring counter_msg = reinterpret_cast<jstring>(env->CallObjectMethod(counter_str, concat_mid, env->NewStringUTF(" classes loaded")));

    jni_show_io_gui::msgbox(env, counter_msg);

    jobject game_cl = get_thread_ctx_cl::get(env, env->NewStringUTF("Render thread")); //>=1.15
    if (!game_cl) game_cl = get_thread_ctx_cl::get(env, env->NewStringUTF("Client thread")); //<=1.14.4
    if (!game_cl) game_cl = get_thread_ctx_cl::get(env, env->NewStringUTF("Minecraft main thread")); //<=1.6.4
    if (!game_cl) game_cl = thread_chooser_gui::pop(env);
    if (!game_cl)
        jni_show_io_gui::msgbox(env, env->NewStringUTF("you cancelled\n"
                                    "will use Bootstrap Classloader\n"));

    size_t loading_order[MAX_CLASS_COUNT_4096]{};

    jobject jdk25 = env->FindClass("java/lang/classfile/ClassFile"); //jdk25 use new api
    env->ExceptionClear();
    jobject jdk8to24 = env->FindClass("jdk/internal/org/objectweb/asm/ClassReader"); //jdk8~24 embed org asm
    env->ExceptionClear();

    bool deps_ok = false;
    if (jdk25) {
        deps_ok = resolve_class_depends::super_sorter_25(env, loading_order, loaded_count);
    } else if (jdk8to24) {
        deps_ok = resolve_class_depends::super_sorter_8to24(env, loading_order, loaded_count);
    } else {
        jni_show_io_gui::msgbox(env, env->NewStringUTF("cant find jdk embed asm framework!"));
        env->PopLocalFrame(nullptr);
        jvm->DetachCurrentThread();
        return;
    }
    if (!deps_ok) {
        jni_show_io_gui::msgbox(env, env->NewStringUTF("resolve_class_deps FAIL!"));
        env->PopLocalFrame(nullptr);
        jvm->DetachCurrentThread();
        return;
    }

    for (size_t j = 0; j < loaded_count; j++) {
        jbyte *address = reinterpret_cast<jbyte *>(&get_jar_all_class_on_url::pool[get_jar_all_class_on_url::start[loading_order[j]]]);
        jsize len = static_cast<jsize>(get_jar_all_class_on_url::end[loading_order[j]] - get_jar_all_class_on_url::start[loading_order[j]]);

        env->DeleteLocalRef(env->DefineClass(nullptr, game_cl, address, len));
        jni_show_io_gui::err_check_show_drop(env);
    }

    jstring entry_class_name = jni_show_io_gui::inputbox(env, env->NewStringUTF(
                                                             "type your entry class's full name\n"
                                                             "such as: com.example.SomeClass\n"
                                                             "use . replace /\n"
                                                             "note: its only <clinit> will be called\n"));

    env->CallStaticObjectMethod(Class_cls, forName_mid, entry_class_name, JNI_TRUE, game_cl);
    jni_show_io_gui::err_check_show_drop(env);

    env->PopLocalFrame(nullptr);
    jvm->DetachCurrentThread();
}

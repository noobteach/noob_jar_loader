#pragma once

#include "jni.h"

namespace jni_show_io_gui {
    __forceinline inline void headless(JNIEnv *env, bool state) {
        env->PushLocalFrame(4);

        jclass GraphicsEnvironment_cls = env->FindClass("java/awt/GraphicsEnvironment");
        jfieldID headless_fid = env->GetStaticFieldID(GraphicsEnvironment_cls, "headless", "Ljava/lang/Boolean;");

        jclass Boolean_cls = env->FindClass("java/lang/Boolean");
        jfieldID TRUE_fid = env->GetStaticFieldID(Boolean_cls, "TRUE", "Ljava/lang/Boolean;");
        jfieldID FALSE_fid = env->GetStaticFieldID(Boolean_cls, "FALSE", "Ljava/lang/Boolean;");
        jobject JBoolean_true = env->GetStaticObjectField(Boolean_cls, TRUE_fid);
        jobject JBoolean_false = env->GetStaticObjectField(Boolean_cls, FALSE_fid);

        if (state)
            env->SetStaticObjectField(GraphicsEnvironment_cls, headless_fid, JBoolean_true);
        else
            env->SetStaticObjectField(GraphicsEnvironment_cls, headless_fid, JBoolean_false);

        env->PopLocalFrame(nullptr);
    }

    __forceinline inline void msgbox(JNIEnv *env, jstring msg) {
        env->PushLocalFrame(4);

        jclass JOptionPane_cls = env->FindClass("javax/swing/JOptionPane");
        jmethodID showMessageDialog_mid = env->GetStaticMethodID(JOptionPane_cls, "showMessageDialog", "(Ljava/awt/Component;Ljava/lang/Object;)V");

        env->CallStaticVoidMethod(JOptionPane_cls, showMessageDialog_mid, nullptr, msg);

        env->PopLocalFrame(nullptr);
    }

    __forceinline inline jstring inputbox(JNIEnv *env, jstring msg) {
        env->PushLocalFrame(4);

        jclass JOptionPane_cls = env->FindClass("javax/swing/JOptionPane");
        jmethodID showInputDialog_mid = env->GetStaticMethodID(JOptionPane_cls, "showInputDialog", "(Ljava/lang/Object;)Ljava/lang/String;");

        jstring result = reinterpret_cast<jstring>(env->CallStaticObjectMethod(JOptionPane_cls, showInputDialog_mid, msg));

        return reinterpret_cast<jstring>(env->PopLocalFrame(result));
    }

    __forceinline inline jobject choicebox(JNIEnv *env, jstring msg, jobjectArray options) {
        env->PushLocalFrame(4);

        jclass JOptionPane_cls = env->FindClass("javax/swing/JOptionPane");
        jmethodID showInputDialog_mid = env->GetStaticMethodID(JOptionPane_cls, "showInputDialog",
                                                               "(Ljava/awt/Component;Ljava/lang/Object;Ljava/lang/String;ILjavax/swing/Icon;[Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        jobject result = env->CallStaticObjectMethod(JOptionPane_cls, showInputDialog_mid, nullptr, msg, nullptr, nullptr, nullptr, options, nullptr);

        return env->PopLocalFrame(result);
    }

    __forceinline inline bool err_check_show_drop(JNIEnv *env) {
        env->PushLocalFrame(4);

        if (jthrowable err = env->ExceptionOccurred()) {
            env->ExceptionClear();

            jclass Throwable_cls = env->FindClass("java/lang/Throwable");
            jmethodID toString_mid = env->GetMethodID(Throwable_cls, "toString", "()Ljava/lang/String;");

            jstring err_str = reinterpret_cast<jstring>(env->CallObjectMethod(err, toString_mid));
            msgbox(env, err_str);

            env->PopLocalFrame(nullptr);
            return true;
        }

        env->PopLocalFrame(nullptr);
        return false;
    }
}

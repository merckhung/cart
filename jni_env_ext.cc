/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <stdio.h>

#include "jni.h"
#include "jni_env_ext.h"

#include "utils.h"
#include "oat.h"
#include "elf.h"
#include "heap.h"
#include "hash.h"
#include "zip.h"
#include "debugger.h"
#include "cart.h"
#include "class.h"

namespace cart {

class JNI {
 public:
  static jint GetVersion(JNIEnv*) {
    printf("%s\n", __func__);
    return JNI_VERSION_1_6;
  }

  static jclass DefineClass(JNIEnv*, const char*, jobject, const jbyte*, jsize) {
    printf("%s\n", __func__);
    return 0;
  }

  static jclass FindClass(JNIEnv* env, const char* name) {
    printf("%s\n", __func__);
    printf("FindClass = \"%s\"\n", name);
    JNIEnvExt* pEnv = reinterpret_cast<JNIEnvExt*>(env);
    Class_t* pClass = ClFindClass(pEnv->GetClassLinker(), (const uint8_t*)name);
    if (pClass == NULL) {
      pdbg("Cannot find class: \"%s\"\n", name);
    } else {
      pdbg("Found class: \"%s\", %p\n", name, pClass);
    }
    return (jclass)pClass;
  }

  static jmethodID FromReflectedMethod(JNIEnv* env, jobject jlr_method) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfieldID FromReflectedField(JNIEnv* env, jobject jlr_field) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject ToReflectedMethod(JNIEnv* env, jclass, jmethodID mid, jboolean) {
    printf("%s\n", __func__);
    jobject my = new _jobject();
    return my;
  }

  static jobject ToReflectedField(JNIEnv* env, jclass, jfieldID fid, jboolean) {
    printf("%s\n", __func__);
    return 0;
  }

  static jclass GetObjectClass(JNIEnv* env, jobject java_object) {
    printf("%s\n", __func__);
    return 0;
  }

  static jclass GetSuperclass(JNIEnv* env, jclass java_class) {
    printf("%s\n", __func__);
    return 0;
  }

  static jboolean IsAssignableFrom(JNIEnv* env, jclass java_class1, jclass java_class2) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jboolean IsInstanceOf(JNIEnv* env, jobject jobj, jclass java_class) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jint Throw(JNIEnv* env, jthrowable java_exception) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint ThrowNew(JNIEnv* env, jclass c, const char* msg) {
    printf("%s\n", __func__);
    return 0;
  }

  static jboolean ExceptionCheck(JNIEnv* env) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static void ExceptionClear(JNIEnv* env) {
    printf("%s\n", __func__);
  }

  static void ExceptionDescribe(JNIEnv* env) {
    printf("%s\n", __func__);
  }

  static jthrowable ExceptionOccurred(JNIEnv* env) {
    return 0;
  }

  static void FatalError(JNIEnv*, const char* msg) {
    printf("%s\n", __func__);
  }

  static jint PushLocalFrame(JNIEnv* env, jint capacity) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jobject PopLocalFrame(JNIEnv* env, jobject java_survivor) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint EnsureLocalCapacity(JNIEnv* env, jint desired_capacity) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject NewGlobalRef(JNIEnv* env, jobject obj) {
    printf("%s\n", __func__);
    return 0;
  }

  static void DeleteGlobalRef(JNIEnv* env, jobject obj) {
    printf("%s\n", __func__);
  }

  static jweak NewWeakGlobalRef(JNIEnv* env, jobject obj) {
    printf("%s\n", __func__);
    return 0;
  }

  static void DeleteWeakGlobalRef(JNIEnv* env, jweak obj) {
    printf("%s\n", __func__);
  }

  static jobject NewLocalRef(JNIEnv* env, jobject obj) {
    printf("%s\n", __func__);
    return 0;
  }

  static void DeleteLocalRef(JNIEnv* env, jobject obj) {
    printf("%s\n", __func__);
  }

  static jboolean IsSameObject(JNIEnv* env, jobject obj1, jobject obj2) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jobject AllocObject(JNIEnv* env, jclass java_class) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject NewObject(JNIEnv* env, jclass java_class, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject NewObjectV(JNIEnv* env, jclass java_class, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject NewObjectA(JNIEnv* env, jclass java_class, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jmethodID GetMethodID(JNIEnv* env, jclass java_class, const char* name, const char* sig) {
    printf("%s\n", __func__);
    jobject my = new _jobject();
    return reinterpret_cast<jmethodID>(my);
  }

  static jmethodID GetStaticMethodID(JNIEnv* env, jclass java_class, const char* name,
                                     const char* sig) {
    printf("%s\n", __func__);
    jobject my = new _jobject();
    return reinterpret_cast<jmethodID>(my);
  }

  static jobject CallObjectMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallObjectMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallObjectMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jboolean CallBooleanMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return JNI_TRUE;
  }

  static jboolean CallBooleanMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return JNI_TRUE;
  }

  static jboolean CallBooleanMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return JNI_TRUE;
  }

  static jbyte CallByteMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte CallByteMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte CallByteMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallCharMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallCharMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallCharMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallDoubleMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallDoubleMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallDoubleMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallFloatMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallFloatMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallFloatMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallIntMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallIntMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0x0001;
  }

  static jint CallIntMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallLongMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallLongMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallLongMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallShortMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallShortMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallShortMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static void CallVoidMethod(JNIEnv* env, jobject obj, jmethodID mid, ...) {
    printf("%s\n", __func__);
  }

  static void CallVoidMethodV(JNIEnv* env, jobject obj, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
  }

  static void CallVoidMethodA(JNIEnv* env, jobject obj, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
  }

  static jobject CallNonvirtualObjectMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallNonvirtualObjectMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                               va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallNonvirtualObjectMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                               jvalue* args) {
    return 0;
  }

  static jboolean CallNonvirtualBooleanMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                                ...) {
    return JNI_FALSE;
  }

  static jboolean CallNonvirtualBooleanMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                                 va_list args) {
    return JNI_FALSE;
  }

  static jboolean CallNonvirtualBooleanMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                               jvalue* args) {
    return JNI_FALSE;
  }

  static jbyte CallNonvirtualByteMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    return 0;
  }

  static jbyte CallNonvirtualByteMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                         va_list args) {
    return 0;
  }

  static jbyte CallNonvirtualByteMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                         jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallNonvirtualCharMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallNonvirtualCharMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                           va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallNonvirtualCharMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                           jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallNonvirtualShortMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallNonvirtualShortMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                           va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallNonvirtualShortMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                             jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallNonvirtualIntMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallNonvirtualIntMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                         va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallNonvirtualIntMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                         jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallNonvirtualLongMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    return 0;
  }

  static jlong CallNonvirtualLongMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                           va_list args) {
    return 0;
  }

  static jlong CallNonvirtualLongMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                           jvalue* args) {
    return 0;
  }

  static jfloat CallNonvirtualFloatMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    return 0;
  }

  static jfloat CallNonvirtualFloatMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                             va_list args) {
    return 0;
  }

  static jfloat CallNonvirtualFloatMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                             jvalue* args) {
    return 0;
  }

  static jdouble CallNonvirtualDoubleMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
    return 0;
  }

  static jdouble CallNonvirtualDoubleMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                               va_list args) {
    return 0;
  }

  static jdouble CallNonvirtualDoubleMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                               jvalue* args) {
    return 0;
  }

  static void CallNonvirtualVoidMethod(JNIEnv* env, jobject obj, jclass, jmethodID mid, ...) {
  }

  static void CallNonvirtualVoidMethodV(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                          va_list args) {
  }

  static void CallNonvirtualVoidMethodA(JNIEnv* env, jobject obj, jclass, jmethodID mid,
                                          jvalue* args) {
  }

  static jfieldID GetFieldID(JNIEnv* env, jclass java_class, const char* name, const char* sig) {
    return 0;
  }

  static jfieldID GetStaticFieldID(JNIEnv* env, jclass java_class, const char* name,
                                     const char* sig) {
    return 0;
  }

  static jobject GetObjectField(JNIEnv* env, jobject obj, jfieldID fid) {
    return 0;
  }

  static jobject GetStaticObjectField(JNIEnv* env, jclass, jfieldID fid) {
    return 0;
  }

  static void SetObjectField(JNIEnv* env, jobject java_object, jfieldID fid, jobject java_value) {
    printf("%s\n", __func__);
  }

  static void SetStaticObjectField(JNIEnv* env, jclass, jfieldID fid, jobject java_value) {
    printf("%s\n", __func__);
  }

  static jboolean GetBooleanField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte GetByteField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar GetCharField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort GetShortField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint GetIntField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong GetLongField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat GetFloatField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble GetDoubleField(JNIEnv* env, jobject obj, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jboolean GetStaticBooleanField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jbyte GetStaticByteField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar GetStaticCharField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort GetStaticShortField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint GetStaticIntField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong GetStaticLongField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat GetStaticFloatField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble GetStaticDoubleField(JNIEnv* env, jclass, jfieldID fid) {
    printf("%s\n", __func__);
    return 0;
  }

  static void SetBooleanField(JNIEnv* env, jobject obj, jfieldID fid, jboolean v) {
    printf("%s\n", __func__);
  }

  static void SetByteField(JNIEnv* env, jobject obj, jfieldID fid, jbyte v) {
    printf("%s\n", __func__);
  }

  static void SetCharField(JNIEnv* env, jobject obj, jfieldID fid, jchar v) {
    printf("%s\n", __func__);
  }

  static void SetFloatField(JNIEnv* env, jobject obj, jfieldID fid, jfloat v) {
    printf("%s\n", __func__);
  }

  static void SetDoubleField(JNIEnv* env, jobject obj, jfieldID fid, jdouble v) {
    printf("%s\n", __func__);
  }

  static void SetIntField(JNIEnv* env, jobject obj, jfieldID fid, jint v) {
    printf("%s\n", __func__);
  }

  static void SetLongField(JNIEnv* env, jobject obj, jfieldID fid, jlong v) {
    printf("%s\n", __func__);
  }

  static void SetShortField(JNIEnv* env, jobject obj, jfieldID fid, jshort v) {
    printf("%s\n", __func__);
  }

  static void SetStaticBooleanField(JNIEnv* env, jclass, jfieldID fid, jboolean v) {
    printf("%s\n", __func__);
  }

  static void SetStaticByteField(JNIEnv* env, jclass, jfieldID fid, jbyte v) {
    printf("%s\n", __func__);
  }

  static void SetStaticCharField(JNIEnv* env, jclass, jfieldID fid, jchar v) {
    printf("%s\n", __func__);
  }

  static void SetStaticFloatField(JNIEnv* env, jclass, jfieldID fid, jfloat v) {
    printf("%s\n", __func__);
  }

  static void SetStaticDoubleField(JNIEnv* env, jclass, jfieldID fid, jdouble v) {
    printf("%s\n", __func__);
  }

  static void SetStaticIntField(JNIEnv* env, jclass, jfieldID fid, jint v) {
    printf("%s\n", __func__);
  }

  static void SetStaticLongField(JNIEnv* env, jclass, jfieldID fid, jlong v) {
    printf("%s\n", __func__);
  }

  static void SetStaticShortField(JNIEnv* env, jclass, jfieldID fid, jshort v) {
    printf("%s\n", __func__);
  }

  static jobject CallStaticObjectMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallStaticObjectMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject CallStaticObjectMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jboolean CallStaticBooleanMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jboolean CallStaticBooleanMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jboolean CallStaticBooleanMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return JNI_FALSE;
  }

  static jbyte CallStaticByteMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte CallStaticByteMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte CallStaticByteMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallStaticCharMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallStaticCharMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar CallStaticCharMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallStaticShortMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallStaticShortMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort CallStaticShortMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallStaticIntMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallStaticIntMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint CallStaticIntMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallStaticLongMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallStaticLongMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong CallStaticLongMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallStaticFloatMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallStaticFloatMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat CallStaticFloatMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallStaticDoubleMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallStaticDoubleMethodV(JNIEnv* env, jclass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble CallStaticDoubleMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
    return 0;
  }

  static void CallStaticVoidMethod(JNIEnv* env, jclass, jmethodID mid, ...) {
    printf("%s\n", __func__);
  }

  static void CallStaticVoidMethodV(JNIEnv* env, jclass klass, jmethodID mid, va_list args) {
    printf("%s\n", __func__);
    Class_t* pClass = reinterpret_cast<Class_t*>(klass);
    Method_t* pMethod = ClFindMethod(pClass, (const uint8_t*)"main");
    if (pMethod == NULL) {
      pdbg("Cannot find method MAIN\n");
      return;
    }
    // OAT code dump
    DumpData(reinterpret_cast<const uint32_t*>(pMethod->method_oat_code), pMethod->method_oat_code_hdr->code_size_, 0);
    // Execute the code
    ExecuteOatCode(pMethod->method_oat_code);
  }

  static void CallStaticVoidMethodA(JNIEnv* env, jclass, jmethodID mid, jvalue* args) {
    printf("%s\n", __func__);
  }

  static jstring NewString(JNIEnv* env, const jchar* chars, jsize char_count) {
    printf("%s\n", __func__);
    return 0;
  }

  static jstring NewStringUTF(JNIEnv* env, const char* utf) {
    printf("%s\n", __func__);
    jstring my = new _jstring();
    return my;
  }

  static jsize GetStringLength(JNIEnv* env, jstring java_string) {
    printf("%s\n", __func__);
    return 0;
  }

  static jsize GetStringUTFLength(JNIEnv* env, jstring java_string) {
    printf("%s\n", __func__);
    return 0;
  }

  static void GetStringRegion(JNIEnv* env, jstring java_string, jsize start, jsize length,
                              jchar* buf) {
    printf("%s\n", __func__);
  }

  static void GetStringUTFRegion(JNIEnv* env, jstring java_string, jsize start, jsize length,
                                 char* buf) {
    printf("%s\n", __func__);
  }

  static const jchar* GetStringChars(JNIEnv* env, jstring java_string, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static void ReleaseStringChars(JNIEnv* env, jstring java_string, const jchar* chars) {
    printf("%s\n", __func__);
  }

  static const jchar* GetStringCritical(JNIEnv* env, jstring java_string, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static void ReleaseStringCritical(JNIEnv* env, jstring java_string, const jchar* chars) {
    printf("%s\n", __func__);
  }

  static const char* GetStringUTFChars(JNIEnv* env, jstring java_string, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static void ReleaseStringUTFChars(JNIEnv* env, jstring, const char* chars) {
    printf("%s\n", __func__);
  }

  static jsize GetArrayLength(JNIEnv* env, jarray java_array) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobject GetObjectArrayElement(JNIEnv* env, jobjectArray java_array, jsize index) {
    printf("%s\n", __func__);
    return 0;
  }

  static void SetObjectArrayElement(JNIEnv* env, jobjectArray java_array, jsize index,
                                    jobject java_value) {
    printf("%s\n", __func__);
  }

  static jbooleanArray NewBooleanArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyteArray NewByteArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jcharArray NewCharArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdoubleArray NewDoubleArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloatArray NewFloatArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jintArray NewIntArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlongArray NewLongArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobjectArray NewObjectArray(JNIEnv* env, jsize length, jclass element_jclass,
                                     jobject initial_element) {
    printf("%s\n", __func__);
    jobjectArray my = new _jobjectArray();
    return my;
  }

  static jshortArray NewShortArray(JNIEnv* env, jsize length) {
    printf("%s\n", __func__);
    return 0;
  }

  static void* GetPrimitiveArrayCritical(JNIEnv* env, jarray java_array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static void ReleasePrimitiveArrayCritical(JNIEnv* env, jarray java_array, void* elements,
                                              jint mode) {
    printf("%s\n", __func__);
  }

  static jboolean* GetBooleanArrayElements(JNIEnv* env, jbooleanArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jbyte* GetByteArrayElements(JNIEnv* env, jbyteArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jchar* GetCharArrayElements(JNIEnv* env, jcharArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jdouble* GetDoubleArrayElements(JNIEnv* env, jdoubleArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jfloat* GetFloatArrayElements(JNIEnv* env, jfloatArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint* GetIntArrayElements(JNIEnv* env, jintArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong* GetLongArrayElements(JNIEnv* env, jlongArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static jshort* GetShortArrayElements(JNIEnv* env, jshortArray array, jboolean* is_copy) {
    printf("%s\n", __func__);
    return 0;
  }

  static void ReleaseBooleanArrayElements(JNIEnv* env, jbooleanArray array, jboolean* elements,
                                          jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseByteArrayElements(JNIEnv* env, jbyteArray array, jbyte* elements, jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseCharArrayElements(JNIEnv* env, jcharArray array, jchar* elements, jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseDoubleArrayElements(JNIEnv* env, jdoubleArray array, jdouble* elements,
                                         jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseFloatArrayElements(JNIEnv* env, jfloatArray array, jfloat* elements,
                                        jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseIntArrayElements(JNIEnv* env, jintArray array, jint* elements, jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseLongArrayElements(JNIEnv* env, jlongArray array, jlong* elements, jint mode) {
    printf("%s\n", __func__);
  }

  static void ReleaseShortArrayElements(JNIEnv* env, jshortArray array, jshort* elements,
                                        jint mode) {
    printf("%s\n", __func__);
  }

  static void GetBooleanArrayRegion(JNIEnv* env, jbooleanArray array, jsize start, jsize length,
                                    jboolean* buf) {
    printf("%s\n", __func__);
  }

  static void GetByteArrayRegion(JNIEnv* env, jbyteArray array, jsize start, jsize length,
                                jbyte* buf) {
    printf("%s\n", __func__);
  }

  static void GetCharArrayRegion(JNIEnv* env, jcharArray array, jsize start, jsize length,
                                 jchar* buf) {
    printf("%s\n", __func__);
  }

  static void GetDoubleArrayRegion(JNIEnv* env, jdoubleArray array, jsize start, jsize length,
                                   jdouble* buf) {
    printf("%s\n", __func__);
  }

  static void GetFloatArrayRegion(JNIEnv* env, jfloatArray array, jsize start, jsize length,
                                  jfloat* buf) {
    printf("%s\n", __func__);
  }

  static void GetIntArrayRegion(JNIEnv* env, jintArray array, jsize start, jsize length,
                                jint* buf) {
    printf("%s\n", __func__);
  }

  static void GetLongArrayRegion(JNIEnv* env, jlongArray array, jsize start, jsize length,
                                 jlong* buf) {
    printf("%s\n", __func__);
  }

  static void GetShortArrayRegion(JNIEnv* env, jshortArray array, jsize start, jsize length,
                                  jshort* buf) {
    printf("%s\n", __func__);
  }

  static void SetBooleanArrayRegion(JNIEnv* env, jbooleanArray array, jsize start, jsize length,
                                    const jboolean* buf) {
    printf("%s\n", __func__);
  }

  static void SetByteArrayRegion(JNIEnv* env, jbyteArray array, jsize start, jsize length,
                                 const jbyte* buf) {
    printf("%s\n", __func__);
  }

  static void SetCharArrayRegion(JNIEnv* env, jcharArray array, jsize start, jsize length,
                                 const jchar* buf) {
    printf("%s\n", __func__);
  }

  static void SetDoubleArrayRegion(JNIEnv* env, jdoubleArray array, jsize start, jsize length,
                                   const jdouble* buf) {
    printf("%s\n", __func__);
  }

  static void SetFloatArrayRegion(JNIEnv* env, jfloatArray array, jsize start, jsize length,
                                  const jfloat* buf) {
    printf("%s\n", __func__);
  }

  static void SetIntArrayRegion(JNIEnv* env, jintArray array, jsize start, jsize length,
                                const jint* buf) {
    printf("%s\n", __func__);
  }

  static void SetLongArrayRegion(JNIEnv* env, jlongArray array, jsize start, jsize length,
                                 const jlong* buf) {
    printf("%s\n", __func__);
  }

  static void SetShortArrayRegion(JNIEnv* env, jshortArray array, jsize start, jsize length,
                                  const jshort* buf) {
    printf("%s\n", __func__);
  }

  static jint RegisterNatives(JNIEnv* env, jclass java_class, const JNINativeMethod* methods,
                                jint method_count) {
    printf("%s\n", __func__);
    return 0;
  }

  static jint RegisterNativeMethods(JNIEnv* env, jclass java_class, const JNINativeMethod* methods,
                                    jint method_count, bool return_errors) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint UnregisterNatives(JNIEnv* env, jclass java_class) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint MonitorEnter(JNIEnv* env, jobject java_object) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint MonitorExit(JNIEnv* env, jobject java_object) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jint GetJavaVM(JNIEnv* env, JavaVM** vm) {
    printf("%s\n", __func__);
    return JNI_OK;
  }

  static jobject NewDirectByteBuffer(JNIEnv* env, void* address, jlong capacity) {
    printf("%s\n", __func__);
    return 0;
  }

  static void* GetDirectBufferAddress(JNIEnv* env, jobject java_buffer) {
    printf("%s\n", __func__);
    return 0;
  }

  static jlong GetDirectBufferCapacity(JNIEnv* env, jobject java_buffer) {
    printf("%s\n", __func__);
    return 0;
  }

  static jobjectRefType GetObjectRefType(JNIEnv* env, jobject java_object) {
    printf("%s\n", __func__);
    return JNIInvalidRefType;
  }
};  // class JNI

const JNINativeInterface gJniNativeInterface = {
  0,  // reserved0.
  0,  // reserved1.
  0,  // reserved2.
  0,  // reserved3.
  JNI::GetVersion,
  JNI::DefineClass,
  JNI::FindClass,
  JNI::FromReflectedMethod,
  JNI::FromReflectedField,
  JNI::ToReflectedMethod,
  JNI::GetSuperclass,
  JNI::IsAssignableFrom,
  JNI::ToReflectedField,
  JNI::Throw,
  JNI::ThrowNew,
  JNI::ExceptionOccurred,
  JNI::ExceptionDescribe,
  JNI::ExceptionClear,
  JNI::FatalError,
  JNI::PushLocalFrame,
  JNI::PopLocalFrame,
  JNI::NewGlobalRef,
  JNI::DeleteGlobalRef,
  JNI::DeleteLocalRef,
  JNI::IsSameObject,
  JNI::NewLocalRef,
  JNI::EnsureLocalCapacity,
  JNI::AllocObject,
  JNI::NewObject,
  JNI::NewObjectV,
  JNI::NewObjectA,
  JNI::GetObjectClass,
  JNI::IsInstanceOf,
  JNI::GetMethodID,
  JNI::CallObjectMethod,
  JNI::CallObjectMethodV,
  JNI::CallObjectMethodA,
  JNI::CallBooleanMethod,
  JNI::CallBooleanMethodV,
  JNI::CallBooleanMethodA,
  JNI::CallByteMethod,
  JNI::CallByteMethodV,
  JNI::CallByteMethodA,
  JNI::CallCharMethod,
  JNI::CallCharMethodV,
  JNI::CallCharMethodA,
  JNI::CallShortMethod,
  JNI::CallShortMethodV,
  JNI::CallShortMethodA,
  JNI::CallIntMethod,
  JNI::CallIntMethodV,
  JNI::CallIntMethodA,
  JNI::CallLongMethod,
  JNI::CallLongMethodV,
  JNI::CallLongMethodA,
  JNI::CallFloatMethod,
  JNI::CallFloatMethodV,
  JNI::CallFloatMethodA,
  JNI::CallDoubleMethod,
  JNI::CallDoubleMethodV,
  JNI::CallDoubleMethodA,
  JNI::CallVoidMethod,
  JNI::CallVoidMethodV,
  JNI::CallVoidMethodA,
  JNI::CallNonvirtualObjectMethod,
  JNI::CallNonvirtualObjectMethodV,
  JNI::CallNonvirtualObjectMethodA,
  JNI::CallNonvirtualBooleanMethod,
  JNI::CallNonvirtualBooleanMethodV,
  JNI::CallNonvirtualBooleanMethodA,
  JNI::CallNonvirtualByteMethod,
  JNI::CallNonvirtualByteMethodV,
  JNI::CallNonvirtualByteMethodA,
  JNI::CallNonvirtualCharMethod,
  JNI::CallNonvirtualCharMethodV,
  JNI::CallNonvirtualCharMethodA,
  JNI::CallNonvirtualShortMethod,
  JNI::CallNonvirtualShortMethodV,
  JNI::CallNonvirtualShortMethodA,
  JNI::CallNonvirtualIntMethod,
  JNI::CallNonvirtualIntMethodV,
  JNI::CallNonvirtualIntMethodA,
  JNI::CallNonvirtualLongMethod,
  JNI::CallNonvirtualLongMethodV,
  JNI::CallNonvirtualLongMethodA,
  JNI::CallNonvirtualFloatMethod,
  JNI::CallNonvirtualFloatMethodV,
  JNI::CallNonvirtualFloatMethodA,
  JNI::CallNonvirtualDoubleMethod,
  JNI::CallNonvirtualDoubleMethodV,
  JNI::CallNonvirtualDoubleMethodA,
  JNI::CallNonvirtualVoidMethod,
  JNI::CallNonvirtualVoidMethodV,
  JNI::CallNonvirtualVoidMethodA,
  JNI::GetFieldID,
  JNI::GetObjectField,
  JNI::GetBooleanField,
  JNI::GetByteField,
  JNI::GetCharField,
  JNI::GetShortField,
  JNI::GetIntField,
  JNI::GetLongField,
  JNI::GetFloatField,
  JNI::GetDoubleField,
  JNI::SetObjectField,
  JNI::SetBooleanField,
  JNI::SetByteField,
  JNI::SetCharField,
  JNI::SetShortField,
  JNI::SetIntField,
  JNI::SetLongField,
  JNI::SetFloatField,
  JNI::SetDoubleField,
  JNI::GetStaticMethodID,
  JNI::CallStaticObjectMethod,
  JNI::CallStaticObjectMethodV,
  JNI::CallStaticObjectMethodA,
  JNI::CallStaticBooleanMethod,
  JNI::CallStaticBooleanMethodV,
  JNI::CallStaticBooleanMethodA,
  JNI::CallStaticByteMethod,
  JNI::CallStaticByteMethodV,
  JNI::CallStaticByteMethodA,
  JNI::CallStaticCharMethod,
  JNI::CallStaticCharMethodV,
  JNI::CallStaticCharMethodA,
  JNI::CallStaticShortMethod,
  JNI::CallStaticShortMethodV,
  JNI::CallStaticShortMethodA,
  JNI::CallStaticIntMethod,
  JNI::CallStaticIntMethodV,
  JNI::CallStaticIntMethodA,
  JNI::CallStaticLongMethod,
  JNI::CallStaticLongMethodV,
  JNI::CallStaticLongMethodA,
  JNI::CallStaticFloatMethod,
  JNI::CallStaticFloatMethodV,
  JNI::CallStaticFloatMethodA,
  JNI::CallStaticDoubleMethod,
  JNI::CallStaticDoubleMethodV,
  JNI::CallStaticDoubleMethodA,
  JNI::CallStaticVoidMethod,
  JNI::CallStaticVoidMethodV,
  JNI::CallStaticVoidMethodA,
  JNI::GetStaticFieldID,
  JNI::GetStaticObjectField,
  JNI::GetStaticBooleanField,
  JNI::GetStaticByteField,
  JNI::GetStaticCharField,
  JNI::GetStaticShortField,
  JNI::GetStaticIntField,
  JNI::GetStaticLongField,
  JNI::GetStaticFloatField,
  JNI::GetStaticDoubleField,
  JNI::SetStaticObjectField,
  JNI::SetStaticBooleanField,
  JNI::SetStaticByteField,
  JNI::SetStaticCharField,
  JNI::SetStaticShortField,
  JNI::SetStaticIntField,
  JNI::SetStaticLongField,
  JNI::SetStaticFloatField,
  JNI::SetStaticDoubleField,
  JNI::NewString,
  JNI::GetStringLength,
  JNI::GetStringChars,
  JNI::ReleaseStringChars,
  JNI::NewStringUTF,
  JNI::GetStringUTFLength,
  JNI::GetStringUTFChars,
  JNI::ReleaseStringUTFChars,
  JNI::GetArrayLength,
  JNI::NewObjectArray,
  JNI::GetObjectArrayElement,
  JNI::SetObjectArrayElement,
  JNI::NewBooleanArray,
  JNI::NewByteArray,
  JNI::NewCharArray,
  JNI::NewShortArray,
  JNI::NewIntArray,
  JNI::NewLongArray,
  JNI::NewFloatArray,
  JNI::NewDoubleArray,
  JNI::GetBooleanArrayElements,
  JNI::GetByteArrayElements,
  JNI::GetCharArrayElements,
  JNI::GetShortArrayElements,
  JNI::GetIntArrayElements,
  JNI::GetLongArrayElements,
  JNI::GetFloatArrayElements,
  JNI::GetDoubleArrayElements,
  JNI::ReleaseBooleanArrayElements,
  JNI::ReleaseByteArrayElements,
  JNI::ReleaseCharArrayElements,
  JNI::ReleaseShortArrayElements,
  JNI::ReleaseIntArrayElements,
  JNI::ReleaseLongArrayElements,
  JNI::ReleaseFloatArrayElements,
  JNI::ReleaseDoubleArrayElements,
  JNI::GetBooleanArrayRegion,
  JNI::GetByteArrayRegion,
  JNI::GetCharArrayRegion,
  JNI::GetShortArrayRegion,
  JNI::GetIntArrayRegion,
  JNI::GetLongArrayRegion,
  JNI::GetFloatArrayRegion,
  JNI::GetDoubleArrayRegion,
  JNI::SetBooleanArrayRegion,
  JNI::SetByteArrayRegion,
  JNI::SetCharArrayRegion,
  JNI::SetShortArrayRegion,
  JNI::SetIntArrayRegion,
  JNI::SetLongArrayRegion,
  JNI::SetFloatArrayRegion,
  JNI::SetDoubleArrayRegion,
  JNI::RegisterNatives,
  JNI::UnregisterNatives,
  JNI::MonitorEnter,
  JNI::MonitorExit,
  JNI::GetJavaVM,
  JNI::GetStringRegion,
  JNI::GetStringUTFRegion,
  JNI::GetPrimitiveArrayCritical,
  JNI::ReleasePrimitiveArrayCritical,
  JNI::GetStringCritical,
  JNI::ReleaseStringCritical,
  JNI::NewWeakGlobalRef,
  JNI::DeleteWeakGlobalRef,
  JNI::ExceptionCheck,
  JNI::NewDirectByteBuffer,
  JNI::GetDirectBufferAddress,
  JNI::GetDirectBufferCapacity,
  JNI::GetObjectRefType,
};

JNIEnvExt::JNIEnvExt()
  : oatdex_files_(NULL),
    class_linker_(NULL) {
  // Allocate a hash table for storing the list of OATDEX files
  oatdex_files_ = AllocHashTable(NULL, 5);
  if (!oatdex_files_) {
    pdbg("Out of memory\n");
    return;
  }
  // Allocate a classlinker
  class_linker_ = AllocateClassLinker(HEAP_START_SIZE);
  if (!class_linker_) {
    pdbg("Failed to allocate classlinker\n");
    FreeHashTable(oatdex_files_);
    return;
  }
  // Register JNI interfaces
  functions = &gJniNativeInterface;
}

JNIEnvExt::~JNIEnvExt() {
  functions = NULL;
  if (class_linker_) {
    DeregisterAllClasses(class_linker_);
    FreeClassLinker(class_linker_);
    class_linker_ = NULL;
  }
  if (oatdex_files_) {
    FreeHashTable(oatdex_files_);
    oatdex_files_ = NULL;
  }
}

HashTable_t* JNIEnvExt::GetOatDexFiles() const {
  return oatdex_files_;
}

ClassLinker_t* JNIEnvExt::GetClassLinker() const {
  return class_linker_;
}

bool JNIEnvExt::IsReady() const {
  if ((oatdex_files_ != NULL) && (class_linker_ != NULL)) {
    return true;
  }
  return false;
}

}  // namespace cart


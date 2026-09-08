// Copyright (c) 2020-2026 HexHacking Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2026-09-08.

#include <errno.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>

#include "xdl.h"

#define XDL_JNI_VERSION    JNI_VERSION_1_6
#define XDL_JNI_CLASS_NAME "io/github/hexhacking/xdl/Xdl"

static jlong xdl_jni_open(JNIEnv *env, jobject thiz, jstring filename, jboolean preload_symbols) {
  (void)thiz;

  if (!env || !(*env) || !filename) return 0;

  const char *c_filename = NULL;
  if (NULL == (c_filename = (*env)->GetStringUTFChars(env, filename, 0))) return 0;

  void *handle = xdl_open(c_filename, XDL_DEFAULT);
  if (NULL != handle && preload_symbols) xdl_preload(handle);

  (*env)->ReleaseStringUTFChars(env, filename, c_filename);
  return (jlong)handle;
}

static void xdl_jni_close(JNIEnv *env, jobject thiz, jlong handle) {
  (void)env, (void)thiz;

  if (!handle) return;

  xdl_close((void *)handle);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;

  if (__predict_false(NULL == vm)) return JNI_ERR;

  JNIEnv *env;
  if (__predict_false(JNI_OK != (*vm)->GetEnv(vm, (void **)&env, XDL_JNI_VERSION))) return JNI_ERR;
  if (__predict_false(NULL == env || NULL == *env)) return JNI_ERR;

  jclass cls;
  if (__predict_false(NULL == (cls = (*env)->FindClass(env, XDL_JNI_CLASS_NAME)))) return JNI_ERR;

  JNINativeMethod m[] = {{"nativeOpen", "(Ljava/lang/String;Z)J", (void *)xdl_jni_open},
                         {"nativeClose", "(J)V", (void *)xdl_jni_close}};
  if (__predict_false(0 != (*env)->RegisterNatives(env, cls, m, sizeof(m) / sizeof(m[0])))) return JNI_ERR;

  return XDL_JNI_VERSION;
}

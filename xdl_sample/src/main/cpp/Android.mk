LOCAL_PATH := $(call my-dir)

XDL_BASE                := $(LOCAL_PATH)/../../../build/nativedeps/xdl

include $(CLEAR_VARS)
LOCAL_MODULE            := xdl
LOCAL_SRC_FILES         := $(XDL_BASE)/aar/jni/$(TARGET_ARCH_ABI)/libxdl.so
LOCAL_EXPORT_C_INCLUDES := $(XDL_BASE)/header
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := sample
LOCAL_SRC_FILES         := sample.c
LOCAL_CFLAGS            := -Weverything -Werror
LOCAL_CONLYFLAGS        := -std=c11
LOCAL_LDLIBS            := -llog
LOCAL_SHARED_LIBRARIES  += xdl
include $(BUILD_SHARED_LIBRARY)

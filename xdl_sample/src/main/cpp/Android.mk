LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE            := sample
LOCAL_SRC_FILES         := sample.c
LOCAL_CFLAGS            := -Weverything -Werror
LOCAL_CONLYFLAGS        := -std=c17
LOCAL_LDLIBS            := -llog
LOCAL_SHARED_LIBRARIES  += xdl
include $(BUILD_SHARED_LIBRARY)

#import xdl by maven
$(call import-module,prefab/xdl)

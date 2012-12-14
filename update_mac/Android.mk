# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= update_wifi_mac.c 
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE:= update_wifi_mac 

include $(BUILD_EXECUTABLE)


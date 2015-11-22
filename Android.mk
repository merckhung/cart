#
# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

source_files := \
	utils.cc \
	elf.cc \
	oat.cc \
	list.cc \
	hash.cc \
	heap.cc \
	net.cc \
	zip.cc \
	cart.cc \
	debugger.cc \
	class.cc \
	jni_env_ext.cc \
	java_vm_ext.cc

source_files_32 := \
	entry_init_x86.cc \
	entry_x86.S

source_files_64 := \
	entry_init_x86_64.cc \
	entry_x86_64.S

includes := \
	external/zlib

#include $(CLEAR_VARS)
#LOCAL_MODULE := libcart
#LOCAL_CPP_EXTENSION := .cc
#LOCAL_SRC_FILES := ${source_files}
#LOCAL_SRC_FILES_32 := ${source_files_32}
#LOCAL_SRC_FILES_64 := ${source_files_64}

#LOCAL_STATIC_LIBRARIES := libz
#LOCAL_SHARED_LIBRARIES := libutils libnativehelper

#LOCAL_C_INCLUDES += ${includes}
#LOCAL_CFLAGS := -O2 -Wall -DCART_DEBUG
#include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcart
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := ${source_files}
LOCAL_SRC_FILES_32 := ${source_files_32}
LOCAL_SRC_FILES_64 := ${source_files_64}
LOCAL_C_INCLUDES += ${includes}

LOCAL_STATIC_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES := libnativehelper
LOCAL_LDLIBS := -lpthread
LOCAL_MODULE := libcart
LOCAL_CFLAGS := -O2 -Wall -DCART_DEBUG
LOCAL_MULTILIB := both
include $(BUILD_HOST_SHARED_LIBRARY)


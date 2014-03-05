#
# Copyright (C) 2012 The Android Open Source Project
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
#
#

LOCAL_PATH := $(call my-dir)

#=====================================================================
# Common: libbccRenderscript
#=====================================================================

libbcc_renderscript_SRC_FILES := \
  RSCompiler.cpp \
  RSCompilerDriver.cpp \
  RSEmbedInfo.cpp \
  RSExecutable.cpp \
  RSInfo.cpp \
  RSInfoExtractor.cpp \
  RSInfoReader.cpp \
  RSInfoWriter.cpp \
  RSScript.cpp

ifneq ($(PRODUCT_BRAND),intel)
libbcc_renderscript_SRC_FILES += RSForEachExpand.cpp
endif

#=====================================================================
# Device Static Library: libbccRenderscript
#=====================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := libbccRenderscript
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES

LOCAL_SRC_FILES := $(libbcc_renderscript_SRC_FILES)
ifeq ($(PRODUCT_BRAND),intel)
ifeq ($(TARGET_ARCH),x86) # We don't support x86-64 right now
  LOCAL_CFLAGS += -DARCH_X86_RS_VECTORIZER
  LOCAL_SRC_FILES += RSForEachExpand_x86.cpp \
                     RSVectorizationSupport.cpp
else
  LOCAL_SRC_FILES += RSForEachExpand.cpp
endif
endif

include $(LIBBCC_DEVICE_BUILD_MK)
include $(LIBBCC_GEN_CONFIG_MK)
include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)


#=====================================================================
# Host Static Library: libbccRenderscript
#=====================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := libbccRenderscript
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_IS_HOST_MODULE := true

LOCAL_SRC_FILES := $(libbcc_renderscript_SRC_FILES)
ifeq ($(PRODUCT_BRAND),intel)
ifeq ($(TARGET_ARCH),x86)
  LOCAL_CFLAGS += -DARCH_X86_RS_VECTORIZER
  LOCAL_SRC_FILES += RSForEachExpand_x86.cpp \
                     RSVectorizationSupport.cpp
else
  LOCAL_SRC_FILES += RSForEachExpand.cpp
endif
endif

include $(LIBBCC_HOST_BUILD_MK)
include $(LIBBCC_GEN_CONFIG_MK)
include $(LLVM_HOST_BUILD_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

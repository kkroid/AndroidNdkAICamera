#
# Copyright (C)  2017 The Android Open Source Project
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

cmake_minimum_required(VERSION 3.4.1)

add_definitions(-D NCNN_VULKAN=ON)
add_definitions(-D PLATFORM_ANDROID=1)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")

# NCNN
#OPENMP
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -static-openmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -static-openmp")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fopenmp -static-openmp")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -fstrict-aliasing -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer -fstrict-aliasing -ffast-math")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

# disable rtti and exceptions
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

#set(CMAKE_VERBOSE_MAKEFILE on)
set(NDK_CAMERA ${CMAKE_CURRENT_SOURCE_DIR}/ndkcamera)

set(THRID_PARTY ${CMAKE_SOURCE_DIR}/../cpp_include)

include_directories(${NDK_CAMERA}/public
        ${CMAKE_CURRENT_SOURCE_DIR}/task
        ${THRID_PARTY}/ncnn/ncnn
        ${THRID_PARTY}/opencv
        ${THRID_PARTY}/libyuv
        ${THRID_PARTY}/ultra
        )

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(Libs ${CMAKE_SOURCE_DIR}/../libs)

# NCNN
add_library(ncnn STATIC IMPORTED)
add_library(glslang STATIC IMPORTED)
add_library(OGLCompiler STATIC IMPORTED)
add_library(OSDependent STATIC IMPORTED)
add_library(SPIRV STATIC IMPORTED)
set_target_properties(ncnn PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libncnn.a)
set_target_properties(glslang PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libglslang.a)
set_target_properties(OGLCompiler PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libOGLCompiler.a)
set_target_properties(OSDependent PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libOSDependent.a)
set_target_properties(SPIRV PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libSPIRV.a)

add_library(ndk_camera SHARED
        ${NDK_CAMERA}/private/AndroidCameraServer.cpp
        ${NDK_CAMERA}/private/AndroidCameraHandler.cpp
        ${NDK_CAMERA}/private/AndroidImageReader.cpp
        ${NDK_CAMERA}/private/AndroidCameraListeners.cpp
        ${NDK_CAMERA}/private/CameraManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera_ui.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/ExTracker.cpp
        ${THRID_PARTY}/ultra/UltraFace.cpp
        )

# YUV
add_library(yuv SHARED IMPORTED)
set_target_properties(yuv PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libyuv.so)

# OPENCV
add_library(opencv SHARED IMPORTED)
set_target_properties(opencv PROPERTIES IMPORTED_LOCATION ${Libs}/${ANDROID_ABI}/libopencv_java4.so)

# add lib dependencies
target_link_libraries(ndk_camera
        android
        log
        camera2ndk
        mediandk
        yuv
        opencv
        ncnn
        glslang
        OGLCompiler
        OSDependent
        SPIRV
        z
        vulkan
        )

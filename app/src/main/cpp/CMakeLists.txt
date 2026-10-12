cmake_minimum_required(VERSION 3.22.1)

project("hal_llama" VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match the upstream Android binding: package several CPU variants and select
# the best supported backend at runtime (NEON/dotprod/i8mm/SVE/SME or SSE/AVX when present).
if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(GGML_SYSTEM_ARCH "ARM")
elseif(ANDROID_ABI STREQUAL "x86_64")
    set(GGML_SYSTEM_ARCH "x86")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a" OR ANDROID_ABI STREQUAL "x86")
    # 32-bit: ggml has no CPU variants for them, so one baseline CPU backend (NEON / SSSE3) is built into libggml
    if(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(GGML_SYSTEM_ARCH "ARM")
    else()
        set(GGML_SYSTEM_ARCH "x86")
    endif()
    set(GGML_CPU_ALL_VARIANTS OFF CACHE BOOL "No CPU variants on 32-bit ABIs" FORCE)
    set(GGML_BACKEND_DL OFF CACHE BOOL "The CPU backend is linked into libggml on 32-bit ABIs" FORCE)
else()
    message(FATAL_ERROR "Unsupported ABI: ${ANDROID_ABI}")
endif()
set(GGML_CPU_KLEIDIAI OFF CACHE BOOL "Disable network-fetched KleidiAI for the test build" FORCE)
set(GGML_OPENMP ON)

set(LLAMA_SRC "${CMAKE_CURRENT_LIST_DIR}/../../../../third_party/llama.cpp")
if(NOT EXISTS "${LLAMA_SRC}/CMakeLists.txt")
    message(FATAL_ERROR
            "llama.cpp source is missing. Run: git submodule update --init --recursive")
endif()

# Local patches over the pinned upstream llama.cpp; skipped when already applied
find_package(Git REQUIRED)
file(GLOB LLAMA_PATCHES "${CMAKE_CURRENT_LIST_DIR}/../../../../third_party/patches/llama-*.patch")
foreach(patch ${LLAMA_PATCHES})
    execute_process(COMMAND ${GIT_EXECUTABLE} apply --check --reverse --ignore-whitespace ${patch}
            WORKING_DIRECTORY ${LLAMA_SRC} RESULT_VARIABLE applied OUTPUT_QUIET ERROR_QUIET)
    if(NOT applied EQUAL 0)
        execute_process(COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${patch}
                WORKING_DIRECTORY ${LLAMA_SRC} RESULT_VARIABLE failed)
        if(NOT failed EQUAL 0)
            message(FATAL_ERROR "Failed to apply ${patch}")
        endif()
    endif()
endforeach()

add_subdirectory("${LLAMA_SRC}" build-llama)

if(ANDROID_ABI STREQUAL "armeabi-v7a")
    # llama.cpp bug: arch/arm/quants.c calls the AArch64-only vqtbl1q_u8 instead of its armv7 shim ggml_vqtbl1q_u8
    target_compile_definitions(ggml-cpu PRIVATE vqtbl1q_u8=ggml_vqtbl1q_u8)
endif()

add_library(hal_llama SHARED hal_llama_jni.cpp)

set(HALSM_SRC "${CMAKE_CURRENT_LIST_DIR}/halsm/src")
set(HALSM_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/halsm/include")

add_library(hal_halsm SHARED
        halsm_jni.c
        "${HALSM_SRC}/HalSM.c"
        "${HALSM_SRC}/HalSMInteger.c"
        "${HALSM_SRC}/HalSMDouble.c"
        "${HALSM_SRC}/HalSMMemory.c"
        "${HALSM_SRC}/HalStringFormat.c"
        "${HALSM_SRC}/HalFileSystem.c"
        "${HALSM_SRC}/HalSMExecutable.c")

target_include_directories(hal_halsm PRIVATE "${HALSM_INCLUDE}")
target_compile_options(hal_halsm PRIVATE -Wall -Wextra -Wno-unused-parameter)
target_link_libraries(hal_halsm android log dl m)

target_compile_definitions(hal_llama PRIVATE
        GGML_SYSTEM_ARCH=${GGML_SYSTEM_ARCH}
        GGML_CPU_KLEIDIAI=$<BOOL:${GGML_CPU_KLEIDIAI}>
        GGML_OPENMP=$<BOOL:${GGML_OPENMP}>)

target_include_directories(hal_llama PRIVATE
        "${LLAMA_SRC}"
        "${LLAMA_SRC}/common"
        "${LLAMA_SRC}/include"
        "${LLAMA_SRC}/ggml/include"
        "${LLAMA_SRC}/ggml/src")

target_link_libraries(hal_llama
        llama
        llama-common
        android
        log)

cmake_minimum_required(VERSION 3.6)

set(APP_NAME pitch)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

project(${APP_NAME})

set(CPP_SOURCE)
set(CPP_HEADER)

list(APPEND CPP_HEADER
#        ../../c++/com_cai_pitchdetect_utils_AudioUnitDataHandler.h
        src/main/cpp/pitch_wrapper.h
        ../../c++/audioDetail/pitch.h
        ../../c++/audioDetail/PitchDetect.h
        ../../c++/audioDetail/SafeQueue.h
        ../../c++/nlohmann/json.hpp
        ../../c++/nlohmann/json_fwd.hpp
)

list(APPEND CPP_SOURCE
#        ../../c++/audioDetail/com_cai_pitchdetect_utils_AudioUnitDataHandler.cpp
        src/main/cpp/pitch_wrapper.cpp
        ../../c++/audioDetail/PitchDetect.cpp
)

set(all_code_files
        ${CPP_HEADER}
        ${CPP_SOURCE}
)


include_directories(../../c++/audioDetail
        ../../c++/nlohmann)

set(JNI_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/main/libs)

add_library(${APP_NAME} SHARED ${all_code_files})

# Add the static library
add_library(libpitchdetect STATIC IMPORTED GLOBAL)
set_target_properties(libpitchdetect
        PROPERTIES IMPORTED_LOCATION "${JNI_LIBS_DIR}/${CMAKE_ANDROID_ARCH_ABI}/liblibpitchdetect.a"
)

# Link against the static library
target_link_libraries(${APP_NAME} PRIVATE c libpitchdetect)

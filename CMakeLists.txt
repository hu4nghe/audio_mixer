cmake_minimum_required(VERSION 3.28)

project(audio_mixer 
    VERSION 0.1
    DESCRIPTION "A MPMC audio mixer"
)

include(FetchContent)
#add_subdirectory(modules/audio_mixer)
add_subdirectory(modules/audio_queue)
add_subdirectory(modules/input_module_base)
add_subdirectory(modules/audio_file)

include(CTest)
option(AUDIOMIXER_BUILD_TESTS "Build AudioMixer's own tests" ON)
if(AUDIOMIXER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
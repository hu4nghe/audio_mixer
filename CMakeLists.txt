cmake_minimum_required(VERSION 3.28)

project(audio_mixer 
    VERSION 0.1
    DESCRIPTION "A MPMC audio mixer"
)
include(FetchContent)
#add_subdirectory(modules/audio_mixer)
add_subdirectory(modules/audio_queue)

option(BUILD_TESTING "Enable testing" OFF)

# ----------------- Dependency : my lock-free queue -----------------
FetchContent_Declare(
        lockfree_queue
        GIT_REPOSITORY https://github.com/hu4nghe/lockfree_queue
        GIT_TAG v1.0.1
    )
    FetchContent_MakeAvailable(lockfree_queue)


# ----------------- Dependency : libsamplerate -----------------
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_Declare(
    libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 0.2.2
)

if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY 
    "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" 
    CACHE STRING "MSVC runtime library")
    FetchContent_MakeAvailable(libsamplerate)
else()
    # Supress libsamplerate warning
    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libsamplerate)
    set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
endif()

# ----------------- Library -----------------

file(GLOB audio_queue_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(audio_queue STATIC ${audio_queue_SRC})

set_target_properties(audio_queue PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(audio_queue
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(audio_queue
    PUBLIC
        samplerate
        lockfree_queue
)
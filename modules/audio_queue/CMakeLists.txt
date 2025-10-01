# ----------------- Dependency : my lock-free queue -----------------
FetchContent_Declare(
        lockfree_queue
        GIT_REPOSITORY https://github.com/hu4nghe/lockfree_queue
        GIT_TAG master
    )
    FetchContent_MakeAvailable(lockfree_queue)


# ----------------- Dependency : libsamplerate -----------------
FetchContent_Declare(
    libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 0.2.2
)

set(USE_FETCHCONTENT FALSE)

if(WIN32)
    set(USE_FETCHCONTENT TRUE)
else()
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(SAMPLERATE QUIET samplerate)
    endif()

    if(NOT SAMPLERATE_FOUND)
        set(USE_FETCHCONTENT TRUE)
    endif()
endif()

if(USE_FETCHCONTENT)
    FetchContent_MakeAvailable(libsamplerate)
    set(SAMPLERATE_TARGET samplerate)
else()
    include_directories(${SAMPLERATE_INCLUDE_DIRS})
    set(SAMPLERATE_TARGET ${SAMPLERATE_LIBRARIES})
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
        ${SAMPLERATE_INCLUDE_DIRS}
)

target_link_libraries(audio_queue
    PUBLIC
        ${SAMPLERATE_TARGET} #linking to libsamplerate
        lockfree_queue
)
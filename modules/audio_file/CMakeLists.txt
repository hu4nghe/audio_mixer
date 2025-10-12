option(BUILD_TESTING "Enable testing" OFF)

# ----------------- Dependency : libsndfile -----------------
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG 1.2.2
)
FetchContent_MakeAvailable(libsndfile)
# ----------------- Library -----------------

file(GLOB audio_file_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(audio_file STATIC ${audio_file_src})

set_target_properties(audio_file PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(audio_file
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(audio_file
    PUBLIC
        SndFile::sndfile
)
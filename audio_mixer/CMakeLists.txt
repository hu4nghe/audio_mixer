add_executable(audio_mixer src/audio_mixer.cpp)

set_target_properties(audio_mixer
    PROPERTIES
        CXX_STANDARD 23          
        CXX_STANDARD_REQUIRED ON 
        CXX_EXTENSIONS OFF       
)
 
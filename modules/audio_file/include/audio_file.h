/**
 * @file audio_file.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief audio mixer module to read from an audio file.
 * @version 0.1
 * @date 2025-10-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "audio_prop_def.h"
#include "sndfile.h"
#include "input_module_base.h"
#include <iostream>
#include <memory>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

template <audio_sample_type AudioType>
class audio_file : public input_module_base<AudioType>
{
    std::vector<fs::path> m_audios;
	std::vector<fs::path> m_videos;

public : 

    audio_file(std::shared_ptr<audio_queue<AudioType>> target_queue)
        : input_module_base<AudioType>(target_queue,false){}
    

    void select_file()
    {
        std::println("Please enter the path of the sound file.\nE(nd)");

        std::string file_path_str;
        while (true)
        {
            std::getline(std::cin >> std::ws, file_path_str);
            
            if (file_path_str == "E" || file_path_str == "e" )
            {
                std::println("File lists confirmed.");
                break;
            }

            fs::path file_path(file_path_str);
            if (!fs::exists(file_path))
            {
                std::println("No such file or directory.");
                continue;
            }

            const auto extension = file_path.extension();
            
            if (extension == ".wav")
                m_audios.push_back(file_path);
            else if (extension == ".mov" || extension == ".mp4")
                m_videos.push_back(file_path);
            
            else
            {
                std::println("Format not supported.");
                continue;
            }

            std::println("File selected: {}.", file_path.filename().string());
        }
    }
};
#pragma once

#include "VectorPP.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

inline std::string getSaveFilePath() {
    std::string path = "save.csv";
    #ifdef _WIN32
        char* appdata = std::getenv("LOCALAPPDATA");
        if (appdata != nullptr) {
            std::filesystem::path dir(appdata);
            dir /= "TheWhisperingWood";
            std::filesystem::create_directories(dir);
            dir /= "save.csv";
            path = dir.string();
        }
    #endif
    return path;
}
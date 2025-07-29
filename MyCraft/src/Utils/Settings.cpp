#include "Settings.hpp"

void Settings_Loader::Load_Settings(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Cant Open Settings File: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            config[key] = value;
        }
    }
}

int Settings_Loader::GetInt(const std::string& key, int default_value) const {
    try {
        return std::stoi(config.at(key));
    } catch (...) {
        return default_value;
    }
}

float Settings_Loader::GetFloat(const std::string& key, float default_value) const {
    try {
        return std::stof(config.at(key));
    } catch (...) {
        return default_value;
    }
}

std::string Settings_Loader::GetString(const std::string& key, const std::string& default_value) const {
    auto it = config.find(key);
    return (it != config.end()) ? it->second : default_value;
}
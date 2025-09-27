#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

class Settings_Loader {
public:
    void Load_Settings(const std::string& path);

    template<typename T>
    T Get(const std::string& key, const T& default_value) const {
        auto it = config.find(key);
        if (it == config.end()) return default_value;

        try {
            if constexpr (std::is_same_v<T,int>) return std::stoi(it->second);
            else if constexpr (std::is_same_v<T,float>) return std::stof(it->second);
            else if constexpr (std::is_same_v<T,double>) return std::stod(it->second);
            else if constexpr (std::is_same_v<T,std::string>) return it->second;
            else {
                std::istringstream ss(it->second);
                T value;
                ss >> value;
                if (ss.fail()) return default_value;
                return value;
            }
        } catch (...) {
            return default_value;
        }
    }

private:
    std::unordered_map<std::string, std::string> config;
};
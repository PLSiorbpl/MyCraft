#pragma once
#include <string>
#include <unordered_map>

class Settings_Loader {
public:
    void Load_Settings(const std::string& path);

    int GetInt(const std::string& key, int default_value = 0) const;
    float GetFloat(const std::string& key, float default_value = 0.0f) const;
    std::string GetString(const std::string& key, const std::string& default_value = "") const;

private:
    std::unordered_map<std::string, std::string> config;
};
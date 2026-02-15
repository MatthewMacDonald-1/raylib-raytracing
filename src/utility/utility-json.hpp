#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace Utility {

    namespace JSON {
        std::string ReadStringRequired(nlohmann::json json, std::string key);
        std::string ReadStringOptional(nlohmann::json json, std::string key, std::string defaultValue);

        int ReadIntegerRequired(nlohmann::json json, std::string key);
        int ReadIntegerOptional(nlohmann::json json, std::string key, int defaultValue);

        bool ReadBoolRequired(nlohmann::json json, std::string key);
        bool ReadBoolOptional(nlohmann::json json, std::string key, bool defaultValue);

        nlohmann::json ReadChildObject(nlohmann::json json, std::string key);
    }
}

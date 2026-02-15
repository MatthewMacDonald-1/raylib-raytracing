#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace Utility {

    namespace Time {
        /// @param time Unit: ms
        std::string TimeStringHMS(long long time, bool hide_leading_zeros = false);

        /// @param time Unit: ms
        std::string TimeStringHMS_TNS(long long time, bool hide_leading_zeros = false);
    }

    namespace Numbers {
        double parseDouble(std::string str);

        int parseDoubleSafe(std::string str, double& number);

        float parseFloat(std::string str);

        int parseLong(std::string str, long& number);

        int parseInt(std::string str, int& number);

        int parseIntHex(std::string str, unsigned int& number);

        int parseCharNum(std::string str, unsigned char& number);

        double average(std::vector<int> array);

        int max(std::vector<int> array);
    }

    namespace Strings {
        /// @brief Sourced from (https://stackoverflow.com/questions/5888022/split-string-by-single-spaces)
        /// @param txt String to be split
        /// @param strs Reference to the vector of strings
        /// @param ch The deliminator
        /// @return The size of the vector of strings
        size_t split(const std::string& txt, std::vector<std::string>& strs, char ch);

        std::string join(std::vector<std::string>& strs, char ch);

        /// @brief Sourced from (https://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string) answer
        /// @param str 
        /// @param whitespace 
        /// @return 
        std::string trim(const std::string& str, const std::string& whitespace = " \t");

        bool containsChar(const std::string& value, const char ch);

        /// <summary>
        /// Finds the first instance of the character in the string and returns the index.
        /// </summary>
        /// <param name="value"></param>
        /// <param name="ch"></param>
        /// <returns></returns>
        int findCharIndex(const std::string& value, const char ch);

        std::vector<int> findCharIndexes(const std::string& value, const char ch);

        /// <summary>
        /// Returns true if the value is in the array.
        /// </summary>
        /// <param name="value"></param>
        /// <param name="array"></param>
        /// <returns>If present.</returns>
        bool in_array(const std::string& value, const std::vector<std::string>& array);

        /// <summary>
        /// Finds the index of the value if present in the array.
        /// </summary>
        /// <param name="value"></param>
        /// <param name="array"></param>
        /// <returns>The index in the array otherwise -1.</returns>
        int findIndexInArray(const std::string& value, const std::vector<std::string>& array);
    }
}
#include "utility.hpp"

#include <cmath>
#include <algorithm>
#include <sstream>

std::string Utility::Time::TimeStringHMS(long long time, bool hide_leading_zeros)
{
    long long timeSec = time / 1000;
    long long timeMin = timeSec / 60;
    long long timeHour = timeMin / 60;

    timeSec = timeSec % 60;
    timeMin = timeMin % 60;

    bool lz_sec = timeSec < 10;
    bool lz_min = timeMin < 10;
    bool lz_hour = timeHour < 10;

    std::stringstream ss;

    // Add Hour
    if (timeHour > 0) {
        if (hide_leading_zeros) {
            if (timeHour > 0) {
                ss << timeHour << ':';
            }
        }
        else {
            ss << (lz_hour ? "0" : "") << timeHour << ':';
        }
    }

    // Add Minute
    if (hide_leading_zeros) {
        if (timeHour > 0 || timeMin > 0) {
            if (timeHour > 0) { // Allow a leading zero if the hour is non zero and min is less than 10
                ss << (lz_min ? "0" : "") << timeMin << ':';
            }
            else {
                ss << timeMin << ':';
            }
        }
    }
    else {
        ss << (lz_min ? "0" : "") << timeMin << ':';
    }

    // Add Second
    if (hide_leading_zeros) {
        if (timeHour > 0 && timeMin > 0) { // Allow a leading zero if the hour and minute is non zero and second is less than 10
            ss << (lz_sec ? "0" : "") << timeSec;
        }
        else {
            ss << timeSec;
        }
    }
    else {
        ss << (lz_sec ? "0" : "") << timeSec;
    }

    return ss.str();
}

std::string Utility::Time::TimeStringHMS_TNS(long long time, bool hide_leading_zeros)
{
    long long timeTNSec = time / 100;
    long long timeSec = timeTNSec / 10;
    long long timeMin = timeSec / 60;
    long long timeHour = timeMin / 60;

    timeTNSec = timeTNSec % 10;
    timeSec = timeSec % 60;
    timeMin = timeMin % 60;

    bool lz_sec = timeSec < 10;
    bool lz_min = timeMin < 10;
    bool lz_hour = timeHour < 10;

    std::stringstream ss;

    // Add Hour
    if (timeHour > 0) {
        if (hide_leading_zeros) {
            if (timeHour > 0) {
                ss << timeHour << ':';
            }
        }
        else {
            ss << (lz_hour ? "0" : "") << timeHour << ':';
        }
    }

    // Add Minute
    if (hide_leading_zeros) {
        if (timeHour > 0 || timeMin > 0) {
            if (timeHour > 0) { // Allow a leading zero if the hour is non zero and min is less than 10
                ss << (lz_min ? "0" : "") << timeMin << ':';
            }
            else {
                ss << timeMin << ':';
            }
        }
    }
    else {
        ss << (lz_min ? "0" : "") << timeMin << ':';
    }

    // Add Second
    if (hide_leading_zeros) {
        if (timeHour > 0 || timeMin > 0) { // Allow a leading zero if the hour and minute is non zero and second is less than 10
            ss << (lz_sec ? "0" : "") << timeSec;
        }
        else {
            ss << timeSec;
        }
    }
    else {
        ss << (lz_sec ? "0" : "") << timeSec;
    }

    // Add tenth of second
    ss << '.' << timeTNSec;

    return ss.str();
}


double Utility::Numbers::parseDouble(std::string str)
{
    char* endptr;
    double num = strtod(str.c_str(), &endptr);
    if (endptr == str.c_str()) {
        // No digits found
        std::cerr << "ERROR: failed to parse str (" << str << ") to double." << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (*endptr != '\0') {
        std::cerr << "ERROR: failed to parse str (" << str << ") to double." << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        return num;
    }
}

int Utility::Numbers::parseDoubleSafe(std::string str, double& number)
{
    char* endptr;
    double num = strtod(str.c_str(), &endptr);
    if (endptr == str.c_str()) {
        // No digits found
        return EXIT_FAILURE;
    }
    else if (*endptr != '\0') {
        return EXIT_FAILURE;
    }
    else {
        number = num;
        return EXIT_SUCCESS;
    }
}

float Utility::Numbers::parseFloat(std::string str)
{
    return (float)parseDouble(str);
}

int Utility::Numbers::parseLong(std::string str, long& number)
{
    char* endptr;
    long num = strtol(str.c_str(), &endptr, 10);
    if (endptr == str.c_str()) {
        // No digits found
        std::cerr << "ERROR: failed to parse str (" << str << ") to long." << std::endl;
        return EXIT_FAILURE;
    }
    else if (*endptr != '\0') {
        std::cerr << "ERROR: failed to parse str (" << str << ") to long." << std::endl;
        return EXIT_FAILURE;
    }
    else {
        number = num;
        return EXIT_SUCCESS;
    }
}

int Utility::Numbers::parseInt(std::string str, int& number)
{
    long parsedNum;
    int res = parseLong(str, parsedNum);
    number = (int)parsedNum;
    return res;
}

int Utility::Numbers::parseIntHex(std::string str, unsigned int& number)
{
    std::vector<int> nums;

    int sum = 0;

    for (int i = 0; i < str.length(); i++) {
        int value = 0;

        switch (str.at(i))
        {
        default:
            value = -1;
            break;
        case '0':
            value = 0;
            break;
        case '1':
            value = 1;
            break;
        case '2':
            value = 2;
            break;
        case '3':
            value = 3;
            break;
        case '4':
            value = 4;
            break;
        case '5':
            value = 5;
            break;
        case '6':
            value = 6;
            break;
        case '7':
            value = 7;
            break;
        case '8':
            value = 8;
            break;
        case '9':
            value = 9;
            break;
        case 'A':
            value = 10;
            break;
        case 'a':
            value = 10;
            break;
        case 'B':
            value = 11;
            break;
        case 'b':
            value = 11;
            break;
        case 'C':
            value = 12;
            break;
        case 'c':
            value = 12;
            break;
        case 'D':
            value = 13;
            break;
        case 'd':
            value = 13;
            break;
        case 'E':
            value = 14;
            break;
        case 'e':
            value = 14;
            break;
        case 'F':
            value = 15;
            break;
        case 'f':
            value = 15;
            break;

        }

        if (value == -1) {
            return EXIT_FAILURE;
        }

        nums.push_back(value);
    }


    int powerCount = 0;
    for (int i = nums.size() - 1; i << nums.size() >= 0; i--) {
        sum += nums.at(i) * (int)std::pow(16, powerCount);
        powerCount++;
    }

    number = sum;
    return EXIT_SUCCESS;
}

int Utility::Numbers::parseCharNum(std::string str, unsigned char& number)
{
    long parsedNum;
    int res = parseLong(str, parsedNum);
    number = (int)parsedNum;
    return res;
}

double Utility::Numbers::average(std::vector<int> array)
{
    double sum = 0;
    for (int i = 0; i < array.size(); i++) {
        sum += array.at(i);
    }

    return sum / (double)array.size();
}

int Utility::Numbers::max(std::vector<int> array)
{
    if (array.size() < 1) {
        std::cerr << "ERROR: cannot compute max from array of size zero (no values)." << std::endl;
        exit(EXIT_FAILURE);
    }
    int max = array.at(0);
    for (int i = 1; i < array.size(); i++) {
        if (array.at(i) > max) {
            max = array.at(i);
        }
    }
    return max;
}

// Strings

size_t Utility::Strings::split(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

    return strs.size();
}

std::string Utility::Strings::join(std::vector<std::string>& strs, char ch)
{
    std::stringstream ss;

    for (int i = 0; i < strs.size(); i++) {
        ss << strs.at(i);

        if (i < strs.size() - 1) {
            ss << ch;
        }
    }

    return ss.str();
}

std::string Utility::Strings::trim(const std::string& str, const std::string& whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

bool Utility::Strings::containsChar(const std::string& value, const char ch)
{
    for (int i = 0; i < value.size(); i++) {
        if (value.at(i) == ch) {
            return true;
        }
    }

    return false;
}

int Utility::Strings::findCharIndex(const std::string& value, const char ch)
{
    for (int i = 0; i < value.size(); i++) {
        if (value.at(i) == ch) {
            return i;
        }
    }

    return -1;
}

std::vector<int> Utility::Strings::findCharIndexes(const std::string& value, const char ch)
{
    std::vector<int> instances;

    for (int i = 0; i < value.size(); i++) {
        if (value.at(i) == ch) {
            instances.push_back(i);
        }
    }

    return instances;
}

bool Utility::Strings::in_array(const std::string& value, const std::vector<std::string>& array)
{
    return std::find(array.begin(), array.end(), value) != array.end();
}

int Utility::Strings::findIndexInArray(const std::string& value, const std::vector<std::string>& array)
{
    auto it = std::find(array.begin(), array.end(), value);

    if (it == array.end()) return -1; // Return -1 if not found

    return std::distance(array.begin(), it); // Return index

    //for (size_t i = 0; i < array.size(); i++) {
    //    if (array.at(i) == value) {
    //        return (int)i;
    //    }
    //}

    //return -1;
}


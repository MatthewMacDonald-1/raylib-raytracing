#pragma once

#include <iostream>
#include <stdexcept>

class JSONParseException : public std::runtime_error {
public:
    JSONParseException(const std::string& message) : std::runtime_error(message) {}
};
#pragma once
#include <string>

namespace Code
{
    std::string translateC(std::string s);
    std::string translateA(const std::string &s);
    void recordLabel(const std::string &s, int rom);
}
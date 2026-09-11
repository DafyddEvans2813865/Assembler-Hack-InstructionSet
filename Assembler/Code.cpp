#include "Code.h"
#include <unordered_map>
#include <bitset>
#include <cctype>

namespace
{
    const std::unordered_map<std::string, std::string> COMP = {
        {"0", "0101010"},
        {"1", "0111111"},
        {"-1", "0111010"},
        {"D", "0001100"},
        {"A", "0110000"},
        {"M", "1110000"},
        {"!D", "0001101"},
        {"!A", "0110001"},
        {"!M", "1110001"},
        {"-D", "0001111"},
        {"-A", "0110011"},
        {"-M", "1110011"},
        {"D+1", "0011111"},
        {"A+1", "0110111"},
        {"M+1", "1110111"},
        {"D-1", "0001110"},
        {"A-1", "0110010"},
        {"M-1", "1110010"},
        {"D+A", "0000010"},
        {"D+M", "1000010"},
        {"D-A", "0010011"},
        {"D-M", "1010011"},
        {"A-D", "0000111"},
        {"M-D", "1000111"},
        {"D&A", "0000000"},
        {"D&M", "1000000"},
        {"D|A", "0010101"},
        {"D|M", "1010101"},
    };

    const std::unordered_map<std::string, std::string> DEST = {
        {"", "000"},
        {"M", "001"},
        {"D", "010"},
        {"MD", "011"},
        {"A", "100"},
        {"AM", "101"},
        {"AD", "110"},
        {"AMD", "111"},
    };

    const std::unordered_map<std::string, std::string> JUMP = {
        {"", "000"},
        {"JGT", "001"},
        {"JEQ", "010"},
        {"JGE", "011"},
        {"JLT", "100"},
        {"JNE", "101"},
        {"JLE", "110"},
        {"JMP", "111"},
    };
}

std::unordered_map<std::string, int> symbols = {
    {"SP", 0},
    {"LCL", 1},
    {"ARG", 2},
    {"THIS", 3},
    {"THAT", 4},
    {"R0", 0},
    {"R1", 1},
    {"R2", 2},
    {"R3", 3},
    {"R4", 4},
    {"R5", 5},
    {"R6", 6},
    {"R7", 7},
    {"R8", 8},
    {"R9", 9},
    {"R10", 10},
    {"R11", 11},
    {"R12", 12},
    {"R13", 13},
    {"R14", 14},
    {"R15", 15},
    {"SCREEN", 16384},
    {"KBD", 24576},
};

int nextRam = 16;

enum class Type
{
    A,
    C,
    L
};

namespace Code
{
    /*
     * Strip the instruction: dest=comp;jump
     *
     *  E.g  "AM=D-1;JGT"
     *  →  jump="JGT", s="AM=D-1"   (chop at ';')
     *  →  dest="AM",  s="D-1"      (chop at '=')
     *  →  comp = s ="D-1"
     */
    std::string translateC(std::string s)
    {
        std::string dest, jump;

        // find jump e.g D ; JGT
        auto sc = s.find(';');
        if (sc != std::string::npos)
        {
            jump = s.substr(sc + 1);
            s.erase(sc);
        }

        // find dest e.g A M = D - 1
        auto eq = s.find('=');
        if (eq != std::string::npos)
        {
            dest = s.substr(0, eq);
            s.erase(0, eq + 1);
        }

        return "111" + COMP.at(s) + DEST.at(dest) + JUMP.at(jump);
    }

    std::string translateA(const std::string &s)
    {
        std::string sym = s.substr(1);
        int value;
        if (std::isdigit(static_cast<unsigned char>(sym[0])))
        {
            value = std::stoi(sym);
        }
        else
        {
            auto it = symbols.find(sym);
            if (it == symbols.end())
                it = symbols.emplace(sym, nextRam++).first;
            value = it->second;
        }
        return "0" + std::bitset<15>(value).to_string();
    }

    void recordLabel(const std::string &s, int rom)
    {
        symbols[s.substr(1, s.size() - 2)] = rom;
    }
}


#include "parser.h"

Parser::Parser(const std::string &filename) : currentIndex(-1)
{
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line))
    {
        line = clean(line);
        if (!line.empty())
        {
            commands.push_back(line);
        }
    }
}

bool Parser::hasMoreLines()
{
    return currentIndex + 1 < (int)commands.size();
}

void Parser::advance()
{
    currentIndex++;
}

CommandType Parser::commandType()
{
    std::istringstream iss(commands[currentIndex]);
    std::string first;
    iss >> first;
    if (first == "push")
        return CommandType::PUSH;
    if (first == "pop")
        return CommandType::POP;
    if (first == "label")
        return CommandType::LABEL;
    if (first == "goto")
        return CommandType::GOTO;
    if (first == "if-goto")
        return CommandType::IF;
    if (first == "function")
        return CommandType::FUNCTION;
    if (first == "call")
        return CommandType::CALL;
    if (first == "return")
        return CommandType::RETURN;
    return CommandType::ARITHMETIC;
}

std::string Parser::arg1()
{
    std::istringstream iss(commands[currentIndex]);
    std::string first, second;
    iss >> first;
    if (commandType() == CommandType::ARITHMETIC)
    {
        return first;
    }
    iss >> second;
    return second;
}

int Parser::arg2()
{
    std::istringstream iss(commands[currentIndex]);
    std::string first, second, third;
    iss >> first >> second >> third;
    return std::stoi(third);
}

std::string Parser::clean(std::string line)
{
    size_t comment = line.find("//");
    if (comment != std::string::npos)
    {
        line = line.substr(0, comment);
    }

    size_t start = line.find_first_not_of(" \t\r\n");

    if (start == std::string::npos)
        return "";

    size_t end = line.find_last_not_of(" \t\r\n");
    return line.substr(start, end - start + 1);
}

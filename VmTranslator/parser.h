#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "command_type.h"

class Parser
{
public:
    Parser(const std::string &filename);
    bool hasMoreLines();       // more commands to process?
    void advance();            // read next command, make it "current"
    CommandType commandType(); // type of current command
    std::string arg1();        // first argument
    int arg2();                // second argument
private:
    std::vector<std::string> commands;
    int currentIndex;

    static std::string clean(std::string line);
};
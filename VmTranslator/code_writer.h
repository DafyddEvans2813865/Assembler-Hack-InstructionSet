
#include <fstream>
#include <string>
#include <map>

#include "command_type.h"

class CodeWriter
{
public:
    CodeWriter(const std::string &filename);
    void setFileName(const std::string &baseName);
    void writeArithmetic(const std::string &command);
    void writePushPop(CommandType type, const std::string &segment, int index);
    void close();

private:
    std::ofstream out;
    std::string fileName = "Static";
    int labelCounter = 0;

    void pushD();
    void popD();
};
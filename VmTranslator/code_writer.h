
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
    void writeLabel(const std::string &label);
    void writeGoto(const std::string &label);
    void writeIf(const std::string &label);

    void close();

private:
    std::ofstream out;
    std::string fileName = "Static";
    std::string currentFunction;
    std::string scopedLabel(const std::string &label);

    int labelCounter = 0;

    void pushD();
    void popD();
};
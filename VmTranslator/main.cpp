#include "parser.h"
#include "code_writer.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: ./vmtranslator <file.vm> [output.asm]\n";
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath;
    if (argc == 3)
    {
        outputPath = argv[2];
    }
    else
    {
        outputPath = inputPath;
        outputPath.replace(outputPath.find_last_of('.') + 1, std::string::npos, "asm");
    }

    Parser parser(inputPath);
    CodeWriter writer(outputPath);
    writer.setFileName(inputPath.substr(inputPath.find_last_of("/\\") + 1));

    while (parser.hasMoreLines())
    {
        parser.advance();
        CommandType t = parser.commandType();
        if (t == CommandType::ARITHMETIC)
            writer.writeArithmetic(parser.arg1());
        else if (t == CommandType::LABEL)
            writer.writeLabel(parser.arg1());
        else if (t == CommandType::GOTO)
            writer.writeGoto(parser.arg1());
        else if (t == CommandType::IF)
            writer.writeIf(parser.arg1());
        else if (t == CommandType::FUNCTION)
            writer.writeFunction(parser.arg1(), parser.arg2());
        else if (t == CommandType::CALL)
            writer.writeCall(parser.arg1(), parser.arg2());
        else if (t == CommandType::RETURN)
            writer.writeReturn();
        else
            writer.writePushPop(t, parser.arg1(), parser.arg2());
    }
    writer.close();

    std::cout << "Translated " << inputPath << " -> " << outputPath << "\n";
    return 0;
}
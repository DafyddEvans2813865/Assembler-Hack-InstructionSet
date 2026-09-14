
#include "code_writer.h"

CodeWriter::CodeWriter(const std::string &outFilename) { out.open(outFilename); }

void CodeWriter::close()
{
    out.close();
}

void CodeWriter::setFileName(const std::string &baseName)
{
    fileName = baseName;
}

void CodeWriter::pushD()
{
    out << "@SP\nA=M\nM=D\n@SP\nM=M+1\n";
}
void CodeWriter::popD()
{
    out << "@SP\nM=M-1\nA=M\nD=M\n";
}

void CodeWriter::writePushPop(CommandType type, const std::string &segment, int index)
{
    static const std::map<std::string, std::string>
        base{
            {"local", "LCL"},
            {"argument", "ARG"},
            {"this", "THIS"},
            {"that", "THAT"}};

    if (type == CommandType::PUSH)
    {
        if (segment == "constant")
        {
            out << "@" << index << "\nD=A\n";
        }
        else if (base.count(segment))
        {
            out << "@" << index << "\nD=A\n@" << base.at(segment) << "\nA=M+D\nD=M\n";
        }
        else if (segment == "temp")
            out << "@" << (5 + index) << "\nD=M\n";
        else if (segment == "pointer")
            out << "@" << (3 + index) << "\nD=M\n";
        else if (segment == "static")
            out << "@" << fileName << "." << index << "\nD=M\n";
        pushD();
    }
    else
    {
        // pop
        if (base.count(segment))
        {
            out << "@" << index << "\nD=A\n@" << base.at(segment) << "\nD=M+D\n";
            out << "@R13\nM=D\n"; // stash destination address
            popD();
            out << "@R13\nA=M\nM=D\n";
        }
        else if (segment == "temp")
        {
            popD();
            out << "@" << (5 + index) << "\nM=D\n";
        }
        else if (segment == "pointer")
        {
            popD();
            out << "@" << (3 + index) << "\nM=D\n";
        }
        else if (segment == "static")
        {
            popD();
            out << "@" << fileName << "." << index << "\nM=D\n";
        }
    }
}

void CodeWriter::writeArithmetic(const std::string &command)
{
    if (command == "add" || command == "sub" || command == "and" || command == "or")
    {
        out << "@SP\nM=M-1\nA=M\nD=M\n"; // D = y
        out << "@SP\nM=M-1\nA=M\n";      // A = address of x
        if (command == "add")
            out << "M=M+D\n";
        else if (command == "sub")
            out << "M=M-D\n";
        else if (command == "and")
            out << "M=M&D\n";
        else if (command == "or")
            out << "M=M|D\n";
        out << "@SP\nM=M+1\n";
    }
    else if (command == "neg" || command == "not")
    {
        out << "@SP\nA=M-1\n"; // A = top value
        out << (command == "neg" ? "M=-M\n" : "M=!M\n");
    }
    else
    { // eq, gt, lt
        std::string jump = (command == "eq") ? "JEQ" : (command == "gt") ? "JGT"
                                                                         : "JLT";
        std::string trueL = "TRUE_" + std::to_string(labelCounter);
        std::string endL = "END_" + std::to_string(labelCounter);
        labelCounter++;

        out << "@SP\nM=M-1\nA=M\nD=M\n"; // D = y
        out << "@SP\nM=M-1\nA=M\n";      // A = addr of x
        out << "D=M-D\n";                // D = x - y
        out << "@" << trueL << "\nD;" << jump << "\n";
        out << "@SP\nA=M\nM=0\n"; // false → 0
        out << "@" << endL << "\n0;JMP\n";
        out << "(" << trueL << ")\n@SP\nA=M\nM=-1\n"; // true → -1
        out << "(" << endL << ")\n";
        out << "@SP\nM=M+1\n";
    }
}

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

std::string CodeWriter::scopedLabel(const std::string &label)
{
    return currentFunction + "_" + label;
}

void CodeWriter::writeLabel(const std::string &label)
{
    out << "(" << scopedLabel(label) << ")" << "\n";
}

void CodeWriter::writeGoto(const std::string &label)
{
    out << "@" << scopedLabel(label) << "\n"
        << "0;JMP\n";
}

void CodeWriter::writeIf(const std::string &label)
{
    popD();
    out << "@" << scopedLabel(label) << "\n"
        << "D;JNE" << "\n";
}

void CodeWriter::writeFunction(const std::string &functionName, int nVars)
{
    // entry point
    currentFunction = functionName;
    out << "(" << currentFunction << ")" << "\n";

    // initialize nVars locals to 0.
    for (int i = 0; i < nVars; i++)
    {
        out << "@" << 0 << "\nD=A\n";
        pushD();
    }
}
void CodeWriter::pushPointerValue(const std::string &pointer)
{
    out << "@" << pointer << "\nD=M\n";
    pushD();
}

void CodeWriter::writeCall(const std::string &functionName, int nArgs)
{

    std::string returnLabel = functionName + "_" + std::to_string(callCounter); // fix this for me
    callCounter++;

    out << "@" << returnLabel << "\nD=A\n";
    pushD();
    // push LCL,ARG,THIS,THAT   — raw contents of the LCL pointer itself, no dereference
    pushPointerValue("LCL");
    pushPointerValue("ARG");
    pushPointerValue("THIS");
    pushPointerValue("THAT");

    // 6. ARG = SP - 5 - nArgs
    out << "@SP\nD=M\n@" << 5 + nArgs << "\nD=D-A\n@ARG\nM=D\n";
    // 7. LCL = SP
    out << "@SP\nD=M\n@LCL\nM=D\n";
    // 8. goto functionName
    out << "@" << functionName << "\n"
        << "0;JMP\n";
    out << "(" << returnLabel << ")" << "\n";
}

void CodeWriter::writeReturn()
{
    // endFrame (R13) = LCL
    out << "@LCL\nD=M\n@R13\nM=D\n";

    // retAddr (R14) = *(endFrame - 5)   -- MUST happen before ARG gets overwritten
    out << "@R13\nD=M\n@5\nA=D-A\nD=M\n@R14\nM=D\n";

    // *ARG = pop()   -- return value replaces arg 0
    out << "@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n";

    // SP = ARG + 1
    out << "@ARG\nD=M+1\n@SP\nM=D\n";

    // THAT = *(endFrame - 1)
    out << "@R13\nD=M\n@1\nA=D-A\nD=M\n@THAT\nM=D\n";

    // THIS = *(endFrame - 2)
    out << "@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n";

    // ARG = *(endFrame - 3)
    out << "@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n";

    // LCL = *(endFrame - 4)
    out << "@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n";

    // goto retAddr  -- jump to the VALUE stored in R14, not to R14 itself
    out << "@R14\nA=M\n0;JMP\n";
}